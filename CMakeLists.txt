cmake_minimum_required(VERSION 3.16)
project(StockNotifier)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(stock_notifier notifier.cpp)

target_include_directories(stock_notifier PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(stock_notifier PRIVATE ${CURL_LIBRARIES} nlohmann_json::nlohmann_json)

