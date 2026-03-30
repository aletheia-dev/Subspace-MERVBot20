export module ClientTypes;

import <cstdint>;


export uint16_t UnknownId{ 0xFFFF };
export uint16_t SpecFreq{ 8025 };

export typedef uint16_t FlagId;


// Raw ship type numbers
// c2s (Change ship) 18 __
export enum class Ship
{
    Warbird,
    Javelin,
    Spider,
    Leviathan,
    Terrier,
    Weasel,
    Lancaster,
    Shark,
    Spectator
};


// Raw prize type numbers
export enum class Prize
{
    Unknown,
    Recharge,
    Energy,
    Rotation,
    Stealth,
    Cloak,
    XRadar,
    Warp,
    Guns,
    Bombs,
    BBullets,
    Thruster,
    TopSpeed,
    FullCharge,
    EngineShutdown,
    Multifire,
    Proximity,
    Super,
    Shields,
    Antiwarp,
    Repel,
    Burst,
    Decoy,
    Thor,
    Multiprize,
    Brick,
    Rocket,
    Portal
};


// Password response constants
// s2c (Password response) 0A __ 86 00 00 00 00 00 00 00 E8 9C 42 F1 00 00 00 00 00 00 48 C9 1C 28 7C 37 92 E6 00 00 00 00 20 00 00 00
export enum class LoginParam
{
    Continue,                       // 0   - Move along.
    NewUser,                        // 1   - Unknown player, continue as new user?
    InvalidPassword,                // 2   - Invalid password for specified user.  The name you have chosen is probably in use by another player, try picking a different name.
    FullArena,                      // 3   - This arena is currently full, try again later.
    LockedOut,                      // 4   - You have been locked out of SubSpace, for more information inquire on Web BBS.
    NoPermission,                   // 5   - You do not have permission to play in this arena, see Web Site for more information.
    SpectateOnly,                   // 6   - You only have permission to spectate in this arena.
    TooManyPoints,                  // 7   - You have too many points to play in this arena, please choose another arena.
    SlowConnection,                 // 8   - Your connection appears to be too slow to play in this arena.
    NoPermission2,                  // 9   - You do not have permission to play in this arena, see Web Site for more information.
    NoNewConnections,               // 10  - The server is currently not accepting new connections.
    InvalidName,                    // 11  - Invalid user name entered, please pick a different name.
    ObsceneName,                    // 12  - Possibly offensive user name entered, please pick a different name.
    BillerDown,                     // 13  - NOTICE: Server difficulties; this zone is currently not keeping track of scores.  Your original score will be available later.  However, you are free to play in the zone until we resolve this problem.
    BusyProcessing,                 // 14  - The server is currently busy processing other login requests, please try again in a few moments.
    ExperiencedOnly,                // 15  - This zone is restricted to experienced players only (ie. certain number of game-hours logged).
    UsingDemoVersion,               // 16  - You are currently using the demo version.  Your name and score will not be kept track of.
    TooManyDemos,                   // 17  - This arena is currently has(sic) the maximum Demo players allowed, try again later.
    ClosedToDemos,                  // 18  - This arena is closed to Demo players.
    UnknownResponse,                // ... - Unknown response type, please go to Web site for more information and to obtain latest version of the program.
    NeedModerator = 255             // 255 - Moderator access required for this zone (MGB addition)
};


// Chat parameters
// c2s (Chat message) 06 __ 00 00 00 MM EE SS SS AA GG EE 00
export enum class ChatMode
{
    Arena,                          // 00 Same for *arena, *zone, *szone, **
    PublicMacro,                    // 01 Macro calcs are done client-side
    Public,                         // 02 Public chat
    Team,                           // 03 (//) or (')
    TeamPrivate,                    // 04 Player to all members of another team (")
    Private,                        // 05 Only shows messages addressed to the bot
    PlayerWarning,                  // 06 Red message, with a name tag
    RemotePrivate,                  // 07 Do not trust the sender with any private information.
    ServerError,                    // 08 Red server errors, without a name tag
    Channel                         // 09 Arrives in the same format SubSpace displays
};


// Item info concatenated to the position packets
// c2s (Position packet) 03 00 1D 23 02 00 05 00 27 10 FF SI 25 01 01 00 2D 00 3F 01 WW II 3F 01 00 00 00 00 __ __ __ __
#pragma pack(push)
#pragma pack(1)    // For bitfields

export union ItemInfo
{
    struct
    {
        uint32_t shields : 1;       // Has it or not
        uint32_t supers : 1;
        uint32_t burst : 4;         // Item counts
        uint32_t repel : 4;
        uint32_t thor : 4;
        uint32_t brick : 4;
        uint32_t decoy : 4;
        uint32_t rocket : 4;
        uint32_t portal : 4;
        uint32_t pack : 2;          // Probably used somehow
    };

    uint32_t value;
};


// Weapon types for weaponInfo.type
export enum class Projectile
{
    // Seen "in the wild"
    None,
    Bullet,
    BBullet,
    Bomb,
    PBomb,
    Repel,
    Decoy,
    Burst,
    Thor,

    // Internal to the bot
    InactiveBullet,
    Shrapnel
};


// Weapon levels for weaponInfo.level
export enum class WeaponLevel
{
    One,
    Two,
    Three,
    Four
};


// Weapon info concatenated to the position packets
// c2s (Position packet) 03 00 1D 23 02 00 05 00 27 10 FF SI 25 01 01 00 2D 00 3F 01 __ __ 3F 01 00 00 00 00 II TT EE MM
export union WeaponInfo
{
    struct
    {
        Projectile type : 5;        // enum Projectile_Types
        WeaponLevel level : 2;      // Only for bombs/bullets
        uint16_t shrapBounce : 1;   // Bouncing shrapnel?
        uint16_t shrapLevel : 2;    // Shrapnel level 0..3
        uint16_t shrapCount : 5;    // 0-31
        uint16_t fireType : 1;      // Bombs -> Mines, Bullets -> Multifire
    };

    uint16_t value;
};


// State info concatenated to the position packets
// c2s (Position packet) 03 00 1D 23 02 00 05 00 27 10 FF __ 25 01 01 00 2D 00 3F 01 WW II 3F 01 00 00 00 00 II TT EE MM
export union StateInfo
{
    struct
    {
        uint8_t stealth : 1;
        uint8_t cloaked : 1;
        uint8_t xradar : 1;
        uint8_t awarp : 1;
        uint8_t flash : 1;          // Uncloaking, portaling, etc.
        uint8_t safety : 1;         // In a safety zone
        uint8_t ufo : 1;            // *ufo - Illegal usage caught in sg9+
        uint8_t pack : 1;           // ?
    };

    uint8_t value;
};


// Security checksum responses, violation codes
// c2s (Violation notification) 1B __
export enum class SecurityViolation
{
    // NOTE: These may only be sent in response to a security checksum request
    NormalIntegrity,                // 0x00 - Normal integrity
    SlowFrameDetected,              // 0x01 - Slow frame detected
    Energy_CurrentOverTop,          // 0x02 - Current energy higher than top energy
    Energy_TopOverMax,              // 0x03 - Top energy higher than max energy
    Energy_MaxWithoutPrize,         // 0x04 - Max energy without getting prizes
    Recharge_CurrentOverMax,        // 0x05 - Recharge rate higher than max recharge rate
    Recharge_MaxWithoutPrize,       // 0x06 - Max recharge rate without getting prizes
    Burst_TooMany,                  // 0x07 - Too many burst used
    Repel_TooMany,                  // 0x08 - Too many repel used
    Decoy_TooMany,                  // 0x09 - Too many decoy used
    Thor_TooMany,                   // 0x0A - Too many thor used
    Brick_TooMany,                  // 0x0B - Too many wall blocks used
    Stealth_WithoutPrize,           // 0x0C - Stealth on but never collected
    Cloak_WithoutPrize,             // 0x0D - Cloak on but never collected
    XRadar_WithoutPrize,            // 0x0E - XRadar on but never collected
    AntiWarp_WithoutPrize,          // 0x0F - AntiWarp on but never collected
    PBombs_WithoutPrize,            // 0x10 - Proximity bombs but never collected
    BBullets_WithoutPrize,          // 0x11 - Bouncing bullets but never collected
    Guns_MaxWithoutPrize,           // 0x12 - Max guns without getting prizes
    Bombs_MaxWithoutPrize,          // 0x13 - Max bombs without getting prizes
    Specials_OnTooLong,             // 0x14 - Shields or Super on longer than possible

    // NOTE: This block of codes is the exception.  You may send them at any time.
    ShipWeaponLimitsTooHigh,        // 0x15 - Saved ship weapon limits too high (burst/repel/etc)
    ShipWeaponLevelTooHigh,         // 0x16 - Saved ship weapon level too high (guns/bombs)
    LoginChecksumMismatch,          // 0x17 - Login checksum mismatch (program exited)
    PositionChecksumMismatch,       // 0x18 - Position checksum mismatch
    ShipChecksumMismatch,           // 0x19 - Saved ship checksum mismatch

    // NOTE: These may only be sent in response to a security checksum request
    SoftICE,                        // 0x1A - Softice Debugger Running
    DataChecksumMismatch,           // 0x1B - Data checksum mismatch
    ParameterMismatch,              // 0x1C - Parameter mismatch
    UnknownViolation,               // .... - Unknown integrity violation
    HighLatency = 0x3C              // 0x3C - Unknown integrity violation (High latency in Continuum)
};


// Arena login type-code field
export enum class ArenaCode
{
    RandomMain = 0xFFFF,            // ?go
    Private = 0xFFFD                // ?go arena  ?go #arena
};


// RegForm player sex field

export enum class RegFormSex
{
    Male = 'M',                     // Male
    Female = 'F',                   // Female

    Alien = 'A',
    Bot = 'B',
    Other = 'O',
    Undetermined = 'U',
};


// ConnectType field from *info

export enum class ConnectMode
{
    Unknown,                        // 00 
    SlowModem,                      // 01 
    FastModem,                      // 02 
    UnknownModem,                   // 03 
    UnknownNotRAS,                  // 04 
    ISDN,                           // 05 Will show up as "InvalidValue"
    PAD,                            // 06 Will show up as "InvalidValue"
    Switch,                         // 07 Will show up as "InvalidValue"
    InvalidValue                    // 08 All others are also invalid
};


// Chat message sounduint8_ts
// c2s (Chat) 06 09 __ 01 00 MM EE SS SS AA GG EE 00

export enum class ChatSoundCode
{                                   // Soundcodes courtesy of MGB
    None,                           // 0  = Silence
    BassBeep,                       // 1  = BEEP!
    TrebleBeep,                     // 2  = BEEP!
    ATT,                            // 3  = You're not dealing with AT&T
    Discretion,                     // 4  = Due to some violent content, parental discretion is advised
    Hallellula,                     // 5  = Hallellula
    Reagan,                         // 6  = Ronald Reagan
    Inconceivable,                  // 7  = Inconceivable
    Churchill,                      // 8  = Winston Churchill
    SnotLicker,                     // 9  = Listen to me, you pebble farting snot licker
    Crying,                         // 10 = Crying
    Burp,                           // 11 = Burp
    Girl,                           // 12 = Girl
    Scream,                         // 13 = Scream
    Fart,                           // 14 = Fart1
    Fart2,                          // 15 = Fart2
    Phone,                          // 16 = Phone ring
    WorldUnderAttack,               // 17 = The world is under attack at this very moment
    Gibberish,                      // 18 = Gibberish
    Ooooo,                          // 19 = Ooooo
    Geeee,                          // 20 = Geeee
    Ohhhh,                          // 21 = Ohhhh
    Ahhhh,                          // 22 = Awwww
    ThisGameSucks,                  // 23 = This game sucks
    Sheep,                          // 24 = Sheep
    CantLogIn,                      // 25 = I can't log in!
    MessageAlarm,                   // 26 = Beep
    StartMusic = 100,               // 100= Start music playing
    StopMusic,                      // 101= Stop music
    PlayOnce,                       // 102= Play music for 1 iteration then stop
    VictoryBell,                    // 103= Victory bell
    Goal                            // 104= Goal!
};

#pragma pack(pop)    // End of bitfields


// Continuum object toggling
// s2c (Object toggle) 35 __ __
#pragma pack(push)    // More bitfields
#pragma pack(1)

export union LvzObjectInfo
{
    struct
    {
        uint16_t id : 15;           // Object ident
        uint16_t disabled : 1;      // 1=off, 0=on
    };

    uint16_t value;
};


// Continuum object modification
// c2s (Object modify) 0a <pid(2)> <subtype=36(1)> <array of modifiers>
export enum class _ObjectLayer
{
    BelowAll,
    AfterBackground,
    AfterTiles,
    AfterWeapons,
    AfterShips,
    AfterGauges,
    AfterChat,
    TopMost,
};


export enum class _ObjectMode
{
    ShowAlways,
    EnterZone,
    EnterArena,
    Kill,
    Death,
    ServerControlled,
};


export struct LvzObject  /* 11 uint8_ts */
{
    uint8_t change_xy : 1{};        // what properties to change for this object
    uint8_t change_image : 1{};
    uint8_t change_layer : 1{};
    uint8_t change_time : 1{};
    uint8_t change_mode : 1{};
    uint8_t reserved : 3{};

    uint16_t mapobj : 1{};
    uint16_t id : 15{};
    uint16_t x, y{};                // for screen objects, upper 12 bits are value, lower 4 are relative to what corner
    uint8_t image{};
    uint8_t layer{};
    uint16_t time : 12{};           // 1/10th seconds
    uint16_t mode : 4{};            // 0=AlwaysOn 5=ServerControlled
};

#pragma pack(pop)    // End of bitfields
