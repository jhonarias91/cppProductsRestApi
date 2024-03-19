#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include "functions.h"
#include <thread> // Necesario para std::this_thread::sleep_for

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;


int main() {
    uri_builder uri(U("http://0.0.0.0:4300"));
    auto addr = uri.to_uri().to_string();
    http_listener listener(addr);
    
    listener.support(methods::GET, handle_get);

    try {
        listener
            .open()
            .then([&listener]() { std::cout << "Starting to listen at: " << listener.uri().to_string() << std::endl; })
            .wait();

        // Mantener la aplicación ejecutándose indefinidamente sin terminar.
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(60)); // Pausa eficiente, cuando despierta vuelva a entrar al while.
        }

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    return 0;
}
