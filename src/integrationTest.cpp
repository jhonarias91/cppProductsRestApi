#include <gtest/gtest.h>
#include <cpprest/http_client.h>
#include <thread>
#include "functions.h"

using namespace web;
std::thread serverThread;

void startServer(int port) {
    serverRunning.store(true);  
    // Esta función inicia el servidor en un nuevo hilo
    serverThread = std::thread(startHttpServer, port);
}

void stopServer() {    

    serverRunning.store(false);  
    // Esperar a que el hilo del servidor termine
    serverThread.join(); 
}

/*
* THis function allow to wait for ${timeoutSeconds} 
* until the server is ready to allo the test to start
*/
bool waitForServerReady(const std::string& uri, int timeoutSeconds) {
    using namespace std::chrono;
    auto startTime = steady_clock::now();
    while (duration_cast<seconds>(steady_clock::now() - startTime).count() < timeoutSeconds) {
        try {
            web::http::client::http_client client(uri);
            auto task = client.request(web::http::methods::GET, U("/health")); // Use the health check endpoint
            auto response = task.get(); // Wait for the response
            if (response.status_code() == web::http::status_codes::OK) {
                return true; // Server ready
            }
        } catch (...) {
            // Server not ready yet, wait a bit before retrying
            std::this_thread::sleep_for(milliseconds(100));
        }
    }
    return false; // Timeout reached, server not ready
}



TEST(ServerTest, TestGetId1oK) {
    startServer(4301);
    ASSERT_TRUE(waitForServerReady("http://localhost:4301", 5)); // 5 seconds timeout

    web::http::client::http_client client(U("http://localhost:4301"));
    auto query = uri::encode_uri(U("?id=1")); 
    client.request(web::http::methods::GET, query)
        .then([](web::http::http_response response) {
            EXPECT_EQ(response.status_code(), web::http::status_codes::OK);

            auto body = response.extract_json().get();
            EXPECT_EQ(body[U("id")].as_integer(), 1);
             EXPECT_EQ(body[U("name")].as_string(), "Tableta Electrónica");
              EXPECT_EQ(body[U("price")].as_double(), 553.37);
            
        }).wait();

   stopServer();
}

TEST(ServerTest, TestGetId5oK) {
    startServer(4302);
    ASSERT_TRUE(waitForServerReady("http://localhost:4302", 5)); // 5 seconds timeout

    web::http::client::http_client client(U("http://localhost:4302"));
    auto query = uri::encode_uri(U("?id=5")); 
    client.request(web::http::methods::GET, query)
        .then([](web::http::http_response response) {
            EXPECT_EQ(response.status_code(), web::http::status_codes::OK);

            auto body = response.extract_json().get();
            EXPECT_EQ(body[U("id")].as_integer(), 5);
             EXPECT_EQ(body[U("name")].as_string(), "Teléfono Inteligente");
              EXPECT_EQ(body[U("price")].as_double(), 812.25);
            
        }).wait();

    stopServer();
}

