
/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Wed 06 Aug 2025 16:59:59 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#include "ipparser.h"

IpParser::IpParser()
{
}

IpParser::~IpParser()
{
    
}

bool IpParser::isTelcome(const char *pIp)
{
    if(!pIp)
    {
        return false;
    }
    CStrExplode strExp((char*)pIp,'.');
    if(strExp.GetItemCnt() != 4)
    {
        return false;
    }
    return true;
}
