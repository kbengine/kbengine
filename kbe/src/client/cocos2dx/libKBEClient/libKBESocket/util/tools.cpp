#include "tools.h"
USING_NS_CC;

NS_GC_BEGIN

void getLocalFileLength(const char* pszFileName, long long* pSize)
{
	CCAssert(pszFileName != NULL && pSize != NULL, "Invalid parameters.");
	*pSize = 0;
	do
	{
		// read the file from hardware
		std::string fullPath = CCFileUtils::sharedFileUtils()->fullPathForFilename(pszFileName);
		FILE *fp = fopen(fullPath.c_str(), "rb");
		CC_BREAK_IF(!fp);

		fseek(fp,0,SEEK_END);
		*pSize = ftell(fp);
		fseek(fp,0,SEEK_SET);
		fclose(fp);
	} while (0);
}

float heronsformula(float x1,float y1,float x2,float y2,float x3,float y3)
{
	float a=sqrt(pow(x1-x2,2)+pow(y1-y2,2));
	float b=sqrt(pow(x2-x3,2)+pow(y2-y3,2));
	float c=sqrt(pow(x3-x1,2)+pow(y3-y1,2));
	float s=(a+b+c)/2;

	return sqrt(s*(s-a)*(s-b)*(s-c));
}

bool triangleContainPoint(float x1,float y1,float x2,float y2,float x3,float y3,float px,float py)
{
	float s1=heronsformula(x1,y1,x2,y2,px,py);
	float s2=heronsformula(x2,y2,x3,y3,px,py);
	float s3=heronsformula(x3,y3,x1,y1,px,py);
	float s=heronsformula(x1,y1,x2,y2,x3,y3);

	return fabs(s-(s1+s2+s3))<0.001f;
}

// #if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
// //字符转换，使cocos2d-x在win32平台支持中文显示
// int GBKToUTF8(std::string &gbkStr,const char* toCode,const char* formCode)
// {
// 	iconv_t iconvH;
// 	iconvH = iconv_open(formCode,toCode);
// 	if(iconvH == 0)
// 	{
// 		return -1;
// 	}
// 
// 	const char* strChar = gbkStr.c_str();
// 	const char** pin = &strChar;
// 
// 	size_t strLength = gbkStr.length();
// 	char* outbuf = (char*)malloc(strLength*4);
// 	char* pBuff = outbuf;
// 	memset(outbuf,0,strLength*4);
// 	size_t outLength = strLength*4;
// 	if(-1 == iconv(iconvH,pin,&strLength,&outbuf,&outLength))
// 	{
// 		iconv_close(iconvH);
// 		return -1;
// 	}
// 
// 	gbkStr = pBuff;
// 	iconv_close(iconvH);
// 	return 0;
// }
// #endif

std::vector<tLinkData> getLinkDataFromStr(std::string str, char frontSplit, char middleSplit, char backSplit)
{
	int startPos = 0;
	int endPos = 0;
	int lastStartPos = 0;
	int lastEndPos = 0;
	std::vector<tLinkData> tldList;
	std::string value;
	do 
	{
		startPos = str.find(frontSplit);
		endPos = str.find(backSplit);
		if (startPos==str.npos || endPos==str.npos || endPos<startPos)
		{
			break;
		}
		value = str.substr(startPos+1, endPos-startPos-1);

		tLinkData tld;
		int size = value.size();
		int pos;
		int index = 0;
		for (int i=0;i<size;i++, index++)
		{
			pos = value.find(middleSplit, i);
			if (pos<size)
			{
				switch (index)
				{
				case 0:
					tld.showStr = value.substr(i,pos-i);
					break;
				case 1:
					tld.linkStr = value.substr(i,pos-i);
					break;
				case 2:
					tld.value = atoi((value.substr(i)).c_str());
					break;
				}
				i = pos;
			}
			if (pos==value.npos || index==2)
			{
				break;
			}
		}
		lastStartPos += startPos;
		lastEndPos += endPos;
		tld.startPos = lastStartPos;
		tld.endPos = lastEndPos;
		if (index==2)
		{
			tldList.push_back(tld);
		}
		str = str.substr(endPos+1);
	} while (1);
	return tldList;
}
tColorStrData getColorStrDataFromStr(std::string str, char frontSplit, char middleSplit, char backSplit)
{
	tColorStrData csd;
	csd.startPos = str.find(frontSplit);
	csd.endPos = str.find(backSplit);
	int wellPos = str.find(middleSplit);
	if (csd.startPos==str.npos || csd.endPos==str.npos || wellPos==str.npos || csd.endPos<(wellPos+6) || wellPos<csd.startPos)
	{
		csd.showStr = "";
		return csd;
	}
	csd.showStr = str.substr(csd.startPos+1, wellPos-csd.startPos-1);
	std::string value = str.substr(wellPos+1, 2);
	sscanf(value.c_str(), "%x", &csd.color.r);
	value = str.substr(wellPos+3, 2);
	sscanf(value.c_str(), "%x", &csd.color.g);
	value = str.substr(wellPos+5, 2);
	sscanf(value.c_str(), "%x", &csd.color.b);

	return csd;
}
cocos2d::CCRect getRect2(cocos2d::CCNode * pNode)
{
	cocos2d::CCRect rc;
	rc.origin = pNode->getPosition();
	rc.size = pNode->getContentSize();
	rc.origin.x -= rc.size.width / 2;
	rc.origin.y -= rc.size.height / 2;
	return rc;
}

cocos2d::CCRect getRect(cocos2d::CCNode * pNode)
{
	cocos2d::CCRect rc;
	rc.origin = pNode->getPosition();
	rc.size = pNode->getContentSize();
	return rc;
}

NS_GC_END