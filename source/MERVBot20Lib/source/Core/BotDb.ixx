export module BotDb;

import Algorithms;
import Command;
import Host;

import <list>;
import <string>;


export class OpEntry
{
public:
    OpEntry(std::string_view name, std::string_view pass, OperatorLevel access) 
        : m_name(name), m_pass(pass), m_access(access) {}

    bool operator == (const OpEntry&) const& = default;

    uint32_t getOverallCount() const {
        return m_failedAttempts;
    }

    void addCounter() {
        ++m_loginCount;
    }

    uint32_t getFailureCount() {
        return m_failedAttempts;
    }

    void addFailure() {
        ++m_failedAttempts;
    }

    OperatorLevel getAccess() const {
        return m_access;
    }

    void setAccess(OperatorLevel access) {
        m_access = access;
    };

    std::string_view getName() const {
        return m_name;
    }

    void setName(std::string_view name) {
        m_name = name;
    };

    bool validateName(std::string_view name) const {
        return toLower(m_name) == toLower(name);
    }

    const std::string_view getPassword() const {
        return m_pass;
    }

    void setPassword(std::string_view pass) {
        m_pass = pass;
    };

    bool validatePass(std::string_view pass) const {
        return pass.empty() || m_pass == pass ? true : false;
    }

private:
    uint32_t m_loginCount{};    // Overall count of logins, this instantiation
    uint32_t m_failedAttempts{};    // Count of failed logins, this instantiation
    OperatorLevel m_access;
    std::string m_name;
    std::string m_pass;
};


export struct BotDatabase
{
    //////// Database ////////

    std::string iniFilePath;
    
    BotDatabase();

    void save();

    uint64_t lastSave;  // in hundredths of a second

    //////// Parameters ////////

    BotInfo botInfo;  // this is a blue-print to be used for creating spawns! each Host has it's own copy of BotInfo
    
    void gotSpawn(std::string_view line);

    void loadSpawns();
    
    //////// Operators ////////

    std::list<OpEntry> operators;
    std::string defaultName;
    bool operatorsUpdated;
    
    OpEntry* findOperator(std::string_view name);

    OpEntry& addOperator(std::string_view name, OperatorLevel level = OperatorLevel::Unknown);

    OpEntry& addOperator(std::string_view name, std::string_view pass, OperatorLevel level);

    bool removeOperator(std::string_view name);
    
    void gotOperator(std::string_view line);

    void loadOperators();

    void saveOperators();
    
    //////// INI ////////

    std::string regName;
    std::string regEMail;
    std::string regState;
    uint8_t regAge;
    uint32_t loginIP;
    uint16_t loginPort;
    bool recordLogins;
    uint16_t resX;
    uint16_t resY;
    bool chatterLog;
    bool encryptMode;
    std::string windowCaption;
    bool maskBan;
    bool remoteInterpreter;
    bool remoteOperator;
    bool playerVoices;
    uint32_t saveInterval;      // in hundredths of a second
    std::string spawnsFile;
    std::string commandsFile;
    std::string operatorsFile;
    uint32_t maxSpawns{ 3 };
    LogLevel logLevel;
    bool forceContinuum;        // hidden option
    bool runSilent;     // hidden option
    bool disablePub;
    bool noisySpectator;
    uint32_t maxTries;
    std::string initialChatChannels;
    
    void reloadIniFile(bool doLogin);   // doLogin: Ignore [Login] section to avoid DoS attempts
};


/// <summary>
/// Get the bot database singleton.
/// </summary>
/// <returns>Bot database.</returns>
export BotDatabase& getBotDatabase()
{
    static BotDatabase botDatabase;

    return botDatabase;
}
