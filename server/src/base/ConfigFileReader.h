/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ConfigFileReader.h
 Update Time: Mon 12 Jun 2023 23:06:46 CST
 brief: 
*/

#ifndef CONFIGFILEREADER_H_
#define CONFIGFILEREADER_H_

#include "util.h"

class CConfigFileReader {
public:
    CConfigFileReader(const char* filename);
    ~CConfigFileReader();

    //根据配置项名称获取对应的配置值
    char* GetConfigName(const char* name);
    //设置配置项的值
    int SetConfigValue(const char* name, const char* value);
private:
    //加载配置文件内容
    void _LoadFile(const char* filename);
    //将配置内容写入文件
    int _WriteFIle(const char* filename = NULL);
    //解析配置文件中的一行内容
    void _ParseLine(char* line);
    //去除配置项名称中的空格
    char* _TrimSpace(char* name);

    bool m_load_ok;//配置文件是否成功加载
    map<string, string> m_config_map;//存储配置项和对应值的映射关系的容器
    string m_config_file;//配置文件的路径
};

#endif
