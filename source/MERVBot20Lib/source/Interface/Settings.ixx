export module Settings;

import <format>;

export uint32_t SubspaceVersion{ 134 };
export uint32_t ContinuumVersion{ 38 };
export uint32_t CoreMajorVersion{ 2 };
export uint32_t CoreMinorVersion{ 0 };

export std::string IniFileName{ "MERVBot20.INI" };
export std::string VersionInfo{ std::format("MERVBot20 v{}.{}  by Aletheia (based on Catid's MERVBot v6.8)", 
    CoreMajorVersion, CoreMinorVersion) };


/// <summary>
/// Ship settings. 
/// </summary>
export struct ShipSettings              // 144 uint8_ts wide, offsets are for warbird
{
    uint16_t SuperTime;                 // 0004 All:SuperTime:1::How long Super lasts on the ship (in hundredths of a second)
    uint16_t UNKNOWN0;                  // 0006 (100)    Salt for actual super time?
    uint16_t ShieldsTime;               // 0008 All:ShieldsTime:1::How long Shields lasts on the ship (in hundredths of a second)
    uint16_t UNKNOWN1;                  // 0010 (30)    Salt for actual shields time?
    uint16_t Gravity;                   // 0012 All:Gravity:::Uses this formula, where R = raduis (tiles) and g = this setting; R = 1.325 * (g ^ 0.507)  IE: If set to 500, then your ship will start to get pulled in by the wormhole once you come within 31 tiles of it
    uint16_t GravityTopSpeed;           // 0014 All:GravityTopSpeed:::Ship are allowed to move faster than their maximum speed while effected by a wormhole.  This determines how much faster they can go (0 = no extra speed)
    uint16_t BulletFireEnergy;          // 0016 All:BulletFireEnergy:::Amount of energy it takes a ship to fire a single L1 bullet
    uint16_t MultiFireEnergy;           // 0018 All:MultiFireEnergy:::Amount of energy it takes a ship to fire multifire L1 bullets
    uint16_t BombFireEnergy;            // 0020 All:BombFireEnergy:::Amount of energy it takes a ship to fire a single bomb
    uint16_t BombFireEnergyUpgrade;     // 0022 All:BombFireEnergyUpgrade:::Extra amount of energy it takes a ship to fire an upgraded bomb. ie. L2 = BombFireEnergy+BombFireEnergyUpgrade
    uint16_t MineFireEnergy;            // 0024 All:LandmineFireEnergy:::Amount of energy it takes a ship to place a single L1 mine
    uint16_t MineFireEnergyUpgrade;     // 0026 All:LandmineFireEnergyUpgrade:::Extra amount of energy it takes to place an upgraded landmine.  ie. L2 = LandmineFireEnergy+LandmineFireEnergyUpgrade
    uint16_t BulletSpeed;               // 0028 All:BulletSpeed:::How fast bullets travel
    uint16_t BombSpeed;                 // 0030 All:BombSpeed:::How fast bombs travel

    uint32_t SeeBombLevel : 2;          // 0032 All:SeeBombLevel:0:4:If ship can see bombs on radar (0=Disabled, 1=All, 2=L2 and up, 3=L3 and up, 4=L4 bombs only) [Continuum .36]
    uint32_t DisableFastBombs : 1;      // 0032 All:DisableFastShooting:0:1:If firing bullets, bombs, or thors is disabled after using afterburners (1=enabled) [Continuum .36]
    uint32_t Radius : 7;                // 0032 All:Radius:::The ship's radius from center to outside, in pixels. Standard value is 14 pixels. [Continuum .37]
    uint32_t pack0 : 6;                 // 0033 Unused (fixed/updated to whatever is current by Niadh@columbus.rr.com)
    uint32_t MultiFireAngle : 16;       // 0034 All:MultiFireAngle:::Angle spread between multi-fire bullets and standard forward firing bullets. (111 = 1 degree, 1000 = 1 ship-rotation-point)

    uint16_t CloakEnergy;               // 0036 All:CloakEnergy:0:32000:Amount of energy required to have 'Cloak' activated (thousanths per hundredth of a second)
    uint16_t StealthEnergy;             // 0038 All:StealthEnergy:0:32000:Amount of energy required to have 'Stealth' activated (thousanths per hundredth of a second)
    uint16_t AntiWarpEnergy;            // 0040 All:AntiWarpEnergy:0:32000:Amount of energy required to have 'Anti-Warp' activated (thousanths per hundredth of a second)
    uint16_t XRadarEnergy;              // 0042 All:XRadarEnergy:0:32000:Amount of energy required to have 'X-Radar' activated (thousanths per hundredth of a second)
    uint16_t MaximumRotation;           // 0044 All:MaximumRotation:::Maximum rotation rate of the ship (0 = can't rotate, 400 = full rotation in 1 second)
    uint16_t MaximumThrust;             // 0046 All:MaximumThrust:::Maximum thrust of ship (0 = none)
    uint16_t MaximumSpeed;              // 0048 All:MaximumSpeed:::Maximum speed of ship (0 = can't move)
    uint16_t MaximumRecharge;           // 0050 All:MaximumRecharge:::Maximum recharge rate, or how quickly this ship recharges its energy.
    uint16_t MaximumEnergy;             // 0052 All:MaximumEnergy:::Maximum amount of energy that the ship can have.
    uint16_t InitialRotation;           // 0054 All:InitialRotation:::Initial rotation rate of the ship (0 = can't rotate, 400 = full rotation in 1 second)
    uint16_t InitialThrust;             // 0056 All:InitialThrust:::Initial thrust of ship (0 = none)
    uint16_t InitialSpeed;              // 0058 All:InitialSpeed:::Initial speed of ship (0 = can't move)
    uint16_t InitialRecharge;           // 0060 All:InitialRecharge:::Initial recharge rate, or how quickly this ship recharges its energy.
    uint16_t InitialEnergy;             // 0062 All:InitialEnergy:::Initial amount of energy that the ship can have.
    uint16_t UpgradeRotation;           // 0064 All:UpgradeRotation:::Amount added per 'Rotation' Prize
    uint16_t UpgradeThrust;             // 0066 All:UpgradeThrust:::Amount added per 'Thruster' Prize
    uint16_t UpgradeSpeed;              // 0068 All:UpgradeSpeed:::Amount added per 'Speed' Prize
    uint16_t UpgradeRecharge;           // 0070 All:UpgradeRecharge:::Amount added per 'Recharge Rate' Prize
    uint16_t UpgradeEnergy;             // 0072 All:UpgradeEnergy:::Amount added per 'Energy Upgrade' Prize
    uint16_t AfterburnerEnergy;         // 0074 All:AfterburnerEnergy:::Amount of energy required to have 'Afterburners' activated.
    uint16_t BombThrust;                // 0076 All:BombThrust:::Amount of back-thrust you receive when firing a bomb.
    uint16_t BurstSpeed;                // 0078 All:BurstSpeed:::How fast the burst shrapnel is for this ship.
    uint16_t TurretThrustPenalty;       // 0080 All:TurretThrustPenalty:::Amount the ship's thrust is decreased with a turret riding
    uint16_t TurretSpeedPenalty;        // 0082 All:TurretSpeedPenalty:::Amount the ship's speed is decreased with a turret riding
    uint16_t BulletFireDelay;           // 0084 All:BulletFireDelay:::delay that ship waits after a bullet is fired until another weapon may be fired (in hundredths of a second)
    uint16_t MultiFireDelay;            // 0086 All:MultiFireDelay:::delay that ship waits after a multifire bullet is fired until another weapon may be fired (in hundredths of a second)
    uint16_t BombFireDelay;             // 0088 All:BombFireDelay:::delay that ship waits after a bomb is fired until another weapon may be fired (in hundredths of a second)
    uint16_t LandmineFireDelay;         // 0090 All:LandmineFireDelay:::delay that ship waits after a mine is fired until another weapon may be fired (in hundredths of a second)
    uint16_t RocketTime;                // 0092 All:RocketTime:::How long a Rocket lasts (in hundredths of a second)
    uint16_t InitialBounty;             // 0094 All:InitialBounty:::Number of 'Greens' given to ships when they start
    uint16_t DamageFactor;              // 0096 All:DamageFactor:::How likely a the ship is to take damamage (ie. lose a prize) (0=special-case-never, 1=extremely likely, 5000=almost never)
    uint16_t PrizeShareLimit;           // 0098 All:PrizeShareLimit:::Maximum bounty that ships receive Team Prizes
    uint16_t AttachBounty;              // 0100 All:AttachBounty:::Bounty required by ships to attach as a turret
    uint16_t SoccerThrowTime;           // 0102 All:SoccerThrowTime:::Time player has to carry soccer ball (in hundredths of a second)
    uint16_t SoccerBallFriction;        // 0104 All:SoccerBallFriction:::Amount the friction on the soccer ball (how quickly it slows down -- higher numbers mean faster slowdown)
    uint16_t SoccerBallProximity;       // 0106 All:SoccerBallProximity:::How close the player must be in order to pick up ball (in pixels)
    uint16_t SoccerBallSpeed;           // 0108 All:SoccerBallSpeed:::Initial speed given to the ball when fired by the carrier.

    uint8_t TurretLimit;                // 0110 All:TurretLimit:::Number of turrets allowed on a ship.
    uint8_t BurstShrapnel;              // 0111 All:BurstShrapnel:::Number of bullets released when a 'Burst' is activated
    uint8_t MaxMines;                   // 0112 All:MaxMines:::Maximum number of mines allowed in ships
    uint8_t RepelMax;                   // 0113 All:RepelMax:::Maximum number of Repels allowed in ships
    uint8_t BurstMax;                   // 0114 All:BurstMax:::Maximum number of Bursts allowed in ships
    uint8_t DecoyMax;                   // 0115 All:DecoyMax:::Maximum number of Decoys allowed in ships
    uint8_t ThorMax;                    // 0116 All:ThorMax:::Maximum number of Thor's Hammers allowed in ships
    uint8_t BrickMax;                   // 0117 All:BrickMax:::Maximum number of Bricks allowed in ships
    uint8_t RocketMax;                  // 0118 All:RocketMax:::Maximum number of Rockets allowed in ships
    uint8_t PortalMax;                  // 0119 All:PortalMax:::Maximum number of Portals allowed in ships
    uint8_t InitialRepel;               // 0120 All:InitialRepel:::Initial number of Repels given to ships when they start
    uint8_t InitialBurst;               // 0121 All:InitialBurst:::Initial number of Bursts given to ships when they start
    uint8_t InitialBrick;               // 0122 All:InitialBrick:::Initial number of Bricks given to ships when they start
    uint8_t InitialRocket;              // 0123 All:InitialRocket:::Initial number of Rockets given to ships when they start
    uint8_t InitialThor;                // 0124 All:InitialThor:::Initial number of Thor's Hammers given to ships when they start
    uint8_t InitialDecoy;               // 0125 All:InitialDecoy:::Initial number of Decoys given to ships when they start
    uint8_t InitialPortal;              // 0126 All:InitialPortal:::Initial number of Portals given to ships when they start
    uint8_t BombBounceCount;            // 0127 All:BombBounceCount:::Number of times a ship's bombs bounce before they explode on impact

    uint32_t ShrapnelMax : 5;           // 0128 All:ShrapnelMax:0:31:Maximum amount of shrapnel released from a ship's bomb
    uint32_t ShrapnelRate : 5;          // 0128 All:ShrapnelRate:0:31:Amount of additional shrapnel gained by a 'Shrapnel Upgrade' prize.
    uint32_t CloakStatus : 2;           // 0129 All:CloakStatus:0:2:Whether ships are allowed to receive 'Cloak' 0=no 1=yes 2=yes/start-with
    uint32_t StealthStatus : 2;         // 0129 All:StealthStatus:0:2:Whether ships are allowed to receive 'Stealth' 0=no 1=yes 2=yes/start-with
    uint32_t XRadarStatus : 2;          // 0129 All:XRadarStatus:0:2:Whether ships are allowed to receive 'X-Radar' 0=no 1=yes 2=yes/start-with
    uint32_t AntiwarpStatus : 2;        // 0130 All:AntiWarpStatus:0:2:Whether ships are allowed to receive 'Anti-Warp' 0=no 1=yes 2=yes/start-with
    uint32_t InitialGuns : 2;           // 0130 All:InitialGuns:0:3:Initial level a ship's guns fire 0=no guns
    uint32_t MaxGuns : 2;               // 0130 All:MaxGuns:0:3:Maximum level a ship's guns can fire 0=no guns
    uint32_t InitialBombs : 2;          // 0130 All:InitialBombs:0:3:Initial level a ship's bombs fire 0=no bombs
    uint32_t MaxBombs : 2;              // 0131 All:MaxBombs:0:3:Maximum level a ship's bombs can fire 0=no bombs
    uint32_t DoubleBarrel : 1;          // 0131 All:DoubleBarrel:0:1:Whether ships fire with double barrel bullets 0=no 1=yes
    uint32_t EmpBomb : 1;               // 0131 All:EmpBomb:0:1:Whether ships fire EMP bombs 0=no 1=yes
    uint32_t SeeMines : 1;              // 0131 All:SeeMines:0:1:Whether ships see mines on radar 0=no 1=yes
    uint32_t UNKNOWN2 : 3;              // 0131 ?

    uint8_t UNKNOWN3[16];               // 0132 ?
};


/// <summary>
/// Prize settings.
/// </summary>
export struct PrizeSettings             // 28 uint8_ts wide
{
    uint8_t Recharge;                   // 1400 [fixed] PrizeWeight:QuickCharge:::Likelyhood of 'Recharge' prize appearing
    uint8_t Energy;                     // 1401 PrizeWeight:Energy:::Likelyhood of 'Energy Upgrade' prize appearing
    uint8_t Rotation;                   // 1402 PrizeWeight:Rotation:::Likelyhood of 'Rotation' prize appearing
    uint8_t Stealth;                    // 1403 PrizeWeight:Stealth:::Likelyhood of 'Stealth' prize appearing
    uint8_t Cloak;                      // 1404 PrizeWeight:Cloak:::Likelyhood of 'Cloak' prize appearing
    uint8_t XRadar;                     // 1405 PrizeWeight:XRadar:::Likelyhood of 'XRadar' prize appearing
    uint8_t Warp;                       // 1406 PrizeWeight:Warp:::Likelyhood of 'Warp' prize appearing
    uint8_t Gun;                        // 1407 PrizeWeight:Gun:::Likelyhood of 'Gun Upgrade' prize appearing
    uint8_t Bomb;                       // 1408 PrizeWeight:Bomb:::Likelyhood of 'Bomb Upgrade' prize appearing
    uint8_t BouncingBullets;            // 1409 PrizeWeight:BouncingBullets:::Likelyhood of 'Bouncing Bullets' prize appearing
    uint8_t Thruster;                   // 1410 PrizeWeight:Thruster:::Likelyhood of 'Thruster' prize appearing
    uint8_t TopSpeed;                   // 1411 PrizeWeight:TopSpeed:::Likelyhood of 'Speed' prize appearing
    uint8_t QuickCharge;                // 1412 [fixed] PrizeWeight:Recharge:::Likelyhood of 'Full Charge' prize appearing (NOTE! This is FULL CHARGE, not Recharge!! stupid vie)
    uint8_t Glue;                       // 1413 PrizeWeight:Glue:::Likelyhood of 'Engine Shutdown' prize appearing
    uint8_t MultiFire;                  // 1414 PrizeWeight:MultiFire:::Likelyhood of 'MultiFire' prize appearing
    uint8_t Proximity;                  // 1415 PrizeWeight:Proximity:::Likelyhood of 'Proximity Bomb' prize appearing
    uint8_t AllWeapons;                 // 1416 PrizeWeight:AllWeapons:::Likelyhood of 'Super!' prize appearing
    uint8_t Shields;                    // 1417 PrizeWeight:Shields:::Likelyhood of 'Shields' prize appearing
    uint8_t Shrapnel;                   // 1418 PrizeWeight:Shrapnel:::Likelyhood of 'Shrapnel Upgrade' prize appearing
    uint8_t AntiWarp;                   // 1419 PrizeWeight:AntiWarp:::Likelyhood of 'AntiWarp' prize appearing
    uint8_t Repel;                      // 1420 PrizeWeight:Repel:::Likelyhood of 'Repel' prize appearing
    uint8_t Burst;                      // 1421 PrizeWeight:Burst:::Likelyhood of 'Burst' prize appearing
    uint8_t Decoy;                      // 1422 PrizeWeight:Decoy:::Likelyhood of 'Decoy' prize appearing
    uint8_t Thor;                       // 1423 PrizeWeight:Thor:::Likelyhood of 'Thor' prize appearing
    uint8_t MultiPrize;                 // 1424 PrizeWeight:MultiPrize:::Likelyhood of 'Multi-Prize' prize appearing
    uint8_t Brick;                      // 1425 PrizeWeight:Brick:::Likelyhood of 'Brick' prize appearing
    uint8_t Rocket;                     // 1426 PrizeWeight:Rocket:::Likelyhood of 'Rocket' prize appearing
    uint8_t Portal;                     // 1427 PrizeWeight:Portal:::Likelyhood of 'Portal' prize appearing
};


/// <summary>
/// Arena settings.
/// </summary>
export struct ArenaSettings             // 1428 uint8_ts wide
{
    uint32_t Version : 8;               // 0000 VIE:15
    uint32_t ExactDamage : 1;           // 0001 Bullet:ExactDamage:0:1:If damage is to be random or not (1=exact, 0=random) [Continuum .36]
    uint32_t HideFlags : 1;             // 0001 Spectator:HideFlags:0:1:If flags are to be shown to specs when they are dropped (1=can't see them) [Continuum .36]
    uint32_t NoXRadar : 1;              // 0001 Spectator:NoXRadar:0:1:If specs are allowed to have X (0=yes, 1=no) [Continuum .36]
    uint32_t pack0 : 21;                // 0001 (14)

    ShipSettings ships[8];              // 0004 See shipSettings declaration...

    uint32_t BulletDamageLevel;         // 1156 [BulletDamageLevel * 1000] Bullet:BulletDamageLevel:::Maximum amount of damage that a L1 bullet will cause. Formula; damage = squareroot(rand# * (max damage^2 + 1))
    uint32_t BombDamageLevel;           // 1160 [BombDamageLevel * 1000] Bomb:BombDamageLevel:::Amount of damage a bomb causes at its center point (for all bomb levels)
    uint32_t BulletAliveTime;           // 1164 Bullet:BulletAliveTime:::How long bullets live before disappearing (in hundredths of a second)
    uint32_t BombAliveTime;             // 1168 Bomb:BombAliveTime:::Time bomb is alive (in hundredths of a second)
    uint32_t DecoyAliveTime;            // 1172 Misc:DecoyAliveTime:::Time a decoy is alive (in hundredths of a second)
    uint32_t SafetyLimit;               // 1176 Misc:SafetyLimit:::Amount of time that can be spent in the safe zone. (90000 = 15 mins)
    uint32_t FrequencyShift;            // 1180 Misc:FrequencyShift:0:10000:Amount of random frequency shift applied to sounds in the game.
    uint32_t MaxFrequency;              // 1184 Team:MaxFrequency:::Maximum number of frequencies allowed in arena (5 would allow frequencies 0,1,2,3,4)
    uint32_t RepelSpeed;                // 1188 Repel:RepelSpeed:::Speed at which players are repelled
    uint32_t MineAliveTime;             // 1192 Mine:MineAliveTime:0:60000:Time that mines are active (in hundredths of a second)
    uint32_t BurstDamageLevel;          // 1196 [BurstDamageLevel * 1000] Burst:BurstDamageLevel:::Maximum amount of damage caused by a single burst bullet.
    uint32_t BulletDamageUpgrade;       // 1200 [BulletDamageUpgrade * 1000] Bullet:BulletDamageUpgrade:::Amount of extra damage each bullet level will cause
    uint32_t FlagDropDelay;             // 1204 Flag:FlagDropDelay:::Time before flag is dropped by carrier (0=never)
    uint32_t EnterGameFlaggingDelay;    // 1208 Flag:EnterGameFlaggingDelay:::Time a new player must wait before they are allowed to see flags
    uint32_t RocketThrust;              // 1212 Rocket:RocketThrust:::Thrust value given while a rocket is active.
    uint32_t RocketSpeed;               // 1216 Rocket:RocketSpeed:::Speed value given while a rocket is active.
    uint32_t InactiveShrapnelDamage;    // 1220 [InactiveShrapnelDamage * 1000] Shrapnel:InactiveShrapDamage:::Amount of damage shrapnel causes in it's first 1/4 second of life.
    uint32_t WormholeSwitchTime;        // 1224 Wormhole:SwitchTime:::How often the wormhole getSwitches its destination.
    uint32_t ActivateAppShutdownTime;   // 1228 Misc:ActivateAppShutdownTime:::Amount of time a ship is shutdown after application is reactivated (ie. when they come back from windows mode)
    uint32_t ShrapnelSpeed;             // 1232 Shrapnel:ShrapnelSpeed:::Speed that shrapnel travels

    uint8_t UNKNOWN0[16];               // 1236 ?

    uint16_t SendRoutePercent;          // 1252 Latency:SendRoutePercent:300:800:Percentage of the ping time that is spent on the ClientToServer portion of the ping. (used in more accurately syncronizing clocks)
    uint16_t BombExplodeDelay;          // 1254 Bomb:BombExplodeDelay:::How long after the proximity sensor is triggered before bomb explodes. (note: it explodes immediately if ship moves away from it after triggering it)
    uint16_t SendPositionDelay;         // 1256 Misc:SendPositionDelay:0:20:Amount of time between position packets sent by client.
    uint16_t BombExplodePixels;         // 1258 Bomb:BombExplodePixels:::Blast radius in pixels for an L1 bomb (L2 bombs double this, L3 bombs triple this)
    uint16_t DeathPrizeTime;            // 1260 Prize:DeathPrizeTime:::How long the prize exists that appears after killing somebody.
    uint16_t JitterTime;                // 1262 Bomb:JitterTime:::How long the screen jitters from a bomb hit. (in hundredths of a second)
    uint16_t EnterDelay;                // 1264 Kill:EnterDelay:::How long after a player dies before he can re-enter the game.
    uint16_t EngineShutdownTime;        // 1266 Prize:EngineShutdownTime:::Time the player is affected by an 'Engine Shutdown' Prize (in hundredth of a second)
    uint16_t ProximityDistance;         // 1268 Bomb:ProximityDistance:::Radius of proximity trigger in tiles.  Each bomb level adds 1 to this amount.
    uint16_t BountyIncreaseForKill;     // 1270 Kill:BountyIncreaseForKill:::Number of points added to players bounty each time he kills an opponent.
    uint16_t BounceFactor;              // 1272 Misc:BounceFactor:::How bouncy the walls are (16=no-speed-loss)
    uint16_t MapZoomFactor;             // 1274 Radar:MapZoomFactor:8:1000:A number representing how far you can see on radar.
    uint16_t MaxBonus;                  // 1276 Kill:MaxBonus:::Let's ignore these for now. Or let's not. :) This is if you have flags, can add more points per a kill. Founded by MGB
    uint16_t MaxPenalty;                // 1278 Kill:MaxPenalty:::Let's ignore these for now. Or let's not. :) This is if you have flags, can take away points per a kill. Founded by MGB
    uint16_t RewardBase;                // 1280 Kill:RewardBase:::Let's ignore these for now. Or let's not. :) This is shown added to a person's bty, but isn't added from points for a kill. Founded by MGB
    uint16_t RepelTime;                 // 1282 Repel:RepelTime:::Time players are affected by the repel (in hundredths of a second)
    uint16_t RepelDistance;             // 1284 Repel:RepelDistance:::Number of pixels from the player that are affected by a repel.
    uint16_t HelpTickerDelay;           // 1286 Misc:TickerDelay:::Amount of time between ticker help messages.
    uint16_t FlaggerOnRadar;            // 1288 Flag:FlaggerOnRadar:::Whether the flaggers appear on radar in red 0=no 1=yes
    uint16_t FlaggerKillMultiplier;     // 1290 Flag:FlaggerKillMultiplier:::Number of times more points are given to a flagger (1 = double points, 2 = triple points)
    uint16_t PrizeFactor;               // 1292 Prize:PrizeFactor:::Number of prizes hidden is based on number of players in game.  This number adjusts the formula, higher numbers mean more prizes. (*Note: 10000 is max, 10 greens per person)
    uint16_t PrizeDelay;                // 1294 Prize:PrizeDelay:::How often prizes are regenerated (in hundredths of a second)
    uint16_t PrizeMinimumVirtual;       // 1296 Prize:MinimumVirtual:::Distance from center of arena that prizes/flags/soccer-balls will generate
    uint16_t PrizeUpgradeVirtual;       // 1298 Prize:UpgradeVirtual:::Amount of additional distance added to MinimumVirtual for each player that is in the game.
    uint16_t PrizeMaxExist;             // 1300 Prize:PrizeMaxExist:::Maximum amount of time that a hidden prize will remain on screen. (actual time is random)
    uint16_t PrizeMinExist;             // 1302 Prize:PrizeMinExist:::Minimum amount of time that a hidden prize will remain on screen. (actual time is random)
    uint16_t PrizeNegativeFactor;       // 1304 Prize:PrizeNegativeFactor:::Odds of getting a negative prize.  (1 = every prize, 32000 = extremely rare)
    uint16_t DoorDelay;                 // 1306 Door:DoorDelay:::How often doors attempt to switch their state.
    uint16_t AntiwarpPixels;            // 1308 Toggle:AntiWarpPixels:::Distance Anti-Warp affects other players (in pixels) (note: enemy must also be on radar)
    int16_t DoorMode;                   // 1310 Door:DoorMode:::Door mode (-2=all doors completely random, -1=weighted random (some doors open more often than others), 0-255=fixed doors (1 bit of uint8_t for each door specifying whether it is open or not)
    uint16_t FlagBlankDelay;            // 1312 Flag:FlagBlankDelay:::Amount of time that a user can get no data from server before flags are hidden from view for 10 seconds.
    uint16_t NoDataFlagDropDelay;       // 1314 Flag:NoDataFlagDropDelay:::Amount of time that a user can get no data from server before flags he is carrying are dropped.
    uint16_t MultiPrizeCount;           // 1316 Prize:MultiPrizeCount:::Number of random 'Greens' given with a 'MultiPrize'
    uint16_t BrickTime;                 // 1318 Brick:BrickTime:::How long bricks last (in hundredths of a second)
    uint16_t WarpRadiusLimit;           // 1320 Misc:WarpRadiusLimit:::When ships are randomly placed in the arena, this parameter will limit how far from the center of the arena they can be placed (1024=anywhere)
    uint16_t EBombShutdownTime;         // 1322 Bomb:EBombShutdownTime:::Maximum time recharge is stopped on players hit with an EMP bomb.
    uint16_t EBombDamagePercent;        // 1324 Bomb:EBombDamagePercent:::Percentage of normal damage applied to an EMP bomb 0=0% 1000=100% 2000=200%
    uint16_t RadarNeutralSize;          // 1326 Radar:RadarNeutralSize:0:1024:Size of area between blinded radar zones (in pixels)
    uint16_t WarpPointDelay;            // 1328 Misc:WarpPointDelay:::How long a Portal point is active.
    uint16_t NearDeathLevel;            // 1330 Misc:NearDeathLevel:::Amount of energy that constitutes a near-death experience (ships bounty will be decreased by 1 when this occurs -- used for dueling zone)
    uint16_t BBombDamagePercent;        // 1332 Bomb:BBombDamagePercent:::Percentage of normal damage applied to a bouncing bomb 0=0% 1000=100% 2000=200%
    uint16_t ShrapnelDamagePercent;     // 1334 Shrapnel:ShrapnelDamagePercent:::Percentage of normal damage applied to shrapnel (relative to bullets of same level) 0=0% 1000=100% 2000=200%
    uint16_t ClientSlowPacketTime;      // 1336 Latency:ClientSlowPacketTime:20:200:Amount of latency S2C that constitutes a slow packet.
    uint16_t FlagDropResetReward;       // 1338 Flag:FlagDropResetReward:::Minimum kill reward that a player must get in order to have his flag drop timer reset.
    uint16_t FlaggerFireCostPercent;    // 1340 Flag:FlaggerFireCostPercent:::Percentage of normal weapon firing cost for flaggers 0=Super 1000=100% 2000=200%
    uint16_t FlaggerDamagePercent;      // 1342 Flag:FlaggerDamagePercent:::Percentage of normal damage received by flaggers 0=Invincible 1000=100% 2000=200%
    uint16_t FlaggerBombFireDelay;      // 1344 Flag:FlaggerBombFireDelay:::Delay given to flaggers for firing bombs (0=ships normal firing rate -- note: please do not set this number less than 20)
    uint16_t SoccerPassDelay;           // 1346 Soccer:PassDelay:0:10000:How long after the ball is fired before anybody can pick it up (in hundredths of a second)
    uint16_t SoccerBallBlankDelay;      // 1348 Soccer:BallBlankDelay:::Amount of time a player can receive no data from server and still pick up the soccer ball.
    uint16_t S2CNoDataKickoutDelay;     // 1350 Latency:S2CNoDataKickoutDelay:100:32000:Amount of time a user can receive no data from server before connection is terminated.
    uint16_t FlaggerThrustAdjustment;   // 1352 Flag:FlaggerThrustAdjustment:::Amount of thrust adjustment player carrying flag gets (negative numbers mean less thrust)
    uint16_t FlaggerSpeedAdjustment;    // 1354 Flag:FlaggerSpeedAdjustment:::Amount of speed adjustment player carrying flag gets (negative numbers mean slower)
    uint16_t CliSlowPacketSampleSize;   // 1356 Latency:ClientSlowPacketSampleSize:50:1000:Number of packets to sample S2C before checking for kickout.

    uint8_t UNKNOWN1[10];               // 1358 ?

    uint8_t RandomShrapnel;             // 1368 Shrapnel:Random:0:1:Whether shrapnel spreads in circular or random patterns 0=circular 1=random
    uint8_t SoccerBallBounce;           // 1369 Soccer:BallBounce:0:1:Whether the ball bounces off walls (0=ball go through walls, 1=ball bounces off walls)
    uint8_t SoccerAllowBombs;           // 1370 Soccer:AllowBombs:0:1:Whether the ball carrier can fire his bombs (0=no 1=yes)
    uint8_t SoccerAllowGuns;            // 1371 Soccer:AllowGuns:0:1:Whether the ball carrier can fire his guns (0=no 1=yes)
    uint8_t SoccerMode;                 // 1372 Soccer:Mode:0:6:Goal configuration (0=any goal, 1=left-half/right-half, 2=top-half/bottom-half, 3=quadrants-defend-one-goal, 4=quadrants-defend-three-goals, 5=sides-defend-one-goal, 6=sides-defend-three-goals)
    uint8_t MaxPerTeam;                 // 1373 Team:MaxPerTeam:::Maximum number of players on a non-private frequency
    uint8_t MaxPerPrivateTeam;          // 1374 Team:MaxPerPrivateTeam:::Maximum number of players on a private frequency (0=same as MaxPerTeam)
    uint8_t TeamMaxMines;               // 1375 Mine:TeamMaxMines:0:32000:Maximum number of mines allowed to be placed by an entire team
    uint8_t WormholeGravityBombs;       // 1376 Wormhole:GravityBombs:0:1:Whether a wormhole affects bombs (0=no 1=yes)
    uint8_t BombSafety;                 // 1377 Bomb:BombSafety:0:1:Whether proximity bombs have a firing safety (0=no 1=yes).  If enemy ship is within proximity radius, will it allow you to fire.
    uint8_t MessageReliable;            // 1378 Message:MessageReliable:0:1:Whether messages are sent reliably.
    uint8_t TakePrizeReliable;          // 1379 Prize:TakePrizeReliable:0:1:Whether prize packets are sent reliably (C2S)
    uint8_t AllowAudioMessages;         // 1380 Message:AllowAudioMessages:0:1:Whether players can send audio messages (0=no 1=yes)
    uint8_t PrizeHideCount;             // 1381 Prize:PrizeHideCount:::Number of prizes that are regenerated every PrizeDelay.
    uint8_t ExtraPositionData;          // 1382 Misc:ExtraPositionData:0:1:Whether regular players receive sysop data about a ship (leave this at zero)
    uint8_t SlowFrameCheck;             // 1383 Misc:SlowFrameCheck:0:1:Whether to check for slow frames on the client (possible cheat technique) (flawed on some machines, do not use)
    uint8_t CarryFlags;                 // 1384 Flag:CarryFlags:0:2:Whether the flags can be picked up and carried (0=no, 1=yes, 2=yes-one at a time)
    uint8_t AllowSavedShip;             // 1385 Misc:AllowSavedShips:0:1:Whether saved ships are allowed (do not allow saved ship in zones where sub-arenas may have differing parameters) 1 = Savedfrom last arena/lagout, 0 = New Ship when entering arena/zone
    uint8_t RadarMode;                  // 1386 Radar:RadarMode:0:4:Radar mode (0=normal, 1=half/half, 2=quarters, 3=half/half-see team mates, 4=quarters-see team mates)
    uint8_t VictoryMusic;               // 1387 Misc:VictoryMusic:0:1:Whether the zone plays victory music or not.
    uint8_t FlaggerGunUpgrade;          // 1388 Flag:FlaggerGunUpgrade:0:1:Whether the flaggers get a gun upgrade 0=no 1=yes
    uint8_t FlaggerBombUpgrade;         // 1389 Flag:FlaggerBombUpgrade:0:1:Whether the flaggers get a bomb upgrade 0=no 1=yes
    uint8_t SoccerUseFlagger;           // 1390 Soccer:UseFlagger:0:1:If player with soccer ball should use the Flag:Flagger* ship adjustments or not (0=no, 1=yes)
    uint8_t SoccerBallLocation;         // 1391 Soccer:BallLocation:0:1:Whether the balls location is displayed at all times or not (0=not, 1=yes)

    uint8_t UNKNOWN2[8];                // 1392 ?

    PrizeSettings pw;                   // 1400 See prizeSettings declaration...
};
