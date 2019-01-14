#pragma once

#include <string>
#include <vector>
#include <istream>
#include <ostream>
#include <functional>
#include <unordered_map>
#include "Thread.h"

class Server;

class Shell: public Thread
{
    std::istream &input;
    std::ostream &output;
    Server &server;

    const std::string HLINE{"------------------------------------------------------------"};

    typedef void (Shell::*Command)(std::vector<std::string>);

    std::unordered_map<std::string, Command> actions{
        {"exit", &Shell::cmdExit},
        {"games", &Shell::cmdGames},
        {"players", &Shell::cmdPlayers},
        {"info", &Shell::cmdInfo},
        {"stats", &Shell::cmdStats},
        {"help", &Shell::cmdHelp},
    };

    void handle(std::string command);

    void cmdExit(std::vector<std::string> arguments);
    void cmdGames(std::vector<std::string> arguments);
    void cmdPlayers(std::vector<std::string> arguments);
    void cmdInfo(std::vector<std::string> arguments);
    void cmdStats(std::vector<std::string> arguments);
    void cmdHelp(std::vector<std::string> arguments);

public:
    Shell(const std::istream &input, const std::ostream &output, Server &server);
    void run() override;
    void after() override;
    bool stop(bool wait) override;
};

class UnknownCommandException: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class TooFewArgumentsException: public std::invalid_argument
{
    using std::invalid_argument::invalid_argument;
};

class BadArgumentException: public std::invalid_argument
{
    using std::invalid_argument::invalid_argument;
};