module;
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

module BotStore;

import BotDb;
import ClientProt;
import Command;
import Algorithms;
import Host;
import Playfield;
import Observable;

import <format>;
import <functional>;
import <ranges>;


typedef Observable& (__stdcall *pfnCreateSpawn)(std::string, std::string, Observable&);
typedef void (__stdcall* pfnSetupSpawn)(std::string, std::string);
typedef void (__stdcall *pfnEnsureInit)(void);
typedef void (__stdcall *pfnForceTerm)(void);


//////// Plugins ////////

BotStore::BotStore(Host& h)
    : m_host(h)
{
    for (int slot = 0; slot < DllMaxLoaded; ++slot) {
        m_dllHandles[slot] = nullptr;
    }
}


/// <summary>
/// Import plugin DLLs.
/// </summary>
/// <param name="dllNames">Comma-separated list of DLL names.</param>
/// <returns>True, if the import was successful.</returns>
bool BotStore::importLibrary(std::string dllNames)
{
    bool succeeded{ true };
    std::string pluginName = splitFirst(dllNames, ',');

    if (pluginName != dllNames && !dllNames.empty()) {
        succeeded &= importLibrary(dllNames);
    }

    // Find open import slot
    int slot;

    for (slot = 0; slot < DllMaxLoaded; ++slot) {
        if (!m_dllHandles[slot])
            break;
    }

    if (slot == DllMaxLoaded) {
        handleBotEventExit(std::format("The maximum number of {} DLLs has been exceeded!", DllMaxLoaded));
        return false;
    }

    // Avoid directory traversal exploits.
    erase(pluginName, '/');
    erase(pluginName, '\\');

    try {
        m_dllHandles[slot] = LoadLibrary(pluginName.c_str());
    }
    catch (...) {
        handleBotEventExit(std::format("Exception in DLLMain while loading plugin {} at slot {}", pluginName, slot));
        return false;
    }

    // NOTE: This function is not strictly required for mixed-mode DLLs but it is the approach that Microsoft 
    // recommends. Any mixed-mode DLL should initialize the C runtime in this function.
    pfnEnsureInit pfnDll = (pfnEnsureInit)GetProcAddress(m_dllHandles[slot], "DllEnsureInit");

    if (pfnDll) {
        pfnDll();
    }

    m_pluginNames[slot] = pluginName;

    try {
        // create a spawn of the plugin
        auto createSpawn{ (pfnCreateSpawn)GetProcAddress(m_dllHandles[slot], "createSpawn") };
        Observable& obSpawn = createSpawn(m_host.botInfo.name, getPluginName(slot), m_host);

        // register event handlers for the events from the plugin
        obSpawn.registerEventHandler(BotEvent::Echo, &BotStore::handleBotEventEcho, this);
        obSpawn.registerEventHandler(BotEvent::Warning, &BotStore::handleBotEventWarning, this);
        obSpawn.registerEventHandler(BotEvent::Error, &BotStore::handleBotEventError, this);
        obSpawn.registerEventHandler(BotEvent::Exit, &BotStore::handleBotEventExit, this);
        obSpawn.registerEventHandler(BotEvent::Say, &BotStore::handleBotEventSay, this);
        obSpawn.registerEventHandler(BotEvent::Die, &BotStore::handleBotEventDie, this);
        obSpawn.registerEventHandler(BotEvent::Attach, &BotStore::handleBotEventAttach, this);
        obSpawn.registerEventHandler(BotEvent::Detach, &BotStore::handleBotEventDetach, this);
        obSpawn.registerEventHandler(BotEvent::Following, &BotStore::handleBotEventFollowing, this);
        obSpawn.registerEventHandler(BotEvent::Flying, &BotStore::handleBotEventFlying, this);
        obSpawn.registerEventHandler(BotEvent::Banner, &BotStore::handleBotEventBanner, this);
        obSpawn.registerEventHandler(BotEvent::DropBrick, &BotStore::handleBotEventDropBrick, this);
        obSpawn.registerEventHandler(BotEvent::Ship, &BotStore::handleBotEventShip, this);
        obSpawn.registerEventHandler(BotEvent::Team, &BotStore::handleBotEventTeam, this);
        obSpawn.registerEventHandler(BotEvent::GrabFlag, &BotStore::handleBotEventGrabFlag, this);
        obSpawn.registerEventHandler(BotEvent::SendPosition, &BotStore::handleBotEventSendPosition, this);
        obSpawn.registerEventHandler(BotEvent::DropFlags, &BotStore::handleBotEventDropFlags, this);
        obSpawn.registerEventHandler(BotEvent::SpawnBot, &BotStore::handleBotEventSpawnBot, this);
        obSpawn.registerEventHandler(BotEvent::ChangeArena, &BotStore::handleBotEventChangeArena, this);
        obSpawn.registerEventHandler(BotEvent::FireWeapon, &BotStore::handleBotEventFireWeapon, this);
        obSpawn.registerEventHandler(BotEvent::ToggleObjects, &BotStore::handleBotEventToggleObjects, this);
        obSpawn.registerEventHandler(BotEvent::ModifyObjects, &BotStore::handleBotEventModifyObjects, this);
        obSpawn.registerEventHandler(BotEvent::GrabBall, &BotStore::handleBotEventGrabBall, this);
        obSpawn.registerEventHandler(BotEvent::FireBall, &BotStore::handleBotEventFireBall, this);
        obSpawn.registerEventHandler(BotEvent::ChangeSettings, &BotStore::handleBotEventChangeSettings, this);

        // set up the spawn in a separate step after registering the event handlers
        auto setupSpawn{ (pfnSetupSpawn)GetProcAddress(m_dllHandles[slot], "setupSpawn") };

        setupSpawn(m_host.botInfo.name, getPluginName(slot));
    }
    catch (std::exception& ex) {
        handleBotEventExit(ex.what());
        return false;
    }

    // send an initial event with initialization parameters to this plugin
    m_host.raiseEvent(BotEvent::Init, uint32_t(CoreMajorVersion << 16) | CoreMinorVersion,
        m_host.players, m_host.flags, m_host.bricks, (Playfield)m_host.playfieldTiles, "m_host.creation_parameters");
    
    // in case we are already in an arena, send further configuration parameters to the plugin
    if (m_host.inArena) {
        m_host.raiseEvent(BotEvent::ArenaEnter, m_host.botInfo.initialArena, m_host.me(), m_host.billerOnline);
        m_host.raiseEvent(BotEvent::ArenaSettings, m_host.settings);
    }

    return succeeded;
}


/// <summary>
/// Clear the plugin DLL of a specified slot.
/// </summary>
/// <param name="slot"></param>
void BotStore::clearImport(int slot)
{
    if (slot < 0 || slot >= DllMaxLoaded)
        return;

    if (m_dllHandles[slot]) {
        m_host.raiseEvent(BotEvent::Term);

        // NOTE: This function is not strictly required for mixed-mode DLLs but it is the approach that Microsoft 
        // recommends. Any mixed-mode DLL should terminate the C runtime in this function.
        pfnForceTerm pfnDll = (pfnForceTerm)GetProcAddress(m_dllHandles[slot], "DllForceTerm");

        if (pfnDll) {
            pfnDll();
        }

        FreeLibrary(m_dllHandles[slot]);
        m_dllHandles[slot] = nullptr;
    }
}


/// <summary>
/// Clear all imported plugin DLLs.
/// </summary>
/// <param name="slot"></param>
void BotStore::clearImports()
{
    for (int slot = 0; slot < DllMaxLoaded; ++slot) {
        clearImport(slot);
    }
}


std::string BotStore::getPluginName(int slot) const
{
    if (slot < 0 || slot >= DllMaxLoaded)
        return "";
    if (!m_dllHandles[slot])
        return "";

    return m_pluginNames[slot];
}


//////// Events DLL->Bot ////////

void BotStore::handleBotEventEcho(std::string_view msg)
{
    m_host.logEvent(std::format("Ext: {}" , msg));
}


void BotStore::handleBotEventWarning(std::string_view msg)
{
    m_host.logWarning("Ext: {}", msg);
}


void BotStore::handleBotEventError(std::string_view msg)
{
    m_host.logError("Ext: {}", msg);
}


void BotStore::handleBotEventExit(std::string_view msg)
{
    // mark this host to be deleted
    m_host.logError("Ext: {}", msg);
    m_host.killMe = true;
}


void BotStore::handleBotEventSay(ChatMode chatMode, ChatSoundCode soundCode, uint16_t recipient, 
    std::string_view msg)
{
    // note: team private messages need a player ident param, not a team number
    if (chatMode == ChatMode::TeamPrivate) {
        for (Player& p : m_host.players | std::views::values) {
            if (p.team == recipient) {
                // recipient is a team freq
                m_host.tryChat(chatMode, soundCode, p.ident, msg);
                return;
            }
        }
    }
    else {
        // recipient is a player Id, send the message to this player
        m_host.tryChat(chatMode, soundCode, recipient, msg);
    }
}


void BotStore::handleBotEventDie(const Player& player)
{
    if (m_host.playerId != UnassignedId) {
        m_host.postRR(generateDeath(player.ident, m_host.me().bounty));
    }
}


void BotStore::handleBotEventAttach(const Player& player)
{
    m_host.postRR(generateAttachRequest(player.ident));
}


void BotStore::handleBotEventDetach()
{
    m_host.postRR(generateAttachRequest(UnassignedId));
}


void BotStore::handleBotEventFollowing(bool isFollowing)
{
    //the original logic:
    //bool following = *(bool*)&event.p[0];
    //m_host.follow = !following ? NULL : m_host.playerlist.head;
    m_host.spectateNext();
}


void BotStore::handleBotEventFlying(bool isFlying)
{
    m_host.DLLFlying = isFlying;
}


void BotStore::handleBotEventBanner(Banner banner)
{
    m_host.postRR(generateChangeBanner(banner));
}


void BotStore::handleBotEventDropBrick()
{
    m_host.postRU(generateBrickDrop(m_host.me().tile[0], m_host.me().tile[1]));
}


void BotStore::handleBotEventShip(Ship ship)
{
    m_host.postRR(generateChangeShip(ship));
}


void BotStore::handleBotEventTeam(uint16_t team)
{
    m_host.postRR(generateChangeTeam(team));
}


void BotStore::handleBotEventGrabFlag(FlagId flagId)
{
    m_host.postRR(generateFlagRequest(flagId));
}


void BotStore::handleBotEventSendPosition(bool isReliable)
{
    m_host.sendPosition(isReliable);
}


void BotStore::handleBotEventDropFlags()
{
    m_host.postRR(generateFlagDrop());
}


void BotStore::handleBotEventSpawnBot(std::string_view name, std::string_view password, std::string_view staff,
    std::string_view arena)
{
    if (getHostList().getConnections() >= m_host.botInfo.maxSpawns) {
        return;
    }

    std::string arenaName{ arena };
    std::string newName{ name };
    std::string newPassword{ password };
    std::string newStaff{ staff };
    BotInfo bi;

//    bi.set(m_host.botInfo);

    // Name
    if (invalidName(newName)) {
        return;
    }
    // Password
    if (newPassword.empty()) {
        newPassword = bi.password;
    }
    // Staff
    if (newStaff.empty()) {
        newStaff = bi.staffpw;
    }
    // Arena
    if (arenaName.empty()) {
        arenaName = bi.initialArena;
    }
    if (invalidArena(arenaName)) {
        return;
    }
    bi.setLogin(newName, newPassword, newStaff);
    bi.setArena(arenaName, bi.initialShip, bi.xres, bi.yres, bi.allowAudio);

    // connect the host
    getHostList().connectHost(bi);
}


void BotStore::handleBotEventChangeArena(std::string_view arena)
{
    m_host.botInfo.setArena(arena, m_host.botInfo.initialShip, m_host.botInfo.xres, m_host.botInfo.yres, 
        m_host.botInfo.allowAudio);

    std::string newArena{ arena };
        
    if (!invalidArena(newArena)) {
        m_host.changeArena(newArena);
    }
}


void BotStore::handleBotEventFireWeapon(WeaponInfo weaponInfo)
{
    m_host.sendPosition(false, m_host.getHostTime(), weaponInfo.type, weaponInfo.level, weaponInfo.shrapBounce, 
        weaponInfo.shrapLevel, weaponInfo.shrapCount, weaponInfo.fireType);
}


void BotStore::handleBotEventToggleObjects(PlayerId playerId, const std::list<LvzObjectInfo>& objectInfos)
{
    if (objectInfos.size() == 0) {
        return;
    }

    if (m_host.hasSysOp) {
        m_host.postRR(generateObjectToggle(playerId, objectInfos));
    }
    else {
        std::string s{ "*objset " };

        for (const LvzObjectInfo& objectInfo : objectInfos) {
            if (objectInfo.disabled) {
                s += std::format("-{},", (int)objectInfo.id);
            }
            else {
                s += std::format("+{},", (int)objectInfo.id);
            }
        }

        if (playerId == UnassignedId) {
            m_host.tryChat(ChatMode::Public, ChatSoundCode::None, 0, s);
        }
        else {
            m_host.tryChat(ChatMode::Private, ChatSoundCode::None, playerId, s);
        }
    }
}


void BotStore::handleBotEventModifyObjects(PlayerId playerId, const std::list<LvzObject>& objects)
{
    if (objects.size() == 0) {
        return;
    }

    m_host.postRR(generateObjectModify(playerId, objects));
}


void BotStore::handleBotEventGrabBall(PlayerId playerId)
{
    for (PowerBall& ball : m_host.balls) {
        if (playerId == ball.ident) {
            m_host.postRR(generatePowerballRequest(ball.hosttime, (uint8_t)playerId));
            return;
        }
    }
}


void BotStore::handleBotEventFireBall(uint8_t ballId, Coord x, Coord y, Velocity vx, Velocity vy)
{
    try {
        m_host.postRR(generatePowerballUpdate(m_host.getHostTime(), ballId, x, y, vx, vy, 0xffff));
        // Cyan~Fire noticed this didn't compile well in MinGW
    }
    catch (...) {
        m_host.logError("Exception during EVENT 'BotEventFireBall'");
    }
}


void BotStore::handleBotEventChangeSettings(const std::list<std::string>& settings)
{
    try {
        m_host.postRR(generateChangeSettings(settings));
    }
    catch (...) {
        m_host.logError("Exception during EVENT 'BotEventChangeSettings'");
    }
}
