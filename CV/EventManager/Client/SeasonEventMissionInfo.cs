using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using BaseTable;
using NLibCs;
using DEFINE;
using Contents.User;


public class SeasonEventMissionInfo : UIForm
{
    [SerializeField] private GameObject clearMark_1;                       // ���� �̼� 1 Ŭ���� UI ǥ�� ����
    [SerializeField] private GameObject clearMark_2;                       // ���� �̼� 2 Ŭ���� UI ǥ�� ����
    [SerializeField] private GameObject clearMark_3;                       // ���� �̼� 3 Ŭ���� UI ǥ�� ����


    [SerializeField] private UIPlaySound playSound_1;                         // ���� ����Ʈ
    [SerializeField] private UIPlaySound playSound_2;                         // ���� ����Ʈ
    [SerializeField] private UIPlaySound playSound_3;                         // ���� ����Ʈ

    [SerializeField] private UILabel seasonMission_1;                      // ���� �̼� 1 ��
    [SerializeField] private UILabel seasonMission_2;                      // ���� �̼� 2 ��
    [SerializeField] private UILabel seasonMission_3;                      // ���� �̼� 3 ��

    [SerializeField] private UILabel seasonMission_1_Progress;             // ���� �̼� 1 �� ���൵
    [SerializeField] private UILabel seasonMission_2_Progress;             // ���� �̼� 2 �� ���൵
    [SerializeField] private UILabel seasonMission_3_Progress;             // ���� �̼� 3 �� ���൵

    [SerializeField] private UISprite rewardImage;                         // ���� �̹���
    [SerializeField] private UILabel rewardLabelVolume;                    // ���� ����
    [SerializeField] private UILabel rewardLabelValue;                     // ���� ��
    [SerializeField] private GameObject rewardEffect;                      // ���� ����Ʈ
    public void SetMissionLabel(int index, string MissionTextKey,int curProcess, int targetProcess)
    {
        if (index == 1) 
        { 
            if(seasonMission_1)seasonMission_1.text = NTextManager.Instance.GetText(MissionTextKey) ;
            if(seasonMission_1_Progress)seasonMission_1_Progress.text = curProcess.ToString() + "/" + targetProcess.ToString();
        }
        else if (index == 2)
        {
            if(seasonMission_2)seasonMission_2.text = NTextManager.Instance.GetText(MissionTextKey);
            if(seasonMission_2_Progress)seasonMission_2_Progress.text = curProcess.ToString() + "/" + targetProcess.ToString();
        }
        else if (index == 3)
        { 
            if(seasonMission_3)seasonMission_3.text = NTextManager.Instance.GetText(MissionTextKey);
            if(seasonMission_3_Progress)seasonMission_3_Progress.text = curProcess.ToString() + "/" + targetProcess.ToString();
        }
    }

    public override void BindUIEvents()
    {
        base.BindUIEvents();
    }

    public void SetActiveMissionClearMark(int index, bool setClear)
    {
        if (index == 1) clearMark_1.SetActive(setClear);
        else if (index == 2) clearMark_2.SetActive(setClear);
        else if (index == 3) clearMark_3.SetActive(setClear);
    }
    public void RewardEffect()
    {
        rewardEffect.SetActive(true);
    }
    public void SetRewardItem(int  kind, int value)
    {
        var itemInfo = TableItemInfo.Instance.Get(kind);
        if (itemInfo == null) return;

        if(rewardImage)SetSprite(ref rewardImage, itemInfo.atlasName, itemInfo.itemIcon);
        if(rewardLabelValue) SetText(ref rewardLabelValue, value.ToString());
        if(rewardLabelVolume) SetText(ref rewardLabelVolume, PublicUIMethod.GetString_ItemVolumeText(itemInfo));
    }

    public void FinishSound()
    {
        if (playSound_1) playSound_1.enabled = false;
        if (playSound_2) playSound_2.enabled = false;
        if (playSound_3) playSound_3.enabled = false;
    }

}
