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
updating(0)
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
			pNode = pNextNode;

			delete pNode;
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
		pNode->pRangeList(this);
		size_ = 1;
		return true;
	}

	// 设置旧坐标为负无穷大
	pNode->old_x(-FLT_MAX);
	pNode->old_y(-FLT_MAX);
	pNode->old_z(-FLT_MAX);

	// 将新加入的节点插入到第一个节点
	first_x_rangeNode_->pPrevX(pNode);
	pNode->pNextX(first_x_rangeNode_);
	first_x_rangeNode_ = pNode;

	if(RangeList::hasY)
	{
		first_y_rangeNode_->pPrevY(pNode);
		pNode->pNextY(first_y_rangeNode_);
		first_y_rangeNode_ = pNode;
	}

	first_z_rangeNode_->pPrevZ(pNode);
	pNode->pNextZ(first_z_rangeNode_);
	first_z_rangeNode_ = pNode;

	pNode->pRangeList(this);
	++size_;
	
	// 节点开始参与计算
	update(pNode);
	return true;
}

//-------------------------------------------------------------------------------------
bool RangeList::remove(RangeNode* pNode)
{
	pNode->flags(pNode->flags() | RANGENODE_FLAG_REMOVE);
	pNode->onRemove();
	update(pNode);

	std::list<RangeNode*>::iterator iter = std::find(dels_.begin(), dels_.end(), pNode);
	if(iter == dels_.end())
		dels_.push_back(pNode);
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
void RangeList::update(RangeNode* pNode)
{
	pNode->flags(pNode->flags() | RANGENODE_FLAG_PENDING);
	++updating;

	// 如果坐标有改变才进行计算
	if(pNode->x() != pNode->old_x())
	{
		RangeNode* pCurrNode = pNode->pPassPrevX(RANGENODE_FLAG_PENDING);
		
		// 向前寻找到适合的插入位置， 越是前边值越小
		while(pCurrNode && pCurrNode->x() >  pNode->x())
		{
			// 如果节点是隐藏的则不进入判断, 其他节点不会感知
			if((pNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVE) <= 0)
			{
				/*
				DEBUG_MSG(boost::format("RangeList::update: [X] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
					pNode->old_x() % pNode->old_y() % pNode->old_z() % px % py % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
					pNode % pCurrNode);
				*/

				pCurrNode->onNodePassX(pNode, true);
			}

			if((pCurrNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVE) <= 0)
			{
				pNode->onNodePassX(pCurrNode, false);
			}

			if(pCurrNode->pPassPrevX(RANGENODE_FLAG_PENDING) == NULL)
				break;

			pCurrNode = pCurrNode->pPassPrevX(RANGENODE_FLAG_PENDING);
		};
		
		// 向后寻找到适合的插入位置， 越是后边值越大
		if(pNode->pPassNextX(RANGENODE_FLAG_PENDING) && pNode->pPassNextX(RANGENODE_FLAG_PENDING)->x() <  pNode->x())
		{
			pCurrNode = pNode->pPassNextX(RANGENODE_FLAG_PENDING);

			while(pCurrNode && pCurrNode->x() <  pNode->x())
			{
				if((pNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVE) <= 0)
				{
					/*
					DEBUG_MSG(boost::format("RangeList::update: [X] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
						pNode->old_x() % pNode->old_y() % pNode->old_z() % px % py % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
						pNode % pCurrNode);
					*/

					pCurrNode->onNodePassX(pNode, true);
				}

				if((pCurrNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVE) <= 0)
				{
					pNode->onNodePassX(pCurrNode, false);
				}

				if(pCurrNode->pPassNextX(RANGENODE_FLAG_PENDING) == NULL)
					break;

				pCurrNode = pCurrNode->pPassNextX(RANGENODE_FLAG_PENDING);
			};
		}
		
		// 找到插入点
		if(pCurrNode != NULL && pCurrNode != pNode)
		{
			// 如果当前节点x大于要插入的节点
			// 那么将插入节点插入在该插入点的前面
			if(pCurrNode->x() >  pNode->x())
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
				// 插入到插入点的后面
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

	if(RangeList::hasY &&  pNode->y() != pNode->old_y())
	{
		RangeNode* pCurrNode = pNode->pPassPrevY(RANGENODE_FLAG_PENDING);

		while(pCurrNode && pCurrNode->y() > pNode->y())
		{
			if((pNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVE) <= 0)
			{
				/*
				DEBUG_MSG(boost::format("RangeList::update: [Y] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
					pNode->old_x() % pNode->old_y() % pNode->old_z() % px % py % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
					pNode % pCurrNode);
				*/

				pCurrNode->onNodePassY(pNode, true);
			}

			if((pCurrNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVE) <= 0)
				pNode->onNodePassY(pCurrNode, false);
			
			if(pCurrNode->pPassPrevY(RANGENODE_FLAG_PENDING) == NULL)
				break;

			pCurrNode = pCurrNode->pPassPrevY(RANGENODE_FLAG_PENDING);
		};
		
		if(pNode->pPassNextY(RANGENODE_FLAG_PENDING) && pNode->pPassNextY(RANGENODE_FLAG_PENDING)->y() < pNode->y())
		{
			pCurrNode = pNode->pPassNextY(RANGENODE_FLAG_PENDING);

			while(pCurrNode && pCurrNode->y() < pNode->y())
			{
				if((pNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVE) <= 0)
				{
					/*
					DEBUG_MSG(boost::format("RangeList::update: [Y] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
						pNode->old_x() % pNode->old_y() % pNode->old_z() % px % py % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
						pNode % pCurrNode);
					*/

					pCurrNode->onNodePassY(pNode, true);
				}

				if((pCurrNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVE) <= 0)
				{
					pNode->onNodePassY(pCurrNode, false);
				}

				if(pCurrNode->pPassNextY(RANGENODE_FLAG_PENDING) == NULL)
					break;

				pCurrNode = pCurrNode->pPassNextY(RANGENODE_FLAG_PENDING);
			};
		}

		if(pCurrNode != NULL && pCurrNode != pNode)
		{
			if(pCurrNode->y() > pNode->y())
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

	// 如果坐标有改变才进行计算
	if(pNode->z() != pNode->old_z())
	{
		RangeNode* pCurrNode = pNode->pPassPrevZ(RANGENODE_FLAG_PENDING);

		while(pCurrNode && pCurrNode->z() > pNode->z())
		{
			if((pNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVE) <= 0)
			{
				/*
				DEBUG_MSG(boost::format("RangeList::update: [Z] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
					pNode->old_x() % pNode->old_y() % pNode->old_z() % px % py % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
					pNode % pCurrNode);
				*/

				pCurrNode->onNodePassZ(pNode, true);
			}

			if((pCurrNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVE) <= 0)
				pNode->onNodePassZ(pCurrNode, false);
			
			if(pCurrNode->pPassPrevZ(RANGENODE_FLAG_PENDING) == NULL)
				break;

			pCurrNode = pCurrNode->pPassPrevZ(RANGENODE_FLAG_PENDING);
		};
		
		if(pNode->pPassNextZ(RANGENODE_FLAG_PENDING) && pNode->pPassNextZ(RANGENODE_FLAG_PENDING)->z() < pNode->z())
		{
			pCurrNode = pNode->pPassNextZ(RANGENODE_FLAG_PENDING);

			while(pCurrNode && pCurrNode->z() < pNode->z())
			{
				if((pNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVE) <= 0)
				{
					/*
					DEBUG_MSG(boost::format("RangeList::update: [Z] node_%10%(%1%, %2%, %3%)->(%4%, %5%, %6%), passNode_%11%(%7%, %8%, %9%)\n") %
						pNode->old_x() % pNode->old_y() % pNode->old_z() % px % py % pz % pCurrNode->x() % pCurrNode->y() % pCurrNode->z() %
						pNode % pCurrNode);
					*/

					pCurrNode->onNodePassZ(pNode, true);
				}

				if((pCurrNode->flags() & RANGENODE_FLAG_HIDE_OR_REMOVE) <= 0)
				{
					pNode->onNodePassZ(pCurrNode, false);
				}

				if(pCurrNode->pPassNextZ(RANGENODE_FLAG_PENDING) == NULL)
					break;

				pCurrNode = pCurrNode->pPassNextZ(RANGENODE_FLAG_PENDING);
			};
		}

		if(pCurrNode != NULL && pCurrNode != pNode)
		{
			if(pCurrNode->z() > pNode->z())
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

	pNode->resetOld();
	pNode->flags(pNode->flags() & ~RANGENODE_FLAG_PENDING);

	--updating;

	if(updating == 0)
		removeDelNodes();
	
	/*
	DEBUG_MSG(boost::format("RangeList::update[ x ]:[%1%]\n") % pNode);
	first_x_rangeNode_->debugX();
	DEBUG_MSG(boost::format("RangeList::update[ y ]:[%1%]\n") % pNode);
	if(first_y_rangeNode_)first_y_rangeNode_->debugY();
	DEBUG_MSG(boost::format("RangeList::update[ z ]:[%1%]\n") % pNode);
	first_z_rangeNode_->debugZ();
	*/
}

//-------------------------------------------------------------------------------------
}
