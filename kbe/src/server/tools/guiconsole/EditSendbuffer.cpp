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
		//L R auto fill
		else if (pMsg->wParam == VK_CONTROL)
		{
			CString prefix;
			this->GetWindowTextW(prefix);
			if (prefix.GetLength() > 0) {
				prefix.MakeLower();
				if (m_baseInput.Compare(prefix) != 0) {
					m_idxRelatedCmds = -1;
					m_vecRelatedCmds.clear();
					m_baseInput = prefix;
					dlg->getHistoryCmdsByPrefix(m_baseInput, m_vecRelatedCmds);
				}
			}
		}
		else if ((GetKeyState(VK_MENU) < 0 || GetKeyState(VK_CONTROL) < 0) && GetKeyState(VK_RIGHT) < 0)
		{
			int nCmds = static_cast<int>(m_vecRelatedCmds.size());
			if (nCmds > 0) {
				m_idxRelatedCmds++;
				if (m_idxRelatedCmds >= nCmds) {
					m_idxRelatedCmds = 0;
				}
				else if (m_idxRelatedCmds < 0) {
					m_idxRelatedCmds = nCmds - 1;
				}

				this->SetWindowTextW(m_vecRelatedCmds[m_idxRelatedCmds]);
			}			
		}
		else if ((GetKeyState(VK_MENU) < 0 || GetKeyState(VK_CONTROL) < 0) && GetKeyState(VK_LEFT) < 0)
		{
			int nCmds = static_cast<int>(m_vecRelatedCmds.size());
			if (nCmds > 0) {
				m_idxRelatedCmds--;
				if (m_idxRelatedCmds >= nCmds) {
					m_idxRelatedCmds = 0;
				}
				else if (m_idxRelatedCmds < 0) {
					m_idxRelatedCmds = nCmds - 1;
				}

				this->SetWindowTextW(m_vecRelatedCmds[m_idxRelatedCmds]);
			}
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}
// CEditSendbuffer message handlers


