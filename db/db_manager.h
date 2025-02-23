/*
 * Authors: Yuyang Tian and Arun Mekkad
 * Date: 2025/2/16
 * Purpose: Header file for DB managment
 */

#ifndef PROJ3_DB_MANAGER_H
#define PROJ3_DB_MANAGER_H
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/collection.hpp>
#include <vector>
#include <string>

class DBManager {
public:
    DBManager();
    int writeFeatureVector(const std::string &label, const std::vector<float>& featureVector);
    int loadFeatureVectors(std::vector<std::pair<std::string, std::vector<float>>>& data);
    int deleteAll();
private:
    mongocxx::client client;
    mongocxx::collection collection;
};
#endif //PROJ3_DB_MANAGER_H
