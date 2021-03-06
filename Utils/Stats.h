#pragma once

#include <string>
#include <chrono>

#include "Lockable.h"

class Stats: public Lockable
{
    std::chrono::system_clock::time_point started;
    uint64_t messagesReceived;
    uint64_t messagesDropped;
    uint64_t bytesReceived;
    uint64_t bytesDropped;
    uint64_t messagesSent;
    uint64_t bytesSent;

public:
    Stats();

    void setStarted(std::chrono::system_clock::time_point started);
    void addPacketsReceived(uint64_t count);
    void addPacketsDropped(uint64_t count);
    void addBytesReceived(uint64_t count);
    void addBytesDropped(uint64_t count);
    void addPacketsSent(uint64_t count);
    void addBytesSent(uint64_t count);

    std::string toLog() const;
};


