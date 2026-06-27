#include "openai_provider.h"
#include <cpr/cpr.h>
#include <cpr/response.h>
#include <iostream>
#include <string>

std::string Openai::ask(std::string &prompt, Config &config) {
    const std::string API_KEY = config.getApiKey("openai");
    
    // 1. Construct the payload
    json json_payload = {
        {"model", "gpt-5.5"},
        {"reasoning", {{"effort", "low"}}},
        {"input", json::array({
            {{"role", "developer"}, {"content", "You are a helpful assistant."}},
            {{"role", "user"}, {"content", prompt}}
        })}
    };

    // 2. Perform the network request
    cpr::Response r = cpr::Post(
        cpr::Url{"https://api.openai.com/v1/responses"},
        cpr::Body{json_payload.dump()},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Header{{"Authorization", "Bearer " + API_KEY}}
    );

    // 3. Handle network/HTTP communication failures immediately
    if (r.status_code != 200) {
        std::cerr << "HTTP Error " << r.status_code << ": " << r.text << "\n";
        return "Error: Communication failure with API server.";
    }

    // 4. Parse response safely
    try {
        json response_json = json::parse(r.text);
        int output_array_size =response_json["output"].size(); 
        return response_json["output"][output_array_size-1]["content"][0]["text"];
    } 
    catch (const json::exception &e) {
        std::cerr << "JSON Exception: " << e.what() << "\n";
    }

    return "Error: Unable to process response payload.";
}


std::string Openai::getName() const { return "openai"; }
