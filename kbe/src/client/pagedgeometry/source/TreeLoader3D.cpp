/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

#include "TreeLoader3D.h"
#include "PagedGeometry.h"
#include "PropertyMaps.h"

#include <OgreRoot.h>
#include <OgreException.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
using namespace Ogre;

namespace Forests {

TreeLoader3D::TreeLoader3D(PagedGeometry *geom, const TBounds &bounds)
{
	//Calculate grid size
	TreeLoader3D::geom = geom;
	pageSize = geom->getPageSize();

	//Make sure the bounds are aligned with PagedGeometry's grid, so the TreeLoader's grid tiles will have a 1:1 relationship
	actualBounds = bounds;
	gridBounds = bounds;
	gridBounds.left = pageSize * Math::Floor((gridBounds.left - geom->getBounds().left) / pageSize) + geom->getBounds().left;
	gridBounds.top = pageSize * Math::Floor((gridBounds.top - geom->getBounds().top) / pageSize) + geom->getBounds().top;
	gridBounds.right = pageSize * Math::Ceil((gridBounds.right - geom->getBounds().left) / pageSize) + geom->getBounds().left;
	gridBounds.bottom = pageSize * Math::Ceil((gridBounds.bottom - geom->getBounds().top) / pageSize) + geom->getBounds().top;

	//Calculate page grid size
	pageGridX = Math::Ceil(gridBounds.width() / pageSize) + 1;
	pageGridZ = Math::Ceil(gridBounds.height() / pageSize) + 1;

	//Reset color map
	colorMap = NULL;
	colorMapFilter = MAPFILTER_NONE;

	//Default scale range
	maximumScale = 2.0f;
	minimumScale = 0.0f;
}

TreeLoader3D::~TreeLoader3D()
{
	//Delete all page grids
	PageGridListIterator i;
	for (i = pageGridList.begin(); i != pageGridList.end(); ++i){
		delete[] i->second;
	}
	pageGridList.clear();
}

void TreeLoader3D::addTree(Entity *entity, const Ogre::Vector3 &position, Degree yaw, Real scale, void* userData)
{
	//First convert the coordinate to PagedGeometry's local system
	#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
	Vector3 pos = geom->_convertToLocal(position);
	#else
	Vector3 pos = position;
	#endif

	//Check that the tree is within bounds (DEBUG)
	#ifdef _DEBUG
	const Real smallVal = 0.01f;
	if (pos.x < actualBounds.left-smallVal || pos.x > actualBounds.right+smallVal || pos.z < actualBounds.top-smallVal || pos.z > actualBounds.bottom+smallVal)
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Tree position is out of bounds", "TreeLoader::addTree()");
	if (scale < minimumScale || scale > maximumScale)
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Tree scale out of range", "TreeLoader::addTree()");
	#endif

	//If the tree is slightly out of bounds (due to imprecise coordinate conversion), fix it
	if (pos.x < actualBounds.left)
		pos.x = actualBounds.left;
	else if (pos.x > actualBounds.right)
		pos.x = actualBounds.right;

	if (pos.z < actualBounds.top)
		pos.z = actualBounds.top;
	else if (pos.z > actualBounds.bottom)
		pos.z = actualBounds.bottom;
	
	//Find the appropriate page grid for the entity
	PageGridListIterator i;
	i = pageGridList.find(entity);

	std::vector<TreeDef> *pageGrid;
	if (i != pageGridList.end()){
		//If it exists, get the page grid
		pageGrid = i->second;
	} else {
		//If it does not exist, create a new page grid
		pageGrid = new std::vector<TreeDef>[pageGridX * pageGridZ];

		//Register the new page grid in the pageGridList for later retrieval
		pageGridList.insert(PageGridListValue(entity, pageGrid));
	}

	//Calculate the gridbounds-relative position of the tree
	Real xrel = pos.x - gridBounds.left;
	Real zrel = pos.z - gridBounds.top;

	//Get the appropriate grid element based on the new tree's position
	int pageX = Math::Floor(xrel / pageSize);
	int pageZ = Math::Floor(zrel / pageSize);
	std::vector<TreeDef> &treeList = _getGridPage(pageGrid, pageX, pageZ);

	//Create the new tree
	TreeDef tree;
	tree.yPos = pos.y;
	tree.xPos = 65535 * (xrel - (pageX * pageSize)) / pageSize;
	tree.zPos = 65535 * (zrel - (pageZ * pageSize)) / pageSize;
	tree.rotation = 255 * (yaw.valueDegrees() / 360.0f);
	tree.scale = 255 * ((scale - minimumScale) / maximumScale);

#ifdef PAGEDGEOMETRY_USER_DATA
	tree.userData = userData;
#endif

	//Add it to the tree list
	treeList.push_back(tree);

	//Rebuild geometry if necessary
	geom->reloadGeometryPage(pos);
}

#ifdef PAGEDGEOMETRY_USER_DATA
std::vector<void*>
#else
void
#endif
TreeLoader3D::deleteTrees(const Vector3 &position, Real radius, Entity *type)
{
	//First convert the coordinate to PagedGeometry's local system
	#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
	Vector3 pos = geom->_convertToLocal(position);
	#else
	Vector3 pos = position;
	#endif

#ifdef PAGEDGEOMETRY_USER_DATA
	//Keep a list of user-defined data associated with deleted trees
	std::vector<void*> deletedUserData;
#endif
	
	//If the position is slightly out of bounds, fix it
	if (pos.x < actualBounds.left)
		pos.x = actualBounds.left;
	else if (pos.x > actualBounds.right)
		pos.x = actualBounds.right;

	if (pos.z < actualBounds.top)
		pos.z = actualBounds.top;
	else if (pos.x > actualBounds.bottom)
		pos.z = actualBounds.bottom;

	//Determine the grid blocks which might contain the requested trees
	int minPageX = Math::Floor(((pos.x-radius) - gridBounds.left) / pageSize);
	int minPageZ = Math::Floor(((pos.z-radius) - gridBounds.top) / pageSize);
	int maxPageX = Math::Floor(((pos.x+radius) - gridBounds.left) / pageSize);
	int maxPageZ = Math::Floor(((pos.z+radius) - gridBounds.top) / pageSize);
	Real radiusSq = radius * radius;

	if (minPageX < 0) minPageX = 0; else if (minPageX >= pageGridX) minPageX = pageGridX-1;
	if (minPageZ < 0) minPageZ = 0; else if (minPageZ >= pageGridZ) minPageZ = pageGridZ-1;
	if (maxPageX < 0) maxPageX = 0; else if (maxPageX >= pageGridX) maxPageX = pageGridX-1;
	if (maxPageZ < 0) maxPageZ = 0; else if (maxPageZ >= pageGridZ) maxPageZ = pageGridZ-1;

	PageGridListIterator it, end;
	if (type == NULL){
		//Scan all entity types
		it = pageGridList.begin();
		end = pageGridList.end();
	} else {
		//Only scan entities of the given type
		it = pageGridList.find(type);
		assert(it != pageGridList.end());
		end = it; ++end;
	}

	//Scan all the grid blocks
	while (it != end){
		std::vector<TreeDef> *pageGrid = it->second;

		for (int tileZ = minPageZ; tileZ <= maxPageZ; ++tileZ){
			for (int tileX = minPageX; tileX <= maxPageX; ++tileX){
				bool modified = false;

				//Scan all trees in grid block
				std::vector<TreeDef> &treeList = _getGridPage(pageGrid, tileX, tileZ);
				uint32 i = 0;
				while (i < treeList.size()){
					//Get tree distance
					float distX = (gridBounds.left + (tileX * pageSize) + ((Real)treeList[i].xPos / 65535) * pageSize) - pos.x;
					float distZ = (gridBounds.top + (tileZ * pageSize) + ((Real)treeList[i].zPos / 65535) * pageSize) - pos.z;
					float distSq = distX * distX + distZ * distZ;

					if (distSq <= radiusSq){
#ifdef PAGEDGEOMETRY_USER_DATA
						deletedUserData.push_back(treeList[i].userData);
#endif
						//If it's within the radius, delete it
						treeList[i] = treeList.back();
						treeList.pop_back();
						modified = true;
					}
					else
						++i;
				}

				//Rebuild geometry if necessary
				if (modified){
					Vector3 pos(gridBounds.left + ((0.5f + tileX) * pageSize), 0, gridBounds.top + ((0.5f + tileZ) * pageSize));
					geom->reloadGeometryPage(pos);
				}
			}
		}

		++it;
	}

#ifdef PAGEDGEOMETRY_USER_DATA
	return deletedUserData;
#endif
}

#ifdef PAGEDGEOMETRY_USER_DATA
std::vector<void*>
#else
void
#endif
TreeLoader3D::deleteTrees(TBounds area, Ogre::Entity *type)
{
#ifdef PAGEDGEOMETRY_USER_DATA
	//Keep a list of user-defined data associated with deleted trees
	std::vector<void*> deletedUserData;
#endif

	//If the area is slightly out of bounds, fix it
	if (area.left < actualBounds.left) area.left = actualBounds.left;
	else if (area.left > actualBounds.right) area.left = actualBounds.right;
	if (area.top < actualBounds.top) area.top = actualBounds.top;
	else if (area.top > actualBounds.bottom) area.top = actualBounds.bottom;
	if (area.right < actualBounds.left) area.right = actualBounds.left;
	else if (area.right > actualBounds.right) area.right = actualBounds.right;
	if (area.bottom < actualBounds.top) area.bottom = actualBounds.top;
	else if (area.bottom > actualBounds.bottom) area.bottom = actualBounds.bottom;

	//Determine the grid blocks which might contain the requested trees
	int minPageX = Math::Floor((area.left - gridBounds.left) / pageSize);
	int minPageZ = Math::Floor((area.top - gridBounds.top) / pageSize);
	int maxPageX = Math::Floor((area.right - gridBounds.left) / pageSize);
	int maxPageZ = Math::Floor((area.bottom - gridBounds.top) / pageSize);

	if (minPageX < 0) minPageX = 0; else if (minPageX >= pageGridX) minPageX = pageGridX-1;
	if (minPageZ < 0) minPageZ = 0; else if (minPageZ >= pageGridZ) minPageZ = pageGridZ-1;
	if (maxPageX < 0) maxPageX = 0; else if (maxPageX >= pageGridX) maxPageX = pageGridX-1;
	if (maxPageZ < 0) maxPageZ = 0; else if (maxPageZ >= pageGridZ) maxPageZ = pageGridZ-1;

	PageGridListIterator it, end;
	if (type == NULL){
		//Scan all entity types
		it = pageGridList.begin();
		end = pageGridList.end();
	} else {
		//Only scan entities of the given type
		it = pageGridList.find(type);
		assert(it != pageGridList.end());
		end = it; ++end;
	}

	//Scan all the grid blocks
	while (it != end){
		std::vector<TreeDef> *pageGrid = it->second;

		for (int tileZ = minPageZ; tileZ <= maxPageZ; ++tileZ){
			for (int tileX = minPageX; tileX <= maxPageX; ++tileX){
				bool modified = false;

				//Scan all trees in grid block
				std::vector<TreeDef> &treeList = _getGridPage(pageGrid, tileX, tileZ);
				uint32 i = 0;
				while (i < treeList.size()){
					//Check if tree is in bounds
					float posX = (gridBounds.left + (tileX * pageSize) + ((Real)treeList[i].xPos / 65535) * pageSize);
					float posZ = (gridBounds.top + (tileZ * pageSize) + ((Real)treeList[i].zPos / 65535) * pageSize);
					if (posX >= area.left && posX <= area.right && posZ >= area.top && posZ <= area.bottom) {
						//If so, delete it
#ifdef PAGEDGEOMETRY_USER_DATA
						deletedUserData.push_back(treeList[i].userData);
#endif
						//If it's within the radius, delete it
						treeList[i] = treeList.back();
						treeList.pop_back();
						modified = true;
					}
					else
						++i;
				}

				//Rebuild geometry if necessary
				if (modified){
					Vector3 pos(gridBounds.left + ((0.5f + tileX) * pageSize), 0, gridBounds.top + ((0.5f + tileZ) * pageSize));
					geom->reloadGeometryPage(pos);
				}
			}
		}

		++it;
	}

#ifdef PAGEDGEOMETRY_USER_DATA
	return deletedUserData;
#endif
}


#ifdef PAGEDGEOMETRY_USER_DATA
std::vector<void*> TreeLoader3D::findTrees(const Ogre::Vector3 &position, Real radius, Entity *type)
{
	//First convert the coordinate to PagedGeometry's local system
	#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
	Vector3 pos = geom->_convertToLocal(position);
	#else
	Vector3 pos = position;
	#endif

	//Keep a list of user-defined data associated with deleted trees
	std::vector<void*> foundUserData;


	//If the position is slightly out of bounds, fix it
	if (pos.x < actualBounds.left)
		pos.x = actualBounds.left;
	else if (pos.x > actualBounds.right)
		pos.x = actualBounds.right;

	if (pos.z < actualBounds.top)
		pos.z = actualBounds.top;
	else if (pos.x > actualBounds.bottom)
		pos.z = actualBounds.bottom;

	//Determine the grid blocks which might contain the requested trees
	int minPageX = Math::Floor(((pos.x-radius) - gridBounds.left) / pageSize);
	int minPageZ = Math::Floor(((pos.z-radius) - gridBounds.top) / pageSize);
	int maxPageX = Math::Floor(((pos.x+radius) - gridBounds.left) / pageSize);
	int maxPageZ = Math::Floor(((pos.z+radius) - gridBounds.top) / pageSize);
	Real radiusSq = radius * radius;

	if (minPageX < 0) minPageX = 0; else if (minPageX >= pageGridX) minPageX = pageGridX-1;
	if (minPageZ < 0) minPageZ = 0; else if (minPageZ >= pageGridZ) minPageZ = pageGridZ-1;
	if (maxPageX < 0) maxPageX = 0; else if (maxPageX >= pageGridX) maxPageX = pageGridX-1;
	if (maxPageZ < 0) maxPageZ = 0; else if (maxPageZ >= pageGridZ) maxPageZ = pageGridZ-1;

	PageGridListIterator it, end;
	if (type == NULL){
		//Scan all entity types
		it = pageGridList.begin();
		end = pageGridList.end();
	} else {
		//Only scan entities of the given type
		it = pageGridList.find(type);
		assert(it != pageGridList.end());
		end = it; ++end;
	}

	//Scan all the grid blocks
	while (it != end){
		std::vector<TreeDef> *pageGrid = it->second;

		for (int tileZ = minPageZ; tileZ <= maxPageZ; ++tileZ){
			for (int tileX = minPageX; tileX <= maxPageX; ++tileX){
				bool modified = false;

				//Scan all trees in grid block
				std::vector<TreeDef> &treeList = _getGridPage(pageGrid, tileX, tileZ);
				uint32 i = 0;
				while (i < treeList.size()){
					//Get tree distance
					float distX = (gridBounds.left + (tileX * pageSize) + ((Real)treeList[i].xPos / 65535) * pageSize) - pos.x;
					float distZ = (gridBounds.top + (tileZ * pageSize) + ((Real)treeList[i].zPos / 65535) * pageSize) - pos.z;
					float distSq = distX * distX + distZ * distZ;

					if (distSq <= radiusSq){
						foundUserData.push_back(treeList[i].userData);
					}
					else
						++i;
				}
			}
		}

		++it;
	}

	return foundUserData;
}
#endif


void TreeLoader3D::setColorMap(const String &mapFile, MapChannel channel)
{
	if (colorMap){
		colorMap->unload();
		colorMap = NULL;
	}
	if (mapFile != ""){
		colorMap = ColorMap::load(mapFile, channel);
		colorMap->setFilter(colorMapFilter);
	}
}

void TreeLoader3D::setColorMap(TexturePtr map, MapChannel channel)
{
	if (colorMap){
		colorMap->unload();
		colorMap = NULL;
	}
	if (map.isNull() == false){
		colorMap = ColorMap::load(map, channel);
		colorMap->setFilter(colorMapFilter);
	}
}

void TreeLoader3D::loadPage(PageInfo &page)
{
	//Calculate x/z indexes for the grid array
	page.xIndex -= Math::Floor(gridBounds.left / pageSize);
	page.zIndex -= Math::Floor(gridBounds.top / pageSize);

	//Check if the requested page is in bounds
	if (page.xIndex < 0 || page.zIndex < 0 || page.xIndex >= pageGridX || page.zIndex >= pageGridZ)
		return;

	//For each tree type...
	PageGridListIterator i;
	for (i = pageGridList.begin(); i != pageGridList.end(); ++i){
		//Get the appropriate tree list for the specified page
		std::vector<TreeDef> *pageGrid = i->second;
		std::vector<TreeDef> &treeList = _getGridPage(pageGrid, page.xIndex, page.zIndex);
		Entity *entity = i->first;

		//Load the listed trees into the page
		std::vector<TreeDef>::iterator o;
		for (o = treeList.begin(); o != treeList.end(); ++o){
			//Get position
			Vector3 pos;
			pos.x = page.bounds.left + ((Real)o->xPos / 65535) * pageSize;
			pos.z = page.bounds.top + ((Real)o->zPos / 65535) * pageSize;
			pos.y = o->yPos;

			//Get rotation
			Degree angle((Real)o->rotation * (360.0f / 255));
			Quaternion rot(angle, Vector3::UNIT_Y);

			//Get scale
			Vector3 scale;
			scale.y = (Real)o->scale * (maximumScale / 255) + minimumScale;
			scale.x = scale.y;
			scale.z = scale.y;

			//Get color
			ColourValue color;
			if (colorMap)
				color = colorMap->getColorAt_Unpacked(pos.x, pos.z, actualBounds);
			else
				color = ColourValue::White;

			addEntity(entity, pos, rot, scale, color);
		}
	}
}

TreeIterator3D TreeLoader3D::getTrees()
{
	return TreeIterator3D(this);
}



TreeIterator3D::TreeIterator3D(TreeLoader3D *trees)
{
	TreeIterator3D::trees = trees;

	//Test if the GridList has anything in it
	if (trees->pageGridList.empty()) {
		// If not, set hasMore to false and return.
		hasMore = false;
		return;
	}

	//Setup iterators
	currentGrid = trees->pageGridList.begin();
	currentX = 0; currentZ = 0;
	currentTreeList = &trees->_getGridPage(currentGrid->second, currentX, currentZ);
	currentTree = currentTreeList->begin();
	hasMore = true;

	//If there's not tree in the first page, keep looking
	if (currentTree == currentTreeList->end())
		moveNext();

	//Read the first tree's data
	_readTree();

	//Read one more tree, so peekNext() will properly return the first item, while the system
	//actually is looking ahead one item (this way it can detect the end of the list before
	//it's too late)
	if (hasMore)
		moveNext();
}

TreeRef TreeIterator3D::getNext()
{
	TreeRef tree = peekNext();
	moveNext();
	return tree;
}

void TreeIterator3D::moveNext()
{
	//Out of bounds check
	if (!hasMore)
		OGRE_EXCEPT(1, "Cannot read past end of TreeIterator list", "TreeIterator::moveNext()");

	//Preserve the last tree
	prevTreeDat = currentTreeDat;

	//Increment iterators to the next tree
	if (currentTree != currentTreeList->end())
		++currentTree;
	while (currentTree == currentTreeList->end()){
		if (++currentX >= trees->pageGridX){
			currentX = 0;
			if (++currentZ >= trees->pageGridZ){
				++currentGrid;
				if (currentGrid == trees->pageGridList.end()){
					//No more trees left
					hasMore = false;
					return;
				}
				currentX = 0; currentZ = 0;
			}
		}

		currentTreeList = &trees->_getGridPage(currentGrid->second, currentX, currentZ);
		currentTree = currentTreeList->begin();
	}

	//Read the current tree data
	_readTree();
}

void TreeIterator3D::_readTree()
{
	TreeLoader3D::TreeDef treeDef = *currentTree;

	//Get position
	Real boundsLeft = trees->gridBounds.left + trees->pageSize * currentX;
	Real boundsTop = trees->gridBounds.top + trees->pageSize * currentZ;
	currentTreeDat.position.x = boundsLeft + ((Real)treeDef.xPos / 65535) * trees->pageSize;
	currentTreeDat.position.z = boundsTop + ((Real)treeDef.zPos / 65535) * trees->pageSize;
	currentTreeDat.position.y = treeDef.yPos;

	//Get rotation
	currentTreeDat.yaw = Degree((Real)treeDef.rotation * (360.0f / 255));

	//Get scale
	currentTreeDat.scale = (Real)treeDef.scale * (trees->maximumScale / 255) + trees->minimumScale;

	//Get entity
	currentTreeDat.entity = currentGrid->first;
}
}
