/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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


namespace KBEngine { 

INLINE script::Script* TelnetServer::pScript() const{ return pScript_; }
INLINE void TelnetServer::pScript(script::Script* p){ pScript_ = p; }
INLINE std::string TelnetServer::passwd(){ return passwd_; }
INLINE int TelnetServer::deflayer(){ return deflayer_; }
INLINE Network::NetworkInterface* TelnetServer::pNetworkInterface() const{ return pNetworkInterface_; }
INLINE uint32 TelnetServer::port(){ return port_; }

}
