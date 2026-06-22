#pragma once


#include <string>
class Provider {
  public:
    virtual std::string ask(std::string &prompt) = 0;
    virtual std::string getName() const = 0;
    virtual ~Provider()=default;
};
