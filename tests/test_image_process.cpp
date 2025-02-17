/*
 * Authors: Yuyang Tian
 * Date: 2025/2/15
 * Purpose: Testing the image processing data pipeline
 */

#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include "../include/image_process.h"
#include "../include/obb_feature_extraction.h"
#include "../db/db_manager.h"

using namespace cv;

// Helper function to load a test image
cv::Mat loadTestImage(const std::string &filename) {
    cv::Mat image = cv::imread("../test-imgs/" + filename, cv::IMREAD_GRAYSCALE);
    EXPECT_FALSE(image.empty()) << "Failed to load test image: " << filename;
    return image;
}


class ImageProcessingTest : public ::testing::Test {
protected:
    cv::Mat binaryImage;
    cv::Mat regionMap;
    vector<float> features;
    // TODO: Any prerequisite could be exectued here
    void SetUp() override {
        binaryImage = loadTestImage("example001.png");
        twoPassSegmentation8conn(binaryImage, regionMap);
        ASSERT_EQ(regionMap.type(), CV_32S);
        ASSERT_GT(cv::countNonZero(regionMap), 0); // Ensure valid segmentation
        printLabel();
    }
    void printLabel() {
        std::set<int> uniqueLabels;
        for (int i = 0; i < regionMap.rows; i++) {
            for (int j = 0; j < regionMap.cols; j++) {
                uniqueLabels.insert(regionMap.at<int>(i, j));
            }
        }
        std::cout << "Unique region IDs in regionMap: ";
        for (int id : uniqueLabels) std::cout << id << " ";
        std::cout << std::endl;
    }
};

// Test two-pass segmentation inside the fixture
TEST_F(ImageProcessingTest, TwoPassSegmentationTest) {
    cv::Mat colorizedRegions;
    colorizeRegions(regionMap, colorizedRegions);

    cv::imshow("Segmented Regions", colorizedRegions);
    cv::waitKey(0);
}

// Test OBB using the fixture data
// NOTE: Since the segmentation is executed before the OBBTest, we can safely use `regionMap`
TEST_F(ImageProcessingTest, OBBTest) {
    int regionId = 2;
    cv::Mat dst;
    vector<float> features;
    int result = computeRegionFeatures(regionMap, regionId, binaryImage, dst, features);

// Validate the function execution
    EXPECT_EQ(result, 0) << "Function failed to execute properly.";

// Validate the output
    EXPECT_FALSE(dst.empty()) << "Output image is empty.";
    EXPECT_FALSE(dst.empty()) << "Feature vector is empty.";
    EXPECT_EQ(dst.size(), binaryImage.size()) << "Output dimensions do not match input.";
}

TEST_F(ImageProcessingTest, DBWriteTest) {
    int regionId = 2;
    cv::Mat dst;
    vector<float> features;
    int result = computeRegionFeatures(regionMap, regionId, binaryImage, dst, features);
//     Validate the function execution
    EXPECT_EQ(result, 0) << "Function failed to execute properly.";

// Validate the output
    EXPECT_FALSE(dst.empty()) << "Output image is empty.";
    EXPECT_FALSE(dst.empty()) << "Feature vector is empty.";
    EXPECT_EQ(dst.size(), binaryImage.size()) << "Output dimensions do not match input.";
    // Write to DB

}