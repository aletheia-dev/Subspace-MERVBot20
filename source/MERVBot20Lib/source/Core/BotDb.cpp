module;
#include <fstream>
#include <filesystem>

module BotDb;

import Algorithms;
import Command;
import Config;
import Settings;
import Sockets;
import System;

import <iostream>;
import <format>;


//////// Spawns ////////

void BotDatabase::gotSpawn(std::string_view line)
{
    std::string text{ line };
    std::string name{ splitFirst(text, ':') };
    std::string pass{ splitFirst(text, ':') };
    std::string arena{ splitFirst(text, ':') };
    std::string dlls{ splitFirst(text, ':') };
    std::string staff{ splitFirst(text, ':') };
    std::string params = text;

    botInfo.setLogin(name, pass, staff);
    botInfo.setSpawn(dlls);
    botInfo.setArena(arena, Ship::Spectator, resX, resY, playerVoices);
    botInfo.setParams(params);

    // create the spawn
    getHostList().connectHost(botInfo);
}


void BotDatabase::loadSpawns()
{
    readDataLines(spawnsFile, &BotDatabase::gotSpawn, this);
}


//////// Operators ////////

OpEntry* BotDatabase::findOperator(const std::string_view name)
{
    for (OpEntry& opEntry : operators) {
        if (opEntry.validateName(name)) {
            return &opEntry;
        }
    }
    return nullptr;
}


OpEntry& BotDatabase::addOperator(const std::string_view name, const std::string_view pass, OperatorLevel access)
{
    std::list<OpEntry>::iterator opIt{ operators.begin() };

    while (opIt != operators.end()) {
        if (opIt->getAccess() < access) {
            operators.insert(opIt, OpEntry(name, pass, access));
            operatorsUpdated = true;

            return operators.back();
        }
        opIt++;
    }

    operators.push_back(OpEntry(name, pass, access));
    operatorsUpdated = true;

    return operators.back();
}


OpEntry& BotDatabase::addOperator(const std::string_view name, OperatorLevel level)
{
    return addOperator(name, "", level);
}


bool BotDatabase::removeOperator(const std::string_view name)
{
    OpEntry* op = findOperator(name);

    if (op == nullptr) {
        return false;
    }

    operators.remove(*op);
    operatorsUpdated = true;

    return true;
}


void BotDatabase::gotOperator(std::string_view line)
{
    std::string text{ line };
    int lvl{ stoi(splitFirst(text, ':')) };

    OperatorLevel level{ OperatorLevel::Player };
    std::string levelDesc{ getLevelDescription(OperatorLevel::Unknown) };

    if (lvl >= (int)OperatorLevel::Limited && lvl <= (int)OperatorLevel::God) {
        level = (OperatorLevel)lvl;
        levelDesc = getLevelDescription(level);
    }

    std::string name{ splitFirst(text, ':') };
    std::string pass{ splitFirst(text, ':') };

    if (level >= OperatorLevel::Limited && level <= OperatorLevel::God && !invalidName(name)) {
        OpEntry *op = findOperator(name);

        if (!op) {
            addOperator(name, pass, level);

            if (!pass.empty())
                std::cout << format("Added {}-level operator: {} ({})\n", levelDesc, name, pass);
            else
                std::cout << format("Added {}-level operator: {}\n", levelDesc, name);
        }
        else {
            op->setAccess(level);
            op->setPassword(pass);
        }

        if (!findOperator(defaultName))
            addOperator(defaultName);
    }
    else {
        std::cout << format("Invalid {}-level operator ignored: {} ({})\n", levelDesc, name, pass);
    }
}


void BotDatabase::loadOperators()
{
    readDataLines(operatorsFile, &BotDatabase::gotOperator, this);
    operatorsUpdated = false;
}


void BotDatabase::saveOperators()
{
    if (!operatorsUpdated)
        return;

    std::ofstream file(operatorsFile);

    if (!file.good()) {
        std::cout << "WARNING: Unable to overwrite {} for Operators database save\n" << operatorsFile;
        return;
    }
    std::cout << "Saving operators database... ";

    for (OpEntry& opEntry : operators) {
        if (opEntry.getName() != defaultName) {
            file << int32_t(opEntry.getAccess()) << ":" << opEntry.getName() << ":" << opEntry.getPassword() << "\r\n";
        }
    }
    operatorsUpdated = false;
    std::cout << "Completed.\n";
}


//////// INI ////////

BotDatabase::BotDatabase()
{
    // Retrieve path
    iniFilePath = (std::filesystem::current_path() / IniFileName).string();

    lastSave = getTickCount();
    saveInterval = 30000;
    
    defaultName = std::format("default {}", toLower(getLevelDescription(OperatorLevel::Unknown)));
    operatorsUpdated = false;

    // load INI file
    reloadIniFile(true);
    // load operators
    loadOperators();
    // load aliases
    getAliasList().loadAliases(commandsFile);
}


void BotDatabase::reloadIniFile(bool doLogin)
{
    // [Login]
    if (doLogin) {
        std::pair<std::string, uint16_t> ipPort;

        readConfigParam("Login", "Zone", ipPort, iniFilePath, "127.0.0.1:2000");
        loginIP = resolveHostname(ipPort.first);
        loginPort = ipPort.second;
    }
    readConfigParam("Login", "RecordLogins", recordLogins, iniFilePath, "1");
    readConfigParam("Login", "ResX", resX, iniFilePath, "1280");
    readConfigParam("Login", "ResY", resY, iniFilePath, "1024");
    
    // [Database]
    readConfigParam("Database", "Spawns", spawnsFile, iniFilePath, "Spawns.txt");
    readConfigParam("Database", "Commands", commandsFile, iniFilePath, "Commands.txt");
    readConfigParam("Database", "Operators", operatorsFile, iniFilePath, "Operators.txt");
    readConfigParam("Database", "SaveInterval", saveInterval, iniFilePath, "300");
    saveInterval *= 100;

    // [Misc]
    readConfigParam("Misc", "WindowCaption", windowCaption, iniFilePath, "MERVBot20 Terminal");
    readConfigParam("Misc", "MaxSpawns", maxSpawns, iniFilePath, "3");
    readConfigParam("Misc", "MaskBan", maskBan, iniFilePath, "0");
    readConfigParam("Misc", "PlayerVoices", playerVoices, iniFilePath, "0");
    readConfigParam("Misc", "ForceContinuum", forceContinuum, iniFilePath, "0");
    readConfigParam("Misc", "RunSilent", runSilent, iniFilePath, "0");
    readConfigParam("Misc", "DisablePubCommands", disablePub, iniFilePath, "0");
    readConfigParam("Misc", "NoisySpectator", noisySpectator, iniFilePath, "0");
    readConfigParam("Misc", "MaxConnectionTries", maxTries, iniFilePath, "3");
    readConfigParam("Misc", "InitialChatChannels", initialChatChannels, iniFilePath, "");

    // [Security]
    readConfigParam("Security", "LogLevel", logLevel, iniFilePath, "0");
    readConfigParam("Security", "EncryptMode", encryptMode, iniFilePath, "1");
    readConfigParam("Security", "RemoteInterpreter", remoteInterpreter, iniFilePath, "1");
    readConfigParam("Security", "RemoteOperator", remoteOperator, iniFilePath, "1");
    readConfigParam("Security", "ChatterLog", chatterLog, iniFilePath, "0");

    // [Registration]
    readConfigParam("Registration", "Name", regName, iniFilePath, "Catid");
    readConfigParam("Registration", "EMail", regEMail, iniFilePath, "cat02e@fsu.edu");
    readConfigParam("Registration", "State", regState, iniFilePath, "California");
    readConfigParam("Registration", "Age", regAge, iniFilePath, "17");

    // Save INI settings
    botInfo.setLogin("UNKNOWN BOT ERROR", "AHH CRAP", "");
    botInfo.setZone(loginIP, loginPort);
    botInfo.setArena("0", Ship::Spectator, resX, resY, playerVoices);
    botInfo.setReg(regName, regEMail, "", regState, RegFormSex::Male, regAge, false, false, false);
    
    if (maskBan) {
        botInfo.maskBan();
    }
}


void BotDatabase::save()
{
    saveOperators();
    getAliasList().save(commandsFile);
}
