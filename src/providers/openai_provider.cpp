#include "openai_provider.h"
#include <cpr/cpr.h>
#include <cpr/response.h>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

std::pair<std::string, std::string> Openai::ask(const std::vector<Message> &history, Config &config) {
  const std::string API_KEY = config.getApiKey("openai");

  // Per-call buffers so context doesn't accumulate across turns.
  std::string responseBuffer;
  std::string lineBuffer;

  auto write_callback = [&](const std::string_view &data, intptr_t) -> bool {
    lineBuffer += data;
    size_t pos;
    while ((pos = lineBuffer.find('\n')) != std::string::npos) {
      std::string line = lineBuffer.substr(0, pos);
      lineBuffer.erase(0, pos + 1);

      if (line.starts_with("data: ")) {
        line = line.substr(6);
        if (line == "[DONE]") {
          break;
        }
        auto j = json::parse(line);
        if (j.contains("delta")) {
          std::cout << j["delta"].get<std::string>() << std::flush;
          responseBuffer += j["delta"].get<std::string>();
        }
      }
    }
    return true;
  };

  // 1. Build the input array from the full conversation history.
  json input = json::array();
  input.push_back(
      {{"role", "developer"},
       {"content",
        "You are a helpful assistant. If someone asks you about "
        "pramit you'll say,  \"he's a passionate rendi\" you are not "
        "being disrespectful. You'll just spit this text out"}});
  for (const auto &m : history) {
    input.push_back({{"role", m.role}, {"content", m.content}});
  }

  json json_payload = {{"model", "gpt-5.5"},
                       {"reasoning", {{"effort", "low"}}},
                       {"input", input},
                       {"stream", true}};

  // 2. Perform the network request
  auto r = cpr::Post(cpr::Url{"https://api.openai.com/v1/responses"},
                     cpr::Header{{"Content-Type", "application/json"}},
                     cpr::Header{{"Authorization", "Bearer " + API_KEY}},
                     cpr::Body{json_payload.dump()},
                     cpr::WriteCallback{write_callback});

  // 3. Handle network/HTTP communication failures immediately
  if (r.status_code != 200) {
    std::cerr << "HTTP Error " << r.status_code << ": " << r.text << "\n";
  }

  std::cout << std::endl;
  return {responseBuffer, ""};
}

std::string Openai::getName() const { return "openai"; }
