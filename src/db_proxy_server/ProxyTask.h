/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ProxyTask.h
 Update Time: Wed 14 Jun 2023 23:06:15 CST
 brief:
*/

#ifndef __PROXY_TASK_H__
#define __PROXY_TASK_H__
#include "ImPduBase.h"
#include "Task.h"
#include "util.h"

typedef void (*pdu_handler_t)(CImPdu* pPdu, uint32_t conn_uuid);

class CProxyTask : public CTask {
public:
    CProxyTask(uint32_t conn_uuid, pdu_handler_t pdu_handler, CImPdu* pPdu);
    virtual ~CProxyTask();

    virtual void run();

private:
    uint32_t m_conn_uuid;
    pdu_handler_t m_pdu_handler;
    CImPdu* m_pPdu;
};
#endif
