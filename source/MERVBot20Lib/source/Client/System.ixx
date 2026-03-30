module;
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include "zlib.h"
#pragma comment(lib, "zlib.lib")

export module System;

import Algorithms;

import <filesystem>;


export typedef HKEY HKey;


/// <summary>
/// Get the current clock tick since 1st jan 1970 in hundredths of a second.
/// </summary>
/// <returns>Current clock tick in milliseconds.</returns>
export uint64_t getTickCount()
{
    return GetTickCount64() / 10;
}


export uint32_t getSetting32(HKEY baseKey, const char* path, const char* value)
{
    // Open the key
    HKEY key;            // Handle to a session with a registry key

    RegOpenKey((HKEY)baseKey, path, &key);

    // Get a value
    DWORD buffer;        // Results of transaction will go here
    DWORD buflen = 4;    // Length of the buffer is 4, naturally
    DWORD type;            // Type will contain type of data transfered

    RegQueryValueEx(key, value, NULL, &type, (BYTE*)&buffer, &buflen);

    // Close the key
    RegCloseKey(key);

    // Check data for validity
    if (type == REG_DWORD)
        return buffer;
    else
        return 0xffffffff;            // -1 = error
}


export void setSetting32(HKEY baseKey, const char* path, const char* value, uint32_t buffer)
{
    // Open the key
    HKEY key;            // Handle to a session with a registry key

    RegCreateKey(baseKey, path, &key);

    // Get a value
    RegSetValueEx(key, value, 0, REG_DWORD, (BYTE*)&buffer, 4);

    // Close the key
    RegCloseKey(key);
}


export void getServiceString(HKEY baseKey, const char* path, const char* value, char* buffer)
{
    // Open the key
    HKEY key;            // Handle to a session with a registry key

    RegOpenKey(baseKey, path, &key);

    // Get a value
    DWORD buflen = 40;
    DWORD type;            // Type will contain type of data transfered

    RegQueryValueEx(key, value, NULL, &type, (BYTE*)buffer, &buflen);

    // Close the key
    RegCloseKey(key);
}


// Add news checksum to SubSpace news checksum archive in registry
export void addNewsChecksum(uint32_t Checksum)
{
    // Extract location to write the new checksum
    uint32_t Position = getSetting32(HKEY_CURRENT_USER, "Software\\Virgin\\SubSpace\\News", "Pos");

    std::string s;

    if (Position < 1000)
        s += "0";
    if (Position < 100)
        s += "0";
    if (Position < 10)
        s += "0";
    s += Position;

    // Write the checksum
    setSetting32(HKEY_CURRENT_USER, "Software\\Virgin\\SubSpace\\News", s.c_str(), Checksum);

    // Calculate new location to write the next checksum
    ++Position;
    if (Position == 10000)
        Position = 0;

    // Write this location to the registry
    setSetting32(HKEY_CURRENT_USER, "Software\\Virgin\\SubSpace\\News", "Pos", Position);
}


// Works only once! Changes DOS prompt window title
export void setWindowTitle(std::string title)
{
    char fileName[532];
    int32_t len = GetModuleFileName(GetModuleHandle(NULL), fileName, 532);

    if (!SetWindowText(FindWindow(NULL, fileName), title.c_str()))
    {
        int32_t off = -1;

        for (int32_t i = len - 1; i > 1; --i)
        {
            switch (fileName[i])
            {
            case '/':
            case '\\':
                off = i + 1;
                i = 0;
                break;
            case '.':
                fileName[i] = '\0';
            };
        }

        if (off != -1)
        {
            SetWindowText(FindWindow(NULL, fileName + off), title.c_str());
        }
    }
}


// Decompress buffer to a file on disk
export bool _stdcall decompress_to_file(const char* name, void* buffer, size_t buffer_length)
{
    if (buffer_length <= 0) {
        return false;
    }

    std::filesystem::path dirPath = std::filesystem::path(name).parent_path();

    if (!std::filesystem::is_directory(dirPath)) {
        std::filesystem::create_directory(dirPath);
    }

    std::ofstream file(name, std::ios::binary);

    if (!file) {
        return false;
    }

    uint8_t* res = NULL;
    size_t length = buffer_length * 2;
    uLong status = 0;

    do {
        if (res) {
            delete[]res;
        }

        length += buffer_length;
        res = new BYTE[length];

        if (!res) {
            return false;
        }

        status = uncompress(res, (uLongf*)&length, (BYTE*)buffer, (uLong)buffer_length);
    } while (status == Z_BUF_ERROR);

    if (status != Z_OK) {
        return false;
    }

    file.write((char*)res, length);
    delete[]res;

    return true;
}
