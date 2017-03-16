#pragma once

#include <vector>

#define MAX_POINTS 2048 //1 pixel per point, 2048 is the ordinary screen pixels count

using namespace std;
class CLine
{
public:
	BOOL        m_bDraw;     //plot or not
	COLORREF	m_color;     //
	int         m_iStyle;     //solid,dash
	int         m_iThick;     //1pixel,2pixel
	CLine()
	{
		m_bDraw = TRUE;
		m_color = RGB(0,0,0);
		m_iStyle = PS_SOLID;
		m_iThick = 1;
		InitializeCriticalSection ( & g_cs ) ;
	}
	CLine(int iStyle, COLORREF color, int iThick);
	CLine(const CLine& line);
	CLine& operator = (const CLine& line);
	~CLine();
	void AddPoint(double x, double y);
	double GetPointX(int nIndex);
	double GetPointY(int nIndex);
	int    GetPointCount();
	void RemoveAllPoints();
	void RemoveUselessPoints();

private:
	vector<double> m_dXValue;
	vector<double> m_dYValue;
	CRITICAL_SECTION g_cs ;

};
class CAxis
{
public:
	BOOL        m_bDraw;     //plot or not
	BOOL        m_bGrid;     //Draw grid line or not
	double      m_dMinValue;
	double      m_dMaxValue;
	double	    m_dValuePerPixel; //how much a pixel represented

	COLORREF	m_color;     //
	int         m_iStyle;     //
	int         m_iThick;     //
	CAxis()
	{
		m_bDraw = TRUE;
		m_bGrid = TRUE;
		m_dMinValue = 0;
		m_dMaxValue = 10000;
		m_color = RGB(0,0,0);
		m_iStyle = PS_SOLID;
		m_iThick = 1;
	}
};
// CLineChartCtrl

class CLineChartCtrl : public CStatic
{
	DECLARE_DYNAMIC(CLineChartCtrl)

public:
	CLineChartCtrl();
	virtual ~CLineChartCtrl();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	void Start();
	void Stop();

	void CalcLayout();
	void AddLine(int iStyle, COLORREF color, int iThick = 1);
	CLine& GetLine(int nIndex);
	int    GetLineCount();

	void SetRate(int nRate);
	void SetBkColor(COLORREF clrBkColor);
	CAxis& GetAxisY();
	CAxis& GetAxisX();


private:
	void DrawAll(CDC* pDC);
	void DrawBackground(CDC* pDC);
	void DrawAxises(CDC* pDC);
	void DrawLines(CDC* pDC);

	CRect m_rectCtrl;                       // the static rect of chart control
	CRect m_rectAxisY;		                // Y axis rect
	CRect m_rectAxisX;		                // X axis rect
	CRect m_rectPlot;                       // m_rectCtrl - margin - axisRect
	CPoint m_ptInLine[MAX_POINTS];          // points used to draw a line


	int			m_iMarginLeft;			   // left margin in pixels
	int			m_iMarginRight;			   // right margin in pixels
	int			m_iMarginTop;			   // top margin in pixels
	int			m_iMarginBottom;		   // bottom margin in pixels

	CAxis m_axisY;       //Y axis
	CAxis m_axisX;     //X axis

	vector<CLine> m_lines;

	COLORREF m_clrBkColor; //plot control background color
	int      m_nRate;      //update interval
	
};


