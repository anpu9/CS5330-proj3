/*
 * Authors: Yuyang Tian and Arun Mekkad
 * Date: 2025/2/16
 * Purpose: Configuration for MongoDB connection
 */

#include "db_config.h"
mongocxx::client DBConfig::connect() {
    static mongocxx::instance inst{};
    std::string url_yuyang = "mongodb+srv://yuyangtian23:Woaiyangyang0810!@cs5330-proj3.mvlhe.mongodb.net/?retryWrites=true&w=majority&appName=CS5330-Proj3";
    // TODO: add username and password
    std::string url_arun = "mongodb+srv://your-username:your-password@your-cluster.mongodb.net/?retryWrites=true&w=majority&tls=true";
    mongocxx::uri uri(url_yuyang);
    return mongocxx::client(uri);
}