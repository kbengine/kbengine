// StatusWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "StatusWindow.h"
#include "guiconsoleDlg.h"
#include "network/bundle_broadcast.h"
#include "network/message_handler.h"
#include "server/components.h"
#include "helper/console_helper.h"

#include "../../../server/baseappmgr/baseappmgr_interface.h"
#include "../../../server/cellappmgr/cellappmgr_interface.h"
#include "../../../server/baseapp/baseapp_interface.h"
#include "../../../server/cellapp/cellapp_interface.h"
#include "../../../server/dbmgr/dbmgr_interface.h"
#include "../../../server/loginapp/loginapp_interface.h"
#include "../../../server/tools/logger/logger_interface.h"
#include "../../../server/tools/bots/bots_interface.h"
#include "../../../server/tools/interfaces/interfaces_interface.h"

class LookAppTask : public thread::TPTask
{
	COMPONENT_ID _cid;
	uint32 _ip;
	uint16 _port;
	COMPONENT_TYPE _componentType;
	uint64 extradata, extradata1, extradata2, extradata3;
	int8 state;
	bool _success;
	StatusWindow* _pStatusWindow;
public:
	LookAppTask(StatusWindow* pStatusWindow, COMPONENT_TYPE componentType, COMPONENT_ID id, uint32 ip, uint16 port):
	thread::TPTask(),
	_componentType(componentType),
	_cid(id),
	_ip(ip),
	_port(port),
	extradata(0), extradata1(0), extradata2(0), extradata3(0),
	state(0),
	_success(false),
	_pStatusWindow(pStatusWindow)
	{
	}

	virtual ~LookAppTask()
	{
	}

	virtual bool process()
	{
		Network::Address address(_ip, _port);
		Network::EndPoint epListen;
		epListen.socket(SOCK_STREAM);
		if (!epListen.good())
		{
			ERROR_MSG("LookAppTask::process: couldn't create a socket\n");
			return false;
		}
	
		epListen.setnonblocking(true);

		fd_set	frds, fwds;
		struct timeval tv = { 0, 300000 }; // 100ms

		FD_ZERO( &frds );
		FD_ZERO( &fwds );
		FD_SET((int)epListen, &frds);
		FD_SET((int)epListen, &fwds);

		if(epListen.connect(_port, _ip) == -1)
		{
			int selgot = select(epListen+1, &frds, &fwds, NULL, &tv);
			if(selgot <= 0)
			{
				ERROR_MSG(fmt::format("LookAppTask::process: couldn't connect to:{}\n", 
					address.c_str()));

				return false;
			}
		}

		epListen.setnodelay(true);

		Network::Bundle* pBundle = new Network::Bundle();
		if(_componentType != BOTS_TYPE)
		{
			COMMON_NETWORK_MESSAGE(_componentType, (*pBundle), lookApp);
		}
		else
		{
			(*pBundle).newMessage(BotsInterface::lookApp);
		}

		epListen.send(pBundle->pCurrPacket()->data(), pBundle->pCurrPacket()->wpos());
		delete pBundle;

		while(true)
		{
			fd_set	fds;
			struct timeval tv = { 0, 300000 }; // 100ms

			FD_ZERO( &fds );
			FD_SET((int)epListen, &fds);

			int selgot = select(epListen+1, &fds, NULL, NULL, &tv);
			if(selgot == 0)
			{
				// 超时, 可能对方繁忙
				return false;	
			}
			else if(selgot == -1)
			{
				continue;
			}
			else
			{
				COMPONENT_TYPE ctype;
				COMPONENT_ID cid;
				int8 istate = 0;
				ArraySize entitySize = 0, cellSize = 0;
				int32 clientsSize = 0, proxicesSize = 0;
				uint32 telnet_port = 0;

				Network::TCPPacket packet;
				packet.resize(255);
				int recvsize = sizeof(ctype) + sizeof(cid) + sizeof(istate);

				if(_componentType == CELLAPP_TYPE)
				{
					recvsize += sizeof(entitySize) + sizeof(cellSize) + sizeof(telnet_port);
				}

				if(_componentType == BASEAPP_TYPE)
				{
					recvsize += sizeof(entitySize) + sizeof(clientsSize) + sizeof(proxicesSize) + sizeof(telnet_port);
				}

				int len = epListen.recv(packet.data(), recvsize);
				packet.wpos(len);
		
				if(recvsize != len)
				{
					ERROR_MSG(fmt::format("LookAppTask::process: packet invalid(recvsize({}) != ctype_cid_len({}).\n" 
						, len, recvsize));
			
					if(len == 0)
						return false;

					return false;
				}

				packet >> ctype >> cid >> istate;
		
				if(ctype == CELLAPP_TYPE)
				{
					packet >> entitySize >> cellSize >> telnet_port;
				}

				if(ctype == BASEAPP_TYPE)
				{
					packet >> entitySize >> clientsSize >> proxicesSize >> telnet_port;
				}

				if(ctype != _componentType || cid != _cid)
				{
					ERROR_MSG(fmt::format("LookAppTask::process: invalid component(ctype={}, cid={}).\n",
						ctype, cid));

					return false;
				}

				_success = true;

				extradata = entitySize;
				extradata1 = cellSize;
				extradata2 = telnet_port;
				extradata3 = 0;

				state = istate;

				if(ctype == CELLAPP_TYPE)
				{
					extradata = entitySize;
					extradata1 = cellSize;
					extradata3 = telnet_port;
				}
			
				if(ctype == BASEAPP_TYPE)
				{
					extradata = entitySize;
					extradata1 = clientsSize;
					extradata2 = proxicesSize;
					extradata3 = telnet_port;
				}
			}
		}

		return false;
	}

	virtual thread::TPTask::TPTaskState presentMainThread()
	{ 
		if(!_success)
			return thread::TPTask::TPTASK_STATE_COMPLETED; 

		Components::ComponentInfos* winfo = Components::getSingleton().findComponent(_componentType, _cid);
		if(winfo == NULL)
			return thread::TPTask::TPTASK_STATE_COMPLETED; 

		winfo->state = (COMPONENT_STATE)state;
		winfo->extradata = extradata;
		winfo->extradata1 = extradata1;
		winfo->extradata2 = extradata2;
		winfo->extradata3 = extradata3;

		_pStatusWindow->update(*winfo);
		return thread::TPTask::TPTASK_STATE_COMPLETED; 
	}
};

// StatusWindow dialog

IMPLEMENT_DYNAMIC(StatusWindow, CDialog)

StatusWindow::StatusWindow(CWnd* pParent /*=NULL*/)
	: CDialog(StatusWindow::IDD, pParent)
{

}

StatusWindow::~StatusWindow()
{
}

void StatusWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATUS_LIST1, m_statusList);
}


BEGIN_MESSAGE_MAP(StatusWindow, CDialog)
END_MESSAGE_MAP()

BOOL StatusWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	DWORD dwStyle = m_statusList.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;					//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;						//网格线（只适用与report风格的listctrl）
	//dwStyle |= LVS_EX_ONECLICKACTIVATE;
	m_statusList.SetExtendedStyle(dwStyle);				//设置扩展风格

	int idx = 0;
	m_statusList.InsertColumn(idx++, _T("uid"),					LVCFMT_CENTER,	40);
	m_statusList.InsertColumn(idx++, _T("componentType"),		LVCFMT_CENTER,	100);
	m_statusList.InsertColumn(idx++, _T("componentID"),			LVCFMT_CENTER,	120);
	m_statusList.InsertColumn(idx++, _T("cpu"),					LVCFMT_CENTER,	60);
	m_statusList.InsertColumn(idx++, _T("memory"),				LVCFMT_CENTER,	100);
	m_statusList.InsertColumn(idx++, _T("entities"),			LVCFMT_CENTER,	100);
	m_statusList.InsertColumn(idx++, _T("proxies"),				LVCFMT_CENTER,	100);
	m_statusList.InsertColumn(idx++, _T("clients"),				LVCFMT_CENTER,	100);
	m_statusList.InsertColumn(idx++, _T("cells"),				LVCFMT_CENTER,	100);
	m_statusList.InsertColumn(idx++, _T("address"),				LVCFMT_CENTER,	140);
	m_statusList.InsertColumn(idx++, _T("username"),			LVCFMT_CENTER,	100);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void StatusWindow::autoWndSize()
{
	CRect rect;
	GetClientRect(&rect);

	m_statusList.MoveWindow(2, 3, rect.right - 2, rect.bottom - 3, TRUE);
}

void StatusWindow::addApp(Components::ComponentInfos& cinfos)
{
	std::stringstream stream;
	stream << cinfos.cid;
	CString str;
	std::string tstr;
	stream >> tstr;
	wchar_t* ws = KBEngine::strutil::char2wchar(tstr.c_str());
	str = ws;
	free(ws);

	bool found = false;
	for(int icount = 0; icount < m_statusList.GetItemCount(); icount++)
	{
		CString s = m_statusList.GetItemText(icount, 2);
		if(str == s)
		{
			found = true;
			break;
		}
	}

	if(!found)
	{
		CString suid;
		suid.Format(L"%u", cinfos.uid);
		m_statusList.InsertItem(0, suid);
		static int ndata = 0;
		m_statusList.SetItemData(0, ndata++);

		CString cname;
		ws = KBEngine::strutil::char2wchar(COMPONENT_NAME_EX(cinfos.componentType));
		cname = ws;
		free(ws);
		m_statusList.SetItemText(0, 1, cname);

		m_statusList.SetItemText(0, 2, str);

		suid.Format(L"%f", cinfos.cpu);
		m_statusList.SetItemText(0, 3, suid);

		suid.Format(L"%.2fm", float(cinfos.usedmem) / 1024.f / 1024.f);
		m_statusList.SetItemText(0, 4, suid);

		if(cinfos.componentType == CELLAPP_TYPE)
		{
			suid.Format(L"%d", cinfos.extradata);
			m_statusList.SetItemText(0, 5, suid);
			suid.Format(L"%d", cinfos.extradata1);
			m_statusList.SetItemText(0, 8, suid);
		}

		if(cinfos.componentType == BASEAPP_TYPE)
		{
			suid.Format(L"%d", cinfos.extradata);
			m_statusList.SetItemText(0, 5, suid);
			suid.Format(L"%d", cinfos.extradata2);
			m_statusList.SetItemText(0, 6, suid);
			suid.Format(L"%d", cinfos.extradata1);
			m_statusList.SetItemText(0, 7, suid);
		}

		ws = KBEngine::strutil::char2wchar(cinfos.pIntAddr->c_str());
		str = ws;
		free(ws);
		m_statusList.SetItemText(0, 9, str);

		ws = KBEngine::strutil::char2wchar(cinfos.username);
		str = ws;
		free(ws);
		m_statusList.SetItemText(0, 10, str);

		CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
		dlg->addThreadTask(new LookAppTask(this, cinfos.componentType, cinfos.cid, cinfos.pIntAddr->ip, cinfos.pIntAddr->port));
	}
}

void StatusWindow::update(Components::ComponentInfos& cinfos)
{
	int count = m_statusList.GetItemCount();

	std::stringstream stream;
	stream << cinfos.cid;
	CString cidstr;
	wchar_t* ws = KBEngine::strutil::char2wchar(stream.str().c_str());
	cidstr = ws;
	free(ws);

	for(int i = 0; i < count; i++)
	{
		CString str = m_statusList.GetItemText(i, 2);
		if(str == cidstr)
		{
			CString cstr;

			if(cinfos.componentType == CELLAPP_TYPE)
			{
				cstr.Format(L"%d", cinfos.extradata);
				m_statusList.SetItemText(i, 5, cstr);
				cstr.Format(L"%d", cinfos.extradata1);
				m_statusList.SetItemText(i, 8, cstr);
			}

			if(cinfos.componentType == BASEAPP_TYPE)
			{
				cstr.Format(L"%d", cinfos.extradata);
				m_statusList.SetItemText(i, 5, cstr);
				cstr.Format(L"%d", cinfos.extradata2);
				m_statusList.SetItemText(i, 6, cstr);
				cstr.Format(L"%d", cinfos.extradata1);
				m_statusList.SetItemText(i, 7, cstr);
			}
		}
	}
}

// StatusWindow message handlers
