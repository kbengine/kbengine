// StartServerLayoutWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "SetLayoutNameWindow.h"
#include "StartServerLayoutWindow.h"
#include "server/serverconfig.h"
#include "StartServerWindow.h"

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
	DDX_Control(pDX, IDC_COMBO1, m_layoutlist);
}


BEGIN_MESSAGE_MAP(CStartServerLayoutWindow, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CStartServerLayoutWindow::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CStartServerLayoutWindow::OnBnClickedButton2)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CStartServerLayoutWindow::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON3, &CStartServerLayoutWindow::OnBnClickedButton3)
	ON_NOTIFY(HDN_ITEMCHANGED, 0, &CStartServerLayoutWindow::OnHdnItemchangedList1)
	ON_BN_CLICKED(IDC_BUTTON4, &CStartServerLayoutWindow::OnBnClickedButton4)
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

	int count = static_cast<CStartServerWindow*>(this->GetParent())->m_layoutlist.GetCount();
	for(int i=0; i<count; i++)
	{
		CString s;
		static_cast<CStartServerWindow*>(this->GetParent())->m_layoutlist.GetLBText(i, s);
		m_layoutlist.AddString(s);
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
		AfxMessageBox(L"address error!");
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

	char* cs = KBEngine::strutil::wchar2char(layoutname.GetBuffer(0));
	std::vector<CStartServerWindow::LAYOUT_ITEM>& vec = static_cast<CStartServerWindow*>(this->GetParent())->layouts_[cs];
	vec.clear();
	free(cs);

	int count = m_list.GetItemCount();
	for(int i=0; i<count; i++)
	{
		CString s = m_list.GetItemText(i, 0);
		cs = KBEngine::strutil::wchar2char(s.GetBuffer(0));
		CStartServerWindow::LAYOUT_ITEM item;

		item.componentName = cs;
		free(cs);

		s = m_list.GetItemText(i, 1);
		cs = KBEngine::strutil::wchar2char(s.GetBuffer(0));
		item.addr = cs;
		free(cs);

		vec.push_back(item);
	}

	static_cast<CStartServerWindow*>(this->GetParent())->saveLayouts();
	static_cast<CStartServerWindow*>(this->GetParent())->loadLayouts();
}

void CStartServerLayoutWindow::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
	CString s;

	if(m_layoutlist.GetEditSel() < 0 || m_layoutlist.GetEditSel() > (DWORD)m_layoutlist.GetCount())
		return;

	m_layoutlist.GetLBText(m_layoutlist.GetEditSel(), s);
	
	if(s.GetLength() <= 0 )
		return;

	m_list.DeleteAllItems();

	char* cs = KBEngine::strutil::wchar2char(s.GetBuffer(0));
	KBEUnordered_map< std::string, std::vector<CStartServerWindow::LAYOUT_ITEM> >::iterator iter = 
		static_cast<CStartServerWindow*>(this->GetParent())->layouts_.find(cs);

	free(cs);

	if(iter == static_cast<CStartServerWindow*>(this->GetParent())->layouts_.end())
	{
		return;
	}

	std::vector<CStartServerWindow::LAYOUT_ITEM>::iterator iter1 = iter->second.begin();
	for(; iter1 != iter->second.end(); iter1++)
	{
		CStartServerWindow::LAYOUT_ITEM& item = (*iter1);

		wchar_t* ws1 = KBEngine::strutil::char2wchar(item.componentName.c_str());
		wchar_t* ws2 = KBEngine::strutil::char2wchar(item.addr.c_str());

		m_list.InsertItem(0, ws1);
		m_list.SetItemText(0, 1, ws2);

		free(ws1);
		free(ws2);
	}
}

void CStartServerLayoutWindow::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	POSITION pos = m_list.GetFirstSelectedItemPosition(); 

	if(NULL == pos) 
		return; 

	int nItem = m_list.GetNextSelectedItem(pos);

	CStartServerWindow::LAYOUT_ITEM item1;

	CString s = m_list.GetItemText(nItem, 0);
	char* cs = KBEngine::strutil::wchar2char(s.GetBuffer(0));

	item1.componentName = cs;
	free(cs);

	s = m_list.GetItemText(nItem, 1);
	cs = KBEngine::strutil::wchar2char(s.GetBuffer(0));
	item1.addr = cs;
	free(cs);


	if(m_layoutlist.GetEditSel() < 0 || m_layoutlist.GetEditSel() > (DWORD)m_layoutlist.GetCount())
		return;

	m_layoutlist.GetLBText(m_layoutlist.GetEditSel(), s);
	
	if(s.GetLength() <= 0 )
		return;

	cs = KBEngine::strutil::wchar2char(s.GetBuffer(0));
	KBEUnordered_map< std::string, std::vector<CStartServerWindow::LAYOUT_ITEM> >::iterator iter = 
		static_cast<CStartServerWindow*>(this->GetParent())->layouts_.find(cs);

	free(cs);

	if(iter == static_cast<CStartServerWindow*>(this->GetParent())->layouts_.end())
	{
		return;
	}

	bool found = false;
	std::vector<CStartServerWindow::LAYOUT_ITEM>::iterator iter1 = iter->second.begin();
	for(; iter1 != iter->second.end(); iter1++)
	{
		CStartServerWindow::LAYOUT_ITEM& item = (*iter1);
		
		if(item.addr == item1.addr && item.componentName == item1.componentName)
		{
			found = true;
			iter->second.erase(iter1);
			break;
		}
	}

	if(found)
		m_list.DeleteItem(nItem);
}

void CStartServerLayoutWindow::OnHdnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

}

void CStartServerLayoutWindow::OnBnClickedButton4()
{
	CString s;

	if(m_layoutlist.GetCurSel() < 0 || m_layoutlist.GetCurSel() > m_layoutlist.GetCount())
		return;

	m_layoutlist.GetLBText(m_layoutlist.GetCurSel(), s);

	if(s.GetLength() <= 0 )
		return;

	m_list.DeleteAllItems();
	m_layoutlist.DeleteString(m_layoutlist.GetCurSel());

	char* cs = KBEngine::strutil::wchar2char(s.GetBuffer(0));
	KBEUnordered_map< std::string, std::vector<CStartServerWindow::LAYOUT_ITEM> >::iterator iter = 
		static_cast<CStartServerWindow*>(this->GetParent())->layouts_.find(cs);

	free(cs);

	if(iter == static_cast<CStartServerWindow*>(this->GetParent())->layouts_.end())
	{
		return;
	}

	static_cast<CStartServerWindow*>(this->GetParent())->layouts_.erase(iter);
	static_cast<CStartServerWindow*>(this->GetParent())->saveLayouts();
	static_cast<CStartServerWindow*>(this->GetParent())->loadLayouts();
}
