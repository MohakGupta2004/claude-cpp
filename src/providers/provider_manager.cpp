#include "provider_manager.h"
#include "providers.h"
#include <memory>
#include <stdexcept>
#include <string>
Provider &ProviderManager::getCurrentProvider() { return *active_provider; }

void ProviderManager::registerProvider(const std::shared_ptr<Provider> provider) {
  ProviderManager::providers[provider->getName()] = provider;
}

void ProviderManager::setProvider(std::string model_name) {
  if(!ProviderManager::providers[model_name])  {
    throw std::runtime_error("Model does not exists");
  }
  ProviderManager::active_provider = ProviderManager::providers[model_name];
}
