#pragma once

#include "providers.h"
#include <string>
class Claude:public Provider {
  public:
    std::string ask(std::string& prompt, Config& config) override;
    std::string getName() const override;
};
