/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: FileModel.h
 Update Time: Thu 15 Jun 2023 01:05:45 CST
 brief:
*/

#ifndef __FILEMODEL_H__
#define __FILEMODEL_H__
#include "IM.File.pb.h"
#include "ImPduBase.h"

class CFileModel {
public:
    virtual ~CFileModel();
    static CFileModel* getInstance();

    void getOfflineFile(uint32_t userId, list<IM::BaseDefine::OfflineFileInfo>& lsOffline);
    void addOfflineFile(uint32_t fromId, uint32_t toId, string& taskId, string& fileName, uint32_t fileSize);
    void delOfflineFile(uint32_t fromId, uint32_t toId, string& taskId);

private:
    CFileModel();

private:
    static CFileModel* m_pInstance;
};

#endif /*defined(__FILEMODEL_H__) */
