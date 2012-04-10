#include "address.hpp"

namespace KBEngine { 
namespace Mercury
{
char Address::s_stringBuf[ 2 ][32];
int Address::s_currStringBuf = 0;

//-------------------------------------------------------------------------------------
int Address::writeToString(char * str, int length) const
{
	uint32	hip = ntohl(ip);
	uint16	hport = ntohs(port);

	return kbe_snprintf(str, length,
		"%d.%d.%d.%d:%d",
		(int)(uchar)(hip>>24),
		(int)(uchar)(hip>>16),
		(int)(uchar)(hip>>8),
		(int)(uchar)(hip),
		(int)hport);
}

//-------------------------------------------------------------------------------------
char * Address::c_str() const
{
	char * buf = Address::nextStringBuf();
	this->writeToString(buf, 32);
    return buf;
}

//-------------------------------------------------------------------------------------
const char * Address::ipAsString() const
{
	uint32	hip = ntohl(ip);
	char * buf = Address::nextStringBuf();

	kbe_snprintf(buf, 32, "%d.%d.%d.%d",
		(int)(uchar)(hip>>24),
		(int)(uchar)(hip>>16),
		(int)(uchar)(hip>>8),
		(int)(uchar)(hip));

    return buf;
}

//-------------------------------------------------------------------------------------
char * Address::nextStringBuf()
{
	s_currStringBuf = (s_currStringBuf + 1) % 2;
	return s_stringBuf[ s_currStringBuf ];
}

}
}