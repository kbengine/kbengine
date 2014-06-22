#pragma once

#include "KBEClientcore.h"

typedef struct KV//一个键值对
{
	int key;
	int intValue;
	std::string stringValue;
	int64 int64Value;
	std::vector<int> intListValue;
	std::vector<std::string> stringListValue;
	std::vector<int64> int64ListValue;
	void decode(Message& msg)
	{
		msg >> key;
		if (key>=0 && key<=999)
		{
			msg >> intValue;
		}
		else if (key>=1000 && key<=1999)
		{
			msg >> stringValue;
		}
		else if (key>=2000 && key<=2999)
		{
			msg >> int64Value;
		}
		else if (key>=3000 && key<=3999)
		{
			int16 len = 0;
			msg >> len;
			for (int i=0;i<len;i++)
			{
				int value;
				msg >> value;
				intListValue.push_back(value);
			}
		}
		else if (key>=4000 && key<=4999)
		{
			int16 len = 0;
			msg >> len;
			for (int i=0;i<len;i++)
			{
				std::string value;
				msg >> value;
				stringListValue.push_back(value);
			}
		}
	}
}tKV;

class MessageUtil
{
public:
	static MessageUtil& getInstance()
	{
		static MessageUtil instance;
		return instance;
	}

	void decode_k_list(Message &msg, \
		std::map<int, int> &intDic, \
		std::map<int, std::string> &stringDic,\
		std::map<int, int64> &int64Dic, \
		std::map<int, std::vector<int> > &intListDic, \
		std::map<int, std::vector<std::string> > &stringListDic, \
		std::map<int, std::vector<int64> > &int64ListDic);
	void decode_k(Message& msg, \
		int& intValue, \
		std::string& stringValue,\
		int64& int64Value, \
		std::vector<int>& intListValue, \
		std::vector<std::string>& stringListValue, \
		std::vector<int64>& int64ListValue);

private:
	MessageUtil();
	MessageUtil(const MessageUtil&);
	MessageUtil& operator=(const MessageUtil&);
	~MessageUtil();
};

