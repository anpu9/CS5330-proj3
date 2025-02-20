/*
 * Authors: Yuyang Tian and Arun Mekkad
 * Date: 2025/2/16
 * Purpose: Classifiers for object detection
 */

#ifndef PROJ3_CLASSIFIER_H
#define PROJ3_CLASSIFIER_H
#include <iostream>
using namespace std;
// Nearest neighbor Classifiers
string classifyByNN(const vector<pair<string, vector<float>>>& dbFeatures, const vector<float>& features);
string classifyByDecisionTree(const vector<float>& features);
#endif //PROJ3_CLASSIFIER_H
