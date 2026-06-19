#include <iostream>
#include "chat/chatbot.h"
#include "conversation/conversation.h"
int main() {
    Conversation conversation;
    Chatbot bot;
    while(true) {
      std::string input;
      std::cout<<"> ";
      std::getline(std::cin, input);
      if(input.empty()) {
        continue;
      }
      if(input == "exit") {
        break;
      } 
      if(input == "history") {
        conversation.printHistory();
        continue;
      }
      if(input=="clear") {
        conversation.clearHistory();
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
