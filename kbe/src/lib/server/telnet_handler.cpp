// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "telnet_handler.h"
#include "telnet_server.h"
#include "network/bundle.h"
#include "network/endpoint.h"
#include "network/network_interface.h"
#include "pyscript/script.h"

#ifndef CODE_INLINE
#include "telnet_handler.inl"
#endif

namespace KBEngine { 

char _g_state_str[][256] = {
	"password",
	"root",
	"python",
	"readonly"
};

/*
	格式: echo "\033[字背景颜色;字体颜色m字符串\033[0m" 

	例如: 
	echo "\033[41;36m something here \033[0m"  

	其中41的位置代表底色, 36的位置是代表字的颜色 


	那些ascii code 是对颜色调用的始末.  
	\033[ ; m …… \033[0m  



	字背景颜色范围:40----49 
	40:黑 
	41:深红 
	42:绿 
	43:黄色 
	44:蓝色 
	45:紫色 
	46:深绿 
	47:白色 

	字颜色:30-----------39 
	30:黑 
	31:红 
	32:绿 
	33:黄 
	34:蓝色 
	35:紫色 
	36:深绿 
	37:白色 

	\33[0m 关闭所有属性  
	\33[1m 设置高亮度  
	\33[4m 下划线  
	\33[5m 闪烁  
	\33[7m 反显  
	\33[8m 消隐  
	\33[30m -- \33[37m 设置前景色  
	\33[40m -- \33[47m 设置背景色  
	\33[nA 光标上移n行  
	\33[nB 光标下移n行  
	\33[nC 光标右移n行  
	\33[nD 光标左移n行  
	\33[y;xH设置光标位置  
	\33[2J 清屏  
	\33[K 清除从光标到行尾的内容  
	\33[s 保存光标位置  
	\33[u 恢复光标位置  
	\33[?25l 隐藏光标  
	\33[?25h 显示光标  

	使用格式能更复杂： 
	^[[..m;..m;..m;..m
	例如： \033[2;7;1m高亮\033[2;7;0m
*/

#define TELNET_CMD_LEFT							"\033[D"			// 左
#define TELNET_CMD_RIGHT						"\033[C"			// 右
#define TELNET_CMD_UP							"\033[A"			// 上
#define TELNET_CMD_DOWN							"\033[B"			// 下
#define TELNET_CMD_HOME							"\033[1~"			// 移到行首
#define TELNET_CMD_END							"\033[4~"			// 移到行尾

#define TELNET_CMD_DEL							"\033[K"			// 删除字符
#define TELNET_CMD_NEWLINE						"\r\n"				// 新行
#define TELNET_CMD_MOVE_FOCUS_LEFT_MAX			"\33[9999999999D"	// 左移光标到最前面
#define TELNET_CMD_MOVE_FOCUS_RIGHT_MAX			"\33[9999999999C"	// 右移光标到最后面

#define IAC_TERMIAL_TYPE_ANSI					"ANSI"
#define IAC_TERMIAL_TYPE_VT100					"VT100"
#define IAC_TERMIAL_TYPE_XTERM					"XTERM"

#define XTERM_DEL								"\033[3~"

const int TERMINAL_ANSI							= 1;
const int TERMINAL_VT100						= 2;
const int TERMINAL_XTERM						= 3;

//telnet协议
const unsigned char IAC							= 255;					//数据字节255
const unsigned char DONT						= 254;					//选项协商，发送方想让接收端去禁止选项
const unsigned char DO							= 253;					//选项协商,发送方想叫接收端激活选项
const unsigned char WONT						= 252;					//选项协商,发送方本身想禁止选项
const unsigned char WILL						= 251;					//选项协商,发送方本身将激活选项
const unsigned char SB							= 250;					//子选项开始
const unsigned char SE							= 240;					//子选项结束
const unsigned char ECHO						= 1;					//回显
const unsigned char SUPPRESS_GO_AHEAD			= 3;					//抑制继续进行
const unsigned char TT							= 24;					//终端类型

//-------------------------------------------------------------------------------------
TelnetHandler::TelnetHandler(Network::EndPoint* pEndPoint, TelnetServer* pTelnetServer, Network::NetworkInterface* pNetworkInterface, TELNET_STATE defstate):
historyCommand_(),
historyCommandIndex_(0),
command_(),
pEndPoint_(pEndPoint),
pTelnetServer_(pTelnetServer),
state_(defstate),
currPos_(0),
pProfileHandler_(NULL),
pNetworkInterface_(pNetworkInterface),
getingHistroyCmd_(false),
clientTermialType_(0)
{
}

//-------------------------------------------------------------------------------------
TelnetHandler::~TelnetHandler(void)
{
	if(pProfileHandler_) {
		pProfileHandler_->destroy();
		pProfileHandler_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
std::string TelnetHandler::getInputStartString()
{
	return fmt::format("[{}@{} ~]{} ", COMPONENT_NAME_EX(g_componentType), 
		_g_state_str[(int)state_], (state_ == TELNET_STATE_PYTHON ? " >>>" : "#"));
}

//-------------------------------------------------------------------------------------
std::string TelnetHandler::getWelcome()
{
	return fmt::format("\033[1;32mwelcome to {} \r\n"
			"Version: {}. "
			"ScriptVersion: {}. "
			"Config: {}. "
			"Built: {} {}. "
			"AppID: {}. "
			"UID: {}. "
			"PID: {}"
			"\r\n---------------------------------------------"
			"{}"
			"\r\n---------------------------------------------"
			" \033[0m\r\n{}",
		COMPONENT_NAME_EX(g_componentType), KBEVersion::versionString(), KBEVersion::scriptVersionString(),
		KBE_CONFIG, __TIME__, __DATE__,
		g_componentID, getUserUID(), getProcessPID(), help(), getInputStartString());
}

//-------------------------------------------------------------------------------------
std::string TelnetHandler::help()
{
	return 	"\033[1;32m\r\nCommand List:"
		"\r\n[:help          ]: list commands."
		"\r\n[:quit          ]: quit the server."
		"\r\n[:python        ]: python console."
		"\r\n[:root          ]: return to the root layer."
		"\r\n[:cprofile      ]: collects and reports the internal c++ profiles \r\n\t\tof a server process over a period of time."
		"\r\n\t\t usage: \":cprofile 30\""
		"\r\n[:pyprofile     ]: collects and reports the python profiles \r\n\t\tof a server process over a period of time."
		"\r\n\t\t usage: \":pyprofile 30\""
		"\r\n[:eventprofile  ]: a server process over a period of time, \r\n\t\tcollects and reports the all non-volatile cummunication \r\n\t\tdown to the client."
		"\r\n\t\t usage: \":eventprofile 30\""
		"\r\n[:networkprofile]: collects and reports the network profiles \r\n\t\tof a server process over a period of time."
		"\r\n\t\t usage: \":networkprofile 30\""
		"\r\n\r\n\033[0m";
};

//-------------------------------------------------------------------------------------
void TelnetHandler::historyCommandCheck()
{
	if(historyCommand_.size() > 50)
		historyCommand_.pop_front();

	if(historyCommandIndex_ < 0)
		historyCommandIndex_ = historyCommand_.size() - 1;

	if(historyCommandIndex_ > (int)historyCommand_.size() - 1)
		historyCommandIndex_ = 0; 
}

//-------------------------------------------------------------------------------------
std::string TelnetHandler::getHistoryCommand(bool isNextCommand)
{
	if(isNextCommand)
		historyCommandIndex_++;
	else
		historyCommandIndex_--;

	historyCommandCheck();

	if(historyCommand_.size() == 0)
		return "";

	return historyCommand_[historyCommandIndex_];
}

//-------------------------------------------------------------------------------------
Network::Reason TelnetHandler::checkLastErrors()
{
	int err;
	Network::Reason reason;

#if KBE_PLATFORM == PLATFORM_UNIX
	err = errno;

	switch (err)
	{
	case ECONNREFUSED:	reason = Network::REASON_NO_SUCH_PORT; break;
	case EAGAIN:		reason = Network::REASON_RESOURCE_UNAVAILABLE; break;
	case EPIPE:			reason = Network::REASON_CLIENT_DISCONNECTED; break;
	case ECONNRESET:	reason = Network::REASON_CLIENT_DISCONNECTED; break;
	case ENOBUFS:		reason = Network::REASON_TRANSMIT_QUEUE_FULL; break;
	default:			reason = Network::REASON_GENERAL_NETWORK; break;
	}
#else
	err = WSAGetLastError();

	if (err == WSAEWOULDBLOCK || err == WSAEINTR)
	{
		reason = Network::REASON_RESOURCE_UNAVAILABLE;
	}
	else
	{
		switch (err)
		{
		case WSAECONNREFUSED:   reason = Network::REASON_NO_SUCH_PORT; break;
		case WSAECONNRESET:     reason = Network::REASON_CLIENT_DISCONNECTED; break;
		case WSAECONNABORTED:   reason = Network::REASON_CLIENT_DISCONNECTED; break;
		default:                reason = Network::REASON_GENERAL_NETWORK; break;
		}
	}
#endif

	return reason;
}

//-------------------------------------------------------------------------------------
int	TelnetHandler::handleInputNotification(int fd)
{
	KBE_ASSERT((*pEndPoint_) == fd);

	char data[1024] = {0};
	int recvsize = pEndPoint_->recv(data, sizeof(data));

	if(recvsize == -1)
	{
		Network::Reason err = checkLastErrors();
		if (err != Network::REASON_RESOURCE_UNAVAILABLE)
			pTelnetServer_->onTelnetHandlerClosed(fd, this);
		return 0;

	}
	else if(recvsize == 0)
	{
		pTelnetServer_->onTelnetHandlerClosed(fd, this);
		return 0;
	}
	
	if(state_ == TELNET_STATE_READONLY)
		return 0;
	
	onRecvInput(data, recvsize);
	return 0;
}

//-------------------------------------------------------------------------------------
void TelnetHandler::onRecvInput(const char *buffer, int size)
{
	int idx = 0;
	while (idx < size)
	{
		char c = buffer[idx++];

		switch (c)
		{
		case '\r':
		{
			getingHistroyCmd_ = false;
			bool isEnter = false;

			if (size == 1)
			{
				isEnter = true;
			}
			else if (idx < size)
			{
				int cc = buffer[idx++];
				if (cc == '\n' || cc == '\0')
				{
					isEnter = true;
				}
			}

			if (isEnter && !processCommand())
			{
				return;
			}

			break;
		}
		case 8:		// 退格
		case 0x7f: // delete
		{
			processBackSpace();
			break;
		}
		case 27:	// vt100命令码: 0x1b
		{
			std::string s = "";
			std::string vt100cmd(s + c);
			bool shouldBeContinue = true;

			while (shouldBeContinue && idx < size)
			{
				if (buffer[idx] == '\r')
					break;

				c = buffer[idx++];
				vt100cmd.append(s + c);
				switch (c)
				{
				case 'A': // 光标上移n行
				case 'B': // 光标下移n行
				case 'C': // 光标右移n列
				case 'D': // 光标左移n列
				case '~': // home、end等
					shouldBeContinue = false;
					break;

				case 'm': // 颜色等属性或命令
				case 'J': // 清屏
				case 'K': // 清除从光标到行尾的内容
				case 's': // 保存光标位置
				case 'u': // 恢复光标位置
				case 'l': // 隐藏光标
				case 'h': // 显示光标
				case 'H': // 设置光标位置
				default:
					break;
				}
			}

			if (!checkUDLR(vt100cmd))
			{
				// 把不认识的命令原样输出,但会把命令符改成“^”
				// 以避免客户端触发命令操作
				vt100cmd[0] = '^';
				command_.insert(currPos_, vt100cmd);
				currPos_ += vt100cmd.length();

				if (currPos_ == (int32)command_.length())
				{
					pEndPoint_->send(vt100cmd.c_str(), vt100cmd.size());
				}
				else
				{
					std::string s = command_.substr(currPos_ - vt100cmd.length(), command_.size() - currPos_ + vt100cmd.length());
					s += fmt::format("\33[{}D", command_.size() - currPos_);
					pEndPoint_->send(s.c_str(), s.size());
				}
			}
			break;
		}
		case -1: //iac命令,telnet协议
		{
			std::string iaccmd(1, c);
			while (idx < size)
			{
				c = buffer[idx++];
				iaccmd.append(1, c);
			}

			checkTerminalType(iaccmd);
			break;
		}
		default:
			{
				std::string s = "";
				s += c;

				//作回显
				if (state_ != TELNET_STATE_PASSWD)
				{
					pEndPoint_->send(s.c_str(), s.size());
				}
				
				command_.insert(currPos_, s);
				currPos_++;
				checkAfterStr();
				break;
			}
		};
	}
}

//-------------------------------------------------------------------------------------
void TelnetHandler::checkAfterStr()
{
	if(currPos_ != (int32)command_.size())
	{
		std::string s = "";
		s = command_.substr(currPos_, command_.size() - currPos_);
		s += fmt::format("\33[{}D", s.size());
		pEndPoint_->send(s.c_str(), s.size());
	}
}

//-------------------------------------------------------------------------------------
bool TelnetHandler::checkUDLR(const std::string &cmd)
{
	if (cmd.find(TELNET_CMD_UP) != std::string::npos)		// 上 
	{
		pEndPoint_->send(TELNET_CMD_MOVE_FOCUS_LEFT_MAX, strlen(TELNET_CMD_MOVE_FOCUS_LEFT_MAX));
		sendDelChar();
		std::string startstr = getInputStartString();
		pEndPoint_->send(startstr.c_str(), startstr.size());
		resetStartPosition();

		if(!getingHistroyCmd_)
		{
			++historyCommandIndex_;
			getingHistroyCmd_ = true;
		}

		std::string s = getHistoryCommand(false);
		pEndPoint_->send(s.c_str(), s.size());
		command_ = s;
		currPos_ = s.size();
		return true;
	}
	else if (cmd.find(TELNET_CMD_DOWN) != std::string::npos)	// 下
	{
		pEndPoint_->send(TELNET_CMD_MOVE_FOCUS_LEFT_MAX, strlen(TELNET_CMD_MOVE_FOCUS_LEFT_MAX));
		sendDelChar();
		std::string startstr = getInputStartString();
		pEndPoint_->send(startstr.c_str(), startstr.size());
		resetStartPosition();

		if(!getingHistroyCmd_)
		{
			--historyCommandIndex_;
			getingHistroyCmd_ = true;
		}

		std::string s = getHistoryCommand(true);
		pEndPoint_->send(s.c_str(), s.size());
		command_ = s;
		currPos_ = s.size();
		return true;
	}
	else if (cmd.find(TELNET_CMD_RIGHT) != std::string::npos)	// 右
	{
		int cmdlen = strlen(TELNET_CMD_RIGHT);
		if(currPos_ < (int)command_.size())
		{
			currPos_++;
			pEndPoint_->send(TELNET_CMD_RIGHT, cmdlen);
		}
		return true;
	}
	else if (cmd.find(TELNET_CMD_LEFT) != std::string::npos)	// 左 
	{
		int cmdlen = strlen(TELNET_CMD_LEFT);
		if(currPos_ > 0)
		{
			currPos_--;
			pEndPoint_->send(TELNET_CMD_LEFT, cmdlen);
		}
		return true;
	}
	else if (cmd.find(TELNET_CMD_HOME) != std::string::npos)	// 移动到行首
	{
		if (currPos_ > 0)
		{
			std::string cmdstr = fmt::format("\033[{}D", currPos_);
			pEndPoint_->send(cmdstr.c_str(), cmdstr.length());
			currPos_ = 0;
		}
		return true;
	}
	else if (cmd.find(TELNET_CMD_END) != std::string::npos)	    // 移动到行尾
	{
		if (currPos_ != (int32)command_.length())
		{
			std::string cmdstr = fmt::format("\033[{}C", command_.length() - currPos_);
			pEndPoint_->send(cmdstr.c_str(), cmdstr.length());
			currPos_ = command_.length();
		}
		return true;
	}
	else if ( clientTermialType_ == TERMINAL_XTERM && cmd.find(XTERM_DEL) != std::string::npos)
	{
		processBackSpace();
		return true;
	}

	return false;
}

void TelnetHandler::checkTerminalType(std::string &iac)
{
	std::transform(iac.begin(), iac.end(), iac.begin(), toupper);

	if (iac.find(IAC_TERMIAL_TYPE_ANSI) != std::string::npos)
	{
		clientTermialType_ = TERMINAL_ANSI;
	}
	else if (iac.find(IAC_TERMIAL_TYPE_VT100) != std::string::npos)
	{
		clientTermialType_ = TERMINAL_VT100;
	}
	else if (iac.find(IAC_TERMIAL_TYPE_XTERM) != std::string::npos)
	{
		clientTermialType_ = TERMINAL_XTERM;
	}
}

//-------------------------------------------------------------------------------------
bool TelnetHandler::processCommand()
{
	if(command_.size() == 0)
	{
		sendEnter();
		sendNewLine();
		return true;
	}

	if(state_ == TELNET_STATE_PASSWD)
	{
		if(command_ == pTelnetServer_->passwd())
		{
			state_ = (TELNET_STATE)pTelnetServer_->deflayer();
			
			std::string s = getWelcome();
			pEndPoint_->send(s.c_str(), s.size());
			command_ = "";

			sendEnter();
			sendNewLine();

			sendServerTerminalType();
			sendWillSuppressGoAhead();
			sendWillEcho();
			sendDOTT();
			sendQueryClientTerminalType();

			return true;
		}
		else
		{
			command_ = "";
			sendEnter();
			sendNewLine();
			return true;
		}
	}

	bool logcmd = true;
	//for(int i=0; i<(int)historyCommand_.size(); ++i)
	{
		//if(historyCommand_[i] == command_)
		//{
		//	logcmd = false;
		//	break;
		//}
	}

	if(logcmd)
	{
		historyCommand_.push_back(command_);
		historyCommandCheck();
		historyCommandIndex_ = historyCommand_.size() - 1;
	}

	std::string cmd = command_;
	command_ = "";

	if(cmd == ":python")
	{
		if(pTelnetServer_->pScript() == NULL)
			return true;

		state_ = TELNET_STATE_PYTHON;
		sendEnter();
		sendNewLine();
		return true;
	}
	else if(cmd == ":help")
	{
		std::string str = help();
		pEndPoint_->send(str.c_str(), str.size());
		sendNewLine();
		return true;
	}
	else if(cmd == ":root")
	{
		state_ = TELNET_STATE_ROOT;
		sendEnter();
		sendNewLine();
		return true;
	}
	else if(cmd == ":quit")
	{
		state_ = TELNET_STATE_QUIT;
		
		pTelnetServer_->closeHandler((*pEndPoint_), this);
		return false;
	}
	else if(cmd.find(":cprofile") == 0)
	{
		uint32 timelen = 10;
		
		cmd.erase(cmd.find(":cprofile"), strlen(":cprofile"));
		if(cmd.size() > 0)
		{
			try
			{
				KBEngine::StringConv::str2value(timelen, cmd.c_str());
			}
			catch(...)  
			{
				timelen = 10;
			}

			if(timelen < 1 || timelen > 999999999)
				timelen = 10;
		}
		std::string str = fmt::format("\r\nWaiting for {} secs.\r\n", timelen);
		pEndPoint_->send(str.c_str(), str.size());
		
		std::string profileName = KBEngine::StringConv::val2str(KBEngine::genUUID64());

		if(pProfileHandler_) pProfileHandler_->destroy();
		pProfileHandler_ = new TelnetCProfileHandler(this, *pTelnetServer_->pNetworkInterface(), 
			timelen, profileName, pEndPoint_->addr());

		readonly();
		return false;
	}
	else if(cmd.find(":pyprofile") == 0)
	{
		uint32 timelen = 10;

		cmd.erase(cmd.find(":pyprofile"), strlen(":pyprofile"));
		if(cmd.size() > 0)
		{
			try
			{
				KBEngine::StringConv::str2value(timelen, cmd.c_str());
			}
			catch(...)  
			{
				timelen = 10;
			}

			if(timelen < 1 || timelen > 999999999)
				timelen = 10;
		}
		std::string str = fmt::format("\r\nWaiting for {} secs.\r\n", timelen);
		pEndPoint_->send(str.c_str(), str.size());

		std::string profileName = KBEngine::StringConv::val2str(KBEngine::genUUID64());

		if(pProfileHandler_) pProfileHandler_->destroy();
		pProfileHandler_ = new TelnetPyProfileHandler(this, *pTelnetServer_->pNetworkInterface(), 
			timelen, profileName, pEndPoint_->addr());

		readonly();
		return false;
	}
	else if (cmd.find(":pytickprofile") == 0)
	{
		uint32 timelen = 10;

		cmd.erase(cmd.find(":pytickprofile"), strlen(":pytickprofile"));
		if (cmd.size() > 0)
		{
			try
			{
				KBEngine::StringConv::str2value(timelen, cmd.c_str());
			}
			catch (...)
			{
				timelen = 10;
			}

			if (timelen < 1 || timelen > 999999999)
				timelen = 10;
		}

		std::string profileName = KBEngine::StringConv::val2str(KBEngine::genUUID64());

		if (pProfileHandler_) pProfileHandler_->destroy();
		pProfileHandler_ = new TelnetPyTickProfileHandler(this, *pTelnetServer_->pNetworkInterface(),
			timelen, profileName, pEndPoint_->addr());

		readonly();
		return false;
	}
	else if (cmd.find(":eventprofile") == 0)
	{
		uint32 timelen = 10;

		cmd.erase(cmd.find(":eventprofile"), strlen(":eventprofile"));
		if(cmd.size() > 0)
		{
			try
			{
				KBEngine::StringConv::str2value(timelen, cmd.c_str());
			}
			catch(...)  
			{
				timelen = 10;
			}

			if(timelen < 1 || timelen > 999999999)
				timelen = 10;
		}
		std::string str = fmt::format("\r\nWaiting for {} secs.\r\n", timelen);
		pEndPoint_->send(str.c_str(), str.size());

		std::string profileName = KBEngine::StringConv::val2str(KBEngine::genUUID64());

		if(pProfileHandler_) pProfileHandler_->destroy();
		pProfileHandler_ = new TelnetEventProfileHandler(this, *pTelnetServer_->pNetworkInterface(), 
			timelen, profileName, pEndPoint_->addr());

		readonly();
		return false;
	}
	else if(cmd.find(":networkprofile") == 0)
	{
		uint32 timelen = 10;

		cmd.erase(cmd.find(":networkprofile"), strlen(":networkprofile"));
		if(cmd.size() > 0)
		{
			try
			{
				KBEngine::StringConv::str2value(timelen, cmd.c_str());
			}
			catch(...)  
			{
				timelen = 10;
			}

			if(timelen < 1 || timelen > 999999999)
				timelen = 10;
		}

		std::string str = fmt::format("\r\nWaiting for {} secs.\r\n", timelen);
		pEndPoint_->send(str.c_str(), str.size());

		std::string profileName = KBEngine::StringConv::val2str(KBEngine::genUUID64());

		if(pProfileHandler_) pProfileHandler_->destroy();
		pProfileHandler_ = new TelnetNetworkProfileHandler(this, *pTelnetServer_->pNetworkInterface(), 
			timelen, profileName, pEndPoint_->addr());

		readonly();
		return false;
	}

	if(state_ == TELNET_STATE_PYTHON)
	{
		processPythonCommand(cmd);
	}
	
	sendEnter();
	sendNewLine();
	return true;
}

//-------------------------------------------------------------------------------------
void TelnetHandler::processPythonCommand(std::string command)
{
	if(pTelnetServer_->pScript() == NULL || command.size() == 0)
		return;
	
	command += "\n";
	PyObject* pycmd = PyUnicode_DecodeUTF8(command.data(), command.size(), NULL);
	if(pycmd == NULL)
	{
		SCRIPT_ERROR_CHECK();
		return;
	}

	DEBUG_MSG(fmt::format("TelnetHandler::processPythonCommand: size({}), command={}.\n", 
		command.size(), command));

	std::string retbuf = "";
	PyObject* pycmd1 = PyUnicode_AsEncodedString(pycmd, "utf-8", NULL);

	pTelnetServer_->pScript()->run_simpleString(PyBytes_AsString(pycmd1), &retbuf);

	// 把返回值中的'\n'替換成'\r\n'，以解决在vt100终端中显示不正确的问题
	std::string::size_type pos = 0;
	while ((pos = retbuf.find('\n', pos)) != std::string::npos)
	{
		if (retbuf[pos - 1] != '\r')
		{
			retbuf.insert(pos, "\r");
			pos++;
		}
		pos++;
	}
	
	if(retbuf.size() > 0)
	{
		// 将结果返回给客户端
		retbuf.insert(0, "\r\n");
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle) << retbuf;
		pEndPoint_->send(pBundle);
		Network::Bundle::reclaimPoolObject(pBundle);
	}

	Py_DECREF(pycmd);
	Py_DECREF(pycmd1);
}

void TelnetHandler::processBackSpace()
{
	if (currPos_ == 0)
	{
		resetStartPosition();
	}
	else
	{
		sendBackSpace();
		checkAfterStr();
	}
}

//-------------------------------------------------------------------------------------
void TelnetHandler::sendEnter()
{
	pEndPoint_->send(TELNET_CMD_NEWLINE, strlen(TELNET_CMD_NEWLINE));
}

//-------------------------------------------------------------------------------------
void TelnetHandler::sendDelChar()
{
	if(command_.size() > 0)
	{
		if (currPos_ > 0)
		{
			command_.erase(currPos_ - 1, 1);
			currPos_--;
			pEndPoint_->send(TELNET_CMD_DEL, strlen(TELNET_CMD_DEL));
		}
	}
}

//-------------------------------------------------------------------------------------
void TelnetHandler::sendNewLine()
{
	std::string startstr = getInputStartString();
	pEndPoint_->send(startstr.c_str(), startstr.size());
	resetStartPosition();
	currPos_ = 0;
}

void TelnetHandler::sendWillSuppressGoAhead()
{
	unsigned char cmd[] = { IAC, WILL, SUPPRESS_GO_AHEAD, '\0' };
	pEndPoint_->send(cmd, sizeof(cmd));
}

void TelnetHandler::sendDOTT()
{
	unsigned char cmd[] = { IAC, DO, TT, '\0' };
	pEndPoint_->send(cmd, sizeof(cmd));
}

void TelnetHandler::sendQueryClientTerminalType()
{
	unsigned char cmd[] = { IAC, SB, TT, 1, IAC, SE, '\0' };
	pEndPoint_->send(cmd, sizeof(cmd));
}

void TelnetHandler::sendServerTerminalType()
{
	unsigned char cmd[] = { IAC, SB, TT, 0, 'v', 't', '1', '0', '0', IAC, SE, '\0' };
	pEndPoint_->send(cmd, sizeof(cmd));
}

void TelnetHandler::sendWillEcho()
{
	unsigned char cmd[] = { IAC, WILL, ECHO, '\0' };
	pEndPoint_->send(cmd, sizeof(cmd));
}

void TelnetHandler::sendBackSpace()
{
	if (command_.size() > 0)
	{
		if (currPos_ > 0)
		{
			command_.erase(currPos_ - 1, 1);
			currPos_--;

			char cmd[] = { 8, '\0' };
			pEndPoint_->send(cmd, sizeof(cmd));
			pEndPoint_->send(TELNET_CMD_DEL, strlen(TELNET_CMD_DEL));
		}
	}
}

//-------------------------------------------------------------------------------------
void TelnetHandler::resetStartPosition()
{
	pEndPoint_->send(TELNET_CMD_MOVE_FOCUS_LEFT_MAX, strlen(TELNET_CMD_MOVE_FOCUS_LEFT_MAX));
	std::string startstr = getInputStartString();
	std::string backcmd = fmt::format("\33[{}C", startstr.size());
	pEndPoint_->send(backcmd.c_str(), backcmd.size());
}

//-------------------------------------------------------------------------------------
void TelnetHandler::setReadWrite()
{
	state_ = TELNET_STATE_ROOT;
}

//-------------------------------------------------------------------------------------
void TelnetHandler::readonly()
{
	state_ = TELNET_STATE_READONLY;
}

//-------------------------------------------------------------------------------------
void TelnetHandler::onProfileEnd(const std::string& datas)
{
	if (datas.size() > 0)
	{
		sendEnter();
		pEndPoint()->send(datas.c_str(), datas.size());
	}
	setReadWrite();
	sendEnter();
	sendNewLine();
	pProfileHandler_ = NULL;
}

//-------------------------------------------------------------------------------------
void TelnetPyProfileHandler::sendStream(MemoryStream* s)
{
	if(isDestroyed_) return;

	std::string datas;
	(*s) >> datas;

	// 把返回值中的'\n'替換成'\r\n'，以解决在vt100终端中显示不正确的问题
	std::string::size_type pos = 0;
	while ((pos = datas.find('\n', pos)) != std::string::npos)
	{
		if (datas[pos - 1] != '\r')
		{
			datas.insert(pos, "\r");
			pos++;
		}
		pos++;
	}

	pTelnetHandler_->onProfileEnd(datas);
}

//-------------------------------------------------------------------------------------
void TelnetPyTickProfileHandler::sendStream(MemoryStream* s)
{
	if (isDestroyed_ || !pTelnetHandler_)
		return;

	std::string datas;
	(*s) >> datas;

	// 把返回值中的'\n'替換成'\r\n'，以解决在vt100终端中显示不正确的问题
	std::string::size_type pos = 0;
	while ((pos = datas.find('\n', pos)) != std::string::npos)
	{
		if (datas[pos - 1] != '\r')
		{
			datas.insert(pos, "\r");
			pos++;
		}
		pos++;
	}

	pTelnetHandler_->sendEnter();
	pTelnetHandler_->pEndPoint()->send(datas.c_str(), datas.size());
	//pTelnetHandler_->onProfileEnd(datas);
}

//-------------------------------------------------------------------------------------
void TelnetPyTickProfileHandler::timeout()
{
	PyTickProfileHandler::timeout();

	if (isDestroyed_ || !pTelnetHandler_)
		return;

	pTelnetHandler_->onProfileEnd("");
}

//-------------------------------------------------------------------------------------
void TelnetPyTickProfileHandler::destroy()
{
	TelnetProfileHandler::destroy();
	pTelnetHandler_ = NULL;
}

//-------------------------------------------------------------------------------------
void TelnetCProfileHandler::sendStream(MemoryStream* s)
{
	if(isDestroyed_) return;
	
	std::string datas;
	uint32 timinglen;
	ArraySize size;

	(*s) >> timinglen >> size;

	datas = "ncalls\ttottime\tpercall\tcumtime\tpercall\tfilename:lineno(function)\r\n";

	while(size-- > 0)
	{
		uint32 count;
		float lastTime;
		float sumTime;
		float lastIntTime;
		float sumIntTime;
		std::string name;

		(*s) >> name >> count >> lastTime >> sumTime >> lastIntTime >> sumIntTime;

		char buf[256];
		kbe_snprintf(buf, 256, "%u", count);
		datas += buf;
		datas += "\t";

		kbe_snprintf(buf, 256, "%.3f", sumTime);
		datas += buf;
		datas += "\t";

		kbe_snprintf(buf, 256, "%.3f", lastTime);
		datas += buf;
		datas += "\t";

		kbe_snprintf(buf, 256, "%.3f", sumIntTime);
		datas += buf;
		datas += "\t";

		kbe_snprintf(buf, 256, "%.3f", lastIntTime);
		datas += buf;
		datas += "\t";

		datas += name;
		datas += "\r\n";
	};

	pTelnetHandler_->onProfileEnd(datas);
}

//-------------------------------------------------------------------------------------
void TelnetEventProfileHandler::sendStream(MemoryStream* s)
{
	if(isDestroyed_) return;

	std::string datas;
	uint32 timinglen;
	ArraySize size;

	(*s) >> timinglen >> size;

	if(size == 0)
		datas += "results is empty!";

	while(size-- > 0)
	{
		std::string type_name;
		(*s) >> type_name;
		
		datas += fmt::format("Event Type:{}\r\n\r\n(name|count|size)\r\n---------------------\r\n\r\n", type_name);

		KBEngine::ArraySize size1;
		(*s) >> size1;

		while(size1-- > 0)
		{
			uint32 count;
			uint32 eventSize;
			std::string name;

			(*s) >> name >> count >> eventSize;
			
			if(count == 0)
				continue;

			datas += fmt::format("{}\t\t\t\t\t{}\t{}\r\n", name, count, eventSize);
		}

		datas += "\r\n\r\n";
	};

	pTelnetHandler_->onProfileEnd(datas);
}

//-------------------------------------------------------------------------------------
void TelnetNetworkProfileHandler::sendStream(MemoryStream* s)
{
	if(isDestroyed_) return;

	std::string datas;
	uint32 timinglen;
	ArraySize size;

	(*s) >> timinglen >> size;

	datas = "name\tsent#\tsize\tavg\ttotal#\ttotalsize\trecv#\tsize\tavg\ttotal#\ttotalsize\r\n";

	while(size-- > 0)
	{
		std::string name;

		uint32			send_size;
		uint32			send_avgsize;
		uint32			send_count;

		uint32			total_send_size;
		uint32			total_send_count;

		uint32			recv_size;
		uint32			recv_count;
		uint32			recv_avgsize;

		uint32			total_recv_size;
		uint32			total_recv_count;

		(*s) >> name >> send_count >> send_size >> send_avgsize >> total_send_size >> total_send_count;
		(*s)  >> recv_count >> recv_size >> recv_avgsize >> total_recv_size >> total_recv_count;

		char buf[256];

		datas += name;
		datas += "\t";

		kbe_snprintf(buf, 256, "%u", send_count);
		datas += buf;
		datas += "\t";

		kbe_snprintf(buf, 256, "%u", send_size);
		datas += buf;
		datas += "\t";

		kbe_snprintf(buf, 256, "%u", send_avgsize);
		datas += buf;
		datas += "\t";

		kbe_snprintf(buf, 256, "%u", total_send_count);
		datas += buf;
		datas += "\t";

		kbe_snprintf(buf, 256, "%u", total_send_size);
		datas += buf;
		datas += "\t";

		kbe_snprintf(buf, 256, "%u", recv_count);
		datas += buf;
		datas += "\t";

		kbe_snprintf(buf, 256, "%u", recv_size);
		datas += buf;
		datas += "\t";

		kbe_snprintf(buf, 256, "%u", recv_avgsize);
		datas += buf;
		datas += "\t";

		kbe_snprintf(buf, 256, "%u", total_recv_count);
		datas += buf;
		datas += "\t";

		kbe_snprintf(buf, 256, "%u", total_recv_size);
		datas += buf;

		datas += "\r\n";
	};

	pTelnetHandler_->onProfileEnd(datas);
}

//-------------------------------------------------------------------------------------
}

