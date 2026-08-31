#include <iostream>
#include <opencv2/opencv.hpp>

#include "global.hpp"
#include "rasterizer.hpp"
#include "Triangle.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "OBJ_Loader.h"

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1,0,0,-eye_pos[0],
                 0,1,0,-eye_pos[1],
                 0,0,1,-eye_pos[2],
                 0,0,0,1;

    view = translate*view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float angle)
{
    Eigen::Matrix4f rotation;
    angle = angle * MY_PI / 180.f;
    rotation << cos(angle), 0, sin(angle), 0,
                0,  1, 0, 0,
                -sin(angle), 0, cos(angle), 0,
                0, 0, 0, 1;

    Eigen::Matrix4f scale;
    scale << 2.5, 0, 0, 0,
              0, 2.5, 0, 0,
              0, 0, 2.5, 0,
              0, 0, 0, 1;

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1;

    return translate * rotation * scale;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
{
    // 注意: 此函数输入参数zNear和zFar必须大于0，代表绝对值或距离。|
    // 因此这里的矩阵设计增加了符号，与符号解耦，因此矩阵不是在坐标语义而是在距离语义下给出的。
    // 第一步：透视投影 -> 正交投影（视锥压成长方体，近平面上点不动）
    Eigen::Matrix4f persp2ortho = Eigen::Matrix4f::Identity();
    persp2ortho << -zNear, 0,      0,           0,
                   0,     -zNear,  0,           0,
                   0,     0,      (zNear+zFar),  -zNear*zFar,
                   0,     0,      1,           0;

    // 第二步：正交投影 -> NDC 立方体 [-1,1]^3（由 fov/aspect 求半高半宽）
    float fov_rad = eye_fov * MY_PI / 180.0f;
    float t = zNear * std::tan(fov_rad / 2.0f);   // 近平面半高
    float r = t * aspect_ratio;                   // 近平面半宽
    Eigen::Matrix4f ortho = Eigen::Matrix4f::Identity();

    // 使最终图像与视图空间方向一致(视图上=图上)。
    ortho << 1.0f/r,  0,            0,             0,
             0,           1.0f/t,   0,             0,
             0,           0,            -2.0f/(zFar-zNear), (zFar+zNear)/(zFar-zNear),
             0,           0,            0,             1;
    // 这个矩阵需要看做平移后缩放得到的，可以看到缩放的部分都由正数组成。
    return ortho * persp2ortho;   // 先压缩、再归一化，顺序不可颠倒
}

Eigen::Vector3f vertex_shader(const vertex_shader_payload& payload)
{
    return payload.position;
}

Eigen::Vector3f normal_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f return_color = (payload.normal.head<3>().normalized() + Eigen::Vector3f(1.0f, 1.0f, 1.0f)) / 2.f;
    Eigen::Vector3f result;
    result << return_color.x() * 255, return_color.y() * 255, return_color.z() * 255;
    return result;
}

static Eigen::Vector3f reflect(const Eigen::Vector3f& vec, const Eigen::Vector3f& axis)
{
    auto costheta = vec.dot(axis);
    return (2 * costheta * axis - vec).normalized();
}

struct light
{
    Eigen::Vector3f position;
    Eigen::Vector3f intensity;
};

Eigen::Vector3f texture_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f return_color = {0, 0, 0};
    if (payload.texture)
    {
        // TODO: Get the texture value at the texture coordinates of the current fragment
        return_color = payload.texture->getColor(payload.tex_coords.x(), payload.tex_coords.y());

    }
    Eigen::Vector3f texture_color;
    texture_color << return_color.x(), return_color.y(), return_color.z();

    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = texture_color / 255.f;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = texture_color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = ka.cwiseProduct(amb_light_intensity);

    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.
        // ===== Blinn-Phong 光照计算 =====
        Eigen::Vector3f light_dir = light.position - point;
        float r2 = light_dir.squaredNorm();
        Eigen::Vector3f L = light_dir.normalized();
        Eigen::Vector3f V = (eye_pos - point).normalized();
        Eigen::Vector3f h = (L + V).normalized();

        Eigen::Vector3f diffuse  = kd.cwiseProduct(light.intensity / r2) * std::max(0.0f, normal.dot(L));
        Eigen::Vector3f specular = ks.cwiseProduct(light.intensity / r2) * std::pow(std::max(0.0f, normal.dot(h)), p);
        result_color += diffuse + specular;
    }

    return result_color * 255.f;
}

Eigen::Vector3f phong_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = ka.cwiseProduct(amb_light_intensity);  // ambient 只加一次
    for (auto& light : lights)
    {
        // ===== Blinn-Phong: 对每盏灯累加 ambient + diffuse + specular =====
        Eigen::Vector3f light_dir = light.position - point;   // 指向光源的向量
        float r2 = light_dir.squaredNorm();                   // 距离平方（衰减用）
        Eigen::Vector3f L = light_dir.normalized();           // 光方向单位向量

        Eigen::Vector3f V = (eye_pos - point).normalized();   // 视线方向单位向量
        Eigen::Vector3f h = (L + V).normalized();             // 半程向量（Blinn-Phong 核心）

        Eigen::Vector3f diffuse  = kd.cwiseProduct(light.intensity / r2) * std::max(0.0f, normal.dot(L));
        Eigen::Vector3f specular = ks.cwiseProduct(light.intensity / r2) * std::pow(std::max(0.0f, normal.dot(h)), p);

        result_color += diffuse + specular;
    }

    return result_color * 255.f;
}


Eigen::Vector3f displacement_fragment_shader(const fragment_shader_payload& payload)
{
    
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color; 
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    float kh = 0.2, kn = 0.1;
    
    // TODO: Implement displacement mapping here
    // Let n = normal = (x, y, z)
    // Vector t = (x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),z*y/sqrt(x*x+z*z))
    // Vector b = n cross product t
    // Matrix TBN = [t b n]
    // dU = kh * kn * (h(u+1/w,v)-h(u,v))
    // dV = kh * kn * (h(u,v+1/h)-h(u,v))
    // Vector ln = (-dU, -dV, 1)
    // Position p = p + kn * n * h(u,v)
    // Normal n = normalize(TBN * ln)


    // ===== Displacement Mapping 实现 =====
    float x = normal.x();
    float y = normal.y();
    float z = normal.z();

    // 1. 切线向量 t（注意法线沿 Y 轴时 x²+z²≈0 的除零保护）
    float sqrt_xz = std::sqrt(x * x + z * z);
    Eigen::Vector3f t;
    if (sqrt_xz < 1e-6f) {
        t = Eigen::Vector3f(1, 0, 0);
    } else {
        t = Eigen::Vector3f(x * y / sqrt_xz, sqrt_xz, z * y / sqrt_xz);
    }

    // 2. 副切线 b，构建 TBN 矩阵（列向量形式）
    Eigen::Vector3f b = normal.cross(t);
    Eigen::Matrix3f TBN;
    TBN.col(0) = t;
    TBN.col(1) = b;
    TBN.col(2) = normal;

    // 3. 从高度图采样当前点、右侧点、上侧点的高度值
    float u = payload.tex_coords.x();
    float v = payload.tex_coords.y();
    int w = payload.texture->width;
    int h = payload.texture->height;

    float h_uv = payload.texture->getColor(u, v).norm();
    float h_u1 = payload.texture->getColor(u + 1.0f / w, v).norm();
    float h_v1 = payload.texture->getColor(u, v + 1.0f / h).norm();

    // 4. 高度梯度 -> 切线空间法线扰动
    float dU = kh * kn * (h_u1 - h_uv);
    float dV = kh * kn * (h_v1 - h_uv);
    Eigen::Vector3f ln(-dU, -dV, 1.0f);

    // 5. 沿原始法线方向位移顶点位置（位移贴图与 bump 的核心区别）
    point = point + kn * normal * h_uv;

    // 6. 扰动法线从切线空间转到视图空间
    normal = (TBN * ln).normalized();

    Eigen::Vector3f result_color = ka.cwiseProduct(amb_light_intensity);

    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.
        // ===== Blinn-Phong 光照（ambient 已在循环外初始化）=====
        Eigen::Vector3f light_dir = light.position - point;   // 指向光源的向量
        float r2 = light_dir.squaredNorm();                   // 距离平方（衰减用）
        Eigen::Vector3f L = light_dir.normalized();           // 光方向单位向量
        Eigen::Vector3f V = (eye_pos - point).normalized();   // 视线方向单位向量
        Eigen::Vector3f half_vec = (L + V).normalized();      // 半程向量（Blinn-Phong 核心）

        Eigen::Vector3f diffuse  = kd.cwiseProduct(light.intensity / r2) * std::max(0.0f, normal.dot(L));
        Eigen::Vector3f specular = ks.cwiseProduct(light.intensity / r2) * std::pow(std::max(0.0f, normal.dot(half_vec)), p);
        result_color += diffuse + specular;

    }

    return result_color * 255.f;
}
 


Eigen::Vector3f bump_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    float kh = 0.2, kn = 0.1;

    // ===== Bump Mapping =====
    float x = normal.x();
    float y = normal.y();
    float z = normal.z();

    // 1. 计算切线向量 t，注意避免除零（法线沿 Y 轴时 x²+z²≈0）
    float sqrt_xz = std::sqrt(x * x + z * z);
    Eigen::Vector3f t;
    if (sqrt_xz < 1e-6f) {
        t = Eigen::Vector3f(1, 0, 0);
    } else {
        t = Eigen::Vector3f(x * y / sqrt_xz, sqrt_xz, z * y / sqrt_xz);
    }

    // 2. 计算副切线 b，构建 TBN 矩阵（列向量形式）
    Eigen::Vector3f b = normal.cross(t);
    Eigen::Matrix3f TBN;
    TBN.col(0) = t;
    TBN.col(1) = b;
    TBN.col(2) = normal;

    // 3. 从高度图采样当前点、右侧点、上侧点的高度值
    float u = payload.tex_coords.x();
    float v = payload.tex_coords.y();
    int w = payload.texture->width;
    int h = payload.texture->height;

    float h_uv = payload.texture->getColor(u, v).norm();
    float h_u1 = payload.texture->getColor(u + 1.0f / w, v).norm();
    float h_v1 = payload.texture->getColor(u, v + 1.0f / h).norm();

    // 4. 计算高度梯度，得到切线空间法线扰动
    float dU = kh * kn * (h_u1 - h_uv);
    float dV = kh * kn * (h_v1 - h_uv);
    Eigen::Vector3f ln(-dU, -dV, 1.0f);

    // 5. 将扰动法线从切线空间转换到视图空间并归一化
    Eigen::Vector3f n = (TBN * ln).normalized();

    Eigen::Vector3f result_color = {0, 0, 0};
    result_color = n;
    // [-1,1]法向量范围到[0,1]

    return result_color * 255.f;
}

int main(int argc, const char** argv)
{
    std::vector<Triangle*> TriangleList;

    float angle = 140.0;
    bool command_line = false;

    std::string filename = "output.png";
    objl::Loader Loader;
    std::string obj_path = "../models/spot/";

    // Load .obj File
    bool loadout = Loader.LoadFile("../models/spot/spot_triangulated_good.obj");
    for(auto mesh:Loader.LoadedMeshes)
    {
        for(int i=0;i<mesh.Vertices.size();i+=3)
        {
            Triangle* t = new Triangle();
            for(int j=0;j<3;j++)
            {
                t->setVertex(j,Vector4f(mesh.Vertices[i+j].Position.X,mesh.Vertices[i+j].Position.Y,mesh.Vertices[i+j].Position.Z,1.0));
                t->setNormal(j,Vector3f(mesh.Vertices[i+j].Normal.X,mesh.Vertices[i+j].Normal.Y,mesh.Vertices[i+j].Normal.Z));
                t->setTexCoord(j,Vector2f(mesh.Vertices[i+j].TextureCoordinate.X, mesh.Vertices[i+j].TextureCoordinate.Y));
            }
            TriangleList.push_back(t);
        }
    }

    rst::rasterizer r(700, 700);

    auto texture_path = "hmap.jpg";
    r.set_texture(Texture(obj_path + texture_path));

    std::function<Eigen::Vector3f(fragment_shader_payload)> active_shader = phong_fragment_shader;

    if (argc >= 2)
    {
        command_line = true;
        filename = std::string(argv[1]);

        if (argc == 3 && std::string(argv[2]) == "texture")
        {
            std::cout << "Rasterizing using the texture shader\n";
            active_shader = texture_fragment_shader;
            texture_path = "spot_texture.png";
            r.set_texture(Texture(obj_path + texture_path));
        }
        else if (argc == 3 && std::string(argv[2]) == "normal")
        {
            std::cout << "Rasterizing using the normal shader\n";
            active_shader = normal_fragment_shader;
        }
        else if (argc == 3 && std::string(argv[2]) == "phong")
        {
            std::cout << "Rasterizing using the phong shader\n";
            active_shader = phong_fragment_shader;
        }
        else if (argc == 3 && std::string(argv[2]) == "bump")
        {
            std::cout << "Rasterizing using the bump shader\n";
            active_shader = bump_fragment_shader;
        }
        else if (argc == 3 && std::string(argv[2]) == "displacement")
        {
            std::cout << "Rasterizing using the bump shader\n";
            active_shader = displacement_fragment_shader;
        }
    }

    Eigen::Vector3f eye_pos = {0,0,10};

    r.set_vertex_shader(vertex_shader);
    r.set_fragment_shader(active_shader);

    int key = 0;
    int frame_count = 0;

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);
        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45.0, 1, 0.1, 50));

        std::cout << "[1] 模型加载完成，三角形数: " << TriangleList.size() << std::endl;
        //r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);
        r.draw(TriangleList);
        std::cout << "[2] 光栅化完成" << std::endl;  
        // ← 如果这行没打印，说明 draw() 里崩了

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        std::cout << "[3] Mat 创建完成" << std::endl;

        image.convertTo(image, CV_8UC3, 1.0f);
        
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imwrite(filename, image);
        std::cout << "[4] 图片保存完成" << std::endl;  
        // ← 如果这行没打印，说明 imwrite 前已崩
        

        return 0;
    }

    while(key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45.0, 1, 0.1, 50));
        r.draw(TriangleList);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imshow("image", image);
        cv::imwrite(filename, image);
        key = cv::waitKey(10);

        if (key == 'a' )
        {
            angle -= 0.1;
        }
        else if (key == 'd')
        {
            angle += 0.1;
        }

    }
    return 0;
}
