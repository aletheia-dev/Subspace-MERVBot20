//
// VIE map format
//
// X    = WORD((N >> 12) & 0xFFF);    // This puts actual map dimensions at 4095x4095 possible tiles.
// Y    = WORD(N & 0xFFF);
// TILE = uint8_t(v >> 24);
//
// 0        = No tile
// 1-19        = Normal tiles
// 20        = Border
// 21-161    = Normal tiles
// 162-165    = Vertical doors
// 166-169    = Horizontal doors
// 170        = Flag
// 171        = Safe zone
// 172        = Goal area
// 173-175    = Fly over tiles
// 176-191    = Fly under tiles
//
// Warning: Deviating from this format may invalidate the security checksum.
//
export module Playfield;

export import Settings;

import Algorithms;

import <string>;


export typedef int32_t Coord;
export typedef uint8_t* Playfield;

export const Coord PixelMaxX{ 0x4000 };
export const Coord PixelMaxY{ 0x4000 };
export const Coord TileMaxX{ 0x400 };
export const Coord TileMaxY{ 0x400 };
export const Coord TileMaxLinear{ 0x100000 };


/// <summary>
/// Playfield tile format.
/// </summary>
export enum class PlayfieldTileFormat
{
    VieNoTile,            // These are VIE constants

    VieNormalStart = 1,
    VieBorder = 20,         // Borders are not included in the .lvl files
    VieNormalEnd = 161,     // Tiles up to this point are part of sec.chk

    VieVDoorStart = 162,
    VieVDoorEnd = 165,

    VieHDoorStart = 166,
    VieHDoorEnd = 169,

    VieTurfFlag = 170,

    VieSafeZone = 171,      // Also included in sec.chk

    VieGoalArea = 172,

    VieFlyOverStart = 173,
    VieFlyOverEnd = 175,
    VieFlyUnderStart = 176,
    VieFlyUnderEnd = 190,

    VieAsteroidStart = 216,
    VieAsteroidEnd = 218,

    VieStation = 219,

    VieWormhole = 220,

    SsbTeamBrick,       // These are internal
    SsbEnemyBrick,

    SsbTeamGoal,
    SsbEnemyGoal,

    SsbTeamFlag,
    SsbEnemyFlag,

    SsbPrize,

    SsbBorder                    // Use ssbBorder instead of vieBorder to fill border
};


/// <summary>
/// Tile data.
/// </summary>
export struct TileData
{
    Coord x, y;  // Coordinates
    uint8_t type;   // Type
};


/// <summary>
/// Convert rectangular tile coordinates to linear coordinates (0..TileMaxLinear - 1).
/// </summary>
/// <param name="x">X coordinate.</param>
/// <param name="y">Y coordinate.</param>
/// <returns>Linear coordinate.</returns>
export uint32_t getLinear(Coord x, Coord y)
{
    return ((y & (TileMaxX - 1)) << 10) | (x & (TileMaxX - 1));
}


/// <summary>
/// Transform a tile y coordinate to a radar (1..20) y coordinate.
/// </summary>
/// <param name="y">Tile y coordinate.</param>
/// <returns>Radar y coordinate.</returns>
export int32_t getNumeric(Coord y)
{
    return y * 20 / TileMaxX + 1;
}


/// <summary>
/// Transform a tile x coordinate to a radar (A..T) x coordinate.
/// </summary>
/// <param name="y">Tile x coordinate.</param>
/// <returns>Radar x coordinate.</returns>
export char getAlpha(Coord x)
{
    return char('A' + x * 20 / TileMaxX);
}


/// <summary>
/// Get a descriptive string for a radar xy coordinate.
/// </summary>
/// <param name="x">Tile x coordinate.</param>
/// <param name="y">Tile y coordinate.</param>
/// <returns>Descriptive string for a radar xy coordinate.</returns>
export std::string getCoords(Coord x, Coord y)
{
    return std::to_string(getAlpha(x)) + std::to_string(getNumeric(y));
}


/// <summary>
/// Make a tile data instance from raw file data. File data is in blocks of 32 bits.
/// </summary>
/// <param name="raw">File data.</param>
/// <returns>Tile data instance.</returns>
export TileData makeTileData(uint32_t raw)
{
    return { uint16_t(raw & 0xFFF), uint16_t((raw >> 12) & 0xFFF), uint8_t(raw >> 24) };
}


/// <summary>
/// Make a raw tile data block of 32 bit.
/// </summary>
/// <param name="x">Tile x coordinate.</param>
/// <param name="y">Tile y coordinate.</param>
/// <param name="type">Tile type.</param>
/// <returns>Raw tile data block.</returns>
export uint32_t makeTileData(uint16_t x, uint16_t y, uint8_t type)
{
    return (x & 0xFFF) | ((y & 0xFFF) << 12) | (type << 24);
}


/// <summary>
/// Convert from disk to memory format.
/// </summary>
/// <param name="fileData">Raw file data.</param>
/// <param name="playfield">Playfield.</param>
/// <param name="len"></param>
export void convertFileToMatrix(char* fileData, Playfield playfield, size_t len)
{
    memset(playfield, 0, TileMaxLinear);
    
    // Skip the tileset (if present)
    if (fileData[0] == 'B' && fileData[1] == 'M') {
        uint32_t diff = *(uint32_t*)&fileData[2];    // works: part of the bitmap standard

        fileData += diff;
        len -= diff;
    }
    
    // Fill map-tiles
    for (uint32_t i = 0; i < len; i += 4) {
        TileData t = makeTileData(*(uint32_t*)&fileData[i]);
        
        if (t.x >= TileMaxX || t.y >= TileMaxY) {
            continue;
        }
        
        // optimized by average tile frequency
        if (t.type < (uint8_t)PlayfieldTileFormat::VieAsteroidEnd) {
            playfield[getLinear(t.x, t.y)] = t.type;
        }
        else if (t.type == (uint8_t)PlayfieldTileFormat::VieAsteroidEnd) {
            playfield[getLinear(t.x, t.y)] = t.type;
            playfield[getLinear(t.x, t.y + 1)] = t.type;
            playfield[getLinear(t.x + 1, t.y)] = t.type;
            playfield[getLinear(t.x + 1, t.y + 1)] = t.type;
        }
        else if (t.type == (uint8_t)PlayfieldTileFormat::VieWormhole) {
            for (int x = 0; x < 5; x++)
            {
                playfield[getLinear(t.x + x, t.y)] = t.type;
                playfield[getLinear(t.x + x, t.y + 1)] = t.type;
                playfield[getLinear(t.x + x, t.y + 2)] = t.type;
                playfield[getLinear(t.x + x, t.y + 3)] = t.type;
                playfield[getLinear(t.x + x, t.y + 4)] = t.type;
            }
        }
        else if (t.type == (uint8_t)PlayfieldTileFormat::VieStation) {
            for (int x = 0; x < 6; x++) {
                playfield[getLinear(t.x + x, t.y)] = t.type;
                playfield[getLinear(t.x + x, t.y + 1)] = t.type;
                playfield[getLinear(t.x + x, t.y + 2)] = t.type;
                playfield[getLinear(t.x + x, t.y + 3)] = t.type;
                playfield[getLinear(t.x + x, t.y + 4)] = t.type;
                playfield[getLinear(t.x + x, t.y + 5)] = t.type;
            }
        }
        else {
            playfield[getLinear(t.x, t.y)] = t.type;
        }
    }

    // commented out because level checksum has been invalidated from time to time for some reason
    //// Fill border-tiles
    //playfield[0] = char(ssbBorder);
    //playfield[getLinear(0, 1023)] = char(ssbBorder);
    //playfield[getLinear(1023, 0)] = char(ssbBorder);
    //playfield[getLinear(1023, 1023)] = char(ssbBorder);
    //for (uint16_t x = 1; x < 1023; ++x) {
    //    playfield[getLinear(x,    0)] = char(ssbBorder);
    //    playfield[getLinear(x, 1023)] = char(ssbBorder);
    //    playfield[getLinear(0,    x)] = char(ssbBorder);
    //    playfield[getLinear(1023,    x)] = char(ssbBorder);
    //}
}


/// <summary>
/// Retrieve the size of the playfield on disk.
/// </summary>
/// <param name="playfield">Playfield.</param>
/// <returns>Playfield size.</returns>
export uint32_t getMapSize(Playfield playfield)
{
    uint32_t offset = 0;

    for (uint16_t x = 1; x < 1023; ++x)        // ignore border
        for (uint16_t y = 1; y < 1023; ++y) {
            uint8_t t = playfield[getLinear(x, y)];

            if (t == 0) {
                continue;
            }
            if ((t >= (uint8_t)PlayfieldTileFormat::VieNormalStart 
                    && t <= (uint8_t)PlayfieldTileFormat::VieFlyUnderEnd)   // regular tiles
                || (t >= (uint8_t)PlayfieldTileFormat::VieAsteroidStart
                    && t <= (uint8_t)PlayfieldTileFormat::VieWormhole)) {   // animated tiles
                // The tile is not internal
                ++offset;
            }
        }

    return offset << 2;
}


/// <summary>
/// Convert the map to disk format.
/// </summary>
/// <param name="playfield">Playfield.</param>
/// <param name="fileData">Raw file data.</param>
export void convertMatrixToFile(Playfield playfield, char* fileData)
{
    uint32_t offset = 0;

    for (uint16_t x = 1; x < 1023; ++x) {   // ignore border
        for (uint16_t y = 1; y < 1023; ++y) {
            uint8_t t = playfield[getLinear(x, y)];

            if (t == 0) {
                continue;
            }
            if ((t >= (uint8_t)PlayfieldTileFormat::VieNormalStart
                    && t <= (uint8_t)PlayfieldTileFormat::VieFlyUnderEnd)   // Regular tiles
                || (t >= (uint8_t)PlayfieldTileFormat::VieAsteroidStart
                    && t <= (uint8_t)PlayfieldTileFormat::VieWormhole)) {   // Animated tiles
                // The tile is not internal
                ((uint32_t*)fileData)[offset++] = makeTileData(x, y, t);
            }
        }
    }
}
