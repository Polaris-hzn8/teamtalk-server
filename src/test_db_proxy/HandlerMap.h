/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: HandlerMap.h
 Update Time: Thu 15 Jun 2023 01:03:42 CST
 brief:
*/

#ifndef HANDLERMAP_H_
#define HANDLERMAP_H_

#include "../base/util.h"
#include "ProxyTask.h"

typedef map<uint32_t, pdu_handler_t> HandlerMap_t;

class CHandlerMap {
public:
    virtual ~CHandlerMap();

    static CHandlerMap* getInstance();

    void Init();
    pdu_handler_t GetHandler(uint32_t pdu_type);

private:
    CHandlerMap();

private:
    static CHandlerMap* s_handler_instance;

    HandlerMap_t m_handler_map;
};

#endif /* HANDLERMAP_H_ */
