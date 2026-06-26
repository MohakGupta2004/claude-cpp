#pragma once


#include <string>
#include "../config/config.h"
class Provider {
  public:
    virtual std::string ask(std::string &prompt, Config& config) = 0;
    virtual std::string getName() const = 0;
    virtual ~Provider()=default;
};
