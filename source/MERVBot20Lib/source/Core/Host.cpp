module;
#include <stdarg.h>
#include <filesystem>;
module Host;

import Algorithms;
import BotDb;
import Checksum;
import ClientProt;
import SpecialProt;
import System;

import <fstream>;
import <iostream>;
import <ranges>;


//////// Core options ////////

constexpr uint32_t PacketSilenceLimit{ 4000 };
constexpr uint32_t MaxReliableInTransit{ 15 };
constexpr uint32_t ChunkSize{ 496 };
constexpr uint64_t SlowIteration{ 50 };
constexpr uint64_t LogInterval{ 1000 };
constexpr uint64_t SyncInterval{ 2000 };


//////// Host messaging ////////

HostMessage::HostMessage(uint8_t* mmsg, size_t len, Host* ssrc)
{
    msg.resize(len);
    memcpy(&this->msg[0], mmsg, len);
    src = ssrc;
}


uint8_t HostMessage::getType(bool special)
{
    if (special)
        return unsigned(msg[1]);
    else
        return unsigned(msg[0]);
}


//////// Client message ////////

ClientMessage::ClientMessage(size_t len)
{
    msg.resize(len);
    size_t tt = msg.size();
    clear();
}


void ClientMessage::clear()
{
    fill(msg.begin(), msg.end(), 0);
}


//////// Cluster message ////////

ClusterMessage::ClusterMessage(uint8_t* mmsg, size_t llen)
{
    if (llen > PacketMaxLength) {
        std::cout << "ClusterMessage buffer overrun averted at the very last minute\n";
        return;
    }

    msg.resize(llen);
    memcpy(&this->msg[0], mmsg, llen);
}


//////// Reliable message ////////

ReliableMessage::ReliableMessage(uint32_t ack_ID, uint8_t* mmsg, size_t llen)
{
    if (llen > PacketMaxLength) {
        std::cout << "ReliableMessage buffer overrun averted at the very last minute\n";
        return;
    }

    msg.resize(llen);
    memcpy(&this->msg[0], mmsg, llen);
    ACK_ID = ack_ID;
    setTime();
}


void ReliableMessage::setTime()
{
    lastSend = getTickCount();
}


//////// Logged chat message ////////

LoggedChat::LoggedChat(ChatMode mmode, ChatSoundCode ssnd, uint16_t iident, std::string_view mmsg)
    : msg(mmsg)
{
    mode = mmode;
    snd = ssnd;
    ident = iident;
}


//////// Host messaging ////////

void Host::send(uint8_t* msg, size_t len)
{
    uint8_t packet[PacketMaxLength];

#ifdef C2S_LOG
    logOutgoing(msg, len);
#endif
    memcpy(packet, msg, len);
    encryption.encrypt(packet, len);
    socket.send(packet, len);
    ++msgSent;
}


//////// Clustering ////////

void Host::beginCluster()
{
#ifdef CLUSTER_MODE
    clustering = true;
#endif
}


void Host::post(ClientMessage&& cm, bool reliable)
{
    post(&cm.msg[0], cm.msg.size(), reliable);
}


void Host::postRR(ClientMessage&& cm)
{
    post(&cm.msg[0], cm.msg.size(), true);
}


void Host::postRU(ClientMessage&& cm)
{
    post(&cm.msg[0], cm.msg.size(), false);
}


void Host::post(uint8_t* msg, size_t len, bool reliable)
{
    uint8_t buffer[PacketMaxLength]{};

    if (len > ChunkSize + 12) {
        // Chunk it
        if (len > 1000) {
            for (uint32_t i = 0; i < len; i += ChunkSize) {
                buffer[0] = 0x00;
                buffer[1] = 0x0A;
                *(uint32_t*)&buffer[2] = (uint32_t)len;

                size_t remaining = len - i;

                if (remaining > ChunkSize) {
                    // Chunk body
                    memcpy(buffer + 6, msg + i, ChunkSize);

                    post(buffer, ChunkSize + 6, true);
                }
                else {    // Chunk tail
                    memcpy(buffer + 6, msg + i, remaining);

                    post(buffer, remaining + 6, true);
                }
            }
        }
        else {
            buffer[0] = 0x00;
            buffer[1] = 0x08;

            for (uint32_t i = 0; i < len; i += (ChunkSize + 4)) {
                size_t remaining = (len - i);

                if (remaining > (ChunkSize + 4)) {    // Chunk body
                    memcpy(buffer + 2, msg + i, ChunkSize + 4);
                    post(buffer, ChunkSize + 6, true);
                }
                else {
                    // Chunk tail
                    buffer[1] = 0x09;
                    memcpy(buffer + 2, msg + i, remaining);
                    post(buffer, remaining + 2, true);

                    break;
                }
            }
        }
    }
    else {
        // We're dealing with byte-sized messages now
        if (reliable) {
            // Stamp reliable header
            buffer[0] = 0x00;
            buffer[1] = 0x03;
            *(uint32_t*)&buffer[2] = localStep;
            memcpy(buffer + 6, msg, len);
            msg = buffer;
            len += 6;

            if (queue(msg, len) == false)
                return;
        }

        if (clustering) {
            // Append to cluster list
            clustered.push_back(ClusterMessage(msg, len));
        }
        else {
            // Send right away
            send(msg, len);
        }
    }
}


void Host::sendClustered()
{
    uint8_t buffer[PacketMaxLength]{};

    buffer[0] = 0x00;
    buffer[1] = 0x0E;
    size_t len = 2;        // Total length of the cluster message
    uint32_t tctr = 0;    // Total messages clustered
    
    for (ClusterMessage& m : clustered) {
        if (m.msg.size() > 255) {
            // This message will not fit on a cluster
            send(&m.msg[0], m.msg.size());
        }
        else {
            // This message will fit on a cluster
            if (len + m.msg.size() > ChunkSize + 12) {
                // But, this cluster message is full
                if (tctr == 1) {
                    send(buffer + 3, len - 3);
                }
                else {
                    send(buffer, len);
                }

                len = 2;
                tctr = 0;
            }
    
            // Append the next listed message
            buffer[len++] = (char)m.msg.size();
            memcpy(buffer + len, &m.msg[0], m.msg.size());
            len += m.msg.size();
            ++tctr;
        }
    }
    
    // Send the end of the cluster
    switch (tctr)
    {
    case 0:
        break;
    case 1:
        send(buffer + 3, len - 3);
        break;
    default:
        send(buffer, len);
    };
}

void Host::endCluster()
{
    clustering = false;
    sendClustered();
    clustered.clear();
}


//////// Reliable messaging ////////

bool Host::queue(uint8_t* msg, size_t len)
{    // Returns false if the message shouldn't be sent immediately
    ++localStep;    // Increment next ACK_ID
    
    if ((uint32_t)sent.size() < MaxReliableInTransit) {
        // There is a free send slot
        sent.push_back(ReliableMessage(*(uint32_t*)&msg[2], msg, len));
        ++localSent;    // Increment most recently sent ACK_ID
        return true;
    }
    else {    // It must be queued for delivery
        queued.push_back(ReliableMessage(*(uint32_t*)&msg[2], msg, len));
        return false;
    }
}


void Host::checkSent(uint32_t ACK_ID)
{
    // Ignore if we haven't sent it yet
    if (ACK_ID > localSent) {
        return;
    }
    
    // Stop sending if an ACK was recv'd
    std::list<ReliableMessage>::iterator m{ sent.begin() };

    while (m != sent.end()) {
        if (m->ACK_ID == ACK_ID) {
            sent.erase(m);
            sendNext();

            return;
        }
        m++;
    }
}


void Host::sendNext()
{
    // Queue up another packet for sending
    std::list<ReliableMessage>::iterator m{ queued.begin() };

    while (m != queued.end()) {
        if (m->ACK_ID == localSent) {
            // Send it
            post(&m->msg[0], m->msg.size(), false);
            m->setTime();

            // Move the message
            sent.push_back(*m);
            ++localSent;    // Increment most recently sent ACK_ID
            queued.erase(m);

            return;
        }
        m++;
    }
}


void Host::checkReceived(uint32_t ACK_ID, uint8_t* msg, size_t len)
{
    // Ignore if we've already processed it
    if (ACK_ID < remoteStep) {
        return;
    }
    
    // Poke around and find a match
    if (ACK_ID == remoteStep) {
        gotMessage(msg, len);
        ++remoteStep;

        // Process next in sequence
        std::list<ReliableMessage>::iterator recv{ received.begin() };

        while (recv != received.end()) {
            if (recv->ACK_ID == remoteStep) {
                gotMessage(&recv->msg[0], recv->msg.size());
                ++remoteStep;
                received.erase(recv);
                recv = received.begin();
            }
            else {
                recv++;
            }
        }
    }
    else {
        // Do not add if it already exists
        for (ReliableMessage& recv : received) {
            if (recv.ACK_ID == ACK_ID) {
                return;
            }
        }
        // Add it
        received.push_back(ReliableMessage(ACK_ID, msg, len));
    }
}


void Host::sendACK(uint32_t ACK_ID)
{
    uint8_t msg[6]{};
    msg[0] = 0x00;
    msg[1] = 0x04;
    *(uint32_t*)&msg[2] = ACK_ID;

    post(msg, 6, false);
}


void Host::sendQueued()
{
    uint64_t time = getTickCount();
    
    for(ReliableMessage& rlmMsg : sent) {
        if (time - rlmMsg.lastSend > syncPing + 10) {
            post(&rlmMsg.msg[0], rlmMsg.msg.size(), false);
            rlmMsg.setTime();
        }
    }
}


//////// Chatter ////////

/// <summary>
/// Post a chat message for a specified mode.
/// </summary>
/// <param name="mode">Chat mode.</param>
/// <param name="snd">Sound code.</param>
/// <param name="playerId">Player Id.</param>
/// <param name="msg">Message.</param>
void Host::postChat(ChatMode mode, ChatSoundCode snd, PlayerId playerId, std::string_view msg)
{
    postRR(generateChat(mode, snd, playerId, msg));
}


/// <summary>
/// Split a message with endline as delimiter and post the partial strings as chat messages for
/// a specified mode. SysOp messages are sent directly, for all other senders the messages are 
/// buffered and sent maximum 2 per event loop.
/// </summary>
/// <param name="mode">Chat mode.</param>
/// <param name="snd">Sound code.</param>
/// <param name="playerId">Player Id.</param>
/// <param name="msg">Message.</param>
void Host::tryChat(ChatMode mode, ChatSoundCode snd, PlayerId playerId, std::string_view msg)
{
    std::string splitMsg{ msg };

    for (std::string& s : split(splitMsg, '\n')) {
        if (hasSysOp || s.starts_with('*'))
            postChat(mode, snd, playerId, s);
        else
            addChatLog(mode, snd, playerId, s);
    }
}


void Host::sendPrivate(const Player& player, ChatSoundCode snd, std::string_view msg)
{
    if (player.ident != UnassignedId) {
        tryChat(ChatMode::Private, snd, player.ident, msg);
    }
    else {
        // it's a Player object of a remote player
        sendRemotePrivate(player.name, msg);
    }
}


void Host::sendPrivate(const Player& player, std::string_view msg)
{
    sendPrivate(player, ChatSoundCode::None, msg);
}


void Host::sendTeam(std::string_view msg)
{
    tryChat(ChatMode::Team, ChatSoundCode::None, 0, msg);
}


void Host::sendTeam(ChatSoundCode snd, std::string_view msg)
{
    tryChat(ChatMode::Team, snd, 0, msg);
}


void Host::sendTeamPrivate(uint16_t team, std::string_view msg)
{
    for (Player& p : players | std::views::values) {
        if (p.team == team) {
            tryChat(ChatMode::TeamPrivate, ChatSoundCode::None, p.ident, msg);
            return;
        }
    }
}


void Host::sendTeamPrivate(uint16_t team, ChatSoundCode snd, std::string_view msg)
{
    for (Player& p : players | std::views::values) {
        if (p.team == team) {
            tryChat(ChatMode::TeamPrivate, snd, p.ident, msg);
            return;
        }
    }
}


void Host::sendPublic(std::string_view msg)
{
    tryChat(ChatMode::Public, ChatSoundCode::None, 0, msg);
}


void Host::sendPublic(ChatSoundCode snd, std::string_view msg)
{
    tryChat(ChatMode::Public, snd, 0, msg);
}


void Host::sendPublicMacro(std::string_view msg)
{
    tryChat(ChatMode::PublicMacro, ChatSoundCode::None, 0, msg);
}


void Host::sendPublicMacro(ChatSoundCode snd, std::string_view msg)
{
    tryChat(ChatMode::PublicMacro, snd, 0, msg);
}


void Host::sendChannel(std::string_view msg)
{
    tryChat(ChatMode::Channel, ChatSoundCode::None, 0, msg);
}


void Host::sendChannel(uint32_t channel, std::string_view msg)
{
    sendChannel(format("{};{}", channel, msg));
}


void Host::sendRemotePrivate(const std::string_view msg)
{
    tryChat(ChatMode::RemotePrivate, ChatSoundCode::None, 0, msg);
}


void Host::sendRemotePrivate(std::string_view name, std::string_view msg)
{
    sendRemotePrivate(format(":{}:{}", name, msg));
}


//////// Game protocol ////////

uint32_t Host::getHostTime()
{
    // truncate the 64 bit time, as 32 bit are required for the protocol
    return (uint32_t)(timeDiff + getTickCount());
}


uint32_t Host::getLocalTime(uint32_t time)
{
    // truncate the 64 bit time, as 32 bit are required for the protocol
    return (uint32_t)(time - timeDiff);
}


//////// Game objects ////////

// Bricks

bool Host::brickExists(uint16_t ident)
{
    for (Brick& brick : bricks) {
        if (brick.ident == ident) {
            return true;
        }
    }
    return false;
}


void Host::doBrickEvents()
{
    std::list<Brick>::iterator brickIt = bricks.begin();

    while (brickIt != bricks.end()) {
        if (getTickCount() >= brickIt->time){
            playfieldTiles[getLinear(brickIt->x, brickIt->y)] = PlayfieldTileFormat::VieNoTile;
            brickIt = bricks.erase(brickIt);
        }
        else {
            brickIt++;
        }
    }
}


void Host::updateBrickTiles()
{
    uint16_t team = me().team;
    
    for (Brick& b : bricks) {
        if (b.team == team)
            playfieldTiles[getLinear(b.x, b.y)] = PlayfieldTileFormat::SsbTeamBrick;
        else
            playfieldTiles[getLinear(b.x, b.y)] = PlayfieldTileFormat::SsbEnemyBrick;
    }
}


// Powerball

PowerBall& Host::findBall(uint16_t ident)
{
    for (PowerBall& ball : balls) {
        if (ball.ident == ident) {
            return ball;
        }
    }
    balls.push_back({});

    PowerBall& ball{ balls.back() };

    ball.x = 0;
    ball.y = 0;
    ball.vx = 0;
    ball.vx = 0;
    ball.carrier = UnassignedId;
    ball.ident = ident;
    ball.team = UnassignedId;
    ball.time = 0;
    ball.hosttime = 0;
    ball.lastrecv = 0;
    return ball;
}


void Host::doBallEvents()
{
    uint64_t time = getTickCount();
    std::list<PowerBall>::iterator balIt = balls.begin();

    while (balIt != balls.end()) {
        if (balIt->hosttime) {
            if (time - balIt->lastrecv > settings.S2CNoDataKickoutDelay) {
                //logEvent("Ball #{} timed out", pb->ident);
                balIt = balls.erase(balIt);
            }
        }
        balIt++;
    }
}


// Flags

Flag& Host::findFlag(uint16_t ident)
{
    for (Flag& f : flags) {
        if (f.ident == ident) {
            return f;
        }
    }
    flags.push_back({ 1, 1, UnassignedId, ident });
    return flags.back();
}


void Host::claimFlag(uint16_t flag, uint16_t playerId)
{
    Flag& f = findFlag(flag);
    Player& p{ getPlayer(playerId) };

    if (p.isAssigned()) {
        if (!settings.CarryFlags) {
            // Turf mode, don't delete flags
            // Now entering SOS-land.
            // Keep your hands within the handrails and wait for the snippet to stop. 
            raiseEvent(BotEvent::FlagGrab, p, f);
            f.team = p.team;
            // Thank you for visiting SOS-land
        }
        else {    // Warzone, delete flags when claimed
            ++p.flagCount;
            raiseEvent(BotEvent::FlagGrab, p, f);
            flags.remove(f);
        }
    }
}


void Host::dropFlags(uint16_t playerId)
{
    Player& p{ getPlayer(playerId) };

    if (p.isAssigned()) {
        raiseEvent(BotEvent::FlagDrop, p);
        p.flagCount = 0;
    }
}


void Host::resetFlagTiles()
{
    uint16_t team = me().team;

    for (Flag& f : flags) {
        if (f.team != team)
            playfieldTiles[getLinear(f.x, f.y)] = PlayfieldTileFormat::SsbEnemyFlag;
        else
            playfieldTiles[getLinear(f.x, f.y)] = PlayfieldTileFormat::SsbTeamFlag;
    }
}


// Goals

// These gave me a lot of trouble in the SSBot2 core.
// Here, you'll notice i've optimized everything for
// 1. speed and 2. readability.  The code is officially
// exportable, feel free to use it in your own projects
// just like any other snippet you find.

void Host::changeCoordinates()
{
    // Takes ~140ms on my P200 laptops
    // When we set the map

    for (uint32_t off = 0; off < TileMaxLinear; ++off) {
        PlayfieldTileFormat tile = playfieldTiles[off];
        
        if (tile == PlayfieldTileFormat::VieGoalArea) {
            goals.push_back({});

            Goal& goal{ goals.back() };

            goal.x = uint16_t(off & 1023);    // fun: applied strength reduction
            goal.y = uint16_t(off >> 10);
            goal.team = UnassignedId;
        }
    }
    changeGoalMode();    // Only call the top-level function for your event
}


void Host::changeGoalMode()
{
    // Takes <20ms on my P200 laptop
    // When we set the map
    // When we receive settings (won't fail the first time)
    uint8_t mode = settings.SoccerMode;

    for (Goal& goal : goals) {
        uint16_t x = goal.x;
        uint16_t y = goal.y;

        // Soccer:Mode
        switch (mode) {
        case 1:
            // Left/Right (own 1)
            if (x < 512) {
                goal.team = 1;    // L
            }
            else {
                goal.team = 0;    // R
            }
            break;
        case 2:
            // Top/Bottom (own 1)
            if (y < 512) {
                goal.team = 1;    // U
            }
            else {
                goal.team = 0;    // L
            }
            break;
        case 3:
            // Quadrants (own 1)
        case 4:
            // Inverse quadrants (own 3)
            if (y < 512) {
                if (x < 512) {
                    goal.team = 0;    // UL
                }
                else {
                    goal.team = 1;    // UR
                }
            }
            else {
                if (x < 512) {
                    goal.team = 2;    // LL
                }
                else {
                    goal.team = 3;    // LR
                }
            }
            break;
        case 5:        // Iso-quadrants (own 1)
        case 6:        // Inverse iso-quadrants (own 3)
            if (x >= y) {
                if (x > (1023 ^ y)) {
                    goal.team = 3;    // Right
                }
                else {
                    goal.team = 2;    // Top
                }
            }
            else {
                if (x > (1023 ^ y)) {
                    goal.team = 1;    // Bottom
                }
                else {
                    goal.team = 0;    // Left
                }
            }
            break;
        default:
            // Any (own none)
            goal.team = UnassignedId;
            break;
        };
    }
    changeGoalTiles();    // Only call the top-level function for your event
}


void Host::changeGoalTiles()
{
    // Takes <1ms on my P200 laptop
    // When we set the map
    // When we change teams
    // When we receive settings (won't fail the first time)#

    if (!hasBot() || me().team == UnassignedId) {
        return;    // I don't have a team yet
    }

    uint8_t mode = settings.SoccerMode;
    uint16_t team = me().team;

    for (Goal& goal : goals) {
        uint16_t x = goal.x;
        uint16_t y = goal.y;
        PlayfieldTileFormat owner;

        // Soccer:Mode
        switch (mode) {
        case 1:
            // Left/Right (own 1)
        case 2:
            // Top/Bottom (own 1)
            if ((team & 1) == goal.team) {
                owner = PlayfieldTileFormat::SsbTeamGoal;
            }
            else {
                owner = PlayfieldTileFormat::SsbEnemyGoal;
            }
            break;
        case 3:
            // Quadrants (own 1)
        case 5:
            // Iso-quadrants (own 1)
            if ((team & 3) == goal.team) {
                owner = PlayfieldTileFormat::SsbTeamGoal;
            }
            else {
                owner = PlayfieldTileFormat::SsbEnemyGoal;
            }
            break;
        case 4:
            // Inverse quadrants (own 3)
        case 6:
            // Inverse iso-quadrants (own 3)
            if ((team & 3) != goal.team) {
                owner = PlayfieldTileFormat::SsbTeamGoal;
            }
            else {
                owner = PlayfieldTileFormat::SsbEnemyGoal;
            }
            break;
        default:
            // Any (own none)
            owner = PlayfieldTileFormat::SsbEnemyGoal;
            break;
        };
        playfieldTiles[getLinear(x, y)] = owner;
    }
}


void Host::loadTurfFlags()
{
    uint16_t flag = 0;

    for (uint32_t off = 0; off < TileMaxLinear; ++off) {
        PlayfieldTileFormat tile = playfieldTiles[off];

        if (tile == PlayfieldTileFormat::VieTurfFlag) {
            Flag& f = findFlag(flag++);

            f.x = uint16_t(off & 1023);
            f.y = uint16_t(off >> 10);
            raiseEvent(BotEvent::FlagMove, f);
        }
    }
}


// Players

Player& Host::addPlayer(PlayerId playerId, std::string_view name, std::string_view squad, uint32_t flagPoints,
    uint32_t killPoints, uint16_t team, uint16_t wins, uint16_t losses, Ship ship, bool acceptsAudio, 
    uint16_t flagCount)
{
    players.emplace(playerId, Player(playerId, name, squad, flagPoints, killPoints, team, wins, losses, ship, 
        acceptsAudio, flagCount));
    playerIds.push_back(playerId);
    return players[playerId];
}


void Host::killTurret(Player& turretee)
{
    // the turreter is the one who sits on top, the turretee is the driver
    for (Player& turreter : players | std::views::values) {
        if (turreter.turretId == turretee.ident) {
            if (turreter.ident == playerId) {
                // the bot was the turreter, request a detach
                logEvent("detached!");
                postRR(generateAttachRequest(UnassignedId));
            }
            raiseEvent(BotEvent::DeleteTurret, turreter, turretee);
            turreter.turretId = UnassignedId;
            break;
        }
    }
}


void Host::killTurreter(Player& turreter)
{
    // the turreter is the one who sits on top, the turretee is the driver
    if (turreter.turretId != UnassignedId) {
        Player& turretee{ getPlayer(turreter.turretId) };

        raiseEvent(BotEvent::DeleteTurret, turreter, turretee);
        turreter.turretId = UnassignedId;
    }
}


Player& Host::getPlayer(PlayerId playerId)
{
    // If the player Id does not exist in the map, the default behaviour of the map is to create a dummy player for 
    // this Id. A call of isAssigned() for this dummy player will return false. Before calling talk(..) for a player, 
    // the validity should always be checked.
    return players[playerId];
}


Player& Host::findPlayer(const std::string_view name)
{
    for (Player& p : players | std::views::values) {
        if (toLower(p.name) == toLower(name)) {
            return p;
        }
    }
    // return the UnassignedId dummy player, which wll be created if it doesn't exist already
    return players[UnassignedId];
}


void Host::revokeAccess(std::string_view name)
{
    findPlayer(name).access = OperatorLevel::Player;
}


// Revoke access below a level
void Host::revokeAccess(OperatorLevel access)
{
    for (Player& player : players | std::views::values) {
        if (player.access <= access) {
            player.access = OperatorLevel::Player;
        }
    }
}

/* unused
Player *Host::findTeammate(Player *excluded, uint16_t team)
{    // Find any teammate of given team excluding one player (optional if NULL)
    _listnode <Player> *parse = playerlist.head;

    while (parse) {
        Player *p = parse->item;

        if ((excluded != p) && (team == p->team) && (p->ship != Ship::Spectator))
            return p;

        parse = parse->next;
    }
    return NULL;
}

Player *Host::findHighScorer(Player *excluded, uint16_t team)
{
    _listnode <Player> *parse = playerlist.head;
    Player *last = NULL;

    while (parse) {
        Player *p = parse->item;

        if ((excluded != p) && (team == p->team) && (p->ship != Ship::Spectator)) {
            if (!last 
                || (last->score.flagPoints + last->score.killPoints < p->score.flagPoints + p->score.killPoints)) {
                last = p;
            }
        }
        parse = parse->next;
    }
    return last;
}

uint16_t Host::teamPopulation(uint16_t team)
{
    _listnode <Player> *parse = playerlist.head;
    uint16_t pop = 0;

    while (parse) {
        Player *p = parse->item;
        parse = parse->next;

        if ((p->team == team) && (p->ship != Ship::Spectator))
            ++pop;
    }
    return pop;
}
unused */

// Called when a player is overwritten *and* on an arena-part
void Host::killPlayer(Player& player)
{
    // !follow
    if (follow != UnassignedId && (player.ident == follow)) {
        follow = UnassignedId;    // stop following
    }
    // !ownbot
    if (limitedOwnerId == player.ident) {
        limitedOwnerId = UnassignedId;
    }
    // Turrets
    killTurret(player);
    // remove the player from the map
    playerIds.remove(player.ident);
    players.erase(player.ident);
}


// Tile changes

void Host::resetIcons()
{    // When we change teams

    // Bricks. These are either friendly or enemy
    updateBrickTiles();

    // Flags. Player carried - ignore Dropped - assign team color
    resetFlagTiles();

    // Goals. Modal - goal mode creates team pattern Team owned - check ownership against mode
    changeGoalTiles();
}


//////// Logged chat messaging ////////

void Host::addChatLog(ChatMode mode, ChatSoundCode snd, PlayerId playerId, std::string_view msg)
{
    chatLog.push_back(LoggedChat(mode, snd, playerId, msg));
}


bool Host::sendNextLoggedChat()
{
    // return false if the chatLog is empty
    if (chatLog.size() > 0) {
        LoggedChat& lc{ chatLog.front() };

        postChat(lc.mode, lc.snd, lc.ident, lc.msg);
        chatLog.remove(lc);
        return true;
    }
    return false;
}


void Host::doChatLog()
{
    uint64_t time = getTickCount();

    if (time - lastChatSend > 150) {
        lastChatSend = time;
        countChatSend = 0;
    }

    while (countChatSend < 2) {
        if (!sendNextLoggedChat())
            break;
        ++countChatSend;
    }
}


//////// Core protocol ////////

void Host::resetSession()
{
    // Clear routers
    for (uint16_t i = 0; i < 256; ++i) {
        specialRouter.add((uint8_t)i, handleSpecialUnknown);
        generalRouter.add((uint8_t)i, handleUnknown);
    }

    // Routers
    generalRouter.add(0x00, handleSpecialHeader);
    specialRouter.add(0x02, handleEncryptResponse);
    specialRouter.add(0x05, handleSyncRequest);
    specialRouter.add(0x07, handleDisconnect);
    specialRouter.add(0x0E, handleCluster);
    
    // Encryption
    encryption.reset();

    // Transfer mode
    ftLength = 0;
    little.setLimit(1000000);    // 1MB - enough for 20 server lists.
    large.setLimit(5000000);    // 5MB - enough for your average EG map.
                                // Set higher if .LVL file (for example) exceeds 5 MB
    connecting = false;
    syncing = false;
    killMe = false;
    clustered.clear();
    large.deleteMessage();
    little.deleteMessage();

    // State
    timeDiff            = 0;
    msgSent                = 0;
    msgRecv                = 0;
    lastSyncRecv        = 0;
    syncPing            = 100;    // maybe too low, set so that the bot doesn't spam the server when it logs in
    avgPing                = 0;
    accuPing            = 0;
    countPing            = 0;
    highPing            = 0;
    lowPing                = 0;
    weaponCount            = 0;
    hasSysOp            = false;
    hasSMod                = false;
    hasMod                = false;
    inZone                = false;
    inArena                = false;
    downloadingNews        = false;
    billerOnline        = true;
    lastRecv            = getTickCount();
    lastIteration        = -1;

    // Reliables
    remoteStep = 0;
    received.clear();
    localStep = 0;
    localSent = 0;
    sent.clear();
    queued.clear();
    S2CSlowCurrent = 0;
    S2CFastCurrent = 0;
    S2CSlowTotal = 0;
    S2CFastTotal = 0;

    // Chatter
    chatLog.clear();
    lastChatSend = 0;
    countChatSend = 0;
}


Host::Host(BotInfo& bi) 
    : Observable("Host"), botInfo(bi)
{
    // define the events that can be received by observers of the m_host
    addEvent<uint64_t, const PlayerMap&, const FlagList&, const BrickList&, const Playfield&, std::string_view>(BotEvent::Init);
    addEvent(BotEvent::Tick);
    addEvent<std::string_view, Player&, bool>(BotEvent::ArenaEnter);
    addEvent(BotEvent::ArenaLeave);
    addEvent<const ArenaSettings&>(BotEvent::ArenaSettings);
    addEvent<std::string_view, bool, uint16_t>(BotEvent::ArenaListEntry);
    addEvent<std::string_view, bool, uint16_t>(BotEvent::ArenaListEnd);
    addEvent<ChatMode, ChatSoundCode, const Player&, std::string_view>(BotEvent::Chat);
    addEvent<const Player&, const Command&>(BotEvent::Command);
    addEvent<const Player&, const Player&>(BotEvent::CreateTurret);
    addEvent<const Player&, const Player&>(BotEvent::DeleteTurret);
    addEvent<const Player&>(BotEvent::PlayerEntering);
    addEvent<const Player&>(BotEvent::PlayerLeaving);
    addEvent<const Player&>(BotEvent::PlayerMove);
    addEvent<const Player&, WeaponInfo>(BotEvent::PlayerWeapon);
    addEvent<const Player&, const Player&, WeaponInfo, uint16_t, uint16_t>(BotEvent::WatchDamage);
    addEvent<const Player&, const Player&, int32_t, uint16_t>(BotEvent::PlayerDeath);
    addEvent<const Player&, uint16_t>(BotEvent::PlayerPrize);
    addEvent<const Player&>(BotEvent::PlayerScore);
    addEvent<const Player&, uint16_t, Ship>(BotEvent::PlayerShip);
    addEvent<const Player&, uint16_t, Ship>(BotEvent::PlayerSpec);
    addEvent<const Player&, uint16_t, Ship>(BotEvent::PlayerTeam);
    addEvent<const Player&>(BotEvent::BannerChanged);
    addEvent(BotEvent::SelfShipReset);
    addEvent(BotEvent::SelfUFO);
    addEvent<uint16_t, uint16_t>(BotEvent::SelfPrize);
    addEvent<const Player&, const Flag&>(BotEvent::FlagGrab);
    addEvent<const Player&>(BotEvent::FlagDrop);
    addEvent<const Flag&>(BotEvent::FlagMove);
    addEvent<uint16_t, uint32_t>(BotEvent::FlagVictory);
    addEvent(BotEvent::FlagGameReset);
    addEvent<uint16_t, uint32_t>(BotEvent::FlagReward);
    addEvent<const Player&, const Player&, const Player&, const Player&, const Player&>(BotEvent::TimedGameOver);
    addEvent<uint16_t, uint32_t>(BotEvent::SoccerGoal);
    addEvent<std::string_view>(BotEvent::File);
    addEvent<const PowerBall&>(BotEvent::BallMove);
    addEvent(BotEvent::PositionHook);
    addEvent<uint16_t>(BotEvent::ObjectToggled);
    addEvent<Coord, Coord, Coord, Coord, uint16_t>(BotEvent::BrickDropped);
    addEvent(BotEvent::MapLoaded);
    addEvent(BotEvent::Term);

    limitedOwnerId = UnassignedId;
    creation_parameters = bi.params;
    
    // Logs
    lastLogTime = 0;
    logging = false;

    // Session
    resetSession();

    // Stuff that shouldn't change between sessions
    turretMode = 0;
    broadcastingErrors = false;
    botChats = getBotDatabase().initialChatChannels;

    // GUI
//    terminal = NULL;
//    char *title = bi.name;
//    GUI.remoteCreate(terminal, &title);
    
    // Winsock
    remote.set(bi.ip, bi.port);
    socket.create(0);
    socket.set(remote);
    
    // Checksum dictionary
    generateDictionary(dictionary, 0);
    
    // Arena
    resetArena();
    
    // Staff
    if (getBotDatabase().runSilent)
        lowestLevel = OperatorLevel::Owner;
    else
        lowestLevel = OperatorLevel::Player;
        
    // Connect tries
    numTries = getBotDatabase().maxTries;
    
    // Imports
    imports = std::make_unique<BotStore>(*this);
    imports->importLibrary(bi.dllNames);

    lastTick = 0;
}


void Host::shutdown()
{
    if (killMe == false) {
        disconnect(true);
    }

    if (logging) {
        writeLogBuffer();
    }

    // delete all plugins
    imports->clearImports();
}


void Host::resetArena()
{
    bricks.clear();
    flags.clear();
    goals.clear();
    players.clear();
    playerIds.clear();
    balls.clear();

    // State
    position = false;
    playerId = UnassignedId;
    follow = UnassignedId;
    gotMap = false;
    gotTurf = false;
    loadedFlags = false;
    allowLimited = false;
    limitedOwnerId = UnassignedId;
    DLLFlying = false;

    // Paranoia - send extra info
    paranoid = false;
}


void Host::changeArena(std::string_view name)
{
    if (inArena) {
        raiseEvent(BotEvent::ArenaLeave);
        // warum wird das nicht gesendet?
        //postRR(generateArenaLeave());
        
        inArena = false;
    }

    resetArena();
    logEvent("Entering arena '{}'", std::string(name));
    postRR(generateArenaLogin(name, botInfo.initialShip, botInfo.xres, botInfo.yres, botInfo.allowAudio));
}


bool Host::validateSource(INADDR &src)
{
    return (src == remote);
}


void Host::spectateNext()
{
    // get the Id of the spectated player or playerIds.end() in case we are not speccing anyone
    std::list<uint16_t>::iterator nameIdIt = find(playerIds.begin(), playerIds.end(), follow);

    // if we are not following anyone and there also is noone to spec, return
    if (follow == UnassignedId && (nameIdIt = playerIds.begin()) == playerIds.end()) {
        return;
    }
    
    // unset the followed played Id in case we don't find a successor to spec
    follow = UnassignedId;

    if (nameIdIt == playerIds.end()) {
        // we have been following someone before, but now there is noone to follow anymore
        return;
    }

    uint16_t firstId = *nameIdIt;

    // traverse cyclically through the playernames, starting either with the currently followed player or with the 
    // first player in the list
    do {
        if (++nameIdIt == playerIds.end()) {
            nameIdIt = playerIds.begin();
        }

        Player& p{ getPlayer(*nameIdIt) };

        if (p.ship != Ship::Spectator) {
            follow = p.ident;
            break;
        }
    } while (*nameIdIt != firstId);

    if (follow != UnassignedId) {
        // we found the next player to follow, 
        Player& player{ getPlayer(follow) };
     
        if (getBotDatabase().noisySpectator) {
            uint64_t time = getTickCount();

            if (time - getPlayer(follow).lastPositionUpdate > 100 && time - lastSpec > 20) {
                // Request position if we've lost him
                player.lastPositionUpdate = time;
                spectate(player.ident);
            }
        }
       
        me().clone(player);
        me().d = 0;
    }
}


void Host::spectate(uint32_t playerId)
{
    // if playerId is unassigned, all players are spec'ed
    postRU(generateSpectate(playerId));
    speccing = (playerId != UnassignedId);
    lastSpec = getTickCount();
}


void Host::sendPosition(bool reliable, uint32_t timestamp, Projectile ptype, WeaponLevel level, bool shrapBounce, 
    uint8_t shrapLevel, uint8_t shrapCount, bool secondary)
{
    lastPosition = getTickCount();

    if (!hasBot()) {
        return;
    }

    Player& myself{ me() };

    if (myself.ident == UnassignedId)
        return;

    if (paranoid) {
        // We need to send ExtraPositionInfo
        ClientMessage cm = generatePosition(
            timestamp,
            myself.d,
            myself.pos[0],
            myself.pos[1],
            myself.vel[0],
            myself.vel[1],
            myself.stealth,
            myself.cloak,
            myself.xradar,
            myself.awarp,
            myself.flash,
            false,
            myself.ufo,
            myself.bounty,
            myself.energy,
            ptype, level, shrapBounce, shrapLevel, shrapCount, secondary,
            myself.timer,
            (uint16_t)syncPing,
            myself.shields,
            myself.supers,
            myself.burst,
            myself.repel,
            myself.thor,
            myself.brick,
            myself.decoy,
            myself.rocket,
            myself.portal);

        if (reliable)
            postRR(std::move(cm));
        else
            postRU(std::move(cm));
    }
    else {
        // We don't need to send ExtraPositionInfo
        ClientMessage cm = generatePosition(
            timestamp,
            myself.d,
            myself.pos[0],
            myself.pos[1],
            myself.vel[0],
            myself.vel[1],
            myself.stealth,
            myself.cloak,
            myself.xradar,
            myself.awarp,
            myself.flash,
            false,
            myself.ufo,
            myself.bounty,
            myself.energy,
            ptype, level, shrapBounce, shrapLevel, shrapCount, secondary);

        if (reliable)
            postRR(std::move(cm));
        else
            postRU(std::move(cm));
    }
}


void Host::sendPosition(uint32_t timestamp, bool reliable)
{
    sendPosition(reliable, timestamp, Projectile::None, WeaponLevel::One, false, 0, 0, false);
}


void Host::sendPosition(bool reliable)
{
    sendPosition(reliable, getHostTime(), Projectile::None, WeaponLevel::One, false, 0, 0, false);
}


void Host::doEvents()
{
    // clustering

    beginCluster();

    // poll

    UDPPacket *udpPacket;

    while (udpPacket = socket.poll()) {
        if (validateSource(udpPacket->src)) {
            gotPacket((uint8_t*)udpPacket->msg, udpPacket->len);

            if (killMe)
                return;
        }
    }

    // frame-latency

    uint64_t time = getTickCount();

    if (lastIteration != -1 && time - lastIteration >= SlowIteration) {
        logWarning("Slow iteration: {}ms", (time - lastIteration) * 10);
        lastRecv = time;
    }
    lastIteration = time;

    // core

    if (time - lastRecv > PacketSilenceLimit) {
        // auto-reconnect
        logWarning("Automatically reconnecting on timeout...");

        resetSession();
        resetArena();
        connect(false);
        return;
    }

    if (connecting) {
        if (time - lastConnect > (syncPing << 4)) {
            logWarning("Retrying connection...");
            connect(true);
        }
    }

    if (inArena) {
        if (syncing) {
            if (time - lastSync > syncPing + 10)
                syncClocks();
        }
        else if (getTickCount() - lastSync > SyncInterval) {
            syncClocks();
        }
    }

    sendQueued();

    // game

    if (position && playerId != UnassignedId) {
        Player& myself{ me() };

        if (DLLFlying) {
            uint32_t limit = settings.SendPositionDelay;

            if (time - lastPosition > limit) {
                 raiseEvent(BotEvent::PositionHook);
            }
        }
        else if (me().ship == Ship::Spectator) {
            // Spectating
            uint32_t limit = settings.SendPositionDelay;

            if (time - lastPosition > limit) {
                // Cycle player spectated
                if (me().ship == Ship::Spectator)
                    spectateNext();

                sendPosition(false);
            }
        }
        else if (follow != UnassignedId) {
            // Idle
            uint32_t limit = settings.SendPositionDelay * 8;

            if (time - lastPosition > limit) {
                sendPosition(false);
            }
        }
        else {    
            // In-game
            uint32_t limit = settings.SendPositionDelay * 2;

            if (time - lastPosition > limit) {
                // Prediction
                myself.move((uint32_t)(time - lastPosition));

                // Auto-turret
                if (settings.ships[(uint8_t)myself.ship].MaxGuns > 0) {
                    Player player;
                    uint32_t best = botInfo.xres / 2;

                    for (Player& p : players | std::views::values) {
                        if ((p.ident == myself.ident) || (p.ship == Ship::Spectator)) {
                            continue;
                        }

                        if (turretMode == 0) {
                            if (myself.team == p.team) continue;
                        } 
                        else {
                            if (myself.team != p.team) continue;
                        }

                        uint32_t d = distance(myself.pos, p.pos);

                        if (d < best) {
                            player = p;
                            best = d;
                        }
                    }

                    if (player.isAssigned()) {
//                            myself.d = TriangulateFireAngle(myself.pos - p->pos);        // No "leading" bullet streams.
                        
                        myself.d = triangulateFireAngle(vectorSub(myself.work, player.work), vectorSub(player.vel, myself.vel), 
                            settings.ships[(uint8_t)myself.ship].BombSpeed / 1000);
                        if (turretMode == 1) 
                            myself.d = oppositeDirection(myself.d);
                        sendPosition(false, getHostTime(), Projectile::Bullet, WeaponLevel(settings.ships[(uint8_t)myself.ship].MaxGuns - 1), false, 0, 0, true);
                    } 
                    else {
                        sendPosition(false);
                    }
                } 
                else {
                    sendPosition(false);
                }
            }
        }
    }

    // chat

    doChatLog();

    // dll

    if (time - lastTick >= 100) {
        // tick once a second
        lastTick = time;
        raiseEvent(BotEvent::Tick);
    }

    // clustering

    endCluster();

    // items

    //ps.doPrizes((uint16_t)playerlist.total);
    doBrickEvents();
    doBallEvents();

    // logs

    if (logging && (time - lastLogTime > LogInterval)) {
        writeLogBuffer();

        lastLogTime = time;
    }
}


void Host::gotPacket(uint8_t* msg, size_t len)
{
    encryption.decrypt(msg, len);

#ifdef S2C_LOG
    logIncoming(msg, len);
#endif

    gotMessage(msg, len);

    ++msgRecv;    // Note: do not recursively call gotPacket()
    lastRecv = getTickCount();
}


void Host::gotMessage(uint8_t* msg, size_t len)
{
    HostMessage m(msg, len, this);

    gotMessage(m);
}


void Host::gotMessage(HostMessage& msg)
{    // Skip HandleSpecialHeader() for speed
    uint8_t type = msg.getType(false);

    if (type == 0)
        specialRouter.call(msg.getType(true), msg);
    else
        generalRouter.call(type, msg);
}


void Host::gotSpecialMessage(HostMessage& msg)
{
    specialRouter.call(msg.getType(true), msg);
}


void Host::gotEncryptRequest(uint32_t key, uint16_t prot)
{
    // This is not a server
}


void Host::gotEncryptResponse(uint32_t key)
{
    if (encryption.initializeEncryption(key)) {
        // Stuff we won't be needing anymore
        specialRouter.kill(0x05);
        specialRouter.kill(0x02);

        // Reliable messaging
        specialRouter.add(0x03, handleReliable);
        specialRouter.add(0x04, handleACK);

        // Synchronization
        specialRouter.add(0x06, handleSyncResponse);
        syncClocks();

        // Transmission types
        specialRouter.add(0x08, handleChunkBody);
        specialRouter.add(0x09, handleChunkTail);
        specialRouter.add(0x0A, handleBigChunk);
        specialRouter.add(0x0B, handleCancelDownload);
        specialRouter.add(0x0C, handleCancelDownloadAck);

        // Game protocol
        generalRouter.add(0x0A, handlePasswordResponse);
        generalRouter.add(0x10, handleFileTransfer);
        generalRouter.add(0x31, handleLoginNext);

        // Additional Continuum game protocol
        generalRouter.add(0x33, handleCustomMessage);
        generalRouter.add(0x34, handleVersionCheck);

        logEvent("Sending password for {}", botInfo.name);

        if (getBotDatabase().forceContinuum) {
            postRR(generateCtmPassword(false, botInfo.name, botInfo.password, botInfo.machineID, botInfo.timeZoneBias,
                botInfo.permissionID, ContinuumVersion, ConnectMode::UnknownNotRAS));
        }
        else {
            postRR(generatePassword(false, botInfo.name, botInfo.password, botInfo.machineID, botInfo.timeZoneBias, 
                botInfo.permissionID, SubspaceVersion, ConnectMode::UnknownNotRAS));
        }
    }
    connecting = false;
}


void Host::activateGameProtocol()
{
    // Enabled protocol
    generalRouter.add(0x01,    handleIdent);
    generalRouter.add(0x02,    handleInGameFlag);
    generalRouter.add(0x03,    handlePlayerEntering);
    generalRouter.add(0x04,    handlePlayerLeaving);
    generalRouter.add(0x05,    handleWeaponUpdate);
    generalRouter.add(0x06,    handlePlayerDeath);
    generalRouter.add(0x07,    handleChat);
    generalRouter.add(0x08,    handlePlayerPrize);
    generalRouter.add(0x09,    handleScoreUpdate);
    generalRouter.add(0x0B,    handleSoccerGoal);
    generalRouter.add(0x0C,    handlePlayerVoice);
    generalRouter.add(0x0D,    handleSetTeam);
    generalRouter.add(0x0E,    handleCreateTurret);
    generalRouter.add(0x0F,    handleArenaSettings);
    generalRouter.add(0x12,    handleFlagPosition);
    generalRouter.add(0x13,    handleFlagClaim);
    generalRouter.add(0x14,    handleFlagVictory);
    generalRouter.add(0x15,    handleDeleteTurret);
    generalRouter.add(0x16,    handleFlagDrop);
    generalRouter.add(0x18,    handleSynchronization);
    generalRouter.add(0x19,    handleFileRequest);
    generalRouter.add(0x1A,    handleScoreReset);
    generalRouter.add(0x1B,    handleShipReset);
    generalRouter.add(0x1C,    handleSpecPlayer);
    generalRouter.add(0x1D,    handleSetTeamAndShip);
    generalRouter.add(0x1E,    handleBannerFlag);
    generalRouter.add(0x1F,    handlePlayerBanner);
    generalRouter.add(0x20,    handleSelfPrize);
    generalRouter.add(0x21,    handleBrickDrop);
    generalRouter.add(0x22,    handleTurfFlagStatus);
    generalRouter.add(0x23,    handleFlagReward);
    generalRouter.add(0x24,    handleSpeedStats);
    generalRouter.add(0x25,    handleToggleUFO);
    generalRouter.add(0x27,    handleKeepAlive);
    generalRouter.add(0x28,    handlePlayerPosition);
    generalRouter.add(0x29,    handleMapInfo);
    generalRouter.add(0x2A,    handleMapFile);
    generalRouter.add(0x2B,    handleSetKoTHTimer);
    generalRouter.add(0x2C,    handleKoTHReset);
    generalRouter.add(0x2D,    handleAddKoTH);
    generalRouter.add(0x2E,    handleBallPosition);
    generalRouter.add(0x2F,    handleArenaList);
    generalRouter.add(0x30,    handleBannerAds);
    generalRouter.add(0x32,    handleChangePosition);
    generalRouter.add(0x35,    handleObjectToggle);
    generalRouter.add(0x36,    handleReceivedObject);
    generalRouter.add(0x37,    handleDamageToggle);
    generalRouter.add(0x38,    handleWatchDamage);

    // Disabled protocol
    generalRouter.kill(0x0A);    // Password response
    generalRouter.kill(0x31);    // Login next
    generalRouter.kill(0x33);    // Custom response
    generalRouter.kill(0x34);    // Continuum version
}


void Host::gotEncryptResponse(uint32_t key, uint8_t mudge)
{
    gotEncryptResponse(key);
}


void Host::gotReliable(uint32_t id, uint8_t* msg, size_t len)
{
    sendACK(id);
    checkReceived(id, msg, len);
}


void Host::gotACK(uint32_t id)
{
    checkSent(id);
}


void Host::gotSyncRequest(uint32_t time)
{
    uint8_t msg[10]{};
    msg[0] = 0x00;
    msg[1] = 0x06;
    *(uint32_t*)&msg[2] = time;
    *(uint32_t*)&msg[6] = (uint32_t)getTickCount();
    
    post(msg, 10, false);
}


void Host::gotSyncRequest(uint32_t time, uint32_t sent, uint32_t recv)
{
    gotSyncRequest(time);
}


void Host::gotSyncResponse(uint32_t pingTime, uint32_t pongTime)
{    // This function is pretty much straight from SubSpace
    syncing = false;

    // timing
    uint64_t ticks = getTickCount();
    uint64_t round_trip = ticks - pingTime;

    // average ping
    accuPing += round_trip;
    ++countPing;
    avgPing = accuPing / countPing;

    // high ping
    if (round_trip > highPing) {
        highPing = round_trip;
    }

    // low ping
    if ((lowPing == 0) || (round_trip < lowPing)) {
        lowPing = round_trip;
    }

    // slow pings get ignored until the next sec.chk
    if (round_trip > syncPing + 1) {
        if (ticks - lastSyncRecv <= 12000)
            return;
    }

    // 7/26/03 removed a typo here

    // ping spikes get ignored
    if (round_trip >= syncPing * 2) {
        if (ticks - lastSyncRecv <= 60000)
            return;
    }

    timeDiff = ((round_trip * 3) / 5) + pongTime - ticks;    // 64-bit fixed point math sucks

    if (timeDiff >= -10 && timeDiff <= 10) 
        timeDiff = 0;

    lastSyncRecv = ticks;
    syncPing = round_trip;
}


void Host::gotDisconnect()
{    // auto-reconnect
    logEvent("Automatically reconnecting on disconnection...");
    resetSession();
    resetArena();
    connect(false);
}


void Host::gotChunkBody(uint8_t* msg, size_t len)
{
    little.addMessage(msg, len);
}


void Host::gotChunkTail(uint8_t* msg, size_t len)
{
    little.addMessage(msg, len);

    if (little.buffer) {
        gotMessage(little.buffer, little.currentLength);
        little.deleteMessage();
    }
}


void Host::gotBigChunk(uint32_t total, uint8_t* msg, size_t len)
{
    if (ftLength == 0) {
        ftLength = total;
    }
    else if (ftLength != total) {
        // This protocol implementation does not support recursive/parallel transfers
        large.deleteMessage();
        ftLength = total;
    }

    large.addMessage(msg, len);

    if (large.currentLength == total) {
        if (large.buffer) {
            gotMessage(large.buffer, large.currentLength);
            large.deleteMessage();
        }
        ftLength = 0;
    }
    else if (large.currentLength > total) {
        large.deleteMessage();
        ftLength = 0;
    }
}


void Host::gotCancelDownload()
{
    large.deleteMessage();

    ftLength = 0;

    sendDownloadCancelAck();

    logEvent("Download cancelled by peer");
}


void Host::gotCancelDownloadAck()
{
    // not sure if we really need to worry about this ^_^
}


void Host::gotCluster(uint8_t* msg, size_t len)
{
    size_t index = 0;
    
    do {
        // Safe provided <specialprot.h>
        size_t this_len = *(uint8_t*)&msg[index++];

        if ((this_len == 0) || (index + this_len > len)) {
            return;
        }

        uint8_t* this_msg = msg + index;

        gotMessage(this_msg, this_len);

        index += this_len;
    } while (index < len);
}


//////// Core out ////////

void Host::connect(bool postDisconnect)
{
    connecting    = true;
    lastConnect = getTickCount();

    uint8_t msg[8]{};

    msg[0] = 0x00;  // special header
    msg[1] = 0x01;  // type 1: connection

    if (getBotDatabase().encryptMode) {
        // generateKey() should return the same random key every time 
        *(uint32_t*)&msg[2] = encryption.generateKey();
    }
    else {
        *(uint32_t*)&msg[2] = 0;    // disable encryption
    }
    *(uint16_t*)&msg[6] = 0x0001;   // protocol version = 1 (SubSpace), or 17 (Continuum)
    
    logEvent("Connecting zone at {} port {}", remote.getString(), remote.getPort());

    if (postDisconnect) {
        uint8_t dmsg[2]{};

        dmsg[0] = 0x00;
        dmsg[1] = 0x07;

        send(dmsg, 2);
    }
    send(msg, 8);
}


void Host::disconnect(bool notify)
{
    if (notify) {
        if (inArena) {
            ClientMessage cm{ generateArenaLeave() };

            send(&cm.msg[0], cm.msg.size());
            raiseEvent(BotEvent::ArenaLeave);
        }
        uint8_t msg[2]{};

        msg[0] = 0x00;
        msg[1] = 0x07;

        send(msg, 2);
        send(msg, 2);
    }

    killMe = true;
}


void Host::syncClocks()
{
    syncing     = true;
    lastSync = getTickCount();

    uint8_t msg[14]{};

    msg[0] = 0x00;
    msg[1] = 0x05;
    *(uint32_t*)&msg[2]  = (uint32_t)getTickCount();
    *(uint32_t*)&msg[6]  = msgSent;
    *(uint32_t*)&msg[10] = msgRecv;

    post(msg, 14, false);
}


void Host::sendDownloadCancelAck()
{
    uint8_t msg[2]{};

    msg[0] = 0x00;
    msg[1] = 0x0c;
    post(msg, 2, true);
}


//////// Logging ////////

/// <summary>
/// Write a message to the console and optionally (config parameter ChatterLog) to a file. The message will be 
/// logged, if the required log level is less or equal to the configured log level (config parameter LogLevel).
/// </summary>
/// <param name="msg">Message to be logged.</param>
/// <param name="level">Required log level.</param>
/// <param name="color">Log color.</param>
void Host::logEvent(std::string_view msg, LogLevel level, LogColor color)
{
    if (getBotDatabase().logLevel == LogLevel::NoOutput 
        || level > getBotDatabase().logLevel
        || msg.find(getBotDatabase().defaultName) != std::string::npos) {
        // no terminal output
        return;
    }

    if (!logging) {
        if (getBotDatabase().chatterLog) {
            std::string fileName{ "log/" + botInfo.name + ".log" };

            if (!std::filesystem::is_directory(std::filesystem::path("log"))) {
                std::filesystem::create_directory(std::filesystem::path("log"));
            }
            logFile.open(fileName);

            if (logFile) {
                logging = true;
            }
        }
    }

    if (logging) {
        logBuffer += std::format("{} {}\r\n, ", getTimeString(), msg);
    }

    if (loggedChatter.size() == MaxLogLines) {
        loggedChatter.pop_front();
    }
    loggedChatter.push_back(msg.data());

    static std::map<LogColor, std::string> logColor{
        { LogColor::Neutral, "\x1b[0m" },
        { LogColor::Warning, "\x1b[38;2;226;186;8m" },
        { LogColor::Error, "\x1b[31m" }
    };

    std::cout << std::format("{}{}  {}{}\n", logColor[color], botInfo.name, msg, logColor[LogColor::Neutral]);
}


void Host::writeLogBuffer()
{
    logFile.write(logBuffer.c_str(), logBuffer.length());
    logBuffer.clear();
}


void Host::logLogin()
{
    if (getBotDatabase().recordLogins) {
        if (!std::filesystem::is_directory(std::filesystem::path("log"))) {
            std::filesystem::create_directory(std::filesystem::path("log"));
        }

        std::ofstream file("log/Logins.txt", std::ios::app);

        if (!file) {
            return;
        }
        file << botInfo.ip << '\t' << getTimeString() << '\t' << botInfo.name << '\t' << botInfo.password << std::endl;
    }
    numTries = getBotDatabase().maxTries;
}


std::string makePacketLog(std::string prefix, std::vector<uint8_t>& packet, size_t len)
{
    char stringRep[17];
    memset(stringRep, 0, 17);
    uint32_t cnt, strOffset = 0;
    char c;

    std::string s = prefix + "0000 ";

    for (cnt = 0; cnt < len; ++cnt) {
        if (cnt && ((cnt & 15) == 0)) {
            s += stringRep;
            s += "\r\n";
            s += prefix;
            s += std::format("{:04x}", cnt);
            s += " ";

            memset(stringRep, 0, 17);
            strOffset = 0;
        }
        s += std::format("{:02x}", packet[cnt]);
        s += " ";

        c = packet[cnt];
        if (isPrintable((char)c))
            stringRep[strOffset++] = c;
        else
            stringRep[strOffset++] = '.';
    }

    while (cnt & 15) {
        s += "   ";
        ++cnt;
    }
    s += stringRep;
    s += "\r\n";
    return s;
}


void Host::logIncoming(std::vector<uint8_t>& packet, size_t len)
{
    if (!logging) {
        return;
    }
    logBuffer += makePacketLog("IN  ", packet, len);
}


void Host::logOutgoing(std::vector<uint8_t>& packet, size_t len)
{
    if (!logging) {
        return;
    }
    logBuffer += makePacketLog("OUT ", packet, len);
}


std::string Host::getTimeString()
{
    using namespace std::chrono;
    // get a local time_point with system_clock::duration precision
    auto now = zoned_time{ current_zone(), system_clock::now() }.get_local_time();
    // get a local time_point with days precision
    auto ld = floor<days>(now);
    // convert local days-precision time_point to a local {y, m, d} calendar year_month_day ymd{ ld };
    // split time since local midnight into {h, m, s, subseconds}
    hh_mm_ss hms{ now - ld };
    int32_t hour = hms.hours().count();
    int32_t minute = hms.minutes().count();
    int64_t second = hms.seconds().count();

    return std::format("{:#02}:{:#02}:{:#02}", hour, minute, second);
}
