/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "telnet_handler.hpp"
#include "telnet_server.hpp"
#include "network/bundle.hpp"
#include "network/endpoint.hpp"
#include "pyscript/script.hpp"

#ifndef CODE_INLINE
#include "telnet_handler.ipp"
#endif

namespace KBEngine { 

char _g_state_str[][256] = {
	"password",
	"root",
	"python"
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

#define TELNET_CMD_DEL							"\033[K"			// 删除字符
#define TELNET_CMD_NEWLINE						"\r\n"				// 新行
#define TELNET_CMD_MOVE_FOCUS_LEFT_MAX			"\33[9999999999D"	// 左移光标到最前面
#define TELNET_CMD_MOVE_FOCUS_RIGHT_MAX			"\33[9999999999C"	// 右移光标到最后面

//-------------------------------------------------------------------------------------
TelnetHandler::TelnetHandler(Mercury::EndPoint* pEndPoint, TelnetServer* pTelnetServer, TELNET_STATE defstate):
buffer_(),
command_(),
historyCommand_(),
historyCommandIndex_(0),
pEndPoint_(pEndPoint),
pTelnetServer_(pTelnetServer),
state_(defstate),
currPos_(0)
{
}

//-------------------------------------------------------------------------------------
TelnetHandler::~TelnetHandler(void)
{
}

//-------------------------------------------------------------------------------------
std::string TelnetHandler::getInputStartString()
{
	return(boost::format("[%1%@%2% ~]%3% ") % COMPONENT_NAME_EX(g_componentType) % 
		_g_state_str[(int)state_] % (state_ == TELNET_STATE_PYTHON ? " >>>" : "#")).str();
}

//-------------------------------------------------------------------------------------
std::string TelnetHandler::getWelcome()
{
	return (boost::format("\033[1;32mwelcome to %1% \r\n"
			"Version: %2%. "
			"Config: %3%. "
			"Built: %4% %5%. "
			"AppUID: %6%. "
			"UID: %7%. "
			"PID: %8%"
			"\r\n---------------------------------------------"
			"%9%"
			"\r\n---------------------------------------------"
			" \033[0m\r\n%10%") %
		COMPONENT_NAME_EX(g_componentType) % KBEVersion::versionString().c_str() %
		KBE_CONFIG % __TIME__ % __DATE__ %
		g_componentID % getUserUID() % getProcessPID() % help() % getInputStartString()).str();
}

//-------------------------------------------------------------------------------------
std::string TelnetHandler::help()
{
	return 	"\033[1;32m\r\nCommand List:"
		"\r\n[:help        ]: list commands."
		"\r\n[:python      ]: python console."
		"\r\n[:root        ]: return to the root layer."
		"\r\n[:cprofile    ]: collects and reports the internal c++ profiles \r\n\t\tof a server process over a period of time."
		"\r\n\t\t usage: \":cprofile 30\""
		"\r\n[:pyprofile    ]: collects and reports the python profiles \r\n\t\tof a server process over a period of time."
		"\r\n\t\t usage: \":pyprofile 30\""
		"\r\n[:eventprofile]: a server process over a period of time, \r\n\t\tcollects and reports the all non-volatile cummunication \r\n\t\tdown to the client."
		"\r\n\t\t usage: \":eventprofile 30\""
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
int	TelnetHandler::handleInputNotification(int fd)
{
	KBE_ASSERT((*pEndPoint_) == fd);

	char data[1024] = {0};
	int recvsize = pEndPoint_->recv(data, sizeof(data));

	if(recvsize == -1)
	{
		return 0;
	}
	else if(recvsize == 0)
	{
		onRecvInput();
		pTelnetServer_->onTelnetHandlerClosed(fd, this);
		return 0;
	}
	
	for(int i = 0; i < recvsize; i++)
	{
		buffer_.push_back(data[i]);
	}

	onRecvInput();
	return 0;
}

//-------------------------------------------------------------------------------------
void TelnetHandler::onRecvInput()
{
	while (buffer_.size() > 0)
	{
		int c = (unsigned char)buffer_.front();
		buffer_.pop_front();

		switch(c)
		{
		case '\r':
			if(buffer_.size() > 0)
			{
				int cc = (unsigned char)buffer_.front();
				if(cc == '\n')
				{
					buffer_.pop_front();
					processCommand();
					command_ = "";
					sendNewLine();
				}
			}
			break;
		case 8:		// 退格
			sendDelChar();
			break;
		case ' ':	// 空格
		default:
			{
				std::string s = "";
				s += c;
				command_.insert(currPos_, s);
				currPos_++;
				
				if(!checkUDLR())
				{
					if(currPos_ != command_.size())
					{
						s = command_.substr(currPos_, command_.size() - currPos_);
						s += (boost::format("\33[%1%D") % s.size()).str();
						pEndPoint_->send(s.c_str(), s.size());
					}
				}
				break;
			}
		};
	}
}

//-------------------------------------------------------------------------------------
bool TelnetHandler::checkUDLR()
{
	if(command_.find(TELNET_CMD_UP) != -1)		// 上 
	{
		pEndPoint_->send(TELNET_CMD_MOVE_FOCUS_LEFT_MAX, strlen(TELNET_CMD_MOVE_FOCUS_LEFT_MAX));
		sendDelChar();
		std::string startstr = getInputStartString();
		pEndPoint_->send(startstr.c_str(), startstr.size());
		resetStartPosition();
		std::string s = getHistoryCommand(false);
		pEndPoint_->send(s.c_str(), s.size());
		command_ = s;
		buffer_.clear();
		currPos_ = s.size();
		return true;
	}
	else if(command_.find(TELNET_CMD_DOWN) != -1)	// 下
	{
		pEndPoint_->send(TELNET_CMD_MOVE_FOCUS_LEFT_MAX, strlen(TELNET_CMD_MOVE_FOCUS_LEFT_MAX));
		sendDelChar();
		std::string startstr = getInputStartString();
		pEndPoint_->send(startstr.c_str(), startstr.size());
		resetStartPosition();
		std::string s = getHistoryCommand(true);
		pEndPoint_->send(s.c_str(), s.size());
		command_ = s;
		buffer_.clear();
		currPos_ = s.size();
		return true;
	}
	else if(command_.find(TELNET_CMD_RIGHT) != -1)	// 右
	{
		int cmdlen = strlen(TELNET_CMD_RIGHT);
		currPos_-= cmdlen;
		command_.erase(command_.find(TELNET_CMD_RIGHT), cmdlen);

		if(currPos_ < (int)command_.size())
		{
			currPos_++;
			pEndPoint_->send(TELNET_CMD_RIGHT, cmdlen);
		}
		return true;
	}
	else if(command_.find(TELNET_CMD_LEFT) != -1)	// 左 
	{
		int cmdlen = strlen(TELNET_CMD_LEFT);
		currPos_-= (int)(cmdlen + 1);
		if(currPos_ < 0)
		{
			currPos_ = 0;
		}
		else
		{
			pEndPoint_->send(TELNET_CMD_LEFT, cmdlen);
		}

		command_.erase(command_.find(TELNET_CMD_LEFT), cmdlen);
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
void TelnetHandler::processCommand()
{
	if(command_.size() == 0)
		return;

	bool logcmd = true;
	for(int i=0; i<(int)historyCommand_.size(); i++)
	{
		if(historyCommand_[i] == command_)
		{
			logcmd = false;
			break;
		}
	}

	if(logcmd)
	{
		historyCommand_.push_back(command_);
		historyCommandCheck();
		historyCommandIndex_ = historyCommand_.size() - 1;
	}

	if(command_ == ":python")
	{
		if(pTelnetServer_->pScript() == NULL)
			return;

		state_ = TELNET_STATE_PYTHON;
		return;
	}
	else if(command_ == ":help")
	{
		std::string str = help();
		pEndPoint_->send(str.c_str(), str.size());
		return;
	}
	else if(command_ == ":root")
	{
		state_ = TELNET_STATE_ROOT;
		return;
	}
	else if(command_ == ":cprofile")
	{
		return;
	}
	else if(command_ == ":pyprofile")
	{
		return;
	}
	else if(command_ == ":eventprofile")
	{
		return;
	}

	if(state_ == TELNET_STATE_PYTHON)
	{
		processPythonCommand();
		return;
	}
	else if(state_ == TELNET_STATE_PASSWD)
	{
		if(command_ == pTelnetServer_->passwd())
		{
			state_ = (TELNET_STATE)pTelnetServer_->deflayer();
			std::string s = getWelcome();
			pEndPoint_->send(s.c_str(), s.size());
			sendEnter();
		}

		return;
	}
}

//-------------------------------------------------------------------------------------
void TelnetHandler::processPythonCommand()
{
	if(pTelnetServer_->pScript() == NULL || command_.size() == 0)
		return;
	
	PyObject* pycmd = PyUnicode_DecodeUTF8(command_.data(), command_.size(), NULL);
	if(pycmd == NULL)
	{
		SCRIPT_ERROR_CHECK();
		return;
	}

	DEBUG_MSG(boost::format("TelnetHandler::processPythonCommand: size(%1%), command=%2%.\n") % 
		command_.size() % command_);

	std::string retbuf = "";
	PyObject* pycmd1 = PyUnicode_AsEncodedString(pycmd, "utf-8", NULL);

	if(pTelnetServer_->pScript()->run_simpleString(PyBytes_AsString(pycmd1), &retbuf) == 0)
	{
		// 将结果返回给客户端
		Mercury::Bundle bundle;
		bundle << retbuf;
		bundle.send(*pEndPoint_);
		sendEnter();
	}

	Py_DECREF(pycmd);
	Py_DECREF(pycmd1);
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
		command_.erase(command_.size() - 1, 1);
		currPos_--;
		pEndPoint_->send(TELNET_CMD_DEL, strlen(TELNET_CMD_DEL));
	}
	else
	{
		resetStartPosition();
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

//-------------------------------------------------------------------------------------
void TelnetHandler::resetStartPosition()
{
	pEndPoint_->send(TELNET_CMD_MOVE_FOCUS_LEFT_MAX, strlen(TELNET_CMD_MOVE_FOCUS_LEFT_MAX));
	std::string startstr = getInputStartString();
	std::string backcmd = (boost::format("\33[%1%C") % startstr.size()).str();
	pEndPoint_->send(backcmd.c_str(), backcmd.size());
}

//-------------------------------------------------------------------------------------
}

