/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#include "debug_option.h"

namespace KBEngine { 
	
namespace Network
{

bool g_packetAlwaysContainLength = false;

uint8 g_trace_packet = 0;
bool g_trace_encrypted_packet = true;
bool g_trace_packet_use_logfile = false;
std::vector<std::string> g_trace_packet_disables;

}

bool g_debugEntity = false;
int8 g_appPublish = 1;

}
