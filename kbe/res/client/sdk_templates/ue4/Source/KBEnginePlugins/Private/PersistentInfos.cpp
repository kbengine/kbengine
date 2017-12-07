
#include "PersistentInfos.h"
#include "KBEngine.h"
#include "KBDebug.h"

PersistentInfos::PersistentInfos():
	_persistentDataPath(TEXT("")),
	_isGood(false),
	_digest(TEXT(""))
{
}

PersistentInfos::PersistentInfos(const FString& path) :
	_persistentDataPath(path),
	_isGood(false),
	_digest(TEXT(""))
{
	installEvents();
	_isGood = loadAll();
}

PersistentInfos::~PersistentInfos()
{
}

void PersistentInfos::installEvents()
{
}

bool PersistentInfos::loadAll()
{
	return true;
}

void PersistentInfos::onImportClientMessages(const FString& currserver, const TArray<uint8>& stream)
{
}

void PersistentInfos::onImportServerErrorsDescr(const TArray<uint8>& stream)
{
}

void PersistentInfos::onImportClientEntityDef(const TArray<uint8>& stream)
{
}

void PersistentInfos::onVersionNotMatch(const FString& verInfo, const FString& serVerInfo)
{
	clearMessageFiles();
}

void PersistentInfos::onScriptVersionNotMatch(const FString& verInfo, const FString& serVerInfo)
{
	clearMessageFiles();
}

void PersistentInfos::onServerDigest(const FString& currserver, const FString& serverProtocolMD5, const FString& serverEntitydefMD5)
{
	// 我们不需要检查网关的协议， 因为登录loginapp时如果协议有问题已经删除了旧的协议
	if (currserver == TEXT("baseapp"))
	{
		return;
	}
}

void PersistentInfos::clearMessageFiles()
{
	KBEngineApp::getSingleton().resetMessages();
}
