module;
#include <fstream>

export module Config;

import Algorithms;

import <any>;
import <format>;
import <functional>;
import <iostream>;
import <map>;
import <string>;
import <vector>;


//////// Parameter Storage ////////

/// <summary>
/// Storage for reference parameters.
/// </summary>
export class RefParamMap
{
public:
    /// <summary>
    /// Retrieve a reference parameter by name.
    /// </summary>
    /// <typeparam name="T">Type.</typeparam>
    /// <param name="name">Name.</param>
    /// <returns>Reference to the parameter value.</returns>
    template<typename T>
    const T& get(std::string name) const
    {
        return std::any_cast<std::reference_wrapper<T>>(m_refParams.at(name)).get();
    }

    /// <summary>
    /// Store a reference parameter by name.
    /// </summary>
    /// <typeparam name="T">Type.</typeparam>
    /// <param name="name">Name.</param>
    /// <param name="param">Reference to the parameter value.</param>
    template<typename T>
    void set(std::string name, T& param)
    {
        m_refParams[name] = std::ref(param);
    }

private:
    std::map<std::string, std::any> m_refParams;
};


//////// File Access ////////

/// <summary>
/// Read a configuration parameter value from an INI file.
/// </summary>
/// <param name="section">Section name.</param>
/// <param name="key">Parameter name.</param>
/// <param name="filePath">Absolute path of the configuration file.</param>
/// <param name="defValue">Default parameter value.</param>
/// <returns>Parameter value as string.</returns>
export std::string getPrivateProfileString(std::string_view section, std::string_view key, std::string_view filePath, 
    std::string defValue = "")
{
    std::ifstream file(filePath.data());

    try {
        bool sectionFound{};
        std::string line;
        uint32_t count{ 1 };

        while (getline(file, line)) {
            line = trim(line);

            if (line.starts_with("[")) {
                // [<section>]
                if (line.ends_with("]")) {
                    if (line.substr(1, line.length() - 2) == section) {
                        sectionFound = true;
                    }
                }
                else {
                    throw std::exception(std::format("Invalid configuration file '{}'. Missing ']' in line {}!",
                        filePath, count).c_str());
                }
            }
            else if (!line.empty() && !line.starts_with("#") && !line.starts_with("//")) {
                if (line.find("=") != std::string::npos) {
                    // <key>=<value>
                    if (sectionFound) {
                        std::string curKey = splitFirst(line, '=');

                        if (toLower(curKey) == toLower(key)) {
                            if (!line.empty())
                                return line;
                            else
                                return defValue;
                        }
                    }
                }
                else {
                    throw std::exception(std::format("Invalid configuration file '{}'. Missing '=' in line {}!", 
                        filePath, count).c_str());
                }
            }
            ++count;
        }
        return defValue;
    }
    catch (...) {
        file.close();
        throw;
    }
}


/// <summary>
/// Write a configuration parameter value to an INI file.
/// </summary>
/// <param name="section">Section name.</param>
/// <param name="key">Parameter name.</param>
/// <param name="value">Parameter value.</param>
/// <param name="filePath">Absolute path of the configuration file.</param>
export void setPrivateProfileString(std::string_view section, std::string_view key, std::string value, 
    std::string_view filePath)
{
    // read the config file and modify the specified parameter
    std::ifstream inFile(filePath.data());
    std::string dest;

    try {
        bool sectionFound{};
        std::string curLine, line;
        uint32_t count{ 1 };

        while (getline(inFile, curLine)) {
            line = trim(curLine);

            if (line.starts_with("[")) {
                // [<section>]
                if (line.ends_with("]")) {
                    if (line.substr(1, line.length() - 2) == section) {
                        sectionFound = true;
                    }
                }
                else {
                    throw std::exception(std::format("Invalid configuration file '{}'. Missing ']' in line {}!",
                        filePath, count).c_str());
                }
            }
            else if (!line.empty() && !line.starts_with("#") && !line.starts_with("//")) {
                // <key>=<value>
                if (line.find("=") != std::string::npos) {
                    if (sectionFound) {
                        std::string curKey = splitFirst(line, '=');

                        if (toLower(curKey) == toLower(key)) {
                            curLine = std::format("{}={}", curKey, value);
                        }
                    }
                }
                else {
                    throw std::exception(std::format("Invalid configuration file '{}'. Missing '=' in line {}!",
                        filePath, count).c_str());
                }
            }
            dest += curLine + '\n';
            ++count;
        }
        inFile.close();
    }
    catch (...) {
        inFile.close();
        throw;
    }

    // write the modified configuration to the specified file
    std::ofstream outFile(filePath.data());

    try {
        outFile.write(dest.c_str(), dest.length());
        outFile.close();
    }
    catch (...) {
        outFile.close();
        throw;
    }
}


/// <summary>
/// Read a configuration parameter value from an INI file. The parameter is converted to the type of the destination 
/// variable. If no default value is given, the parameter is obligatory.
/// </summary>
/// <param name="section">Section name.</param>
/// <param name="key">Parameter name.</param>
/// <param name="retValue">Output value.</param>
/// <param name="filePath">Absolute path of the configuration file.</param>
/// <param name="defValue">Default parameter value. If undefined, the parameter has to be specified in the config 
/// file, otherwise an exception is thrown.</param>
export template<typename RetT>
void readConfigParam(std::string_view section, std::string_view key, RetT& retValue, std::string_view filePath,
    std::string defValue = "__no_opt__")
{
    std::string configValue;

    if ((configValue = getPrivateProfileString(section, key, filePath, defValue)) != "__no_opt__") {
        try {
            if (typeid(retValue) == typeid(std::string)) {
                retValue = (decltype(retValue))configValue;
            }
            else if (typeid(retValue) == typeid(bool)) {
                auto val{ stoll(configValue) };

                retValue = (decltype(retValue))val;
            }
            else if (typeid(retValue) == typeid(int8_t)
                || typeid(retValue) == typeid(int16_t)
                || typeid(retValue) == typeid(int32_t)
                || typeid(retValue) == typeid(int64_t)) {
                auto val{ stoll(configValue) };

                retValue = (decltype(retValue))val;
            }
            else if (typeid(retValue) == typeid(uint8_t)
                || typeid(retValue) == typeid(uint16_t)
                || typeid(retValue) == typeid(uint32_t)
                || typeid(retValue) == typeid(uint64_t)) {
                auto val{ stoull(configValue) };

                retValue = (decltype(retValue))val;
            }
            else if (typeid(retValue) == typeid(std::list<std::string>)) {
                // <string>[,<string>...]
                std::list<std::string> strList;

                for (std::string& str : split(configValue, ',')) {
                    strList.push_back(str);
                }
                retValue = (decltype(retValue))strList;
            }
            else if (std::is_enum_v<std::remove_reference_t<decltype(retValue)>>) {
                auto val{ stoi(configValue) };

                retValue = (decltype(retValue))val;
            }
            else if (typeid(retValue) == typeid(std::pair<std::string, uint16_t>)) {
                // <string>:<uint16_t>
                std::pair<std::string, uint16_t> val;

                val.first = splitFirst(configValue, ':');
                val.second = (uint16_t)stoul(configValue);
                retValue = (decltype(retValue))val;
            }
            else if (typeid(retValue) == typeid(std::map<std::string, uint64_t>)) {
                // <string>[:<string>][,<string>[:<string>]...]
                std::map<std::string, uint64_t> valStrMap;

                for (std::string& splitConfigValue : split(configValue, ',')) {
                    std::vector<std::string> valStr = split(splitConfigValue, ':');
                    std::string arena = valStr.size() > 1 ? valStr[1] : "0";

                    valStrMap[arena] = stoull(valStr[0]);
                }
                retValue = (decltype(retValue))valStrMap;
            }
            else {
                throw std::exception(std::format("Unable to convert the value '{}' of config parameter "
                    "'{}' to '{}'. No conversion available for this destination type!", configValue, key, 
                    typeid(retValue).name()).c_str());
            }
        }
        catch (...) {
            throw std::exception(std::format("Unable to convert the value '{}' of config parameter "
                "'{}' to '{}'!", configValue, key, typeid(retValue).name()).c_str());
        }
    }
    else {
        // neither was the parameter found nor is there a default value specified
        throw std::exception(std::format("The config parameter '{}' is not defined in '{}'!", key, filePath).c_str());
    }
}


/// <summary>
/// Extract data lines from a mixed-format database file.
/// </summary>
/// <param name="fileName">Parameter name.</param>
/// <param name="callback">Callback function to evaluate a line.</param>
/// <returns></returns>
export bool readDataLines(std::string_view fileName, std::function<void(std::string_view)> callback)
{
    std::ifstream file(fileName.data());

    if (!file)
        return false;

    char c, buffer[256]{};
    int i = 0;
    bool skip = false;

    while ((c = file.get()) != -1) {
        switch (c) {
        case '\n':
            buffer[i] = '\0';
            if ((i > 0) && !skip) {
                callback(buffer);
            }
            i = 0;
            skip = false;
            break;
        default:
            if (i >= 255)
                break;
            if (i == 0) 
                skip = !isAlphaNumeric(c);
            buffer[i++] = c;
        case '\r':;
        }
    }
    buffer[i] = '\0';

    if (i > 0) {
        callback(buffer);
    }
    return true;
}


/// <summary>
/// Extract data lines from a mixed-format database file.
/// </summary>
/// <typeparam name="T">Calling class type.</typeparam>
/// <param name="fileName">Parameter name.</param>
/// <param name="callback">Callback function to evaluate a line.</param>
/// <param name="obj">Instance of the calling class.</param>
/// <returns></returns>
export template<typename T>
bool readDataLines(std::string_view fileName, void(T::* callback)(std::string_view line), T* obj)
{
    return readDataLines(fileName, [callback, obj](std::string_view line) { (obj->*callback)(line); });
}
