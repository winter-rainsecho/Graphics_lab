//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include <iostream>
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

};
#endif //RASTERIZER_TEXTURE_H
