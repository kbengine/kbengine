#include "MessageUtil.h"
//#include "GameDefine.h"

MessageUtil::MessageUtil(void)
{
}
MessageUtil::~MessageUtil(void)
{
}
void MessageUtil::decode_k_list(Message &msg, \
	std::map<int, int> &intDic, \
	std::map<int, std::string> &stringDic,\
	std::map<int, int64> &int64Dic, \
	std::map<int, std::vector<int> > &intListDic, \
	std::map<int, std::vector<std::string> > &stringListDic, \
	std::map<int, std::vector<int64> > &int64ListDic)
{
	int16 listLen = 0;
	msg >> listLen;
	for (int i=0;i<listLen;i++)
	{
		int key = 0;
		msg >> key;
		if (key>=0 && key<=999)
		{
			int value = 0;
			msg >> value;
			intDic[key] = value;
		}
		else if (key>=1000 && key<=1999)
		{
			std::string value = "";
			msg >> value;
			stringDic[key]=value;
		}
		else if (key>=2000 && key<=2999)
		{
			int64 value = 0;
			msg >> value;
			int64Dic[key]=value;
		}
		else if (key>=3000 && key<=3999)
		{
			switch (key)
			{
			case 3999:
			//case ROLECONSTCONFIG_ROLE_ATTR_INSTANCE_TITLE_ICON_ID:
			//case ROLECONSTCONFIG_ROLE_ATTR_BODY_BUFFER:
			//case ROLECONSTCONFIG_ROLE_ATTR_BODY_BUFF_LEVEL:
			//case ROLECONSTCONFIG_ROLE_ATTR_PATH_X:
			//case ROLECONSTCONFIG_ROLE_ATTR_PATH_Y:
			//case ROLECONSTCONFIG_ROLE_ATTR_HONOUR:
				{
					std::vector<int> list;
					int16 len = 0;
					msg >> len;
					for (int i=0;i<len;i++)
					{
						int value;
						msg >> value;
						list.push_back(value);
					}
					intListDic[key]=list;
					break;
				}
			default:
				{
					int64 value = 0;
					msg >> value;
					int64Dic[key]=value;
				}
			}
		}
		else if (key>=4000 && key<=4999)
		{
			std::vector<std::string> list;
			int16 len = 0;
			msg >> len;
			for (int i=0;i<len;i++)
			{
				std::string value;
				msg >> value;
				list.push_back(value);
			}
			stringListDic[key]=list;
		}
	}
}