using UnityEngine;
using BaseTable;
using NLibCs;
using Contents.User;
using FlatBuffers;
using PROTOCOL.FLATBUFFERS;
using PROTOCOL.GAME.ID;
using GameEvent;

public class SeasonEventMission : UIForm
{
    public const string OBJECTID = "SheetBlock_EventList";

    [SerializeField] private GameObject seasonMissionInfo;              // ���� �̼� �󼼺��� ������Ʈ
    [SerializeField] private GameObject seasonMissionClearMark;           // �̼� ��Ŭ���� ����
    [SerializeField] private UILabel seasonMissionName;                 // ���� �̺�Ʈ �̸�
    [SerializeField] private UISprite seasonMissionSprite;              // ���� �̺�Ʈ  �̹���
    [SerializeField] private UIButton buttonMission;                    // �̼� Ŭ�� ��ư ( ���� )
    [SerializeField] private GameObject seasonDataObject;               // �޸���

    [SerializeField] private UILabel labelReward;                   // ���� ��ư ��
    [SerializeField] private UIButton buttonReward;                 // ���� ��ư
    [SerializeField] private GameObject receivedRewardEffect;               // �޸���

    [SerializeField] private GameObject rewardNotice;                      // ���� �����

    public SeasonEventMissionInfo seasonData;           //UI
    public NrChloeEventDataInfo seasonDataInfo;         //DATA
    public EventSeasonDlg eventSeasonDlg;

    public bool completeMission = false;

    public override void Init()
    {
        base.Init();
    }
    public override void BindUIEvents()
    {
        base.BindUIEvents();
        if (buttonMission) EventDelegate.Add(buttonMission.onClick, OnClick_EventIcon);

        if (buttonReward) EventDelegate.Add(buttonReward.onClick, OnClick_RewardButtion);
    }
    public void RefreshSeasonEvent()
    {
        SetSeasonEventInfo(false,seasonDataInfo);
    }
    public void SetActiveMissionClearMark(bool setClear)
    {
        seasonMissionClearMark.SetActive(setClear);
    }
    public void SetMissionLabel(string MissionTextKey)
    {
        seasonMissionName.text = NTextManager.Instance.GetText(MissionTextKey);
    }
    public void SetSpriteKey(string spriteKey)
    {
        if (null == seasonMissionSprite)
            return;

        seasonMissionSprite.spriteName = spriteKey;
        seasonMissionSprite.grayscale = true;
    }

    public void SetMissionPaperOff()
    {
        seasonDataObject.SetActive(false);
    }

    private void OnClick_EventIcon()
    {
        // ī�� Ŭ���ؼ� ���� ����
        seasonDataObject.SetActive(true);
        if (null != seasonDataInfo)
        {
            eventSeasonDlg.AllSeasonEventMissionPaperOff();
            SetSeasonEventInfo(true,seasonDataInfo);
        }
    }
    void SetSeasonEventInfo(bool activeInfo,NrChloeEventDataInfo missionINfo)
    {
        // ����Ȱ��ȭ
        seasonDataObject.SetActive(activeInfo);

        labelReward.text = NTextManager.Instance.GetText("UI_SEASONEVENT_REWARD_TEXT");

        // ��ǥ�� �޼� �̼� üũ
        int missionCount = 0;

        if (missionINfo != null)
        {
            if (missionINfo.SeasonEventMission_1 != null)
            {
                if (missionINfo.SeasonEventMission_1.MissionTitle != null) 
                { 
                    // �̼����� dicSeasionMission���� ���� Kind�� �´� ������ ���� ������
                    var curValue = ChloeStoryEventManager.Instance.dicSeasonMission[missionINfo.SeasonEventMission_1_Kind];
                    var targetValue = missionINfo.SeasonEventMission_1.TargetValue;

                    if (curValue >= targetValue)
                    {
                        missionCount++;
                        seasonData.SetActiveMissionClearMark(1, true);
                        curValue = targetValue;
                    }

                    // �̼� Ÿ��Ʋ ����
                    seasonData.SetMissionLabel(
                        1,                                                  // �ε���
                        missionINfo.SeasonEventMission_1.MissionTitle,      // Ÿ��Ʋ
                        curValue,                                           // ���� �̼� ���ప
                        targetValue                                         // �̼� ���� ��ǥ��
                        );
                }
            }
            if (missionINfo.SeasonEventMission_2 != null)
            {
                if (missionINfo.SeasonEventMission_2.MissionTitle != null)
                {
                    var curValue = ChloeStoryEventManager.Instance.dicSeasonMission[missionINfo.SeasonEventMission_2_Kind];
                    var targetValue = missionINfo.SeasonEventMission_2.TargetValue;

                    if (curValue >= missionINfo.SeasonEventMission_2.TargetValue)
                    {
                        missionCount++;
                        seasonData.SetActiveMissionClearMark(2, true);
                        curValue = targetValue;
                    }

                    seasonData.SetMissionLabel(
                        2,                                                  // �ε���
                        missionINfo.SeasonEventMission_2.MissionTitle,      // Ÿ��Ʋ
                        curValue,                                           // ���� �̼� ���ప
                        targetValue                                         // �̼� ���� ��ǥ��
                        );
                }
            }
            if (missionINfo.SeasonEventMission_3 != null)
            {
                if (missionINfo.SeasonEventMission_3.MissionTitle != null)
                {
                    var curValue = ChloeStoryEventManager.Instance.dicSeasonMission[missionINfo.SeasonEventMission_3_Kind];
                    var targetValue = missionINfo.SeasonEventMission_3.TargetValue;

                    if (curValue >= missionINfo.SeasonEventMission_3.TargetValue)
                    {
                        missionCount++;
                        seasonData.SetActiveMissionClearMark(3, true);
                        curValue = targetValue;
                    }

                    seasonData.SetMissionLabel(
                        3,                                                 // �ε���
                        missionINfo.SeasonEventMission_3.MissionTitle,     // Ÿ��Ʋ
                        curValue,                                          // ���� �̼� ���ప
                        targetValue                                        // �̼� ���� ��ǥ��
                        );
                }
            }

            if (missionINfo.SeasonRewards.Count > 0)
            {
                // NDT���̺��� ������ �о�� ������ ���� �����Ѵ�
                // 22.09.02 ���� 1�� �����۸� ����ϱ�� �����Ƿ� 0 ���
                seasonData.SetRewardItem(missionINfo.SeasonRewards[0].Kind, missionINfo.SeasonRewards[0].Quantity);
            }

            if (missionCount >= 3)
            {
                seasonMissionSprite.grayscale = false;
                Refresh_RewardEffect(missionINfo.SeasonEventKind);
                if (ChloeStoryEventManager.Instance.dicRewardEvent[missionINfo.SeasonEventKind] == (int)ChloeStoryEventManager.SeasonEventRewardState.ReceivedRewad)
                {
                    SetActiveMissionClearMark(true);
                }
                
                if(ChloeStoryEventManager.Instance.dicRewardEvent[missionINfo.SeasonEventKind] == (int)ChloeStoryEventManager.SeasonEventRewardState.CanReceiveReward)
                {
                    rewardNotice.SetActive(true);
                }
                else
                {
                    rewardNotice.SetActive(false);
                }
            }
            else
            {
                rewardNotice.SetActive(false);
            }
        }
    }
    public void Refresh_RewardEffect(int SeasonKind)
    {
        if (ChloeStoryEventManager.Instance.dicRewardEvent.ContainsKey(SeasonKind))
        {
            // dicRewardEvent�� �����Ѵ� : �̹� ���� ó���� �� �̺�Ʈ 
            if (ChloeStoryEventManager.Instance.dicRewardEvent[SeasonKind] == (int)ChloeStoryEventManager.SeasonEventRewardState.ReceivedRewad)
            {
                seasonData.RewardEffect();
            }
        }
        else
        {
            // dicRewardEvent�� ���������ʴ´� : ���� ���� �� �ְԵ� �̺�Ʈ 
            ChloeStoryEventManager.Instance.dicRewardEvent.Add(SeasonKind, (int)ChloeStoryEventManager.SeasonEventRewardState.CanReceiveReward);
        }

    }

    public void OnClick_RewardButtion()
    {
        // ������ ���� ���� ���� ���� Ȯ��
        if (seasonDataInfo != null)
        {
            if (seasonDataInfo.SeasonEventKind != 0)
            {
                if (ChloeStoryEventManager.Instance.dicRewardEvent.ContainsKey(seasonDataInfo.SeasonEventKind)) 
                { 
                    // ���� ���� ó���� ���� ��Ŷ ������ �۽�
                    if (ChloeStoryEventManager.Instance.dicRewardEvent[seasonDataInfo.SeasonEventKind] == (int)ChloeStoryEventManager.SeasonEventRewardState.CanReceiveReward)
                    {
                        NFlatBufferBuilder.SendBytes<GS_SEASON_EVENT_REWARD_REQ>(ePACKET_ID.GS_SEASON_EVENT_REWARD_REQ, () => GS_SEASON_EVENT_REWARD_REQ.CreateGS_SEASON_EVENT_REWARD_REQ(FlatBuffers.NFlatBufferBuilder.FBB,
                         UserBase.User.UID,
                        (int)ChloeStoryEventManager.SeasonEventRewardState.ReceivedRewad,
                        seasonDataInfo.SeasonEventKind
                        ));

                        return;
                    }
                }
                var iteminfo = BaseTable.TableItemInfo.Instance.Get(seasonDataInfo.SeasonRewards[0].Kind);
                if (null == iteminfo)
                    return;
                var titleText = NLibCs.NTextManager.Instance.GetText(iteminfo.itemName);
                var descText = NLibCs.NTextManager.Instance.GetText(iteminfo.itemInstruction);

                ToolTipDlg.SetToolTipDlg(titleText, descText);
            }
        }
    }
}
