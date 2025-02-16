/*
 * Authors: ${AUTHORS}
 * Date: 2025/2/14
 * Purpose: ${PURPOSE}
 */

#include "../include/image_process.h"
#include <iostream>
#include <vector>
#include <unordered_map>

// Find function for Union-Find (Path Compression), return the root
int find(vector<int>& parent, int x) {
    if (parent[x] != x) {
        parent[x] = find(parent, parent[x]); // Path compression
    }
    return parent[x];
}

// Union connect label x and y
void connect(vector<int>& parent, int x, int y) {
    int rootX = find(parent, x);
    int rootY = find(parent, y);

    if (rootX != rootY) {
        parent[rootY] = rootX; // Merge components
    }
}
/**
 * @brief Two-pass segmentation for connected components with 4-connectivity.
 *        Filters out small noisy regions based on a threshold.
 * @param binaryImage Input binary image (CV_8U).
 * @param regionMap Labeled region map (CV_32S).
 * @param minRegionSize Minimum size to keep a region.
 * @return -1 failure, 0 success
 */
int twoPassSegmentation4conn(const Mat& binaryImage, Mat& regionMap, int minRegionSize) {
    if (binaryImage.empty()) {
        cerr << "ERROR: two_pass_segmentation - empty image" << endl;
        return -1;
    }
    if (binaryImage.type() != CV_8UC1) { // check if it's a binary image
        cerr << "ERROR: two_pass_segmentation - not Binary image" << endl;
        return -1;
    }
    int rows = binaryImage.rows, cols = binaryImage.cols;
    regionMap = Mat::zeros(rows, cols, CV_32S); // Initialize region map
    vector<int> parent(1,0); // Initialize the parent array, start with label 0
    int nextLabel = 1;

    // First pass: assign provisional labels and record equivalences
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (binaryImage.at<uchar>(i, j) == 0) continue; // Ignore the background

            vector<int> neighbors; // storing neighbors' label

            // Check neighbor in prior row and col
            if (j > 0 && regionMap.at<int>(i,j-1) > 0) { neighbors.push_back(regionMap.at<int>(i,j-1));} // left
            if (j > 0 && regionMap.at<int>(i-1,j) > 0) { neighbors.push_back(regionMap.at<int>(i-1,j));} // top

            if (neighbors.empty()) { // NO neighbor: assign a new label
                regionMap.at<int>(i, j) = nextLabel;
                parent.push_back(nextLabel);
                nextLabel++;
            } else { // assign the min label
                int minLabel = *min_element(neighbors.begin(), neighbors.end());
                regionMap.at<int>(i, j) = minLabel;
                // Compress path: union all neighbors in set
                for (int label : neighbors) {
                    connect(parent, label, minLabel);
                }
            }
        }
    }

    // Second pass: resolve label equivalences
    unordered_map<int, int> labelCount;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (binaryImage.at<uchar>(i, j) == 0) continue; // Ignore the background
            regionMap.at<int>(i, j) = find(parent, regionMap.at<int>(i, j));
            labelCount[regionMap.at<int>(i, j)]++; // Count pixels per region
        }
    }
    // Filter out small regions
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (regionMap.at<int>(i, j) > 0 && labelCount[regionMap.at<int>(i, j)] < minRegionSize) {
                regionMap.at<int>(i, j) = 0; // Remove small noisy regions
            }
        }
    }
    return 0;
}
/**
 * @brief Two-pass segmentation for connected components with 8-connectivity.
 *        Filters out small noisy regions based on a threshold.
 * @param binaryImage Input binary image (CV_8U).
 * @param regionMap Labeled region map (CV_32S).
 * @param minRegionSize Minimum size to keep a region.
 * @return -1 failure, success return number of regions
 */
int twoPassSegmentation8conn(const Mat& binaryImage, Mat& regionMap, int minRegionSize) {
    if (binaryImage.empty()) {
        cerr << "ERROR: two_pass_segmentation - empty image" << endl;
        return -1;
    }
    if (binaryImage.type() != CV_8UC1) { // check if it's a binary image
        cerr << "ERROR: two_pass_segmentation - not Binary image" << endl;
        return -1;
    }
    int rows = binaryImage.rows, cols = binaryImage.cols;
    regionMap = Mat::zeros(rows, cols, CV_32S); // Initialize region map
    vector<int> parent(1,0); // Initialize the parent array, start with label 0
    int nextLabel = 1;

    // First pass: assign provisional labels and record equivalences
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (binaryImage.at<uchar>(i, j) == 0) continue; // Ignore the background

            vector<int> neighbors; // storing neighbors' label

            // Check neighbor in prior row and col
            if (j > 0 && regionMap.at<int>(i,j-1) > 0) { neighbors.push_back(regionMap.at<int>(i,j-1));} // left
            if (j > 0 && regionMap.at<int>(i-1,j) > 0) { neighbors.push_back(regionMap.at<int>(i-1,j));} // top
            if (i > 0 && j > 0 && regionMap.at<int>(i - 1, j - 1) > 0) neighbors.push_back(regionMap.at<int>(i - 1, j - 1)); // Top-left
            if (i > 0 && j < cols - 1 && regionMap.at<int>(i - 1, j + 1) > 0) neighbors.push_back(regionMap.at<int>(i - 1, j + 1)); // Top-right

            if (neighbors.empty()) { // NO neighbor: assign a new label
                regionMap.at<int>(i, j) = nextLabel;
                parent.push_back(nextLabel);
                nextLabel++;
            } else { // assign the min label
                int minLabel = *min_element(neighbors.begin(), neighbors.end());
                regionMap.at<int>(i, j) = minLabel;
                // Compress path: union all neighbors in set
                for (int label : neighbors) {
                    connect(parent, label, minLabel);
                }
            }
        }
    }

    // Second pass: resolve label equivalences
    unordered_map<int, int> labelCount;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (binaryImage.at<uchar>(i, j) == 0) continue; // Ignore the background
            regionMap.at<int>(i, j) = find(parent, regionMap.at<int>(i, j));
            labelCount[regionMap.at<int>(i, j)]++; // Count pixels per region
        }
    }
    // Filter out small regions
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (regionMap.at<int>(i, j) > 0 && labelCount[regionMap.at<int>(i, j)] < minRegionSize) {
                regionMap.at<int>(i, j) = 0; // Remove small noisy regions
            }
        }
    }
    // Reassign labels to be sequential (1,2,3...)
    unordered_map<int, int> relabelMap;
    int newLabel = 1;
    for (const auto& kv : labelCount) {
        if (kv.first > 0 && kv.second >= minRegionSize) {
            relabelMap[kv.first] = newLabel++;
        }
    }

    // Apply new labels
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (regionMap.at<int>(i, j) > 0) {
                regionMap.at<int>(i, j) = relabelMap[regionMap.at<int>(i, j)];
            }
        }
    }

    return newLabel - 1; // Return number of valid regions
}
// Helper function to visualize the segmentation result
int colorizeRegions(const cv::Mat& labelMap, Mat& colorImage) {
    if (labelMap.empty() || labelMap.type() != CV_32S) {
        cerr << "ERROR: invalid input for regionMap visualization." << endl;
        return -1;  // Invalid input
    }
    cv::Mat normalizedLabels;
    double minVal, maxVal;
    cv::minMaxLoc(labelMap, &minVal, &maxVal);
    // Normalize labels to 255
    labelMap.convertTo(normalizedLabels, CV_8U, 255.0 / maxVal); // Normalize labels to 0-255
    normalizedLabels.setTo(0, labelMap == 0); // Ensure background remains 0
    cv::applyColorMap(normalizedLabels, colorImage, cv::COLORMAP_JET); // Apply color mapping

    // Ensure background remains black (0,0,0)
    colorImage.setTo(cv::Vec3b(0, 0, 0), labelMap == 0);
    return 0;
}