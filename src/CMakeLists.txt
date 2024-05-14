include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/include
)

add_library(Entity STATIC     
	${CMAKE_CURRENT_LIST_DIR}/Entity/basic_function/DatabaseManager.cpp
	${CMAKE_CURRENT_LIST_DIR}/Entity/basic_function/Table.cpp
	${CMAKE_CURRENT_LIST_DIR}/Entity/basic_function/TableManager.cpp
	${CMAKE_CURRENT_LIST_DIR}/Entity/basic_function/AggregationFunctions.cpp
)

add_executable(DBMS ${CMAKE_CURRENT_LIST_DIR}/main.cpp)

target_link_libraries(DBMS Entity)