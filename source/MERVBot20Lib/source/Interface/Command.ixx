module;
#include <fstream>

export module Command;

import Algorithms;
import Config;

import <format>;
import <iostream>;
import <list>;
import <map>;
import <set>;


/// <summary>
/// Operator level.
/// </summary>
export enum class OperatorLevel
{
    Player,             // level 0: Requests information services
    Limited,            // level 1: Player with some privelages
    Moderator,          // level 2: Represents player interests
    SuperModerator,     // level 3: Manages moderators
    SysOp,              // level 4: Adds/removes/modifies available bot services
    Owner,              // level 5: Changes internal bot settings
    Duke,
    Baron,
    King,
    Emperor,
    RockStar,
    Q,
    God,                 // ...Some other ways to say "Owner"
    Unknown
};


/// <summary>
/// Mapping of operator levels and descriptions.
/// </summary>
/// <returns></returns>
export std::map<OperatorLevel, std::string>& getLevelDescriptions()
{
    static std::map<OperatorLevel, std::string> levelDescriptions{
        { OperatorLevel::Player, "Player" },
        { OperatorLevel::Limited, "Limited" },
        { OperatorLevel::Moderator, "Mod" },
        { OperatorLevel::SuperModerator, "SMod" },
        { OperatorLevel::SysOp, "SOp" },
        { OperatorLevel::Owner, "Owner" },
        { OperatorLevel::Duke, "Duke" },
        { OperatorLevel::Baron, "Baron" },
        { OperatorLevel::King, "King" },
        { OperatorLevel::Emperor, "Emperor" },
        { OperatorLevel::RockStar, "RockStar" },
        { OperatorLevel::Q, "Q" },
        { OperatorLevel::God, "God" },
        { OperatorLevel::Unknown, "UNKNOWN" }
    };

    return levelDescriptions;
}


/// <summary>
/// Get the operator level of either an operator description or number.
/// </summary>
/// <param name="numDesc">Operator description or number according to levelDescriptions.</param>
/// <returns>Operator level.</returns>
export OperatorLevel getOperatorLevel(std::string_view numDesc)
{
    if (!numDesc.empty()) {
        for (auto& [level, desc] : getLevelDescriptions()) {
            if (isNumeric(numDesc)) {
                return (OperatorLevel)std::stoi(numDesc.data());
            }
            else if (toLower(desc) == numDesc) {
                return level;
            }
        }
    }
    return OperatorLevel::Unknown;
}


/// <summary>
/// Get operator level descriptions.
/// </summary>
/// <param name="level">Operator level.</param>
/// <returns>Operator level description.</returns>
export std::string getLevelDescription(OperatorLevel level)
{
    const std::map<OperatorLevel, std::string>& levelDescriptions{ getLevelDescriptions() };

    return levelDescriptions.at(level);
}


/// <summary>
/// Command scope. Used to specify the usable scope of a command.
/// </summary>
export enum class CommandScope
{
    Local,      // can be used in continuum chat within the bot's arena only
    Remote,     // can be used in continuum chat (remote or local arena) only
    External        // can be used in a continuum and in an external chat client
};


export struct SwitchInfo
{
    char key;       // switch key
    std::string name;       // parameter name
    std::string help;       // short parameter help
};


export struct ParamInfo
{
    std::string format;     // parameter format info
    std::string help;       // short parameter help
    bool isOptional{};      // true in case of an optional parameter
};


/// <summary>
/// Container for all information about a command.
/// </summary>
export struct CommandInfo
{
    OperatorLevel minLevel;     // necessary operator level
    CommandScope maxScope;      // highest eligible scope
    std::string shortHelp;      // short help description
    ParamInfo paramInfo;        // parameter info
    std::list<SwitchInfo> switchInfos;      // switch infos
    std::string additionalHelp;     // additional help description
    bool externalOnly{};        // true in case the command is only for external chat clients
};


/// <summary>
/// Command info container type.
/// </summary>
export typedef std::map<std::string, std::list<CommandInfo>> CommandInfoMap;


//////// Validation ////////

/// <summary>
/// Check the validity of a player or bot name. Names that exceed the length of 19 characters are still valid, but 
/// will be truncated to 19 characters.
/// </summary>
/// <param name="name">Name.</param>
/// <returns>True, if the name is valid.</returns>
export bool invalidName(std::string& name)
{
    uint32_t len = (uint32_t)name.length();
    bool seenSpace = false;

    // Name cannot be blank
    if (len == 0) {
        return true;
    }
    // Keep name under 20 characters
    if (len > 19) {
        name = name.substr(0, 19);
        len = 19;
    }

    // First character must be alphanumeric
    if (!isAlphaNumeric(name[0])) {
        return true;
    }
    // Cannot end in a space
    if (name[len - 1] == ' ') {
        return true;
    }

    for (uint32_t i = 0; i < len; ++i) {
        // Only legal & printable characters
        if (!isPrintable(name[i]) ||
            (name[i] == ':') ||
            (name[i] == '%')) {
            return true;
        }

        // Only one space in a row
        if (name[i] == ' ') {
            if (seenSpace)
                return true;
            else
                seenSpace = true;
        }
        else {
            seenSpace = false;
        }
    }
    return false;
}


/// <summary>
/// Check the validity of an arena name. Names that exceed the length of 19 characters are still valid, but will be 
/// truncated to 19 characters.
/// </summary>
/// <param name="name">Arena name.</param>
/// <returns>True, if the arena name is valid.</returns>
export bool invalidArena(std::string& name)
{
    uint32_t len = (uint32_t)name.length();

    // blank arena names are permissable
    if (len == 0) {
        return false;
    }
    // keep name under 11 characters
    if (len > 10) {
        name = name.substr(0, 10);
        len = 10;
    }

    // skip valid prefixes
    uint32_t i = 0;

    if (name.starts_with('#')) {
        ++i;
    }

    // prefix must be followed by arena name
    for (; i < len; ++i) {
        // only alpha-numeric characters
        if (!isAlphaNumeric(name[i])) {
            return true;
        }
    }
    return false;
}


/// <summary>
/// Check if a given text has the name format of a local or remote chat message.
/// </summary>
/// <param name="text">Chat message.</param>
/// <returns>True, if the given text is a valid local or remote message.</returns>
export bool isValidChatMessage(std::string_view text)
{
    bool seen = false;

    for (uint16_t i = 0; i < text.length(); ++i) {
        if (text[i] == '>' && text[i + 1] == ' ') {
            if (seen) {
                return false;
            }
            seen = true;
        }
    }
    return seen;
}


/// <summary>
/// Check if a given text has the format of a remote private message.
/// </summary>
/// <param name="text">Remote message.</param>
/// <returns>True, if the given text is a remote message.</returns>
export bool isValidRemotePrivateChatMessage(std::string_view text)
{
    bool seen = false;

    for (uint16_t i = 1; i < text.length() - 1; ++i) {
        if (text[i] == ')' && text[i + 1] == '>') {
            if (seen) {
                return false;
            }
            seen = true;
        }
    }
    return seen;
}


/// <summary>
/// Extract the player name from a remote chat message.
/// </summary>
/// <param name="text">Remote message.</param>
/// <returns>Player name from a remote message.</returns>
export std::string_view getRemoteChatPlayerName(std::string_view text)
{
    for (uint16_t i = 0; i < text.length() - 1; ++i) {
        if (text[i] == ')' && text[i + 1] == '>') {
            text = text.substr(0, i);
            break;
        }
    }
    return text.substr(1);
}


/// <summary>
/// Extract the message text from a remote chat message.
/// </summary>
/// <param name="text">Remote message.</param>
/// <returns>Command from a remote message.</returns>
export std::string_view getRemoteChatMessageText(std::string_view text)
{
    for (uint16_t i = 0; i < text.length() - 2; ++i) {
        if (text[i] == ')' && text[i + 1] == '>') {
            return text.substr(i + 2);
        }
    }
    return text;
}


/// <summary>
/// Extract the player name from a chat message.
/// </summary>
/// <param name="text">Chat message.</param>
/// <returns>Player name.</returns>
export std::string_view getChatPlayerName(std::string_view text)
{
    for (uint16_t i = 0; i < text.length(); ++i) {
        if (text[i] == ':') {
            text = text.substr(i + 1);
            break;
        }
    }

    for (uint16_t i = 0; i < text.length() - 1; ++i) {
        if (text[i] == '>' && text[i + 1] == ' ') {
            text = text.substr(0, i);
            break;
        }
    }
    return text;
}


/// <summary>
/// Extract the message text from a chat message.
/// </summary>
/// <param name="text">Chat message.</param>
/// <returns>Message text.</returns>
export std::string_view getChatMessageText(std::string_view text)
{
    for (uint16_t i = 0; i < text.length(); ++i) {
        if (text[i] == '>' && text[i + 1] == ' ') {
            return text.substr(i + 2);
        }
    }
    return text;
}


//////// Command aliases ////////

/// <summary>
/// Command alias.
/// </summary>
export class CmdAlias
{
public:
    CmdAlias(std::string_view cmd, std::string_view alias)
        : m_cmd(cmd), m_alias(alias) {}

    bool operator == (const CmdAlias& rhs) const
    {
        return m_cmd == rhs.getCommand() && m_alias == rhs.getAlias();
    }

public:
    bool isCmd(std::string_view cmd) const {
        return (m_cmd == toLower(cmd));
    }

    std::string_view getCommand() const {
        return m_cmd;
    }

    std::string_view getAlias() const {
        return m_alias;
    }

    bool isAlias(std::string_view alias) const {
        return (m_alias == toLower(alias));
    }

    // Change alias to command if appliacable
    bool toCommand(std::string& alias) {
        if (m_alias == alias) {
            alias = m_cmd;
            return true;
        }
        return false;
    }

private:
    std::string m_cmd;
    std::string m_alias;
};


/// <summary>
/// Container for command aliases.
/// </summary>
export class AliasList
{
public:
    const std::list<CmdAlias>& getAliases()
    {
        return m_aliases;
    }

    // replace a getCommand with the alias, if applicable
    std::string aliasToCommand(std::string_view alias)
    {
        std::string cmd{ alias };

        for (CmdAlias& cmdAlias : m_aliases) {
            if (cmdAlias.toCommand(cmd)) {
                // cmd was an alias, now turned into a command
                break;
            }
        }
        return cmd;
    }

    void addAlias(std::string_view cmd, std::string_view alias)
    {
        m_aliases.push_back(CmdAlias(cmd, alias));
        m_isUpdated = true;
    }

    bool killAlias(std::string_view alias)
    {
        for (CmdAlias& aliasCmd : m_aliases) {
            if (aliasCmd.isAlias(alias)) {
                m_aliases.remove(aliasCmd);
                m_isUpdated = true;
                return true;
            }
        }
        return false;
    }

    CmdAlias* findAlias(std::string_view alias)
    {
        for (CmdAlias& aliasCmd : m_aliases) {
            if (aliasCmd.isAlias(alias)) {
                return &aliasCmd;
            }
        }
        return nullptr;
    }

    void gotAlias(std::string_view line)
    {
        std::string text{ line };
        std::string alias{ toLower(splitFirst(text, ':')) };
        std::string cmd{ toLower(splitFirst(text, ':')) };
        addAlias(cmd, alias);
    }

    void loadAliases(std::string_view commandsFile)
    {
        readDataLines(commandsFile, &AliasList::gotAlias, this);
        m_isUpdated = false;
    }

    void save(std::string_view commandsFile)
    {
        if (!m_isUpdated)
            return;

        std::ofstream file(commandsFile.data());

        if (!file.good()) {
            std::cout << std::format("WARNING: Unable to overwrite {} for Commands database save\n", commandsFile);
            return;
        }
        std::cout << "Saving aliases database...\n";

        for (CmdAlias& ca : m_aliases) {
            file << ca.getAlias() << ":" << ca.getCommand() << "\r\n";
        }
        m_isUpdated = false;
    }

    std::string getAliasesDescription(std::string_view cmd)
    {
        std::string s;
        uint32_t count = 0;

        for (CmdAlias& aliasCmd : m_aliases) {
            if (aliasCmd.isCmd(cmd)) {
                if (count > 4) {
                    s += "...";
                    break;
                }
                else {
                    if (count++) {
                        s += " ";
                    }
                    s += aliasCmd.getAlias();
                }
            }
        }
        return s;
    }

private:
    std::list<CmdAlias> m_aliases;
    bool m_isUpdated{};
};


/// <summary>
/// Get the command alias list singleton.
/// </summary>
/// <returns>Command alias list.</returns>
export AliasList& getAliasList()
{
    static AliasList aliasList;

    return aliasList;
}


/// <summary>
/// Get a description string for all aliases of a command.
/// </summary>
/// <param name="m_host">Host.</param>
/// <param name="aliasCmd">Command name. Could also be a command alias.</param>
/// <returns>Description string for all aliases of this command.</returns>
export std::string getAliasesDescription(std::string aliasCmd)
{
    // if it's an alias, get the command
    std::string cmd{ getAliasList().aliasToCommand(aliasCmd) };

    // get the description string for the aliases of this command
    std::string aliasesDesc{ getAliasList().getAliasesDescription(cmd) };

    if (!aliasesDesc.empty())
        return std::format("AKA  {} {}\n", cmd, aliasesDesc);
    else
        return "";
}


//////// Commands ////////

/// <summary>
/// Command switch.
/// </summary>
export struct CommandSwitch
{                    //            |----|
    char type;        // !getCommand -a_=blah-s blah
    std::string param;    // !getCommand -at=____-s blah

    CommandSwitch() : type('\0'), param("") {};
    CommandSwitch(char t, std::string_view p) : type(t), param(p) {};
};


export typedef std::list<CommandSwitch> SwitchList;


/// <summary>
/// Command container. Used to perform all operations related to a single command.
/// </summary>
export class Command
{
public:
    // Parse
    Command(std::string_view msg)
    {
        std::string tmp;
        bool inStub = true;
        bool inParam = false;
        bool inFinal = false;
        bool seenSpace = false;

        for (uint32_t i = 0; i < msg.length(); ++i) {
            char c = msg[i];

            switch (c) {
            case ' ': {
                if (inStub) {
                    // store getCommand name
                    m_cmd = getAliasList().aliasToCommand(tmp);
                    tmp.clear();
                    inStub = false;
                }
                else if (inParam) {
                    addParam(tmp);
                    tmp.clear();
                    inParam = false;
                }
                else if (inFinal) {
                    tmp += c;
                }
                seenSpace = true;
            }
                    break;
            case '-': {
                if (inStub) {
                    // store getCommand name
                    m_cmd = getAliasList().aliasToCommand(tmp);
                    tmp.clear();
                    inStub = false;
                    inParam = true;
                }
                else if (inParam || inFinal) {
                    tmp += c;
                }
                else {
                    inParam = true;
                }
                seenSpace = false;
            }
                    break;
            default:
                if (inStub) {
                    c = toLower(c);
                }
                else if (seenSpace) {
                    inFinal = true;
                    seenSpace = false;
                }
                tmp += c;
            }
        }

        if (inStub)
            m_cmd = getAliasList().aliasToCommand(tmp);
        else if (inParam)
            addParam(tmp);
        else
            m_final = tmp;
    }

    std::string_view getCommand() const
    {
        return m_cmd;
    }

    void setCommand(std::string_view cmd)
    {
        m_cmd = cmd;
    }

    const SwitchList& getSwitches() const 
    {
        return (const SwitchList&)m_switchlist;
    }
    
    std::string_view getFinal() const
    {
        return m_final;
    }

    int32_t getFinalAsInt() const
    {
        return std::stoi(m_final);
    }

    bool check(std::string_view cmd) const
    {
        // check if a command name starts with the command (abbreviation) typed by the user
        return cmd.starts_with(m_cmd);
    }

    // Check against final
    bool checkParam(std::string_view msg) const
    {
        return toLower(getFinal()).starts_with(toLower(msg));
    }

    // Add a switch
    void addParam(std::string_view msg)
    {
        bool inSwitches = true;
        std::string tmp;
        char type = '\0';

        for (int32_t i = 0; i < msg.length(); ++i) {
            char c = msg[i];

            switch (c) {
            case '-': {
                // as long as we are within the getSwitches section (before =), irgnore - signs, i.e.
                // -a-s equals -a and -s
                if (!inSwitches) {
                    // outside of the getSwitches section, the - belongs to the parameter
                    tmp += c;
                }
            }
            case '=': {
                inSwitches = false;
                break;
            }
            default:
                if (inSwitches) {
                    // push a switch without parameter, if there is more than one character after -
                    // each character is a separate switch, e.g. -as equals -a and -s
                    type = toLower(c);
                    m_switchlist.push_back(CommandSwitch(type, ""));
                }
                else {
                    tmp += c;
                }
            }
        }

        if (type && (inSwitches == false)) {
            m_switchlist.pop_back();
            m_switchlist.push_back(CommandSwitch(type, tmp));
        }
    }

    // Check against getSwitches
    bool hasParam(char type) const
    {
        for (const CommandSwitch& sw : m_switchlist) {
            if (sw.type == type) {
                return true;
            }
        }
        return false;
    }

    // Check against getSwitches
    bool getParam(char type, CommandSwitch& s) const
    {
        for (const CommandSwitch& sw : m_switchlist) {
            if (sw.type == type) {
                s = sw;
                return true;
            }
        }
        return false;
    }

private:
    SwitchList m_switchlist;
    std::string m_cmd;        // !_______ -at=blah-s blah
    std::string m_final;    // !getCommand -at=blah-s ____
};


/// <summary>
/// Get the help description of a command. If applicable, the help description contains an alias info, a short usage 
/// desciption, a switch descriptions and an additional help description.
/// </summary>
/// <param name="cmd">Command name.</param>
/// <param name="cmdInfo">Command info.</param>
/// <returns>Help description of the command.</returns>
export std::string getCommandHelp(std::string cmd, const CommandInfo& cmdInfo)
{
    std::string cmdHelp;

    // alias info
    cmdHelp.append(getAliasesDescription(cmd));

    // short usage desciption
    cmdHelp.append(std::format("\nUsage: !{}{}{}", cmd,
        !cmdInfo.paramInfo.format.empty() ? std::format("  {}", cmdInfo.paramInfo.format) : "",
        !cmdInfo.shortHelp.empty() ? std::format("  ({})", cmdInfo.shortHelp) : ""));

    // switch descriptions
    if (!cmdInfo.switchInfos.empty()) {
        cmdHelp.append("\nswitches:\n");

        for (auto& switchInfo : cmdInfo.switchInfos) {
            cmdHelp.append(std::format("-{}{}{}\n", switchInfo.key,
                !switchInfo.name.empty() ? std::format("={}", switchInfo.name) : "",
                !switchInfo.help.empty() ? std::format("  ({})", switchInfo.help) : ""));
        }
    }

    // additional help
    cmdHelp.append(cmdInfo.additionalHelp);

    return cmdHelp;
}
