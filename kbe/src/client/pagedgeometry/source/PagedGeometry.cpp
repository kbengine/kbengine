/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
	1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

//PagedGeometry.h
//Main source file for the PagedGeometry engine.
//-------------------------------------------------------------------------------------

#include "PagedGeometry.h"
#include "StaticBillboardSet.h"

#include <OgreRoot.h>
#include <OgreTimer.h>
#include <OgreCamera.h>
#include <OgreVector3.h>
using namespace Ogre;
using namespace std;

namespace Forests {

//-------------------------------------------------------------------------------------
PagedGeometry::PagedGeometry(Camera* cam, const Real pageSize, Ogre::RenderQueueGroupID queue) : mRenderQueue(queue)
{
	//Setup camera, scene manager, and scene node
	if (cam)
	{
		sceneCam = cam;
		sceneMgr = sceneCam->getSceneManager();
		oldCamPos = sceneCam->getDerivedPosition();

		#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
		rootNode = sceneMgr->getRootSceneNode()->createChildSceneNode();	//Create PagedGeometry's root node
		#else
		rootNode = sceneMgr->getRootSceneNode();
		#endif
	} else {
		sceneCam = NULL;
		sceneMgr = NULL;
		rootNode = NULL;
		oldCamPos = Vector3::ZERO;
	}
	lastSceneCam = NULL;
	lastOldCamPos = Vector3::ZERO;

	#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
	//Setup default coordinate system
	coordinateSystemQuat = Quaternion::IDENTITY;
	#endif

	//Init. timer
	timer.reset();
	lastTime = 0;

	//Setup page size / bounds
	PagedGeometry::pageSize = pageSize;
	m_bounds = TBounds(0, 0, 0, 0);

	//Misc.
	pageLoader = NULL;
	geometryAllowedVisible = true;
	tempdir=""; // empty for current working directory
	shadersEnabled = true; // enable shaders by default
}

PagedGeometry::~PagedGeometry()
{
	#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
	//Remove PagedGeometry's coordinate system node
	if (rootNode)
		sceneMgr->destroySceneNode(rootNode->getName());
	#endif

	//Remove all page managers and the geometry associated with them
	removeDetailLevels();
}

void PagedGeometry::setTempDir(Ogre::String dir)
{
   tempdir = dir;
}

void PagedGeometry::setPageLoader(PageLoader *loader)
{
	pageLoader = loader;
}

void PagedGeometry::setCamera(Camera *cam)
{
	if (cam == NULL){
		//Simply set camera to null
		sceneCam = NULL;
	} else {
		if (sceneMgr && cam->getSceneManager() != sceneMgr)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "The specified camera is from the wrong SceneManager", "PagedGeometry::setCamera()");

		if (cam == lastSceneCam){
			//If the cache values for this camera are preserved, use them
			std::swap(oldCamPos, lastOldCamPos);
			std::swap(sceneCam, lastSceneCam);
		} else {
			lastSceneCam = sceneCam;
			lastOldCamPos = oldCamPos;
			sceneCam = cam;
		}
		
		//If sceneMgre is NULL (this only occurs the first time a camera is set),
		//then set the scene manager (it won't change after this point).
		if (sceneMgr == NULL)
			sceneMgr = sceneCam->getSceneManager();

		//If rootNode is NULL (this also only occurs the first time a camera is set),
		//the create a scene node (it won't change after this point) for the coordinate
		//system translations.
		if (rootNode == NULL){
			#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
			rootNode = sceneMgr->getRootSceneNode()->createChildSceneNode();
			rootNode->setOrientation(coordinateSystemQuat);
			#else
			rootNode = sceneMgr->getRootSceneNode();
			#endif
		}
	}
}

#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
void PagedGeometry::setCoordinateSystem(Vector3 up, Vector3 right)
{
	up.z = -up.z;
	Vector3 forward = right.crossProduct(up);
	coordinateSystemQuat = Quaternion(right, up, forward);

	if (rootNode)
		rootNode->setOrientation(coordinateSystemQuat);
}
#endif

#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
Vector3 PagedGeometry::_convertToLocal(const Vector3 &globalVec) const
{
	assert(getSceneNode());
	//Convert from the given global position to the local coordinate system of PagedGeometry's root scene node.
	return (getSceneNode()->getOrientation().Inverse() * globalVec);
}
#else
//Default coordinate system - no conversion
Vector3 PagedGeometry::_convertToLocal(const Vector3 &globalVec) const
{
	return globalVec;
}
#endif

void PagedGeometry::setPageSize(Real size)
{
	if (!managerList.empty())
		OGRE_EXCEPT(0, "PagedGeometry::setPageSize() cannot be called after detail levels have been added. Call removeDetailLevels() first.", "PagedGeometry::setPageSize()");

	pageSize = size;
}

void PagedGeometry::setInfinite()
{
	if (!managerList.empty())
		OGRE_EXCEPT(0, "PagedGeometry::setInfinite() cannot be called after detail levels have been added. Call removeDetailLevels() first.", "PagedGeometry::setInfinite()");

	m_bounds = TBounds(0, 0, 0, 0);
}

void PagedGeometry::setBounds(TBounds bounds)
{
	if (!managerList.empty())
		OGRE_EXCEPT(0, "PagedGeometry::setBounds() cannot be called after detail levels have been added. Call removeDetailLevels() first.", "PagedGeometry::setBounds()");
	if (!Math::RealEqual(bounds.width(), bounds.height(), 0.01f))
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Bounds must be square", "PagedGeometry::setBounds()");
	if (bounds.width() <= 0 || bounds.height() <=0)
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Bounds must have positive width and height", "PagedGeometry::setBounds()");

	m_bounds = bounds;
}

TBounds PagedGeometry::convertAABToTBounds( const Ogre::AxisAlignedBox & aab ) const
{
	Vector3 minimum = _convertToLocal(aab.getMinimum());
	Vector3 maximum = _convertToLocal(aab.getMaximum());
	return TBounds (minimum.x, minimum.z, maximum.x, maximum.z);
}

void PagedGeometry::removeDetailLevels()
{
	std::list<GeometryPageManager *>::iterator it;

	//Delete all the page managers
	for (it = managerList.begin(); it != managerList.end(); ++it){
		GeometryPageManager *mgr = *it;
		delete mgr;
	}

	//Clear the page manager list
	managerList.clear();
}

void PagedGeometry::update()
{
	//If no camera has been set, then return without doing anything
	if (sceneCam == NULL)
		return;

	//Calculate time since last update
	unsigned long deltaTime, tmp;
	tmp = timer.getMilliseconds();
	deltaTime = tmp - lastTime;
	lastTime = tmp;

	//Get camera position and speed
	Vector3 camPos = _convertToLocal(sceneCam->getDerivedPosition());
	Vector3 camSpeed;	//Speed in units-per-millisecond
	if (deltaTime == 0){
		camSpeed.x = 0;
		camSpeed.y = 0;
		camSpeed.z = 0;
	} else {
		camSpeed = (camPos - oldCamPos) / deltaTime;
	}
	oldCamPos = camPos;
	
	if (pageLoader != 0){
		//Update the PageLoader
		pageLoader->frameUpdate();

		//Update all the page managers
		bool enableCache = true;
		std::list<GeometryPageManager *>::iterator it;
		GeometryPageManager *prevMgr = NULL;
		for (it = managerList.begin(); it != managerList.end(); ++it){
			GeometryPageManager *mgr = *it;
			mgr->update(deltaTime, camPos, camSpeed, enableCache, prevMgr);
			prevMgr = mgr;
		}
	}

	//Update misc. subsystems
	StaticBillboardSet::updateAll(_convertToLocal(getCamera()->getDerivedDirection()));
}

void PagedGeometry::reloadGeometry()
{
	assert(pageLoader);

	std::list<GeometryPageManager *>::iterator it;
	for (it = managerList.begin(); it != managerList.end(); ++it){
		GeometryPageManager *mgr = *it;
		mgr->reloadGeometry();
	}
}

void PagedGeometry::reloadGeometryPage(const Vector3 &point)
{
	if (!pageLoader)
		return;

#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
	point = _convertToLocal(point);
#endif

	std::list<GeometryPageManager *>::iterator it;
	for (it = managerList.begin(); it != managerList.end(); ++it)
	{
		GeometryPageManager *mgr = *it;
		mgr->reloadGeometryPage(
#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
			_convertToLocal(point)
#else
			point
#endif
			);
	}
}

void PagedGeometry::reloadGeometryPages(const Ogre::Vector3 &center, Real radius)
{
	if (!pageLoader)
		return;

	std::list<GeometryPageManager *>::iterator it;
	for (it = managerList.begin(); it != managerList.end(); ++it)
	{
		GeometryPageManager *mgr = *it;
		mgr->reloadGeometryPages(
#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
			_convertToLocal(center)
#else
			center
#endif
			, radius);
	}
}

void PagedGeometry::reloadGeometryPages(const TBounds & area)
{
	if (!pageLoader)
		return;

	TBounds localArea = area;

	if (localArea.left > localArea.right) std::swap(localArea.left, localArea.right);
	if (localArea.top > localArea.bottom) std::swap(localArea.top, localArea.bottom);

	std::list<GeometryPageManager *>::iterator it;
	for (it = managerList.begin(); it != managerList.end(); ++it){
		GeometryPageManager *mgr = *it;
		mgr->reloadGeometryPages(localArea);
	}
}

void PagedGeometry::preloadGeometry(const TBounds & area)
{
	if (!pageLoader)
		return;

	TBounds localArea = area;

	if (localArea.left > localArea.right) std::swap(localArea.left, localArea.right);
	if (localArea.top > localArea.bottom) std::swap(localArea.top, localArea.bottom);

	std::list<GeometryPageManager *>::iterator it;
	for (it = managerList.begin(); it != managerList.end(); ++it){
		GeometryPageManager *mgr = *it;
		mgr->preloadGeometry(localArea);
	}
}

void PagedGeometry::resetPreloadedGeometry()
{
	if (!pageLoader)
		return;

	std::list<GeometryPageManager *>::iterator it;
	for (it = managerList.begin(); it != managerList.end(); ++it){
		GeometryPageManager *mgr = *it;
		mgr->resetPreloadedGeometry();
	}
}

void PagedGeometry::_addDetailLevel(GeometryPageManager *mgr, Real maxRange, Real transitionLength)
{
	//Calculate the near range
	Real minRange = 0;
	if (!managerList.empty()){
		GeometryPageManager *lastMgr = managerList.back();
		minRange = lastMgr->getFarRange();
	}

	//Error check
	if (maxRange <= minRange){
		OGRE_EXCEPT(1, "Closer detail levels must be added before farther ones", "PagedGeometry::addDetailLevel()");
	}

	//Setup the new manager
	mgr->setNearRange(minRange);
	mgr->setFarRange(maxRange);
	mgr->setTransition(transitionLength);

	managerList.push_back(mgr);
}

void  PagedGeometry::setCustomParam(string entity, string paramName, float paramValue)
{
	setCustomParam(entity + "." + paramName, paramValue);
}

void  PagedGeometry::setCustomParam(string paramName, float paramValue)
{
	customParam[paramName] = paramValue;
}

float PagedGeometry::getCustomParam(string entity, string paramName, float defaultParamValue) const
{
	return getCustomParam(entity + "." + paramName, defaultParamValue);
}

float PagedGeometry::getCustomParam(string paramName, float defaultParamValue) const
{
	std::map<string, float>::const_iterator it;
	it = customParam.find(paramName);
	if (it != customParam.end()) {
		float x = it->second;
		return x;
	}
	else
		return defaultParamValue;
}

Ogre::RenderQueueGroupID PagedGeometry::getRenderQueue() const
{
	return mRenderQueue;
}

//-------------------------------------------------------------------------------------

GeometryPageManager::GeometryPageManager(PagedGeometry *mainGeom)
: mainGeom(mainGeom)
	, cacheTimer(0) // Reset the cache timer
	, scrollBuffer(NULL)
	, geomGrid(NULL)
	, geomGridX(0)
	, geomGridZ(0)
{
	//Use default cache speeds
	setCacheSpeed();

	//No transition default
	setTransition(0);
}

GeometryPageManager::~GeometryPageManager()
{
	//Delete GeometryPage's
	for (int x = 0; x < geomGridX; ++x)
		for (int z = 0; z < geomGridZ; ++z)
			delete _getGridPage(x, z);

	//Deallocate arrays
	if(geomGrid)
		delete[] geomGrid;
	if(scrollBuffer)
		delete[] scrollBuffer;
}

void GeometryPageManager::update(unsigned long deltaTime, Vector3 &camPos, Vector3 &camSpeed, bool &enableCache, GeometryPageManager *prevManager)
{
	//-- Cache new geometry pages --

	//Cache 1 page ahead of the view ranges
	const Real cacheDist = farTransDist + mainGeom->getPageSize();
	const Real cacheDistSq = cacheDist * cacheDist;
	
	//First calculate the general area where the pages will be processed
	// 0,0 is the left top corner of the bounding box
	int x1 = Math::Floor(((camPos.x - cacheDist) - gridBounds.left) / mainGeom->getPageSize());
	int x2 = Math::Floor(((camPos.x + cacheDist) - gridBounds.left) / mainGeom->getPageSize());
	int z1 = Math::Floor(((camPos.z - cacheDist) - gridBounds.top) / mainGeom->getPageSize());
	int z2 = Math::Floor(((camPos.z + cacheDist) - gridBounds.top) / mainGeom->getPageSize());
	if(scrollBuffer)
	{
		//Check if the page grid needs to be scrolled
		int shiftX = 0, shiftZ = 0;
		if (x1 < 0) shiftX = x1; else if (x2 >= geomGridX-1) shiftX = x2 - (geomGridX-1);
		if (z1 < 0) shiftZ = z1; else if (z2 >= geomGridZ-1) shiftZ = z2 - (geomGridZ-1);
		if (shiftX != 0 || shiftZ != 0)
		{
			//Scroll grid
			_scrollGridPages(shiftX, shiftZ);

			//Update grid bounds and processing area
			gridBounds.left += shiftX * mainGeom->getPageSize();
			gridBounds.right += shiftX * mainGeom->getPageSize();
			gridBounds.top += shiftZ * mainGeom->getPageSize();
			gridBounds.bottom += shiftZ * mainGeom->getPageSize();
			x1 -= shiftX;  x2 -= shiftX;
			z1 -= shiftZ;  z2 -= shiftZ;
		}
	}
	else
	{
		// make sure that values are inbounds
		if(x2 >= geomGridX)
			x2 = geomGridX - 1;
		if(z2 >= geomGridZ)
			z2 = geomGridZ - 1;

		if (x1 < 0)
			x1 = 0;
		if (z1 < 0)
			z1 = 0;
	}
	//Now, in that general area, find what pages are within the cacheDist radius
	//Pages within the cacheDist radius will be added to the pending block list
	//to be loaded later, and pages within farDist will be loaded immediately.
	for (int x = x1; x <= x2; ++x){
		for (int z = z1; z <= z2; ++z){
			GeometryPage *blk = _getGridPage(x, z);
			
			Real dx = camPos.x - blk->_centerPoint.x;
			Real dz = camPos.z - blk->_centerPoint.z;
			Real distSq = dx * dx + dz * dz;
			
			//If the page is in the cache radius...
			if (distSq <= cacheDistSq){
				//If the block hasn't been loaded yet, it should be
				if (blk->_loaded == false){
					//Test if the block's distance is between nearDist and farDist
					if (distSq >= nearDistSq && distSq < farTransDistSq){
						//If so, load the geometry immediately
						_loadPage(blk);
						loadedList.push_back(blk);
						blk->_iter = (--loadedList.end());

						//And remove it from the pending list if necessary
						if (blk->_pending){
							pendingList.remove(blk);
							blk->_pending = false;
						}
					} else {
						//Otherwise, add it to the pending geometry list (if not already)
						//Pages in then pending list will be loaded later (see below)
						if (!blk->_pending){
							pendingList.push_back(blk);
							blk->_pending = true;
						}
					}
				} else {
					//Set the inactive time to 0 (since the page is active). This
					//must be done in order to keep it from expiring (and deleted).
					//This way, blocks not in the cache radius won't have their
					//inactivity clock reset, and they will expire in a few seconds.
					blk->_inactiveTime = 0;
				}
			}
		}
	}
	
	
	//Calculate cache speeds based on camera speed. This is important to keep the cache
	//process running smooth, because if the cache can't keep up with the camera, the
	//immediately visible pages will be forced to load instantly, which can cause
	//noticeable and sudden stuttering. The cache system results in smoother performance
	//because it smooths the loading tasks out across multiple frames. For example,
	//instead of loading 10+ blocks every 2 seconds, the cache would load 1 block every
	//200 milliseconds.
	Real speed = Math::Sqrt(camSpeed.x * camSpeed.x + camSpeed.z * camSpeed.z);
	
	unsigned long cacheInterval;
	if (speed == 0)
		cacheInterval = maxCacheInterval;
	else {
		cacheInterval = (mainGeom->getPageSize() * 0.8f) / (speed * pendingList.size());
		if (cacheInterval > maxCacheInterval)
			cacheInterval = maxCacheInterval;
	}


	TPGeometryPages::iterator i1, i2;

	//Now load a single geometry page periodically, based on the cacheInterval
	cacheTimer += deltaTime;
	if (cacheTimer >= cacheInterval && enableCache){
		//Find a block to be loaded from the pending list
		i1 = pendingList.begin();
		i2 = pendingList.end();
		while (i1 != i2)
		{
			GeometryPage *blk = *i1;
			
			//Remove it from the pending list
			i1 = pendingList.erase(i1);
			blk->_pending = false;
			
			//If it's within the geometry cache radius, load it and break out of the loop
			Real dx = camPos.x - blk->_centerPoint.x;
			Real dz = camPos.z - blk->_centerPoint.z;
			Real distSq = dx * dx + dz * dz;
			if (distSq <= cacheDistSq){
				_loadPage(blk);
				loadedList.push_back(blk);
				blk->_iter = (--loadedList.end());

				enableCache = false;
				break;
			}

			//Otherwise this will keep looping until an unloaded page is found
		}
			
		//Reset the cache timer
		cacheTimer = 0;
	}


	//-- Update existing geometry and impostors --

	//Loop through each loaded geometry block
	i1 = loadedList.begin();
	i2 = loadedList.end();

	//Real fadeBeginDistSq = farDistSq - Math::Sqr(mainGeom->getPageSize() * 1.4142f);
	Real halfPageSize = mainGeom->getPageSize() * 0.5f;
	while (i1 != i2)
	{
		GeometryPage *blk = *i1;
		
		//If the geometry has expired...
		if (blk->_inactiveTime >= inactivePageLife){
			if (!blk->_keepLoaded) {
				//Unload it
				_unloadPage(blk);
				i1 = loadedList.erase(i1);
			} else {
				//This page needs to be kept loaded indefinitely, so don't unload it
				blk->_inactiveTime = 0;
				++i1;
			}
		} else {
			//Update it's visibility/fade status based on it's distance from the camera
			bool visible = false;
			Real dx = camPos.x - blk->_centerPoint.x;
			Real dz = camPos.z - blk->_centerPoint.z;
			Real distSq = dx * dx + dz * dz;

			Real overlap = 0, tmp;

			tmp = blk->_trueBounds.getMaximum().x - halfPageSize;
			if (tmp > overlap) overlap = tmp;
			tmp = blk->_trueBounds.getMaximum().z - halfPageSize;
			if (tmp > overlap) overlap = tmp;
			tmp = blk->_trueBounds.getMinimum().x + halfPageSize;
			if (tmp > overlap) overlap = tmp;
			tmp = blk->_trueBounds.getMinimum().z + halfPageSize;
			if (tmp > overlap) overlap = tmp;

			Real pageLengthSq = Math::Sqr((mainGeom->getPageSize() + overlap) * 1.41421356f);
		
			if (distSq + pageLengthSq >= nearDistSq && distSq - pageLengthSq < farTransDistSq){
				//Fade the page when transitioning
				bool enable = false;
				Real fadeNear = 0;
				Real fadeFar = 0;
				
				if (fadeEnabled && distSq + pageLengthSq >= farDistSq){
					//Fade in
					visible = true;
					enable = true;
					fadeNear = farDist;
					fadeFar = farTransDist;
				}
				else if (prevManager && prevManager->fadeEnabled && (distSq - pageLengthSq < prevManager->farTransDistSq)){
					//Fade out
					visible = true;
					enable = true;
					fadeNear = prevManager->farDist + (prevManager->farTransDist - prevManager->farDist) * 0.5f;	//This causes geometry to fade out faster than it fades in, avoiding a state where a transition appears semitransparent
					fadeFar = prevManager->farDist;
				}
				
				//Apply fade settings
				if (enable != blk->_fadeEnable){
					blk->setFade(enable, fadeNear, fadeFar);
					blk->_fadeEnable = enable;
				}
			}
			//Non-fade visibility
			if (distSq >= nearDistSq && distSq < farDistSq)
				visible = true;
			//Hide all?
			if (!mainGeom->getVisible())
				visible = false;

			//Update visibility
			if (visible){
				//Show the page if it isn't visible
				if (blk->_visible != true){
					blk->setVisible(true);
					blk->_visible = true;
				}
			} else {
				//Hide the page if it's not already
				if (blk->_visible != false){
					blk->setVisible(false);
					blk->_visible = false;
				}
			}

			//And update it
			blk->update();
				
			++i1;
		}
		
		//Increment the inactivity timer for the geometry
		blk->_inactiveTime += deltaTime;		
	}
}

//Clears all GeometryPage's
void GeometryPageManager::reloadGeometry()
{
	TPGeometryPages::iterator it;
	for (it = loadedList.begin(); it != loadedList.end(); ++it)
	{
		GeometryPage *page = *it;
		_unloadPage(page);
	}
	loadedList.clear();
}

//Clears a single page (which contains the given point)
void GeometryPageManager::reloadGeometryPage(const Vector3 &point)
{
	//Determine which grid block contains the given points
	const int x = Math::Floor(geomGridX * (point.x - gridBounds.left) / gridBounds.width());
	const int z = Math::Floor(geomGridZ * (point.z - gridBounds.top) / gridBounds.height());
	
	//Unload the grid block if it's in the grid area, and is loaded
	if (x >= 0 && z >= 0 && x < geomGridX && z < geomGridZ){
		GeometryPage *page = _getGridPage(x, z);
		if (page->_loaded){
			_unloadPage(page);
			loadedList.erase(page->_iter);
		}
	}
}

//Clears pages within the radius
void GeometryPageManager::reloadGeometryPages(const Vector3 &center, Real radius)
{
	//First calculate a square boundary to eliminate the search space
	TBounds area(center.x - radius, center.z - radius, center.x + radius, center.z + radius);

	//Determine which grid block contains the top-left corner
	int x1 = Math::Floor(geomGridX * (area.left - gridBounds.left) / gridBounds.width());
	int z1 = Math::Floor(geomGridZ * (area.top - gridBounds.top) / gridBounds.height());
	if (x1 < 0) x1 = 0; else if (x1 > geomGridX-1) x1 = geomGridX-1;
	if (z1 < 0) z1 = 0; else if (z1 > geomGridZ-1) z1 = geomGridZ-1;
	//...and the bottom right
	int x2 = Math::Floor(geomGridX * (area.right - gridBounds.left) / gridBounds.width());
	int z2 = Math::Floor(geomGridZ * (area.bottom - gridBounds.top) / gridBounds.height());
	if (x2 < 0) x2 = 0; else if (x2 > geomGridX-1) x2 = geomGridX-1;
	if (z2 < 0) z2 = 0; else if (z2 > geomGridZ-1) z2 = geomGridZ-1;

	//Scan all the grid blocks in the region
	Real radiusSq = radius * radius;
	for (int x = x1; x <= x2; ++x) {
		for (int z = z1; z <= z2; ++z) {
			GeometryPage *page = _getGridPage(x, z);
			if (page->_loaded){
				Vector3 pos = page->getCenterPoint();
				Real distX = (pos.x - center.x), distZ = (pos.z - center.z);
				Real distSq = distX * distX + distZ * distZ;
				
				if (distSq <= radius) {
					_unloadPage(page);
					loadedList.erase(page->_iter);
				}
			}
		}
	}
}

//Clears pages within the bounds
void GeometryPageManager::reloadGeometryPages(const TBounds & area)
{
	//Determine which grid block contains the top-left corner
	int x1 = Math::Floor(geomGridX * (area.left - gridBounds.left) / gridBounds.width());
	int z1 = Math::Floor(geomGridZ * (area.top - gridBounds.top) / gridBounds.height());
	if (x1 < 0) x1 = 0; else if (x1 > geomGridX-1) x1 = geomGridX-1;
	if (z1 < 0) z1 = 0; else if (z1 > geomGridZ-1) z1 = geomGridZ-1;
	//...and the bottom right
	int x2 = Math::Floor(geomGridX * (area.right - gridBounds.left) / gridBounds.width());
	int z2 = Math::Floor(geomGridZ * (area.bottom - gridBounds.top) / gridBounds.height());
	if (x2 < 0) x2 = 0; else if (x2 > geomGridX-1) x2 = geomGridX-1;
	if (z2 < 0) z2 = 0; else if (z2 > geomGridZ-1) z2 = geomGridZ-1;

	//Unload the grid blocks
	for (int x = x1; x <= x2; ++x) {
		for (int z = z1; z <= z2; ++z) {
			GeometryPage *page = _getGridPage(x, z);
			if (page->_loaded){
				_unloadPage(page);
				loadedList.erase(page->_iter);
			}
		}
	}
}

//Loads pages within viewing range of the bounds and keeps them loaded
void GeometryPageManager::preloadGeometry(const TBounds & area)
{
	TBounds loadarea;
	loadarea.left = area.left - farDist;
	loadarea.right = area.right + farDist;
	loadarea.top = area.top - farDist;
	loadarea.bottom = area.bottom + farDist;

	//Determine which grid block contains the top-left corner
	int x1 = Math::Floor(geomGridX * (loadarea.left - gridBounds.left) / gridBounds.width());
	int z1 = Math::Floor(geomGridZ * (loadarea.top - gridBounds.top) / gridBounds.height());
	if (x1 < 0) x1 = 0; else if (x1 > geomGridX-1) x1 = geomGridX-1;
	if (z1 < 0) z1 = 0; else if (z1 > geomGridZ-1) z1 = geomGridZ-1;
	//...and the bottom right
	int x2 = Math::Floor(geomGridX * (loadarea.right - gridBounds.left) / gridBounds.width());
	int z2 = Math::Floor(geomGridZ * (loadarea.bottom - gridBounds.top) / gridBounds.height());
	if (x2 < 0) x2 = 0; else if (x2 > geomGridX-1) x2 = geomGridX-1;
	if (z2 < 0) z2 = 0; else if (z2 > geomGridZ-1) z2 = geomGridZ-1;

	//Preload the grid blocks
	for (int x = x1; x <= x2; ++x) {
		for (int z = z1; z <= z2; ++z) {
			GeometryPage *page = _getGridPage(x, z);

			//If the page isn't loaded
			if (!page->_loaded){
				//Load the geometry immediately
				_loadPage(page);
				loadedList.push_back(page);
				page->_iter = (--loadedList.end());

				//And remove it from the pending list if necessary
				if (page->_pending){
					pendingList.remove(page);
					page->_pending = false;
				}
			}

			//Flag the page so it won't expire and be deleted in a few seconds if
			//it's currently not in the viewing range.
			page->_keepLoaded = true;
		}
	}
}

void GeometryPageManager::resetPreloadedGeometry()
{
	//Set all grid blocks' _keepLoaded flag to false
	for (int x = 0; x < geomGridX; ++x) {
		for (int z = 0; z < geomGridZ; ++z) {
			GeometryPage *page = _getGridPage(x, z);
			page->_keepLoaded = true;
		}
	}
}


//Loads the given page of geometry immediately
//Note: _loadPage() does add the page to loadedList, so that will have to be done manually
void GeometryPageManager::_loadPage(GeometryPage *page)
{
	//Calculate page info
	PageInfo info;
	Real halfPageSize = mainGeom->getPageSize() * 0.5f;

	info.bounds.left = page->_centerPoint.x - halfPageSize;
	info.bounds.right = page->_centerPoint.x + halfPageSize;
	info.bounds.top = page->_centerPoint.z - halfPageSize;
	info.bounds.bottom = page->_centerPoint.z + halfPageSize;
	info.centerPoint = page->_centerPoint;
	info.xIndex = page->_xIndex;
	info.zIndex = page->_zIndex;
	info.userData = page->_userData;

	//Check if page needs unloading (if a delayed unload has been issued)
	if (page->_needsUnload){
		page->removeEntities();
		mainGeom->getPageLoader()->unloadPage(info);
		page->_userData = 0;
		page->_needsUnload = false;

		page->clearBoundingBox();
	}

	//Load the page
	page->setRegion(info.bounds.left, info.bounds.top, info.bounds.right, info.bounds.bottom);

	mainGeom->getPageLoader()->geomPage = page;
	mainGeom->getPageLoader()->loadPage(info);

	page->_userData = info.userData;

	page->build();
	page->setVisible(page->_visible);

	page->_inactiveTime = 0;
	page->_loaded = true;
	page->_fadeEnable = false;
}

//Unloads the given page of geometry immediately
//Note: _unloadPage() does not remove the page from loadedList, so that will have to be done manually
void GeometryPageManager::_unloadPage(GeometryPage *page)
{
	//Calculate boundaries to unload
	PageInfo info;
	Real halfPageSize = mainGeom->getPageSize() * 0.5f;

	info.bounds.left = page->_centerPoint.x - halfPageSize;
	info.bounds.right = page->_centerPoint.x + halfPageSize;
	info.bounds.top = page->_centerPoint.z - halfPageSize;
	info.bounds.bottom = page->_centerPoint.z + halfPageSize;
	info.centerPoint = page->_centerPoint;
	info.xIndex = page->_xIndex;
	info.zIndex = page->_zIndex;
	info.userData = page->_userData;

	//Unload the page
	page->removeEntities();
	mainGeom->getPageLoader()->unloadPage(info);
	page->_userData = 0;
	page->_needsUnload = false;

	page->clearBoundingBox();

	page->_inactiveTime = 0;
	page->_loaded = false;
	page->_fadeEnable = false;
}

//"Virtually" unloads the given page of geometry. In reality it is unloaded during the next load.
//Note: _unloadPageDelayed() does not remove the page from loadedList, so that will have to be done manually
void GeometryPageManager::_unloadPageDelayed(GeometryPage *page)
{
	page->_needsUnload = true;
	page->_loaded = false;
}


//Scrolls pages in the grid by the given amount
void GeometryPageManager::_scrollGridPages(int shiftX, int shiftZ)
{
	//Check if the camera moved completely out of the grid
	if (shiftX > geomGridX || shiftX < -geomGridX || shiftZ > geomGridZ || shiftZ < -geomGridZ){
		//If so, just reload all the tiles (reloading is accomplished by unloading - loading is automatic)
		for (int x = 0; x < geomGridX; ++x){
			for (int z = 0; z < geomGridZ; ++z){
				GeometryPage *page = _getGridPage(x, z);
				if (page->_loaded){
					page->_keepLoaded = false;
					_unloadPage(page);
					loadedList.erase(page->_iter);
				}
				page->_centerPoint.x += shiftX * mainGeom->getPageSize();
				page->_centerPoint.z += shiftZ * mainGeom->getPageSize();
				page->_xIndex += shiftX;
				page->_zIndex += shiftZ;
			}
		}
	} else { //If not, scroll the grid by the X and Y values
		//Scroll horizontally (X)
		if (shiftX > 0){
			for (int z = 0; z < geomGridZ; ++z){
				//Temporarily store off-shifted pages first
				for (int x = 0; x < shiftX; ++x){
					GeometryPage *page = _getGridPage(x, z);
					if (page->_loaded){
						page->_keepLoaded = false;
						_unloadPageDelayed(page);
						loadedList.erase(page->_iter);
					}
					scrollBuffer[x] = page;
				}
				//Shift left
				int shiftedMidpoint = geomGridX - shiftX;
				for (int x = 0; x < shiftedMidpoint; ++x)
					_setGridPage(x, z, _getGridPage(x + shiftX, z));
				//Rotate temporary pages around to other side of grid
				for (int x = 0; x < shiftX; ++x){
					scrollBuffer[x]->_centerPoint.x += geomGridX * mainGeom->getPageSize();
					scrollBuffer[x]->_xIndex += geomGridX;
					_setGridPage(x + shiftedMidpoint, z, scrollBuffer[x]);
				}
			}
		}
		else if (shiftX < 0) {
			for (int z = 0; z < geomGridZ; ++z){
				//Temporarily store off-shifted pages first
				int initialMidpoint = geomGridX + shiftX;
				for (int x = initialMidpoint; x < geomGridX; ++x){
					GeometryPage *page = _getGridPage(x, z);
					if (page->_loaded){
						page->_keepLoaded = false;
						_unloadPageDelayed(page);
						loadedList.erase(page->_iter);
					}
					scrollBuffer[x - initialMidpoint] = page;
				}
				//Shift right
				for (int x = geomGridX-1; x >= -shiftX; x--)
					_setGridPage(x, z, _getGridPage(x + shiftX, z));
				//Rotate temporary pages around to other side of grid
				for (int x = 0; x < -shiftX; ++x){
					scrollBuffer[x]->_centerPoint.x -= geomGridX * mainGeom->getPageSize();
					scrollBuffer[x]->_xIndex -= geomGridX;
					_setGridPage(x, z, scrollBuffer[x]);
				}
			}
		}
		//Scroll vertically (Z)
		if (shiftZ > 0){
			for (int x = 0; x < geomGridX; ++x){
				//Temporarily store off-shifted pages first
				for (int z = 0; z < shiftZ; ++z){
					GeometryPage *page = _getGridPage(x, z);
					if (page->_loaded){
						page->_keepLoaded = false;
						_unloadPageDelayed(page);
						loadedList.erase(page->_iter);
					}
					scrollBuffer[z] = page;
				}
				//Shift left
				int shiftedMidpoint = geomGridZ - shiftZ;
				for (int z = 0; z < shiftedMidpoint; ++z)
					_setGridPage(x, z, _getGridPage(x, z + shiftZ));
				//Rotate temporary pages around to other side of grid
				for (int z = 0; z < shiftZ; ++z){
					scrollBuffer[z]->_centerPoint.z += geomGridZ * mainGeom->getPageSize();
					scrollBuffer[z]->_zIndex += geomGridZ;
					_setGridPage(x, z + shiftedMidpoint, scrollBuffer[z]);
				}
			}
		}
		else if (shiftZ < 0) {
			for (int x = 0; x < geomGridX; ++x){
				//Temporarily store off-shifted pages first
				int initialMidpoint = geomGridZ + shiftZ;
				for (int z = initialMidpoint; z < geomGridZ; ++z){
					GeometryPage *page = _getGridPage(x, z);
					if (page->_loaded){
						page->_keepLoaded = false;
						_unloadPageDelayed(page);
						loadedList.erase(page->_iter);
					}
					scrollBuffer[z - initialMidpoint] = page;
				}
				//Shift right
				for (int z = geomGridZ-1; z >= -shiftZ; z--)
					_setGridPage(x, z, _getGridPage(x, z + shiftZ));
				//Rotate temporary pages around to other side of grid
				for (int z = 0; z < -shiftZ; ++z){
					scrollBuffer[z]->_centerPoint.z -= geomGridZ * mainGeom->getPageSize();
					scrollBuffer[z]->_zIndex -= geomGridZ;
					_setGridPage(x, z, scrollBuffer[z]);
				}
			}
		}
	}
}


GeometryPage::GeometryPage()
{
	_visible = _fadeEnable = _pending = _pending = _loaded = _needsUnload = false;
	_trueBoundsUndefined = true;
	_inactiveTime = 0; _xIndex = _zIndex = 0; 
	_centerPoint = Ogre::Vector3::ZERO; 
	_userData = NULL;
	mHasQueryFlag = false;
	mQueryFlag = 0;
}

void GeometryPage::addEntityToBoundingBox(Ogre::Entity *ent, const Ogre::Vector3 &position, const Ogre::Quaternion &rotation, const Ogre::Vector3 &scale)
{
	//Update bounding box
	Ogre::Matrix4 mat(rotation);
	mat.setScale(scale);
	Ogre::AxisAlignedBox entBounds = ent->getBoundingBox();
	entBounds.transform(mat);

	Ogre::Vector3 relPosition = position - _centerPoint;
	if (_trueBoundsUndefined){
		_trueBounds.setMinimum(entBounds.getMinimum() + relPosition);
		_trueBounds.setMaximum(entBounds.getMaximum() + relPosition);
		_trueBoundsUndefined = false;
	} else {
		Ogre::Vector3 min = _trueBounds.getMinimum();
		Ogre::Vector3 max = _trueBounds.getMaximum();
		min.makeFloor(entBounds.getMinimum() + relPosition);
		max.makeCeil(entBounds.getMaximum() + relPosition);
		_trueBounds.setMinimum(min);
		_trueBounds.setMaximum(max);
	}
}

const AxisAlignedBox &GeometryPage::getBoundingBox()
{
	return _trueBounds;
}

void GeometryPage::clearBoundingBox()
{
	_trueBounds = AxisAlignedBox(0, 0, 0, 0, 0, 0);
	_trueBoundsUndefined = true;
}
}
