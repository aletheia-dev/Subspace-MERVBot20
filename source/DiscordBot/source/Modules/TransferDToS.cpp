module;
#pragma warning(disable: 4251)
#include <dpp/dpp.h>
#include <filesystem>
#include <variant>

module TransferDToS;

import Spawn;

import <format>;
import <iostream>;
import <list>;
//import <map>;
import <ranges>;
import <set>;
import <utility>;


// Blacklist of substrings of arena messages which should not be transmitted to discord
const std::list<std::string> _MessageBlacklist{
    "master of chaos",                  // arena spam message
    "caplympics champion",              // arena spam message
    "league forum",                     // arena spam message
    "this arena is continuum-only",     // initial server message
    "you have been disconnected",       // disconnect message
    "reliable kill messages",           // reply to *relkills
};

// Blacklist of mentions that should be filtered out from messages to discord
const std::set<std::string> _MentionBlacklist{
    "@everyone"
};

// Special characters used as prefixes for bot commands executed from Discord
const std::set<std::string> _CommandPrefix{ ".", "!", "@", "/" };

//const std::string UserLinksFilename = "links.txt";


/// <summary>
/// Read config parameters, create the Discord bot client and register bot commands and event handlers.
/// </summary>
void TransferDToS::setup()
{
    // read configuration parameters for this module from the plugin configuration file
    try {
        readConfigParams(std::format("{}.ini", getPluginName()));
    }
    catch (std::exception ex) {
        raiseEvent(BotEvent::Error, std::format("Failed to initialize module {} of plugin {}!", getModuleName(this),
            getPluginName()));
        throw;
    }

    // register for m_host events to observe
//    registerEventHandler(BotEvent::Tick, &TransferDToS::handleBotEventTick, this);
//    registerEventHandler(BotEvent::ArenaEnter, &TransferDToS::handleBotEventArenaEnter, this);
    //registerEventHandler(BotEvent::ArenaEnter, &SendToDiscord::handleBotEventArenaLeave, this);
//    registerEventHandler(BotEvent::Chat, &TransferDToS::handleBotEventChat, this);
    //registerEventHandler(BotEvent::PlayerDeath, &Discord::handleBotEventPlayerDeath, this);
    //registerEventHandler(BotEvent::PlayerSpec, &Discord::handleBotEventPlayerSpec, this);
    //registerEventHandler(BotEvent::PlayerTeam, &Discord::handleBotEventPlayerTeam, this);
    //registerEventHandler(BotEvent::PlayerLeaving, &Discord::handleBotEventPlayerLeaving, this);

    //// add command infos for help
    //// level 0:
    //addCommandInfos("players", {
    //    { OperatorLevel::Player, CommandScope::External,
    //        "Usage: /players  (show a player table with stats of the active players)",
    //        getAliasesDescription("players"), true }
    //    });
    //addCommandInfos("link", {
    //    { OperatorLevel::Player, CommandScope::External,
    //        "Usage: /link  (link a Discord account to a Subspace player)",
    //        getAliasesDescription("link"), true }
    //    });
    //addCommandInfos("unlink", {
    //    { OperatorLevel::Player, CommandScope::External,
    //        "Usage: /unlink  (unlink a Discord account from a Subspace player)",
    //        getAliasesDescription("unlink"), true }
    //    });
    //addCommandInfos("pm", {
    //    { OperatorLevel::Player, CommandScope::External,
    //        "Usage: /pm <player>:<message>  (send a private message to a Subspace player)",
    //        getAliasesDescription("pm"), true, false, "Player name and message." }
    //    });

    //m_commandHandler = new dpp::commandhandler(m_cluster.get());

    //for (std::string_view cmdPrefix : _CommandPrefix) {
    //    m_commandHandler->add_prefix(cmdPrefix);
    //}

    // register all bot commands as discord slash-commands
    m_cluster.on_ready([this](const dpp::ready_t& evt) {
    //    // register all bot commands that can be used in discord
    //    for (auto& [name, cmdInfos] : getCommandInfos()) {
    //        for (CommandInfo& cmdInfo : cmdInfos) {
    //            // only consider commands that can be used both in continuum and in an external chat client
    //            if (cmdInfo.maxScope >= CommandScope::External) {
    //                m_commandHandler->add_command(name,
    //                    { { "param", dpp::param_info(dpp::pt_string, cmdInfo.isOptional, cmdInfo.paramHelp) } },
    //                    [this](std::string_view cmd, const dpp::parameter_list_t& params, dpp::command_source src) {
    //                        this->handleCommand(cmd, params, src); },
    //                        cmdInfo.description, m_serverId);
    //                break;
    //            }
    //        }
    //    }
    //    m_commandHandler->register_commands();

        // get the avaiable roles for replacing the related ids in messages
        m_cluster.roles_get(m_serverId, [this](const dpp::confirmation_callback_t& evt) {
            this->rolesGetCallback(evt); });

    //    // register a handler for created messages
    //    m_cluster.on_message_create([this](const dpp::message_create_t& evt) {
    //        this->messageCreateCallback(evt); });
        });

    //loadSubspaceUserLinks();
}


/// <summary>
/// Delete the Discord players message, command handlers and cluster.
/// </summary>
void TransferDToS::close()
{
    //if (m_plmChPlayersMessage.id) {
    //    try {
    //        m_cluster.message_delete_sync(m_plmChPlayersMessage.id, m_playersChannelId);

    //        if (m_commandHandler) {
    //            delete m_commandHandler;
    //        }
    //    }
    //    catch (std::exception& ex) {
    //        std::cout << "ERROR: Failed to delete message " << m_plmChPlayersMessage.id << ": " << ex.what() << std::endl;
    //    }
    //}

    //m_plmChPlayersMessage = dpp::message();

    m_arenaMessageBuffer.clear();
    m_isTableStarted = false;
    m_isTableHeaderPrinted = false;
    m_isTableBodyPrinted = false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Read configuration parameters for this module from the plugin configuration file.
/// </summary>
/// <param name="fileName">Configuration file name.</param>
void TransferDToS::readConfigParams(std::string_view fileName)
{
    // obtain the folder path of the bot dll and read the config parameters
    std::string filePath{ (std::filesystem::current_path() / fileName).string() };

    readConfigParam("Discord", "ChatChannelIds", m_arenaChatChannelIds, filePath);
    readConfigParam("Discord", "PublicMessageIconId", m_publicMessageIconId, filePath);
    readConfigParam("Discord", "TeamMessageIconId", m_teamMessageIconId, filePath);
    //isValid &= readConfigParam("Discord", "ServerId", m_serverId, filePath);
    //isValid &= readConfigParam("Discord", "CreatePlayersMessage", m_createPlayersMessage, filePath, "0");
    //isValid &= readConfigParam("Discord", "PinPlayersMessage", m_pinPlayersMessage, filePath, "0");
    //isValid &= readConfigParam("Discord", "PlayersMessageChannelId", m_playersChannelId, filePath, "0");
}


/// <summary>
/// Set the status flag for being in an arena. If the bot spawn is in an arena, get the related chat channel Id from 
/// the configuration.
/// </summary>
/// <param name="isInArena">True, if the bot spawn is in an arena, otherwise false.</param>
void TransferDToS::setArenaStatus(bool isInArena)
{
    m_isInArena = isInArena;

    // get the chat channel Id for the arena we are in
    m_chatChannelId = 0;

    if (isInArena) {
        std::string arena{ getArenaName() };

        if (m_arenaChatChannelIds.contains(arena)) {
            // bot entered an arena for which a chat channel is configured
            m_chatChannelId = m_arenaChatChannelIds[arena];
        }
    }
}


//////// Messaging ////////

/// <summary>
/// Process and send a public, team or arena message received from the host to the configured discord channel.
/// </summary>
/// <param name="msg">Text message.</param>
/// <param name="mode">Chat mode.</param>
/// <param name="player">Player name.</param>
void TransferDToS::sendMessage(std::string_view msg, ChatMode chatMode, std::string_view playerName)
{
    if (m_isInArena) {
        if (chatMode == ChatMode::Public || chatMode == ChatMode::Team) {
            // not an arena message, so send all buffered arena messages to discord
            flushArenaMessages();
            // send the current message to discord
            sendMessage(m_chatChannelId, filterMessage(msg), chatMode, playerName);
        }
        else if (chatMode == ChatMode::Arena) {
            // store the arena message to the buffer until we receive a non-arena message
            processArenaMessages(filterMessage(msg));
        }
    }
}


/// Send a public, team or arena message to the specified discord channel. The message is additionally formatted to 
/// show an icon in Discord and truncated if necessary. For team and arena messages, the player is null.
/// </summary>
/// <param name="channelid">Id of the discord channel to send the message to.</param>
/// <param name="msg">Text message.</param>
/// <param name="mode">Chat mode.</param>
/// <param name="player">Player name.</param>
/// <returns>Formatted message as sent to discord.</returns>
std::string TransferDToS::sendMessage(dpp::snowflake channelId, std::string_view msg, ChatMode chatMode,
    std::string_view playerName)
{
    if (msg.empty() || !channelId) {
        return std::string(msg);
    }

    // truncate the message to it's maximum length, convert it to ascii and format it for output in discord according 
    // the the chat mode
    std::string formatMsg{ formatDiscordMessage(msg, chatMode, playerName) };

    try {
        dpp::message discordMsg{ channelId, formatMsg };

        m_cluster.message_create(discordMsg);
    }
    catch (...) {
        // just for the case dpp causes a problem
        std::cout << std::format("ERROR: DPP failed to create message: {}", formatMsg) << std::endl;
    }
    return formatMsg;
}


/// <summary>
/// If no table messages are currently being transmitted, send all buffered arena messages to Discord.
/// </summary>
void TransferDToS::tryFlushArenaMessages()
{
    // flush only if our link to discord is up and we are not transmitting a table right now
    if (m_isInArena && !m_isTableStarted) {
        flushArenaMessages();
    }
}


/// <summary>
/// Filter out messages that shall not be propagated to discord.
/// </summary>
/// <param name="msg">Text message.</param>
/// <returns>Either the same message or an empty string in case the message shall not be sent.</returns>
std::string TransferDToS::filterMessage(std::string_view msg)
{
    std::string filteredMsg{ msg };

    // filter out arena messages that occur as reply to MERVBot requesting *listmod 
    if (toLower(filteredMsg).ends_with(std::format(" - {}", toLower(getArenaDescription())))) {
        return {};
    }
    // filter out messages to discord that start with a blacklisted character
    if (filteredMsg.empty() || _CommandPrefix.contains(filteredMsg.substr(0, 1))) {
        return {};
    }
    // filter out messages that contain a blacklisted substring
    for (std::string ignoreSubstr : _MessageBlacklist) {
        if (toLower(filteredMsg).find(ignoreSubstr) != std::string::npos) {
            return {};
        }
    }

    // filter out messages that contain blacklisted discord mentions
    size_t index = 0;

    for (std::string filterMention : _MentionBlacklist) {
        while ((index = toLower(filteredMsg).find(filterMention, index)) != std::string::npos) {
            filteredMsg.replace(index, filterMention.length(), "");
        }
    }
    return filteredMsg;
}


/// <summary>
/// Format message to show public and team message icons in Discord.
/// </summary>
/// <param name="msg">Text message.</param>
/// <param name="chatMode"Chat mode.></param>
/// <param name="playerName">Player name.</param>
/// <returns>Text message formatted for Discord.</returns>
std::string TransferDToS::formatDiscordMessage(std::string_view msg, ChatMode chatMode, std::string_view playerName)
{
    // truncate the message to it's maximum length and convert it to ascii
    std::string formatMsg = replaceMentions(convertStringToASCII(msg.substr(0, m_maxDiscordMessageLength)));

    // send the message to discord in accordance with it's type
    if (chatMode == ChatMode::Arena) {
        formatMsg = std::format("```ansi\n{}```", formatMsg);
    }
    else if (!playerName.empty()) {
        formatMsg = std::format("**{}> **{}", playerName, formatMsg);

        if (chatMode == ChatMode::Public) {
            if (m_publicMessageIconId) {
                formatMsg = std::format("<:publicmsg:{}>{}", m_publicMessageIconId, formatMsg);
            }
        }
        else if (chatMode == ChatMode::Team) {
            if (m_teamMessageIconId) {
                formatMsg = std::format("<:teammsg:{}>{}", m_teamMessageIconId, formatMsg);
            }
        }
    }
    return formatMsg;
}


/// <summary>
/// Process the arena message queue for the sending of formatted tables. arena messages with tables are formatted by 
/// flushing the queue and inserting extra separator lines.
/// </summary>
/// <param name="msg">text message.</param>
void TransferDToS::processArenaMessages(std::string_view msg)
{
    bool flushMessage = false;

    if (!m_isTableStarted) {
        // output of a final score, take care to show this in a separate message box
        if (msg.starts_with("Final Score :")) {
            flushArenaMessages();
        }
        else if (msg.starts_with("+---")) {
            // upper border of a table, store it for use as a separator
            m_isTableStarted = true;
            m_tableSeparator = msg;
            flushArenaMessages();
        }
        else if (msg.starts_with("LVP: ")) {
            // flush the message queue at the end of mvp/lvp infos that follow a stats table
            flushMessage = true;
        }
        //else if (msg.find("players needed for a 4v4 practice") != std::string::npos) {
        //    // a reply to someone's !spam command, mention discord users of the @4v4prac group
        //    dpp::snowflake role4v4pracId{ getRoleId("4v4prac") };

        //    if (role4v4pracId) {
        //        // must be sent as a public message, otherwise the group mention will not be shown
        //        sendMessage(m_chatChannelId, std::format("<@&{}> ```{}```", role4v4pracId, msg), 
        //            ChatMode::Public);
        //        return;
        //    }
        //}
    }
    else {
        if (msg.starts_with("+---")) {
            if (!m_isTableHeaderPrinted) {
                // separator after a table header, now comes the table body
                m_isTableHeaderPrinted = true;
                m_isTableBodyPrinted = false;
            }
            else {
                // lower border of the current table
                m_isTableBodyPrinted = true;
            }
        }
        else if (msg.starts_with("| F ")) {
            if (m_isTableBodyPrinted) {
                // another header within the same table, add another separator
                flushArenaMessages();
                m_arenaMessageBuffer.push_back(m_tableSeparator);
            }
            m_isTableHeaderPrinted = false;
        }
        else if (m_isTableHeaderPrinted && m_isTableBodyPrinted) {
            // the current line succeeds a finished table body and is not another header, the 
            // table is finished
            m_isTableHeaderPrinted = false;
            m_isTableBodyPrinted = false;
            m_isTableStarted = false;
            flushArenaMessages();
        }
    }

    // queue the current arena message and flush at the end of a stats table for faster output
    m_arenaMessageBuffer.push_back(std::string(msg));

    if (flushMessage) {
        flushArenaMessages();
    }
}


/// <summary>
/// Send all buffered arena messages to discord. these arena messages get printed inside a box with constant font 
/// width.
/// </summary>
void TransferDToS::flushArenaMessages()
{
    if (!m_arenaMessageBuffer.empty()) {
        sendMessage(m_chatChannelId, join(m_arenaMessageBuffer), ChatMode::Arena);
        m_arenaMessageBuffer.clear();
    }
}


//////// Event handlers ////////

/// <summary>
/// Handle the Tick event. Used to flush the arena message buffer.
/// </summary>
void TransferDToS::handleBotEventTick()
{
    // flush all buffered arena messages at least once a second
    tryFlushArenaMessages();
    // update the player list message if applicable
//    updatePlayersMessage();
}


/// <summary>
/// Handle the ArenaEnter event.
/// </summary>
/// <param name="arena">Arena.</param>
/// <param name="me">Bot.</param>
/// <param name="isBillerOnline">True, if the biller is online, otherwise false.</param>
void TransferDToS::handleBotEventArenaEnter(std::string_view arena, Player& me, bool isBillerOnline)
{
    setArenaStatus(true);
    // create an initial players list, if there are players in the bot's arena
    //updatePlayersMessage();
}


/// <summary>
/// Handle the ArenaLeave event. Used to the arena status after leaving the arena.
/// </summary>
void TransferDToS::handleBotEventArenaLeave()
{
    //if (m_chatChPlayersMessage.id) {
    //    try {
    //        m_cluster.message_delete_sync(m_chatChPlayersMessage.id, m_chatChannelId);
    //    }
    //    catch (std::exception& ex) {
    //        std::cout << "ERROR: Failed to delete message " << m_chatChPlayersMessage.id << ": " << ex.what() << std::endl;
    //    }
    //}

    //m_chatChPlayersMessage = dpp::message();
    //m_lastUpdatedPlayersMessage.clear();
    setArenaStatus(false);
}


/// <summary>
/// Handle the Chat event.
/// </summary>
void TransferDToS::handleBotEventChat(ChatMode chatMode, ChatSoundCode soundCode, const Player& player, 
    std::string_view msg)
{
    sendMessage(msg, chatMode, player.getName());
}


//////// Misc ////////

/// <summary>
/// Callback function to receive the Discord role map.
/// </summary>
/// <param name="evt">Callback event.</param>
void TransferDToS::rolesGetCallback(const dpp::confirmation_callback_t& evt)
{
    m_roles = std::get<dpp::role_map>(evt.value);

    //// get verified user names to check abbreviations in private messages to discord users
    //for (auto& [roleId, userRole] : m_roles) {
    //    if (userRole.name == "verified") {
    //        m_verifiedMembers = userRole.get_members();
    //        break;
    //    }
    //}

}


/// <summary>
/// Replace all mentions of groups in a text message with their ids.
/// </summary>
/// <param name="msg">Text message.</param>
/// <returns>Message with replaced mentions.</returns>
std::string TransferDToS::replaceMentions(std::string msg)
{
    if (msg.find(" @") != std::string::npos) {
        for (auto& roleInfo : m_roles | std::views::values) {
            size_t ix = msg.find(" @" + roleInfo.name);

            if (ix != std::string::npos) {
                std::string repRoleId = std::format("<@&{}>", roleInfo.id);

                msg = msg.replace(ix, roleInfo.name.length() + 2, repRoleId);
            }
        }
    }
    return msg;
}


/// <summary>
/// Get the role Id for s specified role name.
/// </summary>
/// <param name="role">Role name.</param>
/// <returns>Either the role Id for the specified role name or 0, if the name is unknown.</returns>
dpp::snowflake TransferDToS::getRoleId(std::string_view role)
{
    for (auto& roleInfo : m_roles | std::views::values) {
        if (roleInfo.name == role) {
            return roleInfo.id;
        }
    }
    return 0;
}
