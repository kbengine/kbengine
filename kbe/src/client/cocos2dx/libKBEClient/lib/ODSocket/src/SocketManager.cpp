//
//  SocketManager.cpp
//  client1
//
//  Created by guoyahui on 13-5-28.
//
//

#include "SocketManager.h"


static SocketManager* instance = NULL;
SocketManager::SocketManager()
{
    

}

SocketManager* SocketManager::getInstance()
{
    if(instance == NULL)
    {
        instance = new SocketManager();
        
    }
    
    return instance;
}

void SocketManager::startSocket()
{
    char IPStr[64]={0};
    hostent * host_entry=gethostbyname("jiumiaoshanyou3.1251001050.lbs.twsapp.com");
    
    if(host_entry !=0){
        
        sprintf(IPStr, "%d.%d.%d.%d",(host_entry->h_addr_list[0][0]&0x00ff),
                
                (host_entry->h_addr_list[0][1]&0x00ff),
                
                (host_entry->h_addr_list[0][2]&0x00ff),
                
                (host_entry->h_addr_list[0][3]&0x00ff));
        
    }
    CCLog("%s",IPStr);
    CCLog("--------------------------");
        _socket = new SocketClient(IPStr,31009,1,1,NULL);
    
//     _socket = new SocketClient("192.168.1.210",11009,1,1,NULL);
//    _socket = new SocketClient("183.60.243.195",31009,1,1,NULL);

    _socket->start();
}

void SocketManager::sendMessage(const char* data,int commandId)
{
    
//    if (_socket->m_iState != SocketClient_OK)
//    {
//        _socket->stop(true);
//        startSocket();
//    }
    
    Message *msg=_socket->constructMessage(data, commandId);
    _socket->sendMessage_(msg, false);
}




