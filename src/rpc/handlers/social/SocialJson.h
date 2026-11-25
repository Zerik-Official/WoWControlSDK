#pragma once

#include "core/api/FriendsAPI.h"
#include "core/api/LfgAPI.h"
#include "core/api/SummonAPI.h"
#include "core/api/TradeAPI.h"
#include "core/api/DuelAPI.h"
#include "core/api/BattlefieldAPI.h"
#include "core/native/PlayerInfo.h"
#include "offsets/OffsetsSocial.h"
#include "utils/json/Json.h"

namespace Rpc
{

inline SDK::Json makeGuidArrayJson(const WoWGUID& guid)
{
    return SDK::Json::array({guid.high, guid.low});
}

inline SDK::Json makeRolesJson(uint32_t roles)
{
    SDK::Json j;
    j["flags"] = roles;
    j["tank"] = (roles & Offsets::Lfg::ROLE_TANK) != 0;
    j["healer"] = (roles & Offsets::Lfg::ROLE_HEALER) != 0;
    j["damage"] = (roles & Offsets::Lfg::ROLE_DAMAGE) != 0;
    return j;
}

inline SDK::Json makeFriendJson(const CoreAPI::FriendInfo& f)
{
    SDK::Json j;
    j["index"] = f.index;
    j["guid"] = makeGuidArrayJson(f.guid);
    j["name"] = f.name;
    j["online"] = f.online;
    j["afk"] = f.afk;
    j["dnd"] = f.dnd;
    j["referAFriend"] = f.referAFriend;
    j["flags"] = f.flags;
    j["note"] = f.note;
    j["level"] = f.level;
    j["classId"] = f.classId;
    j["className"] = f.className;
    j["classFileName"] = f.classFileName;
    j["areaId"] = f.areaId;
    j["areaName"] = f.areaName;
    j["player"] = PlayerInfo::toJson(f.guid);
    return j;
}

inline SDK::Json makeSummonJson(const CoreAPI::SummonInfo& s)
{
    SDK::Json j;
    j["pending"] = s.pending;
    j["summonerGuid"] = makeGuidArrayJson(s.summonerGuid);
    j["summonerName"] = s.summonerName;
    j["areaId"] = s.areaId;
    j["areaName"] = s.areaName;
    j["timeLeftMs"] = s.timeLeftMs;
    j["player"] = PlayerInfo::toJson(s.summonerGuid);
    return j;
}

inline SDK::Json makeLfgDungeonJson(const CoreAPI::LfgDungeon& d)
{
    SDK::Json j;
    j["id"] = d.id;
    j["name"] = d.name;
    j["description"] = d.description;
    j["typeId"] = d.typeId;
    j["random"] = d.random;
    j["holiday"] = d.holiday;
    j["flags"] = d.flags;
    j["mapId"] = d.mapId;
    j["difficulty"] = d.difficulty;
    j["minLevel"] = d.minLevel;
    j["maxLevel"] = d.maxLevel;
    j["targetLevel"] = d.targetLevel;
    j["targetLevelMin"] = d.targetLevelMin;
    j["targetLevelMax"] = d.targetLevelMax;
    j["faction"] = d.faction;
    j["texture"] = d.texture;
    j["expansion"] = d.expansion;
    j["order"] = d.order;
    j["groupId"] = d.groupId;
    return j;
}

inline SDK::Json makeLfgProposalJson(const CoreAPI::LfgProposal& p)
{
    SDK::Json j;
    j["exists"] = p.exists;
    if (!p.exists)
        return j;

    j["proposalId"] = p.proposalId;
    j["dungeonId"] = p.dungeonId;
    j["proposalType"] = p.proposalType;
    j["state"] = p.state;
    j["dungeon"] = makeLfgDungeonJson(p.dungeon);
    j["role"] = p.role;
    j["isLeader"] = p.isLeader;
    j["responded"] = p.responded;
    j["accepted"] = p.accepted;
    j["bossesKilledMask"] = p.bossesKilledMask;
    j["totalEncounters"] = p.totalEncounters;
    j["completedEncounters"] = p.completedEncounters;

    SDK::Json members = SDK::Json::array();
    for (const CoreAPI::LfgProposalMember& m : p.members)
    {
        SDK::Json member;
        member["index"] = m.index;
        member["isPlayer"] = m.isPlayer;
        member["isLeader"] = m.isLeader;
        member["role"] = m.role;
        member["roles"] = makeRolesJson(m.roleFlags);
        member["responded"] = m.responded;
        member["accepted"] = m.accepted;
        members.push_back(member);
    }
    j["members"] = members;

    SDK::Json encounters = SDK::Json::array();
    for (const CoreAPI::LfgEncounter& e : p.encounters)
    {
        SDK::Json encounter;
        encounter["id"] = e.id;
        encounter["name"] = e.name;
        encounter["order"] = e.order;
        encounter["bit"] = e.bit;
        encounter["iconTexture"] = e.iconTexture;
        encounter["completed"] = e.completed;
        encounters.push_back(encounter);
    }
    j["encounters"] = encounters;
    return j;
}

inline SDK::Json makeTradeItemJson(const CoreAPI::TradeItem& item)
{
    SDK::Json j;
    j["slot"] = item.slot;
    j["exists"] = item.exists;
    j["willNotBeTraded"] = item.willNotBeTraded;
    if (!item.exists)
        return j;

    j["guid"] = makeGuidArrayJson(item.guid);
    j["itemId"] = item.itemId;
    j["cached"] = item.cached;
    j["name"] = item.name;
    j["link"] = item.link;
    j["quality"] = item.quality;
    j["count"] = item.count;
    j["enchantId"] = item.enchantId;
    j["gemIds"] = SDK::Json::array({item.gemIds[0], item.gemIds[1], item.gemIds[2]});
    j["suffixId"] = item.suffixId;
    j["uniqueId"] = item.uniqueId;
    return j;
}

inline SDK::Json makeTradeJson(const CoreAPI::TradeInfo& t)
{
    SDK::Json j;
    j["open"] = t.open;

    SDK::Json request;
    request["pending"] = t.request.pending;
    request["requesterName"] = t.request.requesterName;
    request["receivedAtMs"] = t.request.receivedAtMs;
    j["request"] = request;

    if (!t.open)
        return j;

    j["traderGuid"] = makeGuidArrayJson(t.traderGuid);
    j["traderName"] = t.traderName;
    j["player"] = PlayerInfo::toJson(t.traderGuid);
    j["playerAccepted"] = t.playerAccepted;
    j["targetAccepted"] = t.targetAccepted;
    j["playerMoney"] = t.playerMoney;
    j["targetMoney"] = t.targetMoney;

    SDK::Json player = SDK::Json::array();
    for (const auto& item : t.playerItems)
        player.push_back(makeTradeItemJson(item));
    j["playerItems"] = player;

    SDK::Json target = SDK::Json::array();
    for (const auto& item : t.targetItems)
        target.push_back(makeTradeItemJson(item));
    j["targetItems"] = target;
    return j;
}

inline SDK::Json makeDuelJson(const CoreAPI::DuelInfo& d)
{
    SDK::Json j;
    j["pending"] = d.pending;
    j["active"] = d.active;
    j["challengerName"] = d.challengerName;
    j["challengerGuid"] = makeGuidArrayJson(d.challengerGuid);
    j["arbiterGuid"] = makeGuidArrayJson(d.arbiterGuid);
    j["requestedAtMs"] = d.requestedAtMs;
    j["player"] = d.challengerGuid.isValid()
        ? PlayerInfo::toJson(d.challengerGuid)
        : PlayerInfo::toJsonByName(d.challengerName);
    return j;
}

inline SDK::Json makeBattlefieldQueueJson(const CoreAPI::BattlefieldQueue& q)
{
    SDK::Json j;
    j["index"] = q.index;
    j["status"] = q.status;
    j["statusId"] = q.statusId;
    j["typeId"] = q.typeId;
    j["typeName"] = q.typeName;
    j["random"] = q.random;
    j["arena"] = q.arena;
    j["rated"] = q.rated;
    j["teamSize"] = q.teamSize;
    j["mapId"] = q.mapId;
    j["mapName"] = q.mapName;
    j["instanceId"] = q.instanceId;
    j["minLevel"] = q.minLevel;
    j["maxLevel"] = q.maxLevel;
    j["queueGuid"] = makeGuidArrayJson(q.queueGuid);
    j["instanceGuid"] = makeGuidArrayJson(q.instanceGuid);
    j["estimatedWaitMs"] = q.estimatedWaitMs;
    j["timeWaitedMs"] = q.timeWaitedMs;
    j["portExpiresInMs"] = q.portExpiresInMs;
    j["canLeave"] = q.canLeave;
    return j;
}

}
