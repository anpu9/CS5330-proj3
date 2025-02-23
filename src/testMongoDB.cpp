/*
 * Authors: Yuyang Tian and Arun Mekkad
 * Date: 2025/2/16
 * Purpose: ${PURPOSE}
 */
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/collection.hpp>
#include <iostream>

using namespace mongocxx;
using namespace bsoncxx;
using namespace std;

int main() {
    try {
        // Connect to MongoDB instance (assuming it's running locally at default port)
        std::string url_yuyang = "mongodb+srv://yuyangtian23:Woaiyangyang0810!@cs5330-proj3.mvlhe.mongodb.net/?retryWrites=true&w=majority&appName=CS5330-Proj3";
        mongocxx::client conn{mongocxx::uri{url_yuyang}};

        // Specify the database name and collection name
        std::string db_name = "feature_db";       // Replace with your actual database name
        std::string collection_name = "testing";  // Replace with your actual collection name

        // Check for empty db_name or collection_name
        if (db_name.empty() || collection_name.empty()) {
            cerr << "Error: Invalid database name or collection name!" << endl;
            return 1;
        }

        // Get the database
        mongocxx::database db = conn[db_name];

        // Get the collection
        mongocxx::collection collection = db[collection_name];

        // Test by getting the count of documents in the collection
        auto count = collection.count_documents({});
        cout << "There are " << count << " documents in the collection " << collection_name << endl;
    }
    catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
