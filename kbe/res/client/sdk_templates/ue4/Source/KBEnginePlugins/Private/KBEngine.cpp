
#include "KBEngine.h"
#include "KBEngineArgs.h"
#include "Entity.h"
#include "NetworkInterface.h"
#include "Bundle.h"
#include "MemoryStream.h"
#include "PersistentInfos.h"
#include "DataTypes.h"
#include "ScriptModule.h"
#include "Property.h"
#include "Method.h"
#include "EntityCall.h"
#include "Regex.h"
#include "KBDebug.h"

ServerErrorDescrs KBEngineApp::serverErrs_;

bool KBEngineApp::loadingLocalMessages_ = false;


bool KBEngineApp::loginappMessageImported_ = false;
bool KBEngineApp::baseappMessageImported_ = false;
bool KBEngineApp::entitydefImported_ = false;
bool KBEngineApp::isImportServerErrorsDescr_ = false;

KBEngineApp::KBEngineApp() :
	pArgs_(NULL),
	pNetworkInterface_(NULL),
	username_(TEXT("")),
	password_(TEXT("")),
	baseappIP_(TEXT("")),
	baseappPort_(0),
	currserver_(TEXT("")),
	currstate_(TEXT("")),
	serverdatas_(),
	clientdatas_(),
	encryptedKey_(),
	serverVersion_(TEXT("")),
	clientVersion_(TEXT("")),
	serverScriptVersion_(TEXT("")),
	clientScriptVersion_(TEXT("")),
	serverProtocolMD5_(TEXT("@{KBE_SERVER_PROTO_MD5}")),
	serverEntitydefMD5_(TEXT("@{KBE_SERVER_ENTITYDEF_MD5}")),
	persistentInfos_(NULL),
	entity_uuid_(0),
	entity_id_(0),
	entity_type_(TEXT("")),
	useAliasEntityID_(@{KBE_USE_ALIAS_ENTITYID}),
	controlledEntities_(),
	entityServerPos_(),
	spacedatas_(),
	entities_(),
	entityIDAliasIDList_(),
	bufferedCreateEntityMessages_(),
	lastTickTime_(0.0),
	lastTickCBTime_(0.0),
	lastUpdateToServerTime_(0.0),
	spaceID_(0),
	spaceResPath_(TEXT("")),
	isLoadedGeometry_(false),
	component_(TEXT("client"))
{
	INFO_MSG("KBEngineApp::KBEngineApp(): hello!");
}

KBEngineApp::KBEngineApp(KBEngineArgs* pArgs):
	pArgs_(NULL),
	pNetworkInterface_(NULL),
	username_(TEXT("")),
	password_(TEXT("")),
	baseappIP_(TEXT("")),
	baseappPort_(0),
	currserver_(TEXT("")),
	currstate_(TEXT("")),
	serverdatas_(),
	clientdatas_(),
	encryptedKey_(),
	serverVersion_(TEXT("")),
	clientVersion_(TEXT("")),
	serverScriptVersion_(TEXT("")),
	clientScriptVersion_(TEXT("")),
	serverProtocolMD5_(TEXT("@{KBE_SERVER_PROTO_MD5}")),
	serverEntitydefMD5_(TEXT("@{KBE_SERVER_ENTITYDEF_MD5}")),
	persistentInfos_(NULL),
	entity_uuid_(0),
	entity_id_(0),
	entity_type_(TEXT("")),
	useAliasEntityID_(@{KBE_USE_ALIAS_ENTITYID}),
	controlledEntities_(),
	entityServerPos_(),
	spacedatas_(),
	entities_(),
	entityIDAliasIDList_(),
	bufferedCreateEntityMessages_(),
	lastTickTime_(0.0),
	lastTickCBTime_(0.0),
	lastUpdateToServerTime_(0.0),
	spaceID_(0),
	spaceResPath_(TEXT("")),
	isLoadedGeometry_(false),
	component_(TEXT("client"))
{
	INFO_MSG("KBEngineApp::KBEngineApp(): hello!");
	initialize(pArgs);
}

KBEngineApp::~KBEngineApp()
{
	destroy();
	INFO_MSG("KBEngineApp::~KBEngineApp(): destructed!");
}

KBEngineApp& KBEngineApp::getSingleton() 
{
	static KBEngineApp* pKBEngineApp = NULL;

	if (!pKBEngineApp)
		pKBEngineApp = new KBEngineApp();

	return *pKBEngineApp;
}

bool KBEngineApp::initialize(KBEngineArgs* pArgs)
{
	if (isInitialized())
		return false;

	EntityDef::initialize();

	// 允许持久化KBE(例如:协议，entitydef等)
	if (pArgs->persistentDataPath != TEXT(""))
	{
		KBE_SAFE_RELEASE(persistentInfos_);
		persistentInfos_ = new PersistentInfos(pArgs->persistentDataPath);
	}

	// 注册事件
	installEvents();

	pArgs_ = pArgs;
	reset();
	return true;
}

void KBEngineApp::installEvents()
{
	KBENGINE_REGISTER_EVENT_OVERRIDE_FUNC("login", "login", [this](const UKBEventData* pEventData)
	{
		const UKBEventData_login& data = static_cast<const UKBEventData_login&>(*pEventData);
		login(data.username, data.password, data.datas);
	});

	KBENGINE_REGISTER_EVENT_OVERRIDE_FUNC("createAccount", "createAccount", [this](const UKBEventData* pEventData)
	{
		const UKBEventData_createAccount& data = static_cast<const UKBEventData_createAccount&>(*pEventData);
		createAccount(data.username, data.password, data.datas);
	});

	KBENGINE_REGISTER_EVENT_OVERRIDE_FUNC("reloginBaseapp", "reloginBaseapp", [this](const UKBEventData* pEventData)
	{
		reloginBaseapp();
	});

	KBENGINE_REGISTER_EVENT_OVERRIDE_FUNC("resetPassword", "resetPassword", [this](const UKBEventData* pEventData)
	{
		const UKBEventData_resetPassword& data = static_cast<const UKBEventData_resetPassword&>(*pEventData);
		resetPassword(data.username);
	});

	KBENGINE_REGISTER_EVENT_OVERRIDE_FUNC("bindAccountEmail", "bindAccountEmail", [this](const UKBEventData* pEventData)
	{
		const UKBEventData_bindAccountEmail& data = static_cast<const UKBEventData_bindAccountEmail&>(*pEventData);
		bindAccountEmail(data.email);
	});

	KBENGINE_REGISTER_EVENT_OVERRIDE_FUNC("newPassword", "newPassword", [this](const UKBEventData* pEventData)
	{
		const UKBEventData_newPassword& data = static_cast<const UKBEventData_newPassword&>(*pEventData);
		newPassword(data.old_password, data.new_password);
	});

	// 内部事件
	KBENGINE_REGISTER_EVENT_OVERRIDE_FUNC("_closeNetwork", "_closeNetwork", [this](const UKBEventData* pEventData)
	{
		_closeNetwork();
	});
}

void KBEngineApp::destroy()
{
	reset();
	KBENGINE_DEREGISTER_ALL_EVENT();
	resetMessages();

	KBE_SAFE_RELEASE(pArgs_);
	KBE_SAFE_RELEASE(pNetworkInterface_);
	KBE_SAFE_RELEASE(persistentInfos_);
}

void KBEngineApp::resetMessages()
{
	loadingLocalMessages_ = false;
	loginappMessageImported_ = false;
	baseappMessageImported_ = false;
	entitydefImported_ = false;
	isImportServerErrorsDescr_ = false;

	serverErrs_.Clear();

	Messages::getSingleton().clear();
	EntityDef::clear();
	Entity::clear();

	INFO_MSG("KBEngineApp::resetMessages(): done!");
}

void KBEngineApp::reset()
{
	KBEvent::clearFiredEvents();

	clearEntities(true);

	currserver_ = TEXT("");
	currstate_ = TEXT("");

	serverdatas_.Empty();

	serverVersion_ = TEXT("");
	clientVersion_ = TEXT("@{KBE_VERSION}");
	serverScriptVersion_ = TEXT("");
	clientScriptVersion_ = TEXT("@{KBE_SCRIPT_VERSION}");

	entity_uuid_ = 0;
	entity_id_ = 0;
	entity_type_ = TEXT("");

	entityIDAliasIDList_.Empty();
	bufferedCreateEntityMessages_.Empty();

	lastTickTime_ = getTimeSeconds();
	lastTickCBTime_ = getTimeSeconds();
	lastUpdateToServerTime_ = getTimeSeconds();

	spacedatas_.Empty();

	spaceID_ = 0;
	spaceResPath_ = TEXT("");
	isLoadedGeometry_ = false;
	
	initNetwork();
}

bool KBEngineApp::initNetwork()
{
	if (pNetworkInterface_)
		delete pNetworkInterface_;

	pNetworkInterface_ = new NetworkInterface();
	return true;
}

void KBEngineApp::_closeNetwork()
{
	if (pNetworkInterface_)
		pNetworkInterface_->close();
}

bool KBEngineApp::validEmail(const FString& strEmail)
{
	const FRegexPattern spattern(FString(TEXT("[a-z0-9._%+-]+@[a-z0-9.-]+\\.[a-z]{2,4}")));

	FRegexMatcher fMatcher(spattern, strEmail);

	if (fMatcher.FindNext()) {
		return true;
	}

	return false;
}

void KBEngineApp::process()
{
	// 处理网络
	if (pNetworkInterface_)
		pNetworkInterface_->process();

	// 处理外层抛入的事件
	KBEvent::processInEvents();

	// 向服务端发送心跳以及同步角色信息到服务端
	sendTick();
}

void KBEngineApp::sendTick()
{
	if (!pNetworkInterface_ || !pNetworkInterface_->valid())
		return;

	if (!loginappMessageImported_ && !baseappMessageImported_)
		return;

	double span = getTimeSeconds() - lastTickTime_;

	// 更新玩家的位置与朝向到服务端
	updatePlayerToServer();

	if (span > pArgs_->serverHeartbeatTick)
	{
		span = lastTickCBTime_ - lastTickTime_;

		// 如果心跳回调接收时间小于心跳发送时间，说明没有收到回调
		// 此时应该通知客户端掉线了
		if (span < 0)
		{
			SCREEN_ERROR_MSG("KBEngineApp::sendTick(): Receive appTick timeout!");
			pNetworkInterface_->close();
			return;
		}

		Message** Loginapp_onClientActiveTickMsgFind = Messages::getSingleton().messages.Find("Loginapp_onClientActiveTick");
		Message** Baseapp_onClientActiveTickMsgFind = Messages::getSingleton().messages.Find("Baseapp_onClientActiveTick");

		if (currserver_ == TEXT("loginapp"))
		{
			if (Loginapp_onClientActiveTickMsgFind)
			{
				Bundle* pBundle = Bundle::createObject();
				pBundle->newMessage(*Loginapp_onClientActiveTickMsgFind);
				pBundle->send(pNetworkInterface_);
			}
		}
		else
		{
			if (Baseapp_onClientActiveTickMsgFind)
			{
				Bundle* pBundle = Bundle::createObject();
				pBundle->newMessage(*Baseapp_onClientActiveTickMsgFind);
				pBundle->send(pNetworkInterface_);
			}
		}

		lastTickTime_ = getTimeSeconds();
	}
}

Entity* KBEngineApp::player()
{
	return findEntity(entity_id_);
}

Entity* KBEngineApp::findEntity(int32 entityID)
{
	Entity** pEntity = entities_.Find(entityID);
	if (pEntity == nullptr)
		return NULL;

	return *pEntity;
}

FString KBEngineApp::serverErr(uint16 id)
{
	return serverErrs_.ServerErrStr(id);
}

void KBEngineApp::updatePlayerToServer()
{
	if (!pArgs_->syncPlayer || spaceID_ == 0)
		return;

	double tnow = getTimeSeconds();
	double span = tnow - lastUpdateToServerTime_;

	if (span < 0.1)
		return;

	Entity* pPlayerEntity = player();
	if (!pPlayerEntity || !pPlayerEntity->inWorld()  || pPlayerEntity->isControlled())
		return;

	lastUpdateToServerTime_ = tnow;
	const FVector& position = pPlayerEntity->position;
	const FVector& direction = pPlayerEntity->direction;

	bool posHasChanged = (pPlayerEntity->entityLastLocalPos - position).Size() > 0.001f;
	bool dirHasChanged = (pPlayerEntity->entityLastLocalDir - direction).Size() > 0.001f;

	if (posHasChanged || dirHasChanged)
	{
		pPlayerEntity->entityLastLocalPos = position;
		pPlayerEntity->entityLastLocalDir = direction;

		Bundle* pBundle = Bundle::createObject();
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Baseapp_onUpdateDataFromClient"]));
		(*pBundle) << position.X;
		(*pBundle) << position.Y;
		(*pBundle) << position.Z;

		(*pBundle) << direction.X;
		(*pBundle) << direction.Y;
		(*pBundle) << direction.Z;

		(*pBundle) << (uint8)pPlayerEntity->isOnGround();
		(*pBundle) << spaceID_;

		pBundle->send(pNetworkInterface_);
	}

	// 开始同步所有被控制了的entity的位置
	for(auto& item : controlledEntities_)
	{
		Entity* pEntity = item;
		const FVector& e_position = pEntity->position;
		const FVector& e_direction = pEntity->direction;

		posHasChanged = (pEntity->entityLastLocalPos - e_position).Size() > 0.001f;
		dirHasChanged = (pEntity->entityLastLocalDir - e_direction).Size() > 0.001f;

		if (posHasChanged || dirHasChanged)
		{
			pEntity->entityLastLocalPos = e_position;
			pEntity->entityLastLocalDir = e_direction;

			Bundle* pBundle = Bundle::createObject();
			pBundle->newMessage(Messages::getSingleton().messages[TEXT("Baseapp_onUpdateDataFromClientForControlledEntity"]));
			(*pBundle) << pEntity->id();
			(*pBundle) << e_position.X;
			(*pBundle) << e_position.Y;
			(*pBundle) << e_position.Z;

			(*pBundle) << e_direction.X;
			(*pBundle) << e_direction.Y;
			(*pBundle) << e_direction.Z;

			(*pBundle) << (uint8)pEntity->isOnGround();
			(*pBundle) << spaceID_;

			pBundle->send(pNetworkInterface_);
		}
	}

}

void KBEngineApp::Client_onAppActiveTickCB()
{
	lastTickCBTime_ = getTimeSeconds();
}

void KBEngineApp::hello()
{
	Bundle* pBundle = Bundle::createObject();
	if (currserver_ == TEXT("loginapp"))
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Loginapp_hello")]);
	else
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Baseapp_hello")]);

	(*pBundle) << clientVersion_;
	(*pBundle) << clientScriptVersion_;
	pBundle->appendBlob(encryptedKey_);
	pBundle->send(pNetworkInterface_);
}

void KBEngineApp::Client_onHelloCB(MemoryStream& stream)
{
	stream >> serverVersion_;
	stream >> serverScriptVersion_;
	stream >> serverProtocolMD5_;
	stream >> serverEntitydefMD5_;

	int32 ctype;
	stream >> ctype;

	INFO_MSG("KBEngineApp::Client_onHelloCB(): verInfo(%s), scriptVersion(%s), srvProtocolMD5(%s), srvEntitydefMD5(%s), ctype(%d)!", 
		*serverVersion_, *serverScriptVersion_, *serverProtocolMD5_, *serverEntitydefMD5_, ctype);

	onServerDigest();

	if (currserver_ == TEXT("baseapp"))
	{
		onLogin_baseapp();
	}
	else
	{
		onLogin_loginapp();
	}
}

void KBEngineApp::Client_onVersionNotMatch(MemoryStream& stream)
{
	stream >> serverVersion_;

	ERROR_MSG("KBEngineApp::Client_onVersionNotMatch(): verInfo=%s(server: %s)", *clientVersion_, *serverVersion_);

	UKBEventData_onVersionNotMatch* pEventData = NewObject<UKBEventData_onVersionNotMatch>();
	pEventData->clientVersion = clientVersion_;
	pEventData->serverVersion = serverVersion_;
	KBENGINE_EVENT_FIRE("onVersionNotMatch", pEventData);

	if (persistentInfos_)
		persistentInfos_->onVersionNotMatch(clientVersion_, serverVersion_);
}

void KBEngineApp::Client_onScriptVersionNotMatch(MemoryStream& stream)
{
	stream >> serverScriptVersion_;

	ERROR_MSG("KBEngineApp::Client_onScriptVersionNotMatch(): verInfo=%s(server: %s)", *clientScriptVersion_, *serverScriptVersion_);

	UKBEventData_onScriptVersionNotMatch* pEventData = NewObject<UKBEventData_onScriptVersionNotMatch>();
	pEventData->clientScriptVersion = clientScriptVersion_;
	pEventData->serverScriptVersion = serverScriptVersion_;
	KBENGINE_EVENT_FIRE("onScriptVersionNotMatch", pEventData);

	if (persistentInfos_)
		persistentInfos_->onScriptVersionNotMatch(clientScriptVersion_, serverScriptVersion_);
}

void KBEngineApp::Client_onKicked(uint16 failedcode)
{
	DEBUG_MSG("KBEngineApp::Client_onKicked(): failedcode=%d, %s", failedcode, *serverErr(failedcode));

	UKBEventData_onKicked* pEventData = NewObject<UKBEventData_onKicked>();
	pEventData->failedcode = failedcode;
	pEventData->errorStr = serverErr(failedcode);
	KBENGINE_EVENT_FIRE("onKicked", pEventData);
}

void KBEngineApp::Client_onImportServerErrorsDescr(MemoryStream& stream)
{
	TArray<uint8> datas;
	datas.SetNumUninitialized(stream.length());
	memcpy(datas.GetData(), stream.data() + stream.rpos(), stream.length());

	onImportServerErrorsDescr(stream);

	if (persistentInfos_)
		persistentInfos_->onImportServerErrorsDescr(datas);
}

void KBEngineApp::onImportServerErrorsDescr(MemoryStream& stream)
{
	uint16 size = 0;
	stream >> size;

	while (size > 0)
	{
		size -= 1;

		FKServerErr e;
		e.id = stream.read<uint16>();
		stream.readUTF8String(e.name);
		stream.readUTF8String(e.descr);

	

		DEBUG_MSG("KBEngineApp::onImportServerErrorsDescr(): id=%d, name=%s, descr=%s", e.id, *e.name, *e.descr);
	}
}

void KBEngineApp::onServerDigest()
{
	if (persistentInfos_)
		persistentInfos_->onServerDigest(currserver_, serverProtocolMD5_, serverEntitydefMD5_);
}

void KBEngineApp::onConnectCallback(FString ip, uint16 port, bool success, int userdata)
{
	if (userdata == 0)
	{
		onConnectTo_loginapp_login_callback(ip, port, success);
	}
	else if (userdata == 1)
	{
		onConnectTo_loginapp_create_callback(ip, port, success);
	}
	else if (userdata == 2)
	{
		onConnectTo_baseapp_callback(ip, port, success);
	}
	else if (userdata == 3)
	{
		onReloginTo_baseapp_callback(ip, port, success);
	}
	else if (userdata == 4)
	{
		onConnectTo_resetpassword_callback(ip, port, success);
	}
	else if (userdata == 5)
	{
		//onConnectTo_resetpassword_callback(ip, port, success);
	}
	else
	{
		check(false);
	}
}

bool KBEngineApp::login(const FString& username, const FString& password, const TArray<uint8>& datas)
{
	if (username.Len() == 0)
	{
		ERROR_MSG("KBEngineApp::login(): username is empty!");
		return false;
	}

	if (password.Len() == 0)
	{
		ERROR_MSG("KBEngineApp::login(): password is empty!");
		return false;
	}

	username_ = username;
	password_ = password;
	clientdatas_ = datas;

	login_loginapp(true);
	return true;
}

void KBEngineApp::login_loginapp(bool noconnect)
{
	if (noconnect)
	{
		reset();
		pNetworkInterface_->connectTo(pArgs_->ip, pArgs_->port, this, 0);
	}
	else
	{
		INFO_MSG("KBEngineApp::login_loginapp(): send login! username=%s", *username_);
		Bundle* pBundle = Bundle::createObject();
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Loginapp_login"]));
		(*pBundle) << (uint8)pArgs_->clientType;
		pBundle->appendBlob(clientdatas_);
		(*pBundle) << username_;
		(*pBundle) << password_;
		pBundle->send(pNetworkInterface_);
	}
}

void KBEngineApp::onConnectTo_loginapp_login_callback(FString ip, uint16 port, bool success)
{
	if (!success)
	{
		ERROR_MSG("KBEngineApp::onConnectTo_loginapp_login_callback(): connect %s:%d is error!", *ip, port);
		return;
	}

	currserver_ = TEXT("loginapp");
	currstate_ = TEXT("login");

	INFO_MSG("KBEngineApp::onConnectTo_loginapp_login_callback(): connect %s:%d is success!", *ip, port);

	hello();
}

void KBEngineApp::onLogin_loginapp()
{
	lastTickCBTime_ = getTimeSeconds();

	if (!loginappMessageImported_)
	{
		Bundle* pBundle = Bundle::createObject();
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Loginapp_importClientMessages"]));
		pBundle->send(pNetworkInterface_);

		DEBUG_MSG("KBEngineApp::onLogin_loginapp(): send importClientMessages ...");

		UKBEventData_Loginapp_importClientMessages* pEventData = NewObject<UKBEventData_Loginapp_importClientMessages>();
		KBENGINE_EVENT_FIRE("Loginapp_importClientMessages", pEventData);
	}
	else
	{
		onImportClientMessagesCompleted();
	}
}

void KBEngineApp::Client_onLoginFailed(MemoryStream& stream)
{
	uint16 failedcode = 0;
	stream >> failedcode;
	stream.readBlob(serverdatas_);
	ERROR_MSG("KBEngineApp::Client_onLoginFailed(): failedcode(%d:%s), datas(%d)!", failedcode, *serverErr(failedcode), serverdatas_.Num());

	UKBEventData_onLoginFailed* pEventData = NewObject<UKBEventData_onLoginFailed>();
	pEventData->failedcode = failedcode;
	pEventData->errorStr = serverErr(failedcode);
	KBENGINE_EVENT_FIRE("onLoginFailed", pEventData);
}

void KBEngineApp::Client_onLoginSuccessfully(MemoryStream& stream)
{
	FString accountName;
	stream >> accountName;
	username_ = accountName;
	stream >> baseappIP_;
	stream >> baseappPort_;

	DEBUG_MSG("KBEngineApp::Client_onLoginSuccessfully(): accountName(%s), addr("
		 "%s:%d), datas(%d)!", *accountName, *baseappIP_, baseappPort_, serverdatas_.Num());

	stream.readBlob(serverdatas_);
	login_baseapp(true);
}

void KBEngineApp::login_baseapp(bool noconnect)
{
	if (noconnect)
	{
		KBENGINE_EVENT_FIRE("onLoginBaseapp", NewObject<UKBEventData_onLoginBaseapp>());

		pNetworkInterface_->destroy();
		pNetworkInterface_ = NULL;
		initNetwork();
		pNetworkInterface_->connectTo(baseappIP_, baseappPort_, this, 2);
	}
	else
	{
		Bundle* pBundle = Bundle::createObject();
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Baseapp_loginBaseapp"]));
		(*pBundle) << username_;
		(*pBundle) << password_;
		pBundle->send(pNetworkInterface_);
	}
}

void KBEngineApp::onConnectTo_baseapp_callback(FString ip, uint16 port, bool success)
{
	lastTickCBTime_ = getTimeSeconds();

	if (!success)
	{
		ERROR_MSG("KBEngineApp::onConnectTo_baseapp_callback(): connect %s:%d is error!", *ip, port);
		return;
	}

	currserver_ = TEXT("baseapp");
	currstate_ = TEXT("");

	DEBUG_MSG("KBEngineApp::onConnectTo_baseapp_callback(): connect %s:%d is successfully!", *ip, port);

	hello();
}

void KBEngineApp::onLogin_baseapp()
{
	lastTickCBTime_ = getTimeSeconds();

	if (!baseappMessageImported_)
	{
		Bundle* pBundle = Bundle::createObject();
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Baseapp_importClientMessages"]));
		pBundle->send(pNetworkInterface_);
		DEBUG_MSG("KBEngineApp::onLogin_baseapp(): send importClientMessages ...");
		KBENGINE_EVENT_FIRE("Baseapp_importClientMessages", NewObject<UKBEventData_Baseapp_importClientMessages>());
	}
	else
	{
		onImportClientMessagesCompleted();
	}
}

void KBEngineApp::reloginBaseapp()
{
	if(pNetworkInterface_->valid())
		return;

	UKBEventData_onReloginBaseapp* pEventData = NewObject<UKBEventData_onReloginBaseapp>();
	KBENGINE_EVENT_FIRE("KBEngineApp::reloginBaseapp(): onReloginBaseapp", pEventData);

	pNetworkInterface_->connectTo(baseappIP_, baseappPort_, this, 3);
}

void KBEngineApp::onReloginTo_baseapp_callback(FString ip, uint16 port, bool success)
{
	if (!success)
	{
		ERROR_MSG("KBEngineApp::onReloginTo_baseapp_callback(): connect %s:%d is error!", *ip, port);
		return;
	}

	INFO_MSG("KBEngineApp::onReloginTo_baseapp_callback(): connect %s:%d is success!", *ip, port);

	Bundle* pBundle = Bundle::createObject();
	pBundle->newMessage(Messages::getSingleton().messages[TEXT("Baseapp_reloginBaseapp"]));
	(*pBundle) << username_;
	(*pBundle) << password_;
	(*pBundle) << entity_uuid_;
	(*pBundle) << entity_id_;
	pBundle->send(pNetworkInterface_);

	lastTickCBTime_ = getTimeSeconds();
}

void KBEngineApp::Client_onLoginBaseappFailed(uint16 failedcode)
{
	ERROR_MSG("KBEngineApp::Client_onLoginBaseappFailed(): failedcode(%d:%s)!", failedcode, *serverErr(failedcode));

	UKBEventData_onLoginBaseappFailed* pEventData = NewObject<UKBEventData_onLoginBaseappFailed>();
	pEventData->failedcode = failedcode;
	pEventData->errorStr = serverErr(failedcode);
	KBENGINE_EVENT_FIRE("onLoginBaseappFailed", pEventData);
}

void KBEngineApp::Client_onReloginBaseappFailed(uint16 failedcode)
{
	ERROR_MSG("KBEngineApp::Client_onReloginBaseappFailed(): failedcode(%d:%s)!", failedcode, *serverErr(failedcode));

	UKBEventData_onReloginBaseappFailed* pEventData = NewObject<UKBEventData_onReloginBaseappFailed>();
	pEventData->failedcode = failedcode;
	pEventData->errorStr = serverErr(failedcode);
	KBENGINE_EVENT_FIRE("onReloginBaseappFailed", pEventData);
}

void KBEngineApp::Client_onReloginBaseappSuccessfully(MemoryStream& stream)
{
	stream >> entity_uuid_;
	ERROR_MSG("KBEngineApp::Client_onReloginBaseappSuccessfully(): name(%s)!", *username_);
	UKBEventData_onReloginBaseappSuccessfully* pEventData = NewObject<UKBEventData_onReloginBaseappSuccessfully>();
	KBENGINE_EVENT_FIRE("onReloginBaseappSuccessfully", pEventData);
}

void KBEngineApp::Client_onCreatedProxies(uint64 rndUUID, int32 eid, FString& entityType)
{
	DEBUG_MSG("KBEngineApp::Client_onCreatedProxies(): eid(%d), entityType(%s)!", eid, *entityType);

	entity_uuid_ = rndUUID;
	entity_id_ = eid;
	entity_type_ = entityType;

	if (!entities_.Contains(eid))
	{
		ScriptModule** pModuleFind = EntityDef::moduledefs.Find(entityType);
		if (!pModuleFind)
		{
			SCREEN_ERROR_MSG("KBEngineApp::Client_onCreatedProxies(): not found ScriptModule(%s)!", *entityType);
			return;
		}

		ScriptModule* pModule = *pModuleFind;

		EntityCreator* pEntityCreator = pModule->pEntityCreator;
		if (!pEntityCreator)
			return;

		Entity* pEntity = pEntityCreator->create();
		pEntity->id(eid);
		pEntity->className(entityType);

		EntityCall* baseMB = new EntityCall();
		pEntity->base(baseMB);
		baseMB->id = eid;
		baseMB->className = entityType;
		baseMB->type = EntityCall::ENTITYCALL_TYPE_BASE;

		entities_.Add(eid, pEntity);

		MemoryStream** entityMessageFind = bufferedCreateEntityMessages_.Find(eid);
		if (entityMessageFind)
		{
			MemoryStream* entityMessage = *entityMessageFind;
			Client_onUpdatePropertys(*entityMessage);
			bufferedCreateEntityMessages_.Remove(eid);
			MemoryStream::reclaimObject(entityMessage);
		}

		pEntity->__init__();
		pEntity->inited(true);

		if (pArgs_->isOnInitCallPropertysSetMethods)
			pEntity->callPropertysSetMethods();
	}
	else
	{
		MemoryStream** entityMessageFind = bufferedCreateEntityMessages_.Find(eid);
		if (entityMessageFind)
		{
			MemoryStream* entityMessage = *entityMessageFind;
			Client_onUpdatePropertys(*entityMessage);
			bufferedCreateEntityMessages_.Remove(eid);
			MemoryStream::reclaimObject(entityMessage);
		}
	}
}

ENTITY_ID KBEngineApp::getViewEntityIDFromStream(MemoryStream& stream)
{
	ENTITY_ID id = 0;

	if (!pArgs_->useAliasEntityID)
	{
		stream >> id;
		return id;
	}

	if (entityIDAliasIDList_.Num()> 255)
	{
		stream >> id;
	}
	else
	{
		uint8 aliasID = 0;
		stream >> aliasID;

		// 如果为0且客户端上一步是重登陆或者重连操作并且服务端entity在断线期间一直处于在线状态
		// 则可以忽略这个错误, 因为cellapp可能一直在向baseapp发送同步消息， 当客户端重连上时未等
		// 服务端初始化步骤开始则收到同步信息, 此时这里就会出错。
		if (entityIDAliasIDList_.Num() <= aliasID)
			return 0;

		id = entityIDAliasIDList_[aliasID];
	}

	return id;
}

void KBEngineApp::Client_onUpdatePropertysOptimized(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);
	onUpdatePropertys_(eid, stream);
}

void KBEngineApp::Client_onUpdatePropertys(MemoryStream& stream)
{
	ENTITY_ID eid;
	stream >> eid;
	onUpdatePropertys_(eid, stream);
}

void KBEngineApp::onUpdatePropertys_(ENTITY_ID eid, MemoryStream& stream)
{
	Entity** pEntityFind = entities_.Find(eid);

	if (!pEntityFind)
	{
		MemoryStream** entityMessageFind = bufferedCreateEntityMessages_.Find(eid);
		if (entityMessageFind)
		{
			ERROR_MSG("KBEngineApp::onUpdatePropertys_(): entity(%d) not found!", eid);
			return;
		}

		MemoryStream* stream1 = MemoryStream::createObject();
		stream1->append(stream);
		stream1->rpos(stream.rpos() - 4);
		bufferedCreateEntityMessages_.Add(eid, stream1);
		return;
	}
	
	Entity* pEntity = *pEntityFind;

	ScriptModule** smFind = EntityDef::moduledefs.Find(pEntity->className());
	if (!smFind)
	{
		ERROR_MSG("KBEngineApp::onUpdatePropertys_(): ScriptModule(%s) not found!", *pEntity->className());
		return;
	}

	ScriptModule* sm = *smFind;
	TMap<uint16, Property*>& pdatas = sm->idpropertys;

	while (stream.length() > 0)
	{
		uint16 utype = 0;

		if (sm->usePropertyDescrAlias)
		{
			utype = stream.read<uint8>();
		}
		else
		{
			utype = stream.read<uint16>();
		}

		Property* propertydata = pdatas[utype];
		utype = propertydata->properUtype;
		EntityDefMethodHandle* pSetMethod = propertydata->pSetMethod;

		KBVar* val = propertydata->pUtype->createFromStream(stream);
		// DEBUG_MSG("KBEngineApp::onUpdatePropertys_(): entity.className + "(id=" + eid  + " " + 
		// propertydata.name + "=" + val->c_str() + "), hasSetMethod=" + setmethod + "!");

		EntityDefPropertyHandle* pEntityDefPropertyHandle = EntityDefPropertyHandles::find(pEntity->className(), propertydata->name);
		if (!pEntityDefPropertyHandle)
		{
			SCREEN_ERROR_MSG("KBEngineApp::onUpdatePropertys_(): %s(%d) not found property(%s), update error! Please register with ENTITYDEF_PROPERTY_REGISTER(XXX, %s) in (%s, %s).cpp",
				*pEntity->className(), eid, *propertydata->name,
				*propertydata->name, *pEntity->className(), *sm->pEntityCreator->parentClasses());

			delete val;
			continue;
		}

		KBVar* oldval = pEntityDefPropertyHandle->getPropertyValue(pEntity);
		pEntityDefPropertyHandle->setPropertyValue(pEntity, val);

		if (pSetMethod)
		{
			if (propertydata->isBase())
			{
				if (pEntity->inited())
					pSetMethod->callMethod(pEntity, *oldval);
			}
			else
			{
				if (pEntity->inWorld())
					pSetMethod->callMethod(pEntity, *oldval);
			}
		}

		delete oldval;
		delete val;
	}
}

void KBEngineApp::Client_onEntityDestroyed(int32 eid)
{
	DEBUG_MSG("KBEngineApp::Client_onEntityDestroyed(): entity(%d)!", eid);

	Entity** pEntityFind = entities_.Find(eid);

	if (!pEntityFind)
	{
		ERROR_MSG("KBEngineApp::Client_onEntityDestroyed(): entity(%d) not found!", eid);
		return;
	}

	Entity* pEntity = (*pEntityFind);

	if (pEntity->inWorld())
	{
		if (entity_id_ == eid)
			clearSpace(false);

		(*pEntityFind)->leaveWorld();
	}

	if (controlledEntities_.Contains(pEntity))
	{
		controlledEntities_.Remove(pEntity);

		UKBEventData_onLoseControlledEntity* pEventData = NewObject<UKBEventData_onLoseControlledEntity>();
		pEventData->entityID = pEntity->id();
		KBENGINE_EVENT_FIRE("onLoseControlledEntity", pEventData);
	}

	entities_.Remove(eid);
	pEntity->onDestroy();
}

void KBEngineApp::clearSpace(bool isall)
{
	entityIDAliasIDList_.Empty();
	spacedatas_.Empty();
	clearEntities(isall);
	isLoadedGeometry_ = false;
	spaceID_ = 0;
}

void KBEngineApp::clearEntities(bool isall)
{
	controlledEntities_.Empty();

	if (!isall)
	{
		Entity* pEntity = player();

		for(auto& item : entities_)
		{
			if (item.Key == pEntity->id())
				continue;

			if (item.Value->inWorld())
				item.Value->leaveWorld();

			item.Value->onDestroy();
		}

		entities_.Empty();
		entities_.Add(pEntity->id(), pEntity);
	}
	else
	{
		for (auto& item : entities_)
		{
			if (item.Value->inWorld())
				item.Value->leaveWorld();

			item.Value->onDestroy();
		}

		entities_.Empty();
	}
}

void KBEngineApp::Client_initSpaceData(MemoryStream& stream)
{
	clearSpace(false);
	stream >> spaceID_;

	while (stream.length() > 0)
	{
		FString key;
		FString val;

		stream >> key >> val;
		Client_setSpaceData(spaceID_, key, val);
	}

	DEBUG_MSG("KBEngineApp::Client_initSpaceData(): spaceID(%d), size(%d)!", spaceID_, spacedatas_.Num());
}

void KBEngineApp::Client_setSpaceData(uint32 spaceID, const FString& key, const FString& value)
{
	DEBUG_MSG("KBEngineApp::Client_setSpaceData(): spaceID(%d), key(%s), value(%s)!", spaceID_, *key, *value);
	spacedatas_.Add(key, value);

	if (key == TEXT("_mapping"))
		addSpaceGeometryMapping(spaceID, value);

	UKBEventData_onSetSpaceData* pEventData = NewObject<UKBEventData_onSetSpaceData>();
	pEventData->spaceID = spaceID_;
	pEventData->key = key;
	pEventData->value = value;
	KBENGINE_EVENT_FIRE("onSetSpaceData", pEventData);
}

void KBEngineApp::Client_delSpaceData(uint32 spaceID, const FString& key)
{
	DEBUG_MSG("KBEngineApp::Client_delSpaceData(): spaceID(%d), key(%s)!", spaceID_, *key);

	spacedatas_.Remove(key);

	UKBEventData_onDelSpaceData* pEventData = NewObject<UKBEventData_onDelSpaceData>();
	pEventData->spaceID = spaceID_;
	pEventData->key = key;
	KBENGINE_EVENT_FIRE("onDelSpaceData", pEventData);
}

void KBEngineApp::addSpaceGeometryMapping(uint32 uspaceID, const FString& respath)
{
	DEBUG_MSG("KBEngineApp::addSpaceGeometryMapping(): spaceID(%d), respath(%s)!", spaceID_, *respath);

	isLoadedGeometry_ = true;
	spaceID_ = uspaceID;
	spaceResPath_ = respath;

	UKBEventData_addSpaceGeometryMapping* pEventData = NewObject<UKBEventData_addSpaceGeometryMapping>();
	pEventData->spaceResPath = spaceResPath_;
	KBENGINE_EVENT_FIRE("addSpaceGeometryMapping", pEventData);
}

FString KBEngineApp::getSpaceData(const FString& key)
{
	FString* valFind = spacedatas_.Find(key);

	if(!valFind)
		return FString();

	return (*valFind);
}

void KBEngineApp::onImportClientMessagesCompleted()
{
	DEBUG_MSG("KBEngineApp::onImportClientMessagesCompleted(): successfully! currserver=%s, currstate=%s", *currserver_, *currstate_);

	if (currserver_ == TEXT("loginapp"))
	{
		if (!isImportServerErrorsDescr_ && !loadingLocalMessages_)
		{
			DEBUG_MSG("KBEngineApp::onImportClientMessagesCompleted(): send importServerErrorsDescr!");
			isImportServerErrorsDescr_ = true;
			Bundle* pBundle = Bundle::createObject();
			pBundle->newMessage(Messages::getSingleton().messages[TEXT("Loginapp_importServerErrorsDescr"]));
			pBundle->send(pNetworkInterface_);
		}

		if (currstate_ == TEXT("login"))
		{
			login_loginapp(false);
		}
		else if (currstate_ == TEXT("autoimport"))
		{
		}
		else if (currstate_ == TEXT("resetpassword"))
		{
			resetpassword_loginapp(false);
		}
		else if (currstate_ == TEXT("createAccount"))
		{
			createAccount_loginapp(false);
		}
		else {
		}

		loginappMessageImported_ = true;
	}
	else
	{
		baseappMessageImported_ = true;

		if (!entitydefImported_ && !loadingLocalMessages_)
		{
			DEBUG_MSG("KBEngineApp::onImportClientMessagesCompleted(): send importEntityDef(%d) ...", entitydefImported_);
			Bundle* pBundle = Bundle::createObject();
			pBundle->newMessage(Messages::getSingleton().messages[TEXT("Baseapp_importClientEntityDef"]));
			pBundle->send(pNetworkInterface_);
			KBENGINE_EVENT_FIRE("Baseapp_importClientEntityDef", NewObject<UKBEventData_Baseapp_importClientEntityDef>());
		}
		else
		{
			onImportEntityDefCompleted();
		}
	}
}

void KBEngineApp::createDataTypeFromStreams(MemoryStream& stream, bool canprint)
{
	uint16 aliassize = 0;
	stream >> aliassize;

	DEBUG_MSG("KBEngineApp::createDataTypeFromStreams(): importAlias(size=%d)!", aliassize);

	while (aliassize > 0)
	{
		aliassize--;
		createDataTypeFromStream(stream, canprint);
	};

	for (auto& item : EntityDef::datatypes)
	{
		if (item.Value)
		{
			item.Value->bind();
		}
	}
}

void KBEngineApp::createDataTypeFromStream(MemoryStream& stream, bool canprint)
{
	uint16 utype = 0;
	stream >> utype;

	FString name;
	stream >> name;

	FString valname;
	stream >> valname;

	/* 有一些匿名类型，我们需要提供一个唯一名称放到datatypes中
		如：
		<onRemoveAvatar>
		<Arg>	ARRAY <of> INT8 </of>		</Arg>
		</onRemoveAvatar>
	*/
	if (valname.Len() == 0)
		valname = FString::Printf(TEXT("Null_%d"), (int)utype);

	if (canprint)
		DEBUG_MSG("KBEngineApp::createDataTypeFromStream(): importAlias(%s:%s:%d)!", *name, *valname, utype);

	if (name == TEXT("FIXED_DICT"))
	{
		KBEDATATYPE_FIXED_DICT* datatype = new KBEDATATYPE_FIXED_DICT();
		uint8 keysize;
		stream >> keysize;

		stream >> datatype->implementedBy;

		while (keysize > 0)
		{
			keysize--;

			FString keyname;
			stream >> keyname;

			uint16 keyutype;
			stream >> keyutype;

			datatype->dicttype_map.Add(keyname, keyutype);
		};

		EntityDef::datatypes.Add(valname, datatype);
	}
	else if (name == TEXT("ARRAY"))
	{
		uint16 uitemtype;
		stream >> uitemtype;

		KBEDATATYPE_ARRAY* datatype = new KBEDATATYPE_ARRAY();
		datatype->tmpset_uitemtype = (int)uitemtype;
		EntityDef::datatypes.Add(valname, datatype);
	}
	else
	{
		// 可能会重复向map添加基本类型， 此时需要过滤掉
		//if (EntityDef::datatypes.Contains(valname))
		//	return;

		KBEDATATYPE_BASE* val = NULL;
		if (EntityDef::datatypes.Contains(name))
			val = EntityDef::datatypes[name];

		EntityDef::datatypes.Add(valname, val);
	}

	// 可能会重复向map添加基本类型， 此时需要过滤掉
	//if (EntityDef::id2datatypes.Contains(utype))
	//	return;

	EntityDef::id2datatypes.Add(utype, EntityDef::datatypes[valname]);

	// 将用户自定义的类型补充到映射表中
	EntityDef::datatype2id.Add(valname, utype);

}

void KBEngineApp::Client_onImportClientEntityDef(MemoryStream& stream)
{
	TArray<uint8> datas;
	datas.SetNumUninitialized(stream.length());
	memcpy(datas.GetData(), stream.data() + stream.rpos(), stream.length());

	onImportClientEntityDef(stream);

	if (persistentInfos_)
		persistentInfos_->onImportClientEntityDef(datas);
}

void KBEngineApp::onImportClientEntityDef(MemoryStream& stream)
{
	createDataTypeFromStreams(stream, true);

	while (stream.length() > 0)
	{
		FString scriptmodule_name;
		stream >> scriptmodule_name;

		uint16 scriptUtype;
		stream >> scriptUtype;

		uint16 propertysize;
		stream >> propertysize;

		uint16 methodsize;
		stream >> methodsize;

		uint16 base_methodsize;
		stream >> base_methodsize;

		uint16 cell_methodsize;
		stream >> cell_methodsize;

		DEBUG_MSG("KBEngineApp::onImportClientEntityDef(): import(%s), propertys(%d), "
			"clientMethods(%d), baseMethods(%d), cellMethods(%d)!", *scriptmodule_name,
			propertysize, methodsize, base_methodsize, cell_methodsize);

		ScriptModule* module = new ScriptModule(scriptmodule_name);
		EntityDef::moduledefs.Add(scriptmodule_name, module);
		EntityDef::idmoduledefs.Add(scriptUtype, module);

		EntityCreator* pEntityCreator = module->pEntityCreator;

		while (propertysize > 0)
		{
			propertysize--;

			uint16 properUtype;
			stream >> properUtype;

			uint32 properFlags;
			stream >> properFlags;

			int16 paliasID;
			stream >> paliasID;

			FString pname;
			stream >> pname;

			FString defaultValStr;
			stream >> defaultValStr;

			uint16 iutype;
			stream >> iutype;
			KBEDATATYPE_BASE* utype = EntityDef::id2datatypes[iutype];

			EntityDefMethodHandle* pEntityDefMethodHandle = NULL;

			if (pEntityCreator)
				pEntityDefMethodHandle = EntityDefMethodHandles::find(scriptmodule_name, FString::Printf(TEXT("set_%s"), *pname));

			Property* savedata = new Property();
			savedata->name = pname;
			savedata->pUtype = utype;
			savedata->properUtype = properUtype;
			savedata->properFlags = properFlags;
			savedata->aliasID = paliasID;
			savedata->defaultValStr = defaultValStr;
			savedata->pSetMethod = pEntityDefMethodHandle;
			savedata->pdefaultVal = savedata->pUtype->parseDefaultValStr(savedata->defaultValStr);

			module->propertys.Add(pname, savedata);

			if (paliasID != -1)
			{
				module->usePropertyDescrAlias = true;
				module->idpropertys.Add(paliasID, savedata);
			}
			else
			{
				module->usePropertyDescrAlias = false;
				module->idpropertys.Add(properUtype, savedata);
			}

			DEBUG_MSG("KBEngineApp::onImportClientEntityDef(): add(%s), property(%s/%d), hasSetMethod=%s.", *scriptmodule_name, *pname, 
				properUtype, (savedata->pSetMethod ? TEXT("true") : TEXT("false")));
		};

		while (methodsize > 0)
		{
			methodsize--;

			uint16 methodUtype;
			stream >> methodUtype;

			int16 ialiasID;
			stream >> ialiasID;

			FString name;
			stream >> name;

			uint8 argssize;
			stream >> argssize;

			TArray<KBEDATATYPE_BASE*> args;

			while (argssize > 0)
			{
				argssize--;
				uint16 tmpType;
				stream >> tmpType;
				args.Add(EntityDef::id2datatypes[tmpType]);
			};

			Method* savedata = new Method();
			savedata->name = name;
			savedata->methodUtype = methodUtype;
			savedata->aliasID = ialiasID;
			savedata->args = args;

			if (pEntityCreator)
				savedata->pEntityDefMethodHandle = EntityDefMethodHandles::find(scriptmodule_name, name);

			module->methods.Add(name, savedata);

			if (ialiasID != -1)
			{
				module->useMethodDescrAlias = true;
				module->idmethods.Add(ialiasID, savedata);
			}
			else
			{
				module->useMethodDescrAlias = false;
				module->idmethods.Add(methodUtype, savedata);
			}

			DEBUG_MSG("KBEngineApp::onImportClientEntityDef(): add(%s), method(%s).", *scriptmodule_name, *name);
		};

		while (base_methodsize > 0)
		{
			base_methodsize--;

			uint16 methodUtype;
			stream >> methodUtype;

			int16 ialiasID;
			stream >> ialiasID;

			FString name;
			stream >> name;

			uint8 argssize;
			stream >> argssize;

			TArray<KBEDATATYPE_BASE*> args;

			while (argssize > 0)
			{
				argssize--;
				uint16 tmpType;
				stream >> tmpType;
				args.Add(EntityDef::id2datatypes[tmpType]);
			};

			Method* savedata = new Method();
			savedata->name = name;
			savedata->methodUtype = methodUtype;
			savedata->aliasID = ialiasID;
			savedata->args = args;

			module->base_methods.Add(name, savedata);
			module->idbase_methods.Add(methodUtype, savedata);

			DEBUG_MSG("KBEngineApp::onImportClientEntityDef(): add(%s), base_method(%s).", *scriptmodule_name, *name);
		};

		while (cell_methodsize > 0)
		{
			cell_methodsize--;

			uint16 methodUtype;
			stream >> methodUtype;

			int16 ialiasID;
			stream >> ialiasID;

			FString name;
			stream >> name;

			uint8 argssize;
			stream >> argssize;

			TArray<KBEDATATYPE_BASE*> args;

			while (argssize > 0)
			{
				argssize--;
				uint16 tmpType;
				stream >> tmpType;
				args.Add(EntityDef::id2datatypes[tmpType]);
			};

			Method* savedata = new Method();
			savedata->name = name;
			savedata->methodUtype = methodUtype;
			savedata->aliasID = ialiasID;
			savedata->args = args;

			module->cell_methods.Add(name, savedata);
			module->idcell_methods.Add(methodUtype, savedata);

			DEBUG_MSG("KBEngineApp::onImportClientEntityDef(): add(%s), cell_method(%s).", *scriptmodule_name, *name);
		};

		if (!pEntityCreator)
		{
			SCREEN_ERROR_MSG("KBEngineApp::onImportClientEntityDef(): ScriptModule(%s) not found!", *scriptmodule_name);
		}

		for(auto& e : module->methods)
		{
			FString name = e.Key;
			
			if (pEntityCreator && !EntityDefMethodHandles::find(scriptmodule_name, name))
			{
				SCREEN_WARNING_MSG("KBEngineApp::onImportClientEntityDef(): %s:: method(%s) no implement!", *scriptmodule_name, *name);
			}
		};
	};

	onImportEntityDefCompleted();
}

void KBEngineApp::onImportEntityDefCompleted()
{
	DEBUG_MSG("KBEngineApp::onImportEntityDefCompleted(): successfully!");
	entitydefImported_ = true;

	if (!loadingLocalMessages_)
		login_baseapp(false);
}

void KBEngineApp::Client_onImportClientMessages(MemoryStream& stream)
{
	TArray<uint8> datas;
	datas.SetNumUninitialized(stream.length());
	memcpy(datas.GetData(), stream.data() + stream.rpos(), stream.length());

	onImportClientMessages(stream);

	if (persistentInfos_)
		persistentInfos_->onImportClientMessages(currserver_, datas);
}

void KBEngineApp::onImportClientMessages(MemoryStream& stream)
{
	uint16 msgcount = 0;
	stream >> msgcount;

	DEBUG_MSG("KBEngineApp::onImportClientMessages(): start currserver=%s(msgsize=%d)...", *currserver_, msgcount);

	while (msgcount > 0)
	{
		msgcount--;

		MessageID msgid = 0;
		stream >> msgid;

		int16 msglen = 0;
		stream >> msglen;

		FString msgname;
		stream >> msgname;
		
		int8 argstype = 0;
		stream >> argstype;

		uint8 argsize = 0;
		stream >> argsize;

		TArray<uint8> argstypes;

		for (uint8 i = 0; i<argsize; i++)
		{
			uint8 v = 0;
			stream >> v;
			argstypes.Add(v);
		}

		Message* handler = Messages::getSingleton().findMessage(msgname);
		bool isClientMethod = msgname.Contains(TEXT("Client_"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);

		if (isClientMethod)
		{
			if (handler == NULL)
			{
				SCREEN_WARNING_MSG("KBEngineApp::onImportClientMessages(): currserver[%s]: interface(%s/%d/%d) no implement!",
					*currserver_, *msgname, msgid, msglen);
			}
			else
			{
				DEBUG_MSG("KBEngineApp::onImportClientMessages(): imported(%s/%d/%d) successfully!", 
					*msgname, msgid, msglen);
			}
		}

		if (handler)
		{
			FString old_cstr = handler->c_str();
			handler->id = msgid;
			handler->msglen = msglen;

			// 因为握手类ID一开始临时设置为负数， 所以需要重新以正确的ID添加到列表
			if (isClientMethod)
				Messages::getSingleton().add(handler, msgid, msgname, msglen);

			if (handler->id <= 0)
			{
				DEBUG_MSG("KBEngineApp::onImportClientMessages(): currserver[%s]: refreshed(%s => %s)!",
					*currserver_, *old_cstr, *handler->c_str());
			}
		}
		else
		{
			if (msgname != TEXT(""))
			{
				Messages::getSingleton().messages.Add(msgname, new Message(msgid, msgname, msglen));

				if(!isClientMethod)
					DEBUG_MSG("KBEngineApp::onImportClientMessages(): currserver[%s]: imported(%s/%d/%d) successfully!",
						*currserver_, *msgname, msgid, msglen);

				if (isClientMethod)
				{
					Messages::getSingleton().clientMessages.Add(msgid, Messages::getSingleton().messages[msgname]);
				}
				else
				{
					if (currserver_ == TEXT("loginapp"))
						Messages::getSingleton().loginappMessages.Add(msgid, Messages::getSingleton().messages[msgname]);
					else
						Messages::getSingleton().baseappMessages.Add(msgid, Messages::getSingleton().messages[msgname]);
				}
			}
			else
			{
				Message* msg = new Message(msgid, msgname, msglen);

				if(!isClientMethod)
					DEBUG_MSG("KBEngineApp::onImportClientMessages(): currserver[%s]: imported(%s/%d/%d) successfully!",
						*currserver_, *msgname, msgid, msglen);

				if (currserver_ == TEXT("loginapp"))
					Messages::getSingleton().loginappMessages.Add(msgid, msg);
				else
					Messages::getSingleton().baseappMessages.Add(msgid, msg);
			}
		}
	};

	onImportClientMessagesCompleted();
}

void KBEngineApp::resetPassword(const FString& username)
{
	username_ = username;
	resetpassword_loginapp(true);
}

void KBEngineApp::resetpassword_loginapp(bool noconnect)
{
	if (noconnect)
	{
		reset();
		pNetworkInterface_->connectTo(pArgs_->ip, pArgs_->port, this, 4);
	}
	else
	{
		INFO_MSG("KBEngineApp::resetpassword_loginapp(): send resetpassword! username=%s", *username_);
		Bundle* pBundle = Bundle::createObject();
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Loginapp_reqAccountResetPassword"]));
		(*pBundle) << username_;
		pBundle->send(pNetworkInterface_);
	}
}

void KBEngineApp::onOpenLoginapp_resetpassword()
{
	DEBUG_MSG("KBEngineApp::onOpenLoginapp_resetpassword(): successfully!");
	currserver_ = "loginapp";
	currstate_ = "resetpassword";
	lastTickCBTime_ = getTimeSeconds();

	if (!loginappMessageImported_)
	{
		Bundle* pBundle = Bundle::createObject();
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Loginapp_importClientMessages"]));
		pBundle->send(pNetworkInterface_);
		DEBUG_MSG("KBEngineApp::onOpenLoginapp_resetpassword(): send importClientMessages ...");
	}
	else
	{
		onImportClientMessagesCompleted();
	}
}

void KBEngineApp::onConnectTo_resetpassword_callback(FString ip, uint16 port, bool success)
{
	lastTickCBTime_ = getTimeSeconds();

	if (!success)
	{
		ERROR_MSG("KBEngineApp::onConnectTo_resetpassword_callback(): connect %s:%d is error!", *ip, port);
		return;
	}

	INFO_MSG("KBEngineApp::onConnectTo_resetpassword_callback(): connect %s:%d is success!", *ip, port);

	onOpenLoginapp_resetpassword();
}

void KBEngineApp::Client_onReqAccountResetPasswordCB(uint16 failcode)
{
	if (failcode != 0)
	{
		ERROR_MSG("KBEngineApp::Client_onReqAccountResetPasswordCB(): reset failed! code=%d, error=%s! username=%s", failcode, *serverErr(failcode), *username_);
		return;
	}

	DEBUG_MSG("KBEngineApp::Client_onReqAccountResetPasswordCB(): successfully! username=%s", *username_);
}

bool KBEngineApp::createAccount(const FString& username, const FString& password, const TArray<uint8>& datas)
{
	if (username.Len() == 0)
	{
		ERROR_MSG("KBEngineApp::createAccount(): username is empty!");
		return false;
	}

	if (password.Len() == 0)
	{
		ERROR_MSG("KBEngineApp::createAccount(): password is empty!");
		return false;
	}

	username_ = username;
	password_ = password;
	clientdatas_ = datas;

	createAccount_loginapp(true);
	return true;
}

void KBEngineApp::createAccount_loginapp(bool noconnect)
{
	if (noconnect)
	{
		reset();
		pNetworkInterface_->connectTo(pArgs_->ip, pArgs_->port, this, 1);
	}
	else
	{
		INFO_MSG("KBEngineApp::createAccount_loginapp(): send create! username=%s", *username_);
		Bundle* pBundle = Bundle::createObject();
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Loginapp_reqCreateAccount"]));
		(*pBundle) << username_;
		(*pBundle) << password_;
		pBundle->appendBlob(clientdatas_);
		pBundle->send(pNetworkInterface_);
	}
}

void KBEngineApp::onOpenLoginapp_createAccount()
{
	DEBUG_MSG("KBEngineApp::onOpenLoginapp_createAccount(): successfully!");

	currserver_ = TEXT("loginapp");
	currstate_ = TEXT("createAccount");
	lastTickCBTime_ = getTimeSeconds();

	if (!loginappMessageImported_)
	{
		Bundle* pBundle = Bundle::createObject();
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Loginapp_importClientMessages"]));
		pBundle->send(pNetworkInterface_);
		DEBUG_MSG("KBEngineApp::onOpenLoginapp_createAccount(): send importClientMessages ...");
	}
	else
	{
		onImportClientMessagesCompleted();
	}
}

void KBEngineApp::onConnectTo_loginapp_create_callback(FString ip, uint16 port, bool success)
{
	lastTickCBTime_ = getTimeSeconds();

	if (!success)
	{
		ERROR_MSG("KBEngineApp::onConnectTo_loginapp_create_callback(): connect %s:%d is error!", *ip, port);
		return;
	}

	INFO_MSG("KBEngineApp::onConnectTo_loginapp_create_callback(): connect %s:%d is success!", *ip, port);

	onOpenLoginapp_createAccount();
}

void KBEngineApp::Client_onCreateAccountResult(MemoryStream& stream)
{
	uint16 retcode;
	stream >> retcode;

	TArray<uint8> datas;
	stream.readBlob(datas);

	UKBEventData_onCreateAccountResult* pEventData = NewObject<UKBEventData_onCreateAccountResult>();
	pEventData->errorCode = retcode;
	pEventData->errorStr = serverErr(retcode);
	pEventData->datas = datas;
	KBENGINE_EVENT_FIRE("onCreateAccountResult", pEventData);

	if (retcode != 0)
	{
		WARNING_MSG("KBEngineApp::Client_onCreateAccountResult(): create(%s) failed! error=%d(%s)!", *username_, retcode, *serverErr(retcode));
		return;
	}

	DEBUG_MSG("KBEngineApp::Client_onCreateAccountResult(): create(%s) is successfully!", *username_);
}

void KBEngineApp::bindAccountEmail(const FString& emailAddress)
{
	INFO_MSG("KBEngineApp::bindAccountEmail(): send bindAccountEmail! username=%s", *username_);
	Bundle* pBundle = Bundle::createObject();
	pBundle->newMessage(Messages::getSingleton().messages[TEXT("Baseapp_reqAccountBindEmail"]));
	(*pBundle) << entity_id_;
	(*pBundle) << password_;
	(*pBundle) << emailAddress;
	pBundle->send(pNetworkInterface_);
}

void KBEngineApp::Client_onReqAccountBindEmailCB(uint16 failcode)
{
	if (failcode != 0)
	{
		ERROR_MSG("KBEngineApp::Client_onReqAccountBindEmailCB(): bind failed! code=%d, error=%s! username=%s", failcode, *serverErr(failcode), *username_);
		return;
	}

	DEBUG_MSG("KBEngineApp::Client_onReqAccountBindEmailCB(): successfully! username=%s", *username_);
}

void KBEngineApp::newPassword(const FString& old_password, const FString& new_password)
{
	INFO_MSG("KBEngineApp::newPassword(): send newPassword! username=%s", *username_);
	Bundle* pBundle = Bundle::createObject();
	pBundle->newMessage(Messages::getSingleton().messages[TEXT("Baseapp_reqAccountNewPassword"]));
	(*pBundle) << entity_id_;
	(*pBundle) << password_;
	(*pBundle) << old_password;
	(*pBundle) << new_password;
	pBundle->send(pNetworkInterface_);
}

void KBEngineApp::Client_onReqAccountNewPasswordCB(uint16 failcode)
{
	if (failcode != 0)
	{
		ERROR_MSG("KBEngineApp::Client_onReqAccountNewPasswordCB(): newPassword failed! code=%d, error=%s! username=%s", failcode, *serverErr(failcode), *username_);
		return;
	}

	DEBUG_MSG("KBEngineApp::Client_onReqAccountNewPasswordCB(): successfully! username=%s", *username_);
}

void KBEngineApp::Client_onRemoteMethodCallOptimized(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);
	onRemoteMethodCall_(eid, stream);
}

void KBEngineApp::Client_onRemoteMethodCall(MemoryStream& stream)
{
	ENTITY_ID eid = 0;
	stream >> eid;
	onRemoteMethodCall_(eid, stream);
}

void KBEngineApp::onRemoteMethodCall_(ENTITY_ID eid, MemoryStream& stream)
{
	Entity** pEntityFind = entities_.Find(eid);

	if (!pEntityFind)
	{
		ERROR_MSG("KBEngineApp::onRemoteMethodCall_(): entity(%d) not found!", eid);
		return;
	}

	Entity* pEntity = *pEntityFind;

	uint16 methodUtype = 0;

	ScriptModule** pModuleFind = EntityDef::moduledefs.Find(pEntity->className());
	if (!pModuleFind)
	{
		SCREEN_ERROR_MSG("KBEngineApp::onRemoteMethodCall_(): not found ScriptModule(%s)!", *pEntity->className());
		return;
	}

	ScriptModule* pModule = *pModuleFind;

	if (pModule->useMethodDescrAlias)
		methodUtype = stream.read<uint8>();
	else
		methodUtype = stream.read<uint16>();

	Method* pMethodData = pModule->idmethods[methodUtype];

	TArray<KBVar*> args;
	for (int i = 0; i<pMethodData->args.Num(); ++i)
	{
		KBVar* pVar = pMethodData->args[i]->createFromStream(stream);
		args.Add(pVar);
	}

	if (pMethodData->pEntityDefMethodHandle)
	{
		pMethodData->pEntityDefMethodHandle->callMethod(pEntity, args);
	}
	else
	{
		SCREEN_ERROR_MSG("KBEngineApp::onRemoteMethodCall_(): %s(%d), not found method(%s.%s), update error! Please register with ENTITYDEF_METHOD_ARGS*_REGISTER(XXX, %s) in (%s, %s).cpp",
			*pEntity->className(), eid, *pEntity->className(), *pMethodData->name,
			*pMethodData->name, *pEntity->className(),
			*pModule->pEntityCreator->parentClasses());
	}

	for (auto& item : args)
		delete item;
}

void KBEngineApp::Client_onControlEntity(ENTITY_ID eid, int8 isControlled)
{
	Entity** pEntityFind = entities_.Find(eid);

	if (!pEntityFind)
	{
		ERROR_MSG("KBEngineApp::Client_onControlEntity(): entity(%d) not found!", eid);
		return;
	}

	bool isCont = isControlled != 0;

	if (isCont)
	{
		// 如果被控制者是玩家自己，那表示玩家自己被其它人控制了
		// 所以玩家自己不应该进入这个被控制列表
		if (entity_id_ != (*pEntityFind)->id())
		{
			controlledEntities_.Add((*pEntityFind));
		}
	}
	else
	{
		controlledEntities_.Remove((*pEntityFind));
	}

	(*pEntityFind)->isControlled(isCont);
	(*pEntityFind)->onControlled(isCont);

	UKBEventData_onControlled* pEventData = NewObject<UKBEventData_onControlled>();
	pEventData->entityID = (*pEntityFind)->id();
	pEventData->isControlled = isCont;
	KBENGINE_EVENT_FIRE("onControlled", pEventData);
}

void KBEngineApp::Client_onStreamDataStarted(int16 id, uint32 datasize, FString descr)
{
	UKBEventData_onStreamDataStarted* pEventData = NewObject<UKBEventData_onStreamDataStarted>();
	pEventData->resID = id;
	pEventData->dataSize = datasize;
	pEventData->dataDescr = descr;
	KBENGINE_EVENT_FIRE("onStreamDataStarted", pEventData);
}

void KBEngineApp::Client_onStreamDataRecv(MemoryStream& stream)
{
	UKBEventData_onStreamDataRecv* pEventData = NewObject<UKBEventData_onStreamDataRecv>();

	uint16 id = stream.read<uint16>();
	pEventData->resID = id;
	stream.readBlob(pEventData->data);

	KBENGINE_EVENT_FIRE("onStreamDataRecv", pEventData);
}

void KBEngineApp::Client_onStreamDataCompleted(int16 id)
{
	UKBEventData_onStreamDataCompleted* pEventData = NewObject<UKBEventData_onStreamDataCompleted>();
	pEventData->resID = id;
	KBENGINE_EVENT_FIRE("onStreamDataCompleted", pEventData);
}

void KBEngineApp::Client_onEntityEnterWorld(MemoryStream& stream)
{
	ENTITY_ID eid;
	stream >> eid;

	if (entity_id_ > 0 && entity_id_ != eid)
		entityIDAliasIDList_.Add(eid);

	uint16 uEntityType;

	if (EntityDef::idmoduledefs.Num() > 255)
		uEntityType = stream.read<uint16>();
	else
		uEntityType = stream.read<uint8>();

	int8 isOnGround = 1;

	if (stream.length() > 0)
		isOnGround = stream.read<int8>();

	ScriptModule** pScriptModuleFind = EntityDef::idmoduledefs.Find(uEntityType);
	if (!pScriptModuleFind)
	{
		SCREEN_ERROR_MSG("KBEngineApp::Client_onEntityEnterWorld(): not found ScriptModule(utype = %d)!", uEntityType);
		return;
	}

	ScriptModule* pScriptModule = *pScriptModuleFind;
	FString entityType = pScriptModule->name;
	// DEBUG_MSG("KBEngineApp::Client_onEntityEnterWorld(): %s(%d), spaceID(%d)!", *entityType, eid, spaceID_);

	Entity** pEntityFind = entities_.Find(eid);

	if (!pEntityFind)
	{
		MemoryStream** entityMessageFind = bufferedCreateEntityMessages_.Find(eid);
		if (!entityMessageFind)
		{
			ERROR_MSG("KBEngineApp::Client_onEntityEnterWorld(): entity(%d) not found!", eid);
			return;
		}

		pScriptModuleFind = EntityDef::moduledefs.Find(entityType);
		if (!pScriptModuleFind)
		{
			SCREEN_ERROR_MSG("KBEngineApp::Client_onEntityEnterWorld(): not found ScriptModule(%s)!", *entityType);
			return;
		}

		ScriptModule* pModule = *pScriptModuleFind;

		EntityCreator* pEntityCreator = pModule->pEntityCreator;
		if (!pEntityCreator)
			return;

		Entity* pEntity = pEntityCreator->create();
		pEntity->id(eid);
		pEntity->className(entityType);

		EntityCall* cellMB = new EntityCall();
		pEntity->cell(cellMB);
		cellMB->id = eid;
		cellMB->className = entityType;
		cellMB->type = EntityCall::ENTITYCALL_TYPE_CELL;

		entities_.Add(eid, pEntity);

		Client_onUpdatePropertys(*(*entityMessageFind));
		MemoryStream::reclaimObject((*entityMessageFind));
		bufferedCreateEntityMessages_.Remove(eid);

		pEntity->isOnGround(isOnGround > 0);
		pEntity->set_direction(pEntity->direction);
		pEntity->set_position(pEntity->position);

		pEntity->__init__();
		pEntity->inited(true);
		pEntity->inWorld(true);
		pEntity->enterWorld();

		if (pArgs_->isOnInitCallPropertysSetMethods)
			pEntity->callPropertysSetMethods();
	}
	else
	{
		Entity* pEntity = (*pEntityFind);

		if (!pEntity->inWorld())
		{
			// 安全起见， 这里清空一下
			// 如果服务端上使用giveClientTo切换控制权
			// 之前的实体已经进入世界， 切换后的实体也进入世界， 这里可能会残留之前那个实体进入世界的信息
			entityIDAliasIDList_.Empty();
			clearEntities(false);
			entities_.Add(pEntity->id(), pEntity);

			EntityCall* cellMB = new EntityCall();
			pEntity->cell(cellMB);
			cellMB->id = eid;
			cellMB->className = entityType;
			cellMB->type = EntityCall::ENTITYCALL_TYPE_CELL;

			pEntity->set_direction(pEntity->direction);
			pEntity->set_position(pEntity->position);

			entityServerPos_ = pEntity->position;
			pEntity->isOnGround(isOnGround > 0);
			pEntity->inWorld(true);
			pEntity->enterWorld();

			if (pArgs_->isOnInitCallPropertysSetMethods)
				pEntity->callPropertysSetMethods();
		}
	}
}

void KBEngineApp::Client_onEntityLeaveWorldOptimized(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);
	Client_onEntityLeaveWorld(eid);
}

void KBEngineApp::Client_onEntityLeaveWorld(ENTITY_ID eid)
{
	Entity** pEntityFind = entities_.Find(eid);
	
	if (!pEntityFind)
	{
		ERROR_MSG("KBEngineApp::Client_onEntityLeaveWorld(): entity(%d) not found!", eid);
		return;
	}

	Entity* pEntity = *pEntityFind;

	if (pEntity->inWorld())
		pEntity->leaveWorld();

	if (entity_id_ == eid)
	{
		clearSpace(false);
		pEntity->cell(NULL);
	}
	else
	{
		if (controlledEntities_.Contains(pEntity))
		{
			controlledEntities_.Remove(pEntity);

			UKBEventData_onLoseControlledEntity* pEventData = NewObject<UKBEventData_onLoseControlledEntity>();
			pEventData->entityID = pEntity->id();
			KBENGINE_EVENT_FIRE("onLoseControlledEntity", pEventData);
		}

		entities_.Remove(eid);
		pEntity->onDestroy();
		entityIDAliasIDList_.Remove(eid);
	}
}

void KBEngineApp::Client_onEntityEnterSpace(MemoryStream& stream)
{
	ENTITY_ID eid = stream.read<int32>();
	spaceID_ = stream.read<uint32>();

	int8 isOnGround = 1;

	if (stream.length() > 0)
		isOnGround = stream.read<int8>();

	Entity** pEntityFind = entities_.Find(eid);

	if (!pEntityFind)
	{
		ERROR_MSG("KBEngineApp::Client_onEntityEnterSpace(): entity(%d) not found!", eid);
		return;
	}

	Entity* pEntity = *pEntityFind;
	pEntity->isOnGround(isOnGround > 0);
	entityServerPos_ = pEntity->position;
	pEntity->enterSpace();
}

void KBEngineApp::Client_onEntityLeaveSpace(ENTITY_ID eid)
{
	Entity** pEntityFind = entities_.Find(eid);

	if (!pEntityFind)
	{
		ERROR_MSG("KBEngineApp::Client_onEntityLeaveSpace(): entity(%d) not found!", eid);
		return;
	}

	Entity* pEntity = *pEntityFind;
	pEntity->leaveSpace();

	clearSpace(false);
}

void KBEngineApp::Client_onUpdateBasePos(float x, float y, float z)
{
	entityServerPos_.X = x;
	entityServerPos_.Y = y;
	entityServerPos_.Z = z;

	Entity* pEntity = player();
	if (pEntity && pEntity->isControlled())
	{
		pEntity->position.Set(entityServerPos_.X, entityServerPos_.Y, entityServerPos_.Z);

		UKBEventData_updatePosition* pEventData = NewObject<UKBEventData_updatePosition>();
		KBPos2UE4Pos(pEventData->position, entityServerPos_);
		pEventData->entityID = pEntity->id();
		pEventData->moveSpeed = pEntity->velocity();
		KBENGINE_EVENT_FIRE("updatePosition", pEventData);

		pEntity->onUpdateVolatileData();
	}
}

void KBEngineApp::Client_onUpdateBasePosXZ(float x, float z)
{
	entityServerPos_.X = x;
	entityServerPos_.Z = z;

	Entity* pEntity = player();
	if (pEntity && pEntity->isControlled())
	{
		pEntity->position.X = entityServerPos_.X;
		pEntity->position.Z = entityServerPos_.Z;

		UKBEventData_updatePosition* pEventData = NewObject<UKBEventData_updatePosition>();
		KBPos2UE4Pos(pEventData->position, entityServerPos_);
		pEventData->entityID = pEntity->id();
		pEventData->moveSpeed = pEntity->velocity();
		KBENGINE_EVENT_FIRE("updatePosition", pEventData);

		pEntity->onUpdateVolatileData();
	}
}

void KBEngineApp::Client_onUpdateBaseDir(MemoryStream& stream)
{
	float yaw, pitch, roll;
	stream >> yaw >> pitch >> roll;

	Entity* pEntity = player();
	if (pEntity && pEntity->isControlled())
	{
		pEntity->direction.Set(roll, pitch, yaw);

		UKBEventData_set_direction* pEventData = NewObject<UKBEventData_set_direction>();
		pEventData->direction = pEntity->direction;
		pEventData->entityID = pEntity->id();
		KBENGINE_EVENT_FIRE("set_direction", pEventData);

		pEntity->onUpdateVolatileData();
	}
}

void KBEngineApp::Client_onUpdateData(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	Entity** pEntityFind = entities_.Find(eid);

	if (!pEntityFind)
	{
		ERROR_MSG("KBEngineApp::Client_onUpdateData(): entity(%d) not found!", eid);
		return;
	}
}

void KBEngineApp::Client_onSetEntityPosAndDir(MemoryStream& stream)
{
	ENTITY_ID eid;
	stream >> eid;

	Entity** pEntityFind = entities_.Find(eid);

	if (!pEntityFind)
	{
		ERROR_MSG("KBEngineApp::Client_onSetEntityPosAndDir(): entity(%d) not found!", eid);
		return;
	}
	
	Entity& entity = *(*pEntityFind);

	FVector old_position = entity.position;
	FVector old_direction = entity.direction;

	entity.position.X = stream.read<float>();
	entity.position.Y = stream.read<float>();
	entity.position.Z = stream.read<float>();

	entity.direction.X = stream.read<float>();
	entity.direction.Y = stream.read<float>();
	entity.direction.Z = stream.read<float>();

	entity.entityLastLocalPos = entity.position;
	entity.entityLastLocalDir = entity.direction;

	entity.set_direction(old_direction);
	entity.set_position(old_position);
}

void KBEngineApp::Client_onUpdateData_ypr(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	int8 y = stream.read<int8>();
	int8 p = stream.read<int8>();
	int8 r = stream.read<int8>();

	_updateVolatileData(eid, KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX, y, p, r, -1);
}

void KBEngineApp::Client_onUpdateData_yp(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	int8 y = stream.read<int8>();
	int8 p = stream.read<int8>();

	_updateVolatileData(eid, KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX, y, p, KBE_FLT_MAX, -1);
}

void KBEngineApp::Client_onUpdateData_yr(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	int8 y = stream.read<int8>();
	int8 r = stream.read<int8>();

	_updateVolatileData(eid, KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX, y, KBE_FLT_MAX, r, -1);
}

void KBEngineApp::Client_onUpdateData_pr(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	int8 p = stream.read<int8>();
	int8 r = stream.read<int8>();

	_updateVolatileData(eid, KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX, p, r, -1);
}

void KBEngineApp::Client_onUpdateData_y(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	int8 y = stream.read<int8>();

	_updateVolatileData(eid, KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX, y, KBE_FLT_MAX, KBE_FLT_MAX, -1);
}

void KBEngineApp::Client_onUpdateData_p(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	int8 p = stream.read<int8>();

	_updateVolatileData(eid, KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX, p, KBE_FLT_MAX, -1);
}

void KBEngineApp::Client_onUpdateData_r(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	int8 r = stream.read<int8>();

	_updateVolatileData(eid, KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX, r, -1);
}

void KBEngineApp::Client_onUpdateData_xz(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);

	_updateVolatileData(eid, xz.X, KBE_FLT_MAX, xz.Z, KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX, 1);
}

void KBEngineApp::Client_onUpdateData_xz_ypr(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);

	int8 y = stream.read<int8>();
	int8 p = stream.read<int8>();
	int8 r = stream.read<int8>();

	_updateVolatileData(eid, xz.X, KBE_FLT_MAX, xz.Z, y, p, r, 1);
}

void KBEngineApp::Client_onUpdateData_xz_yp(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);

	int8 y = stream.read<int8>();
	int8 p = stream.read<int8>();

	_updateVolatileData(eid, xz.X, KBE_FLT_MAX, xz.Z, y, p, KBE_FLT_MAX, 1);
}

void KBEngineApp::Client_onUpdateData_xz_yr(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);

	int8 y = stream.read<int8>();
	int8 r = stream.read<int8>();

	_updateVolatileData(eid, xz.X, KBE_FLT_MAX, xz.Z, y, KBE_FLT_MAX, r, 1);
}

void KBEngineApp::Client_onUpdateData_xz_pr(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);

	int8 p = stream.read<int8>();
	int8 r = stream.read<int8>();

	_updateVolatileData(eid, xz.X, KBE_FLT_MAX, xz.Z, KBE_FLT_MAX, p, r, 1);
}

void KBEngineApp::Client_onUpdateData_xz_y(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);

	int8 y = stream.read<int8>();

	_updateVolatileData(eid, xz.X, KBE_FLT_MAX, xz.Z, y, KBE_FLT_MAX, KBE_FLT_MAX, 1);
}

void KBEngineApp::Client_onUpdateData_xz_p(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);

	int8 p = stream.read<int8>();

	_updateVolatileData(eid, xz.X, KBE_FLT_MAX, xz.Z, KBE_FLT_MAX, p, KBE_FLT_MAX, 1);
}

void KBEngineApp::Client_onUpdateData_xz_r(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);

	int8 r = stream.read<int8>();

	_updateVolatileData(eid, xz.X, KBE_FLT_MAX, xz.Z, KBE_FLT_MAX, KBE_FLT_MAX, r, 1);
}

void KBEngineApp::Client_onUpdateData_xyz(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);
	stream.readPackY(xz.Y);

	_updateVolatileData(eid, xz.X, xz.Y, xz.Z, KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX, 0);
}

void KBEngineApp::Client_onUpdateData_xyz_ypr(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);
	stream.readPackY(xz.Y);

	int8 y = stream.read<int8>();
	int8 p = stream.read<int8>();
	int8 r = stream.read<int8>();

	_updateVolatileData(eid, xz.X, xz.Y, xz.Z, y, p, r, 0);
}

void KBEngineApp::Client_onUpdateData_xyz_yp(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);
	stream.readPackY(xz.Y);

	int8 y = stream.read<int8>();
	int8 p = stream.read<int8>();

	_updateVolatileData(eid, xz.X, xz.Y, xz.Z, y, p, KBE_FLT_MAX, 0);
}

void KBEngineApp::Client_onUpdateData_xyz_yr(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);
	stream.readPackY(xz.Y);

	int8 y = stream.read<int8>();
	int8 r = stream.read<int8>();

	_updateVolatileData(eid, xz.X, xz.Y, xz.Z, y, KBE_FLT_MAX, r, 0);
}

void KBEngineApp::Client_onUpdateData_xyz_pr(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);
	stream.readPackY(xz.Y);

	int8 p = stream.read<int8>();
	int8 r = stream.read<int8>();

	_updateVolatileData(eid, xz.X, xz.Y, xz.Z, KBE_FLT_MAX, p, r, 0);
}

void KBEngineApp::Client_onUpdateData_xyz_y(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);
	stream.readPackY(xz.Y);

	int8 y = stream.read<int8>();

	_updateVolatileData(eid, xz.X, xz.Y, xz.Z, y, KBE_FLT_MAX, KBE_FLT_MAX, 0);
}

void KBEngineApp::Client_onUpdateData_xyz_p(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);
	stream.readPackY(xz.Y);

	int8 p = stream.read<int8>();

	_updateVolatileData(eid, xz.X, xz.Y, xz.Z, KBE_FLT_MAX, p, KBE_FLT_MAX, 0);
}

void KBEngineApp::Client_onUpdateData_xyz_r(MemoryStream& stream)
{
	ENTITY_ID eid = getViewEntityIDFromStream(stream);

	FVector xz;
	stream.readPackXZ(xz.X, xz.Z);
	stream.readPackY(xz.Y);

	int8 r = stream.read<int8>();

	_updateVolatileData(eid, xz.X, xz.Y, xz.Z, KBE_FLT_MAX, KBE_FLT_MAX, r, 0);
}

void KBEngineApp::_updateVolatileData(ENTITY_ID entityID, float x, float y, float z, float yaw, float pitch, float roll, int8 isOnGround)
{
	Entity** pEntityFind = entities_.Find(entityID);

	if (!pEntityFind)
	{
		// 如果为0且客户端上一步是重登陆或者重连操作并且服务端entity在断线期间一直处于在线状态
		// 则可以忽略这个错误, 因为cellapp可能一直在向baseapp发送同步消息， 当客户端重连上时未等
		// 服务端初始化步骤开始则收到同步信息, 此时这里就会出错。
		ERROR_MSG("KBEngineApp::_updateVolatileData(): entity(%d) not found!", entityID);
		return;
	}

	Entity& entity = *(*pEntityFind);

	// 小于0不设置
	if (isOnGround >= 0)
	{
		entity.isOnGround(isOnGround > 0);
	}

	bool changeDirection = false;

	if (roll != KBE_FLT_MAX)
	{
		changeDirection = true;
		entity.direction.X = int82angle((int8)roll, false);
	}

	if (pitch != KBE_FLT_MAX)
	{
		changeDirection = true;
		entity.direction.Y = int82angle((int8)pitch, false);
	}

	if (yaw != KBE_FLT_MAX)
	{
		changeDirection = true;
		entity.direction.Z = int82angle((int8)yaw, false);
	}

	bool done = false;
	if (changeDirection == true)
	{
		UKBEventData_set_direction* pEventData = NewObject<UKBEventData_set_direction>();
		pEventData->direction = entity.direction;
		pEventData->entityID = entity.id();
		KBENGINE_EVENT_FIRE("set_direction", pEventData);

		done = true;
	}

        bool positionChanged = x != KBE_FLT_MAX || y != KBE_FLT_MAX || z != KBE_FLT_MAX;
        if (x == KBE_FLT_MAX) x = 0.0f;
        if (y == KBE_FLT_MAX) y = 0.0f;
        if (z == KBE_FLT_MAX) z = 0.0f;
	            
	if (positionChanged)
	{
		entity.position = FVector(x + entityServerPos_.X, y + entityServerPos_.Y, z + entityServerPos_.Z);
		done = true;

		UKBEventData_updatePosition* pEventData = NewObject<UKBEventData_updatePosition>();
		KBPos2UE4Pos(pEventData->position, entity.position);
		pEventData->entityID = entity.id();
		pEventData->moveSpeed = entity.velocity();
		pEventData->isOnGround = entity.isOnGround();
		KBENGINE_EVENT_FIRE("updatePosition", pEventData);
	}

	if (done)
		entity.onUpdateVolatileData();
}