/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: HandlerMap.h
 Update Time: Wed 14 Jun 2023 23:02:17 CST
 brief: 采用单例模式的设计
*/

#ifndef HANDLERMAP_H_
#define HANDLERMAP_H_

#include "../base/util.h"
#include "ProxyTask.h"

//键为uint32_t类型、值为pdu_handler_t类型的映射（map）容器
typedef map<uint32_t, pdu_handler_t> HandlerMap_t;

/**
 * 处理程序映射类 CHandlerMap
 * 用于存储不同类型的 PDU 与处理程序的映射关系
*/
class CHandlerMap {
public:
    //销毁CHandlerMap类的实例
    virtual ~CHandlerMap();

    //静态成员函数 用于获取CHandlerMap类的唯一实例
    static CHandlerMap* getInstance();

    //初始化函数 用于初始化CHandlerMap实例
    void Init();
    //根据给定的PDU类型返回相应的处理程序
    pdu_handler_t GetHandler(uint32_t pdu_type);
private:
    //构造函数用于创建CHandlerMap类的实例
    CHandlerMap();

private:
    //静态成员变量，表示CHandlerMap类的唯一实例
    static CHandlerMap* s_handler_instance;
    //成员变量，存储PDU类型与处理程序的映射关系
    HandlerMap_t m_handler_map;
};

#endif
