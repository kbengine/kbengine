// GraphsWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "GraphsWindow.h"


// CGraphsWindow dialog
#define GET_RANDOM( min, max ) ((rand() % (int)(((max)+1) - (min))) + (min))
IMPLEMENT_DYNAMIC(CGraphsWindow, CDialog)

CGraphsWindow::CGraphsWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CGraphsWindow::IDD, pParent)
{

}

CGraphsWindow::~CGraphsWindow()
{
}

void CGraphsWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRAPH, m_plot);
}


BEGIN_MESSAGE_MAP(CGraphsWindow, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CGraphsWindow::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_plot.AddLine(PS_SOLID,RGB(255,0,0));
	m_plot.AddLine(PS_SOLID,RGB(0,255,0));

	m_plot.GetAxisY().m_color=RGB(0,255,0);
	m_plot.GetAxisY().m_dMinValue=5000;
	m_plot.GetAxisY().m_dMaxValue=6000;
	m_plot.CalcLayout();

	SetTimer(1,150,NULL);
	m_plot.SetRate(150);//
	m_plot.Start();
	return TRUE; 
};

void CGraphsWindow::autoWndSize()
{
	CRect rect;
	GetClientRect(&rect);
}

// CGraphsWindow message handlers

void CGraphsWindow::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	BOOL bAxisYChanged = FALSE;
	CDialog::OnTimer(nIDEvent);

	double x=(double)time(0);
	double y0=(double)GET_RANDOM(4000,6899);
	double y1=(double)GET_RANDOM(6000,8899);
	if(y0<m_plot.GetAxisY().m_dMinValue)
	{
		m_plot.GetAxisY().m_dMinValue=y0*0.95;
		bAxisYChanged = TRUE;
	}else if(y0>m_plot.GetAxisY().m_dMaxValue)
	{
		m_plot.GetAxisY().m_dMaxValue=y0*1.05;
		bAxisYChanged = TRUE;
	}
	if(y1<m_plot.GetAxisY().m_dMinValue)
	{
		m_plot.GetAxisY().m_dMinValue=y1*0.95;
		bAxisYChanged = TRUE;
	}else if(y1>m_plot.GetAxisY().m_dMaxValue)
	{
		m_plot.GetAxisY().m_dMaxValue=y1*1.05;
		bAxisYChanged = TRUE;
	}

	if(bAxisYChanged)
	{
		m_plot.CalcLayout();
	}

	m_plot.GetLine(0).AddPoint(x,y0);
	m_plot.GetLine(1).AddPoint(x,y1);
		
}
