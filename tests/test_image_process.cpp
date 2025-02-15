/*
 * Authors: Yuyang Tian
 * Date: 2025/2/15
 * Purpose: Testing the image processing data pipeline
 */

#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include "../include/image_process.h"

using namespace cv;

// Helper function to load a test image
cv::Mat loadTestImage(const std::string &filename) {
    cv::Mat image = cv::imread("../test-imgs/" + filename, cv::IMREAD_GRAYSCALE);
    EXPECT_FALSE(image.empty()) << "Failed to load test image: " << filename;
    return image;
}

// Test threshold function
//TEST(ImageProcessTest, ThresholdTest) {
//    cv::Mat input = loadTestImage("example001.png");
//    cv::Mat output;
//    int result = threshold(input, output);
//
//    EXPECT_EQ(result, 0);
//    EXPECT_EQ(output.type(), CV_8UC1);
//}

// Test morphological filtering
//TEST(ImageProcessTest, MorphologicalFilterTest) {
//    cv::Mat input = loadTestImage("example001.png");
//    cv::Mat output;
//    threshold(input, output); // Ensure binary image before morphological filtering
//    int result = morphologicalFilter(output, output);
//
//    EXPECT_EQ(result, 0);
//    EXPECT_EQ(output.type(), CV_8UC1);
//}

// Test two-pass segmentation
TEST(ImageProcessTest, TwoPassSegmentationTest) {
//    cv::Mat binaryImage = loadTestImage("example001.png");
    std::string imagePath = "../test-imgs/example001.png";
    cv::Mat binaryImage = cv::imread(imagePath, IMREAD_GRAYSCALE);
    cv::Mat regionMap;

    int result_4conn = two_pass_segmentation_4conn(binaryImage, regionMap);
    int result_8conn = two_pass_segmentation_8conn(binaryImage, regionMap);

    EXPECT_EQ(result_4conn, 0);
    EXPECT_EQ(result_8conn, 0);
    EXPECT_EQ(regionMap.type(), CV_32S);

// Ensure that at least one region has been labeled
    EXPECT_GT(cv::countNonZero(regionMap), 0);

    // Colorize the result
    cv::Mat colorizedRegions4conn, colorizedRegions8conn;
    colorizeRegions(regionMap, colorizedRegions4conn);
    colorizeRegions(regionMap, colorizedRegions8conn);

    // Display original and segmented images
    cv::imshow("Binary Image", binaryImage);
    cv::imshow("Labeled Regions - 4 connectivity", colorizedRegions4conn);
    cv::imshow("Labeled Regions - 8 connectivity", colorizedRegions8conn);
    cv::waitKey(0);  // Press any key to close
}

// Main function to run all src
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // Set Google Test filter to run only a specific test case
    ::testing::GTEST_FLAG(filter) = "ImageProcessTest.TwoPassSegmentationTest";

    return RUN_ALL_TESTS();
}
