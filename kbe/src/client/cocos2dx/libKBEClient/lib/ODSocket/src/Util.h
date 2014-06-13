/*
 *  Util.h
 *  test
 *
 *  Created by takehu on 11-7-20.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _UTIL_H_
#define _UTIL_H_
#include "cocos2d.h"

#include <vector>
#include <string>
#include <cmath>
//algorithm头文件包含make_heap,pop_heap等堆操作（android）by hubin.
#include <algorithm>
//增加sleep方法的头文件，不然android版本着不到对应的函数，iphone的是默认找的苹果系统里的同样方法 by hubin
#include <unistd.h>
#include "pthread.h"


typedef signed char byte;
typedef std::string String;
typedef bool boolean;
class ByteBuffer; 


using namespace std;

#define  Float_MAX_VALUE  3.4028235E+38F
#define  Float_MIN_VALUE  1.4E-45F;

// Helpers to safely delete objects and arrays
#define SQ_SAFE_DELETE(x)       {if(x){ delete x; x = 0; }}
#define SQ_SAFE_DELETE_ARRAY(x) {if(x){ delete[] x; x = 0; }}
#define SQ_SAFE_DELETE_VEC(x) {for(int i = 0; i < x->size();i++){delete (x+i);}delete[] x;}
/**
 * @Fields ERROR_INS : 误差修正
 */
static const double ERROR_INS = 0.005;
extern unsigned char* readFile(const char *filename);
extern string byteToHexStr(unsigned char *byte_arr, int arr_len);
extern double distance(double x1, double y1, double x2, double y2);
extern unsigned long msNextPOT(unsigned long x);

#define SAFE_DELETE_ELEMENT( ptr ) if (ptr != NULL) {delete ptr; ptr = NULL;}
#define SAFE_DELETE_ARRAY( ptr )if (ptr != NULL){delete[] ptr;ptr = NULL;}

template<typename _RandomAccessIterator>
inline void safe_delete_vector(_RandomAccessIterator __first, _RandomAccessIterator __last)
{

	for (_RandomAccessIterator it = __first;it!= __last ; ++it) {
		if( (*it)!=NULL){
			delete *it;
			*it = NULL;
		}
	}
}	


//CString StringConvertToUnicodeBytesCodes(const wchar_t* lpszNeedConvert);

template <class Class, typename TT>
inline bool instanceof(TT const &object)
{
    return dynamic_cast<Class const *>(&object);
	//return false;
}

const char* fullpathFromRelatePath(const char* relatePath);


/**
 return : 是否存档成功
 **/
bool writeSaveData(const char* relatepath,char* buf,int len);
/**
 读取存档
 */
ByteBuffer* readSaveData(const char* relatepath);

//读取assets包裹资源,isfullPath是否是全目录,如果是资源目录中的资源就用false，如果不是就用true
ByteBuffer* getFileData(const char* pszfullFilepath, bool isfullPath);

class MyLock{
	pthread_mutex_t* mutex_t;
public:
	MyLock(pthread_mutex_t* _mutex_t);
	~MyLock();
	
};



String createRandString(int len,boolean filter);



#endif
