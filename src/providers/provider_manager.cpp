#include "provider_manager.h"
#include "providers.h"
#include <memory>
#include <stdexcept>
#include <string>
Provider &ProviderManager::getCurrentProvider() { return *active_provider; }

void ProviderManager::registerProvider(const std::shared_ptr<Provider> &provider) {
  providers[provider->getName()] = provider;
}

void ProviderManager::setProvider(std::string model_name) {
  if(!providers[model_name])  {
    throw std::runtime_error("Model does not exists");
  }
  active_provider = providers[model_name];
}
