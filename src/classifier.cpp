/*
 * Authors: Yuyang Tian and Arun Mekkad
 * Date: 2025/2/16
 * Purpose: Classify object by different defined classifiers
 */

#include "../include/classifier.h"
#include <vector>
#include <iostream>
#include <cmath>

using namespace std;

/**
 * Decision tree generated by py sklearn.tree
 * @param the current object features vector
 * @return the closest label
 */
string classifyByDecisionTree(const vector<float>& features) {
    if (features[0] <= 0.403538){
        if (features[1] <= 1.408385) {
            if (features[4] <= 4.194975) {
                return "spatula";
            } else {
                return "pen";
            }
        } else {
            return "hair tie";
        }
    } else {
        if (features[2] <= 3.092119) {
            if (features[7] <= 2.607938) {
                return "socks";
            } else {
                return "glass";
            }
        } else {
            if (features[1] <= 2.532149) {
                return "glass";
            } else {
                return "tea bag";
            }
        }
    }
}
// Compute standard deviation for each feature
vector<float> computeFeatureStdDevs(const vector<pair<string, vector<float>>>& dbFeatures) {
    if (dbFeatures.empty()) return {};

    size_t featureSize = dbFeatures[0].second.size();
    vector<float> stdevs(featureSize, 0.0f);

    for (size_t i = 0; i < featureSize; ++i) {
        float sum = 0.0f, sum_sq = 0.0f;
        for (const auto& entry : dbFeatures) {
            sum += entry.second[i];
            sum_sq += entry.second[i] * entry.second[i];
        }
        float mean = sum / dbFeatures.size();
        float variance = (sum_sq / dbFeatures.size()) - (mean * mean);
        stdevs[i] = sqrt(variance);
    }
    return stdevs;
}

// Compute scaled Euclidean distance
float calculateScaledEuclideanDistance(const vector<float>& v1, const vector<float>& v2, const vector<float>& stdevs) {
    float distance = 0.0f;
    for (size_t i = 0; i < v1.size(); ++i) {
        float scaled_diff = (v1[i] - v2[i]) / (stdevs[i] + 1e-6); // Avoid division by zero
        distance += scaled_diff * scaled_diff;
    }
    return sqrt(distance);
}

/**
* Decided by Nearest neighbor
* @param dbFeatures all training data
* @param features the current object features vector
* @return the closest label
*/
string classifyByNN(const vector<pair<string, vector<float>>>& dbFeatures, const vector<float>& features) {
    if (dbFeatures.empty()) return "Unknown";
    vector<float> stdevs = computeFeatureStdDevs(dbFeatures);

    string closestLabel = "Unknown";
    float minDistance = numeric_limits<float>::max();

    for (const auto& entry : dbFeatures) {
        float distance = calculateScaledEuclideanDistance(features, entry.second, stdevs);
        if (distance < minDistance) {
            minDistance = distance;
            closestLabel = entry.first;
        }
    }

    return (minDistance > 5.0f) ? "Unknown" : closestLabel;
}
