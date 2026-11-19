#include "runtime/events/GameEventBridge.h"
#include "runtime/events/EventPipe.h"
#include "rpc/handlers/social/SocialJson.h"
#include "core/api/FriendsAPI.h"
#include "core/api/GroupAPI.h"
#include "core/api/LfgAPI.h"
#include "core/api/SummonAPI.h"
#include "core/api/TradeAPI.h"
#include "core/api/DuelAPI.h"
#include "core/api/BattlefieldAPI.h"
#include "core/api/ChatAPI.h"
#include "core/native/ObjectManager.h"
#include "offsets/OffsetsSocial.h"
#include <Windows.h>
#include <unordered_map>
#include <unordered_set>

namespace Runtime
{
    namespace GameEventBridge
    {
        namespace
        {
            struct FriendState
            {
                bool online;
                bool afk;
                bool dnd;
            };

            const std::unordered_set<std::string> HANDLED_EVENTS = {
                "PARTY_INVITE_REQUEST",
                "PARTY_INVITE_CANCEL",
                "PARTY_MEMBERS_CHANGED",
                "CONFIRM_SUMMON",
                "CANCEL_SUMMON",
                "LFG_PROPOSAL_SHOW",
                "LFG_PROPOSAL_UPDATE",
                "LFG_PROPOSAL_FAILED",
                "LFG_PROPOSAL_SUCCEEDED",
                "FRIENDLIST_UPDATE",
                "CHAT_MSG_WHISPER",
                "CHAT_MSG_BN_WHISPER",
                "TRADE_REQUEST",
                "TRADE_REQUEST_CANCEL",
                "TRADE_SHOW",
                "TRADE_CLOSED",
                "TRADE_UPDATE",
                "TRADE_ACCEPT_UPDATE",
                "TRADE_PLAYER_ITEM_CHANGED",
                "TRADE_TARGET_ITEM_CHANGED",
                "TRADE_MONEY_CHANGED",
                "DUEL_REQUESTED",
                "DUEL_FINISHED",
                "DUEL_OUTOFBOUNDS",
                "DUEL_INBOUNDS",
                "UPDATE_BATTLEFIELD_STATUS"
            };

            std::unordered_map<std::string, int> s_suppressed;
            SRWLOCK s_suppressLock = SRWLOCK_INIT;
            int s_battlefieldStatus[Offsets::Battlefield::MAX_QUEUES] = {};
            bool s_tradeOpen = false;
            bool s_tradeBothAccepted = false;

            bool consumeSuppressed(const std::string& name)
            {
                AcquireSRWLockExclusive(&s_suppressLock);
                auto it = s_suppressed.find(name);
                bool suppressed = it != s_suppressed.end() && it->second > 0;
                if (suppressed && --it->second == 0)
                    s_suppressed.erase(it);
                ReleaseSRWLockExclusive(&s_suppressLock);
                return suppressed;
            }

            std::unordered_map<uint64_t, FriendState> s_friends;
            uint64_t s_friendsOwner   = 0;
            bool     s_summonPending  = false;

            uint64_t now()
            {
                return static_cast<uint64_t>(GetTickCount64());
            }

            std::string argString(const SDK::Json& args, size_t index)
            {
                if (!args.is_array() || index >= args.size() || !args[index].is_string())
                    return {};
                return args[index].get<std::string>();
            }

            int64_t argInteger(const SDK::Json& args, size_t index)
            {
                if (!args.is_array() || index >= args.size()) return 0;
                const SDK::Json& value = args[index];
                if (value.is_number_integer()) return value.get<int64_t>();
                if (value.is_number()) return static_cast<int64_t>(value.get<double>());
                if (value.is_string()) return std::strtoll(value.get<std::string>().c_str(), nullptr, 10);
                return 0;
            }

            SDK::Json argValue(const SDK::Json& args, size_t index)
            {
                if (!args.is_array() || index >= args.size())
                    return nullptr;
                return args[index];
            }

            void closeInvite(const char* reason, uint32_t roles)
            {
                CoreAPI::GroupInvite invite = CoreAPI::Group::GetInvite();
                if (!invite.pending)
                    return;

                CoreAPI::Group::ClearInvite();
                SDK::Json data;
                data["inviterName"] = invite.inviterName;
                data["reason"] = reason;
                data["roles"] = Rpc::makeRolesJson(roles);
                EventPipe::PushEvent("group.inviteClosed", data, now());
            }

            void onInviteRequest(const SDK::Json& args, uint64_t timeMs)
            {
                std::string inviter = argString(args, 0);
                CoreAPI::Group::SetInvite(inviter, timeMs);

                SDK::Json data;
                data["inviterName"] = inviter;
                EventPipe::PushEvent("group.inviteReceived", data, timeMs);
            }

            void onPartyMembersChanged()
            {
                CoreAPI::GroupInvite invite = CoreAPI::Group::GetInvite();
                if (invite.pending && CoreAPI::Group::IsInGroup())
                    closeInvite("joined", 0);
            }

            void onConfirmSummon(uint64_t timeMs)
            {
                s_summonPending = true;
                EventPipe::PushEvent("summon.requested", Rpc::makeSummonJson(CoreAPI::Summon::Get()), timeMs);
            }

            void closeSummon(const char* reason)
            {
                if (!s_summonPending)
                    return;

                s_summonPending = false;
                SDK::Json data = Rpc::makeSummonJson(CoreAPI::Summon::Get());
                data["reason"] = reason;
                EventPipe::PushEvent("summon.closed", data, now());
            }

            void onProposal(const char* sdkEvent, uint64_t timeMs)
            {
                EventPipe::PushEvent(sdkEvent, Rpc::makeLfgProposalJson(CoreAPI::Lfg::GetProposal()), timeMs);
            }

            void onFriendListUpdate(uint64_t timeMs)
            {
                CoreAPI::FriendList list = CoreAPI::Friends::GetList();
                uint64_t owner = WoW::GetLocalGUID().raw();

                std::unordered_map<uint64_t, FriendState> next;
                for (const CoreAPI::FriendInfo& f : list.friends)
                    next[f.guid.raw()] = FriendState{ f.online, f.afk, f.dnd };

                bool sameSession = owner != 0 && owner == s_friendsOwner;
                if (sameSession)
                {
                    for (const CoreAPI::FriendInfo& f : list.friends)
                    {
                        auto previous = s_friends.find(f.guid.raw());
                        if (previous == s_friends.end())
                            continue;

                        const FriendState& before = previous->second;
                        if (!before.online && f.online)
                            EventPipe::PushEvent("friends.online", Rpc::makeFriendJson(f), timeMs);
                        else if (before.online && !f.online)
                            EventPipe::PushEvent("friends.offline", Rpc::makeFriendJson(f), timeMs);
                        else if (f.online && (before.afk != f.afk || before.dnd != f.dnd))
                            EventPipe::PushEvent("friends.statusChanged", Rpc::makeFriendJson(f), timeMs);
                    }
                }

                s_friends = std::move(next);
                s_friendsOwner = owner;
            }

            void onWhisper(const SDK::Json& args, bool battleNet, uint64_t timeMs)
            {
                SDK::Json data;
                data["battleNet"] = battleNet;
                data["text"] = argString(args, 0);
                data["author"] = argString(args, 1);
                data["language"] = argString(args, 2);
                data["status"] = argString(args, 5);
                data["lineId"] = argValue(args, 10);
                data["guid"] = argValue(args, 11);
                data["presenceId"] = battleNet ? argValue(args, 12) : SDK::Json(nullptr);
                data["args"] = args;

                CoreAPI::WhisperRecord record = {};
                record.author       = argString(args, 1);
                record.battleNet    = battleNet;
                record.lineId       = argInteger(args, 10);
                record.guid         = argString(args, 11);
                record.presenceId   = battleNet ? static_cast<uint32_t>(argInteger(args, 12)) : 0;
                record.receivedAtMs = timeMs;
                CoreAPI::Chat::RecordWhisper(record);

                EventPipe::PushEvent("chat.whisperReceived", data, timeMs);
            }

            void closeTradeRequest(const char* reason)
            {
                CoreAPI::TradeRequest request = CoreAPI::Trade::GetRequest();
                if (!request.pending)
                    return;

                CoreAPI::Trade::ClearRequest();
                SDK::Json data;
                data["requesterName"] = request.requesterName;
                data["reason"] = reason;
                EventPipe::PushEvent("trade.requestClosed", data, now());
            }

            void onTradeRequest(const SDK::Json& args, uint64_t timeMs)
            {
                std::string requester = argString(args, 0);
                CoreAPI::Trade::SetRequest(requester, timeMs);

                SDK::Json data;
                data["requesterName"] = requester;
                EventPipe::PushEvent("trade.requested", data, timeMs);
            }

            void onTradeShow(uint64_t timeMs)
            {
                closeTradeRequest("accepted");
                CoreAPI::TradeInfo trade = CoreAPI::Trade::Get();
                s_tradeOpen = true;
                s_tradeBothAccepted = false;
                EventPipe::PushEvent("trade.opened", Rpc::makeTradeJson(trade), timeMs);
            }

            void onTradeChanged(const char* change, uint64_t timeMs)
            {
                if (!s_tradeOpen)
                    return;

                CoreAPI::TradeInfo trade = CoreAPI::Trade::Get();
                s_tradeBothAccepted = trade.playerAccepted && trade.targetAccepted;
                SDK::Json data = Rpc::makeTradeJson(trade);
                data["change"] = change;
                EventPipe::PushEvent("trade.updated", data, timeMs);
            }

            void onTradeClosed(uint64_t timeMs)
            {
                closeTradeRequest("closed");
                if (!s_tradeOpen)
                    return;

                s_tradeOpen = false;
                SDK::Json data;
                data["bothAccepted"] = s_tradeBothAccepted;
                EventPipe::PushEvent("trade.closed", data, timeMs);
            }

            void onDuelRequested(const SDK::Json& args, uint64_t timeMs)
            {
                CoreAPI::Duel::SetRequested(argString(args, 0), timeMs);
                EventPipe::PushEvent("duel.requested", Rpc::makeDuelJson(CoreAPI::Duel::Get()), timeMs);
            }

            void onDuelFinished(uint64_t timeMs)
            {
                CoreAPI::DuelInfo duel = CoreAPI::Duel::Get();
                if (!duel.pending && !duel.active)
                    return;

                CoreAPI::Duel::SetFinished();
                SDK::Json data = Rpc::makeDuelJson(duel);
                data["reason"] = duel.pending ? "cancelled" : "finished";
                EventPipe::PushEvent("duel.closed", data, timeMs);
            }

            void onDuelBounds(bool inBounds, uint64_t timeMs)
            {
                SDK::Json data;
                data["inBounds"] = inBounds;
                EventPipe::PushEvent("duel.boundsChanged", data, timeMs);
            }

            void onBattlefieldStatus(uint64_t timeMs)
            {
                for (int i = 1; i <= Offsets::Battlefield::MAX_QUEUES; i++)
                {
                    CoreAPI::BattlefieldQueue queue = CoreAPI::Battlefield::GetQueue(i);
                    int previous = s_battlefieldStatus[i - 1];
                    if (queue.statusId == previous)
                        continue;

                    s_battlefieldStatus[i - 1] = queue.statusId;
                    SDK::Json data = Rpc::makeBattlefieldQueueJson(queue);
                    data["previousStatusId"] = previous;
                    EventPipe::PushEvent("battlefield.statusChanged", data, timeMs);
                    if (queue.statusId == 2)
                        EventPipe::PushEvent("battlefield.ready", Rpc::makeBattlefieldQueueJson(queue), timeMs);
                }
            }
        }

        bool Handles(const std::string& eventName)
        {
            return HANDLED_EVENTS.find(eventName) != HANDLED_EVENTS.end();
        }

        std::vector<std::string> HandledEvents()
        {
            return std::vector<std::string>(HANDLED_EVENTS.begin(), HANDLED_EVENTS.end());
        }

        void OnGameEvent(const std::string& eventName, const SDK::Json& args, uint64_t timeMs)
        {
            if (consumeSuppressed(eventName))
                return;

            if (eventName == "PARTY_INVITE_REQUEST")        onInviteRequest(args, timeMs);
            else if (eventName == "PARTY_INVITE_CANCEL")    closeInvite("cancelled", 0);
            else if (eventName == "PARTY_MEMBERS_CHANGED")  onPartyMembersChanged();
            else if (eventName == "CONFIRM_SUMMON")         onConfirmSummon(timeMs);
            else if (eventName == "CANCEL_SUMMON")          closeSummon("cancelled");
            else if (eventName == "LFG_PROPOSAL_SHOW")      onProposal("lfg.proposalShown", timeMs);
            else if (eventName == "LFG_PROPOSAL_UPDATE")    onProposal("lfg.proposalUpdated", timeMs);
            else if (eventName == "LFG_PROPOSAL_FAILED")    onProposal("lfg.proposalFailed", timeMs);
            else if (eventName == "LFG_PROPOSAL_SUCCEEDED") onProposal("lfg.proposalSucceeded", timeMs);
            else if (eventName == "FRIENDLIST_UPDATE")      onFriendListUpdate(timeMs);
            else if (eventName == "CHAT_MSG_WHISPER")       onWhisper(args, false, timeMs);
            else if (eventName == "CHAT_MSG_BN_WHISPER")    onWhisper(args, true, timeMs);
            else if (eventName == "TRADE_REQUEST")          onTradeRequest(args, timeMs);
            else if (eventName == "TRADE_REQUEST_CANCEL")   closeTradeRequest("cancelled");
            else if (eventName == "TRADE_SHOW")             onTradeShow(timeMs);
            else if (eventName == "TRADE_CLOSED")           onTradeClosed(timeMs);
            else if (eventName == "TRADE_UPDATE")           onTradeChanged("update", timeMs);
            else if (eventName == "TRADE_ACCEPT_UPDATE")    onTradeChanged("accept", timeMs);
            else if (eventName == "TRADE_PLAYER_ITEM_CHANGED") onTradeChanged("playerItem", timeMs);
            else if (eventName == "TRADE_TARGET_ITEM_CHANGED") onTradeChanged("targetItem", timeMs);
            else if (eventName == "TRADE_MONEY_CHANGED")    onTradeChanged("money", timeMs);
            else if (eventName == "DUEL_REQUESTED")         onDuelRequested(args, timeMs);
            else if (eventName == "DUEL_FINISHED")          onDuelFinished(timeMs);
            else if (eventName == "DUEL_OUTOFBOUNDS")       onDuelBounds(false, timeMs);
            else if (eventName == "DUEL_INBOUNDS")          onDuelBounds(true, timeMs);
            else if (eventName == "UPDATE_BATTLEFIELD_STATUS") onBattlefieldStatus(timeMs);
        }

        void OnInviteAnswered(bool accepted, uint32_t roles)
        {
            closeInvite(accepted ? "accepted" : "declined", roles);
        }

        void OnSummonAnswered(bool accepted)
        {
            closeSummon(accepted ? "accepted" : "declined");
        }

        void OnTradeRequestAnswered(bool accepted)
        {
            closeTradeRequest(accepted ? "accepted" : "declined");
        }

        void OnDuelAnswered(bool accepted)
        {
            CoreAPI::DuelInfo duel = CoreAPI::Duel::Get();
            if (!duel.pending)
            {
                if (!accepted && duel.active)
                {
                    CoreAPI::Duel::SetFinished();
                    SDK::Json data = Rpc::makeDuelJson(duel);
                    data["reason"] = "forfeited";
                    EventPipe::PushEvent("duel.closed", data, now());
                }
                return;
            }

            CoreAPI::Duel::SetAnswered(accepted);
            SDK::Json data = Rpc::makeDuelJson(duel);
            data["reason"] = accepted ? "accepted" : "declined";
            EventPipe::PushEvent(accepted ? "duel.accepted" : "duel.closed", data, now());
            if (!accepted)
                CoreAPI::Duel::SetFinished();
        }

        void Suppress(const char* gameEvent)
        {
            if (!gameEvent) return;
            AcquireSRWLockExclusive(&s_suppressLock);
            s_suppressed[gameEvent]++;
            ReleaseSRWLockExclusive(&s_suppressLock);
        }

        void Unsuppress(const char* gameEvent)
        {
            if (!gameEvent) return;
            AcquireSRWLockExclusive(&s_suppressLock);
            auto it = s_suppressed.find(gameEvent);
            if (it != s_suppressed.end() && --it->second <= 0)
                s_suppressed.erase(it);
            ReleaseSRWLockExclusive(&s_suppressLock);
        }
    }
}
