// functions.h
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <map>
#include <string>
#include <utility> 
#include <cpprest/http_listener.h> // For http_request and related classes
#include <cpprest/json.h>          // For json::value

//extern indicate that serverRunning is declared in other place
extern std::atomic<bool> serverRunning;

// Function declarations
std::map<int, std::pair<std::string, double>> loadDataFromCSV(const std::string filename);
web::json::value getData(int id, const std::map<int, std::pair<std::string, double>> data);
void handle_get(web::http::http_request request);
void startHttpServer(int port);
void handle_post(web::http::http_request request);
bool insertRecord(int id, const std::string name, double price);
void handle_delete(web::http::http_request request);
bool deleteRecord(int id);
#endif // FUNCTIONS_H
