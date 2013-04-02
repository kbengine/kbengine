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

	pNode->old_x(-FLT_MAX);
	pNode->old_y(-FLT_MAX);
	pNode->old_z(-FLT_MAX);

	first_x_rangeNode_->pPrevX(pNode);
	pNode->pNextX(first_x_rangeNode_);
	first_x_rangeNode_ = pNode;

	first_y_rangeNode_->pPrevY(pNode);
	pNode->pNextY(first_y_rangeNode_);
	first_y_rangeNode_ = pNode;

	first_z_rangeNode_->pPrevZ(pNode);
	pNode->pNextZ(first_z_rangeNode_);
	first_z_rangeNode_ = pNode;

	pNode->pRangeList(this);
	++size_;

	update(pNode);
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
			if((pNode->flags() & RANGENODE_FLAG_HIDE) <= 0)
			{
				/*
				DEBUG_MSG(boost::format("RangeList::update: [X] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
					pNode->old_x() % pNode->old_y() % pNode->old_z() % px % py % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
					pNode % pCurrNode);
				*/

				pCurrNode->onNodePassX(pNode, true);
			}

			if((pCurrNode->flags() & RANGENODE_FLAG_HIDE) <= 0)
			{
				pNode->onNodePassX(pCurrNode, false);
			}

			if(pCurrNode->pPrevX() == NULL)
				break;

			pCurrNode = pCurrNode->pPrevX();
		};
		
		if(pNode->pNextX() && pNode->pNextX()->x() < px)
		{
			pCurrNode = pNode->pNextX();

			while(pCurrNode && pCurrNode->x() < px)
			{
				if((pNode->flags() & RANGENODE_FLAG_HIDE) <= 0)
				{
					/*
					DEBUG_MSG(boost::format("RangeList::update: [X] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
						pNode->old_x() % pNode->old_y() % pNode->old_z() % px % py % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
						pNode % pCurrNode);
					*/

					pCurrNode->onNodePassX(pNode, true);
				}

				if((pCurrNode->flags() & RANGENODE_FLAG_HIDE) <= 0)
				{
					pNode->onNodePassX(pCurrNode, false);
				}

				if(pCurrNode->pNextX() == NULL)
					break;

				pCurrNode = pCurrNode->pNextX();
			};
		}

		if(pCurrNode != NULL)
		{
			if(pCurrNode->x() > px)
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
			else
			{
				RangeNode* pNextNode = pCurrNode->pNextX();
				pCurrNode->pNextX(pNode);
				if(pNextNode)
					pNextNode->pPrevX(pNode);

				if(pNode->pPrevX())
					pNode->pPrevX()->pNextX(pNode->pNextX());

				if(pNode->pNextX())
				{
					pNode->pNextX()->pPrevX(pNode->pPrevX());
				
					if(pNode == first_x_rangeNode_)
						first_x_rangeNode_ = pNode->pNextX();
				}

				pNode->pPrevX(pCurrNode);
				pNode->pNextX(pNextNode);
			}
		}
	}

	if(py != pNode->old_y())
	{
		RangeNode* pCurrNode = pNode->pPrevY();

		while(pCurrNode && pCurrNode->y() > py)
		{
			if((pNode->flags() & RANGENODE_FLAG_HIDE) <= 0)
			{
				/*
				DEBUG_MSG(boost::format("RangeList::update: [Y] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
					pNode->old_x() % pNode->old_y() % pNode->old_z() % px % py % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
					pNode % pCurrNode);
				*/

				pCurrNode->onNodePassY(pNode, true);
			}

			if((pCurrNode->flags() & RANGENODE_FLAG_HIDE) <= 0)
				pNode->onNodePassY(pCurrNode, false);
			
			if(pCurrNode->pPrevY() == NULL)
				break;

			pCurrNode = pCurrNode->pPrevY();
		};
		
		if(pNode->pNextY() && pNode->pNextY()->y() < py)
		{
			pCurrNode = pNode->pNextY();

			while(pCurrNode && pCurrNode->y() < py)
			{
				if((pNode->flags() & RANGENODE_FLAG_HIDE) <= 0)
				{
					/*
					DEBUG_MSG(boost::format("RangeList::update: [Y] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
						pNode->old_x() % pNode->old_y() % pNode->old_z() % px % py % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
						pNode % pCurrNode);
					*/

					pCurrNode->onNodePassY(pNode, true);
				}

				if((pCurrNode->flags() & RANGENODE_FLAG_HIDE) <= 0)
				{
					pNode->onNodePassY(pCurrNode, false);
				}

				if(pCurrNode->pNextY() == NULL)
					break;

				pCurrNode = pCurrNode->pNextY();
			};
		}

		if(pCurrNode != NULL)
		{
			if(pCurrNode->y() > py)
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
			else
			{
				RangeNode* pNextNode = pCurrNode->pNextY();
				pCurrNode->pNextY(pNode);
				if(pNextNode)
					pNextNode->pPrevY(pNode);

				if(pNode->pPrevY())
					pNode->pPrevY()->pNextY(pNode->pNextY());

				if(pNode->pNextY())
				{
					pNode->pNextY()->pPrevY(pNode->pPrevY());
				
					if(pNode == first_y_rangeNode_)
						first_y_rangeNode_ = pNode->pNextY();
				}

				pNode->pPrevY(pCurrNode);
				pNode->pNextY(pNextNode);
			}
		}
	}

	if(pz != pNode->old_z())
	{
		RangeNode* pCurrNode = pNode->pPrevZ();

		while(pCurrNode && pCurrNode->z() > pz)
		{
			if((pNode->flags() & RANGENODE_FLAG_HIDE) <= 0)
			{
				/*
				DEBUG_MSG(boost::format("RangeList::update: [Z] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
					pNode->old_x() % pNode->old_y() % pNode->old_z() % px % py % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
					pNode % pCurrNode);
				*/

				pCurrNode->onNodePassZ(pNode, true);
			}

			if((pCurrNode->flags() & RANGENODE_FLAG_HIDE) <= 0)
				pNode->onNodePassZ(pCurrNode, false);
			
			if(pCurrNode->pPrevZ() == NULL)
				break;

			pCurrNode = pCurrNode->pPrevZ();
		};
		
		if(pNode->pNextZ() && pNode->pNextZ()->z() < pz)
		{
			pCurrNode = pNode->pNextZ();

			while(pCurrNode && pCurrNode->z() < pz)
			{
				if((pNode->flags() & RANGENODE_FLAG_HIDE) <= 0)
				{
					/*
					DEBUG_MSG(boost::format("RangeList::update: [Z] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
						pNode->old_x() % pNode->old_y() % pNode->old_z() % px % py % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
						pNode % pCurrNode);
					*/

					pCurrNode->onNodePassZ(pNode, true);
				}

				if((pCurrNode->flags() & RANGENODE_FLAG_HIDE) <= 0)
				{
					pNode->onNodePassZ(pCurrNode, false);
				}

				if(pCurrNode->pNextZ() == NULL)
					break;

				pCurrNode = pCurrNode->pNextZ();
			};
		}

		if(pCurrNode != NULL)
		{
			if(pCurrNode->z() > pz)
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
			else
			{
				RangeNode* pNextNode = pCurrNode->pNextZ();
				pCurrNode->pNextZ(pNode);
				if(pNextNode)
					pNextNode->pPrevZ(pNode);

				if(pNode->pPrevZ())
					pNode->pPrevZ()->pNextZ(pNode->pNextZ());

				if(pNode->pNextZ())
				{
					pNode->pNextZ()->pPrevZ(pNode->pPrevZ());
				
					if(pNode == first_z_rangeNode_)
						first_z_rangeNode_ = pNode->pNextZ();
				}

				pNode->pPrevZ(pCurrNode);
				pNode->pNextZ(pNextNode);
			}
		}
	}

	pNode->resetOld();
}

//-------------------------------------------------------------------------------------
}
