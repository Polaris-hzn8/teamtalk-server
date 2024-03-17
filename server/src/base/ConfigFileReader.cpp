/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ConfigFileReader.cpp
 Update Time: Mon 12 Jun 2023 19:41:00 CST
 brief: 
*/

#include "ConfigFileReader.h"
CConfigFileReader::CConfigFileReader(const char* filename) {
    _LoadFile(filename);
}

CConfigFileReader::~CConfigFileReader() {}

char* CConfigFileReader::GetConfigName(const char* name) {
    if (!m_load_ok) return NULL;
	// 用于存储查询结果
	char* value = NULL;
	// 在map中查找对应的配置项的值
    map<string, string>::iterator it = m_config_map.find(name);
	// 将查到的结果赋值给value
    if (it != m_config_map.end()) value = (char*)it->second.c_str();
    return value;
}

int CConfigFileReader::SetConfigValue(const char* name, const char* value) {
    if (!m_load_ok) return -1;
	// 在map中查找配置项名称
    map<string, string>::iterator it = m_config_map.find(name);
	// 将value设置到map配置项中
    if (it != m_config_map.end()) it->second = value;
    else m_config_map.insert(make_pair(name, value));
	// 将map写入到文件中
    return _WriteFIle();
}

void CConfigFileReader::_LoadFile(const char* filename) {
    m_config_file.clear();
	// 1.以可读的方式打开配置文件
    m_config_file.append(filename);
    FILE* fp = fopen(filename, "r");
	// 2.打开失败则记录错误日志并返回
    if (!fp) {
        log_error("can not open %s,errno = %d", filename, errno);
        return;
    }

	// 3.循环读取文件中的每一行内容，直到文件结束
    char buf[256];
    for (;;) {
		// 3-1.获取每一行内容
        char* p = fgets(buf, 256, fp);
        if (!p) break;

		// 3-2.去除行末的换行符\n，将其替换为字符串结束符
        size_t len = strlen(buf);
        if (buf[len - 1] == '\n') buf[len - 1] = 0;

		// 3-3.查找行中的 # 字符，并将其后的内容截断，以去除注释部分
        char* ch = strchr(buf, '#');
        if (ch) *ch = 0;

		// 3-4.如果处理后的行长度为0，则跳过此行
        if (strlen(buf) == 0) continue;

		// 3-5.调用 _ParseLine 函数解析配置项
        _ParseLine(buf);
    }

	// 4.关闭配置文件指针 将加载成功标志设置为 true
    fclose(fp);
    m_load_ok = true;
}

//将配置项写入到配置文件中 
int CConfigFileReader::_WriteFIle(const char* filename) {
    FILE* fp = NULL;

	// 1.根据传入的文件名确定要写入的文件路径，并使用文件指针fp打开文件以供写入
    if (filename == NULL) fp = fopen(m_config_file.c_str(), "w");
    else fp = fopen(filename, "w");
    if (fp == NULL) return -1;//文件打开失败返回错误码 -1

	// 2.遍历配置映射容器m_config_map中的每个配置项，逐个将配置项写入文件
    char szPaire[128];
    map<string, string>::iterator it = m_config_map.begin();
    for (; it != m_config_map.end(); it++) {
        memset(szPaire, 0, sizeof(szPaire));
        snprintf(szPaire, sizeof(szPaire), "%s=%s\n", it->first.c_str(), it->second.c_str());
        uint32_t ret = fwrite(szPaire, strlen(szPaire), 1, fp);
		// 如果写入失败，则关闭文件指针并返回错误码 -1
        if (ret != 1) {
            fclose(fp);
            return -1;
        }
    }

	// 3.关闭文件指针 所有配置项都成功写入文件，则返回成功标志 0
    fclose(fp);
    return 0;
}

//解析配置文件中的某行 插入到配置映射容器m_config_map中
void CConfigFileReader::_ParseLine(char* line) {
    char* p = strchr(line, '=');
    if (p == NULL) return;

    *p = 0;
    char* key = _TrimSpace(line);
    char* value = _TrimSpace(p + 1);
    if (key && value) m_config_map.insert(make_pair(key, value));
}

//去除字符串两端的空格和制表符
char* CConfigFileReader::_TrimSpace(char* name) {
    // 1.定义指针start_pos指向字符串name的起始位置
    char* start_pos = name;
    while ((*start_pos == ' ') || (*start_pos == '\t')) start_pos++;
	// 检查起始位置之后的子串长度，如果长度为0，则返回空指针
    if (strlen(start_pos) == 0) return NULL;

	// 2.定义指针end_pos指向字符串name的结束位置
    char* end_pos = name + strlen(name) - 1;
    while ((*end_pos == ' ') || (*end_pos == '\t')) {
        *end_pos = 0;//设为字符串结束符'\0'
        end_pos--;
    }

	// 3.计算去除空格和制表符后的字符串长度len 
    int len = (int)(end_pos - start_pos) + 1;
    if (len <= 0) return NULL; //如果长度小于等于0，则返回空指针
    return start_pos;
}


