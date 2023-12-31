#include "GameAfx.h"
#include "Util-UseEvent.h"
#include "Season/SeasonManager.h"
#include "_DatabaseThreadManager/DBQUERY/GDB/P_T_USE_EVENT_INFO_FETCH_SERVER.h"
#include "_DatabaseThreadManager/DBQUERY/GDB/P_T_USE_EVENT_INFO_FETCH_USER.h"
#include "_DatabaseThreadManager/DBQUERY/GDB/P_T_USE_EVENT_INFO_INIT_DELETE.h"
#include "_DatabaseThreadManager/DBQUERY/GDB/P_T_USE_EVENT_INFO_UPDATE.h"

#include "_DatabaseThreadManager/DBQUERY/GDB/P_T_USE_EVENT_REWARD_FETCH_USER.h"
#include "_DatabaseThreadManager/DBQUERY/GDB/P_T_USE_EVENT_REWARD_INIT_DELETE.h"
#include "_DatabaseThreadManager/DBQUERY/GDB/P_T_USE_EVENT_REWARD_UPDATE.h"
#include "_DatabaseThreadManager/DBQUERY/GDB/P_T_USE_EVENT_REWARD_FETCH_SERVER.h"

namespace Util::UseEvent
{
	template<class _Ty>
	void SetData_UseEventUserEventInfo(const UserSharedPtr& game_user, const _Ty& row)
	{
 		ASE_INSTANCE(game_user, CUserUseEventManager)->Insert_Info(row.m_nUseEventKind, row.m_nPoint);
	}
	template<class _Ty>
	void SetData_UseEventUserEventReward(const UserSharedPtr& game_user, const _Ty& row)
	{
		ASE_INSTANCE(game_user, CUserUseEventManager)->Insert_Reward(row.m_nUseEventKind,row.m_nStep);
	}

	void SetData_UseEventUserEventInfo(const TLDB::CQuery::SharedPtr& query)
	{
		if (const auto query_server = std::dynamic_pointer_cast<QP_T_USE_EVENT_INFO_FETCH_SERVER>(query))
		{
			for (const auto& row : query_server->GetSET_1())
			{
				if (const auto game_user = CUserManager::Instance()->Seek(row.m_nUserID))
				{
					SetData_UseEventUserEventInfo(game_user, row);
				}
			}
		}
		else if (const auto query_user = std::dynamic_pointer_cast<QP_T_USE_EVENT_INFO_FETCH_USER>(query))
		{
			if (const auto game_user = CUserManager::Instance()->Seek(query_user->m_nUserID))
			{
				for (const auto& row : query_user->GetSET_1())
				{
					SetData_UseEventUserEventInfo(game_user, row);
				}
			}
		}
		else
		{
			// 다른 타입이 들어온경우?
			assert(false);
		}
	}

	void SetData_UseEventUserEventReward(const TLDB::CQuery::SharedPtr& query)
	{
		if (const auto query_server = std::dynamic_pointer_cast<QP_T_USE_EVENT_REWARD_FETCH_SERVER>(query))
		{
			for (const auto& row : query_server->GetSET_1())
			{
				if (const auto game_user = CUserManager::Instance()->Seek(row.m_nUserID))
				{
					SetData_UseEventUserEventReward(game_user, row);
				}
			}
		}
		else if (const auto query_user = std::dynamic_pointer_cast<QP_T_USE_EVENT_REWARD_FETCH_USER>(query))
		{
			if (const auto game_user = CUserManager::Instance()->Seek(query_user->m_nUserID))
			{
				for (const auto& row : query_user->GetSET_1())
				{
					SetData_UseEventUserEventReward(game_user, row);
				}
			}
		}
		else
		{
			// 다른 타입이 들어온경우?
			assert(false);
		}
	}

	void SendToDB_UseEventInfo_Fetch_User(const UserSharedPtr& game_user, const QueryPackLinker::SharedPtr& load_balancer /*= nullptr*/)
	{
		const auto query = std::make_shared<QP_T_USE_EVENT_INFO_FETCH_USER>();
		query->m_nServerID = game_user->GetServerID();
		query->m_nUserID = game_user->UID();

		QueryPacker packer(load_balancer);
		packer.Add_Select(query);
		packer.Request();
	}

	void SendToDB_UseEventInfo_Update(const UserSharedPtr& game_user, const INT64 eventKind, const INT64 eventPoint, const QueryPackLinker::SharedPtr& load_balancer /*= nullptr*/)
	{
		const auto query = std::make_shared<QP_T_USE_EVENT_INFO_UPDATE>();
		query->m_nServerID = game_user->GetServerID();
		query->m_nUserID = game_user->UID();
		query->m_nUseEventKind = eventKind;
		query->m_nPoint = eventPoint;

		QueryPacker packer(load_balancer);
		packer.Add_User(query->m_nUserID, query);
		packer.Request();
	}

	void SendToDB_UseEventInfo_InitDelete(const UserSharedPtr& game_user, const INT64 eventKind, const QueryPackLinker::SharedPtr& load_balancer /*= nullptr*/)
	{
		///추가예정
	}

	void SendToDB_UseEventReward_Fetch_User(const UserSharedPtr& game_user, const QueryPackLinker::SharedPtr& load_balancer /*= nullptr*/)
	{
		const auto query = std::make_shared<QP_T_USE_EVENT_REWARD_FETCH_USER>();
		query->m_nServerID = game_user->GetServerID();
		query->m_nUserID = game_user->UID();

		QueryPacker packer(load_balancer);
		packer.Add_Select(query);
		packer.Request();
	}

	void SendToDB_UseEventReward_Update(const UserSharedPtr& game_user, const INT64 eventKind, const INT64 evetnStep, const QueryPackLinker::SharedPtr& load_balancer /*= nullptr*/)
	{
		const auto query = std::make_shared<QP_T_USE_EVENT_REWARD_UPDATE>();
		query->m_nServerID = game_user->GetServerID();
		query->m_nUserID = game_user->UID();
		query->m_nUseEventKind = eventKind;
		query->m_nStep = evetnStep;

		QueryPacker packer(load_balancer);
		packer.Add_User(query->m_nUserID, query);
		packer.Request();
	}

	void SendToDB_UseEventReward_InitDelete(const UserSharedPtr& game_user, const INT64 eventKind, const QueryPackLinker::SharedPtr& load_balancer /*= nullptr*/)
	{

	}

	void SendUserUseEventInfoClientLoginNFY(const UserSharedPtr& game_User)
	{
		if (IS_NULL(game_User))
			return;

		ASE_INSTANCE(game_User, CUserUseEventManager)->SetUpUseEventUser();

		NEW_FLATBUFFER(GS_USE_EVENT_LOGIN_NFY, packet);
		packet.Build([&](flatbuffers::FlatBufferBuilder& fbb)->auto
		{
			std::vector<flatbuffers::Offset<PROTOCOL::FLATBUFFERS::USE_EVENT_INFO>> useEventList;

			auto uerUseEventManager		= game_User->getCUserUseEventManager();
			auto useUseEventInfo		= uerUseEventManager->GetUseEventInfoRepository();
			auto useUseEventReward		= uerUseEventManager->GetUseEventRewardRepository();
			INT64 userLV				= game_User->GetCastleLevel();
			//INT64 curTimeUTC = GetDueDay_UTC(0);

			for (auto iter = useUseEventInfo.begin(); iter != useUseEventInfo.end(); ++iter)
			{
				int groupKind	= iter->second->useEvent_Info->GetGroupKind();
				int groupPoint	= iter->second->useEvent_Info->GetEventPoint();
				int m_MinLV		= iter->second->useEvent_Info->GetMinLv();
				int m_MaxLV		= iter->second->useEvent_Info->GetMaxLV();

				const auto useEventInfo = BASE::GET_USE_EVENT_INFO_DATA(groupKind);

				if(useEventInfo==nullptr)continue;

				int m_season		 = useEventInfo->eventSeason;

				// 시즌 검사
				if (m_season != CSeasonManager::Instance()->GetSeasonNo())
				{
					if (m_season != 0)
						continue;
				}

				// 레벨검사
				if(userLV < m_MinLV || userLV > m_MaxLV)
					continue;

				if (useEventInfo->eventCondition == GAME::eEVENTCONDITION_TYPE::USE_ITEM_GEM && useEventInfo->eventConditionValue == 1)
				{
					if (Util::GOT::IsBlockedContentsByUserCountry(game_User->UID(), EnumContentBlock::BLOCK_EVENT_USE))
						continue;
				}
				
				int m_chronicle			= useEventInfo->eventChronicle;
				long m_chronicleTime	= (0 == m_chronicle ? 0 : GLOBAL::CHRONICLE_MANAGER.GetChronicleOpenTime((EChronicleType)m_chronicle));
				long m_startTime		= m_chronicleTime + useEventInfo->useEventStartTime;
				long m_endTime			= m_chronicleTime + useEventInfo->useEventEndTime;

				std::vector<int> rewardPointGradeList;

				for (auto rewardIter = useUseEventReward.begin(); rewardIter != useUseEventReward.end(); ++rewardIter)
				{
					if (rewardIter->second->useEvent_Reward->GetGroupKind() == groupKind)
					{
						rewardPointGradeList.emplace_back(rewardIter->second->useEvent_Reward->GetEventRewardStep());
					}
				}
				useEventList.emplace_back(PROTOCOL::FLATBUFFERS::CreateUSE_EVENT_INFO(
					fbb,
					groupKind,
					groupPoint,
					fbb.CreateVector(rewardPointGradeList),
					m_startTime,
					m_endTime
					)
				);
			}
			return PROTOCOL::FLATBUFFERS::CreateGS_USE_EVENT_LOGIN_NFY(fbb,
				fbb.CreateVector(useEventList));
		});
		SEND_ACTIVE_USER(game_User, packet);
	}

}
