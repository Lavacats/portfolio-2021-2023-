#include "GameAfx.h"
#include "DailyEvent.h"
#include "Contents/Rank/Util-Rank.h"



//-----------------------------------------------------
// class CDailyEvent
//-----------------------------------------------------
CDailyEvent::CDailyEvent(BASE::EVENT_INFO *info, INT64 startTime, INT64 endTime)
	: IEvent(info)
{
	m_info = info;
	m_kind = m_info->Kind;
	m_period.SetStartTime(startTime);
	m_period.SetEndTime(endTime);

	LoadDailyEventInfo();
}

CDailyEvent::CDailyEvent(INT64 id, INT32 kind, INT32 subKind, INT64 startTime, INT64 endTime, INT64 scheduleID)
	:IEvent(id, kind, subKind, startTime, endTime, scheduleID)
{
	m_bInitialized = true;
}


CDailyEvent::CDailyEvent(BASE::EVENT_INFO *info)
	: IEvent(info)
{

}

CDailyEvent::~CDailyEvent()
{
}

void CDailyEvent::UpdateEventSchedule(INT64 startTime, INT64 endTime)
{
	GetPeriod()->SetStartTime(startTime);
	GetPeriod()->SetEndTime(endTime);
	LoadDailyEventInfo();
}

void CDailyEvent::InitDailyEventInfo()
{
	
}

void CDailyEvent::LoadDailyEventInfo()
{
	// �ѹ� �ʱ�ȭ
	InitDailyEventInfo();
}

//===================== virtual methods from here ======================
void CDailyEvent::OnEvent(INT64 userID, int conditionKind, int conditionValue, int value, int tireValue)
{
	if (false == Util::GOT::IsActive(EnumContentState::PROGRESS_EVENT_DAILY))
		return;
	
	INT16 weekDay = GLOBAL::GetDailyEventWeekDay(GetDueDay_UTC(0));

	FIELD_ISLE_INFO* pIsleInfo = Event::FieldManager::Instance()->GetIsle(PVP_MAP_UNIQUE, userID);
	if (IS_NULL(pIsleInfo))
	{
		SPDLOG_DEBUG("CDailyEvent.OnEvent() FIELD_ISLE_INFO is null  userid[{}]", userID);
		return;
	}

	if (pIsleInfo->UserSN == 0)
		return;

	const auto game_user = CUserManager::Instance()->Seek(userID);
	if (nullptr == game_user)
		return;
	
	BASE::DAILY_EVENT_MISSION_INFO_ARRAY *infoArray = BASE::GetDailyEventMissionInfos(BASE::GetDailyEventMissionType(GLOBAL::EVENT_MANAGER.GetDailyEventWeeks()), weekDay);
	if (IS_NULL(infoArray))
		return;

	// ������ �´� ���� �̺�Ʈ ������ �����ɴϴ�.
	auto levelMinMax = infoArray->GetEventMissionMinMaxLevel(pIsleInfo->Level);

	auto isFind = false;

	// ���� �̺�Ʈ ������ 2�� �̻��� �� �ֱ� ������, �ش� ���Ͽ� ���� ���� �̺�Ʈ�� ��ȸ �մϴ�.
	auto iterS = infoArray->asMissionInfo.begin();
	auto iterE = infoArray->asMissionInfo.end();
	for (; iterS != iterE; ++iterS)
	{
		BASE::DAILY_EVENT_MISSION_INFO * pSubInfo = &(*iterS);
		if (IS_NULL(pSubInfo))
			continue;

		// MinMax ������ ���ƾ� ���� ���� �̺�Ʈ�� ���ϴ�.
		if (levelMinMax.first == pSubInfo->LevelMin && levelMinMax.second == pSubInfo->LevelMax)
		{
			
			for (int i = 0; i < GAME::DAILY_EVENT_MISSION_CONDITION_MAX; ++i)
			{
				if (conditionKind == pSubInfo->Condition[i] && (pSubInfo->SubCondition == 0 || conditionValue == pSubInfo->SubCondition))
				{
					isFind = true;
					break;
				}
			}
		}

		if (isFind)
			break;
	}

	if (!isFind)
		return;



	INT32 addRankPoint = 0;

	if (IsAccumlateEvent(conditionKind))
	{
		INT32 prevAccumlateValue = CalculateAccumlatePoint(userID, conditionKind);

		// ���� ���� ���� ���� ���� ���� ��ȭ���� ����մϴ�.
		INT32 BeforeTotalPoint = CalculateEventPoint(userID, conditionKind, conditionValue, prevAccumlateValue);
		INT32 AftertotalPoint = CalculateEventTotalPoint(userID, conditionKind, conditionValue, value);

		// �� ���� ���� ����մϴ�.
		INT64 totalvalue = prevAccumlateValue + value;

		INT32 addpoint = (AftertotalPoint - BeforeTotalPoint);
		addRankPoint += addpoint;

		if (addRankPoint < 0) 
			addRankPoint = 0;
	}
	else
	{
		INT32 addpoint = CalculateEventPoint(userID, conditionKind, conditionValue, value);
		addRankPoint += addpoint;
	
		if (addRankPoint > 0)
		{
			SendDailyPoint(userID, conditionKind, addRankPoint);
		}
	}
}

void CDailyEvent::GmCommandDaily(INT64 userID, int dailyPoint)
{
	SendDailyPoint(userID, 0, dailyPoint);
}

void CDailyEvent::SendDailyPoint(INT64 userID, INT32 conditionKind, INT32 addPoint)
{
	auto game_user = CUserManager::Instance()->Seek(userID);
	if (nullptr == game_user)
		return;

	// ���������� ����Ʈ�� ������ ��찡 �־ ����͸� �� �α� �߰�
	if (addPoint >= 1000000)
	{
		const STRING logMsg = UTIL::StringFormat(_T("OnEvent - Invaild daily event point! userid: [%I64d], conditionKind:[%d], addPoint:[%d]"), userID, conditionKind, addPoint);
		if (NxC_Agent::Get())
			NxC_Agent::Get()->SendEventLog(EVENT_LOG_QUERY_ERROR, logMsg.c_str());
		return;
	}

	QueryPacker packer;

	INT64 UTCTime = GetDueDay_UTC(0);

	INT32 curWeeklyID = CRankMgr::Instance()->GetCurWeeklyNumber();
	INT16 curEventType = CRankMgr::Instance()->GetCurEventType();

	INT16 checkEventType = GLOBAL::GetDailyEventWeekDay(GetDueDay_UTC(0));
	INT32 checkWeeklyNumber = GLOBAL::GetDailyEventWeeklyID(GetDueDay_UTC(0));

	INT32 curDailyEventPoint = 0;
	INT32 curWeeklyDailyEventPoint = 0;

	std::pair<INT32, INT64> prevRank = std::make_pair(0, 0);
	std::pair<INT32, INT64> curRank = std::make_pair(0, 0);

	// �ְ� �̺�Ʈ ����Ʈ
	curWeeklyDailyEventPoint = CRankMgr::Instance()->GetPoint(GAME::DAILY_EVENT_RANK, GAME::eDAILY_RANK_KIND::EVENT_RANK_WEEKLY, userID);
	if (curWeeklyDailyEventPoint < 0)
		curWeeklyDailyEventPoint = 0;
	curWeeklyDailyEventPoint += addPoint;

	// �ϰ� �̺�Ʈ ����Ʈ
	curDailyEventPoint = CRankMgr::Instance()->GetPoint(GAME::DAILY_EVENT_RANK, checkEventType, userID);
	if (curDailyEventPoint < 0)
		curDailyEventPoint = 0;
	curDailyEventPoint += addPoint;

	CRankMgr::Instance()->GetMyRank(GAME::eRANK_TYPE::DAILY_EVENT_RANK, checkEventType, game_user.get(), prevRank);

	CRankMgr::Instance()->SetPoint(GAME::DAILY_EVENT_RANK, checkEventType, userID, curDailyEventPoint);

	CRankMgr::Instance()->GetMyRank(GAME::eRANK_TYPE::DAILY_EVENT_RANK, checkEventType, game_user.get(), curRank);

	Util::Rank::SendToDB_RankDaily_Update(game_user, curWeeklyID, curEventType, curDailyEventPoint, packer);

	if ((curRank.first >= 0) &&
		(prevRank.first != curRank.first))
	{
		// ���� �̺�Ʈ ��ŷ �α�
		GLOBAL::SendLog(game_user->UID(), 0, DB_LOG::REASON_DAILY_EVENT_RANKING_DAY, 0, 0,
			{ GLOBAL::GS_INFO.SVID, game_user->UID(), game_user->GetGuildID(), checkEventType, checkEventType, curRank.first, addPoint, curRank.second, 0, 0 },
			{ game_user->GetLordName(), CGuildManager::Instance()->GetGuildNameFromPersonID(game_user->UID()) });
	}

	// ����Ʈ ȹ�� ���ӷα�
	GLOBAL::SendLog(userID, 0, DB_LOG::REASON_DAILY_EVENT_POINT, 0, 0, { conditionKind, checkEventType, addPoint, curRank.second }, {});

	INT64 eventStartTime = GLOBAL::GetDailyEventStartTime();

	CRankMgr::Instance()->GetMyRank(GAME::eRANK_TYPE::DAILY_EVENT_RANK, GAME::eDAILY_RANK_KIND::EVENT_RANK_WEEKLY, game_user.get(), prevRank);

	CRankMgr::Instance()->SetPoint(GAME::DAILY_EVENT_RANK, GAME::eDAILY_RANK_KIND::EVENT_RANK_WEEKLY, userID, curWeeklyDailyEventPoint);

	CRankMgr::Instance()->GetMyRank(GAME::eRANK_TYPE::DAILY_EVENT_RANK, GAME::eDAILY_RANK_KIND::EVENT_RANK_WEEKLY, game_user.get(), curRank);

	Util::Rank::SendToDB_RankWeekly_Update(game_user, curWeeklyID, curWeeklyDailyEventPoint, packer);

	if ((curRank.first >= 0) &&
		(prevRank.first != curRank.first))
	{
		// �ְ� �̺�Ʈ ��ŷ �α�
		GLOBAL::SendLog(game_user->UID(), 0, DB_LOG::REASON_DAILY_EVENT_RANKING_WEEK, 0, 0,
			{ GLOBAL::GS_INFO.SVID, game_user->UID(), game_user->GetGuildID(), checkEventType, checkEventType, curRank.first, addPoint, curRank.second, 0, 0 },
			{ game_user->GetLordName(), CGuildManager::Instance()->GetGuildNameFromPersonID(game_user->UID()) });
	}

	// GOT �̺�Ʈ �α�
	GLOBAL::SendLog(game_user->UID(), 0, DB_LOG::REASON_EVENT, 0, 0, { GLOBAL::GS_INFO.SVID, 1, eventStartTime, conditionKind, 1, prevRank.second, addPoint, curRank.second,
	0, 0, 0, 0, 0, 0, 0, 0 }, { game_user->GetLordName(), CGuildManager::Instance()->GetGuildNameFromPersonID(game_user->UID()), GLOBAL::ConvertTimeToString(eventStartTime).c_str() });

	//UTC�� üũ�� ������ ���浵�Ӿ����� ��������.
	if (game_user->CheckDailyPointRefreshData() == false)
	{
		game_user->RefreshDailyEventDatas();
	}

	//��ŷ ����Ʈ ������ ���ȵŷ� ����.
	game_user->SetDailyEventPointUTCTime(UTCTime);
	game_user->SetDailyEventPoint(curDailyEventPoint);
	game_user->SetWeeklyDailyPoint(curWeeklyDailyEventPoint);




	//��Ŷ ������
	if (IS_ACTIVE_USER(game_user))
	{
		NEW_FLATBUFFER(GS_RANKING_DAILY_SET_ACK, pGSPacket);
		pGSPacket.Build([&](flatbuffers::FlatBufferBuilder& fbb)->auto
		{
			return PROTOCOL::FLATBUFFERS::CreateGS_RANKING_DAILY_SET_ACK(fbb, game_user->GetDailyEventPoint(), game_user->GetWeeklyDailyPoint());
		});
		SEND_ACTIVE_USER(game_user, pGSPacket);
	}

	packer.Request();
}

void CDailyEvent::Update()
{
}

void CDailyEvent::Complete()
{
}


INT32 CDailyEvent::CalculateEventPoint(INT64 userID, INT32 conditionKind, INT32 subCondition, INT32 value)
{
	int point2 = 0;
	int addPoint = 0;

	//case GAME::BLESSING_NORAML_TANK_SET:	// common_daily_event_point_info
	//case GAME::BLESSING_UNIQUE_TANK_SET:	// common_daily_event_point_info

	
	BASE::DAILY_EVENT_POINT * daliyEventPointInfo = BASE::GET_DAILY_EVENT_POINT_CONDITIONKIND_INFO(conditionKind);
	if (IS_NOT_NULL(daliyEventPointInfo))
	{
		INT32 calcTargetValue = 0;
		INT32 calcGetPoint = 0;
		INT32 rewardPoint = 0;

		/*if (conditionKind == GAME::MANUFACTURE_BATTLEPOWER || conditionKind == GAME::COND_EQUIP_REINFORCE_BATTLEPOWER)
		{
			daliyEventPointInfo = BASE::GET_DAILY_EVENT_POINT_INFO(conditionKind, 0);
			if (IS_NULL(daliyEventPointInfo))
				return 0;

			return daliyEventPointInfo->GetPoint(value);
		}*/
		if (conditionKind == GAME::BLESSING_NORAML_TANK_SET || conditionKind == GAME::BLESSING_UNIQUE_TANK_SET)
		{
			daliyEventPointInfo = BASE::GET_DAILY_EVENT_POINT_INFO(conditionKind, subCondition);
			if (IS_NULL(daliyEventPointInfo))
				return 0;

			return std::max<INT32>(value * daliyEventPointInfo->GetPointMax(0) / 10000, 1);
		}
		if (daliyEventPointInfo->TARGET_VALUE == 0)
		{
			calcGetPoint = daliyEventPointInfo->GetPoint(subCondition);
			rewardPoint = calcGetPoint * value;
		}
		else
		{
			calcTargetValue = daliyEventPointInfo->TARGET_VALUE;
			calcGetPoint = daliyEventPointInfo->GetPoint(0);

			if (value <= 0 || calcTargetValue <= 0)
				return 0;

			rewardPoint = calcGetPoint * floor((double)value / (double)calcTargetValue);
		}


		addPoint = rewardPoint;
		return addPoint;
	}

	switch (conditionKind)
	{
	case GAME::COLLECT_OIL:						// CV - �ķ�
		addPoint = floor((value/100) * 5);
		break;
	case GAME::COLLECT_IRON:
		addPoint = value;
		break;
	case GAME::COLLECT_SILVER:					// CV - ��
		addPoint = floor((value/4) * 5);
		break;
	case GAME::KILL_MONSTER_UNIQUE_TANK:
	case GAME::KILL_MONSTER:
		addPoint = value;
		break;
	case GAME::KILL_TRADINGTANK:
		addPoint = value;
		break;
	case GAME::RECRUIT_UNIT:
		addPoint = value;
		break;
	case GAME::BUILD_UNIT:
		addPoint = value;
		break;
	case GAME::UPGRADE_TANK:
		addPoint = value;
		break;
	//case GAME::CREATE_EQUIP:
	//	addPoint = value;
	//	break;
	case GAME::KILL_EVENT_UNIT:
		addPoint = value;
		break;
	case GAME::KILL_EVENT_MONSTER:
		addPoint = value;
		break;
	case GAME::KILL_EVENT_TRADINGTANK:
		addPoint = value;
		break;
	case GAME::TANKBUILD_BATTLEPOWER:
		addPoint = value;
		break;
	case GAME::TANKENHANCE_BATTLEPOWER:
		addPoint = value;
		break;
	case GAME::BUILD_UPGRADE_USE_OIL:		// �Ǽ��� ����� �ķ� 1000�� 1����Ʈ
		addPoint = (value/1000);
		break;
	case GAME::BUILD_UPGRADE_USE_IRON:
		addPoint = value;
		break;
	case GAME::BUILD_UPGRADE_USE_SILVER:	// �Ǽ��� ����� �� 40�� 5����Ʈ
		addPoint = ((value/40)*5);
		break;
	case GAME::BUILD_UPGRADE_USE_ITEM:
	case GAME::RESEARCH_COMPLETE_USE_SILVER:	// ������ �Ҹ��� �� 8��5����Ʈ
		addPoint = ((value / 8) * 5);
		break;
	case GAME::RESEARCH_COMPLETE_USE_ITEM:
		addPoint = value;
		break;
	case GAME::BLESSING_NORAML_TANK_SET:
	case GAME::BLESSING_UNIQUE_TANK_SET:
		addPoint = value;
		break;
	case GAME::LEVELUP_SECRET_NORMAL_TANK_WORKOUT:
	case GAME::LEVLEUP_SECRET_UNIQUE_TANK_WORKOUT:
		addPoint = value;
		break;
	default:
		break;
	}
	return addPoint;
}

INT32 CDailyEvent::CalculateEventTotalPoint(INT64 userID, INT32 conditionKind, INT32 subCondition, INT32 value)
{
	//int point2 = 0;
	int addPoint = 0;

	INT64 TotalValue = CalculateAccumlatePoint(userID, conditionKind);
	if (TotalValue <= 0)
		TotalValue = value;
	else
		TotalValue += value;

	BASE::DAILY_EVENT_POINT * daliyEventPointInfo = BASE::GET_DAILY_EVENT_POINT_INFO(conditionKind);
	if (IS_NOT_NULL(daliyEventPointInfo))
	{
		INT32 calcTargetValue = 0;
		INT32 calcGetPoint = 0;
		INT32 rewardPoint = 0;

		if (daliyEventPointInfo->TARGET_VALUE <= 0)
			return 0;

		calcTargetValue = daliyEventPointInfo->TARGET_VALUE;
		calcGetPoint = daliyEventPointInfo->GetPoint(0);

		if (TotalValue <= 0 || calcTargetValue <= 0)
			return 0;

		rewardPoint = calcGetPoint * floor((double)TotalValue / (double)calcTargetValue);

		addPoint = rewardPoint;
		return addPoint;
	}

	return 0;
}

void CDailyEvent::Send_DB_DAILY_EVENT_POINT_UPDATE_REQ(INT64 userID, INT32 addPoint)
{
	
}

void CDailyEvent::ReloadEvent()
{
	LoadDailyEventInfo();
}

INT64 CDailyEvent::CalculateAccumlatePoint(INT64 userID, INT32 conditionKind)
{
	INT32 result = 0;

	auto pUser = CUserManager::Instance()->FindByUID(userID);
	if (IS_NULL(pUser))
	{
		SPDLOG_DEBUG("CDailyEvent.CalculateAccumlatePoint() User Info is Null userid[{}]", userID);
		return result;
	}

	if (pUser->UserSN() == 0)
	{
		return result;
	}

	const PROTOCOL::DAILY_EVENT_POINT_RESOURCE_INFO* info = pUser->GetDailyEventPointResourceInfo(conditionKind, CRankMgr::Instance()->GetCurEventType());
	if (IS_NULL(info))
		return result;

	result = info->TotalValue;
	return result;
}

bool CDailyEvent::IsAccumlateEvent(INT32 conditionkind)
{
	BASE::DAILY_EVENT_POINT * daliyEventPointInfo = BASE::GET_DAILY_EVENT_POINT_INFO(conditionkind);
	if (IS_NULL(daliyEventPointInfo))
		return false;

	if (daliyEventPointInfo->TARGET_VALUE > 0)
		return true;

	return false;
}

void CDailyEvent::Send_DB_DAILY_EVENT_POINT_VALUE_SET_REQ(INT64 userID, INT32 conditionKind, INT32 addPointvalue, INT64 prevTotalValue, INT64 TotalValue)
{
	auto pUser = CUserManager::Instance()->FindByUID(userID);
	if (IS_NULL(pUser))
	{
		SPDLOG_DEBUG("CDailyEvent.Send_DB_DAILY_EVENT_POINT_VALUE_SET_REQ() User Info is Null userid[{}]", userID);
		return;
	}

	if (pUser->UserSN() == 0)
	{
		return;
	}

	// �Ѱ� ����Ʈ�� 0 ���϶�� ��ȿ
	if (TotalValue <= 0)
	{
		return;
	}

	// ���� ������ ��ó���� ���ݴϴ�.
	//pUser->AddDailyEventPointResource(conditionKind, GLOBAL::GetDailyEventWeekDay(GetDueDay_UTC(0)), TotalValue);

	pUser->Send_DB_DAILY_EVENT_POINT_VALUE_INFO_SET_REQ(conditionKind, addPointvalue, prevTotalValue, TotalValue);
}