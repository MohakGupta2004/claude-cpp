#pragma once

#include "providers.h"
#include <string>
class Claude:public Provider {
  public:
    std::string ask(std::string& prompt) override;
    std::string getName() const override;
};
