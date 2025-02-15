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
 * @brief Two-pass segmentation for connected components. 4-connectivity
 * @param src Input binary image (CV_8U).
 * @param dst Labeled region map (CV_32S).
 * @return -1 failure, 0 success
 */
int two_pass_segmentation(const Mat& binaryImage, Mat& regionMap) {
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
            if (j > 0 && regionMap.at<int>(i,j-1) > 0) {
                neighbors.push_back(regionMap.at<int>(i,j-1));
            }
            if (j > 0 && regionMap.at<int>(i,j) > 0) {
                neighbors.push_back(regionMap.at<int>(i-1,j));
            }

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
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (binaryImage.at<uchar>(i, j) == 0) continue; // Ignore the background
            regionMap.at<uchar>(i, j) = find(parent, regionMap.at<uchar>(i, j));
        }
    }
    return 0;
}
// Helper function to visualize the segmentation result
int colorizeRegions(const cv::Mat& labelMap, Mat& colorImage) {
    if (labelMap.empty() || labelMap.type() != CV_32S) {
        cerr << "ERROR: invalid input for regionMap visualization." << endl;
        return -1;  // Invalid input
    }
    unordered_map<int, cv::Vec3b> colorMap;
    colorImage.create(labelMap.size(), CV_8UC3);
    colorImage.setTo(cv::Scalar(0, 0, 0));  // Set background to black


    for (int i = 0; i < labelMap.rows; i++) {
        for (int j = 0; j < labelMap.cols; j++) {
            int label = labelMap.at<int>(i, j);
            if (label == 0) continue; // Background remains black

            if (colorMap.find(label) == colorMap.end()) { // assign a new color
                colorMap[label] = cv::Vec3b(rand() % 256, rand() % 256, rand() % 256);
            }
            colorImage.at<cv::Vec3b>(i, j) = colorMap[label];
        }
    }
    return 0;
}