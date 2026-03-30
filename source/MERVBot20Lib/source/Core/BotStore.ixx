//
// Import bot behavior from DLL's by cat02e@fsu.edu
//
// Cyan~Fire fixed MinGW compilation for DLLImports::talk()
//
module;
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

export module BotStore;

import BotEvent;
import Player;

import <string>;
import <list>;


export extern const uint32_t DllMaxLoaded{ 32 };


export class BotStore
{
public:
    // Initialize all callbacks
    BotStore(class Host& h);

public:
    // Load callbacks from DLL
    bool importLibrary(std::string dllNames);

    /// <summary>
    /// Clear the plugin DLL of a specified slot.
    /// </summary>
    /// <param name="slot"></param>
    void clearImport(int slot);

    /// <summary>
    /// Clear all imported plugin DLLs.
    /// </summary>
    /// <param name="slot"></param>
    void clearImports();

    // Get plugin name, returns NULL if no plugin loaded
    std::string getPluginName(int slot) const;

    //////// Events DLL->Bot ////////

    void handleBotEventEcho(std::string_view msg);

    void handleBotEventWarning(std::string_view msg);
        
    void handleBotEventError(std::string_view msg);

    void handleBotEventExit(std::string_view msg);

    void handleBotEventSay(ChatMode chatMode, ChatSoundCode soundCode, uint16_t recipient, std::string_view msg);

    void handleBotEventDie(const Player& player);

    void handleBotEventAttach(const Player& player);

    void handleBotEventDetach();

    void handleBotEventFollowing(bool isFollowing);

    void handleBotEventFlying(bool isFlying);

    void handleBotEventBanner(Banner banner);

    void handleBotEventDropBrick();

    void handleBotEventShip(Ship ship);

    void handleBotEventTeam(uint16_t team);

    void handleBotEventGrabFlag(FlagId flagId);

    void handleBotEventSendPosition(bool isReliable);

    void handleBotEventDropFlags();

    void handleBotEventSpawnBot(std::string_view name, std::string_view password, std::string_view staff,
        std::string_view arena);

    void handleBotEventChangeArena(std::string_view arena);

    void handleBotEventFireWeapon(WeaponInfo weaponInfo);

    void handleBotEventToggleObjects(PlayerId playerId, const std::list<LvzObjectInfo>& objectInfos);

    void handleBotEventModifyObjects(PlayerId playerId, const std::list<LvzObject>& objects);

    void handleBotEventGrabBall(PlayerId playerId);

    void handleBotEventFireBall(uint8_t ballId, Coord x, Coord y, Velocity vx, Velocity vy);

    void handleBotEventChangeSettings(const std::list<std::string>& settings);

private:
    Host& m_host;
    HMODULE m_dllHandles[DllMaxLoaded]{};
    std::string m_pluginNames[DllMaxLoaded];
};
