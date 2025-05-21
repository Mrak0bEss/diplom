#define CROW_USE_BOOST true
#include <iostream>
#include "crow.h"
#include <rpc.h>
#include <thread>
#include <unordered_map>
#include <chrono>
#pragma comment(lib, "rpcrt4.lib")

std::mutex cacheMutex;
std::unordered_map<std::string, std::vector<std::string>> storage;
std::unordered_map<std::string, std::unordered_map<std::string, size_t>> reverseLookup;
std::unordered_map<std::string, std::unordered_map<size_t, std::chrono::steady_clock::time_point>> ttlStorage;

std::string generateGUID() {
    UUID uuid;
    UuidCreate(&uuid);
    char* guidStr;
    UuidToStringA(&uuid, (RPC_CSTR*)&guidStr);
    std::string guid(guidStr);
    RpcStringFreeA((RPC_CSTR*)&guidStr);
    return guid;
}

void cleanupExpiredData() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::lock_guard<std::mutex> lock(cacheMutex);

        for (auto& [guid, entries] : ttlStorage) {
            auto& dataList = storage[guid];
            auto& indexMap = reverseLookup[guid];

            std::vector<size_t> toRemove;
            auto now = std::chrono::steady_clock::now();

            for (auto& [id, expiry] : entries) {
                if (expiry <= now) {
                    toRemove.push_back(id);
                }
            }

            for (auto id : toRemove) {
                if (id < dataList.size()) {
                    std::string value = dataList[id];
                    dataList[id] = "[EXPIRED]";
                    indexMap.erase(value);
                    entries.erase(id);
                }
            }
        }
    }
}

int main() {
    crow::SimpleApp app;

    std::thread(cleanupExpiredData).detach();

    CROW_ROUTE(app, "/generate_guid").methods(crow::HTTPMethod::GET)
        ([]() {
        std::string guid = generateGUID();
        std::lock_guard<std::mutex> lock(cacheMutex);
        storage[guid] = {};
        reverseLookup[guid] = {};
        ttlStorage[guid] = {};
        return crow::response(200, guid);
            });

    CROW_ROUTE(app, "/store/<string>/<int>").methods(crow::HTTPMethod::POST)
        ([](const crow::request& req, std::string guid, int ttlSeconds) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = storage.find(guid);
        if (it == storage.end()) {
            return crow::response(404, "GUID not found");
        }
        std::string data = req.body;
        it->second.push_back(data);
        size_t id = it->second.size() - 1;
        reverseLookup[guid][data] = id;
        ttlStorage[guid][id] = std::chrono::steady_clock::now() + std::chrono::seconds(ttlSeconds);
        return crow::response(200, std::to_string(id));
            });

    CROW_ROUTE(app, "/retrieve/<string>/<int>").methods(crow::HTTPMethod::GET)
        ([](std::string guid, int id) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = storage.find(guid);
        if (it == storage.end() || id >= it->second.size()) {
            return crow::response(404, "Not found");
        }

        auto ttlIt = ttlStorage[guid].find(id);
        if (ttlIt != ttlStorage[guid].end() && ttlIt->second < std::chrono::steady_clock::now()) {
            return crow::response(410, "Expired");
        }

        return crow::response(200, it->second[id]);
            });

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
        size_t id = dataIt->second;
        auto ttlIt = ttlStorage[guid].find(id);
        if (ttlIt != ttlStorage[guid].end() && ttlIt->second < std::chrono::steady_clock::now()) {
            return crow::response(410, "Expired");
        }
        return crow::response(200, std::to_string(id));
            });

    app.port(8080).multithreaded().run();
}
