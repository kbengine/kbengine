// ColorListBox.cpp : implementation file
//

#include "stdafx.h"
#include "ColorListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorListBox

BEGIN_MESSAGE_MAP(CColorListBox, CListBox)
	//{{AFX_MSG_MAP(CColorListBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorListBox message handlers

void CColorListBox::AddEntry(LPCTSTR *label, COLORREF color, int nIndex /* 0 */)
{
	int index = InsertString(nIndex, *label);
	SetItemData(index,color);
}

void CColorListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
// all items are of fixed size
// must use LBS_OWNERDRAWVARIABLE for this to work

	int nItem = lpMIS->itemID;
	CPaintDC dc(this);
	CString sLabel;
	CRect rcLabel;

	GetText( nItem, sLabel );
	GetItemRect(nItem, rcLabel);

// Using the flags below, calculate the required rectangle for 
// the text and set the item height for this specific item based
// on the return value (new height).

	int itemHeight = dc.DrawText( sLabel, -1, rcLabel, DT_WORDBREAK | DT_CALCRECT );
	lpMIS->itemHeight = itemHeight;
}

void CColorListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	COLORREF rColor = (COLORREF)lpDIS->itemData; // RGB in item data

	CString sLabel;
	GetText(lpDIS->itemID, sLabel);

	// item selected
	if ((lpDIS->itemState & ODS_SELECTED) &&
		(lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		// draw color box
		CBrush colorBrush(rColor);
		CRect colorRect = lpDIS->rcItem;

		// draw label background
		CBrush labelBrush(::GetSysColor(COLOR_HIGHLIGHT));
		CRect labelRect = lpDIS->rcItem;
		pDC->FillRect(&labelRect,&labelBrush);

		// draw label text
		COLORREF colorTextSave;
		COLORREF colorBkSave;

		colorTextSave = pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		colorBkSave = pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		pDC->DrawText( sLabel, -1, &lpDIS->rcItem, DT_WORDBREAK );

		pDC->SetTextColor(colorTextSave);
		pDC->SetBkColor(colorBkSave);

		return;
	}

	// item brought into box
	if (lpDIS->itemAction & ODA_DRAWENTIRE)
	{
		CBrush brush(rColor);
		CRect rect = lpDIS->rcItem;
		pDC->SetBkColor(rColor);
		pDC->FillRect(&rect,&brush);
		pDC->DrawText( sLabel, -1, &lpDIS->rcItem, DT_WORDBREAK );

		return;
	}

	// item deselected
	if (!(lpDIS->itemState & ODS_SELECTED) &&
		(lpDIS->itemAction & ODA_SELECT))
	{
		CRect rect = lpDIS->rcItem;
		CBrush brush(rColor);
		pDC->SetBkColor(rColor);
		pDC->FillRect(&rect,&brush);
		pDC->DrawText( sLabel, -1, &lpDIS->rcItem, DT_WORDBREAK );

		return;
	}
}
