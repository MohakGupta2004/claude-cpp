#include "openai_provider.h"
#include <iostream>
#include <string>
#include <agent_sdk/agent_sdk.hpp>
std::string Openai::ask(std::string &prompt, Config &config) {
  const std::string API_KEY = config.getApiKey("openai");

}

std::string Openai::getName() const { return "openai"; }
