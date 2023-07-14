#include <chrono>
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <string.h> //strlen
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <thread>

#include "Message.h"

#define PORT 8080

uint64_t timeSinceEpochMillisec()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

int main(int argc, char const *argv[])
{
    int status, client_fd;
    struct sockaddr_in serv_addr;
    std::string ticker;
    small_world::SM_Event messageTicker;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // create socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "\n Socket creation error \n";
        return -1;
    }
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        std::cout << "\nInvalid address/ Address not supported \n";
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        std::cout << "\nConnection Failed \n";
        return -1;
    }

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for 500ms
        ticker = std::to_string(timeSinceEpochMillisec());
        messageTicker.set_event_type("tick");
        messageTicker.set_event_time(ticker);
        messageTicker.SerializeToString(&ticker);
        send(client_fd, ticker.c_str(), ticker.length(), 0);
        std::cout << "Tick sent to server" << std::endl;
    }
    // closing the connected socket
    close(client_fd);
    return 0;
}
