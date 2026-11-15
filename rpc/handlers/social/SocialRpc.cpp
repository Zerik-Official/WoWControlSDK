#include "SocialRpc.h"
#include "SocialJson.h"
#include "hooks/GlueHooks.h"
#include "core/api/FriendsAPI.h"
#include "core/api/LfgAPI.h"
#include "core/api/SummonAPI.h"
#include "core/api/TradeAPI.h"
#include "core/api/DuelAPI.h"
#include "core/api/BattlefieldAPI.h"
#include "utils/json/Json.h"

namespace Rpc
{
    using SDK::Json;

    static Json handleFriendsList(const Json&)
    {
        std::string raw = Hooks::Glue::Execute([]() -> std::string {
            CoreAPI::FriendList list = CoreAPI::Friends::GetList();

            Json j;
            j["loaded"] = list.loaded;
            j["count"] = list.count;
            j["onlineCount"] = list.onlineCount;

            Json friends = Json::array();
            for (const auto& f : list.friends)
                friends.push_back(makeFriendJson(f));
            j["friends"] = friends;
            return j.dump();
        });

        return Json::parse(raw);
    }

    static Json summonResultJson(CoreAPI::SummonResult result)
    {
        switch (result)
        {
            case CoreAPI::SummonResult::Ok:             return SDK::okJson();
            case CoreAPI::SummonResult::NotInWorld:     return SDK::makeErrorJson("not in world");
            case CoreAPI::SummonResult::NoSummon:       return SDK::makeErrorJson("no pending summon");
            case CoreAPI::SummonResult::CannotTeleport: return SDK::makeErrorJson("cannot teleport now");
        }
        return SDK::makeErrorJson("unknown result");
    }

    static Json handleSummonGet(const Json&)
    {
        std::string raw = Hooks::Glue::Execute([]() -> std::string {
            Json j = makeSummonJson(CoreAPI::Summon::Get());
            j["canTeleport"] = CoreAPI::Summon::CanTeleport();
            return j.dump();
        });

        return Json::parse(raw);
    }

    static Json handleSummonAccept(const Json&)
    {
        std::string raw = Hooks::Glue::Execute([]() -> std::string {
            return summonResultJson(CoreAPI::Summon::Accept()).dump();
        });

        return Json::parse(raw);
    }

    static Json handleSummonDecline(const Json&)
    {
        std::string raw = Hooks::Glue::Execute([]() -> std::string {
            return summonResultJson(CoreAPI::Summon::Decline()).dump();
        });

        return Json::parse(raw);
    }

    static Json lfgResultJson(CoreAPI::LfgResult result)
    {
        switch (result)
        {
            case CoreAPI::LfgResult::Ok:         return SDK::okJson();
            case CoreAPI::LfgResult::NotInWorld: return SDK::makeErrorJson("not in world");
            case CoreAPI::LfgResult::NoProposal: return SDK::makeErrorJson("no pending proposal");
        }
        return SDK::makeErrorJson("unknown result");
    }

    static Json handleLfgGetProposal(const Json&)
    {
        std::string raw = Hooks::Glue::Execute([]() -> std::string {
            return makeLfgProposalJson(CoreAPI::Lfg::GetProposal()).dump();
        });

        return Json::parse(raw);
    }

    static Json handleLfgGetDungeon(const Json& params)
    {
        if (!params.is_object() || !params.contains("id") || !params["id"].is_number_integer())
            return SDK::makeErrorJson("id must be an integer");

        int id = params["id"].get<int>();
        std::string raw = Hooks::Glue::Execute([id]() -> std::string {
            CoreAPI::LfgDungeon dungeon = CoreAPI::Lfg::GetDungeon(id);
            if (dungeon.id == 0)
                return SDK::makeErrorJson("dungeon not found").dump();
            return makeLfgDungeonJson(dungeon).dump();
        });

        return Json::parse(raw);
    }

    static Json handleLfgAccept(const Json&)
    {
        std::string raw = Hooks::Glue::Execute([]() -> std::string {
            return lfgResultJson(CoreAPI::Lfg::AcceptProposal()).dump();
        });

        return Json::parse(raw);
    }

    static Json handleLfgReject(const Json&)
    {
        std::string raw = Hooks::Glue::Execute([]() -> std::string {
            return lfgResultJson(CoreAPI::Lfg::RejectProposal()).dump();
        });

        return Json::parse(raw);
    }

    template <typename Fn>
    static Json onGameThread(Fn task)
    {
        std::string raw = Hooks::Glue::Execute([task]() -> std::string {
            return task().dump();
        });
        return Json::parse(raw);
    }

    static Json tradeResultJson(CoreAPI::TradeResult result)
    {
        switch (result)
        {
            case CoreAPI::TradeResult::Ok:         return SDK::okJson();
            case CoreAPI::TradeResult::NotInWorld: return SDK::makeErrorJson("not in world");
            case CoreAPI::TradeResult::NoRequest:  return SDK::makeErrorJson("no pending trade request");
            case CoreAPI::TradeResult::NotTrading: return SDK::makeErrorJson("trade window is not open");
        }
        return SDK::makeErrorJson("unknown result");
    }

    static Json duelResultJson(CoreAPI::DuelResult result)
    {
        switch (result)
        {
            case CoreAPI::DuelResult::Ok:         return SDK::okJson();
            case CoreAPI::DuelResult::NotInWorld: return SDK::makeErrorJson("not in world");
            case CoreAPI::DuelResult::NoDuel:     return SDK::makeErrorJson("no pending duel");
        }
        return SDK::makeErrorJson("unknown result");
    }

    static Json battlefieldResultJson(CoreAPI::BattlefieldResult result)
    {
        switch (result)
        {
            case CoreAPI::BattlefieldResult::Ok:           return SDK::okJson();
            case CoreAPI::BattlefieldResult::NotInWorld:   return SDK::makeErrorJson("not in world");
            case CoreAPI::BattlefieldResult::InvalidIndex: return SDK::makeErrorJson("index must be 1 or 2");
            case CoreAPI::BattlefieldResult::NotReady:     return SDK::makeErrorJson("queue is not in the required status");
            case CoreAPI::BattlefieldResult::CannotLeave:  return SDK::makeErrorJson("this queue cannot be left from the invitation");
        }
        return SDK::makeErrorJson("unknown result");
    }

    static int readIndex(const Json& params)
    {
        if (!params.is_object() || !params.contains("index") || !params["index"].is_number_integer())
            return 0;
        return params["index"].get<int>();
    }

    static Json handleTradeGet(const Json&)
    {
        return onGameThread([]() { return makeTradeJson(CoreAPI::Trade::Get()); });
    }

    static Json handleTradeAcceptRequest(const Json&)
    {
        return onGameThread([]() { return tradeResultJson(CoreAPI::Trade::AcceptRequest()); });
    }

    static Json handleTradeDeclineRequest(const Json&)
    {
        return onGameThread([]() { return tradeResultJson(CoreAPI::Trade::DeclineRequest()); });
    }

    static Json handleTradeAccept(const Json&)
    {
        return onGameThread([]() { return tradeResultJson(CoreAPI::Trade::Accept()); });
    }

    static Json handleTradeUnaccept(const Json&)
    {
        return onGameThread([]() { return tradeResultJson(CoreAPI::Trade::Unaccept()); });
    }

    static Json handleTradeCancel(const Json&)
    {
        return onGameThread([]() { return tradeResultJson(CoreAPI::Trade::Cancel()); });
    }

    static Json handleDuelGet(const Json&)
    {
        return onGameThread([]() { return makeDuelJson(CoreAPI::Duel::Get()); });
    }

    static Json handleDuelAccept(const Json&)
    {
        return onGameThread([]() { return duelResultJson(CoreAPI::Duel::Accept()); });
    }

    static Json handleDuelDecline(const Json&)
    {
        return onGameThread([]() { return duelResultJson(CoreAPI::Duel::Decline()); });
    }

    static Json handleBattlefieldGetQueues(const Json&)
    {
        return onGameThread([]() {
            Json queues = Json::array();
            for (const auto& q : CoreAPI::Battlefield::GetQueues())
                queues.push_back(makeBattlefieldQueueJson(q));
            Json j;
            j["queues"] = queues;
            return j;
        });
    }

    static Json handleBattlefieldAcceptPort(const Json& params)
    {
        int index = readIndex(params);
        return onGameThread([index]() { return battlefieldResultJson(CoreAPI::Battlefield::AcceptPort(index)); });
    }

    static Json handleBattlefieldLeaveQueue(const Json& params)
    {
        int index = readIndex(params);
        return onGameThread([index]() { return battlefieldResultJson(CoreAPI::Battlefield::LeaveQueue(index)); });
    }

    void registerSocialMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("trade.get", handleTradeGet);
        registry.registerMethod("trade.acceptRequest", handleTradeAcceptRequest);
        registry.registerMethod("trade.declineRequest", handleTradeDeclineRequest);
        registry.registerMethod("trade.accept", handleTradeAccept);
        registry.registerMethod("trade.unaccept", handleTradeUnaccept);
        registry.registerMethod("trade.cancel", handleTradeCancel);
        registry.registerMethod("duel.get", handleDuelGet);
        registry.registerMethod("duel.accept", handleDuelAccept);
        registry.registerMethod("duel.decline", handleDuelDecline);
        registry.registerMethod("battlefield.getQueues", handleBattlefieldGetQueues);
        registry.registerMethod("battlefield.acceptPort", handleBattlefieldAcceptPort);
        registry.registerMethod("battlefield.leaveQueue", handleBattlefieldLeaveQueue);
        registry.registerMethod("friends.list", handleFriendsList);
        registry.registerMethod("summon.get", handleSummonGet);
        registry.registerMethod("summon.accept", handleSummonAccept);
        registry.registerMethod("summon.decline", handleSummonDecline);
        registry.registerMethod("lfg.getProposal", handleLfgGetProposal);
        registry.registerMethod("lfg.getDungeon", handleLfgGetDungeon);
        registry.registerMethod("lfg.acceptProposal", handleLfgAccept);
        registry.registerMethod("lfg.rejectProposal", handleLfgReject);
    }
}
