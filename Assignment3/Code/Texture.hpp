//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        if (image_data.empty())
        {
            std::cerr << "[Texture] ERROR: failed to load image: " << name << std::endl;
            width = 0;
            height = 0;
            return;
        }
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        if (image_data.empty() || width == 0 || height == 0)
            return Eigen::Vector3f(255, 0, 255);
        u = std::max(0.0f, std::min(1.0f, u));
        v = std::max(0.0f, std::min(1.0f, v));
        auto u_img = u * (width - 1);
        auto v_img = (1 - v) * (height - 1);
        int ui = static_cast<int>(std::round(u_img));
        int vi = static_cast<int>(std::round(v_img));
        ui = std::max(0, std::min(width - 1, ui));
        vi = std::max(0, std::min(height - 1, vi));
        auto color = image_data.at<cv::Vec3b>(vi, ui);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    // Bonus2: 双线性纹理插值采样
    // 与 getColor 保持一致的 (u,v) -> 图像坐标映射：
    //   u_img = u * (width  - 1)
    //   v_img = (1 - v) * (height - 1)   // 注意 v 轴翻转
    // 取相邻四个纹素做双线性加权，消除最近邻的块状锯齿。
    Eigen::Vector3f getColorBilinear(float u, float v)
    {
        if (image_data.empty() || width == 0 || height == 0)
            return Eigen::Vector3f(255, 0, 255);

        u = std::max(0.0f, std::min(1.0f, u));
        v = std::max(0.0f, std::min(1.0f, v));

        float u_img = u * (width  - 1);
        float v_img = (1.0f - v) * (height - 1);

        // 连续坐标的整数部分与小数部分
        int x0 = static_cast<int>(std::floor(u_img));
        int y0 = static_cast<int>(std::floor(v_img));
        int x1 = std::min(x0 + 1, width  - 1);
        int y1 = std::min(y0 + 1, height - 1);
        float s = u_img - x0;   // 水平方向插值因子
        float t = v_img - y0;   // 垂直方向插值因子

        // 四个角纹素 (BGR 顺序，与 getColor 一致)
        auto c00 = image_data.at<cv::Vec3b>(y0, x0);
        auto c10 = image_data.at<cv::Vec3b>(y0, x1);
        auto c01 = image_data.at<cv::Vec3b>(y1, x0);
        auto c11 = image_data.at<cv::Vec3b>(y1, x1);

        auto to_vec = [](const cv::Vec3b& c) {
            return Eigen::Vector3f(c[0], c[1], c[2]);
        };

        // 先在 x 方向插值，再在 y 方向插值
        Eigen::Vector3f top = (1.0f - s) * to_vec(c00) + s * to_vec(c10);
        Eigen::Vector3f bot = (1.0f - s) * to_vec(c01) + s * to_vec(c11);
        return (1.0f - t) * top + t * bot;
    }

};
#endif //RASTERIZER_TEXTURE_H
