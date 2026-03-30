export module Player;

export import Algorithms;
export import Calc;
export import ClientTypes;
export import Command;
export import Playfield;

import <iostream>;
import <list>;
import <map>;
import <ranges>;
import <string>;
import <vector>;

export typedef uint16_t PlayerId;
export typedef int32_t Velocity;
export typedef uint8_t* Banner;

export const PlayerId UnassignedId{ 0xFFFF };
export const uint16_t MaxTeams{ 9999 };
export const uint16_t MaxUsers{ 65535 };


export std::string getPrizeDescription(int16_t prize)
{
    static std::map<int16_t, std::string> prizeDescription{
        { -28, "Lost portal" },
        { -27, "Lost rocket" },
        { -26, "Lost brick" },
        { -25, "UNKNOWN-25" },
        { -24, "Lost thor" },
        { -23, "Lost decoy" },
        { -22, "Lost burst" },
        { -21, "Lost repel" },
        { -20, "Lost antiwarp" },
        { -19, "Lost shrapnel" },
        { -18, "Lost shields!" },
        { -17, "Lost super!" },
        { -16, "Lost proximity" },
        { -15, "Lost multifire" },
        { -14, "Engines shutdown (severe)" },
        { -13, "Energy depleted" },
        { -12, "Top speed downgrade" },
        { -11, "Thrusters downgrade" },
        { -10, "Lost Bouncing bullets" },
        { -9, "Bomb downgrade" },
        { -8, "Gun downgrade" },
        { -7, "Warp!" },
        { -6, "XRadar downgrade" },
        { -5, "Cloak downgrade" },
        { -4, "Stealth downgrade" },
        { -3, "Rotation downgrade" },
        { -2, "Energy downgrade" },
        { -1, "Recharge downgrade" },
        { 0, "Ship reset" },
        { 1, "Recharge upgrade" },
        { 2, "Energy upgrade" },
        { 3, "Rotation" },
        { 4, "Stealth" },
        { 5, "Cloak" },
        { 6, "XRadar" },
        { 7, "Warp" },
        { 8, "Gun upgrade" },
        { 9, "Bomb upgrade" },
        { 10, "Bouncing bullets" },
        { 11, "Thrusters upgrade" },
        { 12, "Top speed upgrade" },
        { 13, "Full charge" },
        { 14, "Engines shutdown" },
        { 15, "Multifire" },
        { 16, "Proximity bombs" },
        { 17, "Super!" },
        { 18, "Shields!" },
        { 19, "Shrapnel increase" },
        { 20, "Antiwarp upgrade" },
        { 21, "Repel" },
        { 22, "Burst" },
        { 23, "Decoy" },
        { 24, "Thor!" },
        { 25, "Multiprize!" },
        { 26, "Brick" },
        { 27, "Rocket" },
        { 28, "Portal" }
    };

    if (prizeDescription.contains(prize))
        return prizeDescription[prize];
    else
        return "UNKNOWN";
}


export struct Score
{
    uint32_t killPoints, flagPoints;
    uint16_t wins, losses;
};


export struct Player
{
    PlayerId ident{ UnassignedId };
    std::string name;
    std::string squad;
    uint16_t team = UnassignedId;
    Ship ship = Ship::Spectator;
    Score score{};
    bool acceptsAudio{};
    uint16_t flagCount = 0;
    
    OperatorLevel access{ OperatorLevel::Player };
    CommandScope scope{ CommandScope::Local };
    PlayerId turretId = UnassignedId;
    uint64_t lastPositionUpdate = 0;
    uint8_t banner[96]{};
    uint8_t d{};        // direction
    uint16_t bounty{};
    uint16_t energy{};
    uint16_t timer{};
    uint16_t S2CLag{};
    std::vector<Coord> tile, pos, work, vel;
    bool koth = false;  // king of the hill
    bool stealth{}, cloak{}, xradar{}, awarp{}, ufo{}, flash{}, safety{}, shields{}, supers{};
    uint8_t burst{}, repel{}, thor{}, brick{}, decoy{}, rocket{}, portal{};

    // dummy constructor, needed for the player map and e.g. also to handle of arena chat messages from server to bot
    Player() = default;

    // dummy constructor for remote commands to a bot dll
    Player(std::string_view nname, OperatorLevel aaccess, CommandScope sscope)
        : name(nname), access(aaccess), scope(sscope) {}

    Player(uint16_t iident, std::string_view nname, std::string_view ssquad, uint32_t fflagPoints,
        uint32_t kkillPoints, uint16_t tteam, uint16_t wwins, uint16_t llosses, Ship sship, bool aacceptsAudio, 
        uint16_t fflagCount)
        : ident(iident), name(nname), squad(ssquad), team(tteam), ship(sship)
    {
        access = OperatorLevel::Player;
        score.flagPoints = fflagPoints;
        score.killPoints = kkillPoints;
        score.wins = wwins;
        score.losses = llosses;
        acceptsAudio = aacceptsAudio;
        flagCount = fflagCount;
        memset(banner, 0, 96);
        //lastPositionUpdate = getTime();

        tile = pos = work = vel = { 0, 0 };
        move(TileMaxX * 8, TileMaxY * 8, 0, 0);
    }

    std::string_view getName() const
    {
        return name;
    }

    void setBanner(uint8_t* bbanner)
    {
        memcpy(banner, bbanner, 96);
    }

    void clone(const Player& player)
    {
        tile = player.tile;
        pos = player.pos;
        work = player.work;
        vel = player.vel;
        d = player.d;
    }

    void move(Coord x, Coord y, Velocity vx, Velocity vy)
    {
        work = { x * 1000, y * 1000 };
        vel = { vx, vy };
        pos = scalarDiv(work, 1000);
        tile = scalarDiv(pos, 16);
    }

    void move(Coord x, Coord y)
    {
        work = { x * 1000, y * 1000 };
        vel = { 0, 0 };
        pos = scalarDiv(work, 1000);
        tile = scalarDiv(pos, 16);
    }

    void move(int32_t time)
    {
        work = vectorAdd(work, scalarMult(vel, time));
        pos = scalarDiv(work, 1000);
        tile = scalarDiv(pos, 16);
    }

    bool isAssigned() const
    {
        return ident != UnassignedId;
    }

    /// <summary>
    /// Check if this player is eligible to use a command.
    /// </summary>
    /// <param name="cmdInfo">Command info.</param>
    /// <param name="errMsg">Returns an error message.</param>
    /// <returns>True, if the command has the required access level and scope. Otherwise false is
    /// returned with an error message.</returns>
    bool checkCommandAccess(const CommandInfo& cmdInfo, std::string& errMsg) const
    {
        errMsg = "";

        if (cmdInfo.minLevel > access) {
            errMsg = "You are not permitted to use this command.";
        }
        else if (cmdInfo.maxScope < scope) {
            if (scope == CommandScope::Remote)
                errMsg = "You are not allowed to use this command from a remote arena.";
            else
                errMsg = "You are not allowed to use this command from an external chat client.";
        }

        return errMsg.empty();
    }
};


export struct Brick
{
    uint16_t x, y;
    uint32_t time;
    uint16_t team;
    uint16_t ident;

    bool operator == (const Brick&) const& = default;
};


export struct Flag
{
    uint16_t x, y;
    uint16_t team;
    uint16_t ident;

    bool operator == (const Flag& f) const
    {
        return x == f.x && y == f.y && team == f.team && ident == f.ident;
    }
};


export struct Goal
{
    uint16_t x, y;
    uint16_t team;

    bool operator == (const Goal& g) const
    {
        return x == g.x && y == g.y && team == g.team;
    }
};


export struct PowerBall
{
    uint16_t x, y;
    int16_t vx, vy;
    uint16_t team;
    uint16_t ident;
    uint16_t carrier;
    uint64_t time;            // Local timestamp on event (not constant)
    uint32_t hosttime;        // Remote timestamp on event (constant until new event)
    uint64_t lastrecv;        // Last update recv time (for timeout)

    bool operator == (const PowerBall&) const& = default;
};

export typedef std::map<PlayerId, Player> PlayerMap;
export typedef std::list<Flag> FlagList;
export typedef std::list<Brick> BrickList;
