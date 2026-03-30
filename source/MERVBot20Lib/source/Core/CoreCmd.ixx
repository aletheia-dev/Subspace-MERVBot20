export module CoreCmd;

export import Command;

import Host;

import <string>;


//////// Commands ////////     

export void gotCommand(Host& host, Player& player, std::string_view msg);

export void gotRemoteCommand(Host& host, std::string_view playerName, std::string_view msg);
