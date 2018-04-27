// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#ifndef KBE_HTTP_UTILTY_H
#define KBE_HTTP_UTILTY_H

#include "common/common.h"

namespace KBEngine 
{
namespace HttpUtility
{ 
    inline uint8 toHex(const uint8 &x)
    {
        return x > 9 ? x -10 + 'A': x + '0';
    }

    inline uint8 fromHex(const uint8 &x)
    {
        return isdigit(x) ? x-'0' : x-'A'+10;
    }
 
    inline std::string URLEncode(const std::string &sIn)
    {
        std::string sOut;
        
        for( size_t ix = 0; ix < sIn.size(); ix++ )
        {      
            uint8 buf[4];
            memset( buf, 0, 4 );
            
            if( isalnum( (uint8)sIn[ix] ) )
            {      
                buf[0] = sIn[ix];
            }
            //else if ( isspace( (uint8)sIn[ix] ) ) //貌似把空格编码成%20或者+都可以
            //{
            //    buf[0] = '+';
            //}
            else
            {
                buf[0] = '%';
                buf[1] = toHex( (uint8)sIn[ix] >> 4 );
                buf[2] = toHex( (uint8)sIn[ix] % 16);
            }
            
            sOut += (char *)buf;
        }
        
        return sOut;
    };

    inline std::string URLDecode(const std::string &sIn)
    {
        std::string sOut;
        
        for( size_t ix = 0; ix < sIn.size(); ix++ )
        {
            uint8 ch = 0;
            if(sIn[ix]=='%')
            {
                ch = (fromHex(sIn[ix+1])<<4);
                ch |= fromHex(sIn[ix+2]);
                ix += 2;
            }
            else if(sIn[ix] == '+')
            {
                ch = ' ';
            }
            else
            {
                ch = sIn[ix];
            }
            
            sOut += (char)ch;
        }
        
        return sOut;
    }
}

}


#endif // KBE_HTTP_UTILTY_H


