/*
 * Authors: Yuyang Tian and Arun Mekkad
 * Date: 2025/2/14
 * Purpose: Header file for image processing functions
 */
#ifndef PROJ3_IMAGE_PROCESS_H
#define PROJ3_IMAGE_PROCESS_H

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

// Convert BGR image to YCbCr
void bgr_to_hsv(const Mat& src, Mat& dst);

// threshold image to separate foreground and background
void threshold(const Mat& src, Mat& dst);

// Clean up your thresholded image with morphological filtering
void applyMorphologicalFiltering(const Mat& src, Mat& dst);

// Two-pass segmentation the image into regions ignoring area smaller than minRegionSize, with 4-connectivity
int twoPassSegmentation4conn(const Mat& binaryImage, Mat& regionMap, int minRegionSize = 50);

// Two-pass segmentation the image into regions ignoring area smaller than minRegionSize, with 8-connectivity
int twoPassSegmentation8conn(const Mat& binaryImage, Mat& regionMap, int minRegionSize = 50);

// Helper function to visualize the segmentation result
int colorizeRegions(const cv::Mat& labelMap, Mat& colorImage);

#endif //PROJ3_IMAGE_PROCESS_H
