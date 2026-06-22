#include <iostream>
#include "chat/chatbot.h"
#include "conversation/conversation.h"
#include "storage/storage.h"
#include "commander/commander.h"
int main() {
    Conversation conversation;
    Storage storage;
    Chatbot bot;
    Commander handler;
    storage.load(conversation);
    while(true) {
      std::string input;
      std::cout<<"> ";
      std::getline(std::cin, input);
      if(input == "exit") {
        break;
      }
      if(handler.handle(input, conversation,storage))  {
        continue;
      }
      conversation.addMessage({
        "user", input
      });
      std::string response = bot.respond(input);
      conversation.addMessage({"assistant", response});
      std::cout<<response<<std::endl;
    }
    return 0;
}
