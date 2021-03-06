#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>

#include "Shell.h"
#include "Text.h"
#include "../App.h"
#include "../Exceptions.h"
#include "../Network/Server.h"
#include "../Game/Game.h"

Shell::Shell(App &app)
    : input(std::cin),
      output(std::cout),
      app(app)
{}

void Shell::run()
{
    while (!shouldStop() && !input.eof()) {
        std::string command;
        getline(input, command);

        if (command.empty()) {
            continue;
        }

        try {
            handle(command);
        }
        catch (std::exception &exception) {
            output << "error: " << exception.what() << std::endl;
        }
    }
}

void Shell::after()
{
    app.stop(false);
    app.getLogger().log("shell stopped");
}

void Shell::handle(std::string line)
{
    // split command into arguments
    std::stringstream lineStream{line};
    std::vector<std::string> arguments{
        std::istream_iterator<std::string>{lineStream},
        std::istream_iterator<std::string>{}};

    if (arguments.empty()) {
        return;
    }

    std::string commandName{arguments[0]};

    auto found = Shell::commands.find(commandName);

    if (found == Shell::commands.end()) {
        throw UnknownCommandException("unknown command");
    }

    Command command = found->second;

    (this->*command)(arguments);
}

void Shell::cmdHelp(std::vector<std::string> arguments)
{
    output << std::endl;
    output << Text::hline() << std::endl;

    output << "Help:" << std::endl << std::endl;

    output << Text::justifyL("help", 10) << "- print help" << std::endl;
    output << Text::justifyL("info", 10) << "- print server info" << std::endl;
    output << Text::justifyL("stats", 10) << "- print server statistics" << std::endl;
    output << Text::justifyL("players", 10) << "- print list of players" << std::endl;
    output << Text::justifyL("games", 10) << "- print list of games" << std::endl;
    output << Text::justifyL("exit", 10) << "- stop the server and exit" << std::endl;

    output << Text::hline() << std::endl;
    output << std::endl;
}

void Shell::cmdInfo(std::vector<std::string> arguments)
{
    output << std::endl;
    output << Text::hline() << std::endl;
    output << "Info:" << std::endl << std::endl;
    output << app.getServer().toLog() << std::endl;
    output << Text::hline() << std::endl;
    output << std::endl;
}

void Shell::cmdStats(std::vector<std::string> arguments)
{
    output << std::endl;
    output << Text::hline() << std::endl;
    output << "Statistics:" << std::endl << std::endl;
    output << app.getStats().toLog() << std::endl;
    output << Text::hline() << std::endl;
    output << std::endl;
}

void Shell::cmdPlayers(std::vector<std::string> arguments)
{
    output << std::endl;
    output << Text::hline() << std::endl;

    output << "Players:" << std::endl << std::endl;

    size_t count = app.forEachConnection([this](Connection &connection) {
        try {
            output << connection.getUid() << ": " << app.getNickname(connection.getUid()) << std::endl;
        }
        catch (NoNicknameException &exception) {
            output << connection.getUid() << ": " << "[not logged]" << std::endl;
        }
    });

    if (count == 0) {
        output << "- no players active -" << std::endl;
    }

    output << Text::hline() << std::endl;
    output << std::endl;
}

void Shell::cmdGames(std::vector<std::string> arguments)
{
    output << std::endl;
    output << Text::hline() << std::endl;

    output << "Games:" << std::endl << std::endl;

    size_t count = app.forEachGame([this](Game &game) {
        std::string playerLeft = app.getNickname(game.getPlayerUid(Side::Left));
        std::string playerRight = app.getNickname(game.getPlayerUid(Side::Right));

        output
            << Text::justifyR(playerLeft, 16)
            << " vs. "
            << Text::justifyL(playerRight, 16)
            << std::endl;
    });

    if (count == 0) {
        output << "- no games active -" << std::endl;
    }

    output << Text::hline() << std::endl;
    output << std::endl;
}

void Shell::cmdExit(std::vector<std::string> arguments)
{
    stop(false);
    app.stop(false);
}
