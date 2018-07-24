// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine { 

INLINE script::Script* TelnetServer::pScript() const{ return pScript_; }
INLINE void TelnetServer::pScript(script::Script* p){ pScript_ = p; }
INLINE std::string TelnetServer::passwd(){ return passwd_; }
INLINE int TelnetServer::deflayer(){ return deflayer_; }
INLINE Network::NetworkInterface* TelnetServer::pNetworkInterface() const{ return pNetworkInterface_; }
INLINE uint32 TelnetServer::port(){ return port_; }

}
