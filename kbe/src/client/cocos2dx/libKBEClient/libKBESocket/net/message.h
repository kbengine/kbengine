/*
 * Message.h
 *
 *  Created on: 2011-9-19
 *      Author: lyh
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "../util/byte_buffer.h"

NS_GC_BEGIN

class Message
{
public:
	Message();
	virtual ~Message();
	uint32 getOpcode();
	void setOpcode(uint32 opcode);//设置消息头
	ByteBuffer& getBuffer();
	void setBuffer(ByteBuffer& buffer);
	Message & operator<<(bool value);
	Message & operator<<(uint8 value);
	Message & operator<<(uint16 value);
	Message & operator<<(uint32 value);
	Message & operator<<(uint64 value);
	Message & operator<<(int8 value);
	Message & operator<<(int16 value);
	Message & operator<<(int32 value);
	Message & operator<<(int64 value);
	Message & operator<<(float value);
	Message & operator<<(double value);
	Message & operator<<(const char* str);
	Message & operator>>(bool & value);
	Message & operator>>(uint8 & value);
	Message & operator>>(uint16 & value);
	Message & operator>>(uint32 & value);
	Message & operator>>(uint64 & value);
	Message & operator>>(int8 & value);
	Message & operator>>(int16 & value);
	Message & operator>>(int32 & value);
	Message & operator>>(int64 & value);
	Message & operator>>(float & value);
	Message & operator>>(double & value);
	Message & operator>>(std::string & value);
private:
	ByteBuffer buffer;
	uint32 opcode;
};

NS_GC_END

#endif /* MESSAGE_H_ */
