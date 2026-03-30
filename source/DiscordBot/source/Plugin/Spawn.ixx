module;
#pragma warning(disable: 4251)
#include <dpp/dpp.h>
#include <filesystem>

export module Spawn;

export import SpawnBase;
import TransferSToD;
import TestHelper;

import <memory>;
import <format>;


const std::string PluginVersion = "1.0";


//////// Spawn ////////

/// <summary>
/// Spawn class. An instance of this class is created for each bot that is spawned to an arena.
/// </summary>
export class Spawn : public SpawnBase
{
public:
    /// <summary>
    /// Constructor. Creates a spawn for a given host observable.
    /// </summary>
    /// <param name="pluginName">Plugin name.</param>
    /// <param name="obHost">Observable host.</param>
    Spawn(std::string pluginName, Observable& hostOb) : SpawnBase(pluginName, hostOb) {}

    /// <summary>
    /// Register event and command handlers.
    /// </summary>
    void setup()
    {
        // register obligatory event and command handlers of the spawn base
        SpawnBase::setup();

        // read the configuration parameters for the spawn
        try {
            readConfigParams(std::format("{}.ini", getPluginName()));
        }
        catch (std::exception& ex) {
            raiseEvent(BotEvent::Error, ex.what());
            raiseEvent(BotEvent::Error, std::format("Failed to configure plugin {}!", getPluginName()));
            throw;
        }

        // register event handlers
        registerEventHandler(BotEvent::Init, &Spawn::handleBotEventInit, this);
        registerEventHandler(BotEvent::Term, &Spawn::handleBotEventTerm, this);

        // register command handlers
        // level 2 (moderator):
        registerCommandHandler("version", &Spawn::handleCommandVersion, this,
            { { OperatorLevel::Moderator, CommandScope::External, "display bot version" } });
        registerCommandHandler("about", &Spawn::handleCommandAbout, this,
            { { OperatorLevel::Player, CommandScope::External, "query me about my function" } });
    }

private:
    //////// Event handlers ////////

    void handleBotEventInit(uint64_t version, const PlayerMap& players, const FlagList& flags, 
        const BrickList& bricks, const Playfield& playfield, std::string_view cmdLineParams)
    {
        if (SpawnBase::isInitialized()) {
            try {
                // create a discord client with the gateway intent to scan message content, make sure to grant all 
                // privileged gateway intents in the discord development portal!
                static dpp::cluster cluster(m_botToken, dpp::i_default_intents | dpp::i_message_content);

                // create and setup the modules
                createModule<Spawn, TransferSToD, dpp::cluster>(this, cluster);
                createModule<Spawn, TestHelper>(this);

                raiseEvent(BotEvent::Echo, "Starting Discord client...");
                cluster.start();
            }
            catch (std::exception& ex) {
                raiseEvent(BotEvent::Exit, ex.what());
            }
        }
    }

    void handleBotEventTerm()
    {
        // shut down all plugin modules
        closeModules<Spawn>();

        raiseEvent(BotEvent::Echo, std::format("DLL plugin '{}' disconnected", getPluginName()));
    }

    //////// Command Handlers ////////

    MessageList handleCommandVersion(const Player& player, const Command& cmd)
    {
        MessageList retMessages;

        retMessages.push_back(std::format("{} v{}  (C)2023 by Aletheia", getPluginName(), PluginVersion));

        return retMessages;
    }

    MessageList handleCommandAbout(const Player& player, const Command& cmd)
    {
        MessageList retMessages;

        retMessages.push_back("Provides various features for the interconnection of Subspace and Discord.");

        return retMessages;
    }

    //////// Misc ////////

    /// <summary>
    /// Read the configuration parameters for the spawn from the plugin configuration file.
    /// </summary>
    /// <param name="fileName">Configuration file name.</param>
    void readConfigParams(std::string_view fileName)
    {
        // obtain the folder path of the bot dll and read the config parameters
        std::string filePath{ (std::filesystem::current_path() / fileName).string() };

        readConfigParam("Discord", "BotToken", m_botToken, filePath);
    }

private:
    // config parameters
    std::string m_botToken;  // Discord bot token
};
