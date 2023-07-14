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

#include "Message.h"

class Tickable
{
    uint64_t last_tick_time = 0;

public:
    virtual void tick(const small_world::SM_Event &event)
    {
        last_tick_time = std::stoull(event.event_time());
    }
};

class StateMachine;
class TimedState;


class RobotState
{
protected:
    std::string state_name = "";
    uint64_t initial_time = 0;
    uint64_t current_time = 0;
    std::map<std::string, std::shared_ptr<RobotState>> next_states;
    std::shared_ptr<StateMachine> owner;

public:
    uint64_t get_elapsed()
    {
        return (current_time - initial_time);
    }
    void set_next_state(const std::string &state_name, std::shared_ptr<RobotState> state)
    {
        next_states[state_name] = state;
    }
    virtual std::string get_state_name() const
    {
        return state_name;
    }

    std::shared_ptr<RobotState> get_next_state(const std::string &transition_name)
    {
         //for(auto const& pair : next_states) {
         //std::cout << "{ " << pair.first << " : " << pair.second->get_state_name() << " }" << std::endl;
         //}
        std::map<std::string, std::shared_ptr<RobotState>>::iterator it = next_states.find(transition_name);
        if (it == next_states.end())
            return nullptr;
        else
            return it->second;
    }

    virtual void tick(const small_world::SM_Event &event)
    {
        if (initial_time == 0)
            initial_time = std::stoull(event.event_time());
        current_time = std::stoull(event.event_time());
        // std::cout << "Current time: " << current_time << ", initial time: " << initial_time << std::endl;
        decide_action(get_elapsed());
    }

    virtual void decide_action(uint64_t elapsed) = 0;
};

class StateMachine : public Tickable
{
    std::shared_ptr<RobotState> current_state;

public:
    virtual void tick(const small_world::SM_Event &event)
    {
        Tickable::tick(event);
        if (current_state != nullptr)
            current_state->tick(event);
    }
    virtual void set_current_state(std::shared_ptr<RobotState> cs)
    {
        current_state = cs;
    }
};

class TimedState : public RobotState
{
    std::string verb_name;
    std::string robot_began_waiting = "The robot began waiting";
    std::string robot_waiting = "The robot is waiting";
    std::string robot_finished_waiting = "The robot finished waiting";

    std::string robot_began_moving = "The robot began moving";
    std::string robot_moving = "The robot is moving";
    std::string robot_finished_moving = "The robot finished moving";

public:
    uint64_t time_to_wait = 3000;
    void set_time_to_wait(uint64_t time)
    {
        time_to_wait = time;
    }
    void set_owner(std::shared_ptr<StateMachine> own)
    {
        owner = own;
    }
    void set_state_name(const std::string &name)
    {
        state_name = name;
    }
    virtual void set_verb_name(const std::string &name)
    {
        verb_name = name;
    }
    virtual void decide_action(uint64_t duration)
    {
        if (duration < time_to_wait)
        {
            std::cout << verb_name << std::endl;
            return;
        }
        std::shared_ptr<RobotState> next = get_next_state("done");

        if (next == nullptr)
        {
            std::cout << "\nCan't get a next state to go to" << std::endl;
            return;
        }
        else
        {
            if(verb_name == robot_moving){
                std::cout << robot_finished_moving << std::endl;
                std::cout << robot_began_waiting << std::endl;
                std::cout << "MOVED TO STATE: " << next->get_state_name() << std::endl;
                owner->set_current_state(next);
            }
            if(verb_name == robot_waiting){
                std::cout << robot_finished_waiting << std::endl;
                std::cout << robot_began_moving << std::endl;
                std::cout << "MOVED TO STATE: " << next->get_state_name() << std::endl;
                owner->set_current_state(next);
            }

        }
    }
};
