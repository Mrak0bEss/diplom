#define CROW_USE_BOOST true
#include <iostream>
#include "crow.h"
#include <rpc.h>
#pragma comment(lib, "rpcrt4.lib")
//#include <rpcdce.h>
std::mutex cacheMutex;
std::unordered_map<std::string, std::vector<std::string>> storage;
std::unordered_map<std::string, std::unordered_map<std::string, size_t>> reverseLookup;


std::string generateGUID() {
    UUID uuid;
    UuidCreate(&uuid);
    char* guidStr;
    UuidToStringA(&uuid, (RPC_CSTR*)&guidStr);
    std::string guid(guidStr);
    RpcStringFreeA((RPC_CSTR*)&guidStr);
    return guid;
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/generate_guid").methods(crow::HTTPMethod::GET)
        ([]() {
        std::string guid = generateGUID();
        std::lock_guard<std::mutex> lock(cacheMutex);
        storage[guid] = {};
        reverseLookup[guid] = {};
        return crow::response(200, guid);
            });

    // Сохранение данных
    CROW_ROUTE(app, "/store/<string>").methods(crow::HTTPMethod::POST)
        ([](const crow::request& req, std::string guid) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = storage.find(guid);
        if (it == storage.end()) {
            return crow::response(404, "GUID not found");
        }
        std::string data = req.body;
        it->second.push_back(data);
        size_t id = it->second.size() - 1;
        reverseLookup[guid][data] = id;
        return crow::response(200, std::to_string(id));
            });

    // Получение данных по GUID
    CROW_ROUTE(app, "/retrieve/<string>/<int>").methods(crow::HTTPMethod::GET)
        ([](std::string guid, int id) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = storage.find(guid);
        if (it == storage.end() || id >= it->second.size()) {
            return crow::response(404, "Not found");
        }
        return crow::response(200, it->second[id]);
            });

    // Поиск ID данных по значению
    CROW_ROUTE(app, "/find/<string>").methods(crow::HTTPMethod::POST)
        ([](const crow::request& req, std::string guid) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = reverseLookup.find(guid);
        if (it == reverseLookup.end()) {
            return crow::response(404, "GUID not found");
        }
        std::string data = req.body;
        auto dataIt = it->second.find(data);
        if (dataIt == it->second.end()) {
            return crow::response(404, "Data not found");
        }
        return crow::response(200, std::to_string(dataIt->second));
            });
    app.port(8080).multithreaded().run();
}
