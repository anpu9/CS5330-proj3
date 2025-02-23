/*
 * Authors: Yuyang Tian and Arun Mekkad
 * Date: 2025/2/14
 * Purpose: Header file for OBB feature extraction
 */

#ifndef PROJ3_OBB_FEATURE_EXTRACTION_H
#define PROJ3_OBB_FEATURE_EXTRACTION_H
#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

// Main function to compute OBB and draw OBB
int computeRegionFeatures(cv::Mat& regionMap, int regionID, cv::Mat& image, cv::Mat& dst, std::vector<float>& features);
#endif //PROJ3_OBB_FEATURE_EXTRACTION_H
