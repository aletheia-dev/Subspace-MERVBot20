//
// Various SubSpace encryption schemes by cat02e@fsu.edu
//
export module Encrypt;

import Algorithms;
import Checksum;
import Prng;
import System;

import <cstdint>;


export uint8_t ROT13(uint8_t c)
{
    if (c >= 'a' && c <= 'z') {
        c += 13;
        if (c > 'z') c -= 26;
    }
    else if (c >= 'A' && c <= 'Z') {
        c += 13;
        if (c > 'Z') c -= 26;
    }

    return c;
}


export struct SS_ENCR
{
    uint32_t key{};   // Session key
    uint32_t sentKey{};   // Client key

    SS_LIGHT_PRNG keygen;   // Key generator
    SS_HEAVY_PRNG prng; // Keystream generator

    uint8_t keystream[520]{};  // Session keystream

    // Make and set the client key
    uint32_t generateKey()
    {
        if (sentKey) {
            return sentKey;
        }

        uint32_t edx = ((uint32_t)getTickCount()) * 0xCCCCCCCD;
        uint32_t res = (std::rand() << 16) + (edx >> 3) + std::rand();

        res = (res ^ edx) - edx;

        if (res <= 0x7fffffff) {
            res = ~res + 1;
        }

        return (sentKey = res);
    }

    // Create a session key from a client key
    uint32_t getSessionKey(uint32_t clientKey)
    {
        // Two's complement, same as arithmetic minus on unsigned data
        return ((~clientKey) + 1);
    }

    // Make sure the key is valid
    bool validateSessionKey(uint32_t serverKey)
    {
        return ((serverKey == sentKey) || (serverKey == getSessionKey(sentKey)));
    }

    // Fill keystream, or revoke session
    bool initializeEncryption(uint32_t serverKey)
    {
        if (!validateSessionKey(serverKey))
            return false;

        if (sentKey == serverKey)
        {
            key = 0;

            memset(keystream, 0, 520);
        }
        else
        {
            key = serverKey;

            uint16_t* stream = (uint16_t*)keystream;

            prng.seed(serverKey);

            for (uint32_t i = 0; i < 260; ++i)
            {
                stream[i] = prng.getNextE();
            }
        }

        return true;
    }

    // Encrypt message
    void encrypt(uint8_t* msg, size_t len)
    {
        if (!key) return;

        uint32_t ksi = 0;
        uint32_t i = 1;
        uint32_t IV = key;

        if (*msg == 0) {
            if (len <= 2) return;

            ++i;
        }

        while (i + 4 <= len) {
            *(uint32_t*)&msg[i] = IV = (*(uint32_t*)&msg[i]) ^ (*(uint32_t*)&keystream[ksi]) ^ IV;

            i += 4;
            ksi += 4;
        }

        size_t diff = len - i;

        if (diff) {
            uint32_t buffer = 0;
            memcpy(&buffer, msg + i, diff);
            buffer ^= (*(uint32_t*)&keystream[ksi]) ^ IV;
            memcpy(msg + i, &buffer, diff);
        }
    }

    // Decrypt message
    void decrypt(uint8_t* msg, size_t len)
    {
        if (!key) return;

        uint32_t ksi = 0,
            i = 1,
            IV = key,
            EDX;

        if (*msg == 0) {
            if (len <= 2) {
                return;
            }
            ++i;
        }

        while (i + 4 <= len) {
            EDX = *(uint32_t*)&msg[i];
            *(uint32_t*)&msg[i] = (*(uint32_t*)&keystream[ksi]) ^ IV ^ EDX;
            IV = EDX;

            i += 4;
            ksi += 4;
        }

        size_t diff = len - i;

        if (diff) {
            uint32_t buffer = 0;
            memcpy(&buffer, msg + i, diff);
            buffer ^= (*(uint32_t*)&keystream[ksi]) ^ IV;
            memcpy(msg + i, &buffer, diff);
        }
    }

    // Nullify encryption
    void reset()
    {
        key = 0;
        sentKey = 0;
    }
};



//////// Billing password encryption ////////
// Also from Coconut Emulator

// Billing server password hashing
export void hashPassword(uint8_t* in, uint8_t* out)
{
    uint32_t L, StrLen = STRLEN((char*)in);
    uint8_t Factor = simpleChecksum(in, StrLen);
    uint8_t Char;

    for (L = 0; L < StrLen; ++L) {
        Char = in[L] ^ Factor;
        Factor = (Factor ^ (Char << (Char & 3))) & 255;
        if (Char == 0)
            Char = 0xED;
        out[L] = Char;
    }

    out[L] = 0;
}


//////// Billing password decryption ////////

// Perform the inverse of HashPassword
export void inverseHash(uint8_t* In, uint8_t* Out, uint8_t Key)
{
    size_t StrLen = STRLEN((char*)In);

    for (uint32_t L = 0; L < StrLen; ++L) {
        uint8_t Char = In[L];
        if (Char == 0xED)
            Char = 0;
        Out[L] = Char ^ (uint8_t)Key;
        Key = (Key ^ (Char << (Char & 3))) & 255;
    }
}


// Put it all together
export void DecryptHashedPassword(uint8_t* Password)
{
    uint32_t StrLen = STRLEN((char*)Password);

    uint8_t* Buffer = new uint8_t[StrLen + 1];
    Buffer[StrLen] = 0;

    // Passwords of EVEN length are very easy to crack.
    if (StrLen & 1) {
        // Guess and check to find one of the solutions
        for (uint16_t i = 0; i < 256; ++i) {
            // Generate a possible solution
            inverseHash(Password, Buffer, (uint8_t)i);

            // Compare resultant hash with given hash
            uint8_t Char, Key = (uint8_t)i;
            bool OK = true;

            for (uint32_t L = 0; L < StrLen; ++L) {
                Char = Buffer[L];

                if (!isPrintable(Char)) {
                    OK = false;

                    break;
                }

                Char ^= Key;
                Key = (Key ^ (Char << (Char & 3))) & 255;
                if (Char == 0)
                    Char = 0xED;

                if (Char != Password[L]) {
                    OK = false;

                    break;
                }
            }

            if (OK)
                break;
        }
    }
    else
    {
        // Generate password checksum
        inverseHash(Password, Buffer, (uint8_t)0);

        // Generate actual password
        inverseHash(Password, Buffer, simpleChecksum(Buffer, StrLen));
    }

    memcpy(Password, Buffer, StrLen);

    delete[] Buffer;
    Buffer = NULL;
}
