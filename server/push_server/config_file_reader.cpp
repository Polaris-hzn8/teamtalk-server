/*
 Reviser: Polaris_hzn8
 Email: lch2022fox@163.com
 filename: ConfigFileReader.cpp
 Update Time: Sun 10 Aug 2025 12:08:22 CST
 brief:
*/

#include "config_file_reader.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>

CConfigFileReader::CConfigFileReader(const char* filename) : m_load_ok(false) {
  _LoadFile(filename);
}

CConfigFileReader::~CConfigFileReader() {}

std::string CConfigFileReader::GetConfigValue(const std::string& name) {
  return GetConfigValue(name.c_str());
}

std::string CConfigFileReader::GetConfigValue(const char* name) {
  if (!m_load_ok)
    return "";
  auto it = m_config_map.find(name);
  if (it != m_config_map.end()) {
    return it->second;
  }
  return "";
}

int CConfigFileReader::GetIntValue(const std::string& name, int default_val) {
  return GetIntValue(name.c_str(), default_val);
}

int CConfigFileReader::GetIntValue(const char* name, int default_val) {
  std::string val = GetConfigValue(name);
  if (val.empty())
    return default_val;
  return std::atoi(val.c_str());
}

uint32_t CConfigFileReader::GetUint32Value(const std::string& name, uint32_t default_val) {
  return GetUint32Value(name.c_str(), default_val);
}

uint32_t CConfigFileReader::GetUint32Value(const char* name, uint32_t default_val) {
  std::string val = GetConfigValue(name);
  if (val.empty())
    return default_val;
  char* end = nullptr;
  long result = std::strtol(val.c_str(), &end, 10);
  if (*end != '\0' || result < 0)
    return default_val;
  return static_cast<uint32_t>(result);
}

bool CConfigFileReader::GetBoolValue(const std::string& name, bool default_val) {
  return GetBoolValue(name.c_str(), default_val);
}

bool CConfigFileReader::GetBoolValue(const char* name, bool default_val) {
  std::string val = GetConfigValue(name);
  if (val.empty())
    return default_val;
  if (val == "1" || val == "true" || val == "yes" || val == "on") {
    return true;
  }
  if (val == "0" || val == "false" || val == "no" || val == "off") {
    return false;
  }
  return default_val;
}

void CConfigFileReader::_LoadFile(const char* filename) {
  FILE* fp = fopen(filename, "r");
  if (!fp) {
    return;
  }

  char buf[256];
  for (;;) {
    char* p = fgets(buf, sizeof(buf), fp);
    if (!p)
      break;

    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
      buf[len - 1] = '\0';
      len--;
    }

    char* ch = strchr(buf, '#');
    if (ch)
      *ch = '\0';

    if (strlen(buf) == 0)
      continue;

    _ParseLine(buf);
  }

  fclose(fp);
  m_load_ok = true;
}

void CConfigFileReader::_ParseLine(char* line) {
  char* p = strchr(line, '=');
  if (p == nullptr)
    return;

  *p = '\0';
  char* key = _TrimSpace(line);
  char* value = _TrimSpace(p + 1);
  if (key && value) {
    m_config_map[key] = value;
  }
}

char* CConfigFileReader::_TrimSpace(char* name) {
  if (name == nullptr)
    return nullptr;

  // remove starting space or tab
  char* start_pos = name;
  while ((*start_pos == ' ') || (*start_pos == '\t')) {
    start_pos++;
  }

  if (strlen(start_pos) == 0)
    return nullptr;

  // remove ending space or tab
  char* end_pos = start_pos + strlen(start_pos) - 1;
  while ((end_pos > start_pos) && ((*end_pos == ' ') || (*end_pos == '\t'))) {
    *end_pos = '\0';
    end_pos--;
  }

  return start_pos;
}
