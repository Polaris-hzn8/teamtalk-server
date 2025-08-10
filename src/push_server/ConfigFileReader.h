/*
 Reviser: Polaris_hzn8
 Email: lch2022fox@163.com
 filename: ConfigFileReader.h
 Update Time: Sun 10 Aug 2025 12:08:33 CST
 brief: 
*/

#ifndef CONFIGFILEREADER_H_
#define CONFIGFILEREADER_H_

#include <map>
#include <string>

class CConfigFileReader
{
public:
	CConfigFileReader(const char* filename);
	~CConfigFileReader();

	char* GetConfigName(const char* name);
private:
	void _LoadFile(const char* filename);
	void _ParseLine(char* line);
	char* _TrimSpace(char* name);

	bool m_load_ok;
	std::map<std::string, std::string>*	m_config_map;
};



#endif /* CONFIGFILEREADER_H_ */
