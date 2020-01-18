// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "http_utility.h"
#include "curl/curl.h"
#include "helper/debug_helper.h"
#include "common/memorystream.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"

namespace KBEngine 
{
namespace Network
{
namespace Http
{ 

static bool _g_init = false;
static Requests* g_pRequests = NULL;

//-------------------------------------------------------------------------------------
bool initialize()
{
	if (!_g_init)
	{
		_g_init = true;

		CURLcode curlCode = curl_global_init(CURL_GLOBAL_ALL);
		if (CURLE_OK != curlCode)
		{
			ERROR_MSG(fmt::format("Http::initialize: "
				"curl_global_init error! curlCode={}\n", curlCode));

			return false;
		}

		g_pRequests = new Requests();
	}

	return true;
}

//-------------------------------------------------------------------------------------
void finalise()
{
	SAFE_RELEASE(g_pRequests);

	if(_g_init)
		curl_global_cleanup();
}

//-------------------------------------------------------------------------------------
Request::Request():
	pContext_(NULL),
	headers_(NULL),
	postData_(NULL),
	httpCode_(0),
	hasSetRedirect_(false),
	retryTimes_(0),
	receivedContent_(),
	receivedHeader_(),
	resultCallback_(),
	called_(false),
	setVerifySSL_(false),
	userargs_(NULL),
	url_()
{
	error_[0] = '\0';

	pContext_ = (void*)curl_easy_init();
	curl_easy_setopt((CURL*)pContext_, CURLOPT_PRIVATE, (void*)this);

	curl_easy_setopt((CURL*)pContext_, CURLOPT_HEADERFUNCTION, Request::receiveHeaderFunction);
	curl_easy_setopt((CURL*)pContext_, CURLOPT_HEADERDATA, &receivedHeader_);

	curl_easy_setopt((CURL*)pContext_, CURLOPT_WRITEFUNCTION, Request::receiveContentFunction);
	curl_easy_setopt((CURL*)pContext_, CURLOPT_WRITEDATA, &receivedContent_);

	curl_easy_setopt((CURL*)pContext_, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt((CURL*)pContext_, CURLOPT_NOSIGNAL, 1);

	curl_easy_setopt((CURL*)pContext_, CURLOPT_CONNECTTIMEOUT_MS, 10000);
	setTimeout(10);

	KBE_ASSERT(sizeof(error_) >= CURL_ERROR_SIZE);
	curl_easy_setopt((CURL*)pContext_, CURLOPT_ERRORBUFFER, error_);

	/* abort if slower than 30 bytes/sec during 5 seconds */
	curl_easy_setopt((CURL*)pContext_, CURLOPT_LOW_SPEED_TIME, 5L);
	curl_easy_setopt((CURL*)pContext_, CURLOPT_LOW_SPEED_LIMIT, 30L);
}

//-------------------------------------------------------------------------------------
Request::~Request()
{
	if (!called_)
		callCallback(false);

	if(headers_)
		curl_slist_free_all((curl_slist *)headers_);

	if(pContext_)
		curl_easy_cleanup((CURL*)pContext_);

	if (postData_)
	{
		MemoryStream::reclaimPoolObject(postData_);
		postData_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
Request::Status Request::setCAInfo(const std::string& cainfo)
{
	CURLcode curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_CAINFO, cainfo.c_str());
	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setCAInfo: error! curlCode={}\n", curlCode));
		return INVALID_OPT;
	}

	return OK;
}

//-------------------------------------------------------------------------------------
Request::Status Request::setSSLCert(const std::string& sslCert)
{
	CURLcode curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_SSLCERT, sslCert.c_str());
	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setSSLCert: error! curlCode={}\n", curlCode));
		return INVALID_OPT;
	}

	return OK;
}

//-------------------------------------------------------------------------------------
Request::Status Request::setSSLKey(const std::string& sslKey)
{
	CURLcode curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_SSLKEY, sslKey.c_str());
	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setSSLKey: error! curlCode={}\n", curlCode));
		return INVALID_OPT;
	}

	return OK;
}

//-------------------------------------------------------------------------------------
Request::Status Request::setSSLKeyPassword(const std::string& sslKeyPwd)
{
	CURLcode curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_KEYPASSWD, sslKeyPwd.c_str());
	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setSSLKeyPassword: error! curlCode={}\n", curlCode));
		return INVALID_OPT;
	}

	return OK;
}

//-------------------------------------------------------------------------------------
Request::Status Request::setSSLVerifyPeer(long v)
{
	CURLcode curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_SSL_VERIFYPEER, v);
	setVerifySSL_ = true;

	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setSSLVerifyPeer: error! curlCode={}\n", curlCode));
		return INVALID_OPT;
	}

	return OK;
}

//-------------------------------------------------------------------------------------
Request::Status Request::setSSLVerifyHost(long v)
{
	CURLcode curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_SSL_VERIFYPEER, v);
	setVerifySSL_ = true;

	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setSSLVerifyHost: error! curlCode={}\n", curlCode));
		return INVALID_OPT;
	}

	return OK;
}

//-------------------------------------------------------------------------------------
Request::Status Request::setURL(const std::string& url)
{
	CURLcode curlCode = CURLE_OK;

	if (!setVerifySSL_/* 如果没设置过，自动设置一个默认值 */ && url.substr(0, 6) == "https:")
	{
		curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_SSL_VERIFYPEER, 0L);
		curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_SSL_VERIFYHOST, 0L);
	}

	curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_URL, url.c_str());
	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setURL: "
			"curl_easy_setopt(CURLOPT_URL) error! curlCode={}, url={}\n", curlCode, url));

		return INVALID_OPT;
	}

	url_ = url;
	return OK;
}

//-------------------------------------------------------------------------------------
Request::Status Request::setFollowURL(int maxRedirs)
{
	CURLcode curlCode = CURLE_OK;

	if (maxRedirs <= 0)
	{
		curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_FOLLOWLOCATION, 0L);
	}
	else
	{
		curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_MAXREDIRS, maxRedirs);
		if (CURLE_OK != curlCode)
		{
			ERROR_MSG(fmt::format("Http::Request::setFollowURL: "
				"curl_easy_setopt(CURLOPT_MAXREDIRS) error! curlCode={}, maxRedirs={}\n", curlCode, maxRedirs));

			return INVALID_OPT;
		}

		curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_FOLLOWLOCATION, 1L);
	}

	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setFollowURL: "
			"curl_easy_setopt(CURLOPT_FOLLOWLOCATION) error! curlCode={}, maxRedirs={}\n", curlCode, maxRedirs));

		return INVALID_OPT;
	}

	hasSetRedirect_ = true;
	return OK;
}

//-------------------------------------------------------------------------------------
Request::Status Request::setPostData(const void* data, unsigned int size)
{
	if (!data)
	{
		ERROR_MSG(fmt::format("Http::Request::setPostData: "
				"data is NULL! size={}\n", size));

		return INVALID_OPT;
	}

	CURLcode curlCode = CURLE_OK;

	curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_POST, 1);
	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setPostData: "
			"curl_easy_setopt(CURLOPT_POST) error! curlCode={}, size={}\n", curlCode, size));

		return INVALID_OPT;
	}

	if (size == 0)
	{
		curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_POSTFIELDS, "");
	}
	else
	{
		if (!postData_)
			postData_ = MemoryStream::createPoolObject(OBJECTPOOL_POINT);

		postData_->clear(false);
		postData_->append((uint8*)data, size);
		curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_POSTFIELDS, postData_->data());
	}

	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setPostData: "
			"curl_easy_setopt(CURLOPT_POSTFIELDS) error! curlCode={}, size={}\n", curlCode, size));

		return INVALID_OPT;
	}

	curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_POSTFIELDSIZE, size);
	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setPostData: "
			"curl_easy_setopt(CURLOPT_POSTFIELDSIZE) error! curlCode={}, size={}\n", curlCode, size));

		return INVALID_OPT;
	}

	return OK;
}

//-------------------------------------------------------------------------------------
Request::Status Request::setHeader(const std::string& header)
{
	if(headers_)
		curl_slist_append((curl_slist *)headers_, strutil::kbe_trim(header).c_str());
	else
		headers_ = curl_slist_append(NULL, strutil::kbe_trim(header).c_str());

	if (!headers_)
	{
		ERROR_MSG(fmt::format("Http::Request::setHeaders: "
			"curl_slist_append error!, header={}\n", header));

		return INVALID_OPT;
	}

	CURLcode curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_HTTPHEADER, (curl_slist *)headers_);

	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setHeader: "
			"curl_easy_setopt(CURLOPT_HTTPHEADER) error! curlCode={}, header={}\n", curlCode, header));

		return INVALID_OPT;
	}

	return OK;
}

//-------------------------------------------------------------------------------------
Request::Status Request::setHeader(const std::vector<std::string>& headers)
{
	if (headers.size() == 0)
	{
		ERROR_MSG(fmt::format("Http::Request::setHeaders: headers size is 0!\n"));
		return INVALID_OPT;
	}

	CURLcode curlCode = CURLE_OK;

	std::vector<std::string>::const_iterator iter = headers.begin();
	for (; iter != headers.end(); ++iter)
	{
		if (headers_)
			curl_slist_append((curl_slist *)headers_, strutil::kbe_trim((*iter)).c_str());
		else
			headers_ = curl_slist_append(NULL, strutil::kbe_trim((*iter)).c_str());

		if (!headers_)
		{
			ERROR_MSG(fmt::format("Http::Request::setHeaders: "
				"curl_slist_append error! curlCode={}, header={}\n", curlCode, (*iter)));

			return INVALID_OPT;
		}
	}

	curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_HTTPHEADER, (curl_slist *)headers_);

	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setHeaders: "
			"curl_easy_setopt(CURLOPT_HTTPHEADER) error! curlCode={}\n", curlCode));

		return INVALID_OPT;
	}

	return OK;
}

//-------------------------------------------------------------------------------------
Request::Status Request::setHeader(const std::map<std::string, std::string>& headers)
{
	if (headers.size() == 0)
	{
		ERROR_MSG(fmt::format("Http::Request::setHeaders: headers size is 0!\n"));
		return INVALID_OPT;
	}

	CURLcode curlCode = CURLE_OK;

	std::map<std::string, std::string>::const_iterator iter = headers.begin();
	for (; iter != headers.end(); ++iter)
	{
		std::string header = strutil::kbe_trim(iter->first);
		header += ": ";
		header += strutil::kbe_trim(iter->second);

		if (headers_)
			curl_slist_append((curl_slist *)headers_, header.c_str());
		else
			headers_ = curl_slist_append(NULL, header.c_str());

		if (!headers_)
		{
			ERROR_MSG(fmt::format("Http::Request::setHeaders: "
				"curl_slist_append error! curlCode={}, header={}\n", curlCode, header));

			return INVALID_OPT;
		}
	}

	curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_HTTPHEADER, (curl_slist *)headers_);

	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setHeaders: "
			"curl_easy_setopt(CURLOPT_HTTPHEADER) error! curlCode={}\n", curlCode));

		return INVALID_OPT;
	}

	return OK;
}

//-------------------------------------------------------------------------------------
Request::Status Request::setTimeout(uint32 secs)
{
	CURLcode curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_TIMEOUT, secs);
	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setTimeout: "
			"curl_easy_setopt(CURLOPT_TIMEOUT) error! curlCode={}, secs={}\n", curlCode, secs));

		return INVALID_OPT;
	}

	return OK;
}

//-------------------------------------------------------------------------------------
Request::Status Request::setProxy(const std::string& proxyIP, long proxyPort)
{
	CURLcode curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_PROXYPORT, proxyPort);
	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setProxy: "
			"curl_easy_setopt(CURLOPT_PROXYPORT) error! curlCode={}\n", curlCode));

		return INVALID_OPT;
	}

	curlCode = curl_easy_setopt((CURL*)pContext_, CURLOPT_PROXY, proxyIP.c_str());
	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::setProxy: "
			"curl_easy_setopt(CURLOPT_PROXYPORT) error! curlCode={}\n", curlCode));

		return INVALID_OPT;
	}

	return OK;
}

//-------------------------------------------------------------------------------------
size_t Request::receiveHeaderFunction(char *buffer, size_t size, size_t nitems, void *userdata)
{
	std::string* receiveData = reinterpret_cast<std::string*>(userdata);
	if (receiveData && buffer)
	{
		receiveData->append(reinterpret_cast<const char*>(buffer), size * nitems);
	}

	return nitems * size;
}

//-------------------------------------------------------------------------------------
size_t Request::receiveContentFunction(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	std::string* receiveData = reinterpret_cast<std::string*>(userdata);
	if (receiveData && ptr)
	{
		receiveData->append(reinterpret_cast<const char*>(ptr), size * nmemb);
	}

	return nmemb * size;
}

//-------------------------------------------------------------------------------------
Request::Status Request::perform()
{
	CURLcode curlCode = CURLE_OK;

	if (!hasSetRedirect_)
	{
		Request::Status status = setFollowURL(5);
		if (OK != status)
			return status;
	}

	receivedContent_.clear();
	receivedHeader_.clear();

	int retryTimes = retryTimes_;
	do
	{
		curlCode = curl_easy_perform((CURL*)pContext_);

		if (curlCode != CURLE_OPERATION_TIMEDOUT)
			break;

	} while (retryTimes-- > 0);

	callCallback(true);

	if (headers_)
		curl_slist_free_all((curl_slist *)headers_);

	headers_ = NULL;
	return OK;
}

//-------------------------------------------------------------------------------------
bool Request::updateHttpCode()
{
	CURLcode curlCode = curl_easy_getinfo((CURL*)pContext_, CURLINFO_RESPONSE_CODE, &httpCode_);
	if (CURLE_OK != curlCode)
	{
		ERROR_MSG(fmt::format("Http::Request::perform: "
			"curl_easy_getinfo(CURLINFO_RESPONSE_CODE) error! curlCode={}\n", curlCode));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Request::callCallback(bool success)
{
	called_ = true;
	updateHttpCode();

	if (strlen(getError()) > 0)
	{
		ERROR_MSG(fmt::format("Http::Request::perform: {}\n", getError()));
		success = false;
	}

	if (resultCallback_)
		resultCallback_((success && httpCode_ == 200), *this, receivedContent_);
}

//-------------------------------------------------------------------------------------
#define __case(code) case code: s = #code

/* Die if we get a bad CURLMcode somewhere */
static void mcode_or_die(const char *where, CURLMcode code)
{
	if (CURLM_OK != code) 
	{
		const char *s;
		switch (code) {
			__case(CURLM_BAD_HANDLE); break;
			__case(CURLM_BAD_EASY_HANDLE); break;
			__case(CURLM_OUT_OF_MEMORY); break;
			__case(CURLM_INTERNAL_ERROR); break;
			__case(CURLM_UNKNOWN_OPTION); break;
			__case(CURLM_LAST); break;
		default: s = "CURLM_unknown"; break;
			__case(CURLM_BAD_SOCKET);
			ERROR_MSG(fmt::format("curl-multi: {} returns {}.\n", where, s));
			/* ignore this error */
			return;
		}

		ERROR_MSG(fmt::format("curl-multi: {} returns {}!\n", where, s));
	}
}

/* Check for completed transfers, and remove their easy handles */
static void check_multi_info(Requests *g)
{
	char *eff_url;
	CURLMsg *msg;
	int msgs_left;
	CURL *easy;
	Request * pRequest = NULL;

	//DEBUG_MSG(fmt::format("check_multi_info: remaining {}.\n", g->still_running));

	while ((msg = curl_multi_info_read((CURLM*)g->pContext(), &msgs_left))) 
	{
		if (msg->msg == CURLMSG_DONE) 
		{
			easy = msg->easy_handle;

			curl_easy_getinfo(easy, CURLINFO_PRIVATE, &pRequest);
			curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &eff_url);

			//DEBUG_MSG(fmt::format("check_multi_info: DONE: {} => ({}) {}\n", eff_url, msg->data.result, pRequest->getError()));

			curl_multi_remove_handle((CURLM*)g->pContext(), easy);
			pRequest->callCallback(true);
			delete pRequest;
		}
	}
}

/* Information associated with a specific socket */
class SockInfo : public InputNotificationHandler, public OutputNotificationHandler
{
public:
	SockInfo()
	{
		pRequests = NULL;
		pRequest = NULL;
		sockfd = 0;
		still_running = 0;
		action = 0;
	}

	virtual ~SockInfo()
	{

	}

	virtual int handleInputNotification(int fd)
	{
		KBE_ASSERT(sockfd == fd);

		Requests* tRequests = pRequests;
		CURLMcode rc;

		rc = curl_multi_socket_action((CURLM*)tRequests->pContext(), fd, CURL_POLL_IN, &tRequests->still_running);

		mcode_or_die("handleInputNotification: curl_multi_socket_action", rc);

		check_multi_info(tRequests);

		if (tRequests->still_running <= 0)
		{
			if (tRequests->timerHandle.isSet())
				tRequests->timerHandle.cancel();
		}

		return 0;
	}

	virtual int handleOutputNotification(int fd)
	{
		KBE_ASSERT(sockfd == fd);

		Requests* tRequests = pRequests;
		CURLMcode rc;

		rc = curl_multi_socket_action((CURLM*)tRequests->pContext(), fd, CURL_POLL_OUT, &tRequests->still_running);

		mcode_or_die("handleOutputNotification: curl_multi_socket_action", rc);

		check_multi_info(tRequests);

		if (tRequests->still_running <= 0)
		{
			if (tRequests->timerHandle.isSet())
				tRequests->timerHandle.cancel();
		}

		return 0;
	}

public:
	Requests* pRequests;
	Request * pRequest;
	curl_socket_t sockfd;
	int still_running;
	int action;
};

/* Assign information to a SockInfo structure */
static void setsock(SockInfo *f, curl_socket_t s, CURL *e, int act, Requests *g)
{
	f->sockfd = s;

	if (f->action == 0)
		f->action = act;
	else if (f->action != act)
		f->action = CURL_POLL_INOUT;

	EventDispatcher& dispatcher = DebugHelper::getSingleton().pNetworkInterface()->dispatcher();

	if (act == CURL_POLL_IN) 
	{
		dispatcher.registerReadFileDescriptor(s, f);
	}
	else if (act == CURL_POLL_OUT) 
	{
		dispatcher.registerWriteFileDescriptor(s, f);
	}
	else if (act == CURL_POLL_INOUT) 
	{
		dispatcher.registerReadFileDescriptor(s, f);
		dispatcher.registerWriteFileDescriptor(s, f);
	}
}

/* Initialize a new SockInfo structure */
static void addsock(curl_socket_t s, CURL *easy, int action, Requests *g)
{
	SockInfo* fdp = new SockInfo();
	curl_easy_getinfo(easy, CURLINFO_PRIVATE, &fdp->pRequest);
	fdp->pRequests = g;

	setsock(fdp, s, easy, action, g);
	CURLMcode rc = curl_multi_assign((CURLM*)g->pContext(), s, fdp);
	mcode_or_die("addsock: curl_multi_add_handle", rc);
}

/* Clean up the SockInfo structure */
static void remsock(SockInfo *f, Requests *g)
{ 
	if (f) 
	{
		EventDispatcher& dispatcher = DebugHelper::getSingleton().pNetworkInterface()->dispatcher();

		if (f->action == CURL_POLL_IN)
		{
			dispatcher.deregisterReadFileDescriptor(f->sockfd);
		}
		else if (f->action == CURL_POLL_OUT)
		{
			dispatcher.deregisterWriteFileDescriptor(f->sockfd);
		}
		else if (f->action == CURL_POLL_INOUT)
		{
			dispatcher.deregisterReadFileDescriptor(f->sockfd);
			dispatcher.deregisterWriteFileDescriptor(f->sockfd);
		}

		delete f;
	}
}

static int multi_sock_cb(CURL* e, curl_socket_t s, int what, void* cbp, void* sockp)
{
	Requests *g = (Requests*)cbp;
	SockInfo *fdp = (SockInfo*)sockp;

	//const char *whatstr[] = { "none", "IN", "OUT", "INOUT", "REMOVE" };
	//DEBUG_MSG(fmt::format("multi_sock_cb: s={}, e={:p}, watch={}\n", s, (void*)e, whatstr[what]));

	if (what == CURL_POLL_REMOVE) 
	{
		remsock(fdp, g);
	}
	else 
	{
		if (!fdp) 
		{
			addsock(s, e, what, g);
		}
		else 
		{
			setsock(fdp, s, e, what, g);
		}
	}

	return 0;
}

static int multi_timer_cb(CURLM* multi, long timeout_ms, Requests *g)
{
	CURLMcode rc;

	/* TODO
	*
	* if timeout_ms is 0, call curl_multi_socket_action() at once!
	*
	* if timeout_ms is -1, just delete the timer
	*
	* for all other values of timeout_ms, this should set or *update*
	* the timer to the new value
	*/

	EventDispatcher &dispatcher = DebugHelper::getSingleton().pNetworkInterface()->dispatcher();
	
	g->timerHandle.cancel();

	if (timeout_ms == 0) 
	{
		rc = curl_multi_socket_action((CURLM*)g->pContext(), CURL_SOCKET_TIMEOUT, 0, &g->still_running);
		mcode_or_die("multi_timer_cb: curl_multi_socket_action", rc);
	}
	else if (timeout_ms > 0)
	{
		g->timerHandle = dispatcher.addTimer(timeout_ms * 1000, g);
	}

	return 0;
}

//-------------------------------------------------------------------------------------
Requests::Requests() :
	still_running(0),
	pRequest(NULL),
	timerHandle(),
	pContext_(NULL)
{
	pContext_ = (void*)curl_multi_init();

	curl_multi_setopt((CURLM*)pContext_, CURLMOPT_SOCKETFUNCTION, multi_sock_cb);
	curl_multi_setopt((CURLM*)pContext_, CURLMOPT_SOCKETDATA, this);
	curl_multi_setopt((CURLM*)pContext_, CURLMOPT_TIMERFUNCTION, multi_timer_cb);
	curl_multi_setopt((CURLM*)pContext_, CURLMOPT_TIMERDATA, this);
}

//-------------------------------------------------------------------------------------
Requests::~Requests()
{
	if (timerHandle.isSet())
		timerHandle.cancel();

	if (pContext_)
		curl_multi_cleanup((CURLM*)pContext_);
}

//-------------------------------------------------------------------------------------
Request::Status Requests::perform(Request* pRequest)
{
	CURLMcode rc = curl_multi_add_handle((CURLM*)pContext_, pRequest->pContext());
	mcode_or_die("Requests::perform: curl_multi_add_handle", rc);

	if (rc != CURLM_OK)
	{
		delete pRequest;
		return Request::INVALID_OPT;
	}

	return Request::OK;
}

//-------------------------------------------------------------------------------------
Request::Status Requests::perform(const std::string& url, const Request::Callback& resultCallback, 
	const std::map<std::string, std::string>& headers)
{
	Network::Http::Request* r = new Network::Http::Request();
	r->setURL(url);
	r->setCallback(resultCallback);

	if (headers.size() > 0)
		r->setHeader(headers);

	Request::Status status = perform(r);
	// 由curl的回调中销毁
	// delete r;
	return status;
}

//-------------------------------------------------------------------------------------
Request::Status Requests::perform(const std::string& url, const Request::Callback& resultCallback, const std::string& postData,
	const std::map<std::string, std::string>& headers)
{
	Network::Http::Request* r = new Network::Http::Request();
	r->setURL(url);
	r->setCallback(resultCallback);
	r->setPostData(postData.data(), postData.size());

	if (headers.size() > 0)
		r->setHeader(headers);

	Request::Status status = perform(r);
	// 由curl的回调中销毁
	// delete r;
	return status;
}

//-------------------------------------------------------------------------------------
void Requests::handleTimeout(TimerHandle, void * pUser)
{
	timerHandle.cancel();

	CURLMcode rc;

	rc = curl_multi_socket_action((CURLM*)pContext_, CURL_SOCKET_TIMEOUT, 0, &still_running);
	mcode_or_die("Requests::handleTimeout: curl_multi_socket_action", rc);
	check_multi_info(this);

	//DEBUG_MSG(fmt::format(" Requests::handleTimeout: still_running={}!\n", still_running));
}


//-------------------------------------------------------------------------------------
Request::Status perform(Request* pRequest)
{
	if (!_g_init)
		initialize();

	return g_pRequests->perform(pRequest);
}

//-------------------------------------------------------------------------------------
Request::Status perform(const std::string& url, const Request::Callback& resultCallback, const std::map<std::string, std::string>& headers)
{
	if (!_g_init)
		initialize();

	return g_pRequests->perform(url, resultCallback, headers);
}

//-------------------------------------------------------------------------------------
Request::Status perform(const std::string& url, const Request::Callback& resultCallback, const std::string& postData, const std::map<std::string, std::string>& headers)
{
	if (!_g_init)
		initialize();

	return g_pRequests->perform(url, resultCallback, postData, headers);
}

//-------------------------------------------------------------------------------------

}
}
}
