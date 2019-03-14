#pragma once


// CEditSendbuffer

class CEditSendbuffer : public CEdit
{
	DECLARE_DYNAMIC(CEditSendbuffer)

	CString m_baseInput; // LowerCase
	int m_idxRelatedCmds;
	std::vector<CString> m_vecRelatedCmds;

public:
	CEditSendbuffer();
	virtual ~CEditSendbuffer();
	
	BOOL PreTranslateMessage(MSG* pMsg);
protected:
	DECLARE_MESSAGE_MAP()
};


