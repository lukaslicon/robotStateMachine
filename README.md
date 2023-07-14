# robotStateMachine
- This assignment gave me a taste of what’s required to write an application which has functionality in
both a client and in a server and uses serialized messages to communicate across a TCP socket
connection.
- the robot server, receives the messages, deserializes them and implements a simple state machine to
make the robot alternate between sleeping and moving states; for each tick message received on the
server, it outputs one of the strings defined in the robot_server Main.cpp file depending on what state the
robot it in
