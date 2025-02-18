/*
 * Authors: ${AUTHORS}
 * Date: 2025/2/14
 * Purpose: ${PURPOSE}
 */

#include "../include/image_process.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <random>
#include <algorithm>

// Convert BGR image to HSV
void bgr_to_hsv(const Mat& src, Mat& dst) {
    // Check if the input image is valid
    if (src.empty() || src.type() != CV_8UC3) {
        cerr << "Error: Invalid input image in bgr_to_hsv" << endl;
        return;
    }

    // Create the destination image
    dst.create(src.size(), CV_8UC3);

    // Image properties
    int rows = src.rows;
    int cols = src.cols;

    // Pointers to image data
    const uchar* src_data = src.data;
    uchar* dst_data = dst.data;

    // Steps through the rows and columns
    size_t src_step = src.step;
    size_t dst_step = dst.step;

    // Cycle through the images and assign the appropriate values
    for (int y = 0; y < rows; ++y) {
        const uchar* src_row = src_data + y * src_step;
        uchar* dst_row = dst_data + y * dst_step;

        for (int x = 0; x < cols; ++x) {
            // Scale BGR values from 0 to 1
            float B = src_row[x * 3 + 0] / 255.0f;
            float G = src_row[x * 3 + 1] / 255.0f;
            float R = src_row[x * 3 + 2] / 255.0f;

            // Value
            float V = max({R, G, B});

            // Saturation
            float S = 0.0f;

            if (V > 0) {
                float minVal = min({R, G, B});
                S = (V - minVal) / V;
            }

            // Hue
            float H = 0.0f;

            if (S > 0) {
                float maxVal = V;
                float minVal = min({R, G, B});
                float delta = maxVal - minVal;

                if (R == maxVal) {
                    H = 60.0f * (G - B) / delta;
                } else if (G == maxVal) {
                    H = 60.0f * (2 + (B - R) / delta);
                } else { // B == maxVal
                    H = 60.0f * (4 + (R - G) / delta);
                }

                if (H < 0) {
                    H += 360.0f;
                }
            }

            // Set HSV values
            dst_row[x * 3 + 0] = static_cast<uchar>((H / 2.0f));   // OpenCV H is in [0, 180]
            dst_row[x * 3 + 1] = static_cast<uchar>(S * 255);   // S is in [0, 255]
            dst_row[x * 3 + 2] = static_cast<uchar>(V * 255);   // V is in [0, 255]
        }
    }
}

// Function to calculate the mean of a vector
float calculateMean(const vector<float>& data) {
    if (data.empty()) return 0;
    float sum = 0;
    for (float val : data) {
        sum += val;
    }
    return sum / data.size();
}

// Function to perform ISODATA (K-Means with K=2) thresholding
void threshold(const Mat& src, Mat& dst) {
    // Check if the input image is valid
    if (src.empty() || src.type() != CV_8UC1) {
        cerr << "Error: Invalid input image in threshold" << endl;
        return;
    }

    // Create the destination image (CV_8UC1)
    dst.create(src.size(), CV_8UC1);

    // 1. Initialization
    float thresholdValue = 128; // Initial threshold
    float oldThreshold;
    bool converged = false;
    int iteration = 0;
    int maxIterations = 50;

    while (!converged && iteration < maxIterations) {
        iteration++;
        oldThreshold = thresholdValue;

        // 2. Segmentation
        vector<float> backgroundPixels, objectPixels;

        // Use pointers for faster pixel access
        uchar* src_data = src.data;
        int src_step = src.step;

        for (int y = 0; y < src.rows; y++) {
            uchar* row = src_data + y * src_step; // Pointer to the beginning of the row
            for (int x = 0; x < src.cols; x++) {
                float pixelValue = row[x]; // Access pixel directly using pointer
                if (pixelValue < thresholdValue) {
                    backgroundPixels.push_back(pixelValue);
                } else {
                    objectPixels.push_back(pixelValue);
                }
            }
        }

        // 3. Calculate new means
        float backgroundMean = calculateMean(backgroundPixels);
        float objectMean = calculateMean(objectPixels);

        // 4. Update threshold
        thresholdValue = (backgroundMean + objectMean) / 2.0f;

        // 5. Check for convergence
        if (abs(thresholdValue - oldThreshold) < 0.5f) {
            converged = true;
        }
    }

    // Apply final threshold
    uchar* dst_data = dst.data;
    int dst_step = dst.step;

    for (int y = 0; y < src.rows; y++) {
        const uchar* src_row = src.ptr<uchar>(y);
        uchar* dst_row = dst_data + y * dst_step;

        for (int x = 0; x < src.cols; x++) {
           // Apply threshold using pointers for direct access
            dst_row[x] = (src_row[x] > thresholdValue) ? 255 : 0;
        }
    }
}

// Clean up your thresholded image with morphological filtering
void applyMorphologicalFiltering(const Mat& src, Mat& dst) {
    // 1. Validation of image data (the source file)
    if (src.empty() || src.type() != CV_8UC1) {
        cerr << "Error: Source invalid" << endl;
        return;
    }

    // 2. Destination Creation
    dst = src.clone();

    // 3. Assign structure and structure value. Based on the comments, the size is set to "1", this number was arrived at after multiple tests.
    int morph_size = 1;
    int elementHeight = 2 * morph_size + 1;
    int elementWidth = 2 * morph_size + 1;
    Mat element = getStructuringElement(
        MORPH_RECT, Size(elementWidth, elementHeight), Point(morph_size, morph_size));

    // 4. Loop through the array and generate the new morph matrix
    for (int y = morph_size; y < src.rows - morph_size; ++y) {
        for (int x = morph_size; x < src.cols - morph_size; ++x) {
            //Apply structuring element
            int local_min = 255;
            for (int i = -morph_size; i <= morph_size; ++i) {
                for (int j = -morph_size; j <= morph_size; ++j) {
                    local_min = min(local_min, (int)src.at<uchar>(y + i, x + j));
                }
            }

            //Erode and create image
            dst.at<uchar>(y, x) = local_min;
        }
    }
}


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
int two_pass_segmentation_4conn(const Mat& binaryImage, Mat& regionMap, int minRegionSize) {
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
 * @return -1 failure, 0 success
 */
int two_pass_segmentation_8conn(const Mat& binaryImage, Mat& regionMap, int minRegionSize) {
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
    return 0;
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