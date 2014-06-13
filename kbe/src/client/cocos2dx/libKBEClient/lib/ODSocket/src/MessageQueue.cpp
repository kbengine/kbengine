//
//  MessageQueue.cpp
//  client1
//
//  Created by 柏占肖 on 13-3-9.
//
//

#include "MessageQueue.h"

MessageQueue::MessageQueue()
{
    
    
}
MessageQueue::~MessageQueue()
{
    
}

MessageQueue* message;

MessageQueue* MessageQueue::getMessage()
{
    if(!message)
    {
        message = new MessageQueue();
    }
    return message;
}
void MessageQueue::initSocket(char * ip,int port){
    sclient = new SocketClient(ip,port,1,1,NULL);
}
//接受数据    pop
//Message* MessageQueue::getMessagedata(int commandId)
//{
//    
//    return sclient->popReceivedMessage();
//}
//发送
void MessageQueue::sendMessage(Message * data,bool flag){
    sclient->sendMessage_(data, flag);
}
//不pop
//Message * MessageQueue::pickReceivedMessage(){
//    return sclient->pickReceivedMessage();
//}
void MessageQueue::start(){
    return sclient->start();
}