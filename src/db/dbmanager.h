#pragma once
#include <mongocxx/client.hpp>

#include <mongocxx/instance.hpp>

class DBManager {
public:
    static DBManager& getInstance();

    mongocxx::client& getClient();

private: // encapsulate
    DBManager();
    mongocxx::instance inst; // initializes the MongoDB driver once
    mongocxx::client client_; // create connection to MongoDB deployment
};