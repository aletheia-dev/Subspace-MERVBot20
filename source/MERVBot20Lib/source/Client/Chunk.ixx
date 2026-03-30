//
//  Chunk message handler by cat02e@fsu.edu
//
export module Chunk;

import <string>;


export class ChunkBuffer
{
    uint32_t maximumLength;     // Message limit (default 1KB)

    // Quick create/copy/destroy mechanism
    void resizeMessage(size_t len)
    {
        uint8_t* newbuff = new uint8_t[len];

        if (buffer) {
            memcpy(newbuff, buffer, currentLength);
            delete[]buffer;
        }
        buffer = newbuff;
    }

public:
    uint8_t* buffer;   // Message contents
    size_t currentLength;     // Length of buffer

    // Reset all
    ChunkBuffer()
    {
        buffer = nullptr;
        currentLength = 0;
        maximumLength = 1000;
    }

    // Delete the transmission if it exists
    ~ChunkBuffer()
    {
        deleteMessage();
    }

    // Set append limit (too high and you run the
    void setLimit(uint32_t max)
    {
        maximumLength = max;
    }

    // risk of allowing Denial of Service attacks)

    // Append some bytes
    void addMessage(uint8_t* msg, size_t len)
    {
        size_t newlen = currentLength + len;

        if (newlen <= maximumLength) {
            resizeMessage(newlen);
            memcpy(buffer + currentLength, msg, len);
            currentLength = newlen;
        }
    }

    // Delete the current transmission
    void deleteMessage()
    {
        if (buffer) {
            delete[] buffer;
            buffer = nullptr;
            currentLength = 0;
        }
    }
};
