//
// SubSpace pseudo-random number generators by cat02e@fsu.edu
//
export module Prng;

import Algorithms;

import <cstdint>;


// Internal bot generator
export class LCG_PRNG
{
    int32_t s1{ 99999 };    // Seeds
    int32_t s2{ 55555 };    // Seeds

public:
    // Automatic seeding
    void seed(int32_t newSeed)
    {
        s1 = 55555 ^ newSeed;
        s2 = newSeed;
    }

    // Manual seeding
    void seed(int32_t newSeedX, int32_t newSeedY)
    {
        s1 = newSeedX;
        s2 = newSeedY;
    }
    /*
     acu: replaced by std rng to avoid assembler code
    int32_t LCG_PRNG::getNextI()
    {
        uint32_tx, y;

        IDIVCOMP(s1, 53668, x, y);
        s1 = (x * 40014) - (y * 12211);
        if (s1 < 0) s1 += 0x7FFFFFAB;

        IDIVCOMP(s1, 52774, x, y);
        s2 = (x * 40692) - (y * 3791);
        if (s2 < 0) s2 += 0x7FFFFF07;

        uint32_tz = (s1 - 0x7FFFFFAB) + s2;
        if (z <= 0) z += 0x7FFFFFAA;

        return z;
    }

    double LCG_PRNG::getNextD()
    {
        return getNextI() / double(0xFFFFFFFF);
    }
    */
};


//////// Linear Feedback Shift Register Generator ////////

// Internal bot generator
export class LFSR_PRNG
{
    int32_t p1, p2;                // Seed indices
    uint32_t randbuffer[11];        // Seed - matter

public:
    // Default initialization
    LFSR_PRNG()
    {
        seed(55555);
    }

    // Do (double) conversion too
    double getNextD()
    {    
        // Using (GetNextI() / MAX) shouldn't make a difference
        uint32_t x = getNextI();

        union
        {
            double randp1;
            uint32_t randbits[2];
        };

        randbits[0] = (x << 20);
        randbits[1] = (x >> 12) | 0x3FF00000;

        return randp1 - 1.0;
    }

    // Get the next number in the series
    uint32_t getNextI()
    {
        uint32_t x = (randbuffer[p1] = ROR(randbuffer[p1] + randbuffer[p2], 13));

        if (--p1 < 0) p1 = 10;    // Spin the counters
        if (--p2 < 0) p2 = 10;

        return x;
    }

    // Re-seed the generator (rather secure)
    void seed(uint32_t newSeed)
    {
        if (newSeed == 0) --newSeed;    // Set seed to 0xFFFFFFFF
        int32_t i;

        for (i = 0; i < 11; ++i) {
            // Loses bits pretty easily, which is good if you want to keep the original seed secret
            newSeed ^= newSeed << 13;
            newSeed ^= newSeed >> 17;
            newSeed ^= newSeed << 5;
            randbuffer[i] = newSeed;
        }

        p1 = 0;
        p2 = 7;

        // Mask original seed further, skip the (double) conversion
        for (i = 0; i < 9; ++i) {
            getNextI();
        }
    }
};


//////// Simple SubSpace LCG ////////

// Assembly provided by Coconut Emulator
// Used to generate 00 01 connection requests
export struct SS_LIGHT_PRNG
{    
    uint32_t s;                    // Seed

    SS_LIGHT_PRNG()
    {
        seed(0);
    }

    // Provide a seed
    SS_LIGHT_PRNG(uint32_t newSeed)
    {
        seed(newSeed);
    }

    // Provide a seed
    void seed(uint32_t newSeed)
    {
        s = newSeed;
    }

    // Get the next number
    uint16_t getNext()
    {
        s *= 0x343FD;    // Has some nice properties, but
        s += 0x269EC3;    //  ultimately fails to pass any
        //  randomality test.

        return uint16_t((s >> 10) & 0x7FFF);    // Return the high word
    }
};


//////// SubSpace Remainder Congruential Generator ////////

const uint32_t KSGSCD{ 0x1F31D };
const uint32_t KSGSCM{ 0x834E0B5F };

// Assembly provided by Coconut Emulator and Kavar!
// Used to generate the keystream and transform synchronized seeds
export struct SS_HEAVY_PRNG
{
    uint32_t s; // Seed

    SS_HEAVY_PRNG()
    {
        seed(0);
    }

    // Provide a seed
    SS_HEAVY_PRNG(uint32_t newSeed)
    {
        seed(newSeed);
    }

    // Provide a seed
    void seed(uint32_t newSeed)
    {
        s = newSeed;
    }

    // Get the next number for encryption
    uint16_t getNextE()
    {
        // Original C++ implementation contributed by UDP
        uint32_t old_seed = s;

        s = (int32_t)(((int64_t)old_seed * KSGSCM) >> 48);
        s = s + (s >> 31);
        s = ((old_seed % KSGSCD) * 16807) - (s * 2836) + 123;
        if ((int32_t)s <= 0) s += 0x7fffffff;

        return uint16_t(s);
    }
/*
     acu: replaced by std rng to avoid assembler code
    // Get the next number for green seeds
    uint16_t SS_HEAVY_PRNG::getNextG()
    {
        uint32_tTSeed = IMULHIDWORD(s, KSGSCM) + s;
        TSeed = ((long)TSeed >> 16) + (TSeed >> 31);
        s = ((s % KSGSCD) * 16807) - (TSeed * 2836) + 123;
        if ((Sint32)s <= 0) s += 0x7fffffff;

        return Uint16(s);
    }
*/
};
