using System;
using UnityEngine;
using Contents.User;
using DEFINE;
using PROTOCOL.FLATBUFFERS;
using PROTOCOL.GAME.ID;
using FlatBuffers;
using BaseTable;
using NLibCs;

public class EventSeasonDlg : UIForm
{
    [SerializeField] private GameObject[] seasonEventMission;           // ���� �̺�Ʈ �̼�1 

    [SerializeField] private GameObject seasonEventMission_ALL;         // ���� �̺�Ʈ ����

    [SerializeField] private UILabel Mission_All_Progress;              // ���� �̺�Ʈ �����
    [SerializeField] private UILabel seasonEventTime;                   // ���� �̺�Ʈ ����ð�
    [SerializeField] private UISprite Mission_All_Sprite;                 // ���� �̺�Ʈ �����
    [SerializeField] private GameObject Mission_All_Sprite_Reward;      // ���� �̺�Ʈ ����ð�

    [SerializeField] private SeasonEventMission[] _ChloeStoryMission;
    [SerializeField] private UIButton buttonBackGround;

    [SerializeField] private GameObject missionReward;               // ����
    [SerializeField] private UISprite rewardImage;               // ���� �̹���
    [SerializeField] private UILabel rewardLabelVolume;               // ���� ����
    [SerializeField] private UILabel rewardLabelValue;               // ���� ��
    [SerializeField] private GameObject rewardEffect;               // ���� ����Ʈ

    [SerializeField] private UILabel completeRewardProgress;               // ���� ��
    [SerializeField] private UIButton buttonReward;                 //��������
    [SerializeField] private GameObject effectRwardReceive;                 //��������


    [SerializeField] private UIButton qustionMarkButton;        //����
    public NrChloeEventDataInfo seasonDataInfo;         //DATA
    public override void Init()
    {
        base.Init();
        var missionData = TableChloeEventInfo.Instance.m_chloeEventDataInfoList;    // �̺�Ʈ �����͸� �ҷ��� ( ��X , ���� X )
        Mission_All_Sprite.grayscale = true;
        if (missionData != null)
        {
            for (int i = 0; i < _ChloeStoryMission.Length; i++)
            {
                _ChloeStoryMission[i].BindUIEvents();
                _ChloeStoryMission[i].eventSeasonDlg = this; 
                _ChloeStoryMission[i].RefreshSeasonEvent();
            }
        }
        
        RefreshCompleteReward();
    }

    public void SetSeasonEventUI(int curSeason, int curPeriod)
    {
        var dicMissionData = TableChloeEventInfo.Instance.m_chloeEventDataInfoList;    // �̺�Ʈ �����͸� �ҷ��� ( ��X , ���� X )

        foreach(var missionData in dicMissionData)
        {
            if (missionData.Value.SeasonKind == curSeason && missionData.Value.Period == curPeriod)
            {
                if (missionData.Value.missionCardNum != 0)
                {
                    _ChloeStoryMission[missionData.Value.missionCardNum - 1].seasonDataInfo = missionData.Value;
                    _ChloeStoryMission[missionData.Value.missionCardNum - 1].SetSpriteKey(missionData.Value.MissionSprite);
                    _ChloeStoryMission[missionData.Value.missionCardNum - 1].SetMissionLabel(missionData.Value.SeasonTitle);
                }
                else
                {
                    seasonDataInfo = missionData.Value;

                    Mission_All_Sprite.spriteName = seasonDataInfo.MissionSprite;
                    
                    // ���� ���� �̼� ����
                    SetRewardItem(missionData.Value.SeasonRewards[0].Kind, missionData.Value.SeasonRewards[0].Quantity);
                }
            }
        }
    }
    public void RefreshSeasonEvent()
    {
        RefreshCompleteReward();
   
        var missionData = TableChloeEventInfo.Instance.m_chloeEventDataInfoList;    // �̺�Ʈ �����͸� �ҷ��� ( ��X , ���� X )


        int curCompleteMissioncount = 0;
        if (missionData != null)
        {
            for (int i = 0; i < _ChloeStoryMission.Length; i++)
            {
                _ChloeStoryMission[i].RefreshSeasonEvent();
                // dicReward�� ��ϵ� ���� 0���� ū ���� count�Ѵ� 
                if (_ChloeStoryMission[i].seasonDataInfo != null)
                {
                    if (ChloeStoryEventManager.Instance.dicRewardEvent.ContainsKey(_ChloeStoryMission[i].seasonDataInfo.SeasonEventKind))
                    {
                        if (ChloeStoryEventManager.Instance.dicRewardEvent[_ChloeStoryMission[i].seasonDataInfo.SeasonEventKind] > 0)
                        {
                            curCompleteMissioncount++;
                        }
                    }
                }
            }
        }
        SetText(ref completeRewardProgress, string.Format(NTextManager.Instance.GetText("UI_SEASONEVENT_PROGRESS"), curCompleteMissioncount, 4));
        if (curCompleteMissioncount >= 4)
        {
            Mission_All_Sprite.grayscale = false;
        }
    }

    public  void AllSeasonEventMissionPaperOff()
    {
        for (int i = 0; i < _ChloeStoryMission.Length; i++)
        {
            _ChloeStoryMission[i].SetMissionPaperOff();

        }
        if (UIFormManager.Instance.IsOpenUIForm<EventSeasonClearDlg>())
        {
            UIFormManager.Instance.CloseUIForm<EventSeasonClearDlg>();
        }
        RefreshCompleteReward();

    }

    public override void BindUIEvents()
    {
        base.BindUIEvents();
        if (qustionMarkButton) EventDelegate.Add(qustionMarkButton.onClick, OnClick_Question);
        if (buttonBackGround) EventDelegate.Add(buttonBackGround.onClick, AllSeasonEventMissionPaperOff);
        if (buttonReward) EventDelegate.Add(buttonReward.onClick, OnClick_RewardButtion);
    }

    public void RefreshCompleteReward()
    {
        int curCompleteMissioncount = 0;
        for (int i = 0; i < _ChloeStoryMission.Length; i++)
        {
            _ChloeStoryMission[i].RefreshSeasonEvent();
            if (_ChloeStoryMission[i].seasonDataInfo != null)
            {
                if (ChloeStoryEventManager.Instance.dicRewardEvent.ContainsKey(_ChloeStoryMission[i].seasonDataInfo.SeasonEventKind))
                {
                    // dicReward�� ��ϵ� ���� 0���� ū ���� count�Ѵ� 
                    if (ChloeStoryEventManager.Instance.dicRewardEvent[_ChloeStoryMission[i].seasonDataInfo.SeasonEventKind] > 0)
                        curCompleteMissioncount++;
                }
            }
        }

        SetText(ref completeRewardProgress, string.Format(NTextManager.Instance.GetText("UI_SEASONEVENT_PROGRESS"), curCompleteMissioncount, 4));

        if (curCompleteMissioncount >= 4)
        {
            if (seasonDataInfo.SeasonEventKind != 0)
            {
                if (ChloeStoryEventManager.Instance.dicRewardEvent.ContainsKey(seasonDataInfo.SeasonEventKind))
                {
                    if (ChloeStoryEventManager.Instance.dicRewardEvent[seasonDataInfo.SeasonEventKind] == (int)ChloeStoryEventManager.SeasonEventRewardState.ReceivedRewad ||
                        ChloeStoryEventManager.Instance.dicRewardEvent[seasonDataInfo.SeasonEventKind] == (int)ChloeStoryEventManager.SeasonEventRewardState.ShowAllCompletedMissionReward)
                    {
                        effectRwardReceive.SetActive(true);
                        Mission_All_Sprite.grayscale = false;
                    }
                }
            }

        }
    }
    public void SetRewardEffect(int seasonKind)
    {
        foreach(var eventData in TableChloeEventInfo.Instance.m_chloeEventDataInfoList)
        {
            if (eventData.Value.SeasonEventKind == seasonKind)
            {
                //Table�� 0���� 4���� ī��Ʈ��
                // 0 = ����
                // 1,2,3,4 =ī��
                if (eventData.Value.missionCardNum == 0)
                {
                    RefreshCompleteReward();
                }
                else
                {
                    _ChloeStoryMission[eventData.Value.missionCardNum-1].Refresh_RewardEffect(seasonKind);
                }
            }
        }
    }

    public void SetRewardItem(int itemKind , int itemCount)
    {
        // ���� ����������� 1���� ����� �� �ֵ��� ����

        var missionData = TableChloeEventInfo.Instance.m_chloeEventDataInfoList;

        var itemInfo = TableItemInfo.Instance.Get(itemKind);
        if (itemInfo == null) return;


        SetSprite(ref rewardImage, itemInfo.atlasName, itemInfo.itemIcon);
        SetText(ref rewardLabelValue, itemCount.ToString());
        SetText(ref rewardLabelVolume, PublicUIMethod.GetString_ItemVolumeText(itemInfo));
    }
    
    public override void Open(IUIParamBase param)
    {
        IsExceptionBlur = true;

        TotalEventListDlg.FirstOpen = false;
        TotalEventListDlg.CloseAllSubEventDlg(this);

        SetPanelDepth_ByLastDepth();
        base.Open(param);
    }

    public override void Close()
    {
        base.Close();
    }

    public override void Close_End()
    {
        base.Close_End();

        if (CloseReason == ReasonType.Escape)
            TotalEventListDlg.CloseReasonEscape();
    }

    public override void Update()
    {
        base.Update();

        var SeasonPassEndTime = SeasonPassManager.Instance.GetSeasonPassEndTime();

        var CurDate = PublicMethod.GetNowDate_Utc();
        var TimeOut = PublicMethod.GetDueDate_Utc(SeasonPassEndTime) - CurDate;

        var hoursText = (TimeOut.Hours / 10 < 1 ? $"0{TimeOut.Hours}" : $"{TimeOut.Hours}");
        var minutesText = (TimeOut.Minutes / 10 < 1 ? $"0{TimeOut.Minutes}" : $"{TimeOut.Minutes}");
        var secondsText = (TimeOut.Seconds / 10 < 1 ? $"0{TimeOut.Seconds}" : $"{TimeOut.Seconds}");

        seasonEventTime.text = $"{TimeOut.Days}" + NTextManager.Instance.GetText("COMMON_MEASURE_DAY_COUNT") + $" {hoursText}" + ":" + $"{minutesText}" + ":" + $"{secondsText}";

    }
    public void OnClick_RewardButtion()
    {
        //�� ���� NDT�� �׻� 0���� �����Ѵ� 
        // ��/ �ı� ����
        // 0 �� ���� ���� 
        // 1 ī�� 1�� ����
        // 2 ī�� 2�� ����
        // ....
        // �����̱� ������ ���⿡�� �ı�� �Ѿ�� 0���� �˻��غ� ������ ����.
        bool activeToolTip = true;
        if (seasonDataInfo.SeasonEventKind != 0)
         {
            if (ChloeStoryEventManager.Instance.dicRewardEvent.ContainsKey(seasonDataInfo.SeasonEventKind))
            {
                if (ChloeStoryEventManager.Instance.dicRewardEvent[seasonDataInfo.SeasonEventKind] == (int)ChloeStoryEventManager.SeasonEventRewardState.CanReceiveReward)
                {
                    // ���� ���� ó���� ���� ��Ŷ ������ �۽�
                    NFlatBufferBuilder.SendBytes<GS_SEASON_EVENT_REWARD_REQ>(ePACKET_ID.GS_SEASON_EVENT_REWARD_REQ, () => GS_SEASON_EVENT_REWARD_REQ.CreateGS_SEASON_EVENT_REWARD_REQ(FlatBuffers.NFlatBufferBuilder.FBB,
                      UserBase.User.UID,
                    (int)ChloeStoryEventManager.SeasonEventRewardState.ShowAllCompletedMissionReward,
                     seasonDataInfo.SeasonEventKind
                     )); //  ����
                    activeToolTip = false;
                }
            }
            if(activeToolTip)
            {
                var iteminfo = BaseTable.TableItemInfo.Instance.Get(seasonDataInfo.SeasonRewards[0].Kind);
                if (null == iteminfo)
                    return;
                var titleText = NLibCs.NTextManager.Instance.GetText(iteminfo.itemName);
                var descText = NLibCs.NTextManager.Instance.GetText(iteminfo.itemInstruction);

                ToolTipDlg.SetToolTipDlg(titleText, descText);
            }
         }
    }

    private void OnClick_Question()
    {

        PublicSoundMethod.PlayClickCommon(GameDefine.eCLICK_SOUND_TYPE.CLICK_SOUND_4);

        var contentskind = (int)GameDefine.eQuestionPopUpContentsType.SeasonEvent;
        BaseQuestionDlg.OpenQuestionDlg(contentskind);
    }

}
