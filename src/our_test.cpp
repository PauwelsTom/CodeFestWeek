#include <algorithm> 
#include <csignal>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include "../include_lib/libfreenect.h"
#include "testZZZ.hh"
#include "traitement.h"

unsigned long long IID_RGB = 0u;
unsigned long long IID_DPT = 0u;

volatile bool running = true;

int height = 480;
int width = 640;
int xmin = 100;
int xmax = 640 - 100;
int ymin = 110;
int ymax = 480 - 135;


void sighand(int signal)
{
    if (signal == SIGINT || signal == SIGTERM || signal == SIGQUIT)
    {
        running = false;
    }

    exit(0);
}

uint8_t* process_depth(std::vector<uint16_t> depth_vector) {
    for (int i = 0; i < 3; i++) {
        apply_mean_filter(depth_vector, width, height);
    }

    int min, max;
    min_max(depth_vector, width, height, min, max);




    // std::vector<std::vector<uint16_t>> traj = getFirstpointTraj(depth_vector, width, height, xmin, xmax, ymin, ymax, 1000);


    static std::vector<std::vector<std::vector<uint16_t>>> trajs = getTraj(depth_vector,
                                            width, height,
                                            /*xmin, xmax, ymin, ymax,*/ 4);
    std::vector<std::vector<uint16_t>> traj = getFirstpointFromTraj(trajs,
                                            width, height, 1);
    


    std::set<std::pair<uint16_t, uint16_t>> trajectorySet;
    for (const auto& point : traj) {
        trajectorySet.insert({point[0], point[1]});
    }
    const double cMin = min;
    const double cMax = max;
    for (auto& element : depth_vector)
        element = static_cast<uint16_t>(((element - cMin) / (cMax - cMin)) * 255);

    cv::Mat depth_img(height, width, CV_8UC3);
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            unsigned i = y * width + x;
            int nb = static_cast<int>(depth_vector[i]);
            cv::Vec3b& color = depth_img.at<cv::Vec3b>(y, x);
            if (isInTrajectory(trajectorySet, x, y)) {
                color = cv::Vec3b(0, 0, 255); // Red color for trajectory
            } else if (nb <= 45) {
                color = cv::Vec3b(86, 87, 93); // Dark gray
            } else if (nb > 45 && nb <= 50) {
                color = cv::Vec3b(116, 117, 113); // Light gray
            } else if (nb > 50 && nb <= 170) {
                color = cv::Vec3b(32, 128, 0); // Green
            } else if (nb > 170 && nb <= 210) {
                color = cv::Vec3b(62, 255, 255); // Yellow
            } else if (nb > 210 && nb <= 225) {
                color = cv::Vec3b(255, 127, 0); // Cyan
            } else if (nb > 225) {
                color = cv::Vec3b(153, 51, 0); // Dark blue
            } else {
                color = cv::Vec3b(nb, nb, nb); // Default grayscale
            }
        }
    }

    uint8_t* res = new uint8_t[depth_img.total() * depth_img.elemSize()];
    std::memcpy(res, depth_img.data, depth_img.total() * depth_img.elemSize());
    return res;
}



void depth_cb(freenect_device* dev, void* data, uint32_t timestamp)
{
    uint16_t* depth = static_cast<uint16_t*>(data);
    std::vector<uint16_t> depth_vector(depth, depth + width * height);
    static std::vector<std::vector<uint16_t>> imgs;
    imgs.push_back(depth_vector);

    if (imgs.size() == 10)
    {

        // Get the dimensions of the images
        size_t numImages = imgs.size();
        size_t rows = height;
        size_t cols = width;
        // Vector to store the mean image
        std::vector<double> meanImage(rows * cols, 0.0);

        // Calculate the mean for each pixel
        for (const auto& image : imgs) {
            for (size_t r = 0; r < rows; ++r) {
                for (size_t c = 0; c < cols; ++c) {
                    meanImage[r * cols + c] += image[r * cols + c];
                }
            }
        }

        // Divide each pixel by the number of images to get the mean
        for (size_t i = 0; i < meanImage.size(); ++i) {
            meanImage[i] /= numImages;
        }

        // Convert the mean image to uint16_t
        std::vector<uint16_t> meanImageUint16(meanImage.size());
        for (size_t i = 0; i < meanImage.size(); ++i) {
            meanImageUint16[i] = static_cast<uint16_t>(meanImage[i]);
        }
        // Open a file to save the mean image
        std::ofstream outFile("data/mean_image.txt");
        if (!outFile.is_open()) {
            std::cerr << "Failed to open the output file." << std::endl;
            return;
        }

        // Write the mean image to the file
        for (const auto& pixel : meanImageUint16) {
            outFile << pixel << "\n";
        }

        outFile.close();

    }

}

void lave_cb(freenect_device* dev, void* data, uint32_t timestamp)
{

    // uint16_t* depth = static_cast<uint16_t*>(data);
    // std::vector<uint16_t> curr_depth_vector(depth, depth + width * height);

    std::vector<uint16_t> depth_vector;
    std::ifstream inFile("data/mean_image.txt");
    if (!inFile) {
        std::cerr << "Failed to open the file: " << "data/mean_image.txt" << std::endl;
        return;
    }
    uint16_t value;
    while (inFile >> value) {
        depth_vector.push_back(value);
    }
    inFile.close();

    uint8_t* processed_depth = process_depth(depth_vector);

    cv::Mat depth_img(height, width, CV_8UC3, processed_depth);
    cv::imshow("Depth", depth_img);
    cv::waitKey(1);
    delete[] processed_depth; // Don't forget to free the allocated memory
}

void rgb_cb(freenect_device* dev, void* data, uint32_t timestamp)
{
    uint8_t* rgb = static_cast<uint8_t*>(data);
    cv::Mat rgb_img(height, width, CV_8UC3, rgb);
    cv::cvtColor(rgb_img, rgb_img, cv::COLOR_RGB2BGR);
    cv::imshow("RGB", rgb_img);
    cv::waitKey(1);
}