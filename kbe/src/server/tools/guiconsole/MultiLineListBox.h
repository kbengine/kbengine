#if !defined(AFX_MULTILINELISTBOX_H__BF8E1EC2_833B_11D2_8BE7_A813DF000000__INCLUDED_)
#define AFX_MULTILINELISTBOX_H__BF8E1EC2_833B_11D2_8BE7_A813DF000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MultiLineListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMultiLineListBox

class CMultiLineListBox : public CListBox
{
// Attributes
public:

// Operations
public:
	void AddEntry(LPCTSTR lpszItem, COLORREF color, int nIndex=0);

	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiLineListBox)
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CMultiLineListBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTILINELISTBOX_H__BF8E1EC2_833B_11D2_8BE7_A813DF000000__INCLUDED_)
