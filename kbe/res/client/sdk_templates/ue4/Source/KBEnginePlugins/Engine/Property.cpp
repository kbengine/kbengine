
#include "Property.h"
#include "KBDebug.h"

Property::Property():
	name(TEXT("")),
	pUtype(NULL),
	properUtype(0),
	properFlags(0),
	aliasID(-1),
	defaultValStr(TEXT("")),
	pDefaultVal(NULL)
{
}

Property::~Property()
{
	KBE_SAFE_RELEASE(pDefaultVal);
}
