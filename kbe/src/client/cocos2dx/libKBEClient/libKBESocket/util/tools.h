#ifndef _TOOLS_H_
#define  _TOOLS_H_

#include "cocos2d.h"
#include "KBEClientCoreMacros.h"

NS_GC_BEGIN
// 
// #if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
// #include "iconv.h"
// #endif

//获取一个文件的长度（[int]pszFileName：文件名，[out]size：文件长度）
void getLocalFileLength(const char* pszFileName, long long* pSize);

float heronsformula(float x1,float y1,float x2,float y2,float x3,float y3);

bool triangleContainPoint(float x1,float y1,float x2,float y2,float x3,float y3,float px,float py);

// #if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
// //字符转换，使cocos2d-x在win32平台支持中文显示
// int GBKToUTF8(std::string &gbkStr,const char* toCode,const char* formCode);
// #endif

class tLinkData//超链接数据
{
public:
	std::string showStr;//显示文字
	std::string linkStr;//连接内容
	int value;//一些值
	int startPos;//超链接所在字串中开始位置
	int endPos;//超链接所在字串中结束位置
};
class tColorStrData//颜色数据
{
public:
	std::string showStr;//显示文字
	cocos2d::ccColor3B color;//颜色值
	int startPos;//超链接所在字串中开始位置
	int endPos;//超链接所在字串中结束位置
};

std::vector<tLinkData> getLinkDataFromStr(std::string str, char frontSplit='(', char middleSplit=',', char backSplit=')');//从str中找出link数据
tColorStrData getColorStrDataFromStr(std::string str, char frontSplit='[', char middleSplit='#', char backSplit=']');//从str中找出颜色数据

cocos2d::CCRect getRect2(cocos2d::CCNode* pNode);
cocos2d::CCRect getRect(cocos2d::CCNode* pNode);

NS_GC_END

#endif //_TOOLS_H_