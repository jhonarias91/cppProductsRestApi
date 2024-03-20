#include <gtest/gtest.h>
#include "functions.h"

#include <cpprest/json.h>
using namespace web;

/// Función auxiliar para crear una respuesta JSON
json::value createJsonResponse(int id, const std::string name, double price) {
    json::value response;
    response[U("id")] = json::value::number(id);
    response[U("name")] = json::value::string(name);
    response[U("price")] = json::value::number(price);
    return response;
}


TEST(GetDataTest, ValidProduct) {
    int id = 2;
    //Data de prueba controlada:
    std::map<int, std::pair<std::string, double>> testData = {
        {1, {"Producto 1", 10.5}}, 
        {2, {"Producto 2", 5.0}}
    };

    json::value result = getData(id,testData);

    ASSERT_EQ(result[U("id")].as_integer(), id);
    ASSERT_EQ(result[U("name")].as_string(), "Producto 2");
    ASSERT_EQ(result[U("price")].as_double(), 5.0);
}

TEST(GetDataTest, InvalidId) {

    std::map<int, std::pair<std::string, double>> testData = {
        {1, {"Producto 1", 10.5}}, 
        {2, {"Producto 2", 5.0}}
    };

    int id = 5; 
    json::value result = getData(id,testData);

    ASSERT_TRUE(result.is_null()); 
}

TEST(GetDataTest, EmptyFile) {

    std::map<int, std::pair<std::string, double>> testData = {};

    int id = 5; 
    json::value result = getData(id,testData);

    ASSERT_TRUE(result.is_null()); 
}

TEST(RecordAdditionTests, AddValidRecord) {
    
    int id = 99999;
    std::string name = "Test Product";
    double price = 9.9999;

    // Act: Attempt to add a record.
    bool resultAdded = insertRecord(id, name, price);

    // Assert: Verify the record was added successfully.
    ASSERT_TRUE(resultAdded);

    //Check if it exists
    std::map<int, std::pair<std::string, double>> data = loadDataFromCSV("data.csv");
    
    json::value result = getData(id,data);

    ASSERT_EQ(result[U("id")].as_integer(), id);
    ASSERT_EQ(result[U("name")].as_string(), name);
    ASSERT_EQ(result[U("price")].as_double(), price);

    bool resultDelete = deleteRecord(id);
    ASSERT_TRUE(resultDelete);
}


TEST(RecordDeleteTests, deleteNotFound) {
    int id = 999990; //Should not exists

    bool resultDelete = deleteRecord(id);
    ASSERT_FALSE(resultDelete);
}

