cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(DBMS LANGUAGES CXX)

include_directories(src/include)

add_executable(DBMS src/main.cpp
        src/Entity/basic_function/AggregationFunctions.cpp
        src/Entity/basic_function/DatabaseManager.cpp
        src/Entity/basic_function/Table.cpp
        src/Entity/basic_function/TableManager.cpp
)