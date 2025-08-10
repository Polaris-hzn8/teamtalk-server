/*
 Reviser: Polaris_hzn8
 Email: lch2022fox@163.com
 filename: base_type.h
 Update Time: Sun 10 Aug 2025 12:22:42 CST
 brief: 
*/

#ifndef BASE_TYPE_H
#define BASE_TYPE_H

#ifdef _WIN32
#else
    #include <sys/types.h>
    #include <stdint.h>		// define int8_t ...
    #include <errno.h>
#endif

#ifdef _WIN32
    typedef int				socklen_t;
    #if (_MSC_VER >= 1800)
        #include <stdint.h>
    #else
        typedef char			int8_t;
        typedef short			int16_t;
        typedef int				int32_t;
        typedef	long long		int64_t;
        typedef unsigned char	uint8_t;
        typedef unsigned short	uint16_t;
        typedef unsigned int	uint32_t;
        typedef	unsigned long long	uint64_t;
    #endif
#else
    typedef unsigned char	uchar_t;
    typedef int BOOL;
    const int TRUE = 1;
    const int FALSE = 0;
    
    #ifndef NULL
    #define NULL 0
    #endif
#endif

#endif
