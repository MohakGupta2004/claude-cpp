cmake_minimum_required(VERSION 3.15)

project(claude-code)

add_executable(claude_cpp src/main.cpp src/storage/storage.cpp  src/conversation/conversation.cpp src/commander/commander.cpp src/providers/provider_manager.cpp src/providers/openai_provider.cpp src/providers/claude_provider.cpp )  
