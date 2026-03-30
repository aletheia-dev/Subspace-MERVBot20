module;
#pragma warning(disable: 4251)
#include <dpp/dpp.h>

export module TransferDToS;

import ModuleBase;


export class Spawn;


/// <summary>
/// Module provides functionality to transfer public, team and arena messages from Discord to Subspace.
/// </summary>
export class TransferDToS : public ModuleBase<Spawn>
{
public:
    /// <summary>
    /// Constructor. Initialize the plugin module base with a reference to the spawn and the Discord cluster.
    /// </summary>
    /// <param name="spawn">Spawn</param>
    /// <param name="cluster">Discord interface.</param>
    TransferDToS(Spawn& spawn, dpp::cluster& cluster) : ModuleBase(spawn), m_cluster(cluster) {}

    /// <summary>
    /// Read config parameters, create the Discord bot client and register bot commands and event handlers.
    /// </summary>
    virtual void setup();

    /// <summary>
    /// Free all Discord ressources.
    /// </summary>
    virtual void close();

private:
    //////// Messaging ////////

    /// <summary>
    /// Process and send a public, team or arena message received from the host to the configured discord channel.
    /// </summary>
    /// <param name="msg">Text message.</param>
    /// <param name="chatMode">Chat mode.</param>
    /// <param name="player">Player name.</param>
    void sendMessage(std::string_view msg, ChatMode chatMode, std::string_view playerName = "");

    /// Send a public, team or arena message to the specified discord channel. The message is additionally formatted 
    /// to show an icon in Discord and truncated if necessary. For team and arena messages, the player is null.
    /// </summary>
    /// <param name="channelId">Id of the Discord channel to send the message to.</param>
    /// <param name="msg">Text message.</param>
    /// <param name="chatMode">Chat mode.</param>
    /// <param name="playerName">Player name.</param>
    /// <returns>Formatted message as sent to Discord.</returns>
    std::string sendMessage(dpp::snowflake channelId, std::string_view msg, ChatMode chatMode,
        std::string_view playerName = "");

    /// <summary>
    /// If no table messages are currently being transmitted, send all buffered arena messages to Discord.
    /// </summary>
    void tryFlushArenaMessages();

    /// <summary>
    /// Filter out arena messages that shall not be propagated to Discord.
    /// </summary>
    /// <param name="msg">Text message.</param>
    /// <returns>Either the same message or an empty string in case the message shall not be sent.</returns>
    std::string filterMessage(std::string_view msg);

    /// <summary>
    /// Format message to show public and team message icons in Discord.
    /// </summary>
    /// <param name="msg">Text message.</param>
    /// <param name="chatMode">Chat mode.></param>
    /// <param name="playerName">Player name.</param>
    /// <returns>Text message formatted for Discord.</returns>
    std::string formatDiscordMessage(std::string_view msg, ChatMode chatMode, std::string_view playerName = "");

    /// <summary>
    /// Process the arena message queue for the sending of formatted tables. Arena messages with tables are formatted 
    /// by flushing the queue and inserting extra separator lines.
    /// </summary>
    /// <param name="msg">Text message.</param>
    void processArenaMessages(std::string_view msg);

    /// <summary>
    /// Send all buffered arena messages to Discord. These arena messages get printed inside a box with constant font 
    /// width.
    /// </summary>
    void flushArenaMessages();

    //void sendDirectMessage(std::string_view msg, std::string_view playerName, std::string_view userName);

    //void updateMessage(dpp::message& discordMsg, std::string_view msg);

    //void saveSubspaceUserLinks();

    //void loadSubspaceUserLinks();

    //////// Event handlers ////////

    /// <summary>
    /// Handle the Tick event. Used to flush the arena message buffer.
    /// </summary>
    void handleBotEventTick();

    /// <summary>
    /// Handle the ArenaEnter event.
    /// </summary>
    /// <param name="arena">Arena.</param>
    /// <param name="me">Bot.</param>
    /// <param name="isBillerOnline">True, if the biller is online, otherwise false.</param>
    void handleBotEventArenaEnter(std::string_view arena, Player& me, bool isBillerOnline);

    /// <summary>
    /// Handle the ArenaLeave event. Used to the arena status after leaving the arena.
    /// </summary>
    void handleBotEventArenaLeave();

    /// <summary>
    /// Handle the Chat event.
    /// </summary>
    void handleBotEventChat(ChatMode chatMode, ChatSoundCode soundCode, const Player& player, std::string_view msg);

    //void handleBotEventPlayerDeath(const Player& player, const Player& killer, int32_t bounty, uint16_t flags);

    //void handleBotEventPlayerSpec(const Player& player, uint16_t oldTeam, Ship oldShip);

    //void handleBotEventPlayerTeam(const Player& player, uint16_t oldTeam, Ship oldShip);

    //void handleBotEventPlayerLeaving(const Player& player);

    //////// Misc ////////

    /// <summary>
    /// Read configuration parameters for this module from the plugin configuration file.
    /// </summary>
    /// <param name="fileName">Configuration file name.</param>
    void readConfigParams(std::string_view fileName);

    /// <summary>
    /// Set the status flag for being in an arena. If the bot spawn is in an arena, get the related chat channel Id 
    /// from the configuration.
    /// </summary>
    /// <param name="isInArena">True, if the bot spawn is in an arena, otherwise false.</param>
    void setArenaStatus(bool isInArena);

    /// <summary>
    /// Callback function to receive the Discord role map.
    /// </summary>
    /// <param name="evt">Callback event.</param>
    void rolesGetCallback(const dpp::confirmation_callback_t& evt);

    /// <summary>
    /// Replace all mentions of groups in a text message with their ids.
    /// </summary>
    /// <param name="msg">Text message.</param>
    /// <returns>Message with replaced mentions.</returns>
    std::string replaceMentions(std::string msg);

    /// <summary>
    /// Get the role Id for s specified role name.
    /// </summary>
    /// <param name="role">Role name.</param>
    /// <returns>Either the role Id for the specified role name or 0, if the name is unknown.</returns>
    dpp::snowflake getRoleId(std::string_view role);

    //void messageCreateCallback(const dpp::message_create_t& evt);

    //std::string getLinkedUserName(dpp::snowflake userId);

    ///// <summary>
    ///// Handle a command that has been entered in Discord.
    ///// </summary>
    ///// <param name="cmd">Command.</param>
    ///// <param name="params">Parameter definition.</param>
    ///// <param name="src">Command source.</param>
    //void handleCommand(std::string_view command, const dpp::parameter_list_t& params, dpp::command_source src);

    ///// <summary>
    ///// Process a bot command that is available to players in subspace.
    ///// </summary>
    ///// <param name="cmd">Command.</param>
    ///// <param name="params">Parameter definition.</param>
    ///// <param name="src">Command source.</param>
    ///// <param name="playerName">Linked discord user name.</param>
    //void processSubspaceBotCommand(std::string command, const dpp::parameter_list_t& params,
    //    const dpp::command_source& src, std::string playerName);

    ///// <summary>
    ///// Process a bot command that is available to Discord users only.
    ///// </summary>
    ///// <param name="cmd">Command.</param>
    ///// <param name="params">Parameter definition.</param>
    ///// <param name="src">Command source.</param>
    ///// <param name="userName">Discord user name.</param>
    //void processDiscordBotCommand(std::string_view command, const dpp::parameter_list_t& params,
    //    const dpp::command_source& src, std::string_view userName);

    //MessageList handleDiscordBotCommandPm(const dpp::parameter_list_t& params, std::string_view userName,
    //    bool isSlashCommand);

    ///// <summary>
    ///// Handle the Discord bot command 'players'. Gets the player table as a list of messages. The output is formatted
    ///// to fit nicely into a pinned Discord message box.
    ///// </summary>
    ///// <param name="params">Command parameters.</param>
    ///// <returns>List of messages.</returns>
    //MessageList handleDiscordBotCommandPlayers(const dpp::parameter_list_t& params);

private:
    // constants
    const uint32_t m_maxDiscordMessageLength{ 1970 };       // max allowed length of a Discord message
    const std::string m_commandPrefixes{ ".!@/" };      // prefixes for bot commands

    // config parameters
    dpp::snowflake m_serverId{};
    std::map<std::string, dpp::snowflake> m_arenaChatChannelIds;
    dpp::snowflake m_publicMessageIconId{};
    dpp::snowflake m_teamMessageIconId{};
    //bool m_createPlayersMessage{};
    //bool m_pinPlayersMessage{};
    //dpp::snowflake m_playersChannelId{};

    //// state variables
    bool m_isInArena{};
    dpp::snowflake m_chatChannelId{};  // Id of the Discord channel for message transer
    //// private direct message channels, key is player name
    //DMChannelMap m_privateDMChannels;
    //// If true, the playerlist should be updated due to an entering or leaving player
    //bool m_pendingPlayerListUpdate{};

    //// container
    dpp::role_map m_roles;
    //std::map<std::string, dpp::snowflake> m_linkRequests;
    //std::map<std::string, dpp::snowflake> m_linkedUsers;
    //dpp::members_container m_verifiedMembers;

    //// dynamic objects
    dpp::cluster& m_cluster;  // Discord interface
//    std::unique_ptr<dpp::commandhandler> m_commandHandler{};  // Discord command handler

    //// message handling
    std::list<std::string> m_arenaMessageBuffer;
    bool m_isTableStarted = false;
    bool m_isTableHeaderPrinted = false;
    bool m_isTableBodyPrinted = false;
    std::string m_tableSeparator;
    //std::string m_lastUpdatedPlayersMessage;
    //dpp::message m_chatChPlayersMessage;
    //dpp::message m_plmChPlayersMessage;
};
