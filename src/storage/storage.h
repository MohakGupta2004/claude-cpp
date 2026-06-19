#pragma once
#include "../conversation/conversation.h"

class Storage {

  public: 
    void save(const Conversation& conversation); 
    void load(Conversation& conversation);
  
};
