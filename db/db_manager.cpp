/*
 * Authors: Yuyang Tian and Arun Mekkad
 * Date: 2025/2/16
 * Purpose: Persistent MongoDB Connection for Feature Storage
 */

#include "db_manager.h"
#include "db_config.h"
#include <iostream>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/json.hpp>

using namespace std;

// Define MongoDB client and connection as **persistent** members
// Create a database called "feature_db", a data collection called "features"
DBManager::DBManager() : client(DBConfig::connect()), collection(client["feature_db"]["features"]) {
    cout << "MongoDB Connection Initialized!" << endl;
}

// Store feature vector in MongoDB
int DBManager::writeFeatureVector(const string &label, const vector<float> &featureVector) {
    try {
        bsoncxx::builder::basic::document document{};
        bsoncxx::builder::basic::array featureArray;

        for (float val: featureVector) {
            featureArray.append(val);
        }

        document.append(
                bsoncxx::builder::basic::kvp("type", label),
                bsoncxx::builder::basic::kvp("features", featureArray)
        );

        auto result = collection.insert_one(document.view());

        if (result && result->result().inserted_count() == 1) {
            cout << "Successfully created a new feature vector in MongoDB!" << endl;
            cout << "Feature vector stored for type: " << label << endl;
            cout << "Feature vector size : " << featureVector.size() << endl;
            return 0;  // Success
        } else {
            cerr << "Error: Insertion failed for label: " << label << endl;
            return -1;  // Failure
        }
    } catch (const exception &e) {
        cerr << "Exception occurred while inserting feature vector: " << e.what() << endl;
        return -1;
    }
}


//  Load all feature vectors from MongoDB
int DBManager::loadFeatureVectors(vector<pair<string, vector<float>>>& data) {
    mongocxx::cursor cursor = collection.find({});

    for (auto&& doc : cursor) {
        string label;
        vector<float> featureVector;

        if (doc["type"] && doc["type"].type() == bsoncxx::type::k_string) {
            label = string(doc["type"].get_string().value); // fix: boost string error
        } else {
            cerr << "Error: 'type' field missing or incorrect type!" << endl;
            continue;
        }

        if (doc["features"] && doc["features"].type() == bsoncxx::type::k_array) {
            auto bsonArray = doc["features"].get_array().value;
            for (auto &&val : bsonArray) {
                if (val.type() == bsoncxx::type::k_double) {
                    featureVector.push_back(val.get_double().value);
                } else {
                    cerr << "Error: Non-float value found in feature vector!" << endl;
                }
            }
        } else {
            cerr << "Error: 'features' field missing or incorrect type!" << endl;
            continue;
        }

        data.emplace_back(label, featureVector);
    }
    return data.size();
}
int DBManager::deleteAll() {
    try {
        auto result = collection.delete_many({});  // Delete all documents

        if (result) {
            cout << "Deleted " << result->deleted_count() << " documents from the collection." << endl;
            return static_cast<int>(result->deleted_count());
        } else {
            cerr << "Error: Deletion failed!" << endl;
            return -1;
        }
    } catch (const exception &e) {
        cerr << "Exception while deleting all documents: " << e.what() << endl;
        return -1;
    }
}