#include <netinet/in.h>
#include <zconf.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <cstring>
#include <thread>
#include <iostream>

#include "Server.h"
#include "../Utils/Shell.h"

Server::Server(uint16_t port, std::string ipAddress)
    : connectionAcceptor{*this},
      connectionWatcher{*this},
      port{port},
      logger{"server.log", "communication.log", "stats.log"},
      shell{std::cin, std::cout, *this}
{
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (ipAddress.empty()) {
        address.sin_addr.s_addr = INADDR_ANY;
    }
    else {
        address.sin_addr.s_addr = inet_addr(ipAddress.c_str());

        if (address.sin_addr.s_addr == INADDR_NONE) {
            throw ServerException("server ip address is in wrong format");
        }
    }

    char ipString[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &(address.sin_addr), ipString, INET_ADDRSTRLEN);

    this->ipString = ipString;
}

void Server::run()
{
    stats.setStarted(std::chrono::system_clock::now());

    // periodically check connections states and remove them eventually
    while (!shouldStop()) {

        if (stats.hasChanged()) {
            logger.writeStats(stats);
        }


        std::this_thread::sleep_for(STATS_WRITE_PERIOD);
    }
}

Connection &Server::addConnection(int socket, sockaddr_in address)
{
    std::unique_lock<std::mutex> lock{connectionsMutex};

    connections.emplace_back(socket, address, *this);

    Connection &connection = connections.back();
    connection.start();

    return connections.back();
}

size_t Server::filterConnections(std::function<bool(Connection &)> filter)
{
    std::unique_lock<std::mutex> lock{connectionsMutex};

    size_t count{0};
    auto connectionPtr = connections.begin();

    while (connectionPtr != connections.end()) {
        if (filter(*connectionPtr)) {

            playersMutex.lock();
            if (connectionPtr->isIdentified()) {
                players.erase(connectionPtr->getNickname());
            }
            playersMutex.unlock();

            connectionPtr = connections.erase(connectionPtr);
            count++;
        }
        else {
            connectionPtr++;
        }
    }

    return count;
}

size_t Server::forEachConnection(std::function<void(Connection &)> function)
{
    std::unique_lock<std::mutex> lock{connectionsMutex};
    size_t count{0};

    for (auto &connection : connections) {
        function(connection);
        count++;
    }

    return count;
}

Stats &Server::getStats()
{
    return stats;
}

Logger &Server::getLogger()
{
    return logger;
}

sockaddr_in &Server::getAddress()
{
    return address;
}

bool Server::stop(bool wait)
{
    connectionAcceptor.stop(false);
    connectionWatcher.stop(false);
    shell.stop(false);
    return Thread::stop(wait);
}

void Server::before()
{
    logger.log("starting server");

    connectionAcceptor.start();
    connectionWatcher.start();
    shell.start();
}

void Server::after()
{
    shell.stop(false);
    connectionAcceptor.stop(true);
    connectionWatcher.stop(true);

    forEachConnection([](Connection &connection)
                      {
                          connection.stop(true);
                      });


    logger.log("server stopped", Logger::Level::Warning);
}

void Server::addPlayer(std::string nickname, Connection *connection)
{
    std::lock_guard<std::mutex> lock{playersMutex};

    auto inserted = players.insert(std::make_pair(nickname, connection));

    if (!inserted.second) {
        throw ServerException("Player nickname already exists");
    }
}

Connection &Server::getConnection(std::string nickname)
{
    auto found = players.find(nickname);

    if (found == players.end()) {
        throw ServerException("Player " + nickname + " does not exist");
    }

    return *found->second;
}
std::string Server::toLogString()
{
    std::string string;
    string += "Listening for the new connections on\n";
    string += "IPv4 address: " + ipString + "\n";
    string += "        port: " + std::to_string(port) + "\n";

    return string;
}
