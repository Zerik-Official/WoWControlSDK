#include "core/native/ClientPacket.h"
#include "offsets/OffsetsSocial.h"

ClientPacket::ClientPacket(uint32_t opcode)
    : m_store{ Offsets::Packet::DATA_STORE_VTABLE, 0, 0, 0, 0, -1 }
{
    writeU32(opcode);
}

ClientPacket::~ClientPacket()
{
    m_store.vtable = Offsets::Packet::DATA_STORE_VTABLE;
    if (m_store.alloc == -1)
        return;

    using FreeFn = void(__thiscall*)(DataStore*, uintptr_t*, uint32_t*, int32_t*);
    auto release = *reinterpret_cast<FreeFn*>(Offsets::Packet::DATA_STORE_FREE_SLOT);
    release(&m_store, &m_store.buffer, &m_store.base, &m_store.alloc);
}

void ClientPacket::writeU8(uint8_t value)
{
    using WriteFn = void(__thiscall*)(DataStore*, uint8_t);
    reinterpret_cast<WriteFn>(Offsets::Packet::WRITE_U8)(&m_store, value);
}

void ClientPacket::writeU32(uint32_t value)
{
    using WriteFn = void(__thiscall*)(DataStore*, uint32_t);
    reinterpret_cast<WriteFn>(Offsets::Packet::WRITE_U32)(&m_store, value);
}

void ClientPacket::writeU64(uint32_t low, uint32_t high)
{
    using WriteFn = void(__thiscall*)(DataStore*, uint32_t, uint32_t);
    reinterpret_cast<WriteFn>(Offsets::Packet::WRITE_U64)(&m_store, low, high);
}

void ClientPacket::writeString(const char* value)
{
    using WriteFn = void(__thiscall*)(DataStore*, const char*);
    reinterpret_cast<WriteFn>(Offsets::Packet::WRITE_STRING)(&m_store, value);
}

void ClientPacket::send()
{
    m_store.read = 0;
    using SendFn = void(__cdecl*)(DataStore*);
    reinterpret_cast<SendFn>(Offsets::Packet::SEND)(&m_store);
}
