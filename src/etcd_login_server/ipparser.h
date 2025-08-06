
/*
 Reviser: Polaris_hzn8
 Email: lch2022fox@163.com
 filename: ipparser.h
 Update Time: Wed 06 Aug 2025 17:04:56 CST
 brief: 
*/

#ifndef _IPPARSER_H_
#define _IPPARSER_H_

#include "util.h"

class IpParser
{
    public:
        IpParser();
        virtual ~IpParser();
    bool isTelcome(const char* ip);
};

#endif

