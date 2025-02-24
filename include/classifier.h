/*
 * Authors: Yuyang Tian and Arun Mekkad
 * Date: 2025/2/16
 * Purpose: Header file for classifiers 
 */

#ifndef PROJ3_CLASSIFIER_H
#define PROJ3_CLASSIFIER_H
#include <iostream>
#include <vector>
using namespace std;
vector<float> computeFeatureStdDevs(const vector<pair<string, vector<float>>>& dbFeatures);
// Nearest neighbor Classifiers
string classifyByNN(const vector<pair<string, vector<float>>>& dbFeatures, const vector<float>& features);
string classifyByDecisionTree(const vector<float>& features);
#endif //PROJ3_CLASSIFIER_H
