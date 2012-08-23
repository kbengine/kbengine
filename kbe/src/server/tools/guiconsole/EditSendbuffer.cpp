// EditSendbuffer.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "EditSendbuffer.h"
#include "guiconsoleDlg.h"

// CEditSendbuffer

IMPLEMENT_DYNAMIC(CEditSendbuffer, CEdit)

CEditSendbuffer::CEditSendbuffer()
{

}

CEditSendbuffer::~CEditSendbuffer()
{
}


BEGIN_MESSAGE_MAP(CEditSendbuffer, CEdit)
END_MESSAGE_MAP()


BOOL CEditSendbuffer::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN || WM_SYSKEYDOWN == pMsg->message)
	{
		CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);

		if(pMsg->wParam == VK_RETURN && GetKeyState(VK_CONTROL)&0x80)
		{
			CString s ;
			this->GetWindowTextW(s);
			dlg->commitPythonCommand(s);
			this->SetWindowTextW(L"");
			return FALSE;  
		}
		else if((GetKeyState(VK_MENU) < 0 || GetKeyState(VK_CONTROL) < 0) && GetKeyState(VK_UP) < 0)
		{
			CString s = dlg->getHistoryCommand(false);
			if(s.GetLength() > 0)
			{
				this->SetWindowTextW(s);
			}
		}
		else if((GetKeyState(VK_MENU) < 0 || GetKeyState(VK_CONTROL) < 0)  && GetKeyState(VK_DOWN) < 0)
		{
			CString s = dlg->getHistoryCommand(true);
			if(s.GetLength() > 0)
			{
				this->SetWindowTextW(s);
			}
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}
// CEditSendbuffer message handlers


