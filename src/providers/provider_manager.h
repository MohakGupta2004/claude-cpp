#pragma once

#include "providers.h"
#include <memory>
#include <string>
#include <unordered_map>
class ProviderManager {
    private: 
      std::unordered_map<std::string, std::shared_ptr<Provider>> providers; 
      std::shared_ptr<Provider> active_provider;
    public:
      void registerProvider(std::shared_ptr<Provider> provider);
      void setProvider(std::string model_name);
      Provider& getCurrentProvider(); 
};
