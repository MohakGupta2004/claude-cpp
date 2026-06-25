#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <string>
class Config {
public:
  void load();

  std::string getApiKey(const std::string &provider);

private: 
  json config;
};
