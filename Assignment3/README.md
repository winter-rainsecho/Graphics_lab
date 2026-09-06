# Assignment3 Bonus 1 / Bonus 2

## 完成情况
完成 Bonus 1（3 分）：已尝试 4 个额外模型，全部位于 `Code/models/` 并包含 Vertex Normal 信息，渲染结果保存在 `images/`。

## 模型清单
| 模型 | OBJ 文件 | 顶点法线来源 |
|---|---|---|
| cube | `Code/models/cube/cube.obj` | 原文件自带 |
| rock | `Code/models/rock/rock.obj` | 原文件自带 |
| bunny | `Code/models/bunny/bunny.obj` | 通过面法线加权生成并写入 |
| Crate | `Code/models/Crate/Crate1.obj` | 通过面法线加权生成并写入 |

## 渲染结果
| 模型 | normal 着色 | phong 着色 |
|---|---|---|
| cube | `images/cube_normal.png` | `images/cube_phong.png` |
| rock | `images/rock_normal.png` | `images/rock_phong.png` |
| bunny | `images/bunny_normal.png` | `images/bunny_phong.png` |
| Crate | `images/Crate_normal.png` | `images/Crate_phong.png` |

---

## Bonus 2 — 双线性纹理插值

### 完成情况
完成 Bonus 2（5 分）：在 `Texture` 类中实现 `getColorBilinear(float u, float v)`，新增 `bilinear_fragment_shader` 调用该方法；为凸显效果，将 `spot_texture.png` 下采样到 **128×128** 生成 `spot_texture_small.png`，并提交最近邻与双线性采样结果及对比。

### 实现要点
- `Code/Texture.hpp::getColorBilinear`：
  - 保持与 `getColor` 一致的坐标映射：`u_img = u*(W-1)`、`v_img = (1-v)*(H-1)`，并对 `u, v` 做 clamp。
  - 取相邻四个纹素，在 x、y 方向分别做线性插值，得到平滑过渡的颜色。
- `Code/main.cpp::bilinear_fragment_shader`：
  - 与 `texture_fragment_shader` 完全相同的光照模型（Blinn-Phong），仅把 `getColor` 换成 `getColorBilinear`。
- 命令行使用：
  - `./Rasterizer out.png bilinear spot`：使用 `spot_texture_small.png` 渲染双线性采样结果。
  - `./Rasterizer out.png texture spot small`：使用 `spot_texture_small.png` 渲染最近邻采样结果，便于公平对比。

### 对比结果
| 采样方式 | 使用纹理 | 渲染结果 | 视觉差异 |
|---|---|---|---|
| 最近邻 (Nearest) | 128×128 `spot_texture_small.png` | `images/spot_texture_nearest.png` | 牛斑、嘴部、眼部出现明显块状像素，锯齿感强 |
| 双线性 (Bilinear) | 128×128 `spot_texture_small.png` | `images/spot_texture_bilinear.png` | 颜色在纹素之间平滑过渡，块状感显著减弱 |

### 结论
在纹理分辨率远低于屏幕像素覆盖范围时（本例 128×128 纹理被拉伸到约 700×700 画面），**最近邻采样**会直接将单个纹素的颜色放大为屏幕上的大块像素，产生明显锯齿；**双线性插值**通过相邻四个纹素加权混合，把突变变成连续渐变，视觉质量更高。这是纹理放大（magnification）场景下最常见的反锯齿手段。

## 主要代码修改
- `Code/main.cpp`：
  - 支持命令行参数 `argv[3]` 选择模型，例如 `./Rasterizer out.png normal cube`。
  - 新增 `get_model_matrix_fit(center, scale, angle)`，根据模型包围盒自动居中并缩放到合适尺寸，使不同尺度模型都能正确显示。
  - 为不同模型配置对应 `.obj` 与纹理文件；保留 `spot` 的原有行为（height map 仍为 `hmap.jpg`，texture shader 仍为 `spot_texture.png`）。
- `Code/models/bunny/bunny.obj`、`Code/models/Crate/Crate1.obj`：
  - 增加 `vn` 顶点法线数据，并改写面索引为 `v//vn` / `v/vt/vn`，满足 Bonus1 对 Vertex Normal 的要求。

## 运行方式
在 `Code/build` 目录下：
```bash
./Rasterizer ../../images/<model>_<shader>.png <shader> <model>
```
其中 `<shader>` 可选 `normal` / `phong` / `texture` / `bilinear` / `bump` / `displacement`，`<model>` 可选 `cube` / `rock` / `bunny` / `Crate` / `spot`。
- `texture spot` 使用原始 `spot_texture.png`（1024×1024）渲染最近邻采样。
- `texture spot small` 使用 `spot_texture_small.png`（128×128）渲染最近邻采样，用于 Bonus2 对比。
- `bilinear spot` 使用 `spot_texture_small.png`（128×128）渲染双线性插值采样。

如果直接执行提示缺少 DLL，请确保 MinGW 与 OpenCV bin 目录在 PATH 中：
```bash
export PATH="/e/Trae_for_html/Graphics_lab/dependencies/opencv-4.12.0/opencv_install/x64/mingw/bin:/c/Program Files/mingw64/bin:$PATH"
```

## 其它得分点
除 Bonus1 外，作业核心功能（参数插值、Blinn-Phong、Texture Mapping、Bump Mapping、Displacement Mapping）及 Bonus2（双线性纹理插值）已实现于 `Code/main.cpp` 与 `Code/Texture.hpp`。
