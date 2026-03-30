module;
#pragma warning(disable: 4251)
#include <dpp/dpp.h>

export module TransferSToD;

import ModuleBase;

export class Spawn;


/// <summary>
/// Module provides functionality to transfer public, team and arena messages from Subspace to Discord.
/// </summary>
export class TransferSToD : public ModuleBase<Spawn>
{
public:
    /// <summary>
    /// Constructor. Initialize the plugin module base with a reference to the spawn and the Discord cluster.
    /// </summary>
    /// <param name="spawn">Spawn</param>
    /// <param name="cluster">Discord interface.</param>
    TransferSToD(Spawn& spawn, dpp::cluster& cluster) : ModuleBase(spawn), m_cluster(cluster) {}

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

    //////// Event handlers ////////

    /// <summary>
    /// Handle the Tick event. Flush the arena message buffer.
    /// </summary>
    void handleBotEventTick();

    /// <summary>
    /// Handle the ArenaEnter event. Set the arena status after leaving the arena.
    /// </summary>
    /// <param name="arena">Arena.</param>
    /// <param name="me">Bot.</param>
    /// <param name="isBillerOnline">True, if the biller is online, otherwise false.</param>
    void handleBotEventArenaEnter(std::string_view arena, Player& me, bool isBillerOnline);

    /// <summary>
    /// Handle the ArenaLeave event. Unset the arena status after leaving the arena.
    /// </summary>
    void handleBotEventArenaLeave();

    /// <summary>
    /// Handle the Chat event. Propagate the public, team or arena message to Discord.
    /// </summary>
    void handleBotEventChat(ChatMode chatMode, ChatSoundCode soundCode, const Player& player, std::string_view msg);

    //////// Misc ////////

    /// <summary>
    /// Read configuration parameters for this module from the plugin configuration file.
    /// </summary>
    /// <param name="fileName">Configuration file name.</param>
    void readConfigParams(std::string_view fileName);

    /// <summary>
    /// Read the message blacklist file.
    /// </summary>
    void readMessageBlacklist();

    /// <summary>
    /// Set the status flag for being in an arena. If the bot spawn is in an arena, get the related chat channel Id 
    /// from the configuration.
    /// </summary>
    /// <param name="isInArena">True, if the bot spawn is in an arena, otherwise false.</param>
    void setArenaStatus(bool isInArena);

    ///// <summary>
    ///// Callback function to receive the Discord role map.
    ///// </summary>
    ///// <param name="evt">Callback event.</param>
    //void rolesGetCallback(const dpp::confirmation_callback_t& evt);

    ///// <summary>
    ///// Replace all mentions of groups in a text message with their ids.
    ///// </summary>
    ///// <param name="msg">Text message.</param>
    ///// <returns>Message with replaced mentions.</returns>
    //std::string replaceMentions(std::string msg);

    ///// <summary>
    ///// Get the role Id for s specified role name.
    ///// </summary>
    ///// <param name="role">Role name.</param>
    ///// <returns>Either the role Id for the specified role name or 0, if the name is unknown.</returns>
    //dpp::snowflake getRoleId(std::string_view role);

private:
    // constants
    const uint32_t m_maxDiscordMessageLength{ 1970 };       // max allowed length of a Discord message
    const std::string m_commandPrefixes{ ".!@/" };      // prefixes for bot commands

    // config parameters
    dpp::snowflake m_serverId{};    // Discord server Id
    std::map<std::string, dpp::snowflake> m_arenaChatChannelIds;  // value format <channel Id>[:<arena>]
    dpp::snowflake m_publicMessageIconId{};     // Discord icon Id for a public message
    dpp::snowflake m_teamMessageIconId{};   // Discord icon Id for a team message
//    std::list<std::string> m_mentionBlacklist;      // mentions that must not appear in Discord
    std::list<std::string> m_messageBlacklist;      // messages that must not be propagated to Discord

    // state variables
    bool m_isInArena{};     // true, if the bot is in an arena
    dpp::snowflake m_chatChannelId{};  // Id of the Discord channel for message propagation

    //// container
//    dpp::role_map m_roles;  // Discord roles for translating mentions

    //// dynamic objects
    dpp::cluster& m_cluster;  // Discord interface

    //// message handling
    std::list<std::string> m_arenaMessageBuffer;    // buffer for arena messages
    bool m_isTableStarted = false;      // true, if processing of a stats table has been started
    bool m_isTableHeaderPrinted = false;    // true, if processing of a stats table header has been finished
    bool m_isTableBodyPrinted = false;      // true, if processing of a stats table body has been finished
    std::string m_tableSeparator;       // stats table separator line
};
