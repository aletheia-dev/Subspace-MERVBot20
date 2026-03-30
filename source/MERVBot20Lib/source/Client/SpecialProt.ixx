//
// Special protocol security services by cat02e@fsu.edu
// -------------------------------------------------------
// This library implements all the SubSpace core protocol
// features in complete safety from exploitation, use only
// in combination with <m_host.h>
//
export module SpecialProt;

import Algorithms;
import Host;

import <compare>;
import <vector>;


#ifndef DEFLATE_CLASS
#define DEFLATE_CLASS \
    Host* h = m.src; \
    std::vector<uint8_t>& msg = m.msg; \
    std::size_t len = msg.size();
#endif


//////// Special handlers ////////

export void __stdcall handleSpecialUnknown(HostMessage& m)
{
    DEFLATE_CLASS

    h->logWarning("Unknown special message {}({})", msg[1], len);
    h->logIncoming(msg, len);
}


// 00 "Request some of the core protocol"
export void __stdcall handleSpecialHeader(HostMessage& m)
{
    DEFLATE_CLASS

    if (len >= 2) {
        h->gotSpecialMessage(m);
    }
    else {
        h->logWarning("Malformed special header ignored.");
    }
}


//////// Core protocol handshake ////////

// 01 <Client key(4)> <Protocol version(2)>
export void __stdcall handleEncryptRequest(HostMessage& m)
{
    DEFLATE_CLASS

    if (len == 8) {
        h->gotEncryptRequest(getLong(msg, 2), getShort(msg, 6));
    }
    else {
        h->logWarning("Malformed encryption request ignored.");
    }
}


// 02 <Session encryption key(4)> [Continuum mudge(1)]
export void __stdcall handleEncryptResponse(HostMessage& m)
{
    DEFLATE_CLASS

    if (len == 6) {
        h->gotEncryptResponse(getLong(msg, 2));
    }
    else if (len >= 7) {
        h->gotEncryptResponse(getLong(msg, 2), getByte(msg, 6));
    }
    else {
        h->logWarning("Malformed encryption response ignored.");
    }
}

// 07 "Session termination"
export void __stdcall handleDisconnect(HostMessage& m)
{
    DEFLATE_CLASS

    if (len == 2) {
        h->gotDisconnect();
    }
    else {
        h->logWarning("Malformed disconnection ignored.");
    }
}


//////// Reliable messaging ////////

// 03 <ACK_ID(4)> <Message(...)>
export void __stdcall handleReliable(HostMessage& m)
{
    DEFLATE_CLASS

    if (len >= 7) {
        h->gotReliable(getLong(msg, 2), &msg[6], len - 6);
    }
    else {
        h->logWarning("Malformed reliable header ignored.");
    }
}


// 04 <ACK_ID(4)>
export void __stdcall handleACK(HostMessage& m)
{
    DEFLATE_CLASS

    if (len == 6) {
        h->gotACK(getLong(msg, 2));
    }
    else {
        h->logWarning("Malformed acknowledgement ignored.");
    }
}


//////// Time synchronization ////////

// 05 <Client time1(4)> [Total packets sent(4)] [Total packets recv'd(4)]
export void __stdcall handleSyncRequest(HostMessage& m)
{
    DEFLATE_CLASS

    if (len == 6) {
        h->gotSyncRequest(getLong(msg, 2));
    }
    else if (len == 14) {
        h->gotSyncRequest(getLong(msg, 2), getLong(msg, 6), getLong(msg, 10));
    }
    else {
        h->logWarning("Malformed synchronization request ignored.");
    }
}


// 06 <Server time(4)> <Client time1(4)>
export void __stdcall handleSyncResponse(HostMessage& m)
{
    DEFLATE_CLASS

    if (len == 10) {
        h->gotSyncResponse(getLong(msg, 2), getLong(msg, 6));
    }
    else {
        h->logWarning("Malformed synchronization response ignored.");
    }
}


//////// Oversized packets ////////

// 08 <Message chunk(...)>
export void __stdcall handleChunkBody(HostMessage& m)
{
    DEFLATE_CLASS

    if (len >= 3) {
        h->gotChunkBody(&msg[2], len - 2);
    }
    else {
        h->logWarning("Malformed chunk body ignored.");
    }
}


// 09 <Message chunk(...)> "Process accumulated buffer as a message"
export void __stdcall handleChunkTail(HostMessage& m)
{
    DEFLATE_CLASS

    if (len >= 3) {
        h->gotChunkTail(&msg[2], len - 2);
    }
    else {
        h->logWarning("Malformed chunk tail ignored.");
    }
}


//////// File transfer ////////

// 0A <Total length(4)> <Message chunk(...)> "Buffer contains up to 20 MB"
export void __stdcall handleBigChunk(HostMessage& m)
{
    DEFLATE_CLASS

    if (len >= 7) {
        h->gotBigChunk(getLong(msg, 2), &msg[6], len - 6);
    }
    else {
        h->logWarning("Malformed big chunk ignored.");
    }
}


// 0B "Nevermind about the 00 0a transfer stuff i was sending you" (Snrrrub)
export void __stdcall handleCancelDownload(HostMessage& m)
{
    DEFLATE_CLASS

    if (len == 2) {
        h->gotCancelDownload();
    }
    else {
        h->logWarning("Malformed cancel download ignored.");
    }
}


// 0C "Cancel received successfully" (Snrrrub)
export void __stdcall handleCancelDownloadAck(HostMessage& m)
{
    DEFLATE_CLASS

    if (len == 2) {
        h->gotCancelDownloadAck();
    }
    else {
        h->logWarning("Malformed cancel download acknowledgement ignored.");
    }
}


// 0E <Message length(1)> <Message(...)> "Carpool into one packet"
export void __stdcall handleCluster(HostMessage& m)
{
    DEFLATE_CLASS

    if (len >= 6) {    // Require at least two messages to be clustered
        h->gotCluster(&msg[2], len - 2);
    }
    else {
        h->logWarning("Malformed cluster ignored (one or no messages).");
    }
}
