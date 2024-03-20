#include "functions.h"
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <thread> // Necesario para std::this_thread::sleep_for

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

// Variable global atómica para nuestro caso multi hilo
std::atomic<bool> serverRunning{true};

std::map<int, std::pair<std::string, double>> loadDataFromCSV(const std::string filename) {
    std::map<int, std::pair<std::string, double>> data;
    std::ifstream file(filename);
    std::string line, idStr, name, priceStr;

    // Skip the header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream s(line);
        std::getline(s, idStr, ',');
        std::getline(s, name, ',');
        std::getline(s, priceStr, ',');

        int id = std::stoi(idStr);
        double price = std::stod(priceStr);

        data[id] = {name, price};
    }

    return data;
}

json::value getData(int id, std::map<int, std::pair<std::string, double>> data){
    // Create a JSON response
    json::value response;

    // Check if the id exists in the data
    if (data.find(id) != data.end()) {
        auto foundData = data[id];
        
        response[U("id")] = json::value::number(id);
        response[U("name")] = json::value::string(foundData.first);
        response[U("price")] = json::value::number(foundData.second);
        
    }
    return response;
}

void handle_get(http_request request) {
    // Check if the request is for the health check endpoint
    if (request.relative_uri().path() == U("/health")) {
        // Construct a simple health check response
        json::value healthResponse;
        healthResponse[U("status")] = json::value::string(U("up"));
        request.reply(status_codes::OK, healthResponse);
        return; // Return early since we've handled the health check
    }

    // Continue with the original function for handling data requests
    try {
        // Parse the query to get the `id`
        auto query = uri::split_query(request.relative_uri().query());
        int id = std::stoi(query[U("id")]);
        auto data = loadDataFromCSV("data.csv");

        json::value response = getData(id, data);
        if (response.is_null()) {
            request.reply(status_codes::NotFound, U("Data not found"));
        } else {
            request.reply(status_codes::OK, response);
        }
    } catch (const std::exception& e) {
        // Handle parsing errors or any other exceptions
        json::value errorResponse;
        errorResponse[U("error")] = json::value::string(U("Invalid request"));
        request.reply(status_codes::BadRequest, errorResponse);
    }
}

void handle_post(http_request request) {
    request.extract_json().then([&](json::value requestBody) {
        int id = requestBody[U("id")].as_integer();
        std::string name = requestBody[U("name")].as_string();
        double price = requestBody[U("price")].as_double();

        if (addRecordToCSV(id, name, price)) {
            // Success
            json::value responseMessage;
            responseMessage[U("message")] = json::value::string(U("Record added successfully"));
            request.reply(status_codes::OK, responseMessage);
        } else {
            // Failure
            request.reply(status_codes::InternalError, U("Failed to open data file"));
        }
    }).wait();
}


bool addRecordToCSV(int id, const std::string name, double price) {
    try {
        std::ofstream file("data.csv", std::ios::app); // Open in append mode
        if (!file.is_open()) {
            return false;
        }
        file << id << "," << name << "," << price << std::endl;
        return true;
    } catch (const std::exception e) {    
        return false;
    }
}

void handle_delete(http_request request) {
    // Extract the ID from the URI
    auto paths = uri::split_path(request.relative_uri().path());
    //The ID is the first path segment following the DELETE endpoint, /delete/{productId}
    if (paths.empty() || paths[0] != U("delete")) {
        request.reply(status_codes::BadRequest, U("Invalid request"));
        return;
    }

    int idToDelete = std::stoi(paths[1]);
    bool deleteSuccess = deleteRecord(idToDelete);

    if (deleteSuccess) {
        request.reply(status_codes::OK, U("Record deleted successfully"));
    } else {
        request.reply(status_codes::NotFound, U("Record not found"));
    }
}

bool deleteRecord(int id) {
    std::vector<std::string> lines;
    std::string line;
    bool found = false;

    // Open the original file
    std::ifstream fileIn("data.csv");
    while (getline(fileIn, line)) {
        std::istringstream s(line);
        std::string idStr;
        getline(s, idStr, ',');
        if (std::stoi(idStr) != id) {
            lines.push_back(line);
        } else {
            found = true;
        }
    }
    fileIn.close();

    if (!found) {
        return false; // Record not found
    }

    // Write the remaining records back to the file
    std::ofstream fileOut("data.csv", std::ios::trunc);
    for (const auto& l : lines) {
        fileOut << l << std::endl;
    }
    fileOut.close();

    return true;
}

void startHttpServer(int port){
    // Cast the port to string
    
    uri_builder uri(U("http://0.0.0.0"));
     uri.set_port(port); // Set the port directly
    auto addr = uri.to_uri().to_string();
    http_listener listener(addr);
    
    listener.support(methods::GET, handle_get);
    listener.support(methods::POST, handle_post);
    listener.support(methods::DEL, handle_delete);

    try {
        listener
            .open()
            .then([&listener]() { std::cout << "Starting to listen at: " << listener.uri().to_string() << std::endl; })
            .wait();

        // Mantener la aplicación ejecutándose indefinidamente sin terminar.
        while (serverRunning.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Pausa eficiente, cuando despierta vuelva a entrar al while.
        }

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }
}
