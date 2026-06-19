#pragma once
#include "../models/message.h"
#include <vector>
class Conversation {
  private:
    std::vector<Message> message;
  public:
  void addMessage(const Message& msg);
  void printHistory() const;
};
