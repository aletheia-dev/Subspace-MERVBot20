module;
//#define _CRT_SECURE_NO_WARNINGS
#include <fstream>

export module ClientProt;

import Algorithms;
import BotDb;
import BotEvent;
import Checksum;
import CoreCmd;
import Host;
import System;

import <format>;
import <iostream>;
import <list>;
import <ranges>;
import <vector>;


/*    c2s Message types
    --    Original SubSpace protocol
    00    Start of a special header
    01    Arena login
    02    Leave arena
    03    Position packet
    04    <disabled?>
    05    Death message
    06    Chat message
    07    Take green
    08    Spectate player
    09    Password packet
    0A    <disabled?>
    0B    SSUpdate.EXE request
    0C    map.lvl request
    0D    news.txt request
    0E    Voice message
    0F    Frequency change
    10    Attach request
    11    <disabled?>
    12    <disabled?>
    13    Flag request
    14    Leave turret
    15    Drop flags
    16    File transfer
    17    Registration information response
    18    Set ship type
    19    Set personal banner
    1A    Security checksum
    1B    Security violation
    1C    Drop brick
    1D    Change settings
    1E    Personal KoTH timer ended
    1F    Fire a ball
    20    Ball request
    21    Soccer Goal

    Snrrrub sez:

    22 <timestamp (4)> <settings_csum (4)> <exe_csum (4)> <map_csum (4)> <securityViolation (1)>

    basically the checksums are the same ones used for the synchronization (18) packet except the checksum key used 
    is 0. The security violation uint8_t is the same violation uint8_t as the parameter for the violation packet (1b)

    --    Additional Continuum protocol
    23
    24    Password packet
    ??    Object toggling
*/

/*    s2c Message types
    --    Original SubSpace protocol
    01    PlayerID change
    02    You are now in the game
    03    Player entering
    04    Player leaving
    05    Player fired a weapon
    06    Player died
    07    Chat
    08    Player took a prize
    09    Player score changed
    0A    Password packet response
    0B    Soccer goal
    0C    Player voice
    0D    Set player frequency (ship optional)
    0E    Create turret link
    0F    Arena settings
    10    File transfer
    11    <no-op>
    12    Flag position
    13    Flag claim
    14    Flag victory
    15    Destroy turret link
    16    Drop flag
    17    <no-op>
    18    Synchronization
    19    Request file
    1A    Reset score(s)
    1B    Personal ship reset
    1C    Put player in spectator mode / change extra info flag
    1D    Player team and ship changed (redundant)
    1E    Banner flag
    1F    Player banner changed
    20    Collected prize
    21    Brick dropped
    22    Turf flag update
    23    Flag reward granted
    24    Speed zone statistics
    25    Toggle UFO ship
    26    <no-op>
    27    Keep-alive
    28    Player position update
    29    Playfield information
    2A    Compressed map file
    2B    Set personal KoTH timer
    2C    KoTH game reset
    2D    Add KoTH Time
    2E    Power-ball position update
    2F    Arena directory listing
    30    Got zone banner advertisements
    31    You are now past the login sequence

    --    Additional Continuum protocol
    32    Change personal ship coordinates
    33    Custom login failure message
    34    Continuum version packet
    35    Object toggling
    36    Received object
    37    Damage toggling
    38    *watchdamage message
*/


//////// C2S protocol ////////

export ClientMessage generateViolation(SecurityViolation code)
{
    ClientMessage ret{ 2 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Violation code
    msg[0] = 0x1B;
    msg[1] = (uint8_t)code;
    return ret;
}


export ClientMessage generateChangeShip(Ship ship)
{
    ClientMessage ret{ 2 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Ship type
    msg[0] = 0x18;
    msg[1] = (uint8_t)ship;
    return ret;
}


export ClientMessage generateSpectate(PlayerId playerId)
{
    ClientMessage ret(3);
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Player ident
    msg[0] = 0x08;
    *(uint16_t*)&msg[1] = playerId;
    return ret;
}


export ClientMessage generateChangeTeam(uint16_t team)
{
    ClientMessage ret{ 3 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Team
    msg[0] = 0x0F;
    *(uint16_t*)&msg[1] = team;
    return ret;
}


export ClientMessage generateChangeBanner(Banner buffer)
{
    ClientMessage ret{ 97 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1     96      Banner
    msg[0] = 0x19;
    memcpy(&msg[1], buffer, 96);
    return ret;
}


export ClientMessage generateDeath(PlayerId playerId, uint16_t bounty)
{
    ClientMessage ret{ 5 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Player ident
    //      3      2      Bounty
    msg[0] = 0x05;
    *(uint16_t*)&msg[1] = playerId;
    *(uint16_t*)&msg[3] = bounty;
    return ret;
}


export ClientMessage generateChat(ChatMode type, ChatSoundCode soundcode, PlayerId playerId, std::string_view text)
{
    uint32_t len = limit((uint32_t)text.length(), 250);
    ClientMessage ret{ 6 + len };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Chat type
    //      2      1      Soundcode
    //      3      2      Player ident
    //      5  ...\0      Message
    msg[0] = 0x06;
    msg[1] = (uint8_t)type;
    msg[2] = (uint8_t)soundcode;
    *(uint16_t*)&msg[3] = playerId;
    strncpy_s((char*)&msg[5], len + 1, text.data(), len + 1);
    return ret;
}


export ClientMessage generateRegForm(std::string_view name, std::string_view email, std::string_view city, 
    std::string_view state, RegFormSex sex, uint8_t age, bool playAtHome, bool playAtWork, bool playAtSchool, 
    uint32_t processor, std::string_view regName, std::string_view regOrg)
{
    ClientMessage ret{ 766 };

    ret.clear();
    
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1     32      Real name
    //     33     64      Email
    //     97     32      City
    //    129     24      State
    //    153      1      Sex('M'/'F')
    //    154      1      Age
    // Connecting from...
    //    155      1      Home
    //    156      1      Work
    //    157      1      School
    // System information
    //    158      4      Processor type (586)
    //    162      2      ?
    //    164      2      ?
    // Windows registration information (SSC RegName ban)
    //    166     40      Real name
    //    206     40      Organization
    // Windows NT-based OS's do not send any hardware information (DreamSpec HardwareID ban)
    //    246     40      System\CurrentControlSet\Services\Class\Display\0000
    //    286     40      System\CurrentControlSet\Services\Class\Monitor\0000
    //    326     40      System\CurrentControlSet\Services\Class\Modem\0000
    //    366     40      System\CurrentControlSet\Services\Class\Modem\0001
    //    406     40      System\CurrentControlSet\Services\Class\Mouse\0000
    //    446     40      System\CurrentControlSet\Services\Class\Net\0000
    //    486     40      System\CurrentControlSet\Services\Class\Net\0001
    //    526     40      System\CurrentControlSet\Services\Class\Printer\0000
    //    566     40      System\CurrentControlSet\Services\Class\MEDIA\0000
    //    606     40      System\CurrentControlSet\Services\Class\MEDIA\0001
    //    646     40      System\CurrentControlSet\Services\Class\MEDIA\0002
    //    686     40      System\CurrentControlSet\Services\Class\MEDIA\0003
    //    726     40      System\CurrentControlSet\Services\Class\MEDIA\0004
    msg[0] = 0x17;
    strncpy_s((char*)&msg[1], 32, name.data(), 32);
    strncpy_s((char*)&msg[33], 64, email.data(), 64);
    strncpy_s((char*)&msg[97], 32, city.data(), 32);
    strncpy_s((char*)&msg[129], 24, state.data(), 24);
    msg[153] = (uint8_t)sex;
    msg[154] = age;
    msg[155] = playAtHome ? 1 : 0;
    msg[156] = playAtWork ? 1 : 0;
    msg[157] = playAtSchool ? 1 : 0;
    *(uint32_t*)&msg[158] = processor;
    *(uint16_t*)&msg[162] = 0xC000;    // magic number 1, i've checked the subspace API imports
    *(uint16_t*)&msg[164] = 2036;        // magic number 2, nothing matches AFAIK =/
    strncpy_s((char*)&msg[166], 40, regName.data(), 40);
    strncpy_s((char*)&msg[206], 40, regOrg.data(), 40);

    // wenn man windows.h included, dann gibt es einen ungeklärten Konflikt mit sockets, daher dieser Trick mit HKey
    HKey HKeyLocalMachine{ (HKey)(uint64_t)0x80000002 };

    // I didn't cache the hardware stuff, it's a pain in the butt.
    getServiceString(HKeyLocalMachine, "System\\CurrentControlSet\\Services\\Class\\Display\\0000", "DriverDesc", (char*)&msg[246]);
    getServiceString(HKeyLocalMachine, "System\\CurrentControlSet\\Services\\Class\\Monitor\\0000", "DriverDesc", (char*)&msg[286]);
    getServiceString(HKeyLocalMachine, "System\\CurrentControlSet\\Services\\Class\\Modem\\0000", "DriverDesc", (char*)&msg[326]);
    getServiceString(HKeyLocalMachine, "System\\CurrentControlSet\\Services\\Class\\Modem\\0001", "DriverDesc", (char*)&msg[366]);
    getServiceString(HKeyLocalMachine, "System\\CurrentControlSet\\Services\\Class\\Mouse\\0000", "DriverDesc", (char*)&msg[406]);
    getServiceString(HKeyLocalMachine, "System\\CurrentControlSet\\Services\\Class\\Net\\0000", "DriverDesc", (char*)&msg[446]);
    getServiceString(HKeyLocalMachine, "System\\CurrentControlSet\\Services\\Class\\Net\\0001", "DriverDesc", (char*)&msg[486]);
    getServiceString(HKeyLocalMachine, "System\\CurrentControlSet\\Services\\Class\\Printer\\0000", "DriverDesc", (char*)&msg[526]);
    getServiceString(HKeyLocalMachine, "System\\CurrentControlSet\\Services\\Class\\MEDIA\\0000", "DriverDesc", (char*)&msg[566]);
    getServiceString(HKeyLocalMachine, "System\\CurrentControlSet\\Services\\Class\\MEDIA\\0001", "DriverDesc", (char*)&msg[606]);
    getServiceString(HKeyLocalMachine, "System\\CurrentControlSet\\Services\\Class\\MEDIA\\0002", "DriverDesc", (char*)&msg[646]);
    getServiceString(HKeyLocalMachine, "System\\CurrentControlSet\\Services\\Class\\MEDIA\\0003", "DriverDesc", (char*)&msg[686]);
    getServiceString(HKeyLocalMachine, "System\\CurrentControlSet\\Services\\Class\\MEDIA\\0004", "DriverDesc", (char*)&msg[726]);
    return ret;
}


export ClientMessage generateKoTHReset()
{
    ClientMessage ret{ 1 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    msg[0] = 0x1E;
    return ret;
}


export ClientMessage generateSecurityChecksum(uint32_t paramChecksum, uint32_t EXEChecksum,  uint32_t levelChecksum, 
    uint16_t S2CRelOut, uint16_t ping, uint16_t avgPing, uint16_t lowPing, uint16_t highPing, uint32_t weaponCount, 
    uint16_t S2CSlowCurrent, uint16_t S2CFastCurrent, uint16_t S2CSlowTotal, uint16_t S2CFastTotal, bool slowFrame)
{
    ClientMessage ret{ 40 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      4      Weapon count
    //      5      4      Parameter checksum
    //      9      4      EXE checksum
    //     13      4      Level checksum
    //     17      4      S2CSlowTotal
    //     21      4      S2CFastTotal
    //     25      2      S2CSlowCurrent
    //     27      2      S2CFastCurrent
    //     29      2      ? S2CRelOut
    //     31      2      Ping
    //     33      2      Average ping
    //     35      2      Low ping
    //     37      2      High ping
    //     39      1      Boolean: Slow frame detected
    msg[0] = 0x1A;
    *(uint32_t*)&msg[1] = weaponCount;
    *(uint32_t*)&msg[5] = paramChecksum;
    *(uint32_t*)&msg[9] = EXEChecksum;
    *(uint32_t*)&msg[13] = levelChecksum;
    *(uint32_t*)&msg[17] = S2CSlowTotal;
    *(uint32_t*)&msg[21] = S2CFastTotal;
    *(uint16_t*)&msg[25] = S2CSlowCurrent;
    *(uint16_t*)&msg[27] = S2CFastCurrent;
    *(uint16_t*)&msg[29] = S2CRelOut;
    *(uint16_t*)&msg[31] = ping;
    *(uint16_t*)&msg[33] = avgPing;
    *(uint16_t*)&msg[35] = lowPing;
    *(uint16_t*)&msg[37] = highPing;
    msg[39] = slowFrame ? 1 : 0;
    return ret;
}


export ClientMessage generatePassword(bool newUser, std::string_view name, std::string_view pass, uint32_t machineID, 
    uint16_t timezoneBias, uint32_t permissionID, uint16_t clientVersion, ConnectMode connectType)
{
    ClientMessage ret{ 101 };
    ret.clear();

    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Boolean: New user
    //      2     32      Name
    //     34     32      Password
    //     66      4      Machine ident
    //     70      1      ConnectType (*info)
    //     71      2      Timezone bias
    //     73      2      ?
    //     75      2      Client version
    //     77      4      444 : set if anims.dll is present, else zero (anims.dll version)
    //     81      4      555 : set if anims.dll is present, else zero (anims.dll version)
    //     85      4      Permission ident
    //     89     12      reserved, set to zero
    msg[0] = 0x09;
    msg[1] = newUser ? 1 : 0;
    strncpy_s((char*)&msg[2], 32, name.data(), 32);
    strncpy_s((char*)&msg[34], 32, pass.data(), 32);
    *(uint32_t*)&msg[66] = machineID;
    msg[70] = (uint8_t)connectType;
    *(uint16_t*)&msg[71] = timezoneBias;
    *(uint16_t*)&msg[73] = 0;
    *(uint16_t*)&msg[75] = clientVersion;
    *(uint32_t*)&msg[77] = 444;
    *(uint32_t*)&msg[81] = 555;
    *(uint32_t*)&msg[85] = permissionID;
    return ret;
}


export ClientMessage generateCtmPassword(bool newUser, std::string_view name, std::string_view pass, 
    uint32_t machineID, uint16_t timezoneBias, uint32_t permissionID, uint16_t clientVersion, ConnectMode connectType)
{
    ClientMessage ret{ 165 };

    ret.clear();

    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Boolean: New user
    //      2     32      Name
    //     34     32      Password
    //     66      4      Machine ident
    //     70      1      ConnectType (*info)
    //     71      2      Timezone bias
    //     73      2      ?
    //     75      2      Client version
    //     77      4      444
    //     81      4      555
    //     85   5*16      ? capture
    const uint8_t capture[] = {
        0x00, 0x00, 0x00, 0x00,  0x01, 0x00, 0x00, 0x7F,  0xB4, 0x07, 0xE0, 0xF2,
        0xF5, 0xA5, 0x00, 0x00,  0xAC, 0xDF, 0xF5, 0x91,  0x7A, 0xE2, 0x55, 0xD9,
        0x00, 0x00, 0x00, 0x00,  0xA0, 0x54, 0xD9, 0x57,  0x49, 0x6A, 0xDC, 0xD6,
        0x8C, 0x8F, 0xE0, 0xDC,  0x2B, 0xF0, 0x07, 0x61,  0x60, 0x4F, 0xB4, 0x62,
        0x87, 0x52, 0xCF, 0x6E,  0x5D, 0xB9, 0x30, 0x3D,  0x3D, 0x3D, 0x01, 0x1B,
        0x89, 0x19, 0x7C, 0x70,  0x8F, 0x5C, 0xDF, 0x37,  0x0D, 0x9A, 0x0C, 0x86,
        0x72, 0x86, 0x96, 0x8C,  0x94, 0x86, 0x4D, 0x66
    };

    msg[0] = 0x24;
    msg[1] = newUser ? 1 : 0;
    strncpy_s((char*)&msg[2], 32, name.data(), 32);
    strncpy_s((char*)&msg[34], 32, pass.data(), 32);
    *(uint32_t*)&msg[66] = machineID;
    msg[70] = (uint8_t)connectType;
    *(uint16_t*)&msg[71] = timezoneBias;
    *(uint16_t*)&msg[73] = 0;
    *(uint16_t*)&msg[75] = clientVersion;
    *(uint32_t*)&msg[77] = 444;
    *(uint32_t*)&msg[81] = 555;
    memcpy(&msg[85], capture, 5 * 16);
    return ret;
}


export ClientMessage generatePowerballRequest(uint32_t timestamp, uint8_t ball)
{
    ClientMessage ret{ 6 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Ball ident
    //      2      4      Timestamp
    msg[0] = 0x20;
    msg[1] = ball;
    *(uint32_t*)&msg[2] = timestamp;
    return ret;
}


export ClientMessage generatePowerballUpdate(uint32_t timestamp, uint8_t ball, Coord x, Coord y, Coord xvelocity, 
    Coord yvelocity, PlayerId owner)
{
    ClientMessage ret{ 16 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Ball ident
    //      2      2      X pixels
    //      4      2      Y pixels
    //      6      2      X velocity
    //      8      2      Y velocity
    //     10      2      Owner ident
    //     12      4      Timestamp
    msg[0] = 0x1F;
    msg[1] = ball;
    *(uint16_t*)&msg[2] = (uint16_t)x;
    *(uint16_t*)&msg[4] = (uint16_t)y;
    *(uint16_t*)&msg[6] = (uint16_t)xvelocity;
    *(uint16_t*)&msg[8] = (uint16_t)yvelocity;
    *(uint16_t*)&msg[10] = owner;
    *(uint32_t*)&msg[12] = timestamp;
    return ret;
}


export ClientMessage generateSoccerGoal(uint32_t timestamp, uint8_t ball)
{
    ClientMessage ret{ 6 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Ball ident
    //      2      4      Timestamp
    msg[0] = 0x21;
    msg[1] = ball;
    *(uint32_t*)&msg[2] = timestamp;
    return ret;
}


export ClientMessage generateFlagRequest(FlagId flag)
{
    ClientMessage ret{ 3 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Flag ident
    msg[0] = 0x13;
    *(uint16_t*)&msg[1] = flag;
    return ret;
}


export ClientMessage generateFlagDrop()
{
    ClientMessage ret{ 1 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    msg[0] = 0x15;
    return ret;
}


export ClientMessage generateEXERequest()
{
    ClientMessage ret{ 1 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    msg[0] = 0x0B;
    return ret;
}


export ClientMessage generateLevelRequest()
{
    ClientMessage ret{ 1 };
    std::vector<uint8_t>& msg{ ret.msg };

    // Field  Length      Description
    //      0      1      Type uint8_t
    msg[0] = 0x0C;
    return ret;
}


export ClientMessage generateNewsRequest()
{
    // Field  Length      Description
    //      0      1      Type uint8_t
    ClientMessage ret{ 1 };
    std::vector<uint8_t>& msg{ ret.msg };

    msg[0] = 0x0D;
    return ret;
}


export ClientMessage generateArenaLogin(std::string_view arena, Ship ship, uint16_t xres, uint16_t yres, 
    bool allowAudio)
{
    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Ship type
    // Note: When !AllowAudio, remote messages aren't heard in private arenas (1.34.11f)
    //      2      2      Boolean: Allow audio
    //      4      2      X resolution
    //      6      2      Y resolution
    //      8      2      Main arena number
    //     10     16      Arena name
    ClientMessage ret{ 26 };
    std::vector<uint8_t>& msg{ ret.msg };

    msg[0] = 0x01;
    msg[1] = (uint8_t)ship;
    *(uint16_t*)&msg[2] = (allowAudio ? 1 : 0);
    *(uint16_t*)&msg[4] = xres;
    *(uint16_t*)&msg[6] = yres;

    if (arena.empty()) {    // Random main arena
        *(uint16_t*)&msg[8] = (uint16_t)ArenaCode::RandomMain;
    }
    else if (isNumeric(arena)) {    // Specific main arena
        *(uint16_t*)&msg[8] = stoi(std::string(arena));
    }
    else {    // Private arena
        *(uint16_t*)&msg[8] = (uint16_t)ArenaCode::Private;
        strncpy_s((char*)&msg[10], 16, arena.data(), 16);
    }
    return ret;
}


export ClientMessage generateArenaLeave()
{
    // Field  Length      Description
    //      0      1      Type uint8_t
    ClientMessage ret{ 1 };
    std::vector<uint8_t>& msg{ ret.msg };

    msg[0] = 0x02;
    return ret;
}


export ClientMessage generateAttachRequest(PlayerId playerId)
{
    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Player ident
    ClientMessage ret{ 3 };
    std::vector<uint8_t>& msg{ ret.msg };

    msg[0] = 0x10;
    *(uint16_t*)&msg[1] = playerId;
    return ret;
}


export ClientMessage generateBrickDrop(uint16_t xtile, uint16_t ytile)
{
    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      X tiles
    //      3      2      Y tiles
    ClientMessage ret{ 5 };
    std::vector<uint8_t>& msg{ ret.msg };

    msg[0] = 0x1C;
    *(uint16_t*)&msg[1] = xtile;
    *(uint16_t*)&msg[3] = ytile;
    return ret;
}


export ClientMessage generatePosition(
    uint32_t timestamp, uint8_t direction, Coord x, Coord y, Velocity vx, Velocity vy, bool stealth, bool cloaked, 
    bool xradar, bool awarp, bool flash, bool safety, bool ufo, uint16_t bounty, uint16_t energy, Projectile ptype, 
    WeaponLevel level, bool shrapBounce, uint8_t shrapLevel, uint8_t shrapCount, bool secondary, uint16_t timer, 
    uint16_t S2CLag, bool shields, bool super, uint8_t burst, uint8_t repel, uint8_t thor, uint8_t brick, 
    uint8_t decoy, uint8_t rocket, uint8_t portal)
{
    // Field  Length      Description
    // This is the extended version of the c2s position update
    //      0      1      Type uint8_t
    //      1      1      Direction
    //      2      4      Timestamp
    //      6      2      X velocity
    //      8      2      Y pixels
    //     10      1      Checksum
    //     11      1      Togglables
    //     12      2      X pixels
    //     14      2      Y velocity
    //     16      2      Bounty
    //     18      2      Energy
    //     20      2      Weapon info
    //     22      2      Energy
    //     24      2      S2C latency
    //     26      2      Timer
    //     28      4      Item information
    ClientMessage ret{ 32 };
    std::vector<uint8_t>& msg{ ret.msg };

    StateInfo si;
    si.awarp = awarp;
    si.cloaked = cloaked;
    si.flash = flash;
    si.pack = 0;
    si.safety = safety;
    si.stealth = stealth;
    si.ufo = ufo;
    si.xradar = xradar;

    WeaponInfo wi;
    wi.shrapBounce = shrapBounce;
    wi.shrapLevel = shrapLevel;
    wi.shrapCount = shrapCount;
    wi.fireType = secondary;
    wi.level = level;
    wi.type = ptype;

    ItemInfo ii;
    ii.brick = brick;
    ii.burst = burst;
    ii.decoy = decoy;
    ii.pack = 0;
    ii.portal = portal;
    ii.repel = repel;
    ii.rocket = rocket;
    ii.shields = shields;
    ii.supers = super;
    ii.thor = thor;

    msg[0] = 0x03;
    msg[1] = direction;
    *(uint32_t*)&msg[2] = timestamp;
    *(uint16_t*)&msg[6] = (uint16_t)vx;
    *(uint16_t*)&msg[8] = (uint16_t)y;
    msg[10] = 0;
    msg[11] = si.value;
    *(uint16_t*)&msg[12] = (uint16_t)x;
    *(uint16_t*)&msg[14] = (uint16_t)vy;
    *(uint16_t*)&msg[16] = bounty;
    *(uint16_t*)&msg[18] = energy;
    *(uint16_t*)&msg[20] = wi.value;

    msg[10] = simpleChecksum(&msg[0], 22);

    *(uint16_t*)&msg[22] = energy;
    *(uint16_t*)&msg[24] = S2CLag;
    *(uint16_t*)&msg[26] = timer;
    *(uint32_t*)&msg[28] = ii.value;
    return ret;
}


export ClientMessage generatePosition(
    uint32_t timestamp, uint8_t direction, Coord x, Coord y, Velocity vx, Velocity vy, bool stealth, bool cloaked, 
    bool xradar, bool awarp, bool flash, bool safety, bool ufo, uint16_t bounty, uint16_t energy, Projectile ptype, 
    WeaponLevel level, bool shrapBounce, uint8_t shrapLevel, uint8_t shrapCount, bool secondary)
{
    // Field  Length      Description
    // This is the basic version of the c2s position update
    //      0      1      Type uint8_t
    //      1      1      Direction
    //      2      4      Timestamp
    //      6      2      X velocity
    //      8      2      Y pixels
    //     10      1      Checksum
    //     11      1      Togglables
    //     12      2      X pixels
    //     14      2      Y velocity
    //      1      2      Bounty
    //      8      2      Energy
    //     20      2      Weapon info
    ClientMessage ret{ 22 };
    std::vector<uint8_t>& msg{ ret.msg };

    StateInfo si;
    si.awarp = awarp;
    si.cloaked = cloaked;
    si.flash = flash;
    si.pack = 0;
    si.safety = safety;
    si.stealth = stealth;
    si.ufo = ufo;
    si.xradar = xradar;

    WeaponInfo wi;
    wi.shrapBounce = shrapBounce;
    wi.shrapLevel = shrapLevel;
    wi.shrapCount = shrapCount;
    wi.fireType = secondary;
    wi.level = level;
    wi.type = ptype;

    msg[0] = 0x03;
    msg[1] = direction;
    *(uint32_t*)&msg[2] = timestamp;
    *(uint16_t*)&msg[6] = (uint16_t)vx;
    *(uint16_t*)&msg[8] = (uint16_t)y;
    msg[10] = 0;
    msg[11] = si.value;
    *(uint16_t*)&msg[12] = (uint16_t)x;
    *(uint16_t*)&msg[14] = (uint16_t)vy;
    *(uint16_t*)&msg[16] = bounty;
    *(uint16_t*)&msg[18] = energy;
    *(uint16_t*)&msg[20] = wi.value;

    msg[10] = simpleChecksum(&msg[0], 22);
    return ret;
}


export ClientMessage generateFileTransfer(char* fileName, char* buffer, uint32_t len)
{
    // Field  Length      Description
    // This is the basic version of the c2s position update
    //    0        1        Type uint8_t
    //    1        16        File name
    //    17        ...        Compressed file
    ClientMessage ret{ 17 + len };
    std::vector<uint8_t>& msg{ ret.msg };

    msg[0] = 0x16;
    strncpy_s((char*)&msg[1], 16, fileName, 16);
    memcpy((char*)&msg[17], buffer, len);
    return ret;
}


export ClientMessage generateSendNewVoice(uint8_t id, PlayerId playerId, char* buffer, uint32_t len)
{
    // Field  Length      Description
    // This is the basic version of the c2s position update
    //      0      1      Type uint8_t
    //      1      1      Voice ident
    //      2      2      Player ident
    //      4    ...      Compressed voice message
    ClientMessage ret{ 4 + len };
    std::vector<uint8_t>& msg{ ret.msg };

    msg[0] = 0x0E;
    msg[1] = id;
    *(uint16_t*)&msg[2] = playerId;
    memcpy(&msg[4], buffer, len);
    return ret;
}


export ClientMessage generateSendOldVoice(uint8_t id, PlayerId playerId)
{
    // Field  Length      Description
    // This is the basic version of the c2s position update
    //      0      1      Type uint8_t
    //      1      1      Voice ident
    //      2      2      Player ident
    ClientMessage ret{ 4 };
    std::vector<uint8_t>& msg{ ret.msg };

    msg[0] = 0x0E;
    msg[1] = id;
    *(uint16_t*)&msg[2] = playerId;
    return ret;
}


export ClientMessage generateObjectModify(PlayerId playerId, const std::list<LvzObject>& objects)
{
    // Field  Length      Description
    //      0      1      Type uint8_t: 0x0a
    //      1      2      Player ident (-1 = all players)
    //      3      1      LVZ type uint8_t: 0x36
    // The following are repeated until the end of the message
    //      4     11      LVZ bitfield
    ClientMessage ret{ (uint32_t)4 + sizeof(LvzObject) * objects.size() };
    std::vector<uint8_t>& msg{ ret.msg };
    uint32_t i{};

    msg[0] = 0x0a;
    *(uint16_t*)&msg[1] = playerId;
    msg[3] = 0x36;

    for (const LvzObject& object : objects) {
        *(LvzObject*)&msg[4 + sizeof(LvzObject) * i++] = object;
    }
    return ret;
}


export ClientMessage generateObjectToggle(PlayerId playerId, const std::list<LvzObjectInfo>& objectInfos)
{
    // Field  Length      Description
    //      0      1      Type uint8_t: 0x0a
    //      1      2      Player ident (-1 = all players)
    //      3      1      LVZ type uint8_t: 0x35
    // The following are repeated until the end of the message
    //      4      2      LVZ bitfield
    ClientMessage ret{ (uint32_t)4 + sizeof(LvzObjectInfo) * objectInfos.size() };
    std::vector<uint8_t>& msg{ ret.msg };
    uint32_t i{};

    msg[0] = 0x0a;
    *(uint16_t*)&msg[1] = playerId;
    msg[3] = 0x35;

    for (const LvzObjectInfo& objectInfo : objectInfos) {
        *(LvzObjectInfo*)&msg[4 + sizeof(LvzObjectInfo) * i++] = objectInfo;
    }
    return ret;
}


export ClientMessage generateTakeGreen(uint32_t timestamp, int16_t prize, uint16_t x, uint16_t y)
{
    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      4      ? Timestamp
    //      5      2      X
    //      7      2      Y
    //      9      2      Prize
    ClientMessage ret{ 11 };
    std::vector<uint8_t>& msg{ ret.msg };

    msg[0] = 0x07;
    *(uint32_t*)&msg[1] = timestamp;
    *(int16_t*)&msg[5] = prize;
    *(uint16_t*)&msg[7] = x;
    *(uint16_t*)&msg[9] = y;
    return ret;
}


export ClientMessage generateChangeSettings(const std::list<std::string>& settings)
{
    // Field  Length      Description
    //      0      1      Type uint8_t
    // The following are repeated until the end of the message
    //      1     ...      "Weasel:SoccerBallSpeed:9000" ASCIIZ
    // End of the message
    //    ...      2      "\0\0"
    uint32_t bytes = 3;    // type uint8_t + 2 term chars

    for (std::string_view s : settings) {
        bytes += (uint32_t)s.length() + 1;
    }
    ClientMessage ret{ bytes };
    std::vector<uint8_t>& msg{ ret.msg };

    msg[0] = 0x1d;

    uint32_t i = 1;

    for (std::string_view s : settings) {
        strncpy_s((char*)&msg[i], s.length() + 1, s.data(), s.length() + 1);    // copy std::string\0

        i += (uint32_t)s.length() + 1;
    }
    msg[i++] = '\0';
    msg[i] = '\0';
    return ret;
}


//////// S2C protocol ////////

#ifndef DEFLATE_CLASS
#define DEFLATE_CLASS \
    Host* h = m.src; \
    std::vector<uint8_t>& msg = m.msg; \
    size_t len = msg.size();
#endif


export void __stdcall handleUnknown(HostMessage& m)
{
    DEFLATE_CLASS

    // Field    Length    Description
    //      0        1    Type uint8_t
    //      1      ...    ?
    h->logEvent("Unknown message type {}({})", msg[0], len);
    h->logIncoming(msg, len);
}


export void __stdcall handleKeepAlive(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    if (len != 1) {
        handleUnknown(m);

        return;
    }
}


export void __stdcall handleBannerFlag(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Boolean: visible?
    uint8_t toggle = getByte(msg, 1);

    if (toggle || (len != 2)) {
        handleUnknown(m);

        return;
    }
    // The banner shouldn't be sent to the client that changed his banner, this message is adequate.
//    h->logEvent("Successfully changed personal banner");
}


export void __stdcall handleLoginNext(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    if (len != 1) {
        handleUnknown(m);

        return;
    }
    //    h->logEvent("Login next");
}


export void __stdcall handleInGameFlag(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    if (len != 1) {
        handleUnknown(m);

        return;
    }

    if (!h->inZone) {
        h->inZone = true;
        h->zoneJoinTime = getTickCount() * 10;

        if (!getBotDatabase().runSilent) {
            if (!h->botInfo.staffpw.empty()) {
                // Send staff password
                h->sendPublic(format("*{}", h->botInfo.staffpw));
            }
            // Check for sysop/smod status
            h->sendPublic("*listmod");

            if (h->hasBot()) {
                // Reliable kills (subgame 11h+)
                h->sendPrivate(h->me(), "*relkills 1");
            }
            // Enter the chats we want
            h->sendPublic(format("?chat={}", h->botChats));
        }
    }

    h->raiseEvent(BotEvent::ArenaEnter, h->botInfo.initialArena, h->me(), h->billerOnline);
    h->arenaJoinTime = getTickCount() * 10;
    //    h->logEvent("Spawn connected.");
}


export void __stdcall handleBannerAds(HostMessage& m)
{
    DEFLATE_CLASS

    // Provided by Ave-iator
    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Display mode
    //      2      2      Width
    //      4      2      Height
    //      6      4      Duration
    //     10      ?      Banner indices
    if (len <= 10) {
        handleUnknown(m);
        return;
    }
    uint8_t mode = getByte(msg, 1);
    uint16_t width = getShort(msg, 2);
    uint16_t height = getShort(msg, 4);
    uint32_t duration = getLong(msg, 6);

    uint8_t* indices = &msg[10];
    size_t length = len - 10;
    // I'm not sure how these are packed (W*H+10 != len) ? zlib
    h->logEvent("Got banner advertisement(s)");
}


//////// FTP ////////

export void __stdcall handleFileTransfer(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1     16      File name
    //     17    ...      [Compressed] file
    char* name = (char*)&msg[1];
    char* buffer = (char*)&msg[17];
    size_t i, length = len - 17;

    if (len <= 17) {
        handleUnknown(m);

        return;
    }

    for (i = 0; i < 16; ++i) {
        char c = name[i];

        if (c == 0)
            break;

        switch (c) {
        case '/':
        case '\\':
            h->logWarning("Invalid downloaded-file name path ignored. ({})", name);
            return;
        };
        if ((c < ' ') || (c > '~')) {
            h->logWarning("Invalid downloaded-file name chars ignored.");
            return;
        }
    }

    if (i == 16) {
        h->logWarning("Unterminated downloaded-file name ignored.");
        return;
    }

    if (*name) {    // regular file
        std::string fileName = "get/" + std::string(name);
        std::ofstream file(fileName, std::ios::binary);

        if (file) {
            file.write(buffer, length);
            h->logEvent("Received file: {}", fileName);
            h->raiseEvent(BotEvent::File, fileName);
        }
        else {
            h->logEvent("Unable to open file for write: {}", fileName);
        }
    }
    else {    // news file
        h->downloadingNews = false;

        if (decompress_to_file("get/news.txt", buffer, length))
            h->logEvent("News file successfully transferred!");
        else
            h->logEvent("Unable to decompress news file.");
    }
}


export void __stdcall handleFileRequest(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1    256      Local file name
    //    257     16      Remote file name
    uint16_t i;
    char* fileName = (char*)&msg[1];
    char* remoteName = (char*)&msg[257];

    if (len != 273) {
        h->logWarning("Requested-file malformed packet ignored.");
        h->logIncoming(msg, len);

        return;
    }

    if (*fileName == 0) {
        h->logWarning("Blank requested-file local name ignored.");
        h->logIncoming(msg, len);

        return;
    }

    for (i = 0; i < 256; ++i) {
        char c = fileName[i];

        if (c == 0)
            break;

        switch (c) {
        case '/':
        case '\\':
            h->logWarning("Invalid requested-file name path ignored. ({})", fileName);
            return;
        };

        if ((c < ' ') || (c > '~')) {
            h->logWarning("Invalid requested-file name chars ignored.");
            return;
        }
    }

    if (i == 256) {
        h->logWarning("Unterminated requested-file local name ignored.");

        return;
    }
    // Put other restrictions here
    h->logEvent("File request: '{}' -> '{}'", fileName, remoteName);

    std::string name = "get/" + std::string(fileName);

    // Encode and transmit file
    std::ifstream file(name, std::ios::binary);

    if (!file) {
        h->logWarning("Unable to read from file for file transfer");

        return;
    }
    file.seekg(0, std::ios::end);
    uint32_t length = (uint32_t)file.tellg();
    file.seekg(0, std::ios::beg);

    if (length < 0) {
        h->logWarning("Unable to get file length for file transfer");

        return;
    }
    char* buffer = new char[length];
    file.read(buffer, length);

    h->postRR(generateFileTransfer(remoteName, buffer, length));
    delete[]buffer;
}


//////// Turret ////////

export void __stdcall handleCreateTurret(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Turreter ident
    //      3      2      Turretee ident (when -1, detaching)
    if (len != 5) {
        handleUnknown(m);

        return;
    }
    uint16_t tid1 = getShort(msg, 1);
    uint16_t tid3 = getShort(msg, 3);

    Player& turreter{ h->getPlayer(tid1) };

    if (!turreter.isAssigned()) {
        return;
    }
    if (tid3 == UnassignedId) {
        // Unattaching
        Player& turretee{ h->getPlayer(turreter.turretId) };

        if (!turretee.isAssigned()) {
            return;
        }
        h->killTurreter(turreter);
        //        h->logEvent("{} detached from {}", turreter->name, turretee->name);
    }
    else {
        // Attaching
        Player& turretee{ h->getPlayer(tid3) };

        if (!turretee.isAssigned()) {
            return;
        }
        turreter.turretId = tid3;
        h->raiseEvent(BotEvent::CreateTurret, turreter, turretee);
        //        h->logEvent("{} turreted {}", turreter->name, turretee->name);
    }
}


export void __stdcall handleDeleteTurret(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Player ident
    if (len != 3) {
        handleUnknown(m);

        return;
    }
    Player& p{ h->getPlayer(getShort(msg, 1)) };

    if (!p.isAssigned()) {
        return;
    }
    // Shaking off turrets
    h->killTurret(p);
    h->logEvent("Deleted turret: {}", p.name);
}


//////// Flags ////////

export void __stdcall handleTurfFlagStatus(HostMessage& m)
{
    DEFLATE_CLASS

    // Field    Length    Description
    //       0        1    Type uint8_t
    // The following are repeated until the end of the message
    //       1        2    Team for flag X
    int32_t index{ 1 };
    uint16_t flag{ 0 };

    if ((len - 1) & 1) {
        handleUnknown(m);

        return;
    }

    while (index < len) {
        Flag& f = h->findFlag(flag);
        f.team = getShort(msg, index);
        h->raiseEvent(BotEvent::FlagMove, f);

        index += 2;
        ++flag;
    }

    if (h->gotMap) {
        if (!h->loadedFlags) {
            h->loadTurfFlags();
            h->logEvent("Loaded turf flags from map data.");

            h->loadedFlags = true;
        }
    }

    h->gotTurf = true;
    //    h->logEvent("Got turf update");
}


export void __stdcall handleFlagPosition(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Flag ident
    //      3      2      X tiles
    //      5      2      Y tiles
    //      7      2      Team
    if (len != 9) {
        handleUnknown(m);

        return;
    }

    uint16_t flag = getShort(msg, 1);
    uint16_t x = getShort(msg, 3);
    uint16_t y = getShort(msg, 5);
    uint16_t team = getShort(msg, 7);

    Flag& f = h->findFlag(flag);

    f.x = x;
    f.y = y;
    f.team = team;
    //    h->postRR(generateFlagRequest(flag));
    //    h->postRR(generateFlagDrop());
    h->raiseEvent(BotEvent::FlagMove, f);
}


export void __stdcall handleFlagVictory(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Team
    //      3      4      Points
    if (len != 7) {
        handleUnknown(m);

        return;
    }
    uint16_t team = getShort(msg, 1);
    uint32_t points = getLong(msg, 3);

    if (team == UnassignedId) {
        // Flag game reset
        h->logEvent("Flag game reset. (why is this shown?)");
        h->raiseEvent(BotEvent::FlagGameReset);

        for (Player& p : h->players | std::views::values) {
            p.flagCount = 0;
        }
    }
    else {
        // Team victory
        h->logEvent("Team #{} won the flag game for {} points.", team, points);

        // Find winning players
        for (Player& p : h->players | std::views::values) {
            if (p.team == team && p.ship != Ship::Spectator && !p.safety)
                p.score.flagPoints += points;

            p.flagCount = 0;
        }
        h->raiseEvent(BotEvent::FlagVictory, team, points);
    }
    h->flags.clear(); // idea by 50%packetloss
}


export void __stdcall handleFlagDrop(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Player ident
    if (len != 3) {
        handleUnknown(m);

        return;
    }
    uint16_t ident = getShort(msg, 1);

    h->dropFlags(ident);
    //    h->logEvent("Flags owned by player #{} dropped", ident);
}


export void __stdcall handleFlagClaim(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Flag ident
    //      3      2      Player ident
    if (len != 5) {
        handleUnknown(m);

        return;
    }
    uint16_t flag = getShort(msg, 1);
    uint16_t player = getShort(msg, 3);

    h->claimFlag(flag, player);
    //    h->logEvent("Flag #{} owned by player #{}", flag, player);
}


export void __stdcall handleFlagReward(HostMessage& m)
{
    DEFLATE_CLASS

    // Field    Length    Description
    //      0        1    Type uint8_t
    // The following are repeated until the end of the message
    //      1        2    Team
    //      3        2    Points
    int32_t index = 1;

    if ((len - 1) & 3) {
        handleUnknown(m);

        return;
    }

    while (index < len) {
        uint16_t team = getShort(msg, index);
        uint32_t points = getShort(msg, index + 2);
        // h->logEvent("Flag reward for team {}: {}", team, points);

        for (Player& p : h->players | std::views::values) {
            if (p.team == team && !p.safety) // safety zone fix by whommy
                p.score.flagPoints += points;
        }
        h->raiseEvent(BotEvent::FlagReward, team, points);
        index += 4;
    }
}


//////// Players ////////

export void __stdcall handlePlayerVoice(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Player ident
    //           ...      Waveform
    if (len < 3) {
        handleUnknown(m);

        return;
    }
    uint16_t ident = getShort(msg, 1);
    Player& p{ h->getPlayer(ident) };

    if (!p.isAssigned()) {
        return;
    }
    h->logEvent("Voice: {}", p.name);
}


export void __stdcall handleScoreReset(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Player ident
    if (len != 3) {
        handleUnknown(m);

        return;
    }
    uint16_t ident = getShort(msg, 1);

    if (ident != UnassignedId) {
        Player& p{ h->getPlayer(ident) };

        if (!p.isAssigned()) {
            return;
        }
        //        h->logEvent("Score reset: {}", p.name);

        p.score.flagPoints = 0;
        p.score.killPoints = 0;
        p.score.wins = 0;
        p.score.losses = 0;

        h->raiseEvent(BotEvent::PlayerScore, p);
    }
    else {
        //        h->logEvent("Score reset: All");
        for (Player& p : h->players | std::views::values) {
            p.score.flagPoints = 0;
            p.score.killPoints = 0;
            p.score.wins = 0;
            p.score.losses = 0;

            h->raiseEvent(BotEvent::PlayerScore, p);
        }
    }
}


export void __stdcall handlePlayerPrize(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      4      Timestamp
    //      5      2      X tiles
    //      7      2      Y tiles
    //      9      2      Prize type
    //      11     2      Player ident
    if (len != 13) {
        handleUnknown(m);

        return;
    }
    uint32_t timestamp = h->getLocalTime(getLong(msg, 1));
    uint16_t x = getShort(msg, 5);
    uint16_t y = getShort(msg, 7);
    uint16_t prize = getShort(msg, 9);

    //    h->ps.killGreen(timestamp, x, y);

    Player& p{ h->getPlayer(getShort(msg, 11)) };

    if (!p.isAssigned()) {
        return;
    }

    if (timestamp > p.lastPositionUpdate) {
        ++p.bounty;
        p.move(x << 4, y << 4);
    }
    h->raiseEvent(BotEvent::PlayerPrize, p, prize);
    //    h->logEvent("{} picked up prize #{} [t={}] ({}, {})", p.name, prize, timestamp, x, y);
}


export void __stdcall handleSpeedStats(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Best
    //      2      2      Your rank
    //      4      4      Your score
    //      8      4      Player 1 score
    //     12      4      Player 2 score
    //     16      4      Player 3 score
    //     20      4      Player 4 score
    //     24      4      Player 5 score
    //     28      2      Player 1 ident
    //     30      2      Player 2 ident
    //     32      2      Player 3 ident
    //     34      2      Player 4 ident
    //     36      2      Player 5 ident
    if (len != 38) {
        handleUnknown(m);
        return;
    }
    uint8_t best = getByte(msg, 1);
    uint16_t myRank = getShort(msg, 2);
    uint32_t myScore = getLong(msg, 4);

    uint16_t ident1 = getShort(msg, 28);
    uint16_t ident2 = getShort(msg, 30);
    uint16_t ident3 = getShort(msg, 32);
    uint16_t ident4 = getShort(msg, 34);
    uint16_t ident5 = getShort(msg, 36);

    Player& p1{ h->getPlayer(ident1) };
    Player& p2{ h->getPlayer(ident2) };
    Player& p3{ h->getPlayer(ident3) };
    Player& p4{ h->getPlayer(ident4) };
    Player& p5{ h->getPlayer(ident5) };

    uint32_t score1 = getShort(msg, 8);
    uint32_t score2 = getShort(msg, 12);
    uint32_t score3 = getShort(msg, 16);
    uint32_t score4 = getShort(msg, 20);
    uint32_t score5 = getShort(msg, 24);

    h->logEvent("Timed-Game Over!");

    if (p1.isAssigned()) h->logEvent("#1    {} {}", score1, p1.name);
    if (p2.isAssigned()) h->logEvent("#2    {} {}", score2, p2.name);
    if (p3.isAssigned()) h->logEvent("#3    {} {}", score3, p3.name);
    if (p4.isAssigned()) h->logEvent("#4    {} {}", score4, p4.name);
    if (p5.isAssigned()) h->logEvent("#5    {} {}", score5, p5.name);

    h->logEvent("Your Rank: #{}  Your Score: {}", myRank, myScore);
    h->raiseEvent(BotEvent::TimedGameOver, p1, p2, p3, p4, p5);
}


export void __stdcall handlePlayerEntering(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    // The following are repeated until the end of the message
    //      0      1      Type uint8_t
    //      1      1      Ship type
    //      2      1      Accepts audio messages
    //      3      20     Player name (confirmed ASCIIZ)
    //     23      20     Squad name (confirmed ASCIIZ)
    //     43      4      Flag points
    //     47      4      Kill points
    //     51      2      Player ident
    //     53      2      Team
    //     55      2      Wins
    //     57      2      Losses
    //     59      2      Turretee ident
    //     61      2      Flags carried
    //     63      1      Boolean: Has KoTH
    int32_t index = 0;

    if (len & 63) {
        handleUnknown(m);
        return;
    }

    while (index < len) {
        Ship ship = (Ship)getByte(msg, index + 1);
        bool acceptsAudio = (getByte(msg, index + 2) != 0);
        std::string name{ (char*)&msg[index + 3] };
        std::string squad{ (char*)&msg[index + 23] };
        uint32_t killPoints = getLong(msg, index + 43);
        uint32_t flagPoints = getLong(msg, index + 47);
        PlayerId ident = getShort(msg, index + 51);
        uint16_t team = getShort(msg, index + 53);
        uint16_t wins = getShort(msg, index + 55);
        uint16_t losses = getShort(msg, index + 57);
        PlayerId turreteeId = getShort(msg, index + 59);
        uint16_t flagCount = getShort(msg, index + 61);
        bool hasKoTH = getByte(msg, index + 63) ? 1 : 0;

        if (h->hasPlayer(ident)) {
            // Remove player with an already existing Id
            h->killPlayer(h->players[ident]);
        }
        Player& player = h->addPlayer(ident, name, squad, flagPoints, killPoints, team, wins,
            losses, ship, acceptsAudio, flagCount);

        player.turretId = turreteeId;
        player.koth = hasKoTH;

        if (ident == h->playerId) {
            if (hasKoTH) {
                h->postRR(generateKoTHReset());
            }
            player.koth = false;
            h->logEvent("Player entering: {} #{} w/l {}:{} pts {}+{} <-- Me", name, ident, wins, losses, killPoints, 
                flagPoints);
        }
        else if (h->billerOnline) {
            // Don't do this if the biller is down
            OpEntry* op = getBotDatabase().findOperator(player.name);

            if (op) {    // Listed
                if (op->validatePass("")) {    // No login password
                    player.access = op->getAccess();
                    op->addCounter();
                }
                h->logEvent("Player entering: {} #{} w/l {}:{} pts {}+{} <-- Op", name, ident, wins, losses, 
                    killPoints, flagPoints);
            }
            else {
                h->logEvent("Player entering: {} #{} w/l {}:{} pts {}+{}", name, ident, wins, losses, killPoints, 
                    flagPoints);
            }
        }
        else {
            h->logEvent("Player entering: {} #{} w/l {}:{} pts {}+{}", name, ident, wins, losses, killPoints, 
                flagPoints);
        }

        if (h->inArena) {
            h->raiseEvent(BotEvent::PlayerEntering, player);
        }
        index += 64;
    }
}


export void __stdcall handleSetTeam(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Player ident
    //      3      2      Team
    //      5      1      Ship changed, only if high bit is not set (Snrrrub)
    if (len != 6) {
        handleUnknown(m);
        return;
    }
    uint16_t ident = getShort(msg, 1);
    Player& player{ h->getPlayer(ident) };

    if (!player.isAssigned()) {
        return;
    }
    uint16_t team = getShort(msg, 3);
    Ship ship = (Ship)getByte(msg, 5);

    uint16_t oldteam = player.team;
    Ship oldship = player.ship;

    if ((ship == Ship::Spectator) && (player.ship != ship)) {   // implicitly fails if high bit is set on ship type
        if (h->follow != UnassignedId && (player.ident == UnassignedId)) {
            h->follow = UnassignedId;
        }
        player.team = team;
        player.ship = ship;
        h->raiseEvent(BotEvent::PlayerSpec, player, oldteam, oldship);
    }
    else {
        if (player.team != team) {
            player.team = team;
            h->raiseEvent(BotEvent::PlayerTeam, player, oldteam, oldship);
        }
        // if high bit is not set, then a ship change has occured
        if (((int)ship & 128) == 0 && player.ship != ship) {
            player.ship = (enum Ship)ship;
            h->raiseEvent(BotEvent::PlayerShip, player, oldteam, oldship);
        }
    }

    if (ident == h->playerId) {
        if (player.team != team) {
            h->resetIcons();
        }
    }
    player.flagCount = 0;
    //    h->logEvent("Player changed teams: {} {{}}", p.name, getByte(msg, 5));
}


export void __stdcall handleSetTeamAndShip(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Ship type
    //      2      2      Player ident
    //      4      2      Team
    if (len != 6) {
        handleUnknown(m);
        return;
    }
    Ship ship = (Ship)msg[1];
    uint16_t ident = getShort(msg, 2);
    Player& player{ h->getPlayer(ident) };

    if (!player.isAssigned()) {
        return;
    }
    uint16_t team = getShort(msg, 4);

    //    h->logEvent("Player changed team and ship: {}", p.name);

    uint16_t oldteam = player.team;
    Ship oldship = player.ship;

    if ((ship == Ship::Spectator) && (player.ship != ship)) {
        if (h->follow != UnassignedId && (player.ident == h->follow))
            h->follow = UnassignedId;

        player.team = team;
        player.ship = ship;

        h->raiseEvent(BotEvent::PlayerSpec, player, oldteam, oldship);
    }
    else {
        if (player.team != team) {
            player.team = team;
            h->raiseEvent(BotEvent::PlayerTeam, player, oldteam, oldship);
        }

        if (player.ship != ship) {
            player.ship = (enum Ship)ship;
            h->raiseEvent(BotEvent::PlayerShip, player, oldteam, oldship);
        }
    }

    if (ident == h->playerId) {
        if (player.team != team) {
            h->resetIcons();
        }
    }
    player.flagCount = 0;
}


export void __stdcall handlePlayerBanner(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Player ident
    //      3     96      Player banner
    if (len != 99) {
        handleUnknown(m);
        return;
    }
    uint16_t ident = getShort(msg, 1);
    Player& player{ h->getPlayer(ident) };

    if (!player.isAssigned()) {
        return;
    }
    uint8_t* banner = (uint8_t*)&msg[3];

    player.setBanner(banner);

    h->raiseEvent(BotEvent::BannerChanged, player);
    //    h->logEvent("Set banner: {}", player.name);
}


export void __stdcall handlePlayerLeaving(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Player ident

    if (len != 3) {
        handleUnknown(m);
        return;
    }
    uint16_t ident = getShort(msg, 1);
    Player& player{ h->getPlayer(ident) };

    if (!player.isAssigned()) {
        return;
    }
    h->logEvent("Player leaving: {}", player.name);
    h->raiseEvent(BotEvent::PlayerLeaving, player);
    h->killPlayer(player);
}


export void __stdcall handleChat(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Message type
    //      2      1      Soundcode
    //      3      2      Player ident
    //      5  ...\0      Message
    if (len <= 5) {
        h->logWarning("Chat message buffer underrun ignored.");
        h->logIncoming(msg, len);
        return;
    }

    if (msg[len - 1]) {
        h->logWarning("Unterminated chat message ignored.");
        h->logIncoming(msg, len);
        return;
    }
    ChatMode chatMode = (ChatMode)getByte(msg, 1);
    ChatSoundCode soundCode = (ChatSoundCode)getByte(msg, 2);
    std::string text{ (char*)&msg[5] };

    if (text.empty()) {
        return;
    }
    uint16_t ident = getShort(msg, 3);
    Player& player{ h->getPlayer(ident) };
    
    h->raiseEvent(BotEvent::Chat, chatMode, soundCode, player, text);
    
    switch (chatMode) {
    case ChatMode::Arena:
        h->logEvent(std::format("A: {}", text), LogLevel::Verbose);

        if (!h->hasMod) {
            Player& myself = h->me();

            if (myself.isAssigned()) {
                std::string s = myself.name + " - Sysop - ";

                if (text.starts_with(s)) {
                    // SysOp checker
                    h->logEvent("^^ I Am SysOp.  Hear me roar!", LogLevel::Verbose);
                    h->hasSysOp = true;
                    h->hasSMod = true;
                    h->hasMod = true;
                    break;
                }

                s = myself.name + " - SMod - ";

                if (text.starts_with(s)) {
                    // SMod checker
                    h->logEvent("^^ I Am SuperModerator.  Hear me roar!", LogLevel::Verbose);
                    h->hasSysOp = false;
                    h->hasSMod = true;
                    h->hasMod = true;
                    break;
                }
            }
        }
        break;
    case ChatMode::PublicMacro:
        if (player.isAssigned()) {
            h->logEvent(std::format("MAC: {}> {}", player.name, text), LogLevel::Verbose);

            if (!getBotDatabase().disablePub)
                if (text.starts_with('!') || text.starts_with('.') || text.starts_with('@'))
                    gotCommand(*h, player, text.substr(1));
        }
        else {
            h->logEvent(std::format("MAC: {}", text), LogLevel::Verbose);
        }
        break;
    case ChatMode::Public:
        if (player.isAssigned()) {
            h->logEvent(std::format("P: {}> {}", player.name, text), LogLevel::Verbose);

            if (!getBotDatabase().disablePub)
                if (text.starts_with('!') || text.starts_with('.') || text.starts_with('@'))
                    gotCommand(*h, player, text.substr(1));
        }
        else {
            h->logEvent(std::format("P: {}", text), LogLevel::Verbose);
        }
        break;
    case ChatMode::Team:
        if (player.isAssigned()) {
            h->logEvent(std::format("T:{}> {}", player.name, text), LogLevel::Verbose);

            if (text.starts_with('!') || text.starts_with('.') || text.starts_with('@'))
                gotCommand(*h, player, text.substr(1));
        }
        else {
            h->logEvent(std::format("T: {}", text), LogLevel::Verbose);
        }
        break;
    case ChatMode::TeamPrivate:
        if (player.isAssigned()) {
            h->logEvent(std::format("TP: {}> {}", player.name, text), LogLevel::Verbose);

            if (text.starts_with('!') || text.starts_with('.') || text.starts_with('@'))
                gotCommand(*h, player, text.substr(1));
        }
        else {
            h->logEvent(std::format("TP: {}", text), LogLevel::Verbose);
        }
        break;
    case ChatMode::Private:
        if (player.isAssigned()) {
            h->logEvent(std::format("PV: {}> {}", player.name, text), LogLevel::Verbose);

            if (text.starts_with('!') || text.starts_with('.') || text.starts_with('@'))
                gotCommand(*h, player, text.substr(1));
        }
        else {
            h->logEvent(std::format("PV: {}", text), LogLevel::Verbose);
        }
        break;
    case ChatMode::PlayerWarning:
        if (player.isAssigned())
            h->logEvent(std::format("W: {}> {}", player.name, text), LogLevel::Verbose);
        else
            h->logEvent(std::format("W: {}", text), LogLevel::Verbose);
        break;
    case ChatMode::RemotePrivate:
        h->logEvent(std::format("REM: {}", text), LogLevel::Verbose);

        if (getBotDatabase().remoteInterpreter && isValidRemotePrivateChatMessage(text)) {
            std::string message{ getRemoteChatMessageText(text) };
            std::string sender{ getRemoteChatPlayerName(text) };

            if (message.starts_with('!') || message.starts_with('.') || message.starts_with('@'))
                gotRemoteCommand(*h, sender, message.substr(1));
        }
        break;
    case ChatMode::ServerError:
        h->logEvent(std::format("ERR: {}", text), LogLevel::Verbose);

        if (h->broadcastingErrors) {
            h->sendChannel(format("1;{}", text));
        }
        break;
    case ChatMode::Channel:
        h->logEvent(std::format("C: {}", text), LogLevel::Verbose);

        if (getBotDatabase().remoteInterpreter && isValidChatMessage(text)) {
            std::string message{ getChatMessageText(text) };
            std::string sender{ getChatPlayerName(text) };

            if (message.starts_with('!') || message.starts_with('.') || message.starts_with('@'))
                gotRemoteCommand(*h, sender, message.substr(1));
        }
        break;
    default:
        h->logWarning("Unknown chat message ignored. ({})", (int)chatMode);
        h->logIncoming(msg, len);
        return;
    };
}


export void __stdcall handleScoreUpdate(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Player ident
    //      3      4      Flag points
    //      7      4      Kill points
    //     11      2      Wins
    //     13      2      Losses
    if (len != 15) {
        handleUnknown(m);
        return;
    }
    Player& p{ h->getPlayer(getShort(msg, 1)) };

    if (!p.isAssigned()) {
        return;
    }
    uint32_t killPoints = getLong(msg, 3);
    uint32_t flagPoints = getLong(msg, 7);
    uint16_t wins = getShort(msg, 11);
    uint16_t losses = getShort(msg, 13);

    //    h->logEvent("Score update: {} w/l {}:{} pts {}+{}", p.name, wins, losses, killPoints, flagPoints);
    p.score.killPoints = killPoints;
    p.score.flagPoints = flagPoints;
    p.score.wins = wins;
    p.score.losses = losses;
    h->raiseEvent(BotEvent::PlayerScore, p);
}


export void __stdcall handleWeaponUpdate(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Rotation
    //      2      2      Timestamp
    //      4      2      X pixels
    //      6      2      Y velocity
    //      8      2      Player ident
    //     10      2      X velocity
    //     12      1      Checksum
    //     13      1      Togglables
    //     14      1      Ping
    //     15      2      Y pixels
    //     17      2      Bounty
    //     19      2      Weapon info
    // Spectating with ExtraPositionData or *energy
    //     21      2      Energy
    // Spectating with ExtraPositionData
    //     23      2      S2CLag
    //     25      2      Timer
    //     27      4      Item information
    if ((len != 21) && (len != 23) && (len != 31)) {
        handleUnknown(m);
        return;
    }
    StateInfo si{};
    WeaponInfo wi{};
    ItemInfo ii;

    bool gotItems,
        gotEnergy;

    if (len >= 23)
        gotEnergy = true;
    else
        gotEnergy = false;

    if (len == 31)
        gotItems = true;
    else
        gotItems = false;

    uint8_t direction = getByte(msg, 1);

    // calculate timestamp (straight from subspace)
    uint32_t loword = getShort(msg, 2);
    uint32_t timestamp = h->getHostTime() & 0x7FFFFFFF;

    if ((timestamp & 0x0000FFFF) >= loword) {
        timestamp &= 0xFFFF0000;
    }
    else {
        timestamp &= 0xFFFF0000;
        timestamp -= 0x00010000;
    }
    timestamp |= loword;    // fill in the low word

    // calculate transit time (straight from subspace)
    int32_t transit_time = h->getHostTime() - timestamp;

    if ((transit_time < 0) || (transit_time > 30000)) {
        transit_time = 0;
    }
    else if (transit_time > 4000) {
        transit_time = 15;
    }
    uint16_t x = getShort(msg, 4);
    int16_t yvel = getShort(msg, 6);
    uint16_t ident = getShort(msg, 8);
    Player& p{ h->getPlayer(ident) };

    if (!p.isAssigned()) {
        return;
    }

    int16_t xvel = getShort(msg, 10);
    uint8_t checksum = getByte(msg, 12);
    si.value = getByte(msg, 13);
    uint8_t ping = getByte(msg, 14);
    uint16_t y = getShort(msg, 15);
    uint16_t bounty = getShort(msg, 17);
    wi.value = getShort(msg, 19);

    p.lastPositionUpdate = timestamp;

    if (x > 0x7fff) 
        x = ~x + 1;    // Fix by Snrrrub
    if (y > 0x7fff) 
        y = ~y + 1;
    msg[12] = 0;

    if (checksum != simpleChecksum(&msg[0], 21)) {
        h->logWarning("Subgame is possibly hacked. Position checksum mismatch from {}", p.name);
        return;
    }

    if (wi.type != Projectile::None)
        ++h->weaponCount;

    //    if (p.lastPositionUpdate > timestamp) return;
    p.d = direction;
    p.bounty = bounty;

    if (gotEnergy)
        p.energy = getShort(msg, 21);
    else
        p.energy = 0;

    if (gotItems) {
        p.S2CLag = getShort(msg, 23);
        p.timer = getShort(msg, 25);
    }
    else {
        p.S2CLag = 0;
        p.timer = 0;
    }

    p.stealth = si.stealth;
    p.cloak = si.cloaked;
    p.xradar = si.xradar;
    p.awarp = si.awarp;
    p.ufo = si.ufo;
    p.safety = si.safety;
    p.flash = si.flash;

    if (gotItems) {
        ii.value = getLong(msg, 27);
        p.shields = ii.shields;
        p.supers = ii.supers;
        p.burst = ii.burst;
        p.repel = ii.repel;
        p.thor = ii.thor;
        p.brick = ii.brick;
        p.decoy = ii.decoy;
        p.rocket = ii.rocket;
        p.portal = ii.portal;
    }

    // prediction
    p.move(x, y, xvel, yvel);
    p.move(transit_time);

    h->raiseEvent(BotEvent::PlayerMove, p);

    if (wi.type != Projectile::None)
        h->raiseEvent(BotEvent::PlayerWeapon, p, wi);

    if (h->follow != UnassignedId && p.ident == h->follow) {
        Player myself = h->me();

        if (myself.isAssigned()) {
            myself.clone(p);

            if (myself.ship != Ship::Spectator) {
                switch (wi.type)
                {
                case Projectile::Bullet:
                case Projectile::BBullet:
                    if (!h->hasSysOp && h->settings.ships[(int)myself.ship].MaxGuns == 0) {
                        h->sendPosition(timestamp, false);
                        return;
                    }

                    wi.level = (WeaponLevel)limit((uint32_t)wi.level, h->settings.ships[(int)myself.ship].MaxGuns - 1);
                    wi.fireType = limit(wi.fireType, h->settings.ships[(int)myself.ship].DoubleBarrel);

                    if (!h->settings.pw.BouncingBullets)
                        wi.type = Projectile::Bullet;
                    break;
                case Projectile::Bomb:
                case Projectile::PBomb:
                    if (!h->hasSysOp && h->settings.ships[(int)myself.ship].MaxBombs == 0) {
                        h->sendPosition(timestamp, false);
                        return;
                    }

                    wi.level = (WeaponLevel)limit((uint32_t)wi.level, h->settings.ships[(int)myself.ship].MaxBombs - 1);
                    wi.shrapCount = limit(wi.shrapCount, h->settings.ships[(int)myself.ship].ShrapnelMax);
                    wi.shrapLevel = limit(wi.shrapLevel, h->settings.ships[(int)myself.ship].MaxBombs - 1);
                    wi.shrapBounce = limit(wi.shrapBounce, !!h->settings.pw.BouncingBullets);
                    wi.fireType = limit(wi.fireType, !!h->settings.ships[(int)myself.ship].MaxMines);
                    break;
                case Projectile::Decoy:
                    if (!h->hasSysOp && h->settings.ships[(int)myself.ship].DecoyMax == 0) {
                        h->sendPosition(timestamp, false);
                        return;
                    }
                    break;
                case Projectile::Thor:
                    if (!h->hasSysOp && h->settings.ships[(int)myself.ship].ThorMax == 0) {
                        h->sendPosition(timestamp, false);
                        return;
                    }
                    break;
                case Projectile::Repel:
                    if (!h->hasSysOp && h->settings.ships[(int)myself.ship].RepelMax == 0) {
                        h->sendPosition(timestamp, false);
                        return;
                    }
                    break;
                case Projectile::Burst:
                    if (!h->hasSysOp && h->settings.ships[(int)myself.ship].BurstMax == 0) {
                        h->sendPosition(timestamp, false);
                        return;
                    }
                    break;
                };

                h->sendPosition(false, timestamp, wi.type, wi.level, wi.shrapBounce, wi.shrapLevel, wi.shrapCount, wi.fireType);
            }
        }
    }
    //    h->logEvent("[{}] Got player/weapon position: {} ({}, {}) {{}}", timestamp, p.name, x, y, si.pack);
}


export void __stdcall handlePlayerDeath(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Death green
    //      2      2      Killer ident
    //      4      2      Killee ident
    //      6      2      Bounty
    //      8      2      Number of flags gained by killing player (Snrrrub)
    if (len != 10) {
        handleUnknown(m);
        return;
    }
    Player& killer{ h->getPlayer(getShort(msg, 2)) };
    Player& killee{ h->getPlayer(getShort(msg, 4)) };

    int32_t bounty = (int16_t)getShort(msg, 6);
    uint16_t flags = getShort(msg, 8);

    if (killer.isAssigned()) {
        Player& myself{ h->me() };

        //This message is followed by a score update,
        //because kills are not visible all the time.
        //The killee & killer are assumed to have updated
        //their own scores, so no score updates are sent.   
        if (killer.flagCount > 0) {    // Apparently bounty is only filtered server-side
            bounty *= h->settings.FlaggerKillMultiplier + 1;
        }

        killer.bounty += h->settings.BountyIncreaseForKill;

        ++killer.score.wins;
        killer.score.killPoints += bounty;

        // Flag drop
        if (killer.ident == myself.ident) {
            //h->postRR(generateFlagDrop());
            // NEUT! FFS! Bots winning flag games is really lame.

            // Find ship the bot is not in.
            Ship ship{ Ship::Warbird };

            if (myself.ship == Ship::Warbird)
                ship = Ship::Javelin;

            Ship old{ myself.ship };

            h->postRR(generateChangeShip(ship));
            h->postRR(generateChangeShip(old));
        }
        killer.flagCount += flags;
    }

    if (killee.isAssigned()) {
        ++killee.score.losses;

        if (killer.isAssigned()) {
            //h->logEvent("{}({}) killed by: {} {{}} [{}]", killee->name, bounty, killer->name, getByte(msg, 1), 
            // getShort(msg, 8));
            h->raiseEvent(BotEvent::PlayerDeath, killee, killer, bounty, flags);
        }
        killee.flagCount = 0;
    }
}


export void __stdcall handleSpecPlayer(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    // Either             Player joining spectator mode
    //      1      2      Player ident
    // Or                 Someone is requesting extra position information
    //      1      1      Boolean: send ExtraPositionInfo
    if (len == 2) {
        bool watched = (getByte(msg, 1) != 0);

        if (watched) {
            h->logEvent("Paranoid flag: Someone's requesting ExtraPositionInfo");
        }
        else {
            h->logEvent("Paranoid flag: Off");
        }
        h->paranoid = watched;
    }
    else if (len == 3) {
        Player& p{ h->getPlayer(getShort(msg, 1)) };

        if (!p.isAssigned()) {
            return;
        }
        //        h->logEvent("Player spectated: {}", p.name);

        Ship oldship = (Ship)p.ship;
        uint16_t oldteam = p.team;

        p.ship = Ship::Spectator;
        h->raiseEvent(BotEvent::PlayerSpec, p, oldteam, oldship);
        p.flagCount = 0;
    }
    else {
        handleUnknown(m);

        return;
    }
}


export void __stdcall handlePlayerPosition(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Rotation
    //      2      2      Time stamp
    //      4      2      X pixels
    //      6      1      Ping
    //      7      1      Bounty
    //      8      1      Player ident
    //      9      1      Togglables
    //     10      2      Y velocity
    //     12      2      Y pixels
    //     14      2      X velocity
    // Spectating with ExtraPositionData or *energy
    //     16      2      Energy
    // Spectating with ExtraPositionData
    //     18      2      S2CLag
    //     20      2      Timer
    //     22      4      Item information
    if ((len != 16) && (len != 18) && (len != 26)) {
        handleUnknown(m);
        return;
    }
    StateInfo si;
    ItemInfo ii;

    bool gotItems,
        gotEnergy;

    if (len >= 18)
        gotEnergy = true;
    else
        gotEnergy = false;

    if (len == 26)
        gotItems = true;
    else
        gotItems = false;

    uint8_t direction = getByte(msg, 1);

    // calculate timestamp (straight from subspace)
    uint32_t loword = getShort(msg, 2);
    uint32_t timestamp = h->getHostTime() & 0x7FFFFFFF;

    if ((timestamp & 0x0000FFFF) >= loword) {
        timestamp &= 0xFFFF0000;
    }
    else {
        timestamp &= 0xFFFF0000;
        timestamp -= 0x00010000;
    }
    timestamp |= loword;    // fill in the low word

    // calculate transit time (straight from subspace)
    int32_t transit_time = h->getHostTime() - timestamp;

    if ((transit_time < 0) || (transit_time > 30000)) {
        transit_time = 0;
    }
    else if (transit_time > 4000) {
        transit_time = 15;
    }
    uint16_t x = getShort(msg, 4);
    uint8_t ping = getByte(msg, 6);
    uint8_t bounty = getByte(msg, 7);
    uint16_t ident = getByte(msg, 8);
    Player& p{ h->getPlayer(ident) };

    if (!p.isAssigned()) {
        return;
    }
    si.value = getByte(msg, 9);
    int16_t yvel = getShort(msg, 10);
    uint16_t y = getShort(msg, 12);
    int16_t xvel = getShort(msg, 14);

    p.lastPositionUpdate = timestamp;

    if (x > 0x7fff) 
        x = ~x + 1;    // Fix by Snrrrub
    if (y > 0x7fff) 
        y = ~y + 1;

    //    if (p.lastPositionUpdate > timestamp) return;

    p.d = direction;
    p.bounty = bounty;

    if (gotEnergy)
        p.energy = getShort(msg, 16);
    else
        p.energy = 0;

    if (gotItems) {
        p.S2CLag = getShort(msg, 18);
        p.timer = getShort(msg, 20);
    }
    else {
        p.S2CLag = 0;
        p.timer = 0;
    }

    p.stealth = si.stealth;
    p.cloak = si.cloaked;
    p.xradar = si.xradar;
    p.awarp = si.awarp;
    p.ufo = si.ufo;
    p.safety = si.safety;
    p.flash = si.flash;

    if (gotItems) {
        ii.value = getLong(msg, 22);
        p.shields = ii.shields;
        p.supers = ii.supers;
        p.burst = ii.burst;
        p.repel = ii.repel;
        p.thor = ii.thor;
        p.brick = ii.brick;
        p.decoy = ii.decoy;
        p.rocket = ii.rocket;
        p.portal = ii.portal;
    }

    // prediction
    p.move(x, y, xvel, yvel);
    p.move(transit_time);

    h->raiseEvent(BotEvent::PlayerMove, p);

    if (h->follow != UnassignedId && p.ident == h->follow) {
        Player& myself = h->me();

        if (myself.ident != UnassignedId) {
            myself.clone(p);

            if (myself.ship == Ship::Spectator) {
                if (h->speccing) {
                    h->spectate(NULL);
                }
            }
            else {
                h->sendPosition(timestamp, false);
            }
        }
    }
    //    h->logEvent("[{}] Got player position: {} ({}, {}) {{}:{}}", timestamp, p.name, x, y, bounty, si.pack);
}


export void __stdcall handleKoTHReset(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Adding KoTH timer
    //      2      4      Timer value
    //      6      2      Player ident
    if (len != 8) {
        handleUnknown(m);
        return;
    }
    bool adding = getByte(msg, 1) != 0;
    uint32_t timer = getLong(msg, 2);
    uint16_t ident = getShort(msg, 6);
    Player& p{ h->getPlayer(ident) };

    if (adding) {
        if (ident != UnassignedId) {
            // Add to one
            if (!p.isAssigned()) {
                return;
            }
            //            h->logEvent("KoTH add: {}", p.name);
            p.koth = true;
        }
        else {
            // Add to all
//            h->logEvent("KoTH add: all");

            for (Player& p : h->players | std::views::values) {
                p.koth = true;
            }
            h->postRR(generateKoTHReset());
        }
    }
    else {
        if (ident != UnassignedId) {
            // Remove from one
            if (!p.isAssigned()) {
                return;
            }
            //            h->logEvent("KoTH remove: {}", p.name);
            p.koth = false;
        }
        else {
            // Remove from all
//            h->logEvent("KoTH remove: all");

            for (Player& p : h->players | std::views::values) {
                p.koth = false;
            }
        }
    }
}


export void __stdcall handleAddKoTH(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      4      Additional time
    if (len != 5) {
        handleUnknown(m);
        return;
    }
    uint32_t timerdiff = getLong(msg, 1);

    //h->logEvent("KoTH timer, plus {}", timerdiff);
    h->postRR(generateKoTHReset());
}


export void __stdcall handleWatchDamage(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Player ident
    //      3      4      Timestamp
    // The following are repeated until the end of the message
    //      7      2      Attacker ident
    //      9      2      Weapon info
    //     11      2      Last energy
    //     13      2      Damage
    //     15      1      ?
    if ((len <= 7) || ((len - 7) % 9)) {
        handleUnknown(m);
        return;
    }
    uint16_t pid = getShort(msg, 1);
    Player& p{ h->getPlayer(pid) };

    if (!p.isAssigned()) {
        return;
    }
    uint32_t timestamp = getLong(msg, 3);

    for (int32_t i = 7; i < len; i += 9) {
        uint16_t kid = getShort(msg, i);
        Player& k{ h->getPlayer(kid) };

        if (!k.isAssigned()) {
            return;
        }
        WeaponInfo wi;
        wi.value = getShort(msg, i + 2);
        uint16_t energy = getShort(msg, i + 4);
        uint16_t damage = getShort(msg, i + 6);

        h->raiseEvent(BotEvent::WatchDamage, p, k, wi, energy, damage);
    }
}


//////// Personal ship ////////

export void __stdcall handleIdent(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      My new ident
    // Continuum
    //      3      1      Boolean: ?
    if ((len != 3) && (len != 4)) {
        handleUnknown(m);
        return;
    }
    uint16_t ident = getShort(msg, 1);

    //    h->logEvent("Personal ident: {}", ident);
    h->playerId = ident;
}


export void __stdcall handleSelfPrize(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Prize count
    //      3      2      Prize type
    if (len != 5) {
        handleUnknown(m);
        return;
    }
    uint16_t count = getShort(msg, 1);
    uint16_t prize = getShort(msg, 3);

    //    h->logEvent("I have been prized");
    h->raiseEvent(BotEvent::SelfPrize, prize, count);
}


export void __stdcall handleSetKoTHTimer(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      4      New timer value
    if (len != 5) {
        handleUnknown(m);
        return;
    }
    uint32_t timer = getLong(msg, 1);

    //    h->logEvent("KoTH timer set to {}", timer);
    h->postRR(generateKoTHReset());
}


export void __stdcall handleShipReset(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    if (len != 1) {
        handleUnknown(m);
        return;
    }
    //    h->logEvent("Personal ship reset");

    Player& myself{ h->me() };

    if (!myself.isAssigned()) {
        return;
    }
    myself.awarp = false;
    myself.bounty = 0;
    myself.brick = 0;
    myself.burst = 0;
    myself.cloak = false;
    myself.decoy = 0;
    myself.energy = 0;
    myself.portal = 0;
    myself.repel = 0;
    myself.rocket = 0;
    myself.shields = false;
    myself.stealth = false;
    myself.supers = false;
    myself.thor = 0;
    myself.timer = 0;
    myself.ufo = false;
    myself.xradar = false;

    h->raiseEvent(BotEvent::SelfShipReset);
}


export void __stdcall handleChangePosition(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      X tiles
    //      3      2      Y tiles
    if (len != 5) {
        handleUnknown(m);
        return;
    }
    uint16_t x = getShort(msg, 1);
    uint16_t y = getShort(msg, 3);
    Player& myself{ h->me() };

    if (!myself.isAssigned()) {
        return;
    }
    myself.move(x << 4, y << 4);

    h->raiseEvent(BotEvent::PlayerMove, myself);
    //    h->logEvent("Personal ship coordinates changed");
}


export void __stdcall handleReceivedObject(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      ?      ?
    if (len != 0) {
        handleUnknown(m);
        return;
    }
}


export void __stdcall handleDamageToggle(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Boolean: send damage info
    if (len != 2) {
        handleUnknown(m);
        return;
    }
    bool sendIt = (msg[1] != 0);
}


export void __stdcall handleToggleUFO(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Boolean: available
    if (len != 2) {
        handleUnknown(m);
        return;
    }
    bool ufo = getByte(msg, 1) != 0;

    //    h->logEvent("Personal UFO: {}", ufo);

    if (!h->hasBot()) {
        return;
    }
    h->me().ufo = ufo;
    h->raiseEvent(BotEvent::SelfUFO);
}


//////// Bricks ////////

export void __stdcall handleBrickDrop(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    // The following are repeated until the end of the message
    //      1      2      X1 tiles
    //      3      2      Y1 tiles
    //      5      2      X2 tiles
    //      7      2      Y2 tiles
    //      9      2      Team
    //     11      2      Brick ident (sent more than once)
    //     13      4      Timestamp

    if ((len - 1) & 15) {
        handleUnknown(m);
        return;
    }

    for (int32_t i = 1; i < len; i += 16) {
        uint16_t ident = getShort(msg, i + 10);

        if (h->brickExists(ident))
            continue;

        int16_t x1 = getShort(msg, i + 0);
        int16_t y1 = getShort(msg, i + 2);
        int16_t x2 = getShort(msg, i + 4);
        int16_t y2 = getShort(msg, i + 6);
        uint16_t team = getShort(msg, i + 8);
        Player& myself = h->me();

        if (!myself.isAssigned()) {
            continue;
        }

        uint32_t timestamp = h->getLocalTime(getLong(msg, i + 12));

        h->logEvent("Brick dropped on team #{} at {}", team, timestamp);

        // Trace brick blocks
        int16_t mx = (int16_t)sgn(x2 - x1);
        int16_t my = (int16_t)sgn(y2 - y1);
        int16_t x = x1, y = y1;

        PlayfieldTileFormat brickIcon = (team == myself.team) ? PlayfieldTileFormat::SsbTeamBrick : 
            PlayfieldTileFormat::SsbEnemyBrick;

        if (mx) {
            while (x != x2) {
                x += mx;
                h->bricks.push_back({ (uint16_t)x, (uint16_t)y, timestamp + h->settings.BrickTime, team, ident });
            }
        }
        else {
            while (y != y2) {
                y += my;
                h->bricks.push_back({ (uint16_t)x, (uint16_t)y, timestamp + h->settings.BrickTime, team, ident });
            }
        }

        h->raiseEvent(BotEvent::BrickDropped, x1, y1, x2, y2, team);
    }
}


//////// Synchronization/Security ////////

export void __stdcall handleSynchronization(HostMessage& m)
{
    DEFLATE_CLASS

    // Fields contributed by Kavar!'s research
    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      4      Green seed
    //      5      4      Door seed
    //      9      4      Timestamp (Kavar! mistook it for a seed)
    //     13      4      Checksum generator key
    if (len != 17) {
        handleUnknown(m);
        return;
    }
    uint32_t greenSeed = getLong(msg, 1);
    uint32_t doorSeed = getLong(msg, 5);
    uint32_t timestamp = h->getLocalTime(getLong(msg, 9));
    uint32_t checksumKey = getLong(msg, 13);

    h->logEvent(std::format("Got synchronization packet [{}][{}]", timestamp, checksumKey), LogLevel::All);

    //    h->ps.randomize(greenSeed, timestamp);
        // fast?
    h->S2CFastCurrent = (uint16_t)h->msgRecv;

    h->S2CSlowTotal += 0;
    h->S2CFastTotal += (uint16_t)h->msgRecv;

    if ((checksumKey != 0) && (h->gotMap)) {
        h->postRR(generateSecurityChecksum(
            generateParameterChecksum(checksumKey, (uint32_t*)&h->settings),
            generateEXEChecksum(checksumKey),
            generateLevelChecksum(checksumKey, (char*)h->playfieldTiles),
            (uint16_t)h->msgRecv,
            (uint16_t)h->syncPing,
            (uint16_t)h->avgPing,
            (uint16_t)h->lowPing,
            (uint16_t)h->highPing,
            h->weaponCount,
            h->S2CSlowCurrent,
            h->S2CFastCurrent,
            h->S2CSlowTotal,
            h->S2CFastTotal,
            false));
    }

    h->S2CFastCurrent = 0;
    h->S2CSlowCurrent = 0;

    // Start sending position updates (game)
    if (!h->position) {
        h->position = true;
        h->lastPosition = 0;
    }
    // Start sending time synchronization requests (core)
    h->inArena = true;
}


//////// Powerball ////////

export void __stdcall handleSoccerGoal(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Team
    //      3      4      Points
    if (len != 7) {
        handleUnknown(m);
        return;
    }
    uint16_t team = getShort(msg, 1);
    uint32_t points = getLong(msg, 3);

    for (Player& p : h->players | std::views::values) {
        if (p.team == team)
            p.score.flagPoints += points;
    }
    h->raiseEvent(BotEvent::SoccerGoal, team, points);
    h->logEvent("Team #{} made a goal for {} points", team, points);
}


export void __stdcall handleBallPosition(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Powerball ident
    //      2      2      X pixels
    //      4      2      Y pixels
    //      6      2      X velocity
    //      8      2      Y velocity
    //     10      2      Owner ident
    //     12      4      Timestamp
    if (len != 16) {
        handleUnknown(m);
        return;
    }
    uint8_t ball = getByte(msg, 1);
    uint16_t x = getShort(msg, 2);
    uint16_t y = getShort(msg, 4);
    int16_t xvel = getShort(msg, 6);
    int16_t yvel = getShort(msg, 8);
    uint16_t carrier = getShort(msg, 10);
    Player& p{ h->getPlayer(carrier) };

    uint32_t hosttime = getLong(msg, 12);
    uint32_t timestamp = h->getLocalTime(hosttime);

    PowerBall& pb = h->findBall(ball);

    if (hosttime == 0) {    // Carried
        if (p.isAssigned()) {    // Someone is carrying it
//            h->logEvent("Ball #{} @ ({}->{}, {}->{}) carried by {}", ball, x, xvel, y, yvel, p.name);

            pb.carrier = p.ident;
            pb.team = p.team;
        }
        else
        {
            pb.carrier = UnassignedId;
            pb.team = UnassignedId;
        }
    }
    else if (pb.hosttime != hosttime) {    // Uncarried
//        h->logEvent("Ball #{} @ ({}->{}, {}->{})", ball, x, xvel, y, yvel);

        pb.carrier = UnassignedId;
        pb.team = UnassignedId;
    }

    pb.x = x;
    pb.y = y;

    if (hosttime == 0) {
        pb.vx = 0;
        pb.vy = 0;
    }
    else {
        pb.vx = xvel;
        pb.vy = yvel;
    }
    pb.time = timestamp;
    pb.hosttime = hosttime;
    pb.lastrecv = getTickCount();

    h->raiseEvent(BotEvent::BallMove, pb);
}


//////// Login ////////

export void __stdcall handleMapInfo(HostMessage& m)
{
    DEFLATE_CLASS

    // This massive task takes ~200ms on my P200 laptop
    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1     16      Playfield name
    //     17      4      Playfield checksum
    // The map size is optional.  It's sent in subgame if client version is not 134,
    // and ASSS may potentially send it anyway.
    //     21      4      Playfield cmplen
    // The following are optionally repeated until the end of the message
    //     25     16      LVZ name
    //     41      4      LVZ checksum
    //     45      4      LVZ cmplen
    if (len != 21) {  // not SubSpace
        if ((len < 25) || ((len - 1) % 24)) {  // not Continuum either
            handleUnknown(m);

            return;
        }
        else {
            for (int32_t i = 25; i < len; i += 24) {
                char* filename = (char*)&msg[i];
                uint32_t checksum = getLong(msg, i + 16);
                uint32_t cmplen = getLong(msg, i + 20);

                //                h->logEvent("LVZ: {} [chk:{} cmplen:{}]", filename, checksum, cmplen);
            }
        }
    }
    char* filename = (char*)&msg[1];
    uint32_t checksum = getLong(msg, 17);
    std::string name = "lvl/" + std::string(filename);

    if (checksum != getFileChecksum(name, h->dictionary)) {
        // We cannot calculate the checksum without Ctm encryption, so we always assume we do not have 
        // the important stuff.

        if (len >= 25)
            h->logEvent("Downloading map file: {} [chk:{} cmplen:{}]", name, checksum, getLong(msg, 21));
        else
            h->logEvent("Downloading map file: {}", name);

        h->postRR(generateLevelRequest());
    }
    else {    // This block takes ~100ms on my desktop computer
        h->logEvent("Reading map file: {}", name);

        std::ifstream file(name, std::ios::binary);
        if (!file)
        {
            h->logEvent("Unable to load map file.");
            return;
        }

        file.seekg(0, std::ios::end);
        uint32_t length = (uint32_t)file.tellg();
        file.seekg(0, std::ios::beg);
        char* buffer = new char[length];
        file.read(buffer, length);

        convertFileToMatrix(buffer, (Playfield)h->playfieldTiles, length);

        h->changeCoordinates();

        delete[]buffer;

        h->gotMap = true;

        if (h->gotTurf) {
            if (!h->loadedFlags)
            {
                h->loadTurfFlags();
                h->logEvent("Loaded turf flags from map data.");

                h->loadedFlags = true;
            }
        }
        h->raiseEvent(BotEvent::MapLoaded);
    }
}


export void __stdcall handleMapFile(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1     16      Playfield name
    //     17    ...      Compressed map
    if (len <= 17) {
        handleUnknown(m);
        return;
    }
    char* name = (char*)&msg[1];
    char* compressed = (char*)&msg[17];
    size_t length = len - 17;

    if (name[15]) {
        h->logWarning("Unterminated downloaded-file name ignored.");
        return;
    }

    for (uint16_t i = 0; i < 16; ++i) {
        char c = name[i];

        if (c == 0)
            break;

        switch (c) {
        case '/':
        case '\\':
            h->logWarning("Invalid downloaded-file name path ignored. ({})", name);
            return;
        default:
            break;
        };

        if ((c < ' ') || (c > '~')) {
            h->logWarning("Invalid downloaded-file name chars ignored.");
            return;
        }
    }

    std::string fileName = "lvl/" + std::string(name);

    if (decompress_to_file(fileName.c_str(), compressed, length)) {
        h->logEvent("Map download complete. Reading: {}", fileName);

        std::ifstream file(fileName, std::ios::binary);
        if (!file) {
            h->logError("Unable to read map file.");
            return;
        }
        file.seekg(0, std::ios::end);
        length = (uint32_t)file.tellg();
        file.seekg(0, std::ios::beg);
        char* buffer = new char[length];
        file.read(buffer, length);

        convertFileToMatrix(buffer, (Playfield)h->playfieldTiles, length);
        h->changeCoordinates();
        delete[]buffer;
        h->gotMap = true;
        h->raiseEvent(BotEvent::MapLoaded);
    }
    else
        h->logError("Unable to decompress map file!");
}


export void __stdcall handleCustomMessage(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1  ...\0      Message
    if (len <= 1) {
        handleUnknown(m);
        return;
    }
    char* text = (char*)&msg[1];

    if ((len < 2) || text[len - 2]) {
        h->logWarning("Unterminated custom message ignored.");
        h->logIncoming(msg, len);

        return;
    }
    h->logEvent("Custom response: {}", text);
}


export void __stdcall handleVersionCheck(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      2      Version number
    //      3      4      Checksum
    if (len != 7) {
        handleUnknown(m);
        return;
    }
    uint16_t version = getShort(msg, 1);
    uint32_t checksum = getLong(msg, 3);

    h->logEvent("Remote Ctm version {} [chk:{}]", version, checksum);
}


export void __stdcall handleObjectToggle(HostMessage& m)
{
    DEFLATE_CLASS

    // Contributed by SOS
    // Ctm .37 handled this wrong, only one object could be toggled at a time
    // Field  Length      Description
    //      0      1      Type uint8_t
    // The following are repeated until the end of the message
    //      1      2      Object Info
    if ((len & 1) != 1) {
        handleUnknown(m);
        return;
    }
    LvzObjectInfo obj;
    int32_t index = 1;

    while (index < len) {
        obj.value = getShort(msg, index);

        h->raiseEvent(BotEvent::ObjectToggled, obj.value);

        if (obj.disabled) {
            h->logEvent("Object number {} disabled", (uint16_t)obj.id);
        }
        else {
            h->logEvent("Object number {} enabled", (uint16_t)obj.id);
        }
        index += 2;
    }
}


export void __stdcall handleArenaList(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    // The following are repeated until the end of the message
    //      1  ...\0      Arena name
    //      ?      2      Arena population
    if (len <= 3) {
        handleUnknown(m);
        return;
    }

    int32_t index = 1;

    while ((index + 3) <= len) {
        char* name = (char*)&msg[index];
        int32_t length = STRLEN(name);
        uint16_t population = getShort(msg, index + length + 1);
        bool current{};

        if (population > 0x7fff) {
            population = ~population + 1;
            h->logEvent("Arena: {} [{}] <-- I am here", name, population);
            current = true;
        }
        else {
            h->logEvent("Arena: {} [{}]", name, population);
        }

        if (index + length + 6 > len) {
            // send the last player name of the list
            h->raiseEvent(BotEvent::ArenaListEnd, name, current, population);
        }
        else {
            // send the current player name of the list
            h->raiseEvent(BotEvent::ArenaListEntry, name, current, population);
        }

        index += length + 3;
    }

    if (index > len) {
        h->logWarning("Malformed arena list ignored");
        h->logIncoming(msg, len);
    }
}


export void __stdcall handleArenaSettings(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0   1428      Settings buffer
    // The type uint8_t is included in checksums
    if (len != 1428) {
        handleUnknown(m);
        return;
    }

    h->logEvent("Re-reading arena settings...");

    //// Watch changes
    //if (h->settings.Version == 15)
    //{
    //    char *settings = (char*)&h->settings;

    //    for (uint32_t i = 0; i < 1428; ++i)
    //    {
    //        if (settings[i] != msg[i])
    //        {
    //            std::string s;
    //            s += "Change at ";
    //            s += i;
    //            s += ".  Old:";
    //            s += settings[i];
    //            s += ".  New:";
    //            s += msg[i];
    //            h->logEvent("{}", s);
    //            h->sendPublic(ChatSoundCode::Inconceivable, s.msg);
    //        }
    //    }

    //    std::string s;
    //    s += "First four uint8_ts of settings: ";
    //    s += settings[0];
    //    s += ", ";
    //    s += settings[1];
    //    s += ", ";
    //    s += settings[2];
    //    s += ", ";
    //    s += settings[3];
    //    s += ".";
    //    h->sendPublic(ChatSoundCode::Inconceivable, s.msg);
    //}
    // Update m_host setting pool
    memcpy(&h->settings, &msg[0], 1428);

    // Update prize system parameters
//    h->ps.setSettings(&h->settings, h->playfieldTiles);

    // Update goal mode
    h->changeGoalMode();
    h->raiseEvent(BotEvent::ArenaSettings, h->settings);
    //    h->logEvent("...Finished reading settings.");
}


export void __stdcall handlePasswordResponse(HostMessage& m)
{
    DEFLATE_CLASS

    // Field  Length      Description
    //      0      1      Type uint8_t
    //      1      1      Accept response meaning
    //      2      4      Server version
    //      6      4      <unused>
    //     10      4      EXE checksum
    //     14      4      <unused>
    //     18      1      <unused>
    //     19      1      Boolean: Request registration form
    //     20      4      SSEXE cksum with seed of zero; if this and EXE checksum are -1, bestows supervisor privs to the client
    //     24      4      News checksum (0 = no news file)
    //     28      4      <unused>
    //     32      4      <unused>
    if (len != 36) {
        handleUnknown(m);
        return;
    }
    LoginParam meaning = (LoginParam)getByte(msg, 1);
    uint32_t version = getLong(msg, 2);
    uint32_t EXEChecksum = getLong(msg, 10);
    //    uint32_t LocalEXEChecksum = getFileChecksum("subspace.bin", h->dictionary);
    uint32_t LocalEXEChecksum = 0xF1429CE8;
    bool requestRegForm = (getByte(msg, 19) != 0);
    uint32_t newsChecksum = getLong(msg, 24);
    uint32_t unk6 = getLong(msg, 6);
    uint32_t unk14 = getLong(msg, 14);
    bool unk18 = (getByte(msg, 18) != 0);
    uint32_t SS_EXE_cksum_0 = getLong(msg, 20);
    uint32_t unk28 = getLong(msg, 28);
    uint32_t unk32 = getLong(msg, 32);
    BotInfo* botInfo = &h->botInfo;

    if (requestRegForm) {
        h->postRR(generateRegForm(botInfo->realName,
            botInfo->email,
            botInfo->city,
            botInfo->state,
            botInfo->sex,
            botInfo->age,
            botInfo->playAtHome,
            botInfo->playAtWork,
            botInfo->playAtSchool,
            botInfo->processor,
            botInfo->regName,
            botInfo->regOrg));

        h->logEvent("Sending registration form");
    }
    //    if (LocalEXEChecksum == -1)
    //        LocalEXEChecksum = 0xF1429CE8;

    if (SS_EXE_cksum_0 != -1 && SS_EXE_cksum_0 != generateEXEChecksum(0)) {
        h->logWarning("SS EXE checksum mismatch on login.  May require privileged access to remain connected.");
    }

    if (EXEChecksum == -1 && SS_EXE_cksum_0 == -1) {
        h->logEvent("EXE checksum and (random) server checksum were sent: I have VIP access in this zone!  ...$");
    }
    else if (EXEChecksum == 0) {
        h->logWarning("Problem found with server: Server doesn't have a copy of subspace.exe so it sent me a zero "
            "checksum.");
    }
    else if (EXEChecksum != LocalEXEChecksum) {
        h->logWarning("Possible virus: remote EXE checksum mismatch (local:{}) (remote:{})",
            LocalEXEChecksum, EXEChecksum);
        h->logWarning("Privileged access is required to remain connected to this zone.");
        //h->logEvent(" FIELDS: 6:{} 14:{} 18:{} 20:{} 28:{} 32:{}", unk6, unk14, unk18, unk20, unk28, unk32);
    }

    if ((!h->downloadingNews) && (newsChecksum != 0) && (newsChecksum != getFileChecksum(
        "get/news.txt", h->dictionary))) {    // News checksum will be invalid if EXE checksum is invalid.
        h->postRR(generateNewsRequest());

        h->logEvent("Downloading latest news.txt [{}]", newsChecksum);

        h->downloadingNews = true;
    }

    --h->numTries;

    switch (meaning) {
    case LoginParam::Continue:
        h->logEvent("Password accepted for {}.", h->botInfo.name);
        h->activateGameProtocol();
        h->changeArena(botInfo->initialArena);
        h->logLogin();
        break;
    case LoginParam::NewUser:
        h->logEvent("Unknown player, continue as new user?");

        if (h->numTries <= 0) {
            h->logEvent("Number of login tries[of {}] exhausted for {}. Bye!", getBotDatabase().maxTries, 
                h->botInfo.name);
            h->disconnect(true);
            break;
        }
        else {
            h->logEvent("Creating account for {}", h->botInfo.name);
        }

        if (getBotDatabase().forceContinuum) {
            h->postRR(generateCtmPassword(true, botInfo->name, botInfo->password, botInfo->machineID, 
                botInfo->timeZoneBias, botInfo->permissionID, ContinuumVersion, ConnectMode::UnknownNotRAS));
        }
        else {
            h->postRR(generatePassword(true, botInfo->name, botInfo->password, botInfo->machineID, 
                botInfo->timeZoneBias, botInfo->permissionID, SubspaceVersion, ConnectMode::UnknownNotRAS));
        }
        break;
    case LoginParam::InvalidPassword:
        h->logEvent("Invalid password for specified user.  The name you have chosen is probably in use by another "
            "player, try picking a different name.");
        h->disconnect(true);
        break;
    case LoginParam::FullArena:
        h->logEvent("This arena is currently full, try again later.");

        if (h->numTries <= 0)
        {
            h->logEvent("Number of login tries[of {}] exhausted for {}. Bye!", getBotDatabase().maxTries, 
                h->botInfo.name);
            h->disconnect(true);
            break;
        }
        else
            h->logEvent("Sending password for {}", h->botInfo.name);

        if (getBotDatabase().forceContinuum) {
            h->postRR(generateCtmPassword(false, botInfo->name, botInfo->password, botInfo->machineID, 
                botInfo->timeZoneBias, botInfo->permissionID, ContinuumVersion, ConnectMode::UnknownNotRAS));
        }
        else {
            h->postRR(generatePassword(false, botInfo->name, botInfo->password, botInfo->machineID, 
                botInfo->timeZoneBias, botInfo->permissionID, SubspaceVersion, ConnectMode::UnknownNotRAS));
        }
        break;
    case LoginParam::LockedOut:
        h->logEvent("You have been locked out of SubSpace, for more information inquire on Web BBS.");
        h->disconnect(true);
        break;
    case LoginParam::NoPermission:
        h->logEvent("You do not have permission to play in this arena(1), see Web Site for more information.");
        h->disconnect(true);
        break;
    case LoginParam::SpectateOnly:
        h->logEvent("You only have permission to spectate in this arena.");
        h->activateGameProtocol();
        h->changeArena(botInfo->initialArena);

        h->logLogin();
        break;
    case LoginParam::TooManyPoints:
        h->logEvent("You have too many points to play in this arena, please choose another arena.");
        h->disconnect(true);
        break;
    case LoginParam::SlowConnection:
        h->logEvent("Your connection appears to be too slow to play in this arena.");
        h->disconnect(true);
        break;
    case LoginParam::NoPermission2:
        h->logEvent("You do not have permission to play in this arena(2), see Web Site for more information.");
        h->disconnect(true);
        break;
    case LoginParam::NoNewConnections:
        h->logEvent("The server is currently not accepting new connections.");

        if (h->numTries <= 0) {
            h->logEvent("Number of login tries[of {}] exhausted for {}. Bye!", getBotDatabase().maxTries, 
                h->botInfo.name);
            h->disconnect(true);
            break;
        }
        else
            h->logEvent("Sending password for {}", h->botInfo.name);

        if (getBotDatabase().forceContinuum) {
            h->postRR(generateCtmPassword(false, botInfo->name, botInfo->password, 
                botInfo->machineID, botInfo->timeZoneBias, botInfo->permissionID, 
                ContinuumVersion, ConnectMode::UnknownNotRAS));
        }
        else {
            h->postRR(generatePassword(false, botInfo->name, botInfo->password, 
                botInfo->machineID, botInfo->timeZoneBias, botInfo->permissionID, 
                SubspaceVersion, ConnectMode::UnknownNotRAS));
        }
        break;
    case LoginParam::InvalidName:
        h->logEvent("Invalid user name entered, please pick a different name.");
        h->disconnect(true);
        break;
    case LoginParam::ObsceneName:
        h->logEvent("Possibly offensive user name entered, please pick a different name.");
        h->disconnect(true);
        break;
    case LoginParam::BillerDown:
        h->logEvent("NOTICE: Server difficulties; this zone is currently not keeping track of scores.  Your "
            "original score will be available later.  However, you are free to play in the zone until we resolve "
            "this problem.");
        h->logWarning("Disabling operator accounts without passwords until biller is restored.");
        h->billerOnline = false;
        h->activateGameProtocol();
        h->changeArena(botInfo->initialArena);
        h->logLogin();
        break;
    case LoginParam::BusyProcessing:
        h->logEvent("The server is currently busy processing other login requests, please try again in a few "
            "moments.");

        if (h->numTries <= 0) {
            h->logEvent("Number of login tries[of {}] exhausted for {}. Bye!", getBotDatabase().maxTries, 
                h->botInfo.name);
            h->disconnect(true);
            break;
        }
        else {
            h->logEvent("Sending password for {}", h->botInfo.name);
        }

        if (getBotDatabase().forceContinuum) {
            h->postRR(generateCtmPassword(false, botInfo->name, botInfo->password, 
                botInfo->machineID, botInfo->timeZoneBias, botInfo->permissionID, 
                ContinuumVersion, ConnectMode::UnknownNotRAS));
        }
        else {
            h->postRR(generatePassword(false, botInfo->name, botInfo->password, 
                botInfo->machineID, botInfo->timeZoneBias, botInfo->permissionID, 
                SubspaceVersion, ConnectMode::UnknownNotRAS));
        }
        break;
    case LoginParam::ExperiencedOnly:
        h->logEvent("This zone is restricted to experienced players only (ie. certain number of game-hours logged).");
        h->disconnect(true);
        break;
    case LoginParam::UsingDemoVersion:
        h->logEvent("You are currently using the demo version.  Your name and score will not be kept track of.");
        h->disconnect(true);
        break;
    case LoginParam::TooManyDemos:
        h->logEvent("This arena is currently has(sic) the maximum Demo players allowed, try again later.");
        h->disconnect(true);
        break;
    case LoginParam::ClosedToDemos:
        h->logEvent("This arena is closed to Demo players.");
        h->disconnect(true);
        break;
    default:
        h->logEvent("Unknown response type, please go to Web site for more information and to obtain latest version "
            "of the program.");
        h->disconnect(true);
    };
}
