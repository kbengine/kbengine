
#include "Method.h"
#include "DataTypes.h"
#include "KBDebug.h"

Method::Method():
	name(TEXT("")),
	methodUtype(0),
	aliasID(-1),
	args(),
	pEntityDefMethodHandle(NULL)
{
}

Method::~Method()
{
	KBE_SAFE_RELEASE(pEntityDefMethodHandle);
}
