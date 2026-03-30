export module BotEvent;

import <functional>;
import <list>;
import <map>;


export enum class BotEvent
{
    //////// Host->Spawn ////////

    /// <summary>
    /// Initialize a bot plugin.
    ///     uint32_t                    version
    ///     const PlayerMap&            players
    ///     const FlagList&             flags
    ///     const BrickList&            bricks
    ///     const Playfield&            playfield
    ///     string_view                 cmdLineParams
    /// </summary>
    Init,

    /// <summary>
    /// Tick once per second.
    /// </summary>
    Tick,

    /// <summary>
    /// Bot entering arena.
    ///     string_view                 arena
    ///     Player&                     me
    ///     bool                        isBillerOnline
    /// </summary>
    ArenaEnter,

    /// <summary>
    /// Bot leaving arena.
    /// </summary>
    ArenaLeave,

    /// <summary>
    /// Arena settings.
    ///     ArenaSettings&              settings
    /// </summary>
    ArenaSettings,

    /// <summary>
    /// N-th arena name of a transmitted arena list.
    ///     string_view                 arena
    ///     bool                        isCurrentArena  // is this the bot's arena?
    ///     uint16_t                    population
    /// </summary>
    ArenaListEntry,

    /// <summary>
    /// Last arena name of a transmitted arena list.
    ///     string_view                 arena
    ///     bool                        isCurrentArena  // is this the bot's arena?
    ///     uint16_t                    population
    /// </summary>
    ArenaListEnd,

    /// <summary>
    /// Chat message received.
    ///     ChatMode                    chatMode
    ///     ChatSoundCode               soundCode
    ///     const Player&               player
    ///     string_view                 message
    /// </summary>
    Chat,

    /// <summary>
    /// Command received.
    ///     const Player&               player
    ///     const Command&              getCommand
    /// </summary>
    Command,

    /// <summary>
    /// Turreter attached to turretee.
    ///     const Player&               turreter
    ///     const Player&               turretee
    /// </summary>
    CreateTurret,

    /// <summary>
    /// Turreter detached from turretee.
    ///     const Player&               turreter
    ///     const Player&               turretee
    /// </summary>
    DeleteTurret,

    /// <summary>
    /// Player entering arena.
    ///     const Player&               player
    /// </summary>
    PlayerEntering,

    /// <summary>
    /// Player leaving arena.
    ///     const Player&               player
    /// </summary>
    PlayerLeaving,

    /// <summary>
    /// Player moved.
    ///     const Player&               player
    /// </summary>
    PlayerMove,

    /// <summary>
    /// Fired weapon update.
    ///     const Player&               player
    ///     WeaponInfo                  weaponInfo
    /// </summary>
    PlayerWeapon,

    /// <summary>
    /// Damage update.
    ///     const Player&               player
    ///     const Player&               killer
    ///     WeaponInfo                  weaponInfo
    ///     uint16_t                    energy
    ///     uint16_t                    damage
    /// </summary>
    WatchDamage,

    /// <summary>
    /// Player died.
    ///     const Player&               player
    ///     const Player&               killer
    ///     int32_t                     bounty
    ///     uint16_t                    flags
    /// </summary>
    PlayerDeath,

    /// <summary>
    /// Player received a prize.
    ///     const Player&               player
    ///     uint16_t                    prize
    /// </summary>
    PlayerPrize,

    /// <summary>
    /// Player score resetted.
    ///     const Player&               player
    /// </summary>
    PlayerScore,

    /// <summary>
    /// Player changed ship.
    ///     const Player&               player
    ///     uint16_t                    oldTeam
    ///     Ship                        oldShip
    /// </summary>
    PlayerShip,

    /// <summary>
    /// Player changed to spectator mode.
    ///     const Player&               player
    ///     uint16_t                    oldTeam
    ///     Ship                        oldShip
    /// </summary>
    PlayerSpec,

    /// <summary>
    /// Player changed team.
    ///     const Player&               player
    ///     uint16_t                    oldTeam
    ///     Ship                        oldShip
    /// </summary>
    PlayerTeam,

    /// <summary>
    /// Player changed banner.
    ///     const Player&               player
    /// </summary>
    BannerChanged,

    /// <summary>
    /// Player resetted ship.
    /// </summary>
    SelfShipReset,

    /// <summary>
    /// Player changed to UFO mode.
    /// </summary>
    SelfUFO,

    /// <summary>
    /// Player granted himself a prize.
    ///     uint16_t                    prize
    ///     uint16_t                    count
    /// </summary>
    SelfPrize,

    /// <summary>
    /// Player grabbed a flag.
    ///     const Player&               player
    ///     const Flag&                 flag
    /// </summary>
    FlagGrab,
    
    /// <summary>
    /// Player dropped a flag.
    ///     const Player&               player
    /// </summary>
    FlagDrop,

    /// <summary>
    /// Flag moved.
    ///     const Flag&                 flag
    /// </summary>
    FlagMove,

    /// <summary>
    /// Team won flag game.
    ///     uint16_t                    team
    ///     uint32_t                    points
    /// </summary>
    FlagVictory,

    /// <summary>
    /// Flag game resetted, part of the flag victory packet.
    /// </summary>
    FlagGameReset,

    /// <summary>
    /// Team receives a flag game reward.
    ///     uint16_t                    team
    ///     uint32_t                    points
    /// </summary>
    FlagReward,

    /// <summary>
    /// Timed game is over.
    ///     const Player&               player1
    ///     const Player&               player2
    ///     const Player&               player3
    ///     const Player&               player4
    ///     const Player&               player5
    /// </summary>
    TimedGameOver,

    /// <summary>
    /// Team made a soccer goal.
    ///     uint16_t                    team
    ///     uint32_t                    points
    /// </summary>
    SoccerGoal,

    /// <summary>
    /// File downloaded.
    ///     string_view                 fileName
    /// </summary>
    File,

    /// <summary>
    /// Ball moved.
    ///     const PowerBall&            powerBall
    /// </summary>
    BallMove,

    /// <summary>
    /// Core is requesting the DLL to send a position packet.
    /// </summary>
    PositionHook,

    /// <summary>
    /// LVZ object was toggled on or off.
    ///     uint16_t                    value
    /// </summary>
    ObjectToggled,

    /// <summary>
    /// Brick dropped.
    ///     Coord                       x1
    ///     Coord                       y1
    ///     Coord                       x2
    ///     Coord                       y2
    ///     uint16_t                    team
    /// </summary>
    BrickDropped,

    /// <summary>
    /// Playfield loaded.
    /// </summary>
    MapLoaded,

    /// <summary>
    /// Plugin DLL is being terminated.
    /// </summary>
    Term,

    //////// Spawn->Host ////////

    /// <summary>
    /// Log a message to the console and store it to a log file.
    ///     string_view                 message
    /// </summary>
    Echo,

    /// <summary>
    /// Log a orange colored message to the console and store it to a log file.
    ///     string_view                 message
    /// </summary>
    Warning,

    /// <summary>
    /// Log a red colored message to the console and store it to a log file.
    ///     string_view                 message
    /// </summary>
    Error,

    /// <summary>
    /// Plugin DLL is shutting down due to an internal error.
    ///     string_view                 message
    /// </summary>
    Exit,

    /// <summary>
    /// Bot sends a private, team or public message.
    ///     ChatMode                    chatMode
    ///     ChatSoundCode               soundCode
    ///     uint16_t                    recipient   // either a played Id or team freq
    ///     string_view                 message
    /// </summary>
    Say,

    /// <summary>
    /// Make the bot die to a certain player.
    ///     const Player&               player
    /// </summary>
    Die,

    /// <summary>
    /// Attach to a player.
    ///     const Player&               player
    /// </summary>
    Attach,

    /// <summary>
    /// Detach.
    /// </summary>
    Detach,

    /// <summary>
    /// Let the bot either start or stop following the next player. When not following, it will 
    /// just sit around.
    ///     bool                        isFollowing
    /// </summary>
    Following,

    /// <summary>
    /// Stops sending telemetry, polls the DLL when it's time to send position packets.
    ///     bool                        isFlying
    /// </summary>
    Flying,

    /// <summary>
    /// Bot requests a banner change.
    ///     Banner                      banner
    /// </summary>
    Banner,

    /// <summary>
    /// Bot requests to drop a brick.
    /// </summary>
    DropBrick,

    /// <summary>
    /// Bot requests a ship change.
    ///     Ship                        ship
    /// </summary>
    Ship,

    /// <summary>
    /// Bot requests a team change.
    ///     uint16_t                    team
    /// </summary>
    Team,

    /// <summary>
    /// Flying bot grabs a flag.
    ///     FlagId                      flagId
    /// </summary>
    GrabFlag,
    
    /// <summary>
    /// Send flying bot's position info.
    ///     bool                        isReliable
    /// </summary>
    SendPosition,

    /// <summary>
    /// Request bot to drop flags.
    /// </summary>
    DropFlags,

    /// <summary>
    /// Spawn the bot to another arena.
    ///     string_view                 name
    ///     string_view                 password
    ///     string_view                 staff
    ///     string_view                 arena
    /// </summary>
    SpawnBot,

    /// <summary>
    /// Bot requests to change arena.
    ///     string_view                 arena
    /// </summary>
    ChangeArena,

    /// <summary>
    /// Flying bot requests to fire a weapon.
    ///     WeaponInfo                  weaponInfo
    /// </summary>
    FireWeapon,

    /// <summary>
    /// Toggle LVZ objects for a player.
    ///     PlayerId                    playerId
    ///     const list<LvzObjectInfo>&  objects
    /// </summary>
    ToggleObjects,

    /// <summary>
    /// Modify LVZ objects for a player.
    ///     PlayerId                    playerId
    ///     const list<LvzObject>&      objects
    /// </summary>
    ModifyObjects,
    
    /// <summary>
    /// Request the bot to grab the ball.
    ///     PlayerId                    playerId
    /// </summary>
    GrabBall,

    /// <summary>
    /// Request the bot to fire the specified ball in a certain direction.
    ///     PlayerId                    playerId
    ///     Coord                       x
    ///     Coord                       y
    ///     Velocity                    vx
    ///     Velocity                    vy
    /// </summary>
    FireBall,

    /// <summary>
    /// Bot requests to change arena settings a la ?setsettings
    ///     const list<string>&         settings
    /// </summary>
    ChangeSettings
};


//////// Bot Event ////////

export using EventHandle = size_t;


/// <summary>
/// Bot event base class. Needed to store the heterogenous instances of TBotEvent.
/// </summary>
export class BotEventBase {};


/// <summary>
/// Bot event class.
/// </summary>
/// <typeparam name="...Args">Variable event argument types.</typeparam>
export template<typename... Args>
class TBotEvent : public BotEventBase
{
public:
    /// <summary>
    /// Register an event handler and return a handle to unregister the handler.
    /// </summary>
    /// <param name="observer">Event handler function.</param>
    /// <returns>Reference to the event.</returns>
    [[nodiscard]] EventHandle operator += (std::function<void(Args...)> observer) 
    {
        m_observers[++m_count] = observer;
        return m_count;
    }

    /// <summary>
    /// Unregister the event handler for a given handle.
    /// </summary>
    /// <param name="hnd">Event handle.</param>
    /// <returns>Reference to the event.</returns>
    TBotEvent& operator -= (EventHandle handle)
    {
        m_observers.erase(handle);
        return *this;
    }

    /// <summary>
    /// Notify all registered observers.
    /// </summary>
    /// <param name="...args">Variable event argument types.</param>
    void raise(Args&&... args) const
    {
        for (auto& observer : m_observers) {
            std::invoke(observer.second, std::forward<Args>(args)...);
        }
    }

private:
    size_t m_count{};
    std::map<EventHandle, std::function<void(Args...)>> m_observers;
};
