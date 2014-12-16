#pragma once

#include <vector>
using namespace std;
// CMultiLineListBox

#define RGB_FOREGROUND RGB(0, 0, 0)
#define RGB_BACKGROUND RGB(255, 255, 255)
#define LISTBOX_BACKGROUND RGB(237, 237,237)

class CMultiLineListBox : public CListBox
{
	DECLARE_DYNAMIC(CMultiLineListBox)

public:
	CMultiLineListBox();
	virtual ~CMultiLineListBox();

typedef struct _LISTBOX_INFO_
{
public:
	typedef struct _SUBNODE_INFO_
	{
	public:
		CString strText;
		COLORREF fgColor;
		COLORREF bgColor;

		_SUBNODE_INFO_()
		{
			clean();
		}
		~_SUBNODE_INFO_()
		{
			clean();
		}
	protected:
		inline void clean(void)
		{
			strText.Empty();
			fgColor = RGB_FOREGROUND;
			bgColor = RGB_BACKGROUND;
		}
	}SUBNODEINFO, *PSUBNODEINFO;

public:
	vector<SUBNODEINFO*> subArray;
	CString strText;
	COLORREF fgColor;
	COLORREF bgColor;

	_LISTBOX_INFO_()
	{
		clean();
	}
	~_LISTBOX_INFO_()
	{
		clean();
	}

protected:
	inline void clean(void)
	{
		subArray.clear();
		strText.Empty();
		fgColor = RGB_FOREGROUND;
		bgColor = RGB_BACKGROUND;
	}
}LISTBOXINFO, * PLISTBOXINFO;

protected:
	static int m_nFocusIndex;
	vector<LISTBOXINFO*> m_sArray;
	int m_nMaxWidth; 
	bool m_autoScroll;
public:
	int InsertString(int nIndex, LPCTSTR pszText, COLORREF fgColor = RGB_FOREGROUND, COLORREF bgColor = RGB_BACKGROUND);
	int AddString(LPCTSTR pszText, COLORREF fgColor = RGB_FOREGROUND, COLORREF bgColor = RGB_BACKGROUND);
	void AddSubString(int nIndex, LPCTSTR pszText, COLORREF fgColor = RGB_FOREGROUND, COLORREF bgColor = RGB_BACKGROUND);
	void Clear(bool destroy = false);
protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	void UpdateItem(void);
	void GetItemHeight(int nIndex);

	void autoScroll();
protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnUpdateItem(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};


