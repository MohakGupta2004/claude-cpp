cmake_minimum_required(VERSION 3.15)

project(claude-code)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(cpr REQUIRED)

add_executable(claude_cpp src/main.cpp src/storage/storage.cpp  src/conversation/conversation.cpp src/commander/commander.cpp src/providers/provider_manager.cpp src/providers/openai_provider.cpp src/providers/claude_provider.cpp src/config/config.cpp)
  
target_link_libraries(
    claude_cpp
    PRIVATE
    nlohmann_json::nlohmann_json
)

target_include_directories(claude_cpp PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external
)

target_link_libraries(claude_cpp PRIVATE cpr::cpr)
