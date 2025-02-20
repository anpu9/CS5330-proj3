#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <limits>
#include <cmath>
#include "../db/db_manager.h"
#include "../include/evaluate.h"

using namespace std;
using namespace cv;

    // Compute weighted SSD between two feature vectors using provided standard deviations
    float calculateWeightedSSD(const vector<float>& v1, const vector<float>& v2, const vector<float>& stdevs) {
        float distance = 0.0f;
        for (size_t i = 0; i < v1.size(); ++i) {
            float diff = v1[i] - v2[i];
            float d = (stdevs[i] < 1e-5f) ? 1e-5f : stdevs[i];
            float scaled = diff / d;
            distance += scaled * scaled;
        }
        return sqrt(distance);
    }

    // Classify a test sample using training data and computed standard deviations
    string classifyTestSample(const vector<float>& sample,
                              const vector<pair<string, vector<float>>>& trainData,
                              const vector<float>& stdevs,
                              float threshold = 5.0f) {
        string closestLabel = "Unknown";
        float minDistance = numeric_limits<float>::max();
        for (const auto &entry : trainData) {
            float distance = calculateWeightedSSD(sample, entry.second, stdevs);
            if (distance < minDistance) {
                minDistance = distance;
                closestLabel = entry.first;
            }
        }
        return (minDistance > threshold) ? "Unknown" : closestLabel;
    }

    // Compute per-feature standard deviations over the training data
    vector<float> computeStdevs(const vector<pair<string, vector<float>>>& data, size_t featureSize) {
        vector<float> means(featureSize, 0.0f), stdevs(featureSize, 0.0f);
        for (const auto &entry : data) {
            for (size_t i = 0; i < featureSize; ++i)
                means[i] += entry.second[i];
        }
        for (size_t i = 0; i < featureSize; ++i)
            means[i] /= data.size();
        for (const auto &entry : data) {
            for (size_t i = 0; i < featureSize; ++i) {
                float diff = entry.second[i] - means[i];
                stdevs[i] += diff * diff;
            }
        }
        for (size_t i = 0; i < featureSize; ++i) {
            stdevs[i] = sqrt(stdevs[i] / data.size());
            if (stdevs[i] < 1e-5f)
                stdevs[i] = 1e-5f;
        }
        return stdevs;
    }

    // Evaluate confusion matrix: group data by label, select 3 random samples per label for test, classify test samples, print matrix
    void evaluateConfusionMatrix() {
        DBManager db;
        vector<pair<string, vector<float>>> dbFeatures;
        db.loadFeatureVectors(dbFeatures);
        if (dbFeatures.empty()) {
            cerr << "No feature data loaded from the database." << endl;
            return;
        }
        // Fixed label mapping
        unordered_map<string, int> labelIndexMapping = {
            {"spatula", 0},
            {"hair tie", 1},
            {"glass", 2},
            {"tea bag", 3},
            {"socks", 4}
        };
        // Group samples by label
        unordered_map<string, vector<pair<string, vector<float>>>> groupedData;
        for (const auto &sample : dbFeatures) {
            groupedData[sample.first].push_back(sample);
        }
        vector<pair<string, vector<float>>> trainData;
        vector<pair<string, vector<float>>> testData;
        random_device rd;
        mt19937 g(rd());
        // For each label, randomly select 3 for test and use the rest for training
        for (auto &group : groupedData) {
            auto &vec = group.second;
            if (vec.empty())
                continue;
            shuffle(vec.begin(), vec.end(), g);
            size_t testCount = (vec.size() >= 3) ? 3 : vec.size();  // select 3 if possible
            testData.insert(testData.end(), vec.begin(), vec.begin() + testCount);
            if (vec.size() > testCount) {
                trainData.insert(trainData.end(), vec.begin() + testCount, vec.end());
            }
        }
        if (trainData.empty()) {
            cerr << "Not enough training data." << endl;
            return;
        }
        // Compute standard deviations from training data
        size_t featureSize = trainData[0].second.size();
        vector<float> stdevs = computeStdevs(trainData, featureSize);
        // Set up a 5x5 confusion matrix (rows=true, cols=predicted)
        vector<vector<int>> confusionMatrix(5, vector<int>(5, 0));
        // For each test sample, classify and update confusion matrix
        for (const auto &sample : testData) {
            string trueLabel = sample.first;
            string predicted = classifyTestSample(sample.second, trainData, stdevs);
            auto itTrue = labelIndexMapping.find(trueLabel);
            auto itPred = labelIndexMapping.find(predicted);
            if (itTrue != labelIndexMapping.end() && itPred != labelIndexMapping.end()) {
                confusionMatrix[itTrue->second][itPred->second]++;
            }
        }
        // Print the confusion matrix
        cout << "Confusion Matrix (True vs. Predicted):\n";
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                cout << confusionMatrix[i][j] << "\t";
            }
            cout << "\n";
        }
    }

