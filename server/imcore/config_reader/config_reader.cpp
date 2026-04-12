/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ConfigFileReader.cpp
 Update Time: Mon 12 Jun 2023 19:41:00 CST
 brief:
*/

#include "config_reader.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "crosslog.h"

CConfigReader::CConfigReader(const char* file_path) : m_load_ok(false) {
  _LoadFile(file_path);
}

CConfigReader::~CConfigReader() {}

std::string CConfigReader::GetConfigValue(const std::string& key) {
  return GetConfigValue(key.c_str());
}

std::string CConfigReader::GetConfigValue(const char* key) {
  if (!m_load_ok)
    return "";
  auto it = m_config_map.find(key);
  if (it != m_config_map.end()) {
    return it->second;
  }
  return "";
}

int CConfigReader::GetIntValue(const std::string& key, int default_val) {
  return GetIntValue(key.c_str(), default_val);
}

int CConfigReader::GetIntValue(const char* key, int default_val) {
  std::string val = GetConfigValue(key);
  if (val.empty())
    return default_val;
  return std::atoi(val.c_str());
}

uint32_t CConfigReader::GetUint32Value(const std::string& key, uint32_t default_val) {
  return GetUint32Value(key.c_str(), default_val);
}

uint32_t CConfigReader::GetUint32Value(const char* key, uint32_t default_val) {
  std::string val = GetConfigValue(key);
  if (val.empty())
    return default_val;
  char* end = nullptr;
  long result = std::strtol(val.c_str(), &end, 10);
  if (*end != '\0' || result < 0)
    return default_val;
  return static_cast<uint32_t>(result);
}

bool CConfigReader::GetBoolValue(const std::string& key, bool default_val) {
  return GetBoolValue(key.c_str(), default_val);
}

bool CConfigReader::GetBoolValue(const char* key, bool default_val) {
  std::string val = GetConfigValue(key);
  if (val.empty())
    return default_val;
  // 支持 "true", "1", "yes", "on" (不区分大小写)
  if (val == "1" || val == "true" || val == "yes" || val == "on") {
    return true;
  }
  // 支持 "false", "0", "no", "off"
  if (val == "0" || val == "false" || val == "no" || val == "off") {
    return false;
  }
  return default_val;
}

bool CConfigReader::SetConfigValue(const char* key, const char* value) {
  return SetConfigValue(std::string(key), std::string(value));
}

bool CConfigReader::SetConfigValue(const std::string& key, const std::string& value) {
  if (!m_load_ok)
    return false;
  m_config_map[key] = value;
  return _WriteFile();
}

bool CConfigReader::SetConfigValue(const std::string& key, int value) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%d", value);
  return SetConfigValue(key, std::string(buf));
}

bool CConfigReader::_LoadFile(const char* file_path) {
  m_load_ok = false;
  m_config_file.clear();
  m_config_file.append(file_path);

  FILE* fp = fopen(file_path, "r");
  if (!fp) {
    log_error("Failed to open config file[%s]: %s (errno=%d)", file_path, strerror(errno), errno);
    return false;
  }

  char buffLine[256] = {0};
  for (;;) {
    // Read file line by line
    char* line = nullptr;
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
bool CConfigReader::_WriteFile(const char* file_path) {
  const char* config_path = file_path ? file_path : m_config_file.c_str();
  FILE* fp = nullptr;
  if ((fp = fopen(config_path, "w")) == nullptr) {
    log_error("Failed to open config file[%s]: %s (errno=%d)", file_path, strerror(errno), errno);
    return false;
  }

  char szPaire[128] = {0};
  for (const auto& pair : m_config_map) {
    memset(szPaire, 0, sizeof(szPaire));
    snprintf(szPaire, sizeof(szPaire), "%s=%s\n", pair.first.c_str(), pair.second.c_str());
    uint32_t ret = fwrite(szPaire, strlen(szPaire), 1, fp);
    if (ret != 1) {
      fclose(fp);
      return false;
    }
  }
  fclose(fp);
  return true;
}

void CConfigReader::_ParseLine(char* line) {
  if (!line || *line == '\0')
    return;

  char* pos = nullptr;
  if ((pos = strchr(line, '=')) == nullptr)
    return;

  *pos = '\0';
  char* key = _TrimSpace(line);
  char* value = _TrimSpace(pos + 1);
  if (key && value)
    m_config_map[key] = value;
}

char* CConfigReader::_TrimSpace(char* str) {
  if (str == nullptr)
    return nullptr;
  char* pos = str;
  // Skip leading spaces and tabs
  while ((*pos == ' ') || (*pos == '\t'))
    pos++;

  // If the string is empty after trimming leading spaces
  if (*str == '\0')
    return nullptr;

  // Move to the end of the string
  char* end = str + strlen(str) - 1;
  while (end > str && (*end == ' ' || *end == '\t')) {
    *end = '\0';  // 直接截断字符串
    --end;
  }
  return str;
}
