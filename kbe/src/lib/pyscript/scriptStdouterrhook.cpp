#include "scriptstdouterrhook.hpp"
namespace KBEngine{ namespace script{
								
SCRIPT_METHOD_DECLARE_BEGIN(ScriptStdOutErrHook)	
SCRIPT_METHOD_DECLARE_END()


SCRIPT_MEMBER_DECLARE_BEGIN(ScriptStdOutErrHook)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ScriptStdOutErrHook)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ScriptStdOutErrHook, 0, 0, 0, 0, 0)		

//-------------------------------------------------------------------------------------
ScriptStdOutErrHook::ScriptStdOutErrHook()
{
}

//-------------------------------------------------------------------------------------
ScriptStdOutErrHook::~ScriptStdOutErrHook()
{
}

//-------------------------------------------------------------------------------------
void ScriptStdOutErrHook::onPrint(const char* msg)
{
	if(buffer_)
		(*buffer_) += msg;
	ScriptStdOutErr::onPrint(msg);
}

//-------------------------------------------------------------------------------------

}
}