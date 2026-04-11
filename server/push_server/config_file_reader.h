/*
 Reviser: Polaris_hzn8
 Email: lch2022fox@163.com
 filename: ConfigFileReader.h
 Update Time: Sun 10 Aug 2025 12:08:33 CST
 brief:
*/

#ifndef CONFIGFILEREADER_H_
#define CONFIGFILEREADER_H_

#include <cstdint>
#include <map>
#include <string>

class CConfigFileReader {
 public:
  explicit CConfigFileReader(const char* filename);
  ~CConfigFileReader();

  std::string GetConfigValue(const std::string& name);
  std::string GetConfigValue(const char* name);

  int GetIntValue(const std::string& name, int default_val = 0);
  int GetIntValue(const char* name, int default_val = 0);
  uint32_t GetUint32Value(const std::string& name, uint32_t default_val = 0);
  uint32_t GetUint32Value(const char* name, uint32_t default_val = 0);
  bool GetBoolValue(const std::string& name, bool default_val = false);
  bool GetBoolValue(const char* name, bool default_val = false);

  bool IsLoadSuccess() const { return m_load_ok; }

 private:
  void _LoadFile(const char* filename);
  void _ParseLine(char* line);
  char* _TrimSpace(char* name);

  bool m_load_ok;
  std::map<std::string, std::string> m_config_map;
};

#endif /* CONFIGFILEREADER_H_ */
