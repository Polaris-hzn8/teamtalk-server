/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: HandlerMap.h
 Update Time: Wed 14 Jun 2023 23:02:17 CST
 brief: 根据PDU类型返回对应处理程序
*/

#ifndef HANDLERMAP_H_
#define HANDLERMAP_H_

#include "util.h"
#include "ProxyTask.h"

typedef std::map<uint32_t, pdu_handler_t> HandlerMap_t;

class CHandlerMap
{
public:
    virtual ~CHandlerMap();
    static CHandlerMap* getInstance();

    void Init();
    pdu_handler_t GetHandler(uint32_t pdu_type);
private:
    CHandlerMap();
    static CHandlerMap* s_handler_instance;
    HandlerMap_t m_handler_map;
};

#endif
