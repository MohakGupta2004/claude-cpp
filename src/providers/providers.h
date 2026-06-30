#pragma once


#include <string>
#include <vector>
#include "../config/config.h"
#include "../models/message.h"
class Provider {
  public:
    virtual std::pair<std::string, std::string> ask(const std::vector<Message> &history, Config& config) = 0;
    virtual std::string getName() const = 0;
    virtual ~Provider()=default;
};
