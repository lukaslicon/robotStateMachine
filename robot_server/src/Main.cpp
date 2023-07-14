
// server
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <map>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#include "stateMachine.h"

#define PORT 8080

int main(int argc, char const *argv[])
{

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << ("socket failure");
        return -1;
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        std::cout << ("setsockopt failure");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cout << ("bind failure") << std::endl;
        return -1;
    }

    if (listen(server_fd, 3) < 0)
    {
        std::cout << ("listen failure") << std::endl;
        return -1;
    }

    std::string robot_began_waiting = "The robot began waiting";
    std::string robot_waiting = "The robot is waiting";
    std::string robot_finished_waiting = "The robot finished waiting";

    std::string robot_began_moving = "The robot began moving";
    std::string robot_moving = "The robot is moving";
    std::string robot_finished_moving = "The robot finished moving";

    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            std::cout << ("accept failure") << std::endl;
            return -1;
        }
        std::cout << ("New client connected from port no %d and IP %s\n", ntohs(address.sin_port), inet_ntoa(address.sin_addr)) << std::endl;

        small_world::SM_Event event;
        std::shared_ptr<StateMachine> sm = std::make_shared<StateMachine>();
        std::shared_ptr<TimedState> s0 = std::make_shared<TimedState>();
        std::cout << robot_began_waiting << std::endl;
        s0->set_state_name(robot_waiting);
        s0->set_verb_name(robot_waiting);
        s0->set_owner(sm);
        std::shared_ptr<TimedState> s1 = std::make_shared<TimedState>();
        s1->set_state_name(robot_moving);
        s1->set_verb_name(robot_moving);
        s1->set_owner(sm);
        s0->set_next_state("done", s1);
        s1->set_next_state("done", s0);
        sm->set_current_state(s0);

        while (1)
        {
            memset(buffer, 0, sizeof(buffer));
            valread = read(new_socket, buffer, 1024);
            if (valread <= 0)
            {
                std::cout << ("\nClient Disconnected\n");
                break;
            }
            event.ParseFromString(buffer);
            sm->tick(event);
            std::cout << buffer << std::endl;
        }
    }
    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}
