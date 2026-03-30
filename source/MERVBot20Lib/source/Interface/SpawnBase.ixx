export module SpawnBase;

export import Algorithms;
export import Config;
export import ModuleBase;
export import Observable;

import <format>;
import <functional>;
import <iostream>;
import <list>;
import <map>;
import <ranges>;
import <tuple>;
import <utility>;


//////// Spawn Base ////////

/// <summary>
/// Base class for the custom Spawn of a bot plugin. Make sure to first call SpawnBase::setup() from the setup() 
/// function of your custom Spawn.
/// </summary>
export class SpawnBase : public Observable
{
public:
    /// <summary>
    /// Constructor. Creates the spawn base of this plugin for a given host.
    /// </summary>
    /// <param name="name">Plugin name.</param>
    /// <param name="obHost">Observable host.</param>
    SpawnBase(std::string_view name, Observable& obHost) 
        : Observable("SpawnBase"), m_pluginName(name), m_obHost(obHost)
    {
        // define the events that can be received by observers of the spawn
        addEvent<std::string_view>(BotEvent::Echo);
        addEvent<std::string_view>(BotEvent::Warning);
        addEvent<std::string_view>(BotEvent::Error);
        addEvent<ChatMode, ChatSoundCode, uint16_t, std::string_view>(BotEvent::Say);
        addEvent<const Player&>(BotEvent::Die);
        addEvent<const Player&>(BotEvent::Attach);
        addEvent(BotEvent::Detach);
        addEvent<bool>(BotEvent::Following);
        addEvent<bool>(BotEvent::Flying);
        addEvent<Banner>(BotEvent::Banner);
        addEvent(BotEvent::DropBrick);
        addEvent<Ship>(BotEvent::Ship);
        addEvent<uint16_t>(BotEvent::Team);
        addEvent<FlagId>(BotEvent::GrabFlag);
        addEvent<bool>(BotEvent::SendPosition);
        addEvent(BotEvent::DropFlags);
        addEvent<std::string_view, std::string_view, std::string_view, std::string_view>(BotEvent::SpawnBot);
        addEvent<std::string_view>(BotEvent::ChangeArena);
        addEvent<WeaponInfo>(BotEvent::FireWeapon);
        addEvent<PlayerId, const std::list<LvzObjectInfo>&>(BotEvent::ToggleObjects);
        addEvent<PlayerId, const std::list<LvzObject>&>(BotEvent::ModifyObjects);
        addEvent<PlayerId>(BotEvent::GrabBall);
        addEvent<PlayerId, Coord, Coord, Velocity, Velocity>(BotEvent::FireBall);
        addEvent<const std::list<std::string>&>(BotEvent::ChangeSettings);
        addEvent(BotEvent::Exit);
    }

    /// <summary>
    /// Register obligatory event and command handlers to be provided by the spawn base.
    /// </summary>
    void setup() 
    {
        // register event handlers
        registerEventHandler(BotEvent::Init, &SpawnBase::handleBotEventInit, this);
        registerEventHandler(BotEvent::ArenaEnter, &SpawnBase::handleBotEventArenaEnter, this);
        registerEventHandler(BotEvent::ArenaSettings, &SpawnBase::handleBotEventArenaSettings, this);
        registerEventHandler(BotEvent::Tick, &SpawnBase::handleBotEventTick, this);
        registerEventHandler(BotEvent::Command, &SpawnBase::handleBotEventCommand, this);

        // register command handlers
        // level 0 (player):
        registerCommandHandler("help", &SpawnBase::handleCommandHelp, this,
            { { OperatorLevel::Player, CommandScope::External } });
    }

    //////// Modules ////////

    /// <summary>
    /// Get all modules.
    /// </summary>
    /// <typeparam name="SpawnT">Spawn type.</typeparam>
    /// <returns>List of module pointers.</returns>
    template<typename SpawnT>
    auto& getModules() 
    {
        static std::list<std::unique_ptr<ModuleBase<SpawnT>>> pluginModules;

        return pluginModules;
    }

    /// <summary>
    /// Create a module.
    /// </summary>
    /// <typeparam name="SpawnT">Spawn type.</typeparam>
    /// <typeparam name="ModuleT">Module type.</typeparam>
    /// <param name="spawn">Spawn pointer.</param>
    /// <param name="...args">Parameters for the module constructor.</param>
    template<typename SpawnT, typename ModuleT>
    void createModule(SpawnT* spawn)
    {
        // create and set up the plugin module
        getModules<SpawnT>().emplace_back(std::make_unique<ModuleT>(ModuleT(*spawn)));
        getModules<SpawnT>().back()->setup();
    }

    /// <summary>
    /// Create a module with additional parameters.
    /// </summary>
    /// <typeparam name="SpawnT">Spawn type.</typeparam>
    /// <typeparam name="ModuleT">Module type.</typeparam>
    /// <typeparam name="...Args">Module argument types.</typeparam>
    /// <param name="spawn">Spawn pointer.</param>
    /// <param name="...args">Parameters for the module constructor.</param>
    template<typename SpawnT, typename ModuleT, typename... Args>
    void createModule(SpawnT* spawn, Args&&... args)
    {
        // create and set up the plugin module
        getModules<SpawnT>().emplace_back(std::make_unique<ModuleT>(ModuleT(*spawn, std::forward(args)...)));
        getModules<SpawnT>().back()->setup();
    }

    /// <summary>
    /// Create a module with additional reference parameters.
    /// </summary>
    /// <typeparam name="SpawnT">Spawn type.</typeparam>
    /// <typeparam name="ModuleT">Module type.</typeparam>
    /// <typeparam name="...Args">Module argument types.</typeparam>
    /// <param name="spawn">Spawn pointer.</param>
    /// <param name="...args">Parameters for the module constructor.</param>
    template<typename SpawnT, typename ModuleT, typename... Args>
    void createModule(SpawnT* spawn, Args&... args)
    {
        // create and set up the plugin module
        getModules<SpawnT>().emplace_back(std::make_unique<ModuleT>(ModuleT(*spawn, args...)));
        getModules<SpawnT>().back()->setup();
    }

    /// <summary>
    /// Close all modules.
    /// </summary>
    /// <typeparam name="SpawnT">Spawn type.</typeparam>
    template<typename SpawnT>
    void closeModules() 
    {
        for (auto& pluginModule : getModules<SpawnT>()) {
            pluginModule->close();
        }
    }

    //////// Accessors ////////

    const PlayerMap& getPlayers() const
    {
        return m_refParams.get<const PlayerMap>("players");
    }

    const FlagList& getFlags() const
    {
        return m_refParams.get<const FlagList>("flags");
    }

    const BrickList& getBricks() const
    {
        return m_refParams.get<const BrickList>("bricks");
    }

    const Playfield getPlayfield() const
    {
        return m_refParams.get<const Playfield>("playfield");
    }

    const ArenaSettings& getSettings() const
    {
        return m_refParams.get<const ArenaSettings>("settings");
    }

    std::string_view getArenaName() const
    {
        return m_arena;
    }

    bool getIsBillerOnline() const
    {
        return m_isBillerOnline;
    }

    std::string_view getCmdLineParams() const
    {
        return m_cmdLineParams;
    }

    const Player& getPlayer(PlayerId playerId) const
    {
        return getPlayers().at(playerId);
    }

    Player& getMe() const
    {
        return (Player&)m_refParams.get<const Player>("me");
    }

    std::string_view getPluginName() const
    {
        return m_pluginName;
    }

    //////// Event Handling ////////

    /// <summary>
    /// Register an event handler.
    /// </summary>
    /// <typeparam name="ObserverT">Obersver type.</typeparam>
    /// <typeparam name="...Args">Event parameter types.</typeparam>
    /// <param name="type">Bot event.</param>
    /// <param name="f">Event handler.</param>
    /// <param name="ob">Observing object to receive the event.</param>
    template<typename ObserverT, typename... Args>
    void registerEventHandler(BotEvent type, void(ObserverT::*f)(Args...), ObserverT* ob)
    {
        m_obHost.registerEventHandler(type, f, ob);
    }

    //////// Command Handling ////////
    
    /// <summary>
    /// Register a command handler.
    /// </summary>
    /// <typeparam name="ObserverT">Obersver type.</typeparam>
    /// <param name="name">Command name.</param>
    /// <param name="f">Command handler.</param>
    /// <param name="ob">Observing object to receive the command.</param>
    /// <param name="cmdInfos">Command infos.</param>
    template<typename ObserverT>
    void registerCommandHandler(std::string name, MessageList(ObserverT::* f)(const Player&, const Command&), 
        ObserverT* ob, const std::list<CommandInfo>& cmdInfos)
    {
        m_cmdHandlers[name] = [f, ob, cmdInfos](const Player& player, const Command& cmd) -> MessageList {
            return (ob->*f)(player, cmd); };
        addCommandInfos(name, cmdInfos);
    }

    //////// Chat Messaging ////////

    void sendArena(std::string_view msg)
    {
        raiseEvent(BotEvent::Say, ChatMode::Public, ChatSoundCode::None, 0, format("*arena {}", msg));
    }

    void sendPrivate(PlayerId playerId, std::string_view msg)
    {
        raiseEvent(BotEvent::Say, ChatMode::Private, ChatSoundCode::None, playerId, msg);
    }

    void sendPrivate(PlayerId playerId, ChatSoundCode snd, std::string_view msg)
    {
        raiseEvent(BotEvent::Say, ChatMode::Private, snd, playerId, msg);
    }

    void sendTeam(std::string_view msg)
    {
        raiseEvent(BotEvent::Say, ChatMode::Team, ChatSoundCode::None, 0, msg);
    }

    void sendTeam(ChatSoundCode snd, std::string_view msg)
    {
        raiseEvent(BotEvent::Say, ChatMode::Team, snd, 0, msg);
    }

    void sendTeamPrivate(uint16_t team, std::string_view msg)
    {
        raiseEvent(BotEvent::Say, ChatMode::TeamPrivate, ChatSoundCode::None, team, msg);
    }

    void sendTeamPrivate(uint16_t team, ChatSoundCode snd, std::string_view msg)
    {
        raiseEvent(BotEvent::Say, ChatMode::TeamPrivate, snd, team, msg);
    }

    void sendPublic(std::string_view msg)
    {
        raiseEvent(BotEvent::Say, ChatMode::Public, ChatSoundCode::None, 0, msg);
    }

    void sendPublic(ChatSoundCode snd, std::string_view msg)
    {
        raiseEvent(BotEvent::Say, ChatMode::Public, snd, 0, msg);
    }

    void sendPublicMacro(std::string_view msg)
    {
        raiseEvent(BotEvent::Say, ChatMode::PublicMacro, ChatSoundCode::None, 0, msg);
    }

    void sendPublicMacro(ChatSoundCode snd, std::string_view msg)
    {
        raiseEvent(BotEvent::Say, ChatMode::PublicMacro, snd, 0, msg);
    }

    void sendChannel(std::string_view msg)
    {
       raiseEvent(BotEvent::Say, ChatMode::Channel, ChatSoundCode::None, getMe().ident, msg);
    }

    void sendChannel(uint32_t channel, std::string_view msg)
    {
        sendChannel(std::format(":{}:{}", channel, msg));
    }

    void sendRemotePrivate(std::string_view msg)
    {
        raiseEvent(BotEvent::Say, ChatMode::RemotePrivate, ChatSoundCode::None, 0, msg);
    }

    void sendRemotePrivate(std::string_view playerName, std::string_view msg)
    {
        sendRemotePrivate(std::format(":{}:{}", playerName, msg));
    }

    //////// Tick Counter ////////

    /// <summary>
    /// Get the tick counters.
    /// </summary>
    /// <returns>Tick counter map.</returns>
    TickCounterMap& getTickCounters()
    {
        return m_tickCounters;
    }

    /// <summary>
    /// Set a callback function of an expiration tick counter with a tick interval of one second.
    /// </summary>
    /// <param name="name">Counter name.</param>
    /// <param name="count">Number of ticks til expiration.</param>
    /// <param name="callback">Callback function.</param>
    void setTickCounter(std::string_view name, uint32_t count, std::function<void()> callback = [] {})
    {
        m_tickCounters[name.data()] = std::pair<uint32_t, std::function<void()>>(count, callback);
    }

    /// <summary>
    /// Check if a tick counter has expired.
    /// </summary>
    /// <param name="name">Counter name.</param>
    /// <returns>True, if the counter with the given name has expired, otherwise false.</returns>
    bool isTickCounterExpired(std::string_view name)
    {
        return !m_tickCounters.contains(name.data()) ? true : false;
    }

    //////// LVZ Objects ////////

    void toggleObjects(PlayerId playerId = UnassignedId)
    {
        if (m_lvzToggleObjects.contains(playerId)) {
            std::list<LvzObjectInfo>& objectInfos{ m_lvzToggleObjects[playerId] };

            raiseEvent(BotEvent::ToggleObjects, playerId, objectInfos);
            objectInfos.clear();
        }
    }

    void queueEnableObject(uint16_t objectId, PlayerId playerId = UnassignedId)
    {
        std::list<LvzObjectInfo>& objectInfos{ m_lvzToggleObjects[playerId] };

        if (objectInfos.size() == m_maxLvzObjects) {
            toggleObjects(playerId);
        }
        objectInfos.emplace_back(LvzObjectInfo{ objectId , false });
    }

    void queueDisableObject(uint16_t objectId, PlayerId playerId = UnassignedId)
    {
        std::list<LvzObjectInfo>& objectInfos{ m_lvzToggleObjects[playerId] };

        if (objectInfos.size() == m_maxLvzObjects) {
            toggleObjects(playerId);
        }
        objectInfos.emplace_back(LvzObjectInfo{ objectId , true });
    }

    void modifyObjects(PlayerId playerId = UnassignedId)
    {
        if (m_lvzModifyObjects.contains(playerId)) {
            std::list<LvzObject>& objects{ m_lvzModifyObjects[playerId] };

            raiseEvent(BotEvent::ModifyObjects, playerId, objects);
            objects.clear();
        }
    }

    void queueModifyObject(LvzObject& object, PlayerId playerId = UnassignedId)
    {
        std::list<LvzObject>& objects{ m_lvzModifyObjects[playerId] };

        if (objects.size() == m_maxLvzObjects) {
            modifyObjects(playerId);
        }

        objects.emplace_back(std::move(object));
    }

    void setObjectPos(uint16_t objectId, uint16_t x, uint16_t y, PlayerId playerId = UnassignedId)
    {
        LvzObject object{ .change_xy = 1, .change_image = 0, .change_layer = 0, .change_time = 0, .change_mode = 0,
            .mapobj = 0, .id = objectId, .x = (uint16_t)((x * 16) + 2), .y = (uint16_t)(y * 16) };

        queueModifyObject(object, playerId);
    }

    void setObjectImage(uint16_t objectId, uint8_t imageId, PlayerId playerId = UnassignedId)
    {
        LvzObject object{ .change_xy = 0, .change_image = 1, .change_layer = 0, .change_mode = 0, .mapobj = 0,
            .id = objectId, .image = imageId };

        queueModifyObject(object, playerId);
    }

    void setObjectImagePos(uint16_t objectId, uint8_t imageId, uint16_t x, uint16_t y,
        PlayerId playerId = UnassignedId)
    {
        LvzObject object{ .change_xy = 1, .change_image = 1, .change_layer = 0, .change_time = 0, .change_mode = 0,
            .mapobj = 0, .id = objectId, .x = (uint16_t)((x * 16) + 2), .y = (uint16_t)(y * 16), .image = imageId };

        queueModifyObject(object, playerId);
    }

    //////// Misc ////////

    /// <summary>
    /// Find all players who's name starts with a specified prefix. Casing is neglected.
    /// </summary>
    /// <param name="namePrefix">Name prefix.</param>
    /// <returns>Ids of players who's name starts with the specified prefix.</returns>
    const std::list<PlayerId> findPlayers(std::string_view namePrefix) const
    {
        std::list<PlayerId> playerIds;

        for (const Player& player : getPlayers() | std::views::values) {
            if (toLower(player.getName()).starts_with(toLower(namePrefix))) {
                playerIds.push_back(player.ident);
            }
        }
        return playerIds;
    }

    /// <summary>
    /// Execute a command issued either by a player or a user of an external chat client.
    /// </summary>
    /// <param name="player">Player. Can be a 'local' player from the player list or a temporary 'remote' or temporary
    /// 'external' player.</param>
    /// <param name="cmd">Command.</param>
    /// <returns>Command reply to the issuer as a list of private messages.</returns>
    MessageList executeCommand(const Player& player, const Command& cmd)
    {
        MessageList replyMessages;
        std::string command{ cmd.getCommand() };
        std::string errMsg;

        if (m_cmdHandlers.contains(command)) {
            for (CommandInfo& cmdInfo : m_cmdInfos[command]) {
                if (player.checkCommandAccess(cmdInfo, errMsg)) {
                    return m_cmdHandlers[command](player, cmd);
                }
            }
            replyMessages.push_back(errMsg);
        }
        return replyMessages;
    }

    /// <summary>
    /// Add a command info.
    /// </summary>
    /// <param name="cmdInfos">Command infos.</param>
    void addCommandInfos(std::string name, const std::list<CommandInfo>& cmdInfos)
    {
        m_cmdInfos[name] = cmdInfos;
    }

    /// <summary>
    /// Get the command infos.
    /// </summary>
    /// <returns>Map with command name as key and list of command infos as value.</returns>
    CommandInfoMap& getCommandInfos()
    {
        return m_cmdInfos;
    }

    /// <summary>
    /// Get the help description for a command with a specific level and scope.
    /// </summary>
    /// <param name="level">Operator level.</param>
    /// <param name="scope">Command scope.</param>
    /// <param name="command">Command name.</param>
    /// <returns>Command help as a list of strings.</returns>
    MessageList findCommandHelp(OperatorLevel level, CommandScope scope, std::string_view command)
    {
        MessageList retMessages;
        std::string cmd{ getAliasList().aliasToCommand(toLower(command)) };

        if (m_cmdInfos.contains(cmd)) {
            for (const CommandInfo& cmdInfo : m_cmdInfos[cmd]) {
                if (cmdInfo.minLevel <= level && cmdInfo.maxScope >= scope) {
                    // detailed help
                    retMessages.push_back(getCommandHelp(cmd, cmdInfo));
                    break;
                }
            }
        }

        if (retMessages.empty()) {
            retMessages.push_back(std::format("unknown command '{}'!", cmd));
        }
        return retMessages;

    }

    /// <summary>
    /// Get an informative arena description.
    /// </summary>
    /// <returns>Arena description.</returns>
    std::string getArenaDescription()
    {
        return (getArenaName() == "0") ? "Public 0" : std::string(getArenaName());
    }

    /// <summary>
    /// Checks if the spawn base has been initialized successfully.
    /// </summary>
    /// <returns>True, if the spawn base has been initialized successfully, otherwise false.</returns>
    bool isInitialized()
    {
        return m_isInitialized;
    }

private:
    //////// Event handlers ////////

    void handleBotEventInit(uint32_t version, const PlayerMap& players, const FlagList& flags,
        const BrickList& bricks, const Playfield& playfield, std::string_view cmdLineParams)
    {
        // store the initialization parameters
        m_refParams.set("players", players);
        m_refParams.set("flags", flags);
        m_refParams.set("bricks", bricks);
        m_refParams.set("playfield", playfield);
        m_cmdLineParams = cmdLineParams;

        // verify the plugin version
        uint32_t major = hiWord(version);
        uint32_t minor = loWord(version);

        if (major > CoreMajorVersion) {
            // major version of the MERVBot20Lib in use is greater than the one this plugin has been built against, 
            // so this plugin is not upward-compatible to the running version of MERVBot20Lib
            raiseEvent(BotEvent::Exit, "DLL plugin cannot connect. This plugin is out of date!");
        }
        else if ((major < CoreMajorVersion) || (minor < CoreMinorVersion)) {
            // major and minor version of the MERVBot20Lib in use are smaller than the ones this plugin has been 
            // built against, so the running version of MERVBot20 is outdated
            raiseEvent(BotEvent::Exit, "DLL plugin cannot connect. This plugin requires the latest version of "
                "MERVBot20!");
        }
        else {
            raiseEvent(BotEvent::Echo, std::format("DLL plugin {} connected", getPluginName()));
            m_isInitialized = true;
        }
    }

    void handleBotEventArenaEnter(std::string_view arena, Player& me, bool isBillerOnline)
    {
        m_refParams.set("me", (const Player&)me);
        m_arena = arena;
        m_isBillerOnline = isBillerOnline;
    }

    void handleBotEventArenaSettings(const ArenaSettings& settings)
    {
        m_refParams.set("settings", settings);
    }

    void handleBotEventTick()
    {
        // process the tick counters
        std::list<std::string> eraseKeys;

        for (auto& [key, tickCountCallback] : getTickCounters()) {
            auto& [count, callback] { tickCountCallback };
            // if this counter has expired, do the callback and remember to erase the counter
            if (count <= 1) {
                callback();
                eraseKeys.push_back(key);
            }
            --count;
        }
        // erase the entries for all counters that have been finished with a callback
        for (auto& key : eraseKeys) {
            getTickCounters().erase(key);
        }
    }

    void handleBotEventCommand(const Player& player, const Command& cmd)
    {
        MessageList replyMessages = executeCommand(player, cmd);

        for (std::string msg : replyMessages) {
            if(player.ident != UnassignedId) {
                sendPrivate(player.ident, msg);
            }
            else {
                sendRemotePrivate(player.name, msg);
            }
        }
    }

    //////// Command Handlers ////////

    MessageList handleCommandHelp(const Player& player, const Command& cmd)
    {
        MessageList retMessages;

        std::string param{ toLower(cmd.getFinal()) };
        OperatorLevel level{ getOperatorLevel(param) };

        if (param.empty() || param == "all") {
            // !help: show all commands that are available for this player
            retMessages.push_back(std::format("{}:", m_pluginName));

            auto& levelDesc{ getLevelDescriptions() };

            for (auto iter = levelDesc.rbegin(); iter != levelDesc.rend(); ++iter) {
                OperatorLevel accessLevel{ iter->first };

                // consider only levels that are available for the player who issued !help
                if (player.access >= accessLevel) {
                    std::string cmdDesc{ getCommandsDescription(accessLevel) };

                    if (!cmdDesc.empty()) {
                        retMessages.push_back(cmdDesc);
                    }
                }
            }
        }
        else if (level != OperatorLevel::Unknown) {
            // !help <operator level>: show all commands that are available for the specified operator level
            if (player.access >= level) {
                std::string cmdDesc{ getCommandsDescription(level) };

                if (!cmdDesc.empty()) {
                    retMessages.push_back(std::format("{}:", m_pluginName));
                    retMessages.push_back(cmdDesc);
                }
            }
        }
        else {
            // !help <command>: show detailed help for a specified command
            std::string paramCmd{ getAliasList().aliasToCommand(toLower(param)) };
            std::string errMsg;

            if (m_cmdInfos.contains(paramCmd)) {
                for (const CommandInfo& cmdInfo : m_cmdInfos[paramCmd]) {
                    if (player.checkCommandAccess(cmdInfo, errMsg)) {
                        retMessages.push_back(getCommandHelp(paramCmd, cmdInfo));
                        break;
                    }
                }
                // in case command access is denied for this player, send a specific message to the player
                if (!errMsg.empty()) {
                    retMessages.push_back(errMsg);
                }
            }
        }
        return retMessages;
    }

    //////// Command Handling ////////

    /// <summary>
    /// Get a description string for all commands of a specified operator level.
    /// </summary>
    /// <param name="level">Operator level</param>
    /// <returns>Description string for all commands of this level.</returns>
    std::string getCommandsDescription(OperatorLevel level)
    {
        std::string desc;

        // traverse all command infos
        for (auto& [name, cmdInfo] : m_cmdInfos) {
            // traverse all definitions for this command
            for (const CommandInfo& cmdInfoElem : cmdInfo) {
                if (cmdInfoElem.minLevel == level) {
                    desc += std::format(" !{}", name);
                    break;
                }
            }
        }

        if (!desc.empty()) {
            if (level >= OperatorLevel::Moderator)
                desc = std::format("lvl {}: {}", (int)level, desc);
            else
                desc = std::format("Basic: {}", desc);
        }
        return desc;
    }

private:
    // constants
    const uint16_t m_maxLvzObjects{ 25 };  // maximum size of an LVZ object queue

    // configuration objects
    Observable& m_obHost;  // host observable
    std::string m_pluginName;
    std::string m_arena;  // either a name or, in case of a public arena, a number
    bool m_isBillerOnline;
    std::string m_cmdLineParams;  // command line parameters of MERVBot
    RefParamMap m_refParams;  // storage for referenced initialization parameters from the host
    std::map<std::string, CommandHandler> m_cmdHandlers;  // command name as key
    CommandInfoMap m_cmdInfos;  // command name as key
    
    // state variables
    std::map<PlayerId, std::list<LvzObjectInfo>> m_lvzToggleObjects;
    std::map<PlayerId, std::list<LvzObject>> m_lvzModifyObjects;
    TickCounterMap m_tickCounters;
    bool m_isInitialized{};
};


/// <summary>
/// Spawn container. Apply this class to define and populate a spawn list within the custom createSpawn function of a 
/// plugin DLL.
/// </summary>
/// <typeparam name="SpawnT"></typeparam>
/// <typeparam name="ObservableT"></typeparam>
export template<typename SpawnT, typename ObservableT>
class SpawnList
{
public:
    /// <summary>
    /// Spawn a plugin for a specified host.
    /// </summary>
    /// <param name="hostName">Host name.</param>
    /// <param name="pluginName">Name of the plugin DLL.</param>
    /// <param name="obHost">Host.</param>
    /// <returns>Reference to the observable of the created spawn.</returns>
    ObservableT& create(std::string hostName, std::string pluginName, ObservableT& obHost)
    {
        // check that there doesn't already exist a spawn for the specified plugin and host
        for (std::string& id : m_spawns | std::views::keys) {
            if (id == hostName + pluginName) {
                throw std::invalid_argument(std::format("There already exists a spawn of plugin '{}' for the "
                    "specified host '{}'!", pluginName, hostName));
            }
        }
        // create the spawn for the specified host
        m_spawns.emplace_back(std::pair(hostName + pluginName, SpawnT{ pluginName, obHost }));
        return m_spawns.back().second;
    }

    /// <summary>
    /// Set up a spawn.
    /// </summary>
    /// <param name="hostName">Host name.</param>
    /// <param name="pluginName">Name of the plugin DLL.</param>
    void setup(std::string hostName, std::string pluginName)
    {
        for (auto& [id, spawn] : m_spawns) {
            if (id == hostName + pluginName) {
                spawn.setup();
            }
        }
    }

private:
    // the list contains all bots to ever be spawned of this type, some entries may no longer exist
    std::list<std::pair<std::string, SpawnT>> m_spawns;
};
