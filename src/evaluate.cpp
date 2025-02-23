/*
 * Authors: Yuyang Tian and Arun Mekkad
 * Date: 2025/2/16
 * Purpose: Evaluation of confusion matrices using various classifiers
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include "../db/db_manager.h"
#include <random>
#include "../include/evaluate.h"
#include "../include/classifier.h"  // Contains classifyByNN, and classifyByDecisionTree

using namespace std;
using namespace cv;

    // Evaluate confusion matrix: group data by label, select 3 random samples per label for test,
    // classify each test sample using the specified classifier (0 = NN, 1 = Decision Tree),
    // and print the confusion matrix.
    void evaluateConfusionMatrix(int classifierType) {
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
        default_random_engine g(rd());
        // For each label, randomly select 3 samples for testing; the remaining are used for training
        for (auto &group : groupedData) {
            auto &vec = group.second;
            if (vec.empty())
                continue;
            shuffle(vec.begin(), vec.end(), g);
            size_t testCount = (vec.size() >= 3) ? 3 : vec.size();
            testData.insert(testData.end(), vec.begin(), vec.begin() + testCount);
            if (vec.size() > testCount) {
                trainData.insert(trainData.end(), vec.begin() + testCount, vec.end());
            }
        }
        if (trainData.empty()) {
            cerr << "Not enough training data." << endl;
            return;
        }
        // Set up a 5x5 confusion matrix (rows: true labels, cols: predicted labels)
        vector<vector<int>> confusionMatrix(5, vector<int>(5, 0));
        // For each test sample, classify and update the corresponding cell in the matrix
        for (const auto &sample : testData) {
            string trueLabel = sample.first;
            string predicted;
            if (classifierType == 0) {
                predicted = classifyByNN(trainData, sample.second);
            } else if (classifierType == 1) {
                predicted = classifyByDecisionTree(sample.second);
            } else {
                predicted = "Unknown";
            }
            auto itTrue = labelIndexMapping.find(trueLabel);
            auto itPred = labelIndexMapping.find(predicted);
            if (itTrue != labelIndexMapping.end() && itPred != labelIndexMapping.end()) {
                confusionMatrix[itTrue->second][itPred->second]++;
            }
        }
        // Print the confusion matrix
        cout << "Confusion Matrix (True vs. Predicted) using "
             << ((classifierType == 0) ? "NN" : "Decision Tree") << " classifier:\n";
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                cout << confusionMatrix[i][j] << "\t";
            }
            cout << "\n";
        }
    }

