#include <iostream>
#include <memory>
#include <stdexcept>
#include "conversation/conversation.h"
#include "providers/claude_provider.h"
#include "providers/openai_provider.h"
#include "providers/provider_manager.h"
#include "storage/storage.h"
#include "commander/commander.h"
int main() {
    Conversation conversation;
    Storage storage;
    Commander handler;
    storage.load(conversation);
    ProviderManager provider;
    provider.registerProvider(std::make_shared<Openai>());
    provider.registerProvider(std::make_shared<Claude>());
    try {
      provider.setProvider("openai");
    } catch (const std::runtime_error e) {
      std::cerr<<"Failed to initialize provider: "<<e.what()<<std::endl;
      return 1;
    }
    while(true) {
      std::string input;
      std::cout<<"> ";
      std::getline(std::cin, input);
      if(input == "exit") {
        break;
      }
      if(handler.handle(input, conversation,storage, provider))  {
        continue;
      }
      conversation.addMessage({
        "user", input
      });
      std::string response = provider.getCurrentProvider().ask(input);
      conversation.addMessage({"assistant", response});
      std::cout<<response<<std::endl;
    }
    return 0;
}
