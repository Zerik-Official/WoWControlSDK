#pragma once

#include <cstdint>

class ClientPacket
{
public:
    explicit ClientPacket(uint32_t opcode);
    ~ClientPacket();

    ClientPacket(const ClientPacket&) = delete;
    ClientPacket& operator=(const ClientPacket&) = delete;

    void writeU8(uint8_t value);
    void writeU32(uint32_t value);
    void writeU64(uint32_t low, uint32_t high);
    void send();

private:
    struct DataStore
    {
        uintptr_t vtable;
        uintptr_t buffer;
        uint32_t  base;
        int32_t   alloc;
        uint32_t  size;
        int32_t   read;
    };

    DataStore m_store;
};
