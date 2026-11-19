#include "core/api/ChatAPI.h"
#include "core/native/ClientDb.h"
#include "core/native/ClientPacket.h"
#include "core/native/ObjectManager.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsGroup.h"
#include "offsets/OffsetsSocial.h"
#include <Windows.h>
#include <algorithm>
#include <cctype>
#include <deque>

namespace
{
    namespace C = Offsets::Chat;
    namespace B = Offsets::BattleNet;

    constexpr size_t MAX_WHISPERS = 20;

    struct ChannelType
    {
        const char* name;
        uint32_t    type;
    };

    const ChannelType CHANNELS[] = {
        { "SAY",          C::TYPE_SAY },
        { "PARTY",        C::TYPE_PARTY },
        { "RAID",         C::TYPE_RAID },
        { "GUILD",        C::TYPE_GUILD },
        { "OFFICER",      C::TYPE_OFFICER },
        { "YELL",         C::TYPE_YELL },
        { "WHISPER",      C::TYPE_WHISPER },
        { "EMOTE",        C::TYPE_EMOTE },
        { "CHANNEL",      C::TYPE_CHANNEL },
        { "RAID_WARNING", C::TYPE_RAID_WARNING },
        { "BATTLEGROUND", C::TYPE_BATTLEGROUND },
    };

    SRWLOCK                   s_whisperLock = SRWLOCK_INIT;
    std::deque<CoreAPI::WhisperRecord> s_whispers;

    std::string upper(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(),
                       [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return value;
    }

    std::optional<uint32_t> channelType(const std::string& channel)
    {
        std::string name = upper(channel);
        for (const ChannelType& entry : CHANNELS)
            if (name == entry.name) return entry.type;
        return std::nullopt;
    }

    std::string escapePipes(const std::string& text)
    {
        std::string out;
        out.reserve(text.size());
        for (char c : text)
        {
            out.push_back(c);
            if (c == '|') out.push_back('|');
        }
        return out;
    }

    uintptr_t localPlayer()
    {
        WoWGUID guid = WoW::GetLocalGUID();
        if (!guid.isValid()) return 0;
        return WoW::GetObjectByGUID(guid);
    }

    bool isGroupLeader()
    {
        WoWGUID leader = Memory::safeRead<WoWGUID>(Offsets::Group::PARTY_LEADER);
        return leader.isValid() && leader == WoW::GetLocalGUID();
    }

    bool inParty()
    {
        for (int i = 0; i < Offsets::Group::PARTY_MAX_MEMBERS; i++)
        {
            WoWGUID guid = Memory::safeRead<WoWGUID>(
                Offsets::Group::PARTY_PLAYER_GUIDS + static_cast<uintptr_t>(i) * sizeof(WoWGUID));
            if (guid.isValid()) return true;
        }
        return false;
    }

    bool inRaid()
    {
        return Memory::safeRead<int>(Offsets::Group::RAID_MEMBER_COUNT) > 0;
    }

    std::optional<std::string> channelName(const std::string& target)
    {
        bool numeric = !target.empty() && std::all_of(target.begin(), target.end(),
                                                      [](unsigned char c) { return std::isdigit(c) != 0; });
        if (!numeric) return target;

        int number = std::atoi(target.c_str());
        if (number < 1 || number > Memory::safeRead<int>(C::CHANNEL_COUNT)) return std::nullopt;

        uintptr_t table = Memory::safeRead<uintptr_t>(C::CHANNEL_TABLE);
        if (!table) return std::nullopt;
        uintptr_t entry = table + static_cast<uintptr_t>(number - 1) * C::CHANNEL_ENTRY_SIZE;
        if (Memory::safeRead<int>(entry + C::CHANNEL_ENTRY_NUMBER) != number) return std::nullopt;
        if (Memory::safeRead<int>(entry + C::CHANNEL_ENTRY_PENDING) != 0) return std::nullopt;

        std::string name = ClientDb::readCString(entry + C::CHANNEL_ENTRY_NAME, C::CHANNEL_ENTRY_PENDING - C::CHANNEL_ENTRY_NAME);
        if (name.empty()) return std::nullopt;
        return name;
    }

    bool isSilenced(uintptr_t player)
    {
        uintptr_t vtable = Memory::safeRead<uintptr_t>(player);
        uintptr_t function = Memory::safeRead<uintptr_t>(vtable + C::VTABLE_IS_SILENCED);
        if (!function) return false;
        using IsSilencedFn = char(__thiscall*)(uintptr_t);
        return reinterpret_cast<IsSilencedFn>(function)(player) != 0;
    }

    bool silenceApplies(uint32_t type)
    {
        return type == C::TYPE_SAY || type == C::TYPE_YELL || type == C::TYPE_EMOTE;
    }

    uint32_t defaultLanguage(uintptr_t player)
    {
        using LanguageFn = uint32_t(__thiscall*)(uintptr_t);
        return reinterpret_cast<LanguageFn>(C::DEFAULT_LANGUAGE)(player);
    }

    CoreAPI::ChatResult sendBattleNetWhisper(const std::string& target, const std::string& text)
    {
        bool numeric = !target.empty() && std::all_of(target.begin(), target.end(),
                                                      [](unsigned char c) { return std::isdigit(c) != 0; });
        if (!numeric) return CoreAPI::ChatResult::MissingTarget;
        uint32_t presenceId = static_cast<uint32_t>(std::strtoul(target.c_str(), nullptr, 10));

        using GetClientFn = uintptr_t(__cdecl*)();
        uintptr_t client = reinterpret_cast<GetClientFn>(B::GET_CLIENT)();
        if (!client) return CoreAPI::ChatResult::BattleNetUnavailable;

        uintptr_t vtable = Memory::safeRead<uintptr_t>(client);
        using StateFn = int(__thiscall*)(uintptr_t);
        using BusyFn = char(__thiscall*)(uintptr_t);
        auto state = reinterpret_cast<StateFn>(Memory::safeRead<uintptr_t>(vtable + B::VTABLE_STATE));
        auto busy = reinterpret_cast<BusyFn>(Memory::safeRead<uintptr_t>(vtable + B::VTABLE_IS_BUSY));
        if (!state || !busy || state(client) != B::STATE_CONNECTED || busy(client) != 0)
            return CoreAPI::ChatResult::BattleNetUnavailable;

        using CheckFn = char(__cdecl*)();
        if (!reinterpret_cast<CheckFn>(B::IS_READY)() || !reinterpret_cast<CheckFn>(B::CAN_SEND)())
            return CoreAPI::ChatResult::BattleNetUnavailable;

        char converted[B::TEXT_BUFFER_SIZE + 3] = {};
        using ConvertFn = void(__cdecl*)(const char*, char*, size_t);
        reinterpret_cast<ConvertFn>(B::CONVERT_TEXT)(text.c_str(), converted, B::TEXT_BUFFER_SIZE);

        int friendInfo[B::FRIEND_INFO_SIZE / sizeof(int)] = {};
        using FriendInfoFn = void(__cdecl*)(int*, uint32_t, uint32_t);
        reinterpret_cast<FriendInfoFn>(B::GET_FRIEND_INFO)(friendInfo, presenceId,
                                                           Memory::safeRead<uint32_t>(B::FRIEND_INFO_CONTEXT));
        if (friendInfo[0] != B::FRIEND_ONLINE) return CoreAPI::ChatResult::BattleNetFriendOffline;

        using SendFn = void(__cdecl*)(uint32_t, const char*);
        reinterpret_cast<SendFn>(B::SEND_WHISPER)(presenceId, converted);
        return CoreAPI::ChatResult::Ok;
    }
}

namespace CoreAPI
{
    namespace Chat
    {
        ChatSendResult Send(const std::string& channel, const std::string& target, const std::string& text,
                            std::optional<uint32_t> language)
        {
            ChatSendResult out = { ChatResult::Ok, upper(channel), target };

            uintptr_t player = localPlayer();
            if (!player) { out.result = ChatResult::NotInWorld; return out; }
            if (text.empty()) { out.result = ChatResult::EmptyMessage; return out; }

            if (out.channel == "BN_WHISPER")
            {
                out.result = sendBattleNetWhisper(target, text);
                return out;
            }

            std::optional<uint32_t> type = channelType(out.channel);
            if (!type) { out.result = ChatResult::UnknownChannel; return out; }

            std::string message = escapePipes(text);
            if (message.size() > C::MAX_MESSAGE_BYTES) { out.result = ChatResult::MessageTooLong; return out; }

            std::string destination;
            switch (*type)
            {
                case C::TYPE_WHISPER:
                    if (target.empty()) { out.result = ChatResult::MissingTarget; return out; }
                    destination = target;
                    break;
                case C::TYPE_CHANNEL:
                {
                    if (target.empty()) { out.result = ChatResult::MissingTarget; return out; }
                    std::optional<std::string> name = channelName(target);
                    if (!name) { out.result = ChatResult::ChannelNotJoined; return out; }
                    destination = *name;
                    out.target = *name;
                    break;
                }
                case C::TYPE_PARTY:
                    if (!inParty() && !inRaid()) { out.result = ChatResult::NotInGroup; return out; }
                    if (isGroupLeader()) type = C::TYPE_PARTY_LEADER;
                    break;
                case C::TYPE_RAID:
                    if (!inRaid()) { out.result = ChatResult::NotInRaid; return out; }
                    if (isGroupLeader()) type = C::TYPE_RAID_LEADER;
                    break;
                case C::TYPE_BATTLEGROUND:
                    if (!inRaid()) { out.result = ChatResult::NotInRaid; return out; }
                    if (isGroupLeader()) type = C::TYPE_BATTLEGROUND_LEADER;
                    break;
                case C::TYPE_RAID_WARNING:
                    if (!inParty() && !inRaid()) { out.result = ChatResult::NotInGroup; return out; }
                    break;
                default:
                    break;
            }

            if (silenceApplies(*type) && isSilenced(player)) { out.result = ChatResult::CannotSpeak; return out; }

            ClientPacket packet(C::OPCODE_MESSAGE);
            packet.writeU32(*type);
            packet.writeU32(language.value_or(defaultLanguage(player)));
            if (*type == C::TYPE_WHISPER || *type == C::TYPE_CHANNEL)
                packet.writeString(destination.c_str());
            packet.writeString(message.c_str());
            packet.send();
            return out;
        }

        ChatSendResult Reply(const std::string& text, std::optional<int64_t> lineId, const std::string& guid)
        {
            std::vector<WhisperRecord> whispers = RecentWhispers();
            auto match = whispers.rbegin();
            for (; match != whispers.rend(); ++match)
            {
                if (lineId && match->lineId != *lineId) continue;
                if (!guid.empty() && match->guid != guid) continue;
                break;
            }
            if (match == whispers.rend())
                return ChatSendResult{ ChatResult::NoWhisperToReply, "WHISPER", "" };

            if (match->battleNet)
                return Send("BN_WHISPER", std::to_string(match->presenceId), text, std::nullopt);
            return Send("WHISPER", match->author, text, std::nullopt);
        }

        void RecordWhisper(const WhisperRecord& whisper)
        {
            AcquireSRWLockExclusive(&s_whisperLock);
            s_whispers.push_back(whisper);
            while (s_whispers.size() > MAX_WHISPERS)
                s_whispers.pop_front();
            ReleaseSRWLockExclusive(&s_whisperLock);
        }

        std::vector<WhisperRecord> RecentWhispers()
        {
            AcquireSRWLockShared(&s_whisperLock);
            std::vector<WhisperRecord> copy(s_whispers.begin(), s_whispers.end());
            ReleaseSRWLockShared(&s_whisperLock);
            return copy;
        }

        const char* ResultMessage(ChatResult result)
        {
            switch (result)
            {
                case ChatResult::Ok:                     return "";
                case ChatResult::NotInWorld:             return "not in world";
                case ChatResult::UnknownChannel:         return "unknown channel";
                case ChatResult::MissingTarget:          return "target is required for this channel";
                case ChatResult::EmptyMessage:           return "text is empty";
                case ChatResult::MessageTooLong:         return "text is longer than 255 bytes";
                case ChatResult::NotInGroup:             return "not in a group";
                case ChatResult::NotInRaid:              return "not in a raid";
                case ChatResult::ChannelNotJoined:       return "channel not joined";
                case ChatResult::CannotSpeak:            return "cannot speak now";
                case ChatResult::BattleNetUnavailable:   return "battle.net is not available";
                case ChatResult::BattleNetFriendOffline: return "battle.net friend is not online";
                case ChatResult::NoWhisperToReply:       return "no whisper to reply to";
            }
            return "unknown error";
        }
    }
}
