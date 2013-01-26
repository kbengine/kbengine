// StartServerLayoutWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "SetLayoutNameWindow.h"
#include "StartServerLayoutWindow.h"
#include "server/serverconfig.hpp"

// CStartServerLayoutWindow dialog

IMPLEMENT_DYNAMIC(CStartServerLayoutWindow, CDialog)

CStartServerLayoutWindow::CStartServerLayoutWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CStartServerLayoutWindow::IDD, pParent)
{

}

CStartServerLayoutWindow::~CStartServerLayoutWindow()
{
}

void CStartServerLayoutWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_LIST3, m_componentlist);
	DDX_Control(pDX, IDC_EDIT1, m_port);
	DDX_Control(pDX, IDC_LIST2, m_log);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ip);
}


BEGIN_MESSAGE_MAP(CStartServerLayoutWindow, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CStartServerLayoutWindow::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CStartServerLayoutWindow::OnBnClickedButton2)
END_MESSAGE_MAP()

// CStartServerWindow message handlers
BOOL CStartServerLayoutWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	loadHistory();

	DWORD dwStyle = m_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;					//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;						//网格线（只适用与report风格的listctrl）
	//dwStyle |= LVS_EX_ONECLICKACTIVATE;
	m_list.SetExtendedStyle(dwStyle);				//设置扩展风格

	int idx = 0;
	m_list.InsertColumn(idx++, _T("componentType"),				LVCFMT_CENTER,	200);
	m_list.InsertColumn(idx++, _T("addr"),						LVCFMT_CENTER,	250);

	m_componentlist.AddString(L"cellapp");
	m_componentlist.AddString(L"baseapp");
	m_componentlist.AddString(L"cellappmgr");
	m_componentlist.AddString(L"baseappmgr");
	m_componentlist.AddString(L"loginapp");
	m_componentlist.AddString(L"dbmgr");
	m_componentlist.AddString(L"resourcemgr");

	m_port.SetWindowTextW(L"20099");

	int i = 0;
	std::deque<CString>::iterator iter = m_historyCommand.begin();
	for(; iter != m_historyCommand.end(); iter++)
	{
		if(i == 0)
		{
			CString output = L"";
			CString output1 = L"";
			AfxExtractSubString(output, (*iter), 0, _T(':'));
			AfxExtractSubString(output1, (*iter), 1, _T(':'));

			char* ip = KBEngine::strutil::wchar2char(output.GetBuffer(0));
			m_ip.SetAddress(ntohl(inet_addr(ip)));
			free(ip);

			m_port.SetWindowTextW(output1);
		}

		i++;
		m_log.AddString((*iter));
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CStartServerLayoutWindow::saveHistory()
{
    //创建一个XML的文档对象。
    TiXmlDocument *pDocument = new TiXmlDocument();

	int i = 0;
	std::deque<CString>::iterator iter = m_historyCommand.begin();
	TiXmlElement *rootElement = new TiXmlElement("root");
	pDocument->LinkEndChild(rootElement);

	for(; iter != m_historyCommand.end(); iter++)
	{
		char key[256] = {0};
		kbe_snprintf(key, 256, "item%d", i++);
		TiXmlElement *rootElementChild = new TiXmlElement(key);
		rootElement->LinkEndChild(rootElementChild);

		char buffer[4096] = {0};
		CString strCommand = (*iter);

		int len = WideCharToMultiByte(CP_ACP, 0, strCommand, strCommand.GetLength(), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_ACP,0, strCommand, strCommand.GetLength(), buffer, len, NULL, NULL);
		buffer[len + 1] = '\0';


		TiXmlText *content = new TiXmlText(buffer);
		rootElementChild->LinkEndChild(content);
	}

    CString appPath = GetAppPath();
    CString fullPath = appPath + L"\\histroycommands1.xml";

	char fname[4096] = {0};

	int len = WideCharToMultiByte(CP_ACP, 0, fullPath, fullPath.GetLength(), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP,0, fullPath, fullPath.GetLength(), fname, len, NULL, NULL);
	fname[len + 1] = '\0';

	pDocument->SaveFile(fname);

	m_log.ResetContent();
	std::deque<CString>::iterator iter1 = m_historyCommand.begin();
	for(; iter1 != m_historyCommand.end(); iter1++)
	{
		m_log.AddString((*iter1));
	}
}

void CStartServerLayoutWindow::loadHistory()
{
    CString appPath = GetAppPath();
    CString fullPath = appPath + L"\\histroycommands1.xml";

	char fname[4096] = {0};

	int len = WideCharToMultiByte(CP_ACP, 0, fullPath, fullPath.GetLength(), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP,0, fullPath, fullPath.GetLength(), fname, len, NULL, NULL);
	fname[len + 1] = '\0';

	TiXmlDocument *pDocument = new TiXmlDocument(fname);
	if(pDocument == NULL || !pDocument->LoadFile(TIXML_ENCODING_UTF8))
		return;

	TiXmlElement *rootElement = pDocument->RootElement();
	TiXmlNode* node = rootElement->FirstChild();
	if(node)
	{
		do																				
		{																				
			std::string c = node->FirstChild()->Value();
			wchar_t* strCommand = KBEngine::strutil::char2wchar(c.c_str());
			m_historyCommand.push_back(strCommand);
			free(strCommand);
		}while((node = node->NextSibling()));												
	}

	pDocument->Clear();
	delete pDocument;
}	

// CStartServerLayoutWindow message handlers

void CStartServerLayoutWindow::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here

	int i = m_componentlist.GetCurSel();

	if(i < 0)
	{
		AfxMessageBox(L"no select componentType!");
		return;
	}

	CString s;
	m_componentlist.GetText(i, s);

	byte ips[4];

	if (0 == m_ip.GetAddress(ips[0],ips[1],ips[2],ips[3]))
	{
		AfxMessageBox(L"address is error!");
		return;
	}
	
	char strip[256];
	sprintf_s(strip, 256, "%d.%d.%d.%d", ips[0],ips[1],ips[2],ips[3]);
	
	KBEngine::u_int16_t port = 0;
	CString sport;
	m_port.GetWindowTextW(sport);

	char* csport = KBEngine::strutil::wchar2char(sport.GetBuffer(0));
	port = atoi(csport);

	std::string command = strip;
	command += ":";
	command += csport;
	free(csport);

	m_list.InsertItem(0, s.GetBuffer());

	wchar_t* ws = KBEngine::strutil::char2wchar(command.c_str());
	m_list.SetItemText(0, 1, ws);
	free(ws);


	wchar_t* wcommand = KBEngine::strutil::char2wchar(command.c_str());
	bool found = false;
	std::deque<CString>::iterator iter = m_historyCommand.begin();
	for(; iter != m_historyCommand.end(); iter++)
	{
		if((*iter) == wcommand)
		{
			found = true;
			break;
		}
	}
	
	if(!found)
	{
		m_historyCommand.push_front(wcommand);
		if(m_historyCommand.size() > 10)
			m_historyCommand.pop_back();

		saveHistory();
	}

	free(wcommand);
}

void CStartServerLayoutWindow::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	CSetLayoutNameWindow dlg;
	dlg.DoModal();

	CString layoutname = dlg.m_name;
	if(layoutname.GetLength() <= 0)
	{
		::AfxMessageBox(L"is failed!");
		return;
	}

	::AfxMessageBox(L"successfully!");
}
