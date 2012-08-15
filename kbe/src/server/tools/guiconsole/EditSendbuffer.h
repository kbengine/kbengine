#pragma once


// CEditSendbuffer

class CEditSendbuffer : public CEdit
{
	DECLARE_DYNAMIC(CEditSendbuffer)

public:
	CEditSendbuffer();
	virtual ~CEditSendbuffer();
	
	BOOL PreTranslateMessage(MSG* pMsg);
protected:
	DECLARE_MESSAGE_MAP()
};


