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
	m_plot.GetAxisY().m_dMinValue=0;
	m_plot.GetAxisY().m_dMaxValue=100;
	m_plot.CalcLayout();

	SetTimer(1,100,NULL);
	startPlot();
	return TRUE; 
};

void CGraphsWindow::autoWndSize()
{
	CRect rect;
	GetClientRect(&rect);

	m_plot.MoveWindow(0, 0, rect.right, rect.bottom, TRUE);
}

// CGraphsWindow message handlers
void CGraphsWindow::resetPlot()
{
	m_plot.Stop();
	for(int i=0;i<m_plot.GetLineCount();i++)
	{
		m_plot.GetLine(i).RemoveAllPoints();
	}
	m_plot.Invalidate();
}

void CGraphsWindow::startPlot()
{
	resetPlot();
	m_plot.SetRate(100);//
	m_plot.Start();
}

void CGraphsWindow::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	BOOL bAxisYChanged = FALSE;
	CDialog::OnTimer(nIDEvent);

	double x=(double)time(0);
	double y0=(double)GET_RANDOM(0,100);
	double y1=(double)GET_RANDOM(0,100);
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
