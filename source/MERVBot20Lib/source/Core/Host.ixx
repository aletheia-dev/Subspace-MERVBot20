module;
#include <fstream>

export module Host;

export import BotStore;
export import BotInfo;
export import Observable;
export import Player;

import Chunk;
import Command;
import Encrypt;
import Sockets;

import <format>;
import <iostream>;
import <list>;
import <map>;
import <memory>;
import <vector>;


//////// Log packets to file ////////

#define S2C_LOG
#define C2S_LOG
#define CLUSTER_MODE

constexpr uint32_t MaxLogLines{ 50 };


//////// Messaging ////////

export struct HostMessage
{
    HostMessage(uint8_t* mmsg, size_t len, class Host* ssrc);

    // Return type byte
    uint8_t getType(bool special);

    std::vector<uint8_t> msg;    // trimmed buffer
    Host* src;    // sender of this message
};


export struct ClientMessage
{
    ClientMessage(size_t len);

    // Zero buffer
    void clear();

    std::vector<uint8_t> msg;    // message buffer
};


export struct ReliableMessage
{
    ReliableMessage(uint32_t ack_ID, uint8_t* mmsg, size_t llen);

    void setTime();     // update lastSend

    std::vector<uint8_t> msg;    // message with the reliable header
    uint32_t ACK_ID;        // ACK identifier
    uint64_t lastSend;      // last time it was sent
};


export struct ClusterMessage
{
    ClusterMessage(uint8_t* mmsg, size_t llen);

    std::vector<uint8_t> msg;    // message without the reliable header
};


//////// Logging ////////

export enum class LogColor
{
    Neutral,
    Warning,
    Error
};


export struct LoggedChat
{
    LoggedChat(ChatMode mmode, ChatSoundCode ssnd, uint16_t iident, std::string_view mmsg);

    bool operator == (const LoggedChat&) const& = default;

    ChatMode mode;
    ChatSoundCode snd;
    uint16_t ident;
    std::string msg;
};


//////// Jump Table ////////

template <typename AnonymousStruct>
class TJumpTable
{
    typedef void(__stdcall* template_func)(AnonymousStruct&);

public:
    // Initialize to zero
    TJumpTable()
    {
        clear();
    }

    // Reset list
    void clear()
    {
        for (int i = 0; i < 256; ++i)
            m_rpc[i] = NULL;
    }
    
    // Create a handler
    void add(uint8_t index, template_func function)
    {
        m_rpc[index] = function;
    }
    
    // Delete a handler
    void kill(uint8_t index)
    {
        m_rpc[index] = NULL;
    }
    
    // Call a handler (false == unhandled)
    bool call(uint8_t index, AnonymousStruct& params)
    {
        if (m_rpc[index]) {
            m_rpc[index](params);
            return true;
        }
        return false;
    }

private:
    template_func m_rpc[256];
};


//////// Host ////////

/// <summary>
/// Log level.
/// </summary>
export enum class LogLevel
{
    NoOutput,           // log level 0: no log output to the console
    Basic,              // log level 1: log the startup phase
    Verbose,            // log level 2: additionally log all messages of the bot
    All                 // log level 3: log all, including synchronization and other messages
};


/// <summary>
/// Plugin Host. There is a separate Host for each bot defined in Spawns.txt, which can hold one or more plugin Dlls.
/// </summary>
export class Host : public Observable
{
public:
    std::string creation_parameters;
    
    Host(BotInfo& bi);

    void shutdown();

    //////// Socket ////////

    UDPSocket socket;       // socket wrapper
    INADDR remote;      // sockaddr wrapper

    // Simply encrypt and send to m_host
    void send(uint8_t* msg, size_t len);

    // Inbox
    uint32_t remoteStep;        // next ACK_ID expected
    std::list<ReliableMessage> received;     // messages waiting for a lost packet
    
    // Process queued messages or queue another
    void checkReceived(uint32_t ACK_ID, uint8_t* msg, size_t len);

    // Send an acknowledgement
    void sendACK(uint32_t ACK_ID);

    // Outbox
    uint32_t localStep;     // next ACK_ID to be used
    uint32_t localSent;     // last ACK_ID to be sent
    std::list<ReliableMessage> sent;     // messages waiting for ACKnowledgement

    // Route ACK messages here
    void checkSent(uint32_t ACK_ID);

    // Resend the lost messages
    void sendQueued();

    std::list<ReliableMessage> queued;       // send-queue overflows go here

    // Start sending a queued message
    void sendNext();

    // Add to the queued list
    bool queue(uint8_t* msg, size_t len);
    
    // General Commands
    TJumpTable<HostMessage> generalRouter;      // server/Client protocol
    // Special Commands
    TJumpTable<HostMessage> specialRouter;      // core protocol
    
    // State
    uint64_t lastRecv;      // time since last packet was recv'd
    bool clustering;        // manage send() timing
    bool gotMap;        // has the map file
    bool gotTurf;       // got a turf update
    bool connecting;        // until we get an 00 02, send 00 01
    uint64_t lastConnect;
    bool syncing;       // until we get an 00 06, send 00 05
    uint64_t lastSync;
    std::string botChats;        // ?chat channels that the bot has subscribed to
    uint32_t dictionary[256];       // file checksum dictionary

    // Transfer modes
    ChunkBuffer little;     // 00 08 / 00 09
    ChunkBuffer large;      // 00 0A
    uint32_t ftLength;      // used to prevent DoS attacks (0 == no transfer)
    std::list<ClusterMessage> clustered;     // clustered messages queued for sending

    void sendClustered();

    // Message encryption
    SS_ENCR encryption;     // 00 01 / 00 02
    
    // Host communication
    friend class HostList;
    bool killMe;        // Tells hostList to clean up
    uint64_t lastIteration;     // Last time we had focus

    //////// Chatter ////////
    
    std::list<LoggedChat> chatLog;       // List of chat messages to-be-sent
    uint64_t lastChatSend;      // Last time segment we reset send count
    uint16_t countChatSend;     // Count since last reset

    std::list<std::string> loggedChatter;     // Log of bot chatter for !log
    
    void postChat(ChatMode mode, ChatSoundCode snd, PlayerId playerId, std::string_view msg);

    void tryChat(ChatMode mode, ChatSoundCode snd, PlayerId playerId, std::string_view msg);
    
    void addChatLog(ChatMode mode, ChatSoundCode snd, PlayerId playerId, std::string_view msg);
    
    bool sendNextLoggedChat();

    void doChatLog();
    
    // Route packets to the correct Host object
    // 
    // Did a message originate here?
    bool validateSource(INADDR& src);

    void doEvents();

    //////// Carpool messages ////////

    // Prepare for message carpooling
    void beginCluster();

    // Queue for clustering
    void post(uint8_t* msg, size_t len, bool reliable);

    // Queue for clustering
    void post(ClientMessage&& cm, bool reliable);

    // Post, release, reliable
    void postRR(ClientMessage&& cm);

    // Post, release, unreliable
    void postRU(ClientMessage&& cm);
    
    // Send queued, clustered messages
    void endCluster();
    
    // Synchronization
    uint32_t getHostTime();
    uint32_t getLocalTime(uint32_t time);

    uint32_t msgSent;       // Total distinct messages sent, 32 required for protocol
    uint32_t msgRecv;       // Total distinct messages recveived, 32 required for protocol
    uint64_t lastSyncRecv;      // Used by subspace to invalidate slow or fast time syncs
    int64_t timeDiff;       // Delta T between server and client - changes over time
    uint64_t syncPing;      // Average m_host response time to sync requests
    uint64_t accuPing;      // Ping time accumulator for average ping time
    uint64_t countPing;     // Ping count accumulator for average ping time
    uint64_t avgPing;       // Average ping time
    uint64_t highPing;      // Highest ping outlier
    uint64_t lowPing;       // Lowest ping outlier
  
    //////// Routing ////////

    void gotPacket(uint8_t* msg, size_t len);

    void gotMessage(uint8_t* msg, size_t len);

    void gotMessage(HostMessage& msg);

    void gotSpecialMessage(HostMessage& msg);

    //////// Core in ////////

    void gotEncryptRequest(uint32_t key, uint16_t prot);

    void gotEncryptResponse(uint32_t key);

    void gotEncryptResponse(uint32_t key, uint8_t mudge);

    void gotReliable(uint32_t id, uint8_t* msg, size_t len);

    void gotACK(uint32_t id);

    void gotSyncRequest(uint32_t time);

    void gotSyncRequest(uint32_t time, uint32_t sent, uint32_t recv);

    void gotSyncResponse(uint32_t pingTime, uint32_t pongTime);

    void gotDisconnect();

    void gotChunkBody(uint8_t* msg, size_t len);

    void gotChunkTail(uint8_t* msg, size_t len);

    void gotBigChunk(uint32_t total, uint8_t* msg, size_t len);

    void gotCancelDownload();

    void gotCancelDownloadAck();

    void gotCluster(uint8_t* msg, size_t len);

    //////// Core out ////////

    void connect(bool postDisconnect);

    void disconnect(bool notify);

    void syncClocks();

    void sendDownloadCancelAck();

    //////// Game protocol ////////

    // Disable password and enable game protocol
    void activateGameProtocol();

    // Clear and prepare game objects
    void resetArena();

    // Reset session completely
    void resetSession();

    // Update team-owned objects' map icons
    void resetIcons();

    uint64_t lastTick;
    friend class BotStore;
    std::unique_ptr<BotStore> imports;  // DLL imports

    //////// Bricks ////////

    BrickList bricks;

    bool brickExists(uint16_t ident);

    void doBrickEvents();

    void updateBrickTiles();

    //////// Goals ////////

    std::list<Goal> goals;

    void changeCoordinates();

    void changeGoalMode();

    void changeGoalTiles();

    //////// PBall ////////

    std::list<PowerBall> balls;

    PowerBall& findBall(uint16_t ident);

    void doBallEvents();
    
    //////// Flags ////////

    FlagList flags;  // only lists uncarried flags

    Flag& findFlag(uint16_t ident);

    void claimFlag(uint16_t flag, uint16_t playerId);

    void dropFlags(uint16_t playerId);

    void loadTurfFlags();

    void resetFlagTiles();
    
    //////// Chat ////////

    void sendPrivate(const Player& player, std::string_view msg);

    void sendPrivate(const Player& player, ChatSoundCode snd, std::string_view msg);
    
    void sendTeam(std::string_view msg);

    void sendTeam(ChatSoundCode snd, std::string_view msg);
    
    void sendTeamPrivate(uint16_t team, std::string_view msg);

    void sendTeamPrivate(uint16_t team, ChatSoundCode snd, std::string_view msg);

    void sendPublic(std::string_view msg);

    void sendPublic(ChatSoundCode snd, std::string_view msg);

    void sendPublicMacro(std::string_view msg);

    void sendPublicMacro(ChatSoundCode snd, std::string_view msg);

    void sendChannel(std::string_view msg);  // #;Message

    void sendChannel(uint32_t channel, std::string_view msg);

    void sendRemotePrivate(std::string_view msg);    // :Name:Message

    void sendRemotePrivate(std::string_view name, std::string_view msg);
    
    // Player

    PlayerMap players;
    std::list<PlayerId> playerIds;

    PlayerId playerId = UnassignedId;   // my player's ident

    /// <summary>
    /// Check if a bot player exists. This is the case, if a player ident has been assigned by the server and there 
    /// actually exists a player with this ident in the player map.
    /// </summary>
    /// <returns>True, if a bot player exists.</returns>
    bool hasBot()
    {
        return playerId != UnassignedId && hasPlayer(playerId);
    }

    bool hasPlayer(PlayerId playerId)
    {
        return players.contains(playerId);
    }

    Player& addPlayer(PlayerId playerId, std::string_view name, std::string_view squad, uint32_t flagPoints,
        uint32_t killPoints, uint16_t team, uint16_t wins, uint16_t losses, Ship ship, bool acceptsAudio, 
        uint16_t flagCount);
    
    Player& getPlayer(PlayerId playerId);
    
    Player& findPlayer(const std::string_view name);

    Player& me() {
        return players[playerId];
    };

    uint16_t follow;        // Followed player
    bool speccing;      // Attempting to spectate followed player?
    uint64_t lastSpec;      // Last time at which we tried to spec
    
    void spectateNext();

    void spectate(uint32_t playerId);

    void killPlayer(Player& player);

    void killTurret(Player& player);        // Shaking off turrets

    void killTurreter(Player& player);      // Detaching from a m_host

    //    Player *findTeammate(Player *excluded, uint16_t team);
    //    Player *findHighScorer(Player *excluded, uint16_t team);

    //    uint16_t teamPopulation(uint16_t team);

    // Reset player access for everyone under given level
    void revokeAccess(OperatorLevel access);

    // Reset player access for one player given name
    void revokeAccess(std::string_view name);
    
    //////// Statistics ////////

    BotInfo botInfo;

    uint32_t weaponCount;
    uint16_t S2CSlowCurrent;
    uint16_t S2CFastCurrent;
    uint16_t S2CSlowTotal;
    uint16_t S2CFastTotal;

    int32_t numTries;       // Number of tries remaining before he stops logging in
    bool billerOnline;      // Biller down? filter op's without passwords
    bool downloadingNews;       // Do not download the news file twice
    bool inZone;        // First time bot enters arena, then constant
    bool inArena;       // Always tracks whether bot is in arena or not
    bool paranoid;      // Required to send extra information?
    bool hasSysOp;      // Result of sysop check
    bool hasSMod;       // Result of smod check
    bool hasMod;        // Result of mod check
    bool position;      // Sending position update messages
    bool DLLFlying;     // Is the DLL in charge of the bot's position?
    bool allowLimited;      // Enable !ownbot !own !give commands
    char turretMode;        // 0:auto-turret, 1:controlled-turret
    uint16_t limitedOwnerId;        // Track Limited bot owner
    bool broadcastingErrors;        // Zone-error broadcast channel
    OperatorLevel lowestLevel;      // Lowest level getCommand access enabled
    uint64_t lastPosition;
    
    void sendPosition(bool reliable, uint32_t timestamp, Projectile ptype, WeaponLevel level, bool shrapBouncing, 
        uint8_t shrapLevel, uint8_t shrapCount, bool secondary);

    void sendPosition(bool reliable);

    void sendPosition(uint32_t timestamp, bool reliable);

    // Up time
    uint64_t arenaJoinTime, zoneJoinTime;

    //////// Arena state ////////

    bool loadedFlags;    // Loaded turf flags?
    PlayfieldTileFormat playfieldTiles[TileMaxLinear];
    ArenaSettings settings;

    void changeArena(std::string_view name);

    //////// Logging ////////
    
    /// <summary>
    /// Write a message to the console and optionally (config parameter ChatterLog) to a file. The message will be
    /// logged, if the required log level is less or equal to the configured log level (config parameter LogLevel).
    /// </summary>
    /// <param name="msg">Message to be logged.</param>
    /// <param name="level">Log level of the message.</param>
    /// <param name="color">Log color.</param>
    void logEvent(std::string_view msg, LogLevel level = LogLevel::Basic, LogColor color = LogColor::Neutral);

    template<typename... Args>
    void logEvent(const std::format_string<Args...> fmt, Args&&... args)
    {
        logEvent(std::format(fmt, std::forward<Args>(args)...));
    }
    
    void logWarning(std::string_view msg)
    {
        logEvent(msg, LogLevel::Basic, LogColor::Warning);
    }

    template<typename... Args>
    void logWarning(const std::format_string<Args...> fmt, Args&&... args)
    {
        logWarning(std::format(fmt, std::forward<Args>(args)...));
    }

    void logError(std::string_view msg)
    {
        logEvent(msg, LogLevel::Basic, LogColor::Error);
    }

    template<typename... Args>
    void logError(const std::format_string<Args...> fmt, Args&&... args)
    {
        logError(std::format(fmt, std::forward<Args>(args)...));
    }

    // Dump to disk
    void writeLogBuffer();

    // Append to Logins.txt
    void logLogin();

    void logIncoming(std::vector<uint8_t>& packet, size_t len);

    void logOutgoing(std::vector<uint8_t>& packet, size_t len);

    std::ofstream logFile;     // Log file for this bot instance
    std::string logBuffer;       // Buffered log data
    uint64_t lastLogTime;       // Last time we wrote the buffer to the disk
    bool logging;       // Writing to Chatter.log?

    //////// Misc ////////

    std::string getTimeString();
};


/// <summary>
/// Container for host instances.
/// </summary>
export class HostList
{
public:
    /// <summary>
    /// Create a host and establish the server connection.
    /// </summary>
    /// <param name="botInfo">Bot info.</param>
    /// <returns>True, if the host was successfully created.</returns>
    bool connectHost(BotInfo& botInfo)
    {
        if (!findSpawn(botInfo.name)) {
            std::shared_ptr<Host> host{ std::make_shared<Host>(botInfo) };
            uint32_t ip = botInfo.ip;

            if ((ip & 0x000000ff) == 0x0000007f) {
                // 127.x.x.x 
                ip += m_mix_ctr;

                if ((ip & 0xff000000) == 0) {
                    ip += 0x01000000;
                    m_mix_ctr += 0x01000000;
                }
                botInfo.ip = ip;
                m_mix_ctr += 0x00000100;
            }

            if (!host->killMe) {
                // the pugin was successfully initialized, establish the connection
                host->connect(false);
                m_hosts.push_back(host);
                return true;
            }
        }
        else {
            std::cout << std::format("Failed to spawn '{}'. A spawn with this name already exists!\n", botInfo.name);
        }
        return false;
    }

    void disconnectAll()
    {
        for (auto& host : m_hosts) {
            host->disconnect(true);
        }
    }

    void clear()
    {
        for (auto& host : m_hosts) {
            host->shutdown();
        }
        m_hosts.clear();
    }

    void restartAll()
    {
        for (auto& host : m_hosts) {
            // simulate a server disconnect
            host->disconnect(true);
            host->gotDisconnect();
            host->killMe = false;
        }
    }

    uint32_t getConnections()
    {
        return (uint32_t)m_hosts.size();
    }

    std::shared_ptr<Host> findSpawn(std::string_view name)
    {
        for (auto& host : m_hosts) {
            if (host->botInfo.name == name) {
                return host;
            }
        }
        return std::shared_ptr<Host>{};
    }

    void doEvents()
    {
        std::list<std::shared_ptr<Host>> deleteHosts;

        for (auto& host : m_hosts) {
            host->doEvents();

            if (host->killMe) {
                deleteHosts.push_back(host);
            }
        }

        // delete the marked hosts
        for (auto& host : deleteHosts) {
            host->shutdown();
            m_hosts.remove(host);
        }
    }

    const std::list<std::shared_ptr<Host>>& getSpawns()
    {
        return m_hosts;
    }

private:
    std::list<std::shared_ptr<Host>> m_hosts;
    uint32_t m_mix_ctr{};
};


/// <summary>
/// Get the host list singleton.
/// </summary>
/// <returns>Host list.</returns>
export HostList& getHostList()
{
    static HostList hosts;

    return hosts;
}
