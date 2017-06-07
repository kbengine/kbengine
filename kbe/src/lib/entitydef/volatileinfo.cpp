/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "volatileinfo.h"
#include "entitydef.h"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(VolatileInfo)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(VolatileInfo)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(VolatileInfo)
SCRIPT_GETSET_DECLARE("position",			pyGetPosition,			pySetPosition,			0,		0)
SCRIPT_GETSET_DECLARE("yaw",				pyGetYaw,				pySetYaw,				0,		0)
SCRIPT_GETSET_DECLARE("pitch",				pyGetPitch,				pySetPitch,				0,		0)
SCRIPT_GETSET_DECLARE("roll",				pyGetRoll,				pySetRoll,				0,		0)
SCRIPT_GETSET_DECLARE("optimized",			pyGetOptimized,			pySetOptimized,			0,		0)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(VolatileInfo, 0, 0, 0, 0, 0)

//-------------------------------------------------------------------------------------
const float VolatileInfo::ALWAYS = FLT_MAX;
const float VolatileInfo::NEVER = 0.f;

//-------------------------------------------------------------------------------------
int VolatileInfo::pySetPosition(PyObject *value)
{
	if (!PyFloat_Check(value))
	{
		PyErr_Format(PyExc_AssertionError, "%s: set position value is not float!\n",
			scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	position_ = (float)PyFloat_AsDouble(value);
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* VolatileInfo::pyGetPosition()
{
	return PyFloat_FromDouble(position_);
}

//-------------------------------------------------------------------------------------
int VolatileInfo::pySetYaw(PyObject *value)
{
	if (!PyFloat_Check(value))
	{
		PyErr_Format(PyExc_AssertionError, "%s: set yaw value is not float!\n",
			scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	yaw_ = (float)PyFloat_AsDouble(value);
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* VolatileInfo::pyGetYaw()
{
	return PyFloat_FromDouble(yaw_);
}

//-------------------------------------------------------------------------------------
int VolatileInfo::pySetPitch(PyObject *value)
{
	if (!PyFloat_Check(value))
	{
		PyErr_Format(PyExc_AssertionError, "%s: set pitch value is not float!\n",
			scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	pitch_ = (float)PyFloat_AsDouble(value);
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* VolatileInfo::pyGetPitch()
{
	return PyFloat_FromDouble(pitch_);
}

//-------------------------------------------------------------------------------------
int VolatileInfo::pySetRoll(PyObject *value)
{
	if (!PyFloat_Check(value))
	{
		PyErr_Format(PyExc_AssertionError, "%s: set roll value is not float!\n",
			scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	roll_ = (float)PyFloat_AsDouble(value);
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* VolatileInfo::pyGetRoll()
{
	return PyFloat_FromDouble(roll_);
}

//-------------------------------------------------------------------------------------
PyObject* VolatileInfo::pyGetOptimized()
{
	return PyBool_FromLong(optimized_);
}

//-------------------------------------------------------------------------------------
int VolatileInfo::pySetOptimized(PyObject *value)
{
	if (!PyBool_Check(value))
	{
		PyErr_Format(PyExc_AssertionError, "%s: set pitch value is not bool!\n",
			scriptName());
		PyErr_PrintEx(0);
		return 0;
	}

	optimized_ = value == Py_True;
	return 0;
}

//-------------------------------------------------------------------------------------
void VolatileInfo::addToStream(KBEngine::MemoryStream& s)
{
	s << position_ << yaw_ << roll_ << pitch_ << optimized_;
}

//-------------------------------------------------------------------------------------
void VolatileInfo::createFromStream(KBEngine::MemoryStream& s)
{
	s >> position_ >> yaw_ >> roll_ >> pitch_ >> optimized_;
}

//-------------------------------------------------------------------------------------


}
