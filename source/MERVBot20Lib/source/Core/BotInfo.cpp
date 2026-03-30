module;
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

module BotInfo;

import Algorithms;
import BotDb;
import Encrypt;
import Sockets;
import System;

import <string>;


//////// Bot descriptor ////////

BotInfo::BotInfo()
{
    setLogin("Bobo the Bot", "+++++", "");
    setZone(resolveHostname("127.0.0.1"), 2000);
    setArena("0", Ship::Spectator, 1024, 768, false);
    resetSystemInfo();
    setReg("Catid.bot", "cat02e@fsu.edu", "CatVille", "CatState", RegFormSex::Male, 17, true, true, true);
    setSpawn("default.dll");
}


//BotInfo::BotInfo(BotInfo&a)
//{
//    set(a);
//}


//void BotInfo::operator=(BotInfo&a)
//{
//    set(a);
//}


//void BotInfo::set(BotInfo&a)
//{
//    setLogin(a.name, a.password, a.staffpw);
//    setZone(a.ip, a.port);
//    setArena(a.initialArena, a.initialShip, a.xres, a.yres, a.allowAudio);
//    setBan(a.machineID, a.timeZoneBias, a.permissionID, a.processor, a.regName, a.regOrg);
//    setReg(a.realName, a.email, a.city, a.state, a.sex, a.age, a.playAtHome, a.playAtWork, a.playAtSchool);
//    setSpawn(a.dllNames);
//}


void BotInfo::resetSystemInfo()
{
    // CPU type
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    processor = si.dwProcessorType;

    // Windows registration name
    {
        char buffer[40];
        HKEY key;            // Handle to a session with a registry key
        uint32_t buflen;        // Length of the buffer
        uint32_t type;        // Type will contain type of data transfered

        // Look up Windows NT version information
        RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", (HKEY*)&key);

        buflen = 40;
        RegQueryValueEx(key, "RegisteredOwner", NULL, (LPDWORD)&type, (BYTE*)&buffer, (LPDWORD)&buflen);
        regName = buffer;
//        strncpy_s(regName, buffer, 40);
        
        buflen = 40;
        RegQueryValueEx(key, "RegisteredOrganization", NULL, (LPDWORD)&type, (BYTE*)&buffer, (LPDWORD)&buflen);
        regOrg = buffer;
//        strncpy_s(regOrg, buffer, 40);

        RegCloseKey(key);
    }

    // Timezone Bias
    TIME_ZONE_INFORMATION tzi;
    GetTimeZoneInformation(&tzi);
    timeZoneBias = (SHORT)tzi.Bias;

    // Permission ID
    permissionID = getSetting32(HKEY_LOCAL_MACHINE, "SOFTWARE", "D2");

    // Install some SubSpace registry keys
    if (permissionID == -1) {
        do {
            permissionID = (GetTickCount() ^ 0xAAAAAAAA) * 0x5f346d + 0x5abcdef;
        }
        while (!permissionID || permissionID == 1 || permissionID == -1)
            ;

        setSetting32(HKEY_LOCAL_MACHINE, "SOFTWARE", "D2", permissionID);
    }
    
    // Machine ID
    GetVolumeInformation("C:\\", NULL, 0, (LPDWORD)&machineID, NULL, NULL, NULL, 0);

    if (!machineID || machineID == 1 || machineID == -1) {
        machineID = getSetting32(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "ProductCode");

        if (!machineID || machineID == 1 || machineID == -1) {
            machineID = permissionID;
            setSetting32(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "ProductCode", machineID);
        }
    }

    if (machineID > 0x7fffffff) {
        machineID += 0x7fffffff;
    }
    setSetting32(HKEY_LOCAL_MACHINE, "SOFTWARE", "D1", machineID);
}


void BotInfo::setSpawn(std::string_view ddllName)
{
    dllNames = ddllName;
}


void BotInfo::setParams(std::string_view pParams)
{
    params = pParams;
}


void BotInfo::setLogin(std::string_view nname, std::string_view ppassword, std::string_view sstaffpw)
{
    name = nname;
    password = ppassword;
    staffpw = sstaffpw;
}


void BotInfo::setZone(uint32_t iip, uint16_t pport)
{
    ip = iip;
    port = pport;
}


void BotInfo::setArena(std::string_view iinitialArena, Ship iinitialShip, uint16_t xxres, uint16_t yyres, bool aallowAudio)
{
    initialArena = iinitialArena;
    initialShip = iinitialShip;
    xres = xxres;
    yres = yyres;
    allowAudio = aallowAudio;
}


void BotInfo::setBan(uint32_t mmachineID, uint16_t ttimeZoneBias, uint32_t ppermissionID, uint32_t pprocessor, 
    std::string_view rregName, std::string_view rregOrg)
{
    machineID = mmachineID;
    timeZoneBias = ttimeZoneBias;
    permissionID = ppermissionID;
    processor = pprocessor;
    regName = rregName;
    regOrg = rregOrg;
    //strncpy_s(regName, rregName, 64);
    //strncpy_s(regOrg, rregOrg, 64);
}


void BotInfo::maskBan()
{
    ++machineID;
    ++permissionID;

    if (processor != 586) 
        processor = 586;

    uint32_t i;

    for (i = 0; i < regName.size(); ++i) {
        regName[i] = ROT13(regName[i]);
    }

    for (i = 0; i < regOrg.size(); ++i) {
        regOrg[i] = ROT13(regOrg[i]);
    }
}


void BotInfo::setReg(std::string_view rrealName, std::string_view eemail, std::string_view ccity, 
    std::string_view sstate, RegFormSex ssex, uint8_t aage, bool pplayAtHome, bool pplayAtWork, bool pplayAtSchool)
{
    realName = rrealName;
    email = eemail;
    city = ccity;
    state = sstate;
    //strncpy_s(realName, rrealName, 64);
    //strncpy_s(email, eemail, 64);
    //strncpy_s(city, ccity, 64);
    //strncpy_s(state, sstate, 64);
    sex = ssex;
    age = aage;
    playAtHome = pplayAtHome;
    playAtWork = pplayAtWork;
    playAtSchool = pplayAtSchool;
}
