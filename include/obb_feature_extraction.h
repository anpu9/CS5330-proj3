/*
 * Authors: ${AUTHORS}
 * Date: 2025/2/14
 * Purpose: computing Rotation, translation, scale invariant features of a bounding box.
 */

#ifndef PROJ3_OBB_FEATURE_EXTRACTION_H
#define PROJ3_OBB_FEATURE_EXTRACTION_H
#include <opencv2/opencv.hpp>
#include <iostream>

//int computeOrientedBoundingBox(cv::Mat& regionMap, int regionID, cv::RotatedRect& obb);
// Draw box, axis, centroid on original image
//void drawResults(cv::Mat& image, cv::Point2f centroid, double theta, cv::RotatedRect& obb);
// Main function to compute OBB and draw OBB
int computeRegionFeatures(cv::Mat& regionMap, int regionID, cv::Mat& image, cv::Mat& dst, std::vector<float>& features);
#endif //PROJ3_OBB_FEATURE_EXTRACTION_H
