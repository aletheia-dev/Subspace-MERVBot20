export module ModuleBase;

export import BotEvent;
export import Player;

import <functional>;
import <map>;
import <string>;
import <list>;
import <utility>;
import <vector>;


// Map of tick counters and related handlers
export typedef std::map<std::string, std::pair<uint32_t, std::function<void()>>> TickCounterMap;
// Command reply message list
export typedef std::list<std::string> MessageList;
// Command handler
export typedef std::function<MessageList(const Player&, const Command&)> CommandHandler;


//////// Module Base ////////

/// <summary>
/// Abstract base class for plugin modules. Each plugin module holds a reference to the spawn and can register its
/// own event and command handlers.
/// </summary>
/// <typeparam name="SpawnT">Spawn type.</typeparam>
export template<typename TSpawn>
class ModuleBase
{
public:
    /// <summary>
    /// Constructor. Initialize the reference to the spawn instance.
    /// </summary>
    /// <param name="spawn"></param>
    ModuleBase(TSpawn& spawn) : m_spawn(spawn) {}

    /// <summary>
    /// Register event and command handlers.
    /// </summary>
    virtual void setup() = 0;

    /// <summary>
    /// Close the plugin module.
    /// </summary>
    virtual void close() = 0;

    //////// Event Handling ////////

    /// <summary>
    /// Notify all registered observers by calling the related event handlers with the given arguments.
    /// </summary>
    /// <typeparam name="...Args">Variable event argument types.</typeparam>
    /// <param name="type">Bot event type.</param>
    /// <param name="...args">Event handler arguments.</param>
    template<typename... Args>
    void raiseEvent(BotEvent type, Args&&... args)
    {
        m_spawn.raiseEvent(type, std::forward<Args>(args)...);
    }

    /// <summary>
    /// Register an event handler for an observer.
    /// </summary>
    /// <typeparam name="ObserverT">Obersver type.</typeparam>
    /// <typeparam name="...Args">Event parameter types.</typeparam>
    /// <param name="type">Bot event.</param>
    /// <param name="f">Event handler.</param>
    /// <param name="ob">Observing object to receive the event.</param>
    template<typename ObserverT, typename... Args>
    void registerEventHandler(BotEvent type, void(ObserverT::* f)(Args...), ObserverT* ob)
    {
        m_spawn.registerEventHandler(type, f, ob);
    }

    //////// Command Handling ////////

    /// <summary>
    /// Register a command handler for an observer.
    /// </summary>
    /// <typeparam name="ObserverT">Obersver type.</typeparam>
    /// <typeparam name="...Args">Command parameter types.</typeparam>
    /// <param name="name">Command name.</param>
    /// <param name="f">Command handler.</param>
    /// <param name="ob">Observing object to receive the command.</param>
    /// <param name="cmdInfos">Command infos.</param>
    template<typename ObserverT, typename... Args>
    void registerCommandHandler(std::string name, MessageList(ObserverT::* f)(Args...), ObserverT* ob,
        const std::list<CommandInfo>& cmdInfos)
    {
        m_spawn.registerCommandHandler(name, f, ob, cmdInfos);
    }

    //////// Accessors ////////

    const TSpawn& getSpawn()
    {
        return m_spawn;
    }

    const PlayerMap& getPlayers()
    {
        return m_spawn.getPlayers();
    }

    const FlagList& getFlags()
    {
        return m_spawn.getFlags();
    }

    const BrickList& getBricks()
    {
        return m_spawn.getBricks();
    }

    const Playfield getPlayfield()
    {
        return m_spawn.getPlayfield();
    }

    const ArenaSettings& getSettings()
    {
        return m_spawn.getSettings();
    }

    std::string_view getArenaName()
    {
        return m_spawn.getArenaName();
    }

    bool getIsBillerOnline()
    {
        return m_spawn.getIsBillerOnline();
    }

    std::string_view getCmdLineParams()
    {
        return m_spawn.getCmdLineParams();
    }

    const Player& getPlayer(PlayerId playerId)
    {
        return m_spawn.getPlayer(playerId);
    }

    Player& getMe()
    {
        return m_spawn.getMe();
    }

    std::string_view getPluginName()
    {
        return m_spawn.getPluginName();
    }

    template<typename ModuleT>
    std::string getModuleName(const ModuleT& mod)
    {
        return split(typeid(mod).name(), ' ')[1];
    }

    //////// Chat Messaging ////////

    void sendArena(std::string_view msg)
    {
        m_spawn.sendArena(msg);
    }

    void sendPrivate(PlayerId playerId, std::string_view msg)
    {
        m_spawn.sendPrivate(playerId, msg);
    }

    void sendPrivate(PlayerId playerId, ChatSoundCode snd, std::string_view msg)
    {
        m_spawn.sendPrivate(playerId, snd, msg);
    }

    void sendTeam(std::string_view msg)
    {
        m_spawn.sendTeam(msg);
    }

    void sendTeam(ChatSoundCode snd, std::string_view msg)
    {
        m_spawn.sendTeam(snd, msg);
    }

    void sendTeamPrivate(uint16_t team, std::string_view msg)
    {
        m_spawn.sendTeamPrivate(team, msg);
    }

    void sendTeamPrivate(uint16_t team, ChatSoundCode snd, std::string_view msg)
    {
        m_spawn.sendTeamPrivate(team, snd, msg);
    }

    void sendPublic(std::string_view msg)
    {
        m_spawn.sendPublic(msg);
    }

    void sendPublic(ChatSoundCode snd, std::string_view msg)
    {
        m_spawn.sendPublic(snd, msg);
    }

    void sendPublicMacro(std::string_view msg)
    {
        m_spawn.sendPublicMacro(msg);
    }

    void sendPublicMacro(ChatSoundCode snd, std::string_view msg)
    {
        m_spawn.sendPublicMacro(snd, msg);
    }

    void sendChannel(std::string_view msg)
    {
        m_spawn.sendChannel(msg);
    }

    void sendChannel(uint32_t channel, std::string_view msg)
    {
        m_spawn.sendChannel(channel, msg);
    }

    void sendRemotePrivate(std::string_view msg)
    {
        m_spawn.sendRemotePrivate(msg);
    }

    void sendRemotePrivate(std::string_view playerName, std::string_view msg)
    {
        m_spawn.sendRemotePrivate(playerName, msg);
    }

    //////// Tick Counter ////////

    /// <summary>
    /// Set a callback function of an expiration tick counter with a tick interval of one second.
    /// </summary>
    /// <param name="name">Counter name.</param>
    /// <param name="count">Number of ticks til expiration.</param>
    /// <param name="callback">Callback function.</param>
    void setTickCounter(std::string_view name, uint32_t count, std::function<void()> callback = [] {})
    {
        m_spawn.setTickCounter(name, count, callback);
    }

    /// <summary>
    /// Check if a tick counter has expired.
    /// </summary>
    /// <param name="name">Counter name.</param>
    /// <returns>True, if the counter with the given name has expired, otherwise false.</returns>
    bool isTickCounterExpired(std::string_view name)
    {
        return m_spawn.isTickCounterExpired(name);
    }

    //////// LVZ Objects ////////

    void toggleObjects(PlayerId playerId = UnassignedId)
    {
        m_spawn.toggleObjects(playerId);
    }

    void queueEnableObject(uint16_t objectId, PlayerId playerId = UnassignedId)
    {
        m_spawn.queueEnableObject(objectId, playerId);
    }

    void queueDisableObject(uint16_t objectId, PlayerId playerId = UnassignedId)
    {
        m_spawn.queueDisableObject(objectId, playerId);
    }

    void modifyObjects(PlayerId playerId = UnassignedId)
    {
        m_spawn.modifyObjects(playerId);
    }

    void queueModifyObject(LvzObject& object, PlayerId playerId = UnassignedId)
    {
        m_spawn.queueModifyObject(object, playerId);
    }

    void setObjectPos(uint16_t objectId, uint16_t x, uint16_t y, PlayerId playerId = UnassignedId)
    {
        m_spawn.setObjectPos(objectId, x, y, playerId);
    }

    void setObjectImage(uint16_t objectId, uint8_t imageId, PlayerId playerId = UnassignedId)
    {
        m_spawn.setObjectImage(objectId, imageId, playerId);
    }

    void setObjectImagePos(uint16_t objectId, uint8_t imageId, uint16_t x, uint16_t y, 
        PlayerId playerId = UnassignedId)
    {
        setObjectImagePos(objectId, imageId, x, y, playerId = UnassignedId);
    }

    //////// Misc ////////

    std::list<PlayerId> findPlayers(std::string_view namePrefix)
    {
        return m_spawn.findPlayers(namePrefix);
    }

    /// <summary>
    /// Execute a command issued either by a player or a user of an external chat client.
    /// </summary>
    /// <param name="player">Player.</param>
    /// <param name="cmd">Command.</param>
    /// <returns>Command reply to the issuer as a list of private messages.</returns>
    MessageList executeCommand(const Player& player, const Command& cmd)
    {
        return m_spawn.executeCommand(player, cmd);
    }

    /// <summary>
    /// Add a command info.
    /// </summary>
    /// <param name="cmdInfos">Command infos.</param>
    void addCommandInfos(std::string name, const std::list<CommandInfo>& cmdInfos)
    {
        m_spawn.addCommandInfos(name, cmdInfos);
    }

    /// <summary>
    /// Get the command infos.
    /// </summary>
    /// <returns>Map with command name as key and list of command infos as value.</returns>
    CommandInfoMap& getCommandInfos()
    {
        return m_spawn.getCommandInfos();
    }

    /// <summary>
    /// Show detailed help for a specific command.
    /// </summary>
    /// <param name="level">Operator level.</param>
    /// <param name="scope">Command scope.</param>
    /// <param name="command">Command name.</param>
    /// <returns>Command help as a list of strings.</returns>
    MessageList getCommandHelp(OperatorLevel level, CommandScope scope, std::string command)
    {
        return m_spawn.getCommandHelp(level, scope, command);
    }

    /// <summary>
    /// Get an informative arena description.
    /// </summary>
    /// <returns>Arena description.</returns>
    std::string getArenaDescription()
    {
        return m_spawn.getArenaDescription();
    }

private:
    TSpawn& m_spawn;
};
