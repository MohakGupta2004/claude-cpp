#pragma once
#include "../conversation/conversation.h"
#include "../storage/storage.h"
#include "../providers/provider_manager.h"
#include <string>
class Commander {
  private:
  public: 
    bool handle(std::string input, Conversation& conversation, Storage& storage, ProviderManager& provider);
};
