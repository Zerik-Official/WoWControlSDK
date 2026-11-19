#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace CoreAPI
{
    enum class ChatResult
    {
        Ok,
        NotInWorld,
        UnknownChannel,
        MissingTarget,
        EmptyMessage,
        MessageTooLong,
        NotInGroup,
        NotInRaid,
        ChannelNotJoined,
        CannotSpeak,
        BattleNetUnavailable,
        BattleNetFriendOffline,
        NoWhisperToReply
    };

    struct WhisperRecord
    {
        std::string author;
        bool        battleNet;
        uint32_t    presenceId;
        int64_t     lineId;
        std::string guid;
        uint64_t    receivedAtMs;
    };

    struct ChatSendResult
    {
        ChatResult  result;
        std::string channel;
        std::string target;
    };

    namespace Chat
    {
        ChatSendResult Send(const std::string& channel, const std::string& target, const std::string& text,
                            std::optional<uint32_t> language);
        ChatSendResult Reply(const std::string& text, std::optional<int64_t> lineId,
                             const std::string& guid);

        void                       RecordWhisper(const WhisperRecord& whisper);
        std::vector<WhisperRecord> RecentWhispers();
        const char*                ResultMessage(ChatResult result);
    }
}
