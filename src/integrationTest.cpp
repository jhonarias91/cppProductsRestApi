#include <gtest/gtest.h>
#include <cpprest/http_client.h>
#include <thread>
#include "functions.h"

using namespace web;
std::thread serverThread;

void startServer(int port) {
    serverRunning.store(true);  
    // Start the server in a new hilo
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
            http::client::http_client client(uri);
            auto task = client.request(http::methods::GET, U("/health")); // Use the health check endpoint
            auto response = task.get(); // Wait for the response
            if (response.status_code() == http::status_codes::OK) {
                return true; // Server ready
            }
        } catch (...) { //... catch any exception
            // Server not ready yet, wait a bit before retrying
            std::this_thread::sleep_for(milliseconds(100));
        }
    }
    return false; // Timeout reached, server not ready
}


TEST(GetTest, TestGetId1oK) {
    startServer(4301);
    ASSERT_TRUE(waitForServerReady("http://localhost:4301", 5)); // 5 seconds timeout

    http::client::http_client client(U("http://localhost:4301"));
    auto query = uri::encode_uri(U("?id=1")); 
    client.request(http::methods::GET, query)
        .then([](http::http_response response) {
            EXPECT_EQ(response.status_code(), http::status_codes::OK);

            auto body = response.extract_json().get();
            EXPECT_EQ(body[U("id")].as_integer(), 1);
             EXPECT_EQ(body[U("name")].as_string(), "Tableta Electrónica");
              EXPECT_EQ(body[U("price")].as_double(), 553.37);
            
        }).wait();

   stopServer();
}

TEST(GetTest, TestGetId5oK) {
    startServer(4302);
    ASSERT_TRUE(waitForServerReady("http://localhost:4302", 5)); // 5 seconds timeout

    http::client::http_client client(U("http://localhost:4302"));
    auto query = uri::encode_uri(U("?id=5")); 
    client.request(http::methods::GET, query)
        .then([](http::http_response response) {
            EXPECT_EQ(response.status_code(), http::status_codes::OK);

            auto body = response.extract_json().get();
            EXPECT_EQ(body[U("id")].as_integer(), 5);
             EXPECT_EQ(body[U("name")].as_string(), "Teléfono Inteligente");
              EXPECT_EQ(body[U("price")].as_double(), 812.25);
            
        }).wait();

    stopServer();
}

TEST(PostTest, TestAddRecordOk) {
    // Inicia el servidor en el puerto 4303
    startServer(4303);
    ASSERT_TRUE(waitForServerReady(U("http://localhost:4303"), 5)); // Espera 5 segundos a que el servidor esté listo

    http::client::http_client client(U("http://localhost:4303"));
    uri_builder builder(U("/insert")); 

    json::value requestBody;
    requestBody[U("id")] = json::value::number(777777);
    requestBody[U("name")] = json::value::string(U("Product 777777 Test"));
    requestBody[U("price")] = json::value::number(199.99);

    // Realiza la solicitud POST
    client.request(http::methods::POST, builder.to_string(), requestBody.serialize(), U("application/json"))
        .then([](http::http_response response) {
            // Verifica que el código de estado HTTP sea 200 OK
            EXPECT_EQ(response.status_code(), http::status_codes::OK);

            // Extrae y verifica el mensaje de respuesta
            auto body = response.extract_json().get();
            EXPECT_TRUE(body.has_field(U("message")));
            EXPECT_EQ(body[U("message")].as_string(), U("Record added successfully"));
        }).wait();

    //Check if was created ok
    auto query = uri::encode_uri(U("?id=777777")); 
    client.request(http::methods::GET, query)
        .then([](http::http_response response) {
            EXPECT_EQ(response.status_code(), http::status_codes::OK);

            auto body = response.extract_json().get();
            EXPECT_EQ(body[U("id")].as_integer(), 777777);
             EXPECT_EQ(body[U("name")].as_string(), "Product 777777 Test");
              EXPECT_EQ(body[U("price")].as_double(), 199.99);
            
        }).wait();


    //Delete test
     uri_builder builderDel(U("/delete/777777")); 
     client.request(http::methods::DEL,builderDel.to_string())
        .then([](http::http_response response) {
            EXPECT_EQ(response.status_code(), http::status_codes::OK);
        }).wait();

    //Check it was deleted ok
    auto queryNotFound = uri::encode_uri(U("?id=777777")); 
        client.request(http::methods::GET, queryNotFound)
            .then([](http::http_response response) {
                EXPECT_EQ(response.status_code(), http::status_codes::NotFound);
            }).wait();

    stopServer();
}

