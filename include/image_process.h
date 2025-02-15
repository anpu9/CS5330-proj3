/*
 * Authors: Yuyang Tian and Arun Merkkad
 * Date: 2025/2/14
 * Purpose: A data pipeline to process image for object detection
 */
#ifndef PROJ3_IMAGE_PROCESS_H
#define PROJ3_IMAGE_PROCESS_H
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;
// threshold image to separate foreground and background
int threshold(const Mat& src, Mat& dst);
// Clean up your thresholded image with morphological filtering
int morphologicalFilter(const Mat& src, Mat& dst);
// Segment the image into regions
int two_pass_segmentation(const Mat& binaryImage, Mat& regionMap)

#endif //PROJ3_IMAGE_PROCESS_H
