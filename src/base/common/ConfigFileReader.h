/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ConfigFileReader.h
 Update Time: Mon 12 Jun 2023 23:06:46 CST
 brief: 
*/

#ifndef _CONFIGFILEREADER_H_
#define _CONFIGFILEREADER_H_

#include <map>
#include "util.h"

class CConfigFileReader
{
public:
    CConfigFileReader(const char* file_path);
    ~CConfigFileReader();

    char*   GetConfigName(const char* key);
    bool    SetConfigValue(const char* key, const char* value);
private:
    bool        _LoadFile(const char* file_path);
    bool        _WriteFIle(const char* file_path = NULL);
    void        _ParseLine(char* line);
    char*       _TrimSpace(char* str);

    bool                                m_load_ok;          //是否加载成功
    std::string                         m_config_file;      //配置文件路径
    std::map<std::string, std::string>  m_config_map;       //配置存储
};

#endif
