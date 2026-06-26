/*
 *  agent-sdk-cpp - A modern C++ library for building ReAct LLM agents
 *  Copyright (c) 2025 Abdelrahman Mostafa <abdomody35@gmail.com>
 *
 *  Licensed under the MIT License
 *  SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <optional>
#include <nlohmann/json.hpp>
#include <httplib.h>

#ifdef AGENT_SDK_ENABLE_LOGGING
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <mutex>

enum class LogLevel
{
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error,
    Critical,
    Off
};

namespace detail
{
    inline LogLevel &get_log_level()
    {
#ifndef AGENT_SDK_LOG_LEVEL
#define AGENT_SDK_LOG_LEVEL 1 // Default to Debug
#endif
        static LogLevel current_level = static_cast<LogLevel>(AGENT_SDK_LOG_LEVEL);
        return current_level;
    }

    inline std::mutex &get_log_mutex()
    {
        static std::mutex log_mutex;
        return log_mutex;
    }

    inline std::string timestamp()
    {
        char date[32];
        time_t t = time(0);
        tm my_tm;

#if defined(_MSC_VER) || defined(__MINGW32__)
#ifdef AGENT_SDK_USE_LOCALTIMEZONE
        localtime_s(&my_tm, &t);
#else
        gmtime_s(&my_tm, &t);
#endif
#else
#ifdef AGENT_SDK_USE_LOCALTIMEZONE
        localtime_r(&t, &my_tm);
#else
        gmtime_r(&t, &my_tm);
#endif
#endif

        size_t sz = strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &my_tm);
        return std::string(date, date + sz);
    }

    inline void output_log(LogLevel level, const std::string &message)
    {
        std::lock_guard<std::mutex> lock(get_log_mutex());

        const char *level_str = "";
        const char *color = "";
        const char *reset = "\033[0m";

        switch (level)
        {
        case LogLevel::Trace:
            level_str = "TRACE   ";
            color = "\033[37m";
            break;
        case LogLevel::Debug:
            level_str = "DEBUG   ";
            color = "\033[36m";
            break;
        case LogLevel::Info:
            level_str = "INFO    ";
            color = "\033[32m";
            break;
        case LogLevel::Warning:
            level_str = "WARNING ";
            color = "\033[33m";
            break;
        case LogLevel::Error:
            level_str = "ERROR   ";
            color = "\033[31m";
            break;
        case LogLevel::Critical:
            level_str = "CRITICAL";
            color = "\033[35m";
            break;
        default:
            break;
        }

#ifndef AGENT_SDK_NO_COLOR
        std::cerr << color << "[" << timestamp() << "] [" << level_str << "] " << reset << message << std::endl;
#else
        std::cerr << "[" << timestamp() << "] [" << level_str << "] " << message << std::endl;
#endif
    }

    // Base case for recursion
    inline void format_message(std::ostringstream &ss, const char *fmt)
    {
        while (*fmt)
        {
            if (*fmt == '{' && *(fmt + 1) == '}')
            {
                ss << "{}";
                fmt += 2;
            }
            else
            {
                ss << *fmt++;
            }
        }
    }

    // Recursive case
    template <typename T, typename... Args>
    inline void format_message(std::ostringstream &ss, const char *fmt, T &&value, Args &&...args)
    {
        while (*fmt)
        {
            if (*fmt == '{' && *(fmt + 1) == '}')
            {
                ss << std::forward<T>(value);
                format_message(ss, fmt + 2, std::forward<Args>(args)...);
                return;
            }
            else
            {
                ss << *fmt++;
            }
        }
    }

    template <typename... Args>
    inline void log_impl(LogLevel level, const char *fmt, Args &&...args)
    {
        if (level >= get_log_level())
        {
            std::ostringstream ss;
            format_message(ss, fmt, std::forward<Args>(args)...);
            output_log(level, ss.str());
        }
    }

    inline const char *to_cstr(const char *str) { return str; }
    inline const char *to_cstr(const std::string &str) { return str.c_str(); }
}

inline void set_log_level(LogLevel level)
{
    detail::get_log_level() = level;
}

inline LogLevel get_log_level()
{
    return detail::get_log_level();
}

template <typename... Args>
inline void LOG_TRACE(const char *fmt, Args &&...args)
{
    detail::log_impl(LogLevel::Trace, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LOG_DEBUG(const char *fmt, Args &&...args)
{
    detail::log_impl(LogLevel::Debug, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LOG_INFO(const char *fmt, Args &&...args)
{
    detail::log_impl(LogLevel::Info, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LOG_WARN(const char *fmt, Args &&...args)
{
    detail::log_impl(LogLevel::Warning, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LOG_ERROR(const char *fmt, Args &&...args)
{
    detail::log_impl(LogLevel::Error, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LOG_CRITICAL(const char *fmt, Args &&...args)
{
    detail::log_impl(LogLevel::Critical, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LOG_TRACE(const std::string fmt, Args &&...args)
{
    detail::log_impl(LogLevel::Trace, fmt.c_str(), std::forward<Args>(args)...);
}

template <typename... Args>
inline void LOG_DEBUG(const std::string fmt, Args &&...args)
{
    detail::log_impl(LogLevel::Debug, fmt.c_str(), std::forward<Args>(args)...);
}

template <typename... Args>
inline void LOG_INFO(const std::string fmt, Args &&...args)
{
    detail::log_impl(LogLevel::Info, fmt.c_str(), std::forward<Args>(args)...);
}

template <typename... Args>
inline void LOG_WARN(const std::string fmt, Args &&...args)
{
    detail::log_impl(LogLevel::Warning, fmt.c_str(), std::forward<Args>(args)...);
}

template <typename... Args>
inline void LOG_ERROR(const std::string fmt, Args &&...args)
{
    detail::log_impl(LogLevel::Error, fmt.c_str(), std::forward<Args>(args)...);
}

template <typename... Args>
inline void LOG_CRITICAL(const std::string fmt, Args &&...args)
{
    detail::log_impl(LogLevel::Critical, fmt.c_str(), std::forward<Args>(args)...);
}
#else

// If logging is disabled, define empty macros
#define LOG_TRACE(fmt, ...)
#define LOG_DEBUG(fmt, ...)
#define LOG_INFO(fmt, ...)
#define LOG_WARN(fmt, ...)
#define LOG_ERROR(fmt, ...)
#define LOG_CRITICAL(fmt, ...)

#endif // AGENT_SDK_ENABLE_LOGGING

/**
 * @brief HTTP client for making REST API calls
 *
 * Provides methods for GET and POST requests, including streaming support.
 * Handles URL parsing, SSL connections, and JSON response parsing.
 *
 * @note This class is thread-safe for concurrent requests.
 * @warning Requires CPPHTTPLIB_OPENSSL_SUPPORT to be defined for HTTPS requests.
 */
class HttpClient
{
public:
    HttpClient() = default;
    ~HttpClient() = default;

    std::optional<nlohmann::json> get(const std::string &url, const std::map<std::string, std::string> &headers = {})
    {
        httplib::Headers h;
        for (const auto &p : headers)
        {
            h.emplace(p.first, p.second);
        }
        return performRequest(url, std::nullopt, h, "GET", "");
    }

    std::optional<nlohmann::json> post(const std::string &url, const nlohmann::json &body, const std::map<std::string, std::string> &headers = {})
    {
        httplib::Headers h;
        for (const auto &p : headers)
        {
            h.emplace(p.first, p.second);
        }
        return performRequest(url, body.dump(), h, "POST", "application/json");
    }

    bool postStream(const std::string &url, const nlohmann::json &body, const std::map<std::string, std::string> &headers, std::function<void(const std::string &)> onChunk)
    {
        std::string host, path;
        if (!parse_url(url, host, path))
        {
            LOG_ERROR("Invalid URL for streaming: {}", url);
            return false;
        }

        httplib::Headers h;
        for (const auto &p : headers)
        {
            h.emplace(p.first, p.second);
        }

        // This buffer will store partial lines between callbacks
        thread_local std::string partialBuffer;

        // Streaming write callback
        auto writeCallback = [&](const char *ptr, size_t size) -> bool
        {
            size_t totalSize = size;

            // Append new data to partial buffer
            partialBuffer.append(ptr, totalSize);

            // Process full lines
            size_t pos = 0;
            while ((pos = partialBuffer.find("\n")) != std::string::npos)
            {
                std::string line = partialBuffer.substr(0, pos);
                partialBuffer.erase(0, pos + 1);

                // Trim CR if present
                if (!line.empty() && line.back() == '\r')
                    line.pop_back();

                if (!line.empty())
                {
                    onChunk(line);
                }
            }

            return true;
        };

        // Determine if we need SSL based on URL scheme
        bool use_ssl = url.find("https://") == 0;

        bool success = false;
        if (use_ssl)
        {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
            httplib::SSLClient cli(host);
            cli.set_connection_timeout(10, 0);
            cli.set_read_timeout(60, 0);

            auto res = cli.Post(path.c_str(), h, body.dump(), "application/json", writeCallback);

            success = (res && res->status == 200);
#else
            LOG_ERROR("HTTPS request failed: httplib was not compiled with SSL support for URL: {}", url);
            return false;
#endif
        }
        else
        {
            httplib::Client cli(host);
            cli.set_connection_timeout(10, 0);
            cli.set_read_timeout(60, 0);

            auto res = cli.Post(path.c_str(), h, body.dump(), "application/json", writeCallback);

            success = (res && res->status == 200);
        }

        // After the stream is finished, process any remaining data in the buffer
        if (!partialBuffer.empty())
        {
            onChunk(partialBuffer);
            partialBuffer.clear();
        }

        if (!success)
        {
            LOG_ERROR("Streaming request to {} failed", url);
            return false;
        }

        return true;
    }

    std::string urlEncode(const std::string &value)
    {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;

        for (char c : value)
        {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            {
                escaped << c;
                continue;
            }
            escaped << std::uppercase;
            escaped << '%' << std::setw(2) << int((unsigned char)c);
            escaped << std::nouppercase;
        }

        return escaped.str();
    }

private:
    static bool parse_url(const std::string &url, std::string &host, std::string &path)
    {
        const auto scheme_end = url.find("://");
        if (scheme_end == std::string::npos)
        {
            return false;
        }

        const auto host_start = scheme_end + 3;
        auto path_start = url.find('/', host_start);

        if (path_start == std::string::npos)
        {
            host = url.substr(host_start);
            path = "/";
        }
        else
        {
            host = url.substr(host_start, path_start - host_start);
            path = url.substr(path_start);
        }
        return true;
    }

    std::optional<nlohmann::json> performRequest(
        const std::string &url,
        const std::optional<std::string> &body,
        const httplib::Headers &headers,
        const std::string &method,
        const std::string &contentType)
    {

        std::string host, path;
        if (!parse_url(url, host, path))
        {
            LOG_ERROR("Invalid URL: {}", url);
            return std::nullopt;
        }

        // Determine if we need SSL based on URL scheme
        bool use_ssl = url.find("https://") == 0;

        std::optional<nlohmann::json> result;

        if (use_ssl)
        {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
            httplib::SSLClient cli(host);
            cli.set_connection_timeout(10, 0); // 10 seconds
            cli.set_read_timeout(30, 0);       // 30 seconds

            auto res = body ? cli.Post(path.c_str(), headers, *body, contentType.c_str())
                            : cli.Get(path.c_str(), headers);

            if (!res)
            {
                LOG_ERROR("HTTP request failed for URL: {}", url);
                return std::nullopt;
            }

            if (res->status < 200 || res->status >= 300)
            {
                LOG_ERROR("HTTP request to {} failed with status {}: {}", url, res->status, res->body);
                return std::nullopt;
            }

            try
            {
                result = nlohmann::json::parse(res->body);
            }
            catch (const nlohmann::json::parse_error &e)
            {
                LOG_ERROR("Failed to parse JSON response: {}", e.what());
                return std::nullopt;
            }
#else
            LOG_ERROR("HTTPS request failed: httplib was not compiled with SSL support for URL: {}", url);
            return std::nullopt;
#endif
        }
        else
        {
            httplib::Client cli(host);
            cli.set_connection_timeout(10, 0); // 10 seconds
            cli.set_read_timeout(30, 0);       // 30 seconds

            auto res = body ? cli.Post(path.c_str(), headers, *body, contentType.c_str())
                            : cli.Get(path.c_str(), headers);

            if (!res)
            {
                LOG_ERROR("HTTP request failed for URL: {}", url);
                return std::nullopt;
            }

            if (res->status < 200 || res->status >= 300)
            {
                LOG_ERROR("HTTP request to {} failed with status {}: {}", url, res->status, res->body);
                return std::nullopt;
            }

            try
            {
                result = nlohmann::json::parse(res->body);
            }
            catch (const nlohmann::json::parse_error &e)
            {
                LOG_ERROR("Failed to parse JSON response: {}", e.what());
                return std::nullopt;
            }
        }

        return result;
    }
};

/**
 * @brief Namespace containing agent SDK functionality
 */
namespace agent_sdk
{
    /**
     * @brief Abstract base class for all tools that can be used by the agent
     *
     * Tools provide additional capabilities to LLM agents, allowing them to perform
     * specific actions or retrieve information from external sources.
     */
    class Tool
    {
    public:
        virtual ~Tool() = default;

        /**
         * @brief Get the name of the tool
         * @return String identifier for the tool
         */
        virtual std::string getName() const = 0;

        /**
         * @brief Get a human-readable description of what this tool does
         * @return Description string explaining the tool's purpose
         */
        virtual std::string getDescription() const = 0;

        /**
         * @brief Get the JSON schema for this tool's parameters
         * @return JSON object describing the expected parameters format
         */
        virtual nlohmann::json getSchema() const = 0;

        /**
         * @brief Execute the tool with the given parameters
         * @param parameters JSON object containing the tool parameters
         * @return JSON object containing the tool execution result
         */
        virtual nlohmann::json execute(const nlohmann::json &parameters) = 0;
    };

    /**
     * @brief Enum representing the available LLM providers
     */
    enum class LlmProvider
    {
        OLLAMA,    /**< Local Ollama provider */
        OPENROUTER /**< OpenRouter API provider */
    };

    /**
     * @brief Main ReAct agent class that provides LLM-powered conversational capabilities with tool integration
     *
     * The Agent class enables interaction with large language models using the Reasoning + Acting (ReAct) pattern,
     * supporting both streaming and non-streaming responses. It can execute tools as requested by the LLM and
     * maintain conversation history for multiple users.
     *
     * @note This class is not thread-safe across different users. Use separate Agent instances for different users
     *       or ensure proper synchronization when sharing across users.
     * @warning Maximum iteration limit is 10 to prevent infinite loops. Complex multi-step tasks may exceed this limit.
     * @see Tool for implementing custom tools
     * @see Builder for fluent construction pattern
     *
     * **Basic Usage Example:**
     * @code{.cpp}
     * // Create agent with Ollama
     * std::vector<std::shared_ptr<agent_sdk::Tool>> tools =  your_tools };
     * agent_sdk::Agent agent(
     *     agent_sdk::LlmProvider::OLLAMA,
     *     "llama3.2",
     *     "You are a helpful assistant.",
     *     tools
     * );
     *
     * // Process a message
     * auto response = agent.processMessage("user123", "What's the weather like?");
     *
     * // Streaming response
     * agent.processMessageStream("user123", "Plan a vacation", [](const auto& chunk) {
     *     switch(chunk.type) {
     *         case agent_sdk::Agent::StreamChunk::THINKING:
     *             std::cout << "Thinking: " << chunk.data["content"] << std::endl;
     *             break;
     *         case agent_sdk::Agent::StreamChunk::ASSISTANT_MESSAGE:
     *             std::cout << "Assistant: " << chunk.data["content"] << std::endl;
     *             break;
     *         // Handle other chunk types...
     *     }
     * });
     * @endcode
     *
     * **Builder Pattern Usage:**
     * @code{.cpp}
     * auto agent = agent_sdk::Agent::Builder()
     *     .provider(agent_sdk::LlmProvider::OPENROUTER)
     *     .model("gpt-4")
     *     .systemPrompt("You are a helpful assistant with access to tools.")
     *     .tools(myTools)
     *     .apiKey("your-api-key")
     *     .build();
     * @endcode
     *
     * **JSON Configuration:**
     * @code{.cpp}
     * nlohmann::json config = {
     *     {"provider", "OPENROUTER"},
     *     {"model", "claude-3-sonnet"},
     *     {"system_prompt", "You are a specialized agent."},
     *     {"api_key", "your-key-here"}
     * };
     * auto agent = agent_sdk::Agent(config, tools);
     * @endcode
     */
    class Agent
    {
    public:
        /**
         * @brief Builder class for constructing Agent instances with fluent interface
         *
         * Provides a clean, readable way to construct Agent instances with many optional parameters.
         * All builder methods return *this for method chaining.
         *
         * @see Agent::Builder::build() for final construction
         */
        class Builder;

        /**
         * @brief Structure representing different types of streaming data chunks
         */
        struct StreamChunk
        {
            /**
             * @brief Enum defining the types of streaming data chunks
             */
            enum Type
            {
                THINKING,          /**< LLM reasoning/thinking content */
                TOOL_CALL,         /**< Tool execution request */
                TOOL_RESULT,       /**< Result of tool execution */
                ASSISTANT_MESSAGE, /**< Regular assistant response message */
                ERROR,             /**< Error information */
                DONE               /**< End of stream indicator */
            };

            Type type;           /**< The type of this chunk */
            nlohmann::json data; /**< The data payload for this chunk */
        };

        /**
         * @brief Construct an Agent with all parameters specified individually
         * @param provider The LLM provider to use (Ollama or OpenRouter)
         * @param model The model name/version to use with the provider
         * @param system_prompt The system prompt that defines the agent's behavior
         * @param tools Vector of tool instances the agent can use
         * @param api_key API key for the provider (if required)
         */
        Agent(
            LlmProvider provider,
            std::string model,
            std::string system_prompt,
            std::vector<std::shared_ptr<Tool>> tools,
            std::string api_key = "");

        /**
         * @brief Construct an Agent from a JSON configuration object
         * @param config JSON object containing agent configuration with keys:
         *               "provider", "model", "system_prompt", "api_key" (optional)
         * @param tools Vector of tool instances the agent can use
         */
        Agent(const nlohmann::json &config, std::vector<std::shared_ptr<Tool>> tools);

        ~Agent() = default;

        Agent(Agent &&) noexcept = default;

        Agent &operator=(Agent &&) noexcept = default;

        Agent(const Agent &) = delete;

        Agent &operator=(const Agent &) = delete;

        /**
         * @brief Process a user message and return the complete response (non-streaming)
         * @param userId Unique identifier for the user/conversation
         * @param message The user's message
         * @return JSON array containing all response chunks (thinking, tool calls, results, messages, etc.)
         */
        nlohmann::json processMessage(const std::string &userId, const std::string &message);

        /**
         * @brief Process a user message with streaming response
         * @param userId Unique identifier for the user/conversation
         * @param message The user's message
         * @param callback Function called for each streaming chunk as it's generated
         */
        void processMessageStream(
            const std::string &userId,
            const std::string &message,
            std::function<void(const StreamChunk &)> callback);

    private:
        class AgentImpl;
        std::unique_ptr<AgentImpl> pImpl;
    };

    class Agent::Builder
    {
    public:
        Builder &provider(LlmProvider provider)
        {
            provider_ = provider;
            return *this;
        }

        Builder &model(std::string model)
        {
            model_ = std::move(model);
            return *this;
        }

        Builder &systemPrompt(std::string system_prompt)
        {
            system_prompt_ = std::move(system_prompt);
            return *this;
        }

        Builder &tools(std::vector<std::shared_ptr<Tool>> tools)
        {
            tools_ = std::move(tools);
            return *this;
        }

        Builder &apiKey(std::string api_key)
        {
            api_key_ = std::move(api_key);
            return *this;
        }

        Agent build()
        {
            return Agent(
                provider_,
                std::move(model_),
                std::move(system_prompt_),
                std::move(tools_),
                std::move(api_key_));
        }

    private:
        LlmProvider provider_;
        std::string model_;
        std::string system_prompt_;
        std::vector<std::shared_ptr<Tool>> tools_;
        std::string api_key_;
    };

    class Agent::AgentImpl
    {
    public:
        using StreamCallback = std::function<void(const Agent::StreamChunk &)>;

        AgentImpl(
            LlmProvider provider,
            std::string model,
            std::string system_prompt,
            std::vector<std::shared_ptr<Tool>> tools,
            std::string api_key)
            : provider_(provider),
              model_(std::move(model)),
              system_prompt_(std::move(system_prompt)),
              api_key_(std::move(api_key)),
              httpClient_(std::make_unique<HttpClient>())
        {

            for (const auto &tool : tools)
            {
                tools_map_[tool->getName()] = tool;
            }

            tool_schemas_ = nlohmann::json::array();
            for (const auto &tool_pair : tools_map_)
            {
                tool_schemas_.push_back(tool_pair.second->getSchema());
            }
        }

        void processMessageStream(const std::string &userId, const std::string &message, StreamCallback callback)
        {
            try
            {
                if (conversationHistories_[userId].empty())
                {
                    conversationHistories_[userId].push_back({{"role", "system"}, {"content", system_prompt_}});
                }
                conversationHistories_[userId].push_back({{"role", "user"}, {"content", message}});
                runAgentIteration(userId, callback);
            }
            catch (const std::exception &e)
            {
                LOG_ERROR("Unhandled exception in processMessageStream: {}", e.what());
                callback({Agent::StreamChunk::ERROR, {{"message", "An internal error occurred."}}});
                callback({Agent::StreamChunk::DONE, {}});
            }
        }

        nlohmann::json processMessage(const std::string &userId, const std::string &message)
        {
            nlohmann::json final_response = nlohmann::json::array();

            processMessageStream(userId, message, [&final_response](const Agent::StreamChunk &chunk)
                                 {
            nlohmann::json chunk_json;
            switch(chunk.type) {
                case Agent::StreamChunk::THINKING:
                    chunk_json = {{"type", "thinking"}, {"content", chunk.data["content"]}};
                    break;
                case Agent::StreamChunk::TOOL_CALL:
                    chunk_json = {{"type", "tool_call"}, {"tool_name", chunk.data["tool_name"]}, {"arguments", chunk.data["arguments"]}};
                    if (chunk.data.contains("tool_call_id"))
                        chunk_json["tool_call_id"] = chunk.data["tool_call_id"];
                    break;
                case Agent::StreamChunk::TOOL_RESULT:
                    chunk_json = {{"type", "tool_result"}, {"tool_name", chunk.data["tool_name"]}, {"result", chunk.data["result"]}};
                    if (chunk.data.contains("tool_call_id"))
                        chunk_json["tool_call_id"] = chunk.data["tool_call_id"];
                    break;
                case Agent::StreamChunk::ASSISTANT_MESSAGE:
                    chunk_json = {{"type", "assistant"}, {"content", chunk.data["content"]}};
                    break;
                case Agent::StreamChunk::ERROR:
                    chunk_json = {{"type", "error"}, {"message", chunk.data["message"]}};
                    break;
                case Agent::StreamChunk::DONE:
                    chunk_json = {{"type", "done"}};
                    break;
            }
            final_response.push_back(chunk_json); });

            return final_response;
        }

    private:
        struct ToolCallInfo
        {
            std::string id;
            std::string name;
            std::string args;
        };

        void runAgentIteration(const std::string &userId, StreamCallback callback, int iteration = 0)
        {
            if (iteration >= 10)
            {
                callback({Agent::StreamChunk::ERROR, {{"message", "Agent reached maximum iterations."}}});
                return;
            }

            LOG_INFO("Agent iteration {}/{}", iteration + 1, 10);

            bool tool_called = false;
            if (provider_ == LlmProvider::OPENROUTER)
            {
                tool_called = makeOpenRouterRequestStream(conversationHistories_[userId], callback);
            }
            else
            {
                tool_called = makeOllamaRequestStream(conversationHistories_[userId], callback);
            }

            if (tool_called)
            {
                runAgentIteration(userId, callback, iteration + 1);
            }
            else
            {
                callback({Agent::StreamChunk::DONE, {}});
            }
        }

        bool makeOpenRouterRequestStream(std::vector<nlohmann::json> &messages, StreamCallback callback);
        bool makeOllamaRequestStream(std::vector<nlohmann::json> &messages, StreamCallback callback);
        bool processToolCalls(
            std::vector<nlohmann::json> &messages,
            const std::vector<ToolCallInfo> &tool_calls_info,
            bool tool_was_called,
            const std::string &accumulated_content,
            StreamCallback callback);

        std::string system_prompt_;
        std::map<std::string, std::shared_ptr<Tool>> tools_map_;
        nlohmann::json tool_schemas_;
        std::map<std::string, std::vector<nlohmann::json>> conversationHistories_;
        std::unique_ptr<HttpClient> httpClient_;
        std::string api_key_;
        std::string model_;
        LlmProvider provider_;
    };

    inline Agent::Agent(
        LlmProvider provider,
        std::string model,
        std::string system_prompt,
        std::vector<std::shared_ptr<Tool>> tools,
        std::string api_key) : pImpl(std::make_unique<AgentImpl>(provider,
                                                                 std::move(model),
                                                                 std::move(system_prompt),
                                                                 std::move(tools),
                                                                 std::move(api_key))) {}

    inline Agent::Agent(const nlohmann::json &config, std::vector<std::shared_ptr<Tool>> tools)
    {
        std::string provider_str = config.value("provider", "OLLAMA");
        LlmProvider provider;
        if (provider_str == "OPENROUTER")
        {
            provider = LlmProvider::OPENROUTER;
        }
        else
        {
            provider = LlmProvider::OLLAMA;
        }

        std::string model = config.at("model");
        std::string system_prompt = config.at("system_prompt");
        std::string api_key = config.value("api_key", "");

        pImpl = std::make_unique<AgentImpl>(provider,
                                            std::move(model),
                                            std::move(system_prompt),
                                            std::move(tools),
                                            std::move(api_key));
    }

    inline nlohmann::json Agent::processMessage(const std::string &userId, const std::string &message)
    {
        return pImpl->processMessage(userId, message);
    }

    inline void Agent::processMessageStream(
        const std::string &userId,
        const std::string &message,
        std::function<void(const StreamChunk &)> callback)
    {
        pImpl->processMessageStream(userId, message, callback);
    }

    inline bool Agent::AgentImpl::makeOpenRouterRequestStream(std::vector<nlohmann::json> &messages, StreamCallback callback)
    {
        if (api_key_.empty())
        {
            LOG_ERROR("API_KEY is not set.");
            callback({Agent::StreamChunk::ERROR, {{"message", "API_KEY is not set."}}});
            return false;
        }

        nlohmann::json requestBody;
        requestBody["model"] = model_;
        requestBody["messages"] = messages;
        requestBody["tools"] = tool_schemas_;
        requestBody["stream"] = true;
        requestBody["reasoning"] = {{"enabled", true}};

        std::map<std::string, std::string> headers;
        headers["Authorization"] = "Bearer " + api_key_;

        bool tool_was_called = false;
        bool error_occurred = false;
        std::string accumulated_content;
        std::vector<ToolCallInfo> tool_calls_info;

        bool success = httpClient_->postStream(
            "https://openrouter.ai/api/v1/chat/completions",
            requestBody,
            headers,
            [&](const std::string &line)
            {
                if (error_occurred)
                    return;

                if (line.rfind("data: ", 0) != 0)
                {
                    // This might be a full error JSON response
                    try
                    {
                        nlohmann::json chunk = nlohmann::json::parse(line);
                        if (chunk.contains("error"))
                        {
                            error_occurred = true;
                            auto error_json = chunk["error"];
                            std::string error_message = error_json.value("message", "Unknown OpenRouter stream error.");
                            int error_code = error_json.value("code", 0);

                            if (error_code == 429)
                            {
                                error_message = "Rate limit exceeded.";
                            }

                            LOG_ERROR("OpenRouter stream error with code {}: {}", error_code, error_message);
                            callback({Agent::StreamChunk::ERROR, {{"message", error_message}}});
                        }
                    }
                    catch (const nlohmann::json::parse_error &e)
                    {
                        // Not a JSON error, and not an SSE line. Ignore.
                    }
                    return;
                }

                std::string jsonStr = line.substr(6);
                if (jsonStr == "[DONE]")
                    return;

                try
                {
                    nlohmann::json chunk = nlohmann::json::parse(jsonStr);

                    if (chunk.contains("error"))
                    {
                        error_occurred = true;
                        std::string error_message = chunk["error"].value("message", "Unknown OpenRouter stream error.");
                        LOG_ERROR("OpenRouter stream error: {}", error_message);
                        callback({Agent::StreamChunk::ERROR, {{"message", error_message}}});
                        return;
                    }

                    if (!chunk.contains("choices") || chunk["choices"].empty())
                        return;

                    auto &choice = chunk["choices"][0];
                    auto &delta = choice["delta"];

                    // Handle reasoning (thinking)
                    if (delta.contains("reasoning") && !delta["reasoning"].is_null() && delta["reasoning"].is_string())
                    {
                        std::string reasoning = delta["reasoning"].get<std::string>();
                        if (!reasoning.empty())
                        {
                            callback({Agent::StreamChunk::THINKING, {{"content", reasoning}}});
                        }
                    }

                    // Handle assistant messsage (content)
                    if (delta.contains("content") && !delta["content"].is_null() && delta["content"].is_string())
                    {
                        std::string content = delta["content"].get<std::string>();
                        if (!content.empty())
                        {
                            accumulated_content += content;
                            callback({Agent::StreamChunk::ASSISTANT_MESSAGE, {{"content", content}}});
                        }
                    }

                    // Handle tool calls
                    if (delta.contains("tool_calls") && delta["tool_calls"].is_array())
                    {
                        for (const auto &tool_call_delta : delta["tool_calls"])
                        {
                            size_t index = tool_call_delta.value("index", 0);
                            if (tool_calls_info.size() <= index)
                            {
                                tool_calls_info.resize(index + 1);
                            }

                            if (tool_call_delta.contains("id") && !tool_call_delta["id"].is_null() && tool_call_delta["id"].is_string())
                            {
                                tool_calls_info[index].id = tool_call_delta["id"].get<std::string>();
                            }
                            if (tool_call_delta.contains("function") && !tool_call_delta["function"].is_null())
                            {
                                auto &func = tool_call_delta["function"];
                                if (func.contains("name") && !func["name"].is_null() && func["name"].is_string())
                                {
                                    tool_calls_info[index].name += func["name"].get<std::string>();
                                }
                                if (func.contains("arguments") && !func["arguments"].is_null() && func["arguments"].is_string())
                                {
                                    tool_calls_info[index].args += func["arguments"].get<std::string>();
                                }
                            }
                        }
                    }

                    if (choice.contains("finish_reason") && !choice["finish_reason"].is_null())
                    {
                        std::string finish_reason = choice["finish_reason"].get<std::string>();
                        if (finish_reason == "tool_calls")
                        {
                            tool_was_called = true;
                        }
                        else if (finish_reason == "error")
                        {
                            error_occurred = true;
                            LOG_ERROR("OpenRouter returned finish_reason: error");
                            callback({Agent::StreamChunk::ERROR, {{"message", "OpenRouter encountered an error."}}});
                            // Error message is usually in the same chunk or a preceding one.
                            // The logic at the top of the lambda should have already caught it.
                        }
                    }
                }
                catch (const std::exception &e)
                {
                    LOG_ERROR("Failed to parse stream chunk: {}", e.what());
                }
            });

        if (!success)
        {
            callback({Agent::StreamChunk::ERROR, {{"message", "Failed to stream from OpenRouter"}}});
            return false;
        }

        if (error_occurred)
            return false;

        return processToolCalls(messages, tool_calls_info, tool_was_called, accumulated_content, callback);
    }

    inline bool Agent::AgentImpl::makeOllamaRequestStream(std::vector<nlohmann::json> &messages, StreamCallback callback)
    {
        nlohmann::json requestBody;
        requestBody["model"] = model_;
        requestBody["messages"] = messages;
        requestBody["stream"] = true;
        requestBody["tools"] = tool_schemas_;
        requestBody["think"] = true;

        bool tool_was_called = false;
        bool error_occurred = false;
        bool is_thinking = false;
        std::string accumulated_content;
        std::vector<ToolCallInfo> tool_calls_info;

        bool success = httpClient_->postStream(
            "http://localhost:11434/api/chat",
            requestBody,
            {},
            [&](const std::string &line)
            {
                if (error_occurred)
                    return;

                try
                {
                    nlohmann::json chunk = nlohmann::json::parse(line);

                    if (chunk.contains("error"))
                    {
                        error_occurred = true;
                        std::string error_message = chunk.value("error", "Unknown Ollama stream error.");
                        LOG_ERROR("Ollama stream error: {}", error_message);
                        callback({Agent::StreamChunk::ERROR, {{"message", error_message}}});
                        return;
                    }

                    if (chunk.contains("message"))
                    {
                        auto &message = chunk["message"];

                        // Handle assistant message (content)
                        if (message.contains("content") && !message["content"].is_null())
                        {
                            std::string content = message["content"].get<std::string>();
                            if (!content.empty())
                            {
                                if (!is_thinking)
                                {
                                    if (content == "<think>")
                                    {
                                        is_thinking = true;
                                    }
                                    else
                                    {
                                        accumulated_content += content;
                                        callback({Agent::StreamChunk::ASSISTANT_MESSAGE, {{"content", content}}});
                                    }
                                }
                                else
                                {
                                    if (content == "</think>")
                                    {
                                        is_thinking = false;
                                    }
                                    else
                                    {
                                        callback({Agent::StreamChunk::THINKING, {{"content", content}}});
                                    }
                                }
                            }
                        }

                        // Handle reasoning (thinking)
                        if (message.contains("thinking") && !message["thinking"].is_null())
                        {
                            std::string reasoning = message["thinking"].get<std::string>();
                            if (!reasoning.empty())
                            {
                                callback({Agent::StreamChunk::THINKING, {{"content", reasoning}}});
                            }
                        }

                        // Handle tool calls
                        if (message.contains("tool_calls") && message["tool_calls"].is_array())
                        {
                            for (const auto &tool_call : message["tool_calls"])
                            {
                                if (tool_call.contains("function") && !tool_call["function"].is_null())
                                {
                                    auto &function = tool_call["function"];

                                    if (!function.contains("name") || function["name"].is_null())
                                    {
                                        LOG_WARN("Skipping tool call with no name");
                                        continue;
                                    }

                                    ToolCallInfo info;
                                    info.name = function["name"].get<std::string>();

                                    if (function.contains("arguments") && !function["arguments"].is_null())
                                    {
                                        auto &args = function["arguments"];
                                        if (args.is_string())
                                        {
                                            info.args = args.get<std::string>();
                                        }
                                        else if (args.is_object() || args.is_array())
                                        {
                                            info.args = args.dump();
                                        }
                                        else
                                        {
                                            info.args = "{}";
                                        }
                                    }
                                    else
                                    {
                                        info.args = "{}";
                                    }

                                    info.id = "ollama_" + info.name + "_" + std::to_string(tool_calls_info.size());

                                    tool_calls_info.push_back(info);
                                    tool_was_called = true;

                                    LOG_DEBUG("Ollama tool call detected: {} with args: {}", info.name, info.args);
                                }
                            }
                        }
                    }

                    if (chunk.value("done", false))
                    {
                        if (chunk.contains("message") && chunk["message"].contains("tool_calls"))
                        {
                            tool_was_called = true;
                        }
                    }
                }
                catch (const std::exception &e)
                {
                    LOG_ERROR("Failed to parse Ollama stream chunk: {}", e.what());
                }
            });

        if (!success)
        {
            callback({Agent::StreamChunk::ERROR, {{"message", "Failed to stream from Ollama"}}});
            return false;
        }

        if (error_occurred)
            return false;

        return processToolCalls(messages, tool_calls_info, tool_was_called, accumulated_content, callback);
    }

    inline bool Agent::AgentImpl::processToolCalls(
        std::vector<nlohmann::json> &messages,
        const std::vector<ToolCallInfo> &tool_calls_info,
        bool tool_was_called,
        const std::string &accumulated_content,
        StreamCallback callback)
    {
        if (tool_was_called && !tool_calls_info.empty())
        {
            nlohmann::json assistant_message = {{"role", "assistant"}};

            if (!accumulated_content.empty())
            {
                assistant_message["content"] = accumulated_content;
            }
            else
            {
                assistant_message["content"] = nlohmann::json();
            }

            nlohmann::json tool_calls_json = nlohmann::json::array();

            for (const auto &info : tool_calls_info)
            {
                if (info.name.empty())
                {
                    LOG_WARN("Skipping tool call with empty name");
                    continue;
                }

                nlohmann::json tool_call_entry;
                std::string args_str = info.args.empty() ? "{}" : info.args;

                if (provider_ == LlmProvider::OPENROUTER)
                {
                    tool_call_entry = {
                        {"type", "function"},
                        {"function", {{"name", info.name}, {"arguments", args_str}}}};
                }
                else
                {
                    tool_call_entry = {
                        {"type", "function"},
                        {"function", {{"name", info.name}, {"arguments", nlohmann::json::parse(args_str)}}}};
                }

                if (!info.id.empty())
                {
                    tool_call_entry["id"] = info.id;
                }

                tool_calls_json.push_back(tool_call_entry);
            }

            assistant_message["tool_calls"] = tool_calls_json;
            messages.push_back(assistant_message);

            for (const auto &info : tool_calls_info)
            {
                if (info.name.empty())
                {
                    continue;
                }

                try
                {
                    nlohmann::json parsed_args;

                    if (info.args.empty())
                    {
                        parsed_args = nlohmann::json::object();
                    }
                    else
                    {
                        try
                        {
                            parsed_args = nlohmann::json::parse(info.args);
                        }
                        catch (const std::exception &e)
                        {
                            LOG_WARN("Failed to parse tool arguments '{}': {}. Using empty object.", info.args, e.what());
                            parsed_args = nlohmann::json::object();
                        }
                    }

                    nlohmann::json callback_data = {{"tool_name", info.name}, {"arguments", parsed_args}};
                    if (!info.id.empty())
                    {
                        callback_data["tool_call_id"] = info.id;
                    }
                    callback({Agent::StreamChunk::TOOL_CALL, callback_data});

                    if (tools_map_.count(info.name))
                    {
                        nlohmann::json tool_result = tools_map_[info.name]->execute(parsed_args);

                        nlohmann::json result_data = {{"tool_name", info.name}, {"result", tool_result}};
                        if (!info.id.empty())
                        {
                            result_data["tool_call_id"] = info.id;
                        }
                        callback({Agent::StreamChunk::TOOL_RESULT, result_data});

                        std::string result_content;
                        if (tool_result.is_null())
                        {
                            result_content = "null";
                        }
                        else if (tool_result.is_string())
                        {
                            result_content = tool_result.get<std::string>();
                        }
                        else
                        {
                            result_content = tool_result.dump();
                        }

                        nlohmann::json tool_message = {{"role", "tool"}, {"content", result_content}};
                        if (!info.id.empty())
                        {
                            tool_message["tool_call_id"] = info.id;
                        }
                        messages.push_back(tool_message);
                    }
                    else
                    {
                        LOG_ERROR("Unknown tool: {}", info.name);
                        callback({Agent::StreamChunk::ERROR, {{"message", "Unknown tool: " + info.name}}});

                        nlohmann::json tool_message = {{"role", "tool"}, {"content", "Error: Unknown tool"}};
                        if (!info.id.empty())
                        {
                            tool_message["tool_call_id"] = info.id;
                        }
                        messages.push_back(tool_message);
                    }
                }
                catch (const std::exception &e)
                {
                    std::string error_msg = "Failed to execute tool: " + std::string(e.what());
                    LOG_ERROR(error_msg);
                    callback({Agent::StreamChunk::ERROR, {{"message", error_msg}}});

                    nlohmann::json tool_message = {{"role", "tool"}, {"content", error_msg}};
                    if (!info.id.empty())
                    {
                        tool_message["tool_call_id"] = info.id;
                    }
                    messages.push_back(tool_message);
                }
            }

            return true;
        }
        else if (!accumulated_content.empty())
        {
            messages.push_back({{"role", "assistant"}, {"content", accumulated_content}});
        }

        return false;
    }

} // namespace agent_sdk
