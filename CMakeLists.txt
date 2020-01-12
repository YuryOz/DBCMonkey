cmake_minimum_required (VERSION 2.6)
project (DBCMonkey)

add_executable(${PROJECT_NAME} src/DBCMonkey.cpp src/DBCParser.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
