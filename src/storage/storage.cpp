#include "storage.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
void Storage::save(const Conversation& conversation) {
  const auto& conversations = conversation.getMessage();
  std::vector<std::string> serialized;
  for(auto msg:conversations) {
    serialized.push_back(msg.role+"|"+msg.content);
  }
  std::ofstream outfile("./meow.txt");
  for(const auto& msg:conversations) {
   outfile<<msg.role<<"|"<<msg.content<<std::endl; 
  }

}

void Storage::load(Conversation &conversation) {
  std::ifstream file("./conversation.txt");
  if(!file.is_open()) {
    return;
  }
  std::string text;
  while(std::getline(file, text)) {
    auto delimiter = text.find("|");
    if(delimiter == std::string::npos) {
      continue;
    }  

    std::string role = text.substr(0, delimiter);
    std::string content = text.substr(delimiter+1);
    conversation.addMessage({role, content});
  }

}
