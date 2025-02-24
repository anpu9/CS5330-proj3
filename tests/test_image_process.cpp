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
cv::Mat loadTestImage(const string &filename) {
    cv::Mat image = cv::imread("../test-imgs/" + filename, cv::IMREAD_GRAYSCALE);
    EXPECT_FALSE(image.empty()) << "Failed to load test image: " << filename;
    return image;
}


class ImageProcessingTest : public ::testing::Test {
protected:
    cv::Mat binaryImage;
    cv::Mat regionMap;
    vector<float> features;
    DBManager db;
    int regionId = 2;
    cv::Mat dst;
    // TODO: Any prerequisite could be exectued here
    void SetUp() override {
        binaryImage = loadTestImage("example001.png");
        // TO test DB write and read, the features are computed in settup
        twoPassSegmentation8conn(binaryImage, regionMap);
        computeRegionFeatures(regionMap, regionId, binaryImage, dst, features);
        ASSERT_EQ(regionMap.type(), CV_32S);
        ASSERT_GT(cv::countNonZero(regionMap), 0); // Ensure valid segmentation
//        printLabel();
        printFeatures();
        db = DBManager();
    }
    void printLabel() {
        set<int> uniqueLabels;
        for (int i = 0; i < regionMap.rows; i++) {
            for (int j = 0; j < regionMap.cols; j++) {
                uniqueLabels.insert(regionMap.at<int>(i, j));
            }
        }
        cout << "Unique region IDs in regionMap: ";
        for (int id : uniqueLabels) cout << id << " ";
        cout << endl;
    }
    void printFeatures() {
        cout << "Features in vector: ";
        for (float num : features) {
            cout << num << " ";
        }
        cout << endl;
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
    db.deleteAll(); // clear previous records
    ASSERT_FALSE(features.empty()) << "Feature vector is empty before DB write!";
    string label = "test";
    int result = db.writeFeatureVector(label, features);
    EXPECT_EQ(result, 0) << "DB Write failed to execute properly.";
}

TEST_F(ImageProcessingTest, DBReadTest) {
    vector<pair<string, vector<float>>> data;
    int result = db.loadFeatureVectors(data);
    // Validate the function execution
    cout << "The size is " << result << endl;
    EXPECT_GT(result, 0) << "DB Write failed to execute properly.";
}