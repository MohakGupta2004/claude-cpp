
#include "config.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
void Config::load() {

  const char *buffer = nullptr;

#if defined(_WIN32) || defined(_WIN64)
  buffer = std::getenv("USERNAME");
#else
  buffer = std::getenv("USER");
#endif

  if (buffer == nullptr) {
    std::cerr << "Could not retrieve username from environment variables."
              << std::endl;
    return;
  }
  std::string username(buffer);
  std::string filepath =
      "/home/" + username + "/.config/claude_cpp/config.json";
  try {
    if (std::filesystem::exists(filepath)) {
      // TODO
      std::fstream f(filepath);
      config = json::parse(f);
    } else {
      // 2. Only create it if it does not exist
      std::filesystem::create_directories(
          std::filesystem::path("/home/" + username + "/.config/claude_cpp/"));
      std::ofstream file(filepath);

      if (file.is_open()) {
        file << "{\n \"API_KEY\": {\n   \"claude\":\"\", \n   "
                "\"openai\":\"\"\n }\n}";
        file.close();
        std::cout << "Config files are created. \n";
        std::cout << "type\n nano ~/.config/claude_cpp/config.json \n add "
                     "your api key and default provider there \n";
      }
    }
  } catch (const std::filesystem::filesystem_error e) {
    std::cout << e.what() << std::endl;
  }
}

std::string Config::getApiKey(const std::string &provider) {
  std::string apikey;
  try {
    if (config["API_KEY"][provider] != NULL) {
      apikey = config["API_KEY"][provider];
    } else {
      throw std::runtime_error(
          "Couldn't able to parse API_KEY. Make sure you provide one.");
    }
  } catch (const std::runtime_error e) {
    std::cerr << e.what() << std::endl;
  } catch (const json::parse_error e) {
    std::cerr << e.what() << std::endl;
  }
  return apikey;
}
