#pragma once
#include "afxcmn.h"
#include "server/components.h"

// StatusWindow dialog
using namespace KBEngine;
class StatusWindow : public CDialog
{
	DECLARE_DYNAMIC(StatusWindow)

public:
	StatusWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~StatusWindow();

	BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_STATUS };
	void autoWndSize();

	void addApp(Components::ComponentInfos& cinfos);
	void update(Components::ComponentInfos& cinfos);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_statusList;
};
