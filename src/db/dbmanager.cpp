#include "dbmanager.h"
#include <cstdlib>
#include <stdexcept>
#include <mongocxx/uri.hpp>

// initialize 'inst' with uri
DBManager::DBManager() : inst{}
{
    const char* uri = std::getenv("MONGODB_URI");
     
    if (!uri) {
        throw std::runtime_error("MONGODB_URI not set.");
    }

    client_ = mongocxx::client{ mongocxx::uri{uri} };
    
}

DBManager& DBManager::getInstance() {
    static DBManager instance;
    return instance;
}

mongocxx::client& DBManager::getClient() {
    return client_;
}