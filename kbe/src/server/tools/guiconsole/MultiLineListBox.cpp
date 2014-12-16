// MultiLineListBox.cpp : implementation file
//

#include "stdafx.h"
#include "MultiLineListBox.h"

// CMultiLineListBox
#define MSG_UPDATEITEM WM_USER + 0x1001
#define ITEM_HEIGHT 20

int CMultiLineListBox::m_nFocusIndex = -1;

IMPLEMENT_DYNAMIC(CMultiLineListBox, CListBox)

CMultiLineListBox::CMultiLineListBox()
{
	m_sArray.clear();
	m_nMaxWidth   =   0; 
	m_autoScroll = true;
}

CMultiLineListBox::~CMultiLineListBox() 
{
	Clear(true);
}

BEGIN_MESSAGE_MAP(CMultiLineListBox, CListBox)
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(MSG_UPDATEITEM, &CMultiLineListBox::OnUpdateItem)
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

void CMultiLineListBox::Clear(bool destroy)
{
	if(!destroy)
		this->ResetContent();

	vector<LISTBOXINFO*>::const_iterator iter1 = m_sArray.begin();
	for(; iter1 != m_sArray.end(); ++iter1)
	{
		LISTBOXINFO* pNode = *iter1;
		vector<LISTBOXINFO::SUBNODEINFO*>::const_iterator iter2 = pNode->subArray.begin();
		for(; iter2 != pNode->subArray.end(); ++iter2)
		{
			LISTBOXINFO::SUBNODEINFO* pSubNode = *iter2;
			delete pSubNode;
			pSubNode = NULL;
		}
		delete pNode;
		pNode = NULL;
	}
	m_sArray.clear();
}

int CMultiLineListBox::InsertString(int nIndex, LPCTSTR pszText, COLORREF fgColor, COLORREF bgColor)
{
	LISTBOXINFO* pListBox = new LISTBOXINFO;
	ASSERT(pListBox);

	ASSERT((nIndex >= 0) && (nIndex <= GetCount()));

	pListBox->strText = pszText;
	pListBox->fgColor = fgColor;
	pListBox->bgColor = bgColor;

	m_sArray.insert(m_sArray.begin() + nIndex, pListBox);
	autoScroll();

	return CListBox::InsertString(nIndex, pszText);
}

int CMultiLineListBox::AddString(LPCTSTR pszText, COLORREF fgColor, COLORREF bgColor)
{
	LISTBOXINFO* pListBox = new LISTBOXINFO;
	ASSERT(pListBox);

	pListBox->strText = pszText;
	pListBox->fgColor = fgColor;
	pListBox->bgColor = bgColor;

	m_sArray.push_back(pListBox);

	int ret = CListBox::AddString(pszText);

	SCROLLINFO scrollInfo;
	memset(&scrollInfo,   0,   sizeof(SCROLLINFO));
	scrollInfo.cbSize   =   sizeof(SCROLLINFO);
	scrollInfo.fMask   =   SIF_ALL;
	GetScrollInfo(SB_VERT,   &scrollInfo,   SIF_ALL); 

	int   nScrollWidth   =   0;
	if(GetCount() > 1 && ((int)scrollInfo.nMax >= (int)scrollInfo.nPage))
	{
		nScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
	} 

	SIZE   sSize;
	CClientDC   myDC(this); 

	CFont* pListBoxFont = GetFont();
	if(pListBoxFont != NULL)
	{
		CFont* pOldFont =  myDC.SelectObject(pListBoxFont); 
		GetTextExtentPoint32(myDC.m_hDC, pszText, ::wcslen(pszText), &sSize);

		m_nMaxWidth   =   max(m_nMaxWidth,   (int)sSize.cx); 
		SetHorizontalExtent(m_nMaxWidth   +   3); 
		myDC.SelectObject(pOldFont);   
	}

	autoScroll();
	return ret;
}

void CMultiLineListBox::AddSubString(int nIndex, LPCTSTR pszText, COLORREF fgColor, COLORREF bgColor)
{
	ASSERT((nIndex >=0) && (nIndex < GetCount()));

	ASSERT(!m_sArray.empty());

	LISTBOXINFO* pListBox = m_sArray.at(nIndex);
	ASSERT(pListBox);

	LISTBOXINFO::SUBNODEINFO* pSubNode = new LISTBOXINFO::SUBNODEINFO;
	ASSERT(pSubNode);

	pSubNode->strText = pszText;
	pSubNode->fgColor = fgColor;
	pSubNode->bgColor = bgColor;
	pListBox->subArray.push_back(pSubNode);
	autoScroll();
}

void CMultiLineListBox::autoScroll()
{
	if(m_autoScroll)
	{
		::SendMessage(m_hWnd, WM_VSCROLL, SB_BOTTOM, 0);
	}
}

// CMultiLineListBox message handlers

void CMultiLineListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// TODO:  Add your code to determine the size of specified item
	ASSERT(lpMeasureItemStruct->CtlType == ODT_LISTBOX);

	lpMeasureItemStruct->itemHeight = ITEM_HEIGHT;
}

void CMultiLineListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO:  Add your code to draw the specified item
	ASSERT(lpDrawItemStruct->CtlType == ODT_LISTBOX);

	int nIndex = lpDrawItemStruct->itemID;

	if((!m_sArray.empty())  && (nIndex < static_cast<int>(m_sArray.size())))
	{
		CDC dc;
		dc.Attach(lpDrawItemStruct->hDC);

		// Save these value to restore them when done drawing.
		COLORREF crOldTextColor = dc.GetTextColor();
		COLORREF crOldBkColor = dc.GetBkColor();

		// If this item is selected, set the background color 
		// and the text color to appropriate values. Also, erase
		// rect by filling it with the background color.
		CRect rc(lpDrawItemStruct->rcItem);

		LISTBOXINFO* pListBox = m_sArray.at(nIndex);
		ASSERT(pListBox);

		if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
			(lpDrawItemStruct->itemState & ODS_SELECTED))
		{
			dc.SetTextColor(pListBox->bgColor);
			dc.SetBkColor(pListBox->fgColor);
			dc.FillSolidRect(&rc, pListBox->fgColor);

			// Draw item the text.
			CRect rect(rc);
			int nItemCount = 1;
			nItemCount += static_cast<int>(pListBox->subArray.size());
			int nItemHeight = rc.Height() / nItemCount;
			rect.bottom = rect.top + nItemHeight;
			dc.DrawText(pListBox->strText, pListBox->strText.GetLength(), CRect(rect.left + 5, rect.top, rect.right, rect.bottom), DT_SINGLELINE | DT_VCENTER);

			// Draw subitem the text.
			CRect rcItem;
			rcItem.SetRectEmpty();
			rcItem.top = rect.bottom;
			rcItem.left = rect.left;
			rcItem.right = rect.right;
			rcItem.bottom = rcItem.top + nItemHeight;

			vector<LISTBOXINFO::SUBNODEINFO*>::const_iterator iter = pListBox->subArray.begin();
			for(; iter != pListBox->subArray.end(); ++iter)
			{
				LISTBOXINFO::SUBNODEINFO* pSubNode = *iter;
				dc.SetTextColor(pSubNode->fgColor);
				dc.SetBkColor(pSubNode->bgColor);
				dc.FillSolidRect(&rcItem, pSubNode->bgColor);

				CRect rectItem(rcItem);
				rectItem.left += 22;
				dc.DrawText(pSubNode->strText, pSubNode->strText.GetLength(), &rectItem, DT_SINGLELINE | DT_VCENTER);

				rcItem.top = rcItem.bottom;
				rcItem.bottom = rcItem.top + nItemHeight;
			}

			dc.DrawFocusRect(rc);	// Draw focus rect
		}
		else
		{
			dc.SetTextColor(pListBox->fgColor);
			dc.SetBkColor(pListBox->bgColor);
			dc.FillSolidRect(&rc, pListBox->bgColor);

			// Draw the text.
			CRect rect(rc);
			rect.left += 5;
			dc.DrawText(pListBox->strText, pListBox->strText.GetLength(), &rect, DT_SINGLELINE | DT_VCENTER);
		}

		// Reset the background color and the text color back to their
		// original values.
		dc.SetTextColor(crOldTextColor);
		dc.SetBkColor(crOldBkColor);

		dc.Detach();
	}
}

BOOL CMultiLineListBox::OnEraseBkgnd(CDC* pDC)
{
	// Set listbox background color
	CRect rc;
	GetClientRect(&rc);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	ASSERT(memDC.GetSafeHdc());

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rc.Width(), rc.Height());
	ASSERT(bmp.GetSafeHandle());

	CBitmap* pOldbmp = (CBitmap*)memDC.SelectObject(&bmp);

	memDC.FillSolidRect(rc, LISTBOX_BACKGROUND); // Set background color which you want
	pDC->BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldbmp);
	bmp.DeleteObject();
	memDC.DeleteDC();

	return TRUE;
}

void CMultiLineListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	CListBox::OnKeyDown(nChar, nRepCnt, nFlags);

	UpdateItem();
}

void CMultiLineListBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CListBox::OnLButtonDown(nFlags, point);

	UpdateItem();
}

void CMultiLineListBox::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CListBox::OnMouseMove(nFlags, point);

	UpdateItem();
}

void CMultiLineListBox::UpdateItem()
{
	// If per item height not equal, you must calculate area between the current focus item with last one,
	// otherwise you must calculate area between the current focus item with previously focus item.
	int nIndex = GetCurSel();
	if((CB_ERR != nIndex) && (m_nFocusIndex != nIndex))
	{
		PostMessage(MSG_UPDATEITEM, (WPARAM)m_nFocusIndex, (LPARAM)nIndex);
		m_nFocusIndex = nIndex; // Set current select focus index
	}
}

LRESULT CMultiLineListBox::OnUpdateItem(WPARAM wParam, LPARAM lParam)
{
	int nPreIndex = static_cast<int>(wParam);
	int nCurIndex = static_cast<int>(lParam);
	if(m_sArray.size() == 0)
		return 0;

	if(-1 != nPreIndex)
	{
		SetItemHeight(nPreIndex, ITEM_HEIGHT);
	}

	if(-1 != nCurIndex)
	{
		int nItemCount = 1;
		LISTBOXINFO* pListBox = m_sArray.at(nCurIndex);
		ASSERT(pListBox);
		nItemCount += static_cast<int>(pListBox->subArray.size());
		SetItemHeight(nCurIndex, ITEM_HEIGHT * nItemCount);
	}

	Invalidate(); // Update item
	return 0;
}

void CMultiLineListBox::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	bool done = false;
	switch(nSBCode)
	{
	case SB_THUMBPOSITION:	//ÍÏ¶¯»¬¿é
	case SB_LINELEFT:		//µã»÷×ó±ßµÄ¼ýÍ·
	case SB_LINERIGHT:		//µã»÷ÓÒ±ßµÄ¼ýÍ·
		done = true;
		break;
	} 

	CListBox::OnVScroll(nSBCode, nPos, pScrollBar); 

	if(done)
	{
		int nMax = GetScrollLimit(SB_VERT);
		int pos = GetScrollPos(SB_VERT);
		m_autoScroll = (pos >= nMax);
	}
}
