/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: security.h
 Update Time: Sun 11 Jun 2023 10:23:52 CST
 brief: 
    主要用于 消息message 与 密码password 的加解密（AES加解密和MD5计算）
     - EncryptMsg：对消息进行加密
     - DecryptMsg：对消息进行解密
     - EncryptPass：对密码进行加密
     - DecryptPass：对密码进行解密
     - Free：释放资源
    
    并且根据不同的平台有相应的实现
    在C++中这些函数声明了C风格的接口
    在Android平台上，提供了与Java代码的交互接口
*/

#ifndef __SECURITY_H__
#define __SECURITY_H__


#ifdef _WIN32
typedef char			int8_t;
typedef short			int16_t;
typedef int				int32_t;
typedef	long long		int64_t;
typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned int	uint32_t;
typedef	unsigned long long	uint64_t;
typedef int				socklen_t;
#else
#include <stdint.h>
#endif


typedef unsigned char	uchar_t;


#ifdef WIN32
#define DLL_MODIFIER __declspec(dllexport)
#else
#define DLL_MODIFIER
#endif


#ifdef __cplusplus
extern "C" {
#endif

// 在编译过程中根据条件选择性地包含或排除特定的代码块
#ifdef __ANDROID__
    // 如果定义了__ANDROID__宏那么以下三行代码将被包含在编译过程中
    // 定义了三个函数原型 的Java方法关联 用于在 Android 平台上执行消息加密和解密操作的本地方法
    jstring Java_com_mogujie_im_security_EncryptMsg(JNIEnv* env, jobject obj, jstring jstr);
    jstring Java_com_mogujie_im_security_DecryptMsg(JNIEnv* env, jobject obj, jstring jstr);
    jstring Java_com_mogujie_im_security_EncryptPass(JNIEnv* env, jobject obj, jstring jstr, jstring jkey);
#else
    /**
     *  @brief 1.对消息加密
     *
     *  @param pInData  待加密的消息内容指针
     *  @param nInLen   待加密消息内容长度
     *  @param pOutData 加密后的文本
     *  @param nOutLen  加密后的文本长度
     *
     *  @return 返回 0-成功; 其他-失败
     */
    DLL_MODIFIER int EncryptMsg(const char* pInData, uint32_t nInLen, char** pOutData, uint32_t& nOutLen);
    
    /**
     *  @brief 2.对消息解密
     *
     *  @param pInData  待解密的消息内容指针
     *  @param nInLen   待解密消息内容长度
     *  @param pOutData 解密后的文本
     *  @param nOutLen  解密后的文本长度
     *
     *  @return 返回 0-成功; 其他-失败
     */
    DLL_MODIFIER int DecryptMsg(const char* pInData, uint32_t nInLen, char** pOutData, uint32_t& nOutLen);
    
    /**
     *  @brief 3.对密码进行加密
     *
     *  @param pInData  待解密的消息内容指针
     *  @param nInLen   待解密消息内容长度
     *  @param pOutData 解密后的文本
     *  @param nOutLen  解密后的文本长度
     *  @param pKey     32位密钥
     *
     *  @return 返回 0-成功; 其他-失败
     */
    DLL_MODIFIER int EncryptPass(const char* pInData, uint32_t nInLen, char** pOutData, uint32_t& nOutLen, const char* pKey);
    
    /**
     *  @brief 4.对密码进行解密
     *
     *  @param pInData  待解密的消息内容指针
     *  @param nInLen   待解密消息内容长度
     *  @param pOutData 解密后的文本
     *  @param nOutLen  解密后的文本长度
     *  @param pKey     32位密钥
     *
     *  @return 返回 0-成功; 其他-失败
     */
    DLL_MODIFIER int DecryptPass(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen, const char* pKey);
    
    /**
     *  @brief 5.释放资源
     *  @param pOutData 需要释放的资源
     */
    DLL_MODIFIER void Free(char* pOutData);
#endif


#ifdef __cplusplus
}
#endif


#endif
