#pragma once

#include "providers.h"
#include <string>
class Openai:public Provider {
  public:
    std::pair<std::string, std::string> ask(const std::vector<Message>& history, Config& config) override;
    std::string getName() const override;
};
