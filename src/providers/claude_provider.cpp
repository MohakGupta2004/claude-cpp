#include "claude_provider.h"
#include <string>
#include <utility>
std::pair<std::string, std::string> Claude::ask(const std::vector<Message> &history, Config& config) { return {"", ""}; }

std::string Claude::getName() const { return "claude"; }
