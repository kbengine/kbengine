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

#ifndef CODE_INLINE
#include "range_list.ipp"
#endif

namespace KBEngine{	

bool RangeList::hasY = false;

//-------------------------------------------------------------------------------------
RangeList::RangeList():
size_(0),
first_x_rangeNode_(NULL),
first_y_rangeNode_(NULL),
first_z_rangeNode_(NULL),
dels_(),
updating_(0)
{
}

//-------------------------------------------------------------------------------------
RangeList::~RangeList()
{
	dels_.clear();

	if(first_x_rangeNode_)
	{
		RangeNode* pNode = first_x_rangeNode_;
		while(pNode != NULL)
		{
			RangeNode* pNextNode = pNode->pNextX();
			pNode->pRangeList(NULL);
			pNode->pPrevX(NULL);
			pNode->pNextX(NULL);
			pNode->pPrevY(NULL);
			pNode->pNextY(NULL);
			pNode->pPrevZ(NULL);
			pNode->pNextZ(NULL);

			delete pNode;

			pNode = pNextNode;
		}

		delete first_x_rangeNode_;
		first_x_rangeNode_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
bool RangeList::insert(RangeNode* pNode)
{
	// 如果链表是空的, 初始第一个和最后一个xz节点为该节点
	if(isEmpty())
	{
		first_x_rangeNode_ = pNode;

		if(RangeList::hasY)
			first_y_rangeNode_ = pNode;

		first_z_rangeNode_ = pNode;

		pNode->pPrevX(NULL);
		pNode->pNextX(NULL);
		pNode->pPrevY(NULL);
		pNode->pNextY(NULL);
		pNode->pPrevZ(NULL);
		pNode->pNextZ(NULL);
		pNode->x(pNode->xx());
		pNode->y(pNode->yy());
		pNode->z(pNode->zz());
		pNode->pRangeList(this);

		size_ = 1;
		
		// 只有一个节点不需要更新
		// update(pNode);
		return true;
	}

	pNode->old_xx(-FLT_MAX);
	pNode->old_yy(-FLT_MAX);
	pNode->old_zz(-FLT_MAX);

	pNode->x(first_x_rangeNode_->x());
	first_x_rangeNode_->pPrevX(pNode);
	pNode->pNextX(first_x_rangeNode_);
	first_x_rangeNode_ = pNode;

	if(RangeList::hasY)
	{
		pNode->y(first_y_rangeNode_->y());
		first_y_rangeNode_->pPrevY(pNode);
		pNode->pNextY(first_y_rangeNode_);
		first_y_rangeNode_ = pNode;
	}
	
	pNode->z(first_z_rangeNode_->z());
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
	pNode->flags(pNode->flags() | RANGENODE_FLAG_REMOVEING);
	pNode->onRemove();
	update(pNode);
	
	pNode->flags(pNode->flags() | RANGENODE_FLAG_REMOVED);
	if((pNode->flags() & RANGENODE_FLAG_PENDING) > 0)
	{
		std::list<RangeNode*>::iterator iter = std::find(dels_.begin(), dels_.end(), pNode);
		if(iter == dels_.end())
			dels_.push_back(pNode);
	}
	else
	{
		removeReal(pNode);
	}

	return true;
}

//-------------------------------------------------------------------------------------
void RangeList::removeDelNodes()
{
	if(dels_.size() == 0)
		return;

	std::list<RangeNode*>::iterator iter = dels_.begin();
	for(; iter != dels_.end(); iter++)
	{
		removeReal((*iter));
	}

	dels_.clear();
}

//-------------------------------------------------------------------------------------
bool RangeList::removeReal(RangeNode* pNode)
{
	if(pNode->pRangeList() == NULL)
	{
		return true;
	}

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

	if(RangeList::hasY)
	{
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
void RangeList::moveNodeX(RangeNode* pNode, float px, RangeNode* pCurrNode)
{
	if(pCurrNode != NULL)
	{
		if(pCurrNode->x() > px)
		{
			RangeNode* pPreNode = pCurrNode->pPrevX();
			pCurrNode->pPrevX(pNode);
			if(pPreNode)
			{
				pPreNode->pNextX(pNode);
				if(pNode == first_x_rangeNode_ && pNode->pNextX())
					first_x_rangeNode_ = pNode->pNextX();
			}
			else
			{
				first_x_rangeNode_ = pNode;
			}

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
			if(pNextNode != pNode)
			{
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
}

//-------------------------------------------------------------------------------------
void RangeList::moveNodeY(RangeNode* pNode, float py, RangeNode* pCurrNode)
{
	if(pCurrNode != NULL)
	{
		if(pCurrNode->y() > py)
		{
			RangeNode* pPreNode = pCurrNode->pPrevY();
			pCurrNode->pPrevY(pNode);
			if(pPreNode)
			{
				pPreNode->pNextY(pNode);
				if(pNode == first_y_rangeNode_ && pNode->pNextY())
					first_y_rangeNode_ = pNode->pNextY();
			}
			else
			{
				first_y_rangeNode_ = pNode;
			}

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
			if(pNextNode != pNode)
			{
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
}

//-------------------------------------------------------------------------------------
void RangeList::moveNodeZ(RangeNode* pNode, float pz, RangeNode* pCurrNode)
{
	if(pCurrNode != NULL)
	{
		if(pCurrNode->z() > pz)
		{
			RangeNode* pPreNode = pCurrNode->pPrevZ();
			pCurrNode->pPrevZ(pNode);
			if(pPreNode)
			{
				pPreNode->pNextZ(pNode);
				if(pNode == first_z_rangeNode_ && pNode->pNextZ())
					first_z_rangeNode_ = pNode->pNextZ();
			}
			else
			{
				first_z_rangeNode_ = pNode;
			}

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
			if(pNextNode != pNode)
			{
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
}

//-------------------------------------------------------------------------------------
void RangeList::update(RangeNode* pNode)
{
	// DEBUG_MSG(boost::format("RangeList::update:[%1%]:  (%2%  %3%  %4%)\n") % pNode % pNode->xx() % pNode->yy() % pNode->zz());
	pNode->flags(pNode->flags() | RANGENODE_FLAG_PENDING);
	++updating_;

	if(pNode->xx() != pNode->old_xx())
	{
		while(true)
		{
			RangeNode* pCurrNode = pNode->pPrevX();
			while(pCurrNode && pCurrNode != pNode && pCurrNode->x() > pNode->xx())
			{
				pNode->x(pCurrNode->x());

				// 先把节点移动过去
				moveNodeX(pNode, pNode->xx(), pCurrNode);

				if((pNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVED) <= 0)
				{
					/*
					DEBUG_MSG(boost::format("RangeList::update: [Z] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
						pNode->old_x() % pNode->old_y() % pNode->old_z() % pNode->x() % pNode->y() % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
						pNode % pCurrNode);
					*/

					pCurrNode->onNodePassX(pNode, true);
				}

				if((pCurrNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVED) <= 0)
					pNode->onNodePassX(pCurrNode, false);

				if(pCurrNode->pPrevX() == NULL)
					break;

				pCurrNode = pCurrNode->pPrevX();
			}

			pCurrNode = pNode->pNextX();
			while(pCurrNode && pCurrNode != pNode && pCurrNode->x() < pNode->xx())
			{
				pNode->x(pCurrNode->x());

				// 先把节点移动过去
				moveNodeX(pNode, pNode->xx(), pCurrNode);

				if((pNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVED) <= 0)
				{
					/*
					DEBUG_MSG(boost::format("RangeList::update: [Z] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
						pNode->old_x() % pNode->old_y() % pNode->old_z() % pNode->x() % pNode->y() % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
						pNode % pCurrNode);
					*/

					pCurrNode->onNodePassX(pNode, true);
				}

				if((pCurrNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVED) <= 0)
					pNode->onNodePassX(pCurrNode, false);

				if(pCurrNode->pNextX() == NULL)
					break;

				pCurrNode = pCurrNode->pNextX();
			}

			if((pNode->pPrevX() == NULL || (pNode->xx() >= pNode->pPrevX()->x())) && 
				(pNode->pNextX() == NULL || (pNode->xx() <= pNode->pNextX()->x())))
			{
				pNode->x(pNode->xx());
				break;
			}
		}
	}

	if(RangeList::hasY && pNode->yy() != pNode->old_yy())
	{
		while(true)
		{
			RangeNode* pCurrNode = pNode->pPrevY();
			while(pCurrNode && pCurrNode != pNode && pCurrNode->y() > pNode->yy())
			{
				pNode->y(pCurrNode->y());

				// 先把节点移动过去
				moveNodeX(pNode, pNode->yy(), pCurrNode);

				if((pNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVED) <= 0)
				{
					/*
					DEBUG_MSG(boost::format("RangeList::update: [Z] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
						pNode->old_x() % pNode->old_y() % pNode->old_z() % pNode->x() % pNode->y() % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
						pNode % pCurrNode);
					*/

					pCurrNode->onNodePassY(pNode, true);
				}

				if((pCurrNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVED) <= 0)
					pNode->onNodePassY(pCurrNode, false);

				if(pCurrNode->pPrevY() == NULL)
					break;

				pCurrNode = pCurrNode->pPrevY();
			}

			pCurrNode = pNode->pNextY();
			while(pCurrNode && pCurrNode != pNode && pCurrNode->y() < pNode->yy())
			{
				pNode->y(pCurrNode->y());

				// 先把节点移动过去
				moveNodeX(pNode, pNode->yy(), pCurrNode);

				if((pNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVED) <= 0)
				{
					/*
					DEBUG_MSG(boost::format("RangeList::update: [Z] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
						pNode->old_x() % pNode->old_y() % pNode->old_z() % pNode->x() % pNode->y() % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
						pNode % pCurrNode);
					*/

					pCurrNode->onNodePassY(pNode, true);
				}

				if((pCurrNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVED) <= 0)
					pNode->onNodePassY(pCurrNode, false);

				if(pCurrNode->pNextY() == NULL)
					break;

				pCurrNode = pCurrNode->pNextY();
			}

			if((pNode->pPrevY() == NULL || (pNode->yy() >= pNode->pPrevY()->y())) && 
				(pNode->pNextY() == NULL || (pNode->yy() <= pNode->pNextY()->y())))
			{
				pNode->y(pNode->yy());
				break;
			}
		}
	}

	if(pNode->zz() != pNode->old_zz())
	{
		while(true)
		{
			RangeNode* pCurrNode = pNode->pPrevZ();
			while(pCurrNode && pCurrNode != pNode && pCurrNode->z() > pNode->zz())
			{
				pNode->z(pCurrNode->z());

				// 先把节点移动过去
				moveNodeZ(pNode, pNode->zz(), pCurrNode);

				if((pNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVED) <= 0)
				{
					/*
					DEBUG_MSG(boost::format("RangeList::update: [Z] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
						pNode->old_x() % pNode->old_y() % pNode->old_z() % pNode->x() % pNode->y() % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
						pNode % pCurrNode);
					*/

					pCurrNode->onNodePassZ(pNode, true);
				}

				if((pCurrNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVED) <= 0)
					pNode->onNodePassZ(pCurrNode, false);

				if(pCurrNode->pPrevZ() == NULL)
					break;

				pCurrNode = pCurrNode->pPrevZ();
			}

			pCurrNode = pNode->pNextZ();
			while(pCurrNode && pCurrNode != pNode && pCurrNode->z() < pNode->zz())
			{
				pNode->z(pCurrNode->z());

				// 先把节点移动过去
				moveNodeZ(pNode, pNode->zz(), pCurrNode);

				if((pNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVED) <= 0)
				{
					/*
					DEBUG_MSG(boost::format("RangeList::update: [Z] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
						pNode->old_x() % pNode->old_y() % pNode->old_z() % pNode->x() % pNode->y() % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
						pNode % pCurrNode);
					*/

					pCurrNode->onNodePassZ(pNode, true);
				}

				if((pCurrNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVED) <= 0)
					pNode->onNodePassZ(pCurrNode, false);

				if(pCurrNode->pNextZ() == NULL)
					break;

				pCurrNode = pCurrNode->pNextZ();
			}

			if((pNode->pPrevZ() == NULL || (pNode->zz() >= pNode->pPrevZ()->z())) && 
				(pNode->pNextZ() == NULL || (pNode->zz() <= pNode->pNextZ()->z())))
			{
				pNode->z(pNode->zz());
				break;
			}
		}
	}


	pNode->resetOld();
	pNode->flags(pNode->flags() & ~RANGENODE_FLAG_PENDING);
	--updating_;

	if(updating_ == 0)
		removeDelNodes();

//	DEBUG_MSG(boost::format("RangeList::update[ x ]:[%1%]\n") % pNode);
//	first_x_rangeNode_->debugX();
//	DEBUG_MSG(boost::format("RangeList::update[ y ]:[%1%]\n") % pNode);
//	if(first_y_rangeNode_)first_y_rangeNode_->debugY();
//	DEBUG_MSG(boost::format("RangeList::update[ z ]:[%1%]\n") % pNode);
//	first_z_rangeNode_->debugZ();
}

//-------------------------------------------------------------------------------------
}
