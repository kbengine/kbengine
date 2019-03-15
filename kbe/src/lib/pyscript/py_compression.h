// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SCRIPT_COMPRESSION_H
#define KBE_SCRIPT_COMPRESSION_H

#include "common/common.h"
#include "scriptobject.h"

namespace KBEngine{ namespace script{

class PyCompression
{						
public:	
	static bool zipCompressDirectory(const std::string& sourceDir, const std::string& outfile);
	static bool tarCompressDirectory(const std::string& sourceDir, const std::string& outfile);

	/** ��ʼ�� */
	static bool initialize(void);
	static void finalise(void);

private:
	static bool	isInit;										// �Ƿ��Ѿ�����ʼ��
} ;

}
}
#endif // KBE_SCRIPT_COMPRESSION_H
