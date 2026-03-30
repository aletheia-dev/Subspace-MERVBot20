export module BotInfo;

import ClientTypes;

import <string>;


export struct BotInfo
{
    std::string params;

    // Login
    std::string name;
    std::string password;
    std::string staffpw;
    uint32_t ip;
    uint16_t port;

    // Spawn
    std::string dllNames;

    // Database
    uint32_t maxSpawns;

    // Ban
    uint32_t machineID;
    uint16_t timeZoneBias;
    uint32_t permissionID;

    // Registration form
    std::string realName;
    std::string email;
    std::string city;
    std::string state;
    RegFormSex sex;
    uint8_t age;
    bool playAtHome;
    bool playAtWork;
    bool playAtSchool;
    uint32_t processor;
    std::string regName;
    std::string regOrg;

    // Arena
    Ship initialShip;
    std::string initialArena;
    uint16_t xres;
    uint16_t yres;
    bool allowAudio;

    BotInfo();

    void setParams(std::string_view pParams);

    void resetSystemInfo();

    void setLogin(std::string_view nname, std::string_view ppassword, std::string_view sstaffpw);

    void setZone(uint32_t iip, uint16_t pport);

    void setSpawn(std::string_view ddllName);

    void setArena(std::string_view iinitialArena, Ship iinitialShip, uint16_t xxres, uint16_t yyres,
        bool aallowAudio);

    void setBan(uint32_t mmachineID, uint16_t ttimeZoneBias, uint32_t ppermissionID, uint32_t pprocessor, 
        std::string_view rregName, std::string_view rregOrg);

    void maskBan();

    void setReg(std::string_view rrealName, std::string_view eemail, std::string_view ccity, std::string_view sstate, 
        RegFormSex ssex, uint8_t aage, bool pplayAtHome, bool pplayAtWork, bool pplayAtSchool);
};
