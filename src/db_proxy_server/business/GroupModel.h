/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: GroupModel.h
 Update Time: Thu 15 Jun 2023 00:30:58 CST
 brief:
*/

#ifndef __IM_GROUP_MODEL__
#define __IM_GROUP_MODEL__

#include <set>
#include <map>
#include <list>
#include <string>
#include <stdio.h>
#include <iostream>

#include "MessageModel.h"
#include "IM.BaseDefine.pb.h"

const uint32_t MAX_UNREAD_COUNT = 100;

class CGroupModel
{
public:
    virtual ~CGroupModel();
    static CGroupModel* getInstance();

public:
    uint32_t createGroup(uint32_t nUserId, const std::string& strGroupName, const std::string& strGroupAvatar, uint32_t nGroupType, std::set<uint32_t>& setMember);
    bool removeGroup(uint32_t nUserId, uint32_t nGroupId, std::list<uint32_t>& lsCurUserId);
    void getUserGroup(uint32_t nUserId, std::list<IM::BaseDefine::GroupVersionInfo>& lsGroup, uint32_t nGroupType);
    void getUserGroupIds(uint32_t nUserId, std::list<uint32_t>& lsGroupId, uint32_t nLimited = 100);
    void getGroupInfo(std::map<uint32_t, IM::BaseDefine::GroupVersionInfo>& mapGroupId, std::list<IM::BaseDefine::GroupInfo>& lsGroupInfo);
    bool setPush(uint32_t nUserId, uint32_t nGroupId, uint32_t nType, uint32_t nStatus);
    void getPush(uint32_t nGroupId, std::list<uint32_t>& lsUser, std::list<IM::BaseDefine::ShieldStatus>& lsPush);
    bool modifyGroupMember(uint32_t nUserId, uint32_t nGroupId, IM::BaseDefine::GroupModifyType nType, std::set<uint32_t>& setUserId, std::list<uint32_t>& lsCurUserId);
    void getGroupUser(uint32_t nGroupId, std::list<uint32_t>& lsUserId);
    bool isInGroup(uint32_t nUserId, uint32_t nGroupId);
    void updateGroupChat(uint32_t nGroupId);
    bool isValidateGroupId(uint32_t nGroupId);
    uint32_t getUserJoinTime(uint32_t nGroupId, uint32_t nUserId);

private:
    CGroupModel();

    bool insertNewGroup(uint32_t reqUserId, const std::string& groupName, const std::string& groupAvatar, uint32_t groupType, uint32_t memCnt, uint32_t& groupId);
    bool insertNewMember(uint32_t nGroupId, std::set<uint32_t>& setUsers);
    string GenerateGroupAvatar(uint32_t groupId);
    void getGroupVersion(std::list<uint32_t>& lsGroupId, std::list<IM::BaseDefine::GroupVersionInfo>& lsGroup, uint32_t nGroupType);
    bool hasModifyPermission(uint32_t nUserId, uint32_t nGroupId, IM::BaseDefine::GroupModifyType nType);
    bool addMember(uint32_t nGroupId, std::set<uint32_t>& setUser, std::list<uint32_t>& lsCurUserId);
    bool removeMember(uint32_t nGroupId, std::set<uint32_t>& setUser, std::list<uint32_t>& lsCurUserId);
    void removeRepeatUser(uint32_t nGroupId, std::set<uint32_t>& setUser);
    void removeSession(uint32_t nGroupId, const std::set<uint32_t>& lsUser);
    bool incGroupVersion(uint32_t nGroupId);
    void clearGroupMember(uint32_t nGroupId);

    void fillGroupMember(std::list<IM::BaseDefine::GroupInfo>& lsGroups);

private:
    static CGroupModel* m_pInstance;
};

#endif /* defined(__IM_GROUP_MODEL__) */
