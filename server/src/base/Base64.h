/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Base64.h
 Update Time: Sun 11 Jun 2023 16:30:23 CST
 brief: 使用以下两个函数来进行 Base64 编码和解码的操作
*/

#ifndef __BASE64_H__
#define __BASE64_H__

#include<iostream>
using namespace std;

// 对 Base64 编码的字符串进行解码，返回解码后的原始数据
string base64_decode(const string &ascdata);
// 对原始数据进行 Base64 编码，返回编码后的字符串
string base64_encode(const string &bindata);

#endif

