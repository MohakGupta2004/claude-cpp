#include <string>
#include "chatbot.h"
std::string Chatbot::respond(const std::string& input) {
      if(input=="hello") {
        return "Hello";
      }
      if(input=="pramit") {
        return "rendi";
      }

      return "Sorry can't help with that";
}
