#include "commander.h"
#include <string>

bool Commander::handle(std::string input, Conversation& conversation,
                       Storage& storage) {
  if (input.empty()) {
    return true;
  }

  if (input == "history") {
    conversation.printHistory();
    return true;
  }
  if (input == "clear") {
    conversation.clearHistory();
    return true;
  }
  if (input == "save") {
    storage.save(conversation);
    return true;
  }

  return false;
}
