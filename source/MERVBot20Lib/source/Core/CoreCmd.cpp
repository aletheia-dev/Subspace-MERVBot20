module CoreCmd;

import BotDb;
import Command;
import Config;
import ClientProt;
import Host;
import System;

import <format>;
import <list>;
import <ranges>;


/// <summary>
/// Get the command info map.
/// </summary>
/// <param name="host">Host.</param>
/// <returns>Command info map.</returns>
CommandInfoMap& getCommandInfos(Host& host)
{
    static CommandInfoMap commandInfos({
        // level 5: Owner
        { "info", { { OperatorLevel::Owner, CommandScope::Local,
            "describe the bot state" } }
        },
        { "autosave", { { OperatorLevel::Owner, CommandScope::Local,
            "change the time between automatic database backups", { "<interval>" } } }
        },
        { "password", { { OperatorLevel::Owner, CommandScope::Local,
            "change or view the bot password", { "[bot password]" } } }
        },
        { "closeall", { { OperatorLevel::Owner, CommandScope::Local,
            "close all bot spawns" } }
        },
        { "save", { { OperatorLevel::Owner, CommandScope::Local,
            "manually backup the database" } }
        },
        { "read", { { OperatorLevel::Owner, CommandScope::Local,
            "manually re-read MERVBot.INI and Operators.txt" } }
        },
        { "set", { { OperatorLevel::Owner, CommandScope::Local,
            "change a value in MERVBot.INI", { "<section>:<key>:<new value>" } } }
        },
        { "get", { { OperatorLevel::Owner, CommandScope::Local,
            "retrieve INI value for a key", { "<section>:<key>" } } }
        },
        { "addalias", { { OperatorLevel::Owner, CommandScope::Local,
            "add a command alias", { "<command>:<new alias>" } } }
        },
        { "killalias", { { OperatorLevel::Owner, CommandScope::Local,
            "remove a command alias", { "<alias>" } } }
        },
        { "aliases", { { OperatorLevel::Owner, CommandScope::Local,
            "list all command aliases, or [just those belonging to a specific command]", { "[command]" } } }
        },
        { "log", { { OperatorLevel::Owner, CommandScope::Local,
            "list the last 5 log items, or [a specific number of items]", { "[maximum]" } } }
        },
        { "restart", { { OperatorLevel::Owner, CommandScope::Local,
            "re-logs all the bots that are currently in a zone" } }
        },

        // level 4: SysOp
        { "spawn", { { OperatorLevel::SysOp, CommandScope::Local,
            "spawn a new bot", { "<bot name>" }, { 
                { 'p', "<password>", "set name password" },
                { 's', "<password>", "set staff password" },
                { 'a', "<arena>", "set arena" },
                { 'i', "<dll name>", "set DLL import" }
            } } }
        },
        { "load", { { OperatorLevel::SysOp, CommandScope::Local,
            "load a plugin", { "<plugin filename>" } } }
        },
        { "unload", { { OperatorLevel::SysOp, CommandScope::Local,
            "drop plugin # from !plugins, ALL=unload all)", { "<plugin #>|ALL" } } }
        },
        { "plugins", { { OperatorLevel::SysOp, CommandScope::Local,
            "list loaded plugins" } }
        },
        { "close", { { OperatorLevel::SysOp, CommandScope::Remote,
            "close this bot spawn" } }
        },
        { "setbanner", { { OperatorLevel::SysOp, CommandScope::Local,
            "make the bot use your banner" } }
        },
        { "ownbot", { { OperatorLevel::SysOp, CommandScope::Local,
            "allow or disallow !own and !give system", { "[on|off]" } } }
        },
        { "error", { { OperatorLevel::SysOp, CommandScope::Local,
            "toggle error broadcasting", { "[on|off]" }, {
                { 'c', "<channel>", "create an error broadcast channel for mods" },
                { 'g', "", "generate a server warning message" }
            } } }
        },

        // level 3: SuperModerator
        { "addop", { { OperatorLevel::SuperModerator, CommandScope::Local,
            "add operator, takes effect when the op sends !login [password]", { "<name>" }, {
                { 'l', "<level>", "set operator level, see !help levels" },
                { 'p', "<password>", "set password, no password by default" }
            } } }
        },
        { "editop", { { OperatorLevel::SuperModerator, CommandScope::Local,
            "edit existing operator, takes effect when the op sends !login [password]", { "<name>" }, {
                { 'l', "<level>", "set operator level, see !help levels" },
                { 'p', "<password>", "set password, no password by default" }
            } } }
        },
        { "deleteop", { { OperatorLevel::SuperModerator, CommandScope::Local,
            "remove operator powers, takes effect after the ex-op leaves the zone", { "<name>" } } }
        },
        { "go", { { OperatorLevel::SuperModerator, CommandScope::Remote,
            "change the bot's arena", { "<arena name>|[#]<public arena number>" } } }
        },
        { "limit", { { OperatorLevel::SuperModerator, CommandScope::Local,
            "remove bot access below the given operator level", { "<numerical level>" } } }
        },
        { "uptime", { { OperatorLevel::SuperModerator, CommandScope::Local,
            "display time since the bot joined the zone and arena" } }
        },
        { "spawns", { { OperatorLevel::SuperModerator, CommandScope::Local,
            "list all bot spawns" } }
        },
        { "levels", { { OperatorLevel::SuperModerator, CommandScope::Local,
            "", {}, {},
            "0 Player                Only !listop\n"
            "1 Player(Limited)       Public, limited bot access\n"
            "2 Moderator(Mod)        Control over basic tools\n"
            "3 SuperModerator(SMod)  Manages Moderator accounts\n"
            "4 SysOp(SOp)            Control over advanced tools\n"
            "5 Owner(etc)            No restrictions"
            } }
        },

        // level 2: Moderator
        { "where", { { OperatorLevel::Moderator, CommandScope::Local,
            "return last known player coordinates", { "<player name>" } } }
        },
        { "version", { { OperatorLevel::Moderator, CommandScope::Remote,
            "display bot version" } }
        },
        { "logout", { { OperatorLevel::Moderator, CommandScope::Local,
            "release operator powers" } }
        },
        { "setlogin", { { OperatorLevel::Moderator, CommandScope::Local,
            "change personal login password", { "<new password>" } } }
        },
        { "zone", { { OperatorLevel::Moderator, CommandScope::Local,
            "broadcast message zone-wide w/ nametag", { "<message>" } } }
        },
        { "say", { { OperatorLevel::Moderator, CommandScope::Local,
            "make the bot say something, use %<num> to specify a sound code", { "<message>" }, {},
            "examples:\n"
            "!say //Team        (send a message to the team frequency)\n"
            "!say ;X;Channel    (send a message to a specified channel)\n"
            "!say :Name:Remote  (send a message to a remote player)" } }
        },
        { "chat", { { OperatorLevel::Moderator, CommandScope::Local,
            "set bot chat(s)", { "<channel[,channel...]>" } } }
        },
        { "listchat", { { OperatorLevel::Moderator, CommandScope::Local,
            "lists all chat channels the bot is currently on, so you don't have to guess" } }
        },
        { "clearchat", { { OperatorLevel::Moderator, CommandScope::Local,
            "clears all chat channels, so the bot is no longer on any chat channels" } }
        },
        { "attach", { { OperatorLevel::Moderator, CommandScope::Local,
            "make the bot attach to a player", { "<player name>" } } }
        },
        { "follow", { { OperatorLevel::Moderator, CommandScope::Local,
            "make the bot follow a player", { "<player name>" } } }
        },
        { "team", { { OperatorLevel::Moderator, CommandScope::Local,
            "", { "<team number>|<player name>" }, {},
            "alternatives:\n"
            "!team <team number>  (make bot change teams)\n"
            "!team <player name>  (return the team of a player)" } }
        },
        { "spec", { { OperatorLevel::Moderator, CommandScope::Local,
            "make the bot spectate the arena" } }
        },
        { "ship", { { OperatorLevel::Moderator, CommandScope::Local,
            "make bot change ships", { "<ship type>" } } }
        },
        { "turret", { { OperatorLevel::Moderator, CommandScope::Local,
            "mode: 1=fires on team command, 0=shoots enemies, none=invert", { "[1|0]" } } }
        },
        { "awarp", { { OperatorLevel::Moderator, CommandScope::Local,
            "turn on/off bot's antiwarp, none=invert", { "[1|0]" } } }
        },

        // level 1/0: Limited, Player
        { "help", { { OperatorLevel::Player, CommandScope::Remote,
            "provide an overview of available commands, help on specific commands or user levels", 
            { "[<command>|<level>]" }, {},
            "alternatives:\n"
            "!help            (display all commands avaliable for the issuer of the help request)\n"
            "!help <command>  (display the usage info of a specified command)\n"
            "!help <level>    (display the usage infos of all commands restricted to a specified user level)\n"
            "conventions:\n"
            "<> argument  | alternative  [] optional  () description  -s[=<value>] switches" } }
        },
        { "login", { { OperatorLevel::Player, CommandScope::Local,
            "grant self operator status", { "[password]" } } }
        },
        { "listop", {
            { OperatorLevel::Limited, CommandScope::Remote,
                "list operators", {}, {
                    { 'o', "", "list online operators only (by default list all operators)" }
                }
            },
            { OperatorLevel::Player, CommandScope::Remote,
                "list the online operators"
            } }
        },
        { "own", { { OperatorLevel::Player, CommandScope::Local,
            "own the bot" } }
        },
        { "give", { { OperatorLevel::Limited, CommandScope::Local,
            "disown the bot" } }
        }
    });

    return commandInfos;
}


/// <summary>
/// Get a description string for all commands of a specified operator level.
/// </summary>
/// <param name="m_host">Host.</param>
/// <param name="level">Operator level.</param>
/// <param name="playerName">Name of the issuer.</param>
/// <returns>Description string for all commands of this level. Returns an empty string for operator level 
/// 'limited', because it's commands will be included for 'player' in case limited privilegues have been allowed by 
/// a sysop</returns>
std::string getCommandsDescription(Host& host, OperatorLevel level, std::string_view playerName)
{
    std::string desc;

    if (level != OperatorLevel::Limited) {
        // traverse all command infos
        for (const auto& [name, cmdInfo] : getCommandInfos(host)) {
            // traverse all definitions for this command
            for (const CommandInfo& cmdInfoElem : cmdInfo) {
                if (cmdInfoElem.minLevel == level
                    // put operator levels 'player' and 'limited' together if 'limited' is allowed
                    || (level == OperatorLevel::Player && cmdInfoElem.minLevel == OperatorLevel::Limited
                        && host.allowLimited)) {
                    // the !login command is shown only to mods
                    if ((name != "login" || getBotDatabase().findOperator(playerName))
                        // show the !give/!own commands to players with 'limited' privilegues, if allowed
                        && ((name != "give" && name != "own") || host.allowLimited)) {
                        desc += std::format(" !{}", name);
                        break;
                    }
                }
            }
        }

        if (!desc.empty()) {
            if (level >= OperatorLevel::Moderator)
                desc = std::format("lvl {}: {}", (int)level, desc);
            else
                desc = std::format("Basic: {}", desc);
        }
    }
    return desc;
}


//////// Commands ////////     

void gotHelp(Host& host, Player& player, const Command& cmd)
{
    std::string param{ toLower(cmd.getFinal()) };
    OperatorLevel level{ getOperatorLevel(param) };

    if (param.empty() || param == "all") {
        // !help: show all commands that are available for this player
        host.sendPrivate(player, std::format("{} command list (Send ::!help [<command>|<level>] for more info)",
            getLevelDescription(player.access)));

        auto& levelDesc{ getLevelDescriptions() };

        for (auto iter = levelDesc.rbegin(); iter != levelDesc.rend(); ++iter) {
            OperatorLevel accessLevel{ iter->first };

            // consider only levels that are available for the player who issued !help
            if (player.access >= accessLevel) {
                std::string cmdDesc{ getCommandsDescription(host, accessLevel, player.name) };

                if (!cmdDesc.empty()) {
                    host.sendPrivate(player, cmdDesc);
                }
            }
        }
    }
    else if (level != OperatorLevel::Unknown) {
        // !help <operator level>: show all commands that are available for the specified operator level
        if (player.access >= level) {
            std::string cmdDesc{ getCommandsDescription(host, level, player.name) };

            if (!cmdDesc.empty()) {
                host.sendPrivate(player, cmdDesc);
            }
        }
    }
    else {
        // !help <command>: show detailed help for a specified command
        std::string paramCmd { getAliasList().aliasToCommand(toLower(param)) };
        std::string errMsg;

        if (getCommandInfos(host).contains(paramCmd)) {
            for (const CommandInfo& cmdInfo : getCommandInfos(host)[paramCmd]) {
                if (player.checkCommandAccess(cmdInfo, errMsg)) {
                    host.sendPrivate(player, getCommandHelp(paramCmd, cmdInfo));
                    break;
                }
            }
            // in case command access is denied for this player, send a specific message to the player
            if (!errMsg.empty()) {
                host.sendPrivate(player, errMsg);
            }
        }
    }
}


void gotCommand(Host& host, Player& player, std::string_view msg)
{
    if (player.access < host.lowestLevel) {
        // Limiter is in effect
        OpEntry* op = getBotDatabase().findOperator(player.name);

        if (!op || (op->getAccess() < host.lowestLevel)) {
            return;
        }
    }

    const CommandInfoMap& commandInfos{ getCommandInfos(host) };
    Command cmd(msg);
    std::string command{ cmd.getCommand() };

    try {
        switch (player.access) {
        case OperatorLevel::Duke:
        case OperatorLevel::Baron:
        case OperatorLevel::King:
        case OperatorLevel::Emperor:
        case OperatorLevel::RockStar:
        case OperatorLevel::Q:
        case OperatorLevel::God:
        case OperatorLevel::Owner:
        case OperatorLevel::Unknown:
            if (cmd.check("password")) {
                if (!cmd.getFinal().empty()) {
                    host.sendPublic(std::format("?password={}", cmd.getFinal()));
                    host.botInfo.setLogin(host.botInfo.name, cmd.getFinal(), host.botInfo.staffpw);
                    host.sendPrivate(player, "Updated local bot parameters and requested network password change");
                }
                else {
                    host.sendPrivate(player, std::format("Current bot password: {}", host.botInfo.password));
                }
            }
            else if (cmd.check("info")) {
                host.sendPrivate(player, std::format(
                    "MISC SysOp:{} SMod:{} Mod:{} Ping:{} Client:{} DLL:{} DELT:{}",
                    host.hasSysOp, host.hasSMod, host.hasMod, host.syncPing,
                    getBotDatabase().forceContinuum ? "Ctm " + ContinuumVersion : "VIE " + SubspaceVersion, 
                        host.botInfo.dllNames, host.timeDiff));
                host.sendPrivate(player, std::format(
                    "USER {}:{}  ZONE {}:{}  ARENA '{}' ship #{} @ {}x{}", host.botInfo.name, 
                    host.botInfo.password, INADDR(host.botInfo.ip, 0).getString(),
                    host.botInfo.port, host.botInfo.initialArena, (int)host.botInfo.initialShip, 
                    host.botInfo.xres, host.botInfo.yres));
                host.sendPrivate(player, std::format(
                    "ban  mid:{} pid:{} tzb:{}", host.botInfo.machineID, host.botInfo.permissionID, 
                    host.botInfo.timeZoneBias));
            }
            else if (cmd.check("closeall")) {
                getHostList().disconnectAll();
                break;
            }
            else if (cmd.check("restart")) {
                getHostList().restartAll();
                break;
            }
            else if (cmd.check("log")) {
                size_t max = 5;

                if (isNumeric(cmd.getFinal())) {
                    max = cmd.getFinalAsInt();

                    if (max < 1) 
                        max = 5;
                }

                if (max > host.loggedChatter.size()) {
                    max = host.loggedChatter.size();
                }

                std::list<std::string>::reverse_iterator iter = host.loggedChatter.rbegin();

                for (int i = 0; i < max; ++i) {
                    host.sendPrivate(player, *iter++);
                }
                break;
            }
            else if (cmd.check("autosave")) {
                if (isNumeric(cmd.getFinal())) {
                    uint32_t interval = cmd.getFinalAsInt();

                    if ((interval >= 30) && (interval <= 10000000)) {
                        getBotDatabase().saveInterval = interval * 100;
                    }
                    host.sendPrivate(player, std::format("Database autosave every {} seconds", getBotDatabase().saveInterval));
                }
                else {
                    host.sendPrivate(player, "Format: !autosave <database interval in seconds>");
                }
            }
            else if (cmd.check("ppl")) {
                // unlisted, spit out diag report
                std::string s;
                uint32_t count{};

                for (auto& [_, p] : host.players) {
                    if (s.length() > 80) {
                        host.sendPublic(s);
                        s.clear();
                        count = 0;
                    }
                    else if (!s.empty()) {
                        s += "  ";
                    }

                    s += std::format("{}({}:{})@{}", p.name, p.score.wins, p.score.losses, p.team);
                    ++count;
                }

                if (count > 0) {
                    host.sendPublic(s);
                }
            }
            else if (cmd.check("save")) {
                getBotDatabase().save();
                host.sendPrivate(player, "Database save completed.");
            }
            else if (cmd.check("read")) {
                getBotDatabase().reloadIniFile(false);
                getBotDatabase().loadOperators();
                host.sendPrivate(player, "Re-loaded INI and Operators.txt settings.");
            }
            else if (cmd.check("set")) {
                std::string out{ cmd.getFinal() };
                std::string app, key, str;

                app = splitFirst(out, ':');
                key = splitFirst(out, ':');
                str = splitFirst(out, ':');

                setPrivateProfileString(app, key, str, getBotDatabase().iniFilePath);
                host.sendPrivate(player, std::format("Set->{}:{}:{}", app, key, str));
            }
            else if (cmd.check("get")) {
                std::string out{ cmd.getFinal() };
                std::string app, key, str;

                app = splitFirst(out, ':');
                key = splitFirst(out, ':');

                str = getPrivateProfileString(app, key, getBotDatabase().iniFilePath, "InvalidTag");
                host.sendPrivate(player, std::format("Get->{}:{}:{}", app, key, str));
            }
            else if (cmd.check("addalias")) {
                std::string cmdFinal{ toLower(cmd.getFinal()) };
                std::string command{ getAliasList().aliasToCommand(splitFirst(cmdFinal, ':')) };
                std::string alias{ splitFirst(cmdFinal, ':') };

                if (!invalidArena(command) && !invalidArena(alias)) {
                    if (getAliasList().findAlias(alias)) {
                        host.sendPrivate(player, std::format("Alias '{}' already exists", alias));
                    }
                    else {
                        getAliasList().addAlias(command, alias);
                        host.sendPrivate(player, std::format("Created alias '{}' for command '{}'", alias, command));
                    }
                }
                else {
                    host.sendPrivate(player, std::format("Invalid command or alias '{}':'{}'", alias, command));
                }
            }
            else if (cmd.check("killalias")) {
                std::string cmdFinal{ toLower(cmd.getFinal()) };

                if (getAliasList().killAlias(cmdFinal)) {
                    host.sendPrivate(player, std::format("Removed alias '{}'", cmdFinal));
                }
                else {
                    host.sendPrivate(player, std::format("Alias '{}' does not exist", cmdFinal));
                }
            }
            else if (cmd.check("aliases")) {
                std::string cmdFinal{ toLower(cmd.getFinal()) };
                bool anycmd{};
                std::string s{ "Cmd: " };
                uint32_t count{};

                if (!cmdFinal.empty()) {
                    s += cmdFinal;
                    ++count;
                }
                else {
                    break;
                }

                for (const CmdAlias& alias : getAliasList().getAliases()) {
                    if (!alias.isCmd(cmdFinal)) {
                        continue;
                    }

                    if (count++) {
                        s += " ";
                    }
                    s += alias.getAlias();
                    anycmd = true;

                    if (s.length() > 80) {
                        host.sendPrivate(player, s);
                        s = "Cmd: ";
                        count = 0;
                    }
                }

                if (!anycmd) {
                    host.sendPrivate(player, "No aliases for that command.");
                }
                else if (count) {
                    host.sendPrivate(player, s);
                }
            }
            else if (cmd.check("pball")) {
                int id = cmd.getFinalAsInt();

                for (PowerBall& pb : host.balls) {
                    if (pb.ident == id) {
                        host.postRR(generatePowerballRequest(pb.hosttime, id));
                        break;
                    }
                }
            }
            else if (cmd.check("test2")) {
                std::list<LvzObjectInfo> objectInfos;

                objectInfos.emplace_back(LvzObjectInfo{ 5656, false });
                host.postRR(generateObjectToggle(-1, objectInfos));
            }
            else if (cmd.check("test_obj")) {
                Player& p{ host.findPlayer(cmd.getFinal()) };

                if (!p.isAssigned()) {
                    break;
                }

                std::list<LvzObject> objects;

                for (uint32_t i = 0; i < 8; i++) {
                    LvzObject object;

                    object.id = 101 + i;
                    object.mapobj = 1;
                    object.change_image = 1;
                    object.image = 1;

                    objects.emplace_back(std::move(object));
                }

                host.postRR(generateObjectModify(-1, objects));
            }
        case OperatorLevel::SysOp:    // FALL THRU
            if (cmd.check("spawn")) {
                if (getHostList().getConnections() >= host.botInfo.maxSpawns) {
                    host.sendPrivate(player, "Aborted: Too many bot spawns are connected or connecting. Try again "
                        "soon");
                    break;
                }

                BotInfo bi = host.botInfo;
                std::string password, staff, arena, dlls;
                CommandSwitch s;

                // Name
                std::string name{ toLower(cmd.getFinal()) };

                if (invalidName(name)) {
                    host.sendPrivate(player, "Aborted: The bot name you gave me is invalid");
                    break;
                }
                // Password
                if (cmd.getParam('p', s)) {
                    password = s.param;
                }
                else {
                    password = bi.password;
                }
                // Staff
                if (cmd.getParam('s', s)) {
                    staff = s.param;
                }
                else {
                    staff = bi.staffpw;
                }
                // Imports
                if (cmd.getParam('i', s)) {
                    dlls = s.param;
                }
                else {
                    dlls = bi.dllNames;
                }
                // Arena
                if (cmd.getParam('a', s)) {
                    arena = s.param;
                }
                else {
                    arena = bi.initialArena;
                }

                if (invalidArena(arena)) {
                    host.sendPrivate(player, "Aborted: Arena name you gave me is invalid");
                    break;
                }

                bi.setLogin(name, password, staff);
                bi.setSpawn(dlls);
                bi.setArena(arena, bi.initialShip, bi.xres, bi.yres, bi.allowAudio);

                if (getHostList().connectHost(bi)) {
                    host.sendPrivate(player, std::format("Attempting to spawn {}. If successful, it will be in arena '{}' "
                        "in a few seconds...", name, arena));
                }
                else {
                    host.sendPrivate(player, std::format("Error while attempting to spawn {}.", name));
                }
            }
            else if (cmd.check("door")) {
                if (cmd.getFinal().empty()) {
                    host.sendPrivate(player, "Syntax: !door <door mode>");
                    break;
                }

                std::list<std::string> settings;

                settings.push_back(std::format("Door:DoorMode:{}", cmd.getFinal()));
                host.postRR(generateChangeSettings(settings));
            }
            else if (cmd.check("load")) {
                if (cmd.getFinal().empty()) {
                    host.sendPrivate(player, "Syntax: !load <filename>");
                    break;
                }

                if (host.imports->importLibrary(cmd.getFinal().data())) {
                    // Attempt to load new callbacks
                    host.sendPrivate(player, std::format("Successfully loaded module(s) '{}'.",
                        cmd.getFinal()));

                    std::string dllNames;
                    bool seen = false;

                    for (int slot = 0; slot < DllMaxLoaded; ++slot) {
                        std::string name{ host.imports->getPluginName(slot) };

                        if (!name.empty()) {
                            if (seen) {
                                dllNames += ", ";
                            }
                            dllNames += name;
                            seen = true;
                        }
                    }

                    host.botInfo.setSpawn(dllNames);
                }
                else {
                    host.sendPrivate(player, "I was unable to load the plugin you requested");
                }
            }
            else if (cmd.check("plugins")) {
                bool found = false;

                for (int slot = 0; slot < DllMaxLoaded; ++slot) {
                    std::string name{ host.imports->getPluginName(slot) };

                    if (!name.empty()) {
                        host.sendPrivate(player, std::format("slot {}: {}", slot, name));
                        found = true;
                    }
                }

                if (!found) {
                    host.sendPrivate(player, "No loaded plugins");
                }
            }
            else if (cmd.check("unload")) {
                if (isNumeric(cmd.getFinal())) {
                    int n = cmd.getFinalAsInt();

                    host.imports->clearImport(n);

                    host.sendPrivate(player, "Unloaded plugin");

                    std::string dllNames;
                    bool seen = false;

                    for (int slot = 0; slot < DllMaxLoaded; ++slot) {
                        std::string name{ host.imports->getPluginName(slot) };

                        if (!name.empty()) {
                            if (seen)
                                dllNames += ", ";
                            dllNames += name;
                            seen = true;
                        }
                    }

                    host.botInfo.setSpawn(dllNames);
                }
                else if (cmd.checkParam("all")) {
                    host.imports->clearImports();

                    host.sendPrivate(player, "Unloaded all plugins");

                    host.botInfo.setSpawn("");
                }
                else {
                    host.sendPrivate(player, "Invalid syntax. Send ::!help unload");
                }
            }
            else if (cmd.check("setbanner")) {
                host.postRR(generateChangeBanner(player.banner));

                host.sendPublic("*banner please");
            }
            else if (cmd.check("ownbot")) {
                if (cmd.checkParam("on")) {
                    host.allowLimited = true;
                }
                else if (cmd.checkParam("off")) {
                    host.allowLimited = false;
                    host.revokeAccess(OperatorLevel::Limited);
                }

                if (host.allowLimited) {
                    host.sendPrivate(player, "!own and !give commands allowed, listed in !help");
                }
                else {
                    host.sendPrivate(player, "!own and !give commands disallowed");
                }
            }
            else if (cmd.check("close")) {
                if (getHostList().getConnections() <= 1) {
                    host.sendPrivate(player, "Aborted: Too few bot spawns are connected or connecting. Try again "
                        "soon");
                    break;
                }

                host.disconnect(true);
            }
            else if (cmd.check("error")) {

                if (cmd.hasParam('g')) {
                    host.postRR(generateViolation(SecurityViolation::ShipChecksumMismatch));
                    break;
                }

                std::string chan;
                CommandSwitch i;

                if (cmd.getParam('c', i)) {
                    chan = i.param;
                }
                else {
                    chan = "error";
                }

                if (cmd.checkParam("on")) {
                    host.broadcastingErrors = true;
                }
                else if (cmd.checkParam("off")) {
                    host.broadcastingErrors = false;
                }

                std::string change = "?chat=";
                change += chan;
                host.sendPublic(change);

                std::string s = std::format("Error broadcasting {}sending to channel '{}'",
                    host.broadcastingErrors ? "on, " : "off, ", chan);

                host.sendPrivate(player, s);
            }
        case OperatorLevel::SuperModerator:    // FALL THRU
            if (cmd.check("uptime")) {
                uint64_t currTime = getTickCount();
                uint64_t arenaUptime = currTime - host.arenaJoinTime;
                uint64_t zoneUptime = currTime - host.zoneJoinTime;

                std::string s{ std::format("Alive in zone:({}d {}h {}m), in arena:({}d {}h {}m)",
                    zoneUptime / (uint64_t)(1000 * 60 * 60 * 24),
                    (zoneUptime / (uint64_t)(1000 * 60 * 60)) % 24,
                    (zoneUptime / (uint64_t)(1000 * 60)) % 60,
                    arenaUptime / (uint64_t)(1000 * 60 * 60 * 24),
                    (arenaUptime / (uint64_t)(1000 * 60 * 60)) % 24,
                    (arenaUptime / (uint64_t)(1000 * 60)) % 60) };

                host.sendPrivate(player, s);
            }
            else if (cmd.check("spawns")) {
                // AGH
                uint32_t n{};

                for (auto& host : getHostList().getSpawns()) {
                    std::string s = std::format("{}.  TypedName:{}  Arena:{}", n++, host->botInfo.name, 
                        host->botInfo.initialArena);

                    if (host->hasBot()) {
                        s += "  PlayingName:" + host->me().name;
                    }
                    else {
                        s += "  Not in an arena";
                    }

                    host->sendPrivate(player, s);
                }
            }
            else if (cmd.check("limit")) {
                if (!cmd.getFinal().empty()) {
                    if (!isNumeric(cmd.getFinal())) {
                        host.sendPrivate(player, "Format: !limit <numerical operator level>");
                        break;
                    }

                    OperatorLevel limit = (OperatorLevel)cmd.getFinalAsInt();

                    if (limit > OperatorLevel::Owner)
                        limit = OperatorLevel::Owner;

                    if (limit > player.access) {
                        host.sendPrivate(player, "Denied! Commiting bot suicide ain't cool");
                        break;
                    }

                    host.lowestLevel = limit;
                }

                host.sendPrivate(player, std::format("Limiting bot access to {} or higher",
                    getLevelDescription(host.lowestLevel)));
            }
            else if (cmd.check("addop")) {
                // Only valid names
                std::string name{ toLower(cmd.getFinal()) };

                if (invalidName(name)) {
                    host.sendPrivate(player, std::format("Invalid operator name '{}'", name));
                    break;
                }

                // Only new names
                OpEntry* op = getBotDatabase().findOperator(name);

                if (op) {
                    host.sendPrivate(player, "Cannot recreate an existing operator.  ::!listop  ::!help addop");
                    break;
                }

                // Process level switch
                OperatorLevel access;
                CommandSwitch i;

                if (cmd.getParam('l', i) && isNumeric(i.param)) {
                    access = (OperatorLevel)std::stoi(i.param);

                    if (access >= player.access && player.access < OperatorLevel::Owner) {
                        access = (OperatorLevel)((int)player.access - 1);
                    }

                    if (std::stoi(i.param) < 0) {
                        access = OperatorLevel::Moderator;
                    }
                }
                else {
                    access = OperatorLevel::Moderator;
                }

                // Add operator
                op = &getBotDatabase().addOperator(name, "", access);

                if (!op) {
                    host.sendPrivate(player, "The fates have conspired against you: addOperator() failed");
                    break;
                }

                // Notify requesting operator
                std::string s = std::format("Created {}: {}", getLevelDescription(op->getAccess()), op->getName());

                if (cmd.getParam('p', i) && (!i.param.empty())) {
                    s += ", with password ";
                    s += i.param;
                    op->setPassword(i.param);
                }
                else {
                    s += ", without a password";
                }

                host.sendPrivate(player, s);
            }
            else if (cmd.check("go")) {
                std::string arena{ toLower(cmd.getFinal()) };

                if (invalidArena(arena)) {
                    host.sendPrivate(player, std::format("Denied! Invalid arena name '{}'", arena));
                    break;
                }

                host.sendRemotePrivate(player.name, std::format("Moving to '{}'", arena));
                host.botInfo.setArena(arena, host.botInfo.initialShip, host.botInfo.xres, host.botInfo.yres, 
                    host.botInfo.allowAudio);
                host.changeArena(arena);
            }
            else if (cmd.check("editop")) {
                OpEntry* op = getBotDatabase().findOperator(cmd.getFinal());

                if (!op) {
                    host.sendPrivate(player, "Cannot modify non-existant operators.  ::!listop  ::!help editop");
                    break;
                }

                if (op->getAccess() >= player.access && player.access < OperatorLevel::Owner) {
                    host.sendPrivate(player, "Cannot modify peer or senior operators.  ::!listop  ::!help editop");
                    break;
                }

                CommandSwitch i;

                if (cmd.getParam('l', i) && isNumeric(i.param)) {
                    OperatorLevel access = (OperatorLevel)std::stoi(i.param);

                    if (std::stoi(i.param) < 0) {
                        host.sendPrivate(player,  "Invalid access level.  ::!listop  ::!help editop");
                        break;
                    }

                    if (access > player.access && player.access < OperatorLevel::Owner) {
                        host.sendPrivate(player, "Cannot promote above your level.  ::!listop  ::!help editop");
                        break;
                    }

                    op->setAccess(access);
                }

                // Notify requesting operator
                std::string s{ std::format("Updated. {}: {}", getLevelDescription(op->getAccess()), op->getName()) };

                if (cmd.getParam('p', i)) {
                    op->setPassword(i.param);

                    if (!i.param.empty()) {
                        s += ", changed password ";
                        s += i.param;

                        op->setPassword(i.param);
                    }
                    else {
                        s += ", removed the password";
                    }
                }

                host.sendPrivate(player, s);
            }
            else if (cmd.check("deleteop")) {
                // Determine which operator is being requested removed
                OpEntry* xop{ getBotDatabase().findOperator(cmd.getFinal()) };

                if (!xop) {
                    host.sendPrivate(player, "Removing a non-existant operator isn't groovy baby");
                    break;
                }

                if (xop->getName() == player.name) {
                    host.sendPrivate(player, "Thou shalt not commit bot suicide");
                    break;
                }

                // Run the request through level restrictions
                if (player.access <= xop->getAccess() && player.access < OperatorLevel::Owner) {
                    host.sendPrivate(player, "Thou shalt not remove thine peers and senior operators");

                    break;
                }

                // Remove the operator
                if (getBotDatabase().removeOperator(xop->getName())) {
                    host.sendPrivate(player, "Successfully removed operator");
                }
                else {
                    host.sendPrivate(player, "Failed to remove operator");
                }
            }
        case OperatorLevel::Moderator:    // FALL THRU
            if (cmd.check("setlogin")) {
                OpEntry* op{ getBotDatabase().findOperator(player.name) };

                op->setPassword(cmd.getFinal());

                std::string s;

                if (cmd.checkParam("")) {
                    s = "Login password no longer required";
                }
                else {
                    s = std::format("Login password changed to {}", cmd.getFinal());
                }

                host.sendPrivate(player, s);
            }
            else if (cmd.check("version")) {
                // unlisted, display bot version
                host.sendPrivate(player, VersionInfo);
            }
            else if (cmd.check("awarp")) {
                if (host.hasBot()) {
                    if (cmd.checkParam("0")) {
                        host.me().awarp = 0;
                    }
                    else if (cmd.checkParam("1")) {
                        host.me().awarp = 1;
                    }
                    else {
                        host.me().awarp = 1 - host.me().awarp;
                    }

                    switch (host.me().awarp) {
                    case 0:
                        host.sendPrivate(player, "Mode set: 0, I will not engage antiwarp.");
                        break;
                    case 1:
                        host.sendPrivate(player, "Mode set: 1, I will project an anti-warp field around my ship.");
                        break;
                    };
                }
            }
            else if (cmd.check("turret")) {
                if (cmd.checkParam("0")) {
                    host.turretMode = 0;
                }
                else if (cmd.checkParam("1")) {
                    host.turretMode = 1;
                }
                else {
                    host.turretMode = 1 - host.turretMode;
                }

                switch (host.turretMode) {
                case 0:
                    host.sendPrivate(player, "Mode set: 0, when in-game and not following I will fire against enemy "
                        "teams");
                    break;
                case 1:
                    host.sendPrivate(player, "Mode set: 1, when in-game and not following I will act like a "
                        "team-controlled turret");
                    break;
                default:
                    host.sendPrivate(player, "Unknown mode set. This be a problem you should talk to Catid about");
                    break;
                };
            }
            else if (cmd.check("drop")) {
                // unlisted, drops carried flags
                host.postRR(generateFlagDrop());
            }
            else if (cmd.check("where")) {
                Player& xp{ cmd.getFinal().empty() ? player : host.findPlayer(cmd.getFinal()) };

                if (xp.isAssigned()) {
                    int32_t x = xp.tile[0];
                    int32_t y = xp.tile[1];

                    std::string s = std::format(
                        "Last seen {} @ ({}, {}) [{}] stl:{} clk:{} x:{} awp:{} ufo:{} fsh:{} sft:{}",
                        xp.name, x, y, getCoords(x, y), xp.stealth, xp.cloak, xp.xradar, xp.awarp,
                        xp.ufo, xp.flash, xp.safety);

                    host.sendPrivate(player, s);
                }
            }
            else if (cmd.check("spec")) {
                host.postRR(generateChangeShip(Ship::Spectator));

                host.botInfo.setArena(host.botInfo.initialArena, Ship::Spectator, host.botInfo.xres, 
                    host.botInfo.yres, host.botInfo.allowAudio);
            }
            else if (cmd.check("follow")) {
                std::string s;

                if (cmd.getFinal().empty()) {
                    if (host.follow != UnassignedId) {
                        s = std::format("Currently following {}", host.getPlayer(host.follow).name);
                    }
                    else {
                        s = "Currently not following anyone";
                    }

                    s += ". Type !follow <name> to start following";
                    host.sendPrivate(player, s);
                    break;
                }

                // because people ignore the !help reference
                Player& xp{ cmd.checkParam("on") ? player : host.findPlayer(cmd.getFinal()) };

                if (xp.isAssigned()) {
                    host.follow = xp.ident;

                    s += "Started following ";
                    s += xp.name;
                    s += ". Will start once player is visible";

                    if (host.hasBot()) {
                        host.me().clone(xp);

                        if (host.me().ship == Ship::Spectator) {
                            host.postRR(generateChangeShip(Ship::Warbird));
                            host.botInfo.setArena(host.botInfo.initialArena, Ship::Warbird, 
                                host.botInfo.xres, host.botInfo.yres, host.botInfo.allowAudio);
                        }
                    }

                    host.sendPrivate(player, s);
                }
                else {
                    if (host.follow != UnassignedId) {
                        s += "Stopped following ";
                        s += host.getPlayer(host.follow).name;

                        if (host.hasBot()) {
                            host.me().vel = { 0, 0 };
                        }
                        host.follow = UnassignedId;
                    }
                    else {
                        s += "Stopped following";
                    }
                    host.sendPrivate(player, s);
                }
            }
            else if (cmd.check("attach")) {
                // because people ignore the !help reference
                Player& p{ cmd.getFinal().empty() || cmd.checkParam("on") ? player : 
                    host.findPlayer(cmd.getFinal()) };

                if (p.isAssigned()) {
                    host.sendPrivate(player, std::format("Attaching to {}...", p.name));
                    host.postRR(generateAttachRequest(p.ident));
                    host.follow = p.ident;
                    host.me().turretId = p.ident;
                }
                else {
                    host.sendPrivate(player, "Detached");
                    host.postRR(generateAttachRequest(UnassignedId));
                    host.me().turretId = UnassignedId;
                }
            }
            else if (cmd.check("ship")) {
                Ship ship = (Ship)((cmd.getFinalAsInt() - 1) & 7);

                host.botInfo.setArena(host.botInfo.initialArena, ship, host.botInfo.xres, host.botInfo.yres, 
                    host.botInfo.allowAudio);
                host.postRR(generateChangeShip(ship));
            }
            else if (cmd.check("team")) {
                if (isNumeric(cmd.getFinal())) {
                    uint16_t team = cmd.getFinalAsInt();

                    team %= MaxTeams;
                    host.postRR(generateChangeTeam(team));
                }
                else {
                    // because people ignore the !help reference
                    Player& p{ cmd.getFinal().empty() ? player : host.findPlayer(cmd.getFinal()) };
                    std::string s;

                    if (p.isAssigned()) {
                        s = std::format("Found {} on team {}", p.name, p.team);
                    }
                    else {
                        s = std::format("Unable to find {}", cmd.getFinal());
                    }
                    host.sendPrivate(player, s);
                }
            }
            else if (cmd.check("listchat")) {
                host.sendPrivate(player, std::format("Current Chat Channels: {}", host.botChats));
            }
            else if (cmd.check("clearchat")) {
                if (host.broadcastingErrors) {
                    host.sendPrivate(player, "Cannot clear chat channel while broadcasting errors");
                }
                else {
                    host.sendPublic("?chat=");
                    host.sendPrivate(player, "Chat channels cleared.");
                    host.botChats = "";
                }
            }
            else if (cmd.check("chat")) {
                if (!cmd.getFinal().empty()) {
                    if (host.broadcastingErrors) {
                        host.sendPrivate(player, "Cannot change chat channel while broadcasting errors");
                    }
                    else {
                        host.sendPublic(std::format("?chat={}", cmd.getFinal()));
                        host.sendPrivate(player, std::format("chat={}", cmd.getFinal()));

                        // OmegaFirebolt added ChatChannelList to keep track of channels
                        host.botChats = cmd.getFinal();
                    }
                }
                else {
                    host.sendPrivate(player, "You must provide channel name(s): !chat squad_chat,newbie_chat,"
                        "zone_chat");
                }
            }
            else if (cmd.check("say")) {
                if (cmd.getFinal().empty()) {
                    break;
                }
                switch (cmd.getFinal()[0]) {
                case '/':    // Team
                    if (cmd.getFinal().length() < 3) {
                        break;
                    }
                    if (cmd.getFinal()[1] != '/') {
                        break;
                    }
                    if (cmd.getFinal()[2] != '*' && cmd.getFinal()[2] != '?') {
                        std::string s{ cmd.getFinal().substr(2) };
                        std::string bong{ "0" };
                        std::size_t i = s.find('%');

                        if (i != std::string::npos) {
                            std::size_t j, k = i;

                            for (j = 0; j < 32; ++j) {
                                if (k < s.length() - 1) {
                                    char c = s[++k];

                                    if (c >= '0' && c <= '9')
                                        bong += c;
                                    else
                                        break;
                                }
                                else
                                    break;
                            }
                            s = s.substr(0, i) + s.substr(i + j + 1);
                        }
                        host.sendTeam((ChatSoundCode)std::stoi(bong), s);
                    }
                    break;
                case '\'':    // Team
                    if (cmd.getFinal().length() < 2) {
                        break;
                    }
                    if (cmd.getFinal()[1] != '*' && cmd.getFinal()[1] != '?') {
                        std::string s{ cmd.getFinal().substr(1) };
                        std::string bong{ "0" };
                        std::size_t i = s.find('%');

                        if (i != std::string::npos) {
                            std::size_t j, k = i;

                            for (j = 0; j < 32; ++j) {
                                if (k < s.length() - 1) {
                                    char c = s[++k];

                                    if (c >= '0' && c <= '9')
                                        bong += c;
                                    else
                                        break;
                                }
                                else
                                    break;
                            }
                            s = s.substr(0, i) + s.substr(i + j + 1);
                        }
                        host.sendTeam((ChatSoundCode)std::stoi(bong), s);
                    }
                    break;
                case ';':    // Channel
                    if (cmd.getFinal().length() < 2) {
                        break;
                    }
                    if (cmd.getFinal()[1] != '*' && cmd.getFinal()[1] != '?') {
                        host.sendChannel(cmd.getFinal().substr(1));
                    }
                    break;
                case ':':    // Remote private
                    if (isValidRemotePrivateChatMessage(cmd.getFinal())) {
                        host.sendRemotePrivate(cmd.getFinal());
                    }
                    break;
                default:    // Public
                    if (cmd.getFinal()[0] != '*' && cmd.getFinal()[0] != '?') {
                        std::string s{ cmd.getFinal() };
                        std::string bong{ "0" };
                        std::size_t i = s.find('%');

                        if (i != std::string::npos) {
                            std::size_t j, k = i;

                            for (j = 0; j < 32; ++j) {
                                if (k < s.length() - 1) {
                                    char c = s[++k];

                                    if (c >= '0' && c <= '9')
                                        bong += c;
                                    else
                                        break;
                                }
                                else
                                    break;
                            }
                            s = s.substr(0, i) + s.substr(i + j + 1);
                        }
                        host.sendPublic((ChatSoundCode)std::stoi(bong), s);
                    }
                };
            }
            else if (cmd.check("zone")) {
                if (cmd.getFinal().empty()) {
                    host.sendPrivate(player, "Here's how \'!zone\' works: !zone <message>  (I will add a - nametag "
                        "to the end)");
                    break;
                }
                host.sendPublic(std::format("*zone{} -{}", cmd.getFinal(), player.name));
            }
            else if (cmd.check("kill")) {
                // disabled in commands.txt by default
                host.postRR(generateDeath(player.ident, 500));
            }
            else if (cmd.check("ident")) {
                // unlisted, get player ident
                Player p{ host.findPlayer(cmd.getFinal()) };

                if (!p.isAssigned()) {
                    host.sendPrivate(player, std::format("Player {}: {}", p.name, player.ident));
                }
                else {
                    host.sendPrivate(player, "Cannot see this player");
                }
            }
            else if (cmd.check("whereflags")) {
                // unlisted, get flag coords
                std::string s = "Flags: ";
                int team = 10000;

                for (Flag& f : host.flags) {
                    s += " ";

                    if (f.team != team) {
                        team = f.team;
                        s += std::format("{}:", team != UnassignedId ? std::to_string(team) : "unowned");
                    }
                    s += getCoords(f.x, f.y);
                }
                host.sendPrivate(player, s);
            }
            else if (cmd.check("logout")) {
                // unlisted, remove operator powers
                player.access = OperatorLevel::Player;
            }
        case OperatorLevel::Limited:    // FALL THRU
            if (host.allowLimited && cmd.check("give")) {
                host.revokeAccess(OperatorLevel::Limited);
                host.limitedOwnerId = UnassignedId;

                Player& p{ host.findPlayer(cmd.getFinal()) };

                if (p.isAssigned() && (p.access == OperatorLevel::Player)) {
                    p.access = OperatorLevel::Limited;
                    host.sendPrivate(p, std::format("You have been granted bot ownership powers by {}.Send ::!help for "
                        "extended commands", player.name));
                    host.sendPrivate(player, "Your ownership powers have been successfully transferred");
                }
                else {
                    host.sendPrivate(player, "Your ownership powers have been successfully released");
                }
            }
        case OperatorLevel::Player:    // FALL THRU
            if (cmd.check("help")) {
                gotHelp(host, player, cmd);
            }
            //else if (cmd.check("settings"))
            //{
            //    _linkedlist<std::string> setts;
            //    setts.append(new std::string("Misc:SheepMessage:catid wuz here"));

            //    m_host.post(generateChangeSettings(setts), true);
            //}
            else if (cmd.check("disabled")) {
                host.sendPrivate(player, "Forget about it! This command is disabled");
            }
            else if (cmd.check("botlag")) {
                // unlisted, display session latency
                // PING Current:BLEH ms  Average:BLEH ms  Low:BLEH ms  High:BLEH ms
                host.sendPrivate(player, std::format(
                    "PING Current:{} ms  Average:{} ms  Low:{} ms  High:{} ms  Delta:{} ms",
                    host.syncPing * 10, host.avgPing * 10, host.lowPing * 10, host.highPing * 10,
                    host.timeDiff * 10));
            }
            else if (cmd.check("sex")) {
                // unlisted, have sex with bot
                host.sendPrivate(player, ChatSoundCode::Girl, "Unf! Unf! Unf!");
            }
            else if (cmd.check("sheep")) {
                // unlisted, sheeply bah
                host.sendPrivate(player, ChatSoundCode::Sheep, "Bah!");
            }
            else if (host.allowLimited && cmd.check("own")) {
                Player& p{ host.getPlayer(host.limitedOwnerId) };

                if (p.isAssigned()) {
                    host.sendPrivate(player, std::format("Player {} has claimed me", p.name));
                }
                else if (player.access == OperatorLevel::Player) {
                    host.sendPrivate(player, "You are now my owner. Send ::!help for extended commands");
                    player.access = OperatorLevel::Limited;
                    host.limitedOwnerId = player.ident;
                }
                else {
                    host.sendPrivate(player, "You cannot claim Limited ownership as an operator of higher rank");
                }
            }
            else if (cmd.check("listop")) {
                OperatorLevel access = OperatorLevel::Player;
                uint32_t total = 0;
                std::string l;

                // compute onlineOnly
                bool onlineOnly = player.access == OperatorLevel::Player ? true : cmd.hasParam('o');

                if (onlineOnly) {
                    // only online operators
                    for (auto& p : host.players | std::views::values) {
                        if (p.access == OperatorLevel::Player || p.access == OperatorLevel::Unknown) {
                            continue;
                        }
                        if (access != p.access) {
                            l += getLevelDescription(access = p.access);
                            l += ": ";
                        }
                        l += p.name;
                        ++total;

                        if (p.isAssigned()) {
                            if (l.length() > 80) {
                                host.sendPrivate(player, l);
                                l.clear();
                                access = OperatorLevel::Player;
                            }
                            else {
                                l += "  ";
                            }
                        }
                    }
                }
                else {
                    // online and offline operators
                    bool isFirst{ true };

                    for (OpEntry& op : getBotDatabase().operators) {
                        if (op.getAccess() == OperatorLevel::Unknown) {
                            continue;
                        }
                        if (!isFirst) {
                            if (l.length() > 80) {
                                host.sendPrivate(player, l);
                                l.clear();
                                access = OperatorLevel::Player;
                            }
                            else {
                                l += "  ";
                            }
                        }

                        if (access != op.getAccess()) {
                            l += getLevelDescription(access = op.getAccess());
                            l += ": ";
                        }

                        Player& p{ host.findPlayer(op.getName()) };

                        if (p.isAssigned() && (p.access != OperatorLevel::Player)) {
                            l += "%";
                        }
                        l += op.getName();

                        ++total;
                        isFirst = false;
                    }
                }

                if (total) {
                    host.sendPrivate(player, l);
                }
                else {
                    host.sendPrivate(player, "No operators");
                }
            }
            else if (cmd.check("login")) {
                OpEntry* op{ getBotDatabase().findOperator(player.name) };

                if (op) {
                    // Listed
                    if (!host.billerOnline && op->validatePass("")) {
                        op->addFailure();

                        host.sendPrivate(player, "Unable to log you on while the biller is down. This failure has "
                            "been logged");
                        break;
                    }

                    if (op->validatePass(cmd.getFinal())) {
                        // Grant access
                        if (player.ident == host.limitedOwnerId) {
                            host.limitedOwnerId = UnassignedId;    // reset Limited ownership
                        }

                        player.access = op->getAccess();
                        op->addCounter();

                        uint32_t failure = op->getFailureCount();
                        uint32_t overall = op->getOverallCount();

                        host.sendPrivate(player, std::format(
                            "{}: WelcoMe(). Sessions  {} logged  {} failed from your handle",
                            getLevelDescription(player.access), overall, failure));
                    }
                    else {
                        // Bad password
                        op->addFailure();
                        host.sendPrivate(player, "Bad password. This failure has been logged");
                        break;
                    }
                }
                else if (player.access > OperatorLevel::Player) {
                    // Not listed, but has powers already
                    host.sendPrivate(player, "Login failure: account revoked. Please contact your senior staff");
                }
            }
        };

        host.raiseEvent(BotEvent::Command, player, cmd);
    }
    catch (std::exception& ex) {
        // catch all command-crash bugs
        host.sendPrivate(player, ChatSoundCode::Inconceivable, "Oh no! You've managed to find a command-crash bug!");
        host.logError(ex.what());
    }
}


void gotRemoteCommand(Host& host, std::string_view playerName, std::string_view msg)
{
    // retrieve operator level
    OpEntry* op{ getBotDatabase().findOperator(playerName) };
    OperatorLevel level{ op ? op->getAccess() : OperatorLevel::Player };

    // operators with passwords may not use remote private messages [optional]
    if (!getBotDatabase().remoteOperator) {
        if (op->validatePass("")) {
            level = OperatorLevel::Player;
        }
    }
    if (!host.billerOnline) {
        // heavens no!
        level = OperatorLevel::Player;
    }
    if (host.lowestLevel > OperatorLevel::Player) {
        // limiter is in effect
        if (level < host.lowestLevel) {
            return;
        }
    }
    Player remotePlayer(playerName, level, CommandScope::Remote);

    gotCommand(host, remotePlayer, msg);
}
