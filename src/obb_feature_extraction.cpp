/*
 * Authors: ${AUTHORS}
 * Date: 2025/2/14
 * Purpose: ${PURPOSE}
 */
#include <opencv2/opencv.hpp>
#include <iostream>
#include "../include/obb_feature_extraction.h"

using namespace cv;
using namespace std;

// Extract binary mask of a specific region ID
int getBinaryMask(Mat& regionMap, int regionID, Mat& binaryMask) {
    binaryMask = (regionMap == regionID);
    if (binaryMask.empty()) {
        return -1;  // Failed to create binary mask
    }
    return 0;  // Success
}

// Compute the centroid of the region
Point2f computeCentroid(Moments& m) {
    return Point2f(m.m10 / m.m00, m.m01 / m.m00);
}

// Compute the least central moment axis (orientation angle)
double computeLeastCentralMomentAxis(Moments& m) {
    double mu20 = m.mu20 / m.m00;
    double mu02 = m.mu02 / m.m00;
    double mu11 = m.mu11 / m.m00;
    return 0.5 * atan2(2 * mu11, mu20 - mu02);  // Orientation in radians
}

// Compute the Oriented Bounding Box (OBB) using PCA

int computeOrientedBoundingBox(Mat& binaryMask, Mat& regionMap, int regionID, RotatedRect& obb) {
    Mat mask;

    vector<Point> regionPixels;
    findNonZero(binaryMask, regionPixels);  // Much faster than manual iteration

    if (regionPixels.empty()) {
        return -1;  // No region found
    }

    obb = minAreaRect(regionPixels);
    return 0;
}

// Draw box, axis, centroid on original image
void drawResults(Mat& image, Point2f centroid, double theta, RotatedRect& obb) {
    if (image.type() == CV_8UC1) {
        cvtColor(image, image, COLOR_GRAY2BGR); // Convert grayscale to 3-channel BGR
    }
    // Draw Centroid
    circle(image, centroid, 4, Scalar(255, 0, 0), -1);

    // Draw Least Central Moment Axis (Red Line)
    Point2f axisVector(100 * cos(theta), 100 * sin(theta));
//    line(image, centroid - axisVector, centroid + axisVector, Scalar(0, 0, 255), 2);
    arrowedLine(image, centroid - axisVector, centroid + axisVector, Scalar(0, 0, 255), 2, LINE_AA, 0, 0.1);  // Arrow at end

    // Draw Oriented Bounding Box (Green Box)
    Point2f boxPoints[4];
    obb.points(boxPoints);
    for (int i = 0; i < 4; i++) {
        line(image, boxPoints[i], boxPoints[(i + 1) % 4], Scalar(0, 255, 0), 2);
    }
}

// Compute Hu Moments (Translation, Scale, and Rotation Invariant Features)
void computeHuMoments(const Moments& m, vector<double>& huMoments) {
    double hu[7];
    HuMoments(m, hu);
    huMoments.assign(hu, hu + 7);

    // Log-scale transformation for numerical stability
    for (double &moment : huMoments) {
        moment = -1 * copysign(1.0, moment) * log10(abs(moment) + 1e-10);
    }
}
// Compute Aspect Ratio of the OBB, it measures the elongation of the region
float computeAspectRatio(const RotatedRect& obb) {
    return obb.size.width / obb.size.height;
}

float computePerimeterToArea(const Mat& binaryMask, const vector<vector<Point>>& contours){
    double perimeter = arcLength(contours[0], true);
    double area = contourArea(contours[0]);

    return (area > 0) ? (perimeter / area) : -1;  // Avoid division by zero
}

float computePercentFilled(const vector<vector<Point>>& contours, const RotatedRect& obb) {
    double regionArea = contourArea(contours[0]);
    double obbArea = obb.size.area();

    return regionArea / obbArea;
}

int computeRegionShapeFeatures(const Mat& binaryMask, const RotatedRect& obb, int regionId, vector<float>& shapeFeatures) {
    vector<vector<Point>> contours;
    findContours(binaryMask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (contours.empty()) {
        cerr << "ERROR: contour is empty!" << endl;
        return -1;
    }
    float aspectRatio = computeAspectRatio(obb);
    float perimeterToArea = computePerimeterToArea(binaryMask, contours);
    float percentFilled = computePercentFilled(contours, obb);

    shapeFeatures.push_back(aspectRatio);
    shapeFeatures.push_back(perimeterToArea);
    shapeFeatures.push_back(percentFilled);

    return 0;
}
// Main function to compute OBB and draw OBB
int computeRegionFeatures(Mat& regionMap, int regionID, Mat& image, Mat& dst, vector<float>& features) {
    // Step 1: Get Binary Mask
    Mat binaryMask;
    if (getBinaryMask(regionMap, regionID, binaryMask) != 0) {
        std::cerr << "Error: Unable to extract binary mask for region ID: " << regionID << std::endl;
        return -1;  // Failed to get binary mask
    }

    cv::Moments m = cv::moments(binaryMask, true);
    if (m.m00 == 0) return -1;

    cv::Point2f centroid(m.m10 / m.m00, m.m01 / m.m00);
    double theta = computeLeastCentralMomentAxis(m);

    RotatedRect obb;
    if (computeOrientedBoundingBox(binaryMask, regionMap, regionID, obb) != 0) {
        std::cerr << "Error: Unable to compute Oriented Bounding Box for region ID: " << regionID << std::endl;
        return -1;  // Failed to compute OBB
    }

    // Draw Features
    dst = image.clone();
    drawResults(dst, centroid, theta, obb);
    imshow("Obb over image", dst);
    waitKey(0);  // Wait indefinitely for a key press

    vector<double> huMoments;
    computeHuMoments(m, huMoments);

    vector<float> shapeFeatures;
    computeRegionShapeFeatures(binaryMask, obb, regionID, shapeFeatures);

    // Combine feature vectors
    features.clear();
    features.insert(features.end(), huMoments.begin(), huMoments.end());
    features.insert(features.end(), shapeFeatures.begin(), shapeFeatures.end());

    return 0;  // Success
}
