/*
 * Authors: Yuyang Tian and Arun Mekkad
 * Date: 2025/2/16
 * Purpose: Header file for MongoDB configuration
 */

#ifndef PROJ3_DB_CONFIG_H
#define PROJ3_DB_CONFIG_H

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
class DBConfig {
public:
    static mongocxx::client connect();
};
#endif //PROJ3_DB_CONFIG_H
