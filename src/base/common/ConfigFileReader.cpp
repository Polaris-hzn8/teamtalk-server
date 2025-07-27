/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ConfigFileReader.cpp
 Update Time: Mon 12 Jun 2023 19:41:00 CST
 brief: 
*/

#include "ConfigFileReader.h"

CConfigFileReader::CConfigFileReader(const char* file_path)
{
    _LoadFile(file_path);
}

CConfigFileReader::~CConfigFileReader()
{

}

char* CConfigFileReader::GetConfigName(const char* key)
{
    if (!m_load_ok)
        return NULL;
	char* value = NULL;
    std::map<std::string, std::string>::iterator it = m_config_map.find(key);
    if (it != m_config_map.end())
        value = (char*)it->second.c_str();
    return value;
}

bool CConfigFileReader::SetConfigValue(const char* key, const char* value)
{
    if (!m_load_ok)
        return false;
    std::map<std::string, std::string>::iterator it = m_config_map.find(key);
    if (it != m_config_map.end()) {
        it->second = value;
    } else {
        m_config_map.insert(std::make_pair(key, value));
    }
    return _WriteFIle();
}

bool CConfigFileReader::_LoadFile(const char* file_path)
{
    m_load_ok = false;
    m_config_file.clear();
    m_config_file.append(file_path);

    FILE* fp = fopen(file_path, "r");
    if (!fp) {
        log_error("Failed to open config file[%s]: %s (errno=%d)", file_path, strerror(errno), errno);
        return false;
    }

    char buffLine[256] = { 0 };
    for (;;) {
        // Read file line by line
        char* line = NULL;
        if ((line = fgets(buffLine, sizeof(buffLine), fp)) == NULL)
            break;

        // Remove trailing newline if exists
        size_t len = strlen(buffLine);
        if (len > 0 && buffLine[len - 1] == '\n') {
            buffLine[len - 1] = '\0';
            len--;
        }
        
        // Ignore empty lines after trimming
        if (len == 0)
            continue;

        // Handle comments (everything after '#')
        char* pos = strchr(buffLine, '#');
        if (pos != nullptr) {
            *pos = '\0';
            // Re-check if line is empty after removing comment
            if (strlen(buffLine) == 0)
                continue;
        }
        // Parse the actual configuration line
        _ParseLine(buffLine);
    }
    m_load_ok = true;
    fclose(fp);
    return true;
}

// 将配置写入到文件中
bool CConfigFileReader::_WriteFIle(const char* file_path)
{
    const char* config_path = file_path ? file_path : m_config_file.c_str();
    FILE* fp = NULL;
    if ((fp = fopen(config_path, "w")) == NULL) {
        log_error("Failed to open config file[%s]: %s (errno=%d)", file_path, strerror(errno), errno);
        return false;
    }

    char szPaire[128] = { 0 };
    std::map<std::string, std::string>::iterator it = m_config_map.begin();
    for (; it != m_config_map.end(); it++) {
        memset(szPaire, 0, sizeof(szPaire));
        snprintf(szPaire, sizeof(szPaire), "%s=%s\n", it->first.c_str(), it->second.c_str());
        uint32_t ret = fwrite(szPaire, strlen(szPaire), 1, fp);
        if (ret != 1) {
            fclose(fp);
            return false;
        }
    }
    fclose(fp);
    return true;
}

void CConfigFileReader::_ParseLine(char* line)
{
    if (!line || *line == '\0')
        return;

    char* pos = NULL;
    if ((pos = strchr(line, '=')) == NULL)
        return;

    *pos = '\0';
    char* key = _TrimSpace(line);
    char* value = _TrimSpace(pos + 1);
    if (key && value)
        m_config_map.insert(std::make_pair(key, value));
}

char* CConfigFileReader::_TrimSpace(char* str)
{
    if (str == NULL)
        return NULL;
    char* pos = str;
    // Skip leading spaces and tabs
    while ((*pos == ' ') || (*pos == '\t'))
        pos++;

    // If the string is empty after trimming leading spaces
    if (*str == '\0')
        return NULL;

    // Move to the end of the string
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t')) {
        *end = '\0';  // 直接截断字符串
        --end;
    }
    return str;
}

