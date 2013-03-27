/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "range_node.hpp"
#include "range_list.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
RangeList::RangeList():
size_(0),
first_x_rangeNode_(NULL),
first_y_rangeNode_(NULL),
first_z_rangeNode_(NULL)
{
}

//-------------------------------------------------------------------------------------
RangeList::~RangeList()
{
}

//-------------------------------------------------------------------------------------
bool RangeList::insert(RangeNode* pNode)
{
	float x = pNode->x();
	float y = pNode->y();
	float z = pNode->z();
	
	// 如果链表是空的, 初始第一个和最后一个xz节点为该节点
	if(isEmpty())
	{
		first_x_rangeNode_ = pNode;
		first_y_rangeNode_ = pNode;
		first_z_rangeNode_ = pNode;

		pNode->pPrevX(NULL);
		pNode->pNextX(NULL);
		pNode->pPrevY(NULL);
		pNode->pNextY(NULL);
		pNode->pPrevZ(NULL);
		pNode->pNextZ(NULL);
		size_ = 1;
		return true;
	}

	// 如果x小于第一个节点则设置当前节点为第一个节点
	if(x < first_x_rangeNode_->x())
	{
		pNode->pPrevX(NULL);
		pNode->pNextX(first_x_rangeNode_);
		first_x_rangeNode_->pPrevX(pNode);
		first_x_rangeNode_ = pNode;
	}
	else
	{
		if(size_ == 1)
		{
			if(x < first_z_rangeNode_->x())
			{
				first_x_rangeNode_->pPrevX(pNode);
				pNode->pNextX(first_x_rangeNode_);
				first_x_rangeNode_ = pNode;
			}
			else
			{
				first_x_rangeNode_->pNextX(pNode);
				pNode->pPrevX(first_x_rangeNode_);
			}
		}
		else
		{
			// 否则遍历找到合适的插入位置
			RangeNode* pCurrInsertPTNode = first_x_rangeNode_->pNextX();
			while(pCurrInsertPTNode != NULL)
			{
				if(x < pCurrInsertPTNode->x())
				{
					pCurrInsertPTNode->pPrevX()->pNextX(pNode);
					pNode->pPrevX(pCurrInsertPTNode->pPrevX());
					pNode->pNextX(pCurrInsertPTNode);
					pCurrInsertPTNode->pPrevX(pNode);
					break;
				}

				if(pCurrInsertPTNode->pNextX())
				{
					pCurrInsertPTNode = pCurrInsertPTNode->pNextX();
				}
				else
				{
					// 如果是最后一个节点则挂接上去
					pCurrInsertPTNode->pNextX(pNode);
					pNode->pPrevX(pCurrInsertPTNode);
					pNode->pNextX(NULL);
					break;
				}
			}
		}
	}

	// 如果y小于第一个节点则设置当前节点为第一个节点
	if(y < first_y_rangeNode_->y())
	{
		pNode->pPrevY(NULL);
		pNode->pNextY(first_y_rangeNode_);
		first_y_rangeNode_->pPrevY(pNode);
		first_y_rangeNode_ = pNode;
	}
	else
	{
		if(size_ == 1)
		{
			if(y < first_y_rangeNode_->y())
			{
				first_y_rangeNode_->pPrevY(pNode);
				pNode->pNextY(first_y_rangeNode_);
				first_y_rangeNode_ = pNode;
			}
			else
			{
				first_y_rangeNode_->pNextY(pNode);
				pNode->pPrevY(first_y_rangeNode_);
			}
		}
		else
		{
			// 否则遍历找到合适的插入位置
			RangeNode* pCurrInsertPTNode = first_y_rangeNode_->pNextY();
			while(pCurrInsertPTNode != NULL)
			{
				if(y < pCurrInsertPTNode->y())
				{
					pCurrInsertPTNode->pPrevY()->pNextY(pNode);
					pNode->pPrevY(pCurrInsertPTNode->pPrevY());
					pNode->pNextY(pCurrInsertPTNode);
					pCurrInsertPTNode->pPrevY(pNode);
					break;
				}

				if(pCurrInsertPTNode->pNextY())
				{
					pCurrInsertPTNode = pCurrInsertPTNode->pNextY();
				}
				else
				{
					// 如果是最后一个节点则挂接上去
					pCurrInsertPTNode->pNextY(pNode);
					pNode->pPrevY(pCurrInsertPTNode);
					pNode->pNextY(NULL);
					break;
				}
			}
		}
	}

	// 如果x小于第一个节点则设置当前节点为第一个节点
	if(z < first_z_rangeNode_->z())
	{
		pNode->pPrevZ(NULL);
		pNode->pNextZ(first_z_rangeNode_);
		first_z_rangeNode_->pPrevZ(pNode);
		first_z_rangeNode_ = pNode;
	}
	else
	{
		if(size_ == 1)
		{
			if(z < first_z_rangeNode_->z())
			{
				first_z_rangeNode_->pPrevZ(pNode);
				pNode->pNextZ(first_z_rangeNode_);
				first_z_rangeNode_ = pNode;
			}
			else
			{
				first_z_rangeNode_->pNextZ(pNode);
				pNode->pPrevZ(first_z_rangeNode_);
			}
		}
		else
		{
			// 否则遍历找到合适的插入位置
			RangeNode* pCurrInsertPTNode = first_z_rangeNode_->pNextZ();
			while(pCurrInsertPTNode != NULL)
			{
				if(z < pCurrInsertPTNode->z())
				{
					pCurrInsertPTNode->pPrevZ()->pNextZ(pNode);
					pNode->pPrevZ(pCurrInsertPTNode->pPrevZ());
					pNode->pNextZ(pCurrInsertPTNode);
					pCurrInsertPTNode->pPrevZ(pNode);
					break;
				}

				if(pCurrInsertPTNode->pNextZ())
				{
					pCurrInsertPTNode = pCurrInsertPTNode->pNextZ();
				}
				else
				{
					// 如果是最后一个节点则挂接上去
					pCurrInsertPTNode->pNextZ(pNode);
					pNode->pPrevZ(pCurrInsertPTNode);
					pNode->pNextZ(NULL);
					break;
				}
			}
		}
	}

	pNode->pRangeList(this);
	++size_;
	return true;
}

//-------------------------------------------------------------------------------------
bool RangeList::remove(RangeNode* pNode)
{
	// 如果是第一个节点
	if(first_x_rangeNode_ == pNode)
	{
		first_x_rangeNode_ = first_x_rangeNode_->pNextX();

		if(first_x_rangeNode_)
		{
			first_x_rangeNode_->pPrevX(NULL);
		}
	}
	else
	{
		pNode->pPrevX()->pNextX(pNode->pNextX());

		if(pNode->pNextX())
			pNode->pNextX()->pPrevX(pNode->pPrevX());
	}

	// 如果是第一个节点
	if(first_y_rangeNode_ == pNode)
	{
		first_y_rangeNode_ = first_y_rangeNode_->pNextY();

		if(first_y_rangeNode_)
		{
			first_y_rangeNode_->pPrevY(NULL);
		}
	}
	else
	{
		pNode->pPrevY()->pNextY(pNode->pNextY());

		if(pNode->pNextY())
			pNode->pNextY()->pPrevY(pNode->pPrevY());
	}

	// 如果是第一个节点
	if(first_z_rangeNode_ == pNode)
	{
		first_z_rangeNode_ = first_z_rangeNode_->pNextZ();

		if(first_z_rangeNode_)
		{
			first_z_rangeNode_->pPrevZ(NULL);
		}
	}
	else
	{
		pNode->pPrevZ()->pNextZ(pNode->pNextZ());

		if(pNode->pNextZ())
			pNode->pNextZ()->pPrevZ(pNode->pPrevZ());
	}

	pNode->pPrevX(NULL);
	pNode->pNextX(NULL);
	pNode->pPrevY(NULL);
	pNode->pNextY(NULL);
	pNode->pPrevZ(NULL);
	pNode->pNextZ(NULL);
	pNode->pRangeList(NULL);

	--size_;
	return true;
}

//-------------------------------------------------------------------------------------
void RangeList::update(RangeNode* pNode)
{
	float px = pNode->x();
	float py = pNode->y();
	float pz = pNode->z();

	if(px != pNode->old_x())
	{
		RangeNode* pCurrNode = pNode->pPrevX();

		while(pCurrNode && pCurrNode->x() > px)
		{
			pCurrNode->onNodePassX(pNode, true);
			pNode->onNodePassX(pCurrNode, false);
			
			if(pCurrNode->pPrevX() == NULL)
				break;

			pCurrNode = pCurrNode->pPrevX();
		};
		
		if(pCurrNode != NULL)
		{
			RangeNode* pPreNode = pCurrNode->pPrevX();
			pCurrNode->pPrevX(pNode);
			if(pPreNode)
				pPreNode->pNextX(pNode);
			else
				first_x_rangeNode_ = pNode;

			if(pNode->pPrevX())
				pNode->pPrevX()->pNextX(pNode->pNextX());

			if(pNode->pNextX())
				pNode->pNextX()->pPrevX(pNode->pPrevX());

			pNode->pPrevX(pPreNode);
			pNode->pNextX(pCurrNode);
		}
	}

	if(py != pNode->old_y())
	{
		RangeNode* pCurrNode = pNode->pPrevY();

		while(pCurrNode && pCurrNode->y() > py)
		{
			pCurrNode->onNodePassY(pNode, true);
			pNode->onNodePassY(pCurrNode, false);
			
			if(pCurrNode->pPrevY() == NULL)
				break;

			pCurrNode = pCurrNode->pPrevY();
		};
		
		if(pCurrNode != NULL)
		{
			RangeNode* pPreNode = pCurrNode->pPrevY();
			pCurrNode->pPrevY(pNode);
			if(pPreNode)
				pPreNode->pNextY(pNode);
			else
				first_y_rangeNode_ = pNode;

			if(pNode->pPrevY())
				pNode->pPrevY()->pNextY(pNode->pNextY());

			if(pNode->pNextY())
				pNode->pNextY()->pPrevY(pNode->pPrevY());

			pNode->pPrevY(pPreNode);
			pNode->pNextY(pCurrNode);
		}
	}

	if(pz != pNode->old_z())
	{
		RangeNode* pCurrNode = pNode->pPrevZ();

		while(pCurrNode && pCurrNode->z() > pz)
		{
			pCurrNode->onNodePassZ(pNode, true);
			pNode->onNodePassZ(pCurrNode, false);
			
			if(pCurrNode->pPrevZ() == NULL)
				break;

			pCurrNode = pCurrNode->pPrevZ();
		};
		
		if(pCurrNode != NULL)
		{
			RangeNode* pPreNode = pCurrNode->pPrevZ();
			pCurrNode->pPrevZ(pNode);
			if(pPreNode)
				pPreNode->pNextZ(pNode);
			else
				first_z_rangeNode_ = pNode;

			if(pNode->pPrevZ())
				pNode->pPrevZ()->pNextZ(pNode->pNextZ());

			if(pNode->pNextZ())
				pNode->pNextZ()->pPrevZ(pNode->pPrevZ());

			pNode->pPrevZ(pPreNode);
			pNode->pNextZ(pCurrNode);
		}
	}

	pNode->resetOld();
}

//-------------------------------------------------------------------------------------
}
