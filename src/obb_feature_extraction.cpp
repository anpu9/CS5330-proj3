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
cv::Point2f computeCentroid(Moments& m) {
    return cv::Point2f(m.m10 / m.m00, m.m01 / m.m00);
}

// Compute the least central moment axis (orientation angle)
double computeLeastCentralMomentAxis(cv::Moments& m) {
    double mu20 = m.mu20 / m.m00;
    double mu02 = m.mu02 / m.m00;
    double mu11 = m.mu11 / m.m00;
    return 0.5 * atan2(2 * mu11, mu20 - mu02);  // Orientation in radians
}

// Compute the Oriented Bounding Box (OBB) using PCA
int computeOrientedBoundingBox(cv::Mat& regionMap, int regionID, cv::RotatedRect& obb) {
    vector<Point2f> regionPixels;
    for (int i = 0; i < regionMap.rows; i++) {
        for (int j = 0; j < regionMap.cols; j++) {
            if (regionMap.at<int>(i, j) == regionID) {
                regionPixels.push_back(Point2f(j, i));
            }
        }
    }
    if (regionPixels.empty()) {
        return -1;  // No region found for the given region ID
    }
    obb = minAreaRect(regionPixels);
    return 0;
}
// Draw box, axis, centroid on original image
void drawResults(cv::Mat& image, cv::Point2f centroid, double theta, cv::RotatedRect& obb) {
    if (image.type() == CV_8UC1) {
        cv::cvtColor(image, image, cv::COLOR_GRAY2BGR); // Convert grayscale to 3-channel BGR
    }
    // Draw Centroid
    cv::circle(image, centroid, 4, cv::Scalar(255, 0, 0), -1);

    // Draw Least Central Moment Axis (Red Line)
    cv::Point2f axisVector(100 * cos(theta), 100 * sin(theta));
//    cv::line(image, centroid - axisVector, centroid + axisVector, cv::Scalar(0, 0, 255), 2);
    cv::arrowedLine(image, centroid - axisVector, centroid + axisVector, cv::Scalar(0, 0, 255), 2, cv::LINE_AA, 0, 0.1);  // Arrow at end

    // Draw Oriented Bounding Box (Green Box)
    cv::Point2f boxPoints[4];
    obb.points(boxPoints);
    for (int i = 0; i < 4; i++) {
        cv::line(image, boxPoints[i], boxPoints[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
    }
}

// Compute Hu Moments (Translation, Scale, and Rotation Invariant Features)
void computeHuMoments(const Moments& m, vector<double>& huMoments) {
    double hu[7];
    cv::HuMoments(m, hu);
    huMoments.assign(hu, hu + 7);

    // Log-scale transformation for numerical stability
    for (double &moment : huMoments) {
        moment = -1 * copysign(1.0, moment) * log10(abs(moment) + 1e-10);
    }
}
// Compute Aspect Ratio of the OBB, it measures the elongation of the region
double computeAspectRatio(const RotatedRect& obb) {
    return obb.size.width / obb.size.height;
}

double computePerimeterToArea(const Mat& binaryMask, const vector<vector<Point>>& contours){
    double perimeter = arcLength(contours[0], true);
    double area = contourArea(contours[0]);

    return (area > 0) ? (perimeter / area) : -1;  // Avoid division by zero
}

double computePercentFilled(const vector<vector<Point>>& contours, const RotatedRect& obb) {
    double regionArea = contourArea(contours[0]);
    double obbArea = obb.size.area();

    return regionArea / obbArea;
}

int computeRegionShapeFeatures(const Mat& binaryMask, const RotatedRect& obb, int regionId) {
    vector<vector<Point>> contours;
    findContours(binaryMask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (contours.empty()) {
        cerr << "ERROR: contour is empty!" << endl;
        return -1;
    }
    double aspectRatio = computeAspectRatio(obb);
    double perimeterToArea = computePerimeterToArea(binaryMask, contours);
    double percentFilled = computePercentFilled(contours, obb);

    // Print Results
    cout << "Region " << regionId << " Shape Features: " << endl;
    cout << "  - Aspect Ratio: " << aspectRatio << endl;
    cout << "  - Perimeter-to-Area Ratio: " << perimeterToArea << endl;
    cout << "  - % of filled: " << percentFilled << endl;
    return 0;
}
// Main function to compute OBB and draw OBB
int computeRegionFeatures(cv::Mat& regionMap, int regionID, cv::Mat& image, cv::Mat& dst) {
    // Step 1: Get Binary Mask
    cv::Mat binaryMask;
    if (getBinaryMask(regionMap, regionID, binaryMask) != 0) {
        std::cerr << "Error: Unable to extract binary mask for region ID: " << regionID << std::endl;
        return -1;  // Failed to get binary mask
    }

    // Step 2: Compute Moments
    cv::Moments m = cv::moments(binaryMask, true);
    if (m.m00 == 0) {
        std::cerr << "No region found for ID: " << regionID << std::endl;
        return -1;  // Failed due to no region found
    }

    // Step 3: Compute Features
    cv::Point2f centroid = computeCentroid(m);
    double theta = computeLeastCentralMomentAxis(m);
    cv::RotatedRect obb;
    if (computeOrientedBoundingBox(regionMap, regionID, obb) != 0) {
        std::cerr << "Error: Unable to compute Oriented Bounding Box for region ID: " << regionID << std::endl;
        return -1;  // Failed to compute OBB
    }
    // Step 4: Compute Hu Moments (Rotation, Scale, and Translation Invariant)
    vector<double> huMoments;
    computeHuMoments(m, huMoments);

    // Step 5: Draw Features
    dst = image.clone();
    drawResults(dst, centroid, theta, obb);
    imshow("Obb over image", dst);
    cv::waitKey(0);  // Wait indefinitely for a key press

    // Step 6: Print Feature Information
    cout << "Region " << regionID << " Features:\n";
    cout << "  - Centroid: (" << centroid.x << ", " << centroid.y << ")\n";
    cout << "  - Least Central Moment Axis Angle: " << theta * 180.0 / CV_PI << " degrees\n";
    cout << "  - OBB Center: (" << obb.center.x << ", " << obb.center.y << ")\n";
    cout << "  - OBB Size: (" << obb.size.width << " x " << obb.size.height << ")\n";
    cout << "  - OBB Angle: " << obb.angle << " degrees\n";

    // Compute and print shape feature
    computeRegionShapeFeatures(binaryMask, obb, regionID);

    // Print Hu Moments (Invariant Features)
    cout << "Region " << regionID << " Hu Moments (Log Transformed):\n";
    for (size_t i = 0; i < huMoments.size(); i++) {
        cout << "    - Hu[" << i + 1 << "]: " << huMoments[i] << "\n";
    }

    return 0;  // Success
}
