/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
	1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

#include "PagedGeometryConfig.h"

//PagedGeometry.h
//Main header file for the PagedGeometry engine.
//-------------------------------------------------------------------------------------

//-------- The following is the main API documentation page (parsed by doxygen) --------
/**
\page Main PagedGeometry API Documentation

\section A Introduction
Although the PagedGeometry engine is fairly simple and easy to use, there are some
advanced features that may be difficult to learn on you own. This API reference is here
for your convenience, to aid you in learning how to get the most out of the PagedGeometry
engine.

Every feature of the engine is covered here in detail, so you won't be left in the dark
about any aspect of PagedGeometry's use (however, some of the internal workings of the
engine are not documented in here - you'll have to refer to the source code comments
for that).


\section B What is PagedGeometry?
The PagedGeometry engine is an add-on to the <a href="http://www.ogre3d.org">OGRE
Graphics Engine</a>, which provides highly optimized methods for rendering massive amounts
of small meshes covering a possibly infinite area. This is especially well suited for dense
forests and outdoor scenes, with millions of trees, bushes, grass, rocks, etc., etc.

Paged geometry gives you many advantages over plain entities, the main one being speed:
With proper usage of detail levels, outdoor scenes managed by PagedGeometry can
be >100x faster than plain entities. Another advantage is that the geometry is paged; in
other words, only entities which are immediately needed (to be displayed) are loaded.
This allows you to expand the boundaries of your virtual world almost infinitely
(only limited by floating point precision), providing the player with a more realistically
scaled game area.


\section C Getting Started

The first thing you should do is follow the instructions in "Getting Started.txt" to
get PagedGeometry compiled and the examples running. Note: Keep in mind that the art
in the examples is not the best, and is there simply to demonstrate the performance of
the engine.

When you're ready to start learning how to use PagedGeometry, the best place to start is
with Tutorial 1 (in the docs folder). The tutorials will teach you how to use many
important PagedGeometry features, step by step. This API reference isn't recommended
for learning, but is a valuable resource when you need specific in-depth information
about a certain function or class.


\section E Credits

<ul>
<li><b>John Judnich</b> - <i>Programming / design / documentation</i></li>
<li><b><a href="http://sjcomp.com">Alexander Shyrokov</a></b> (aka. sj) - <i>Testing / co-design</i></li>
<li><b><a href="http://www.pop-3d.com">Tuan Kuranes</a></b> - <i>Imposter image render technique</i></li>
<li><b></b> (Falagard) - <i>Camera-facing billboard vertex shader</i></li>
<li><b><a href="http://www.wendigostudios.com/">Wendigo Studios</a></b> - <i>Tree animation code & various patches/improvements</i></li>
<li><b><a href="http://www.thomasfischer.biz/">Thomas Fischer</a></b> - <i>Maintainer from Jun/2009</i></li>
</ul>


\section D License
<b>Copyright (c) 2007 John Judnich</b>

<i>
This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

<b>1.</b> The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

<b>2.</b> Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

<b>3.</b> This notice may not be removed or altered from any source distribution.
</i>


*/

//--------------------------------------------------------------------------------------

// this small snipped disables some warnings under MSVC that can be ignored normally
#ifdef _MSC_VER
// disable MSVC warning "... possible loss of data"
# pragma warning(disable: 4244)
#endif //_MSC_VER


#ifndef __PagedGeometry_H__
#define __PagedGeometry_H__

#include <limits> // numeric_limits<>
#include <memory>

#include <OgreRoot.h>
#include <OgrePrerequisites.h>
#include <OgreRenderSystem.h>
#include <OgreEntity.h>
#include <OgreCommon.h>
#include <OgreCamera.h>
#include <OgreVector3.h>
#include <OgreTimer.h>
#include <OgreMesh.h>

namespace Forests {

class GeometryPageManager;
class PageLoader;

/// Define TBounds as a TRect using Real numeric units.
typedef Ogre::TRect<Ogre::Real> TBounds;

//Enable PagedGeometry::setCoordinateSystem()
//#define PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM

//Enable per-entity user-defined data and callbacks
//#define PAGEDGEOMETRY_USER_DATA

//-------------------------------------------------------------------------------------
/**
\brief A class providing highly optimized methods for rendering massive amounts of geometry.

The PagedGeometry class provides highly optimized methods for rendering massive amounts
of small meshes, covering a large area. This is especially well suited for dense
forests, with thousands of trees, bushes, grass, rocks, etc., etc.

The paged geometry works by loading only the geometry that is visible (or will soon
be visible), to save memory. The PagedGeometry engine can display entities using many
different methods (static geometry, impostors, etc.) What method is used depends on the
entities' distance from the camera, and how you configured these methods (see
PagedGeometry::addDetailLevel() for more info about this).

The loading of pages is done through a PageLoader, which you define. This way, you
can program the PagedGeometry to load pages any way you want, whether it's from a
file on your hard drive, from a procedural generation algorithm, or anything else.
See the documentation for the PageLoader class for more information about this.

\note Always remember to call each PagedGeometry object's update() function every
frame; otherwise the geometry you're trying to display will not appear correctly.
See PagedGeometry::update() for more information.
*/
class PagedGeometry
{
public:
	/**
	\brief Initializes a PagedGeometry object.
	\param cam A camera which the PagedGeometry object will use for LOD calculations.
	\param pageSize The page size (pages are square)

	pageSize sets the size of a single "page" of geometry. If your pages are too big,
	you may experience "hiccuping" during the game as these regions are loaded. However,
	regions that are too small may result in lower frame rates (depending on what detail
	levels you are using).

	Also, using larger pages uses slightly less memory, although you should generally
	give performance precedence over memory usage.

	\note You do not need to specify the page size or camera in the constructor if you
	don't want to. PagedGeometry::setCamera() and PagedGeometry::setPageSize() allows
	you to configure these values later.

	\see setCamera(), setPageSize(), setBounds(), setInfinite(), setPageLoader()
	*/
	PagedGeometry(Ogre::Camera *cam = NULL, Ogre::Real pageSize = 100, Ogre::RenderQueueGroupID queue = Ogre::RENDER_QUEUE_6);

	~PagedGeometry();

	/**
	\brief Sets the camera to use when calculating levels of detail.
	\param cam The camera to assign to this PagedGeometry object.

	\note The specified camera must belong to the same scene manager as
	previous ones; once a camera is set, the scene manager that camera
	belongs to will be used permanently for this PagedGeometry instance.

	\warning Changing cameras will often result in low cache efficiency in
	infinite mode. If you are constantly switching between multiple cameras
	that are relatively far apart, consider using bounded mode.
	*/
	void setCamera(Ogre::Camera *cam);

	/**
	\brief Sets the output directory for the imposter pages
	*/
	void setTempDir(Ogre::String dir);
	Ogre::String getTempdir() { return this->tempdir; };

	/**
	\brief Gets the camera which is used to calculate levels of detail.
	\returns The camera assigned to this PagedGeometry object.

	\warning Be careful when storing a local copy of this - the camera returned
	may change at any time, so it's usually best to call getCamera() repeatedly,
	instead of storing a local copy. This is an inline function, so don't worry
	too much about performance.
	*/
	inline Ogre::Camera *getCamera() const
	{
		return sceneCam;
	}

	/**
	\brief Gets the scene manager which is being used to display the geometry
	\returns A SceneManager

	This function simply returns the SceneManager that this PagedGeometry object
	is using. If no camera has been set yet, this will return NULL, since PagedGeometry
	has no way of knowing which SceneManager to use. However, once a camera is set,
	the SceneManager this function returns will always remain the same - even if the
	camera is later set to NULL.
	*/
	inline Ogre::SceneManager *getSceneManager() const
	{
		return sceneMgr;
	}

	/**
	\brief Gets the scene node to which all PagedGeometry geometry is attached
	\returns A SceneNode

	\note Feel free to ignore this function - you can fully make use of PagedGeometry's features
	without it.

	This function returns the SceneNode which PagedGeometry uses to render all it's
	geometry. Everything that PagedGeometry renders to the screen can be found under
	this scene node.

	You don't need to use this function at all to fully make use of PagedGeometry's
	features - it's primary use is for PagedGeometry's internal subsystems to be able
	to create geometry using the proper scene node.

	\warning If no camera has been set yet, this will return NULL, since PagedGeometry
	can't create the SceneNode until it know which SceneManager to use (which is determined
	from the assigned camera). However, once a camera is set, the SceneNode this function
	returns will always remain the same - even if the camera is later set to NULL.
	*/
	inline Ogre::SceneNode *getSceneNode() const
	{
		return rootNode;
	}

	#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
	/**
	\brief Sets the coordinate system to be used by PagedGeometry
	\param up A vector pointing to whatever direction you consider to be "up"
	\param right A vector pointing to whatever direction you consider to be "right"

	By default, PagedGeometry uses the standard coordinate system where X is right, Y is up,
	and Z is back. If you use an alternate coordinate system, for example where Z is up, you'll
	have to use this function to configure PagedGeometry to use that coordinate system; otherwise,
	LOD calculations, impostors, etc. will be all messed up.

	To do so, simply supply which directions you consider "right" and "up". For example, if your
	coordinate system uses X as right, Y as forward, and Z as up, you would set the "right" parameter
	to Vector3::UNIT_X and the "up" parameter to Vector3::UNIT_Z. The forward direction
	(Vector3::UNIT_Y in this case) doesn't need to be supplied since it will be automatically calculated
	from the right and up vectors.

	\warning Be sure to configure PagedGeometry with your coordinate system before using any PageLoader's,
	since they may depend on the current coordinate system to function properly.

	\note By default this function is disabled and won't appear in the PagedGeometry library. To
	enable it, reenable the line near the top of PagedGeometry.h where PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
	is defined by un-commenting it (then recompile).
	*/
	void setCoordinateSystem(Ogre::Vector3 up, Ogre::Vector3 right = Ogre::Vector3::UNIT_X);
	#endif

	/**
	\brief Switches to bounded mode and uses the given boundaries

	By default, PagedGeometry does not place boundaries on the geometry that can
	be added to it though the PageLoader. However, there are cases where specifying
	a strict boundary can improve performance.

	When bounded mode is used, PagedGeometry allocates certain lightweight data
	structures to fill the boundaries. This may result in a slight frame rate boost
	compared to an infinite world, although it will take a little more memory
	(especially with large areas).

	Since bounded mode requires more memory for larger boundaries, it is best suited
	for small to medium sized worlds, while infinite mode is best for huge or even
	near-infinite worlds.

	\note Bounds must be square.

	\see setInfinite()
	*/
	void setBounds(const TBounds bounds);

	/**
	\brief Switches to infinite mode

	By default, PagedGeometry will just allocate enough memory to display and cache
	what is on the screen. This behavior is called "infinite mode", and can be activated
	by calling this function if not already activated (it will be by default).

	Most game worlds have boundaries of some sort, but infinite mode allows you to
	expand the size of your game world almost to infinity. Since only what's on the
	screen is actually loaded, it makes little difference if your world is 100 square
	miles, or 1,000,000 square miles.

	The only disadvantage to using infinite mode is that cache efficiency will be
	slightly reduced in some cases. For example, bounded mode will achieve better
	performance if you are often switching between multiple cameras, since the cache
	is more globally based (unlike the locally based cache of infinite mode).

	\see setBounds()
	*/
	void setInfinite();

	/**
	\brief Gets the current geometry boundary.
	\returns The geometry boundary which was set in the constructor.

	\see The PagedGeometry constructor for information about the geometry boundary.

	This returns a TBounds value, which contains information about the boundaries
	of the geometry. Since TBounds is simply a typedef for TRect<Real>, accessing
	the boundary information is easy, for example:

	\code
	(...) = PagedGeometry::getBounds().top;
	(...) = PagedGeometry::getBounds().bottom;
	(etc.)
	\endcode

	Ogre's documentation should contain more information about TRect members.
	*/
	inline const TBounds &getBounds() const
	{
		return m_bounds;
	}

	/**
	\brief Convert an Ogre::AxisAlignedBox to a TBounds coplanar to the plane defined by the UP axis.
	*/
	TBounds convertAABToTBounds( const Ogre::AxisAlignedBox & aab ) const;

	/**
	\brief Sets the page size

	This sets the size of a single "page" of geometry. If your pages are too big,
	you may experience "hiccuping" during the game as these regions are loaded. However,
	regions that are too small may result in lower frame rates (depending on what detail
	levels you are using).

	Also, using larger pages uses slightly less memory, although you should generally
	give performance precedence over memory usage.
	*/
	void setPageSize(Ogre::Real size);

	/**
	\brief Gets the current page size.
	\returns The current page size.

	\see setPageSize() for more information about page size.
	*/
	inline Ogre::Real getPageSize()
	{
		return pageSize;
	}


	/**
	\brief Adds a detail level to the PagedGeometry object.
	\param PageType The page type class you want to use for this detail level.
	\param maxRange The maximum distance this detail level will be used at.
	\param transitionLength The desired length of fade transitions - optional
	\param data An extra parameter to pass to the PageType constructor - optional

	\note PageType is not really a function parameter, but a template parameter.
	See the code below for an example on how this "parameter" is used.

	On it's own, a plain PagedGeometry object can't display anything. It needs you
	to add detail levels to it with this function. This way, you can easily customize
	the behavior of the PagedGeometry.

	To use this function, simply use the form:

	\code
	pagedGeometry::addDetailLevel<PageType>(farRange);
	\endcode

	In the above example, "farRange" specifies the maximum view distance this detail
	level will be used at. However, the most important part is "<PageType>". Here you
	substitute "PageType" with the name of a installed page type you want to use for
	this detail level. A page type is simply a method of displaying or otherwise
	representing entities.

	The PagedGeometry engine currently comes pre-installed with a few page types:
	BatchPage and ImpostorPage. Refer to their class documentation for detailed
	information about them (although you never actually access these classes yourself).

	You can use these page types in any configuration you want. For example:

	\code
	pagedTrees->addDetailLevel<BatchPage>(100); //Use batched geometry from 0-100
	pagedTrees->addDetailLevel<ImpostorPage>(500); //Use impostors from 100-500
	\endcode

	That example would set up the PagedGeometry object called pagedTrees to use
	batched geometry (StaticGeometry) from 0 to 100 units from the camera, and impostors
	from 100 to 500 units from the camera.

	If the included page types aren't adequate, you can fairly easily add your own
	by simply extending the virtual GeometryPage class properly.

	By default, no fade transitions are used. This means that there will be a noticeable
	"popping" effect when your tree changes from an impostor to a batch, for example.
	Enabling fade transitions will smooth out the change by slowly "morphing" between
	the two detail levels. To enable fade transitions, simply add a second parameter to
	you addDetailLevel() call:

	\code
	//Use batched geometry from 0-100, and transition to the next LOD for 50 units
	pagedTrees->addDetailLevel<BatchPage>(100, 50);
	//Use impostors from 100-500
	pagedTrees->addDetailLevel<ImpostorPage>(500);
	\endcode

	The second parameter seen above will enable transitions between BatchPage and the next
	LOD (which is ImpostorPage, in this case). The number you supply will specify how long
	the transition is. Longer transitions result in smoother "morphs", although shorter
	transitions will give slightly better frame rates.

	The transition parameter can also be applied to the last detail level to cause it to fade out:

	\code
	//Use batched geometry from 0-100, and transition to the next LOD for 50 units
	pagedTrees->addDetailLevel<BatchPage>(100, 50);
	//Use impostors from 100-400, and fade out for 100 units beyond that
	pagedTrees->addDetailLevel<ImpostorPage>(400, 100);
	\endcode

	In the example above, batching is used up to 100 units, where the batched geometry
	starts transitioning for 50 units into the next level of detail (impostors). The
	impostors continue up to 400 units, where they begin to fade out for 100 more units.

	\warning Depending on your page size and transition length, enabling fade transitions
	will often reduce PagedGeometry's rendering performance anywhere from 10 - 30%.

	Transitions can be disabled by omitting the transitionLength parameter, or setting
	it to 0.

	\note Make sure you make any calls to setPageSize() and setBounds() / setInfinite()
	before adding detail levels with this function. After a detail level is added you
	cannot call these functions without first removing them with removeDetailLevels().

	The "data" parameter is entirely optional, so there should never be any requirement that it
	be used. Just what type of data this parameter accepts, and what it does, depends entirely
	on the specific GeometryPage implementation you're using. See the appropriate page type
	documentation for info on how this parameter can be used (if at all).

	\see The GeometryPage class documention for more information on adding custom
	page types.
	*/
	template <class PageType> inline GeometryPageManager* addDetailLevel(Ogre::Real maxRange, Ogre::Real transitionLength = 0, const Ogre::Any &data = Ogre::Any(), Ogre::uint32 queryFlag = 0);

	/**
	\brief Removes all detail levels from the PagedGeometry object.

	This removes all detail levels (added with addDetailLevel) from the PagedGeometry
	object.	This also removes all geometry associated with PagedGeometry from the scene.
	Remember that you will need to re-add all the detail levels again with
	addDetailLevel() before any of the geometry will be displayed.
	*/
	void removeDetailLevels();

	/**
	\brief Returns a reference to a list of all added detail levels.

	This returns a std::list of all detail levels (GeometryPageManager's). These objects
	can be used to set/get advanced properties, such as view ranges and cache speeds.

	Normally you won't ever have to access this data, but it's there in case you need it.
	*/
	inline const std::list<GeometryPageManager*> &getDetailLevels() { return managerList; }


	/**
	\brief Assigns a PageLoader object for the PagedGeometry.
	\param loader A PageLoader object.

	When the page manager decides it should cache a certain region of geometry, it calls
	on your PageLoader to do the job. This way you can load entities from RAM, a hard-drive,
	the internet, or even procedurally. Simply create a PageLoader class and use this function
	to link it to a PagedGeometry object.

	\warning Since you must instantiate your PageLoader yourself, you must also be sure to
	deallocate it properly (as with any other class instance). PagedGeometry will not do this
	for you.

	\see PageLoader documentation for more information on setting up a page loader.
	*/
	void setPageLoader(PageLoader *loader);

	/**
	\brief Gets the PageLoader currently being used to load geometry.
	\returns A PageLoader object which is currently being used to load geometry.

	This can be useful if you want to retrieve the current page loader to delete it,
	or any other task that needs to be done to the currently used page loader.
	*/
	inline PageLoader *getPageLoader()
	{
		return pageLoader;
	}


	/**
	\brief Updates this PagedGeometry object

	This function must be called each frame in order for the PagedGeometry object
	to calculate LODs and perform paging. If this function is not called every frame,
	none of the geometry managed by this PagedGeometry instance will appear (or if it
	does, it will appear incorrectly)
	*/
	void update();


	/**
	\brief Reloads all visible geometry.

	If your PageLoader changes it's output during runtime, you normally won't see
	the changes immediately (and in many cases, you will never see the changes).
	This function provides a way to reload the geometry to force the changes to take
	effect immediately.

	This function will cause ALL visible geometry to be reloaded during the next
	update. This can take up to several	seconds, depending on the complexity of
	the current scene, so use this function only when absolutely necessary.
	*/
	void reloadGeometry();

	/**
	\brief Reloads geometry at the given location.
	\param point The point in 3D space where geometry needs to be reloaded.

	If your PageLoader changes it's output during runtime, you normally won't see
	the changes immediately (and in many cases, you will never see the changes).
	This function provides a way to reload the geometry to force the changes to take
	effect immediately.

	This function will cause a certain page of visible geometry to be reloaded
	during the next update. Unlike reloadGeometry(), this function allows pinpoint
	reloading to take place, resulting in better performance if a small portion
	of the geometry changes.

	Since this doesn't actually reload anything immediately, you can call this
	function as many times as you need without worrying too much about performance.
	For example, if you update 150 trees in your game, simply supply this function
	with the locations of each tree. When the scene is about to be rendered, the
	appropriate geometry pages will automatically be reloaded.
	*/
	void reloadGeometryPage(const Ogre::Vector3 &point);

	/**
	\brief Reloads geometry in the given radius area.
	\param center The center of the area to be reloaded
	\param radius The radius from the center where geometry needs to be reloaded

	\note This is identical to reloadGeometryPage() except it allows you to reload
	an entire area rather than a single point.

	If your PageLoader changes it's output during runtime, you normally won't see
	the changes immediately (and in many cases, you will never see the changes).
	This function provides a way to reload the geometry to force the changes to take
	effect immediately.

	This function will cause a certain area of visible geometry to be reloaded
	during the next update. Unlike reloadGeometry(), this function allows selective
	reloading to take place, resulting in better performance if a small portion
	of the geometry changes.

	Since this doesn't actually reload anything immediately, you can call this
	function as many times as you need without worrying too much about performance.
	For example, if you update 150 trees in your game, simply supply this function
	with the locations of each tree. When the scene is about to be rendered, the
	appropriate geometry pages will automatically be reloaded.
	*/
	void reloadGeometryPages(const Ogre::Vector3 &center, Ogre::Real radius);

	/**
	\brief Reloads geometry in the given rect area.
	\param area A rectangular area that needs reloading

	\note This is identical to reloadGeometryPage() except it allows you to reload
	an entire area rather than a single point.

	If your PageLoader changes it's output during runtime, you normally won't see
	the changes immediately (and in many cases, you will never see the changes).
	This function provides a way to reload the geometry to force the changes to take
	effect immediately.

	This function will cause a certain area of visible geometry to be reloaded
	during the next update. Unlike reloadGeometry(), this function allows selective
	reloading to take place, resulting in better performance if a small portion
	of the geometry changes.

	Since this doesn't actually reload anything immediately, you can call this
	function as many times as you need without worrying too much about performance.
	For example, if you update 150 trees in your game, simply supply this function
	with the locations of each tree. When the scene is about to be rendered, the
	appropriate geometry pages will automatically be reloaded.
	*/
	void reloadGeometryPages(const TBounds & area);

	/**
	\brief Preloads a region of geometry (loads once and never loads again)
	\param area A rectangular area of the world that needs to be preloaded

	You can use this function to preload entire areas of geometry. Doing this
	will basically turn off dynamic paging for the given region, since all the
	pages effecting it will stay loaded forever (until you delete the PagedGeometry
	object, or if using infinite mode, until you move away from the region).

	\note The rectangular bounds value you supply does not indicate a
	rectangular area to preload, but instead a rectangular area in which you need
	the camera to be able to freely move around without having to dynamically load
	any pages. In other words, this function will preload all geometry within
	viewing range of the given bounds area.
	*/
	void preloadGeometry(const TBounds & area);

	/**
	\brief Releases geometry preloaded with preloadGeometry() to be unloaded if necessary

	When you call preloadGeometry() to preload region(s) of geometry, it makes those
	regions un-unloadable; in other words, they will never be unloaded automatically
	by PagedGeometry (except for some cases in infinite mode). This way you can load
	a region of your world once and never have to load it again (optimally).

	This function allows you to undo all this by allowing all the geometry to be
	unloaded once again when necessary. It won't unload anything, but it will
	allow geometry to unload that previously was not allowed.
	*/
	void resetPreloadedGeometry();

	/**
	\brief Hides or unhides all geometry managed by this PagedGeometry instance
	\params visible Whether or not you want this PagedGeometry to be visible

	By default everything is visible. This can be used to hide an entire PagedGeometry
	"group" of geometry if desired.
	*/
	void setVisible(bool visible) { geometryAllowedVisible = visible; }

	/**
	\brief Returns whether or not geometry managed by this PagedGeometry instance is visible

	By default, everything will be visible of course. This function simply returns the
	visible/invisible state as set by the setVisible() command.
	*/
	bool getVisible() { return geometryAllowedVisible; }

	/**
	\brief disables the use of shaders
	*/
	void setShadersEnabled(bool value) { shadersEnabled=value; }
	bool getShadersEnabled() { return shadersEnabled; }

	/*
	\brief Immediately loads visible geometry.
	\param maxTime The maximum amount of time (in milliseconds) which cacheGeometry()
	is allowed to use before returning (roughly).
	\returns Whether or not everything was cached.

	PagedGeometry automatically loads and caches geometry near the camera in real-time.
	This function allows you to easily pre-load this geometry outside of your render loop.

	For example, in your loading code, you might call PagedGeometry::cacheGeometry() to
	load all your trees/etc. managed by PagedGeometry instantly, rather than later on.

	If it takes several seconds to cache geometry, you may want to update a progress bar
	more often. The maxTime parameter allows you to split up this task into smaller
	segments for this purpose. Simply call cacheGeometry(maxTime) repeatedly until
	everything is cached (cacheGeometry() will return true when finished ).
	*/
	//todo
	//bool cacheGeometry(unsigned long maxTime = 0);


	/** INTERNAL FUNCTION - DO NOT USE */
	Ogre::Vector3 _convertToLocal(const Ogre::Vector3 &globalVec) const;

	/**
	\brief Sets or creates a custom parameter for an entity managed by PagedGeometry
	This can be used to set custom parameters / data for entities which can be accessed
	from other PagedGeometry subsystems or your own code. Primarily, this is intended
	for use with GeometryPage implementations or PageLoader implementations.

	PagedGeometry includes a GeometryPage implementation, "WindBatchPage", which applies
	a wind animation shader to your trees, which you can control using these custom
	parameters: windFactorX and windFactorY.

	If you're using 3rd party PagedGeometry "plugins" like GeometryPage implementations,
	etc., there may be more custom parameters available to you. Check with the appropriate
	module documentation for info on supported custom parameters and their usage.

	\param entity Name of the entity
	\param paramName Name of the parameter for this entity
	\param paramValue Value to assign to the parameter
	*/
	void setCustomParam( std::string entity, std::string paramName, float paramValue);

	/**
	\brief Sets or creates a custom parameter for an entity managed by PagedGeometry
	This can be used to set custom parameters / data for entities which can be accessed
	from other PagedGeometry subsystems or your own code. Primarily, this is intended
	for use with GeometryPage implementations or PageLoader implementations.

	PagedGeometry includes a GeometryPage implementation, "WindBatchPage", which applies
	a wind animation shader to your trees, which you can control using these custom
	parameters: windFactorX and windFactorY.

	If you're using 3rd party PagedGeometry "plugins" like GeometryPage implementations,
	etc., there may be more custom parameters available to you. Check with the appropriate
	module documentation for info on supported custom parameters and their usage.

	\param entity Name of the entity
	\param paramName Name of the parameter for this entity
	\param paramValue Value to assign to the parameter
	*/
	void setCustomParam( std::string paramName, float paramValue);

	/**
	\brief Returns the value of the custom parameter
	\param entity Name of the entity
	\param paramName Name of the parameter for this entity
	\param defaultParamValue Value to return if no entry is found
	\returns float value if entry is found or the defaultParamValue if not
	*/
	float getCustomParam( std::string entity, std::string paramName, float defaultParamValue) const;

	/**
	\brief Returns the value of the custom parameter
	\param entity Name of the entity
	\param paramName Name of the parameter for this entity
	\param defaultParamValue Value to return if no entry is found
	\returns float value if entry is found or the defaultParamValue if not
	*/
	float getCustomParam( std::string paramName, float defaultParamValue) const;


	/**
	\brief Returns the rendering queue that paged geometry was constructed with
	\returns Ogre::RenderQueue number of the rendering queue
	*/
	Ogre::RenderQueueGroupID getRenderQueue() const;

protected:
	//Internal function - do not use
	void _addDetailLevel(GeometryPageManager *mgr, Ogre::Real maxRange, Ogre::Real transitionLength);

	Ogre::SceneManager *sceneMgr;
	Ogre::SceneNode *rootNode;				//PagedGeometry's own "root" node
	bool shadersEnabled;

	bool geometryAllowedVisible;	//If set to false, all geometry managed by this PagedGeometry is hidden

	#ifdef PAGEDGEOMETRY_ALTERNATE_COORDSYSTEM
	Ogre::Quaternion coordinateSystemQuat;	//The orientation of rootNode
	#endif

	//Camera data
	Ogre::Camera *sceneCam;
	Ogre::Vector3 oldCamPos;

	Ogre::Camera *lastSceneCam;
	Ogre::Vector3 lastOldCamPos;

	//This list keeps track of all the GeometryPageManager's added with addPageManager()
	std::list<GeometryPageManager*> managerList;

	//The assigned PageLoader used to load entities
	PageLoader *pageLoader;

	//The bounds and page size
	TBounds m_bounds;
	//The page size
	Ogre::Real pageSize;
	//The used rendering queue
	Ogre::RenderQueueGroupID mRenderQueue;

	//Time-related data
	Ogre::Timer timer;
	unsigned long lastTime;
	Ogre::String tempdir;

private:
	std::map<std::string, float> customParam;
};



//-------------------------------------------------------------------------------------
/**
\brief This base-class can be extended to provide different ways of representing entities.

The PagedGeometry engine comes pre-installed with a few GeometryPage sub-classes
(BatchPage, and ImpostorPage). These "page types" can all be supplied to
a PagedGeometry object through addDetailLevel().
\see PagedGeometry::addDetailLevel() for more information about setting up detail levels.

If you need more than the pre-installed page types, you can easily create your own!
Simply make a new class inheriting GeometryPage. Then implement the necessary member
functions, and it should work immediately. No additional setup is required.

There are several virtual member functions you will need to implement in your class:
\code
virtual void init(SceneManager *mgr, Camera *cam) = 0;
virtual void setRegion(Real left, Real top, Real right, Real bottom) = 0;
virtual void addEntity(Entity *ent, const Vector3 &position, const Quaternion &rotation, const Vector3 &scale, const Ogre::ColourValue &color, void* userData = NULL) = 0;
virtual void build() {}
virtual void removeEntities() = 0;
virtual void setVisible(bool visible) = 0;
virtual void setFade(bool enabled, Real visibleDist, Real invisibleDist) = 0;
virtual void update() {}
\endcode

\note For detailed information on implementing each of these functions, please refer to
their documentation.

Here is how the page manager uses these functions:

\b 1. When a PagedGeometry first creates a GeometryPage, it immediately calls
GeometryPage::init(). This function is called just like a constructor, and you should
use it the same way.

\b 2. GeometryPage::setRegion() is called to provide you with the area where upcoming
entities will be added. You can use this information any way you like, or you can omit
this step completely by omitting the setRegion() function from your class definition.

\b 3. To load a page, the addEntity() function is used to insert all the entities into the scene.
Then, build() is called. Entities don't actually have to be displayed until after build()
is called.

\b 4. setVisible() and setFade() will be called occasionally to update the visibility/fade status
of the page.

\b 5. When the page has become obsolete, the contents of it is deleted with removeEntities().
This should return the page to the state it was before addEntity() and build().

\b 6. Steps 2-5 are repeated as pages are loaded/unloaded

Implementing your own geometry page is really very simple. As long as the functions
do their jobs right, everything will work fine. If you learn best be example, you may
want to take a look at how the included page types are implemented also.

\see The BatchPage or GrassPage code for examples of how page types are implemented (the
ImpostorPage can also be used as an example, although it is a somewhat complex technique,
and is not recommended for learning how GeometryPage's work).
*/
class GeometryPage
{
	friend class GeometryPageManager;

public:
	/**
	\brief Prepare a geometry page for use.
	\param geom The PagedGeometry object that's creating this GeometryPage.
	\param data A single parameter of custom data (optional).

	This is called immediately after creating a new GeometryPage. It is never called more
	than once for a single instance of your geometry page.

	The "data" parameter is set for all pages when PagedGeometry::addDetailLevel() is
	called. This parameter is optional and can be used for whatever you like if you
	need a constructor parameter of some kind. Be sure to document what kind of variable
	the user needs to supply and what it's purpose is in your GeometryPage implementation.

	\note If you need to get the current camera, scene manager, etc., use the geom
	parameter. The PagedGeometry class contains inline methods you can use to access
	the camera and scene manager.

	\warning Do NOT store a local copy of geom->getCamera()! The camera returned by this
	function may change at any time!
	*/
	virtual void init(PagedGeometry *geom, const Ogre::Any &data) = 0;

	void setQueryFlag(Ogre::uint32 flag)
	{
		mHasQueryFlag = true;
		mQueryFlag = flag;
	};

	bool hasQueryFlag()
	{
		return mHasQueryFlag;
	};

	Ogre::uint32 getQueryFlag()
	{
		return mQueryFlag;
	};

	/**
	\brief Prepare a geometry page for entities
	\param left The minimum x-coordinate any entities will have.
	\param top The minimum z-coordinate any entities will have.
	\param right The maximum x-coordinate any entities will have.
	\param bottom The maximum z-coordinate any entities will have.

	This basically provides you with a region where upcoming entities will be located,
	since many geometry rendering methods require this data. It's up to you how this
	data is used, if at all.

	setRegion() is never called when the page contains entities; only once just before
	a load process (when entities are added with addEntity).

	\note Implementing this funtion in your GeometryPage is completely optional, since
	most of the time you don't need region information.
	*/
	virtual void setRegion(Ogre::Real left, Ogre::Real top, Ogre::Real right, Ogre::Real bottom) {};

	/**
	\brief Add an entity to the page, at the specified position, rotation, and scale.
	\param ent The entity that is being added. Keep in mind that the same entity may
	be added multiple times.
	\param position The position where the entity must be placed. Under normal
	circumstances, this will never be outside of the bounds supplied to init().
	The only exception is when a PageLoader tries to add an entity outside of the
	bounds it was given.
	\param rotation The rotation which should be applied to the entity.
	\param scale The scale which should be applied to the entity.
	\param color The desired color to apply to the whole entity

	\note The entity does not have to actually appear in the scene until build()
	is called.
	*/
	virtual void addEntity(Ogre::Entity *ent, const Ogre::Vector3 &position, const Ogre::Quaternion &rotation, const Ogre::Vector3 &scale, const Ogre::ColourValue &color) = 0;

	/**
	\brief Perform any final steps to make added entities appear in the scene.

	build() is automatically called right after all the entities have been added with
	addEntity(). Use this if there are any final steps that need to be performed after
	addEntity() has been called in order to display the entities.

	\note This function is not pure virtual, so you don't have to override it if you
	don't need to.
	*/
	virtual void build() {}

	/**
	\brief Remove all geometry/entities from the page completely.

	Make sure this completely reverses the effects of both build() and addEntity(). This
	is necessary, because after this is called, the entities will most likely be added
	again with addEntity() and build().

	Do not leave any remains of the entities in memory after this function is called.
	One of the advantages of using paged geometry is that you can have near-infinite
	game worlds, which would normally exceed a computer's RAM capacity. This advantage
	would completely disappear if you did not clean up properly when the page manager
	calls this function.
	*/
	virtual void removeEntities() = 0;

	/**
	\brief Sets fade behavior for this page.
	\param enabled Whether or not to enable fading
	\param visibleDist The distance where geometry will be fully opaque (alpha 1)
	\param invisibleDist The distance where geometry will be invisible (alpha 0)

	This is called whenever a page needs fading enabled/disabled. The distance ranges
	given specify how the final alpha values should be calculated - geometry at
	visibleDist should have alpha values of 1, while geometry at invisibleDist should
	have alpha values of 0. Important: Distances must be calculated in the xz plane
	only - the y coordinate should be disregarded when calculating distance.

	setFade() won't be called unless the user's computer supports vertex shaders.

	\note invisibleDist may be greater than or less than visibleDist, depending on
	whether the geometry is fading out or in to the distance.
	*/
	virtual void setFade(bool enabled, Ogre::Real visibleDist = 0, Ogre::Real invisibleDist = 0) = 0;

	/**
	\brief Toggle the entire page's visibility.
	\param visible Whether or not this page should be visible.
	*/
	virtual void setVisible(bool visible) = 0;

	/**
	\brief Do whatever needs to be done to keep the page geometry up-to-date.

	update() is called each frame for each GeometryPage instance. This function should
	perform any operations that are needed to keep the geometry page up-to-date.

	\note Overriding this function is optional, however, since not all implementations
	of geometry may need to be updated.
	*/
	virtual void update() {}

	/**
	\brief Gets the center point of the page.
	\returns The center points of the page.
	\note This is a non-virtual utility function common to all GeometryPage classes, don't
	try to override it.
	*/
	inline Ogre::Vector3 &getCenterPoint() { return _centerPoint; }

	/**
	\brief Return the current visibility status of the page.
	\returns The current visibility status of the page.
	\note This is a non-virtual utility function common to all GeometryPage classes, don't
	try to override it.
	*/
	inline bool isVisible() { return (_visible && _loaded); }

	/**
	\brief Advanced: Return the bounding box computed with addEntityToBoundingBox()

	Advanced: Override this function only if your page implementation already computes a
	bounding box (local to the page center) for added entities. This way you can prevent
	the bounding box from being computed twice.

	When performing fade transitions, the page manager needs to know the actual boundaries
	of an entire page of entities in order to avoid entities "popping" into view without a
	smooth transition due to loose grid boundaries. Anyway, as long as this function returns
	the combined bounding box of all entities added to this page properly, fade transitions
	should work fairly smoothly.

	Important: If you implement this function, you must also override addEntityToBoundingBox()
	and clearBoundingBox() (although you don't need to implement them as long as getBoundingBox()
	functions as expected). Otherwise the default implementations of these function will be
	used and therefore result in the bounding box being calculated twice.
	*/
	virtual const Ogre::AxisAlignedBox &getBoundingBox();

	/**
	\brief Advanced: Expand the current bounding box to include the given entity

	Advanced: Override this function only if your page implementation already computes a
	bounding box (local to the page center) for added entities. This way you can prevent
	the bounding box from being computed twice.

	\see getBoundingBox() for important details.
	*/
	virtual void addEntityToBoundingBox(Ogre::Entity *ent, const Ogre::Vector3 &position, const Ogre::Quaternion &rotation, const Ogre::Vector3 &scale);

	/**
	\brief Advanced: Reset the bounding box used by addEntityToBoundingBox()

	Advanced: Override this function only if your page implementation already computes a
	bounding box (local to the page center) for added entities. This way you can prevent
	the bounding box from being computed twice.

	\see getBoundingBox() for important details.
	*/
	virtual void clearBoundingBox();

	/**
	\brief Destructor
	This is defined here so the destructors of derived classes are called properly. Whether
	or not you actually implement a destructor is up to you.
	*/
	virtual ~GeometryPage() {}

	/**
	\brief Constructor
	Initialise everything to zero, false or NULL except for _trueBoundsUndefined that is set to true.
	*/
	GeometryPage();

private:
	//These values and functions are used by the GeometryPageManager internally.
	Ogre::Vector3 _centerPoint;		//The central point of this page (used to visibility calculation)
	int _xIndex, _zIndex;			//The absolute grid position of this page
	unsigned long _inactiveTime;	//How long this page has been inactive (used to calculate expired pages)
	bool _visible;		//Flag indicating if page is visible
	bool _fadeEnable;	//Flag indicating if page fading is enabled

	bool _pending;		//Flag indicating if page needs loading
	bool _loaded;		//Flag indicating if page is loaded
	bool _needsUnload;	//Flag indicating if page needs unloading before next load
	bool _keepLoaded;	//Flag indicating if the page should not be unloaded
	std::list<GeometryPage*>::iterator _iter;	//Iterator in loadedList

	Ogre::AxisAlignedBox _trueBounds;	//Actual bounding box of the 3D geometry added to this page
	bool _trueBoundsUndefined;			//Flag indicating if _trueBounds has not been defined yet

	void *_userData;	//Misc. data associated with this page by the PageLoader

	bool mHasQueryFlag;
	Ogre::uint32 mQueryFlag;
};



//-------------------------------------------------------------------------------------
/**
\brief Useful page information supplied to a pageloader.

When your page loader's loadPage() or unloadPage() is called, you are supplied with a
PageInfo variable. This basically tells you what region in space is to be loaded into
the page, in addition to some other useful information about that region.

\see the PageLoader::loadPage() and PageLoader::unloadPage() documentation for more information.
*/
struct PageInfo
{
	/**
	\brief The page boundaries in which all entities should be placed.

	This specifies the rectangular boundary of the page. Every entity
	contained in the page should reside within these boundaries.

	<ul>
	<li>bounds.left is the minimum X coordinate allowed for any entity in the page.</li>
	<li>bounds.right is the maximum X coordinate allowed for any entity in the page.</li>
	<li>bounds.top is the minimum Z coordinate allowed for any entity in the page.</li>
	<li>bounds.bottom is the maximum Z coordinate allowed for any entity in the page.</li>
	</ul>
	*/
	TBounds bounds;

	/**
	\brief The center of the page (simply the middle of the bounds).
	\note Since there is no way of knowing the elevation of your entities,
	centerPoint.y will always be defaulted at 0.
	*/
	Ogre::Vector3 centerPoint;

	/**
	\brief The X index of the page tile.

	If all the geometry pages were arranged in a big 2D grid, this would be the
	X index of this page in that grid.

	This is mathematically equivalent to Math::Floor( bounds.left / bounds.width() ),
	although this should be used instead due to floating point precision
	issues which may occur otherwise.
	*/
	int xIndex;

	/**
	\brief The Z index of the page tile.

	If all the geometry pages were arranged in a big 2D grid, this would be the
	Z index of this page in that grid.

	This is mathematically equivalent to Math::Floor( bounds.top / bounds.height() ),
	although this should be used instead due to floating point precision
	issues which may occur otherwise.
	*/
	int zIndex;

	/**
	\brief Misc. custom data to associate with this page tile.

	This field can be set in PageLoader::loadPage() to point to custom data
	allocated during the loading of a page. You can later retreive this data in
	PageLoader::unloadPage() in order to deallocate the data if desired.

	\warning After a page is unloaded, userData becomes NULL. Don't attempt to
	use userData to reference an object longer than the page's life span;
	anything userData points to should be fully deallocated when
	PageLoader::unloadPage() is called.
	*/
	void *userData;

	std::vector<Ogre::Mesh*> meshList;
};

/**
\brief A class which you extend to provide a callback function for loading entities.

\note PagedGeometry comes pre-installed with several PageLoader classes, so you probably
won't need to create you're own from scratch. See TreeLoader2D, TreeLoader3D, and GrassLoader
for more info.

Unlike most entity managers, PagedGeometry does not allow you to simply add all your
entities to the object, and let the engine run from that point on. Since the
PagedGeometry engine is designed to work with extremely large game areas, preloading
everything would take far too much memory. Instead, it pages the geometry. In other
words, it loads the geometry only as needed.

Whenever the engine needs a specific region of geometry to be loaded, it calls on your
page loader class's loadPage() function. It's completely up to you how this function
loads the entities, just as long as it gets the job done. When loadPage() is called,
you are provided with a PageInfo variable, which specifies the boundary which must
be loaded (in addition to other useful info). Make sure you don't add anything outside
of this boundary, otherwise the results may be unpredictable.

Within loadPage(), you add entities by calling addEntity(). Simply supply the entity
you want to add (you can add the same entity as many times as you want - in fact, you
should do this as much as possible for better performance), and it's position, rotation,
and scale (optional).

Note that your page loader may be asked to load an area which is out of your world's
bounds (if you have any). In this case simply return without adding any entities.

To set up a page loader, just make a new sub-class (call it anything you want) of
PageLoader. Your class must have one function:

\code
void loadPage(const PageInfo &page);
\endcode

Make sure you get the declaration right, otherwise it won't compile. The rest is up to
you. You can define your own member functions if that helps, just as long as your
loadPage() function does it's job right.

Once you've created your PageLoader-derived class, you need to attach it to a
PagedGeometry object. To do this, simply create an instance of your class, and call

\code
pagedGeometry->setPageLoader(yourPageLoader);
\endcode

Remember: If you ever delete a PagedGeometry object, you will have to delete your page
loader yourself (if you want to). The PagedGeometry destructor won't do this for you.
*/
class PageLoader
{
public:
	/**
	\brief This should be overridden to load a specified region of entities.
	\param page A PageInfo variable which includes boundary information and other useful values.

	Override this function to load entities within the specified boundary. The boundary
	information is contained in the "page" parameter, along with other useful information
	as well (see the PageInfo documentation for more info about this).

	Simply use the member function addEntity() to add all the entities you want. If you
	create your own objects inside this function, you are responsible for deleting it
	appropriately in unloadPage() or somewhere else. The PageInfo::userData member is
	useful here since you can point it to your data structures for later reference in
	unloadPage().

	\warning Do not ever add an entity outside of the given region, otherwise this may
	crash the program (depending on how the current page types handle this situation).

	\see PagedGeometry::addDetailLevel() for information about page types.
	*/
	virtual void loadPage(PageInfo &page) = 0;

	/**
	\brief This may be overridden (optional) to unload custom data associated with a page.
	\param page A PageInfo variable which includes boundary information and other useful values.

	During a PageLoader::loadPage() call, you are supposed to add entities by calling
	the addEntity() member function. In case you created anything else (particle systems,
	sound effects, etc.), this function	gives you a chance to delete them along with
	the rest of the entities added with addEntity().

	\note Entities added with addEntity() will automatically be deleted after this
	function returns, so you don't need to worry about them.

	In most cases you won't need to implement this function in your page loader at all,
	since addEntity() is usually all that is used.
	*/
	virtual void unloadPage(PageInfo &page) {}

	/**
	\brief Provides a method for you to perform per-frame tasks for your PageLoader if overridden (optional)

	frameUpdate() is completely optional, and unnecessary in most cases. However, if you ever
	need to update anything in your PageLoader per-frame, this function is here for that purpose.

	\warning This function is actually called every time PagedGeometry::update() is called, so if the
	application doesn't call PagedGeometry::update() as it should, this function will not be called
	either.

	\note frameUpdate() will be called after PagedGeometry::update() is called but before any
	GeometryPage's are actually loaded/unloaded for the frame.
	*/
	virtual void frameUpdate() {}

	/**
	\brief Destructor
	This is defined here so the destructors of derived classes are called properly. Whether
	or not you actually implement a destructor is up to you.
	*/
	virtual ~PageLoader() {}

protected:
	/**
	\brief Call this from loadPage() to add an entity to the page being loaded.
	\param ent The entity to add. You may add the same entity multiple times.
	\param position The position where the entity will be placed.
	\param rotation The rotation to apply to the entity.
	\param scale The scale to apply to the entity.
	\param color The color to apply to the whole entity

	\note This copies the entity into the page, so don't make copies of the entity
	yourself; you may simply add the same entity over and over again. You are also
	free to	destroy any entities you used when you are finished	adding them.

	\warning This does not double-check whether or not your entity is within the proper
	boundaries (for better performance), so be careful not to add entities out of bounds.
	Depending on what current page types are being used, an out-of-bounds entity could
	cause your program to crash.

	\see PagedGeometry::addDetailLevel() for information about page types.
	*/
	void addEntity(Ogre::Entity *ent, const Ogre::Vector3 &position, const Ogre::Quaternion &rotation, const Ogre::Vector3 &scale = Ogre::Vector3::UNIT_SCALE, const Ogre::ColourValue &color = Ogre::ColourValue::White)
	{
		geomPage->addEntity(ent, position, rotation, scale, color);
		geomPage->addEntityToBoundingBox(ent, position, rotation, scale);
	}

private:
	friend class GeometryPageManager;

	//Do NOT modify or use this variable - it is used internally by addEntity()
	GeometryPage *geomPage;
};



//-------------------------------------------------------------------------------------
/**
\brief Manages the rendering of geometry for a detail level type.

\warning This class is used internally by PagedGeometry, and in most cases you should
ignore it completely. However, this does provide some advanced capabilities such as
modifying the near and far view ranges, which may come in handy.

This class manages pages of geometry, cacheing, deleting, etc. them as necessary.
It analyzes the motion of the camera to determine how fast pages need to be cached,
and deletes obsolete pages which have been invisible for a certain amount of time.

When you call PagedGeometry::addDetailLevel(), a GeometryPageManager is created to
manage the new detail level. addDetailLevel() returns a pointer to this page manager,
allowing you access to some useful functions, documented below.

\note Some functions (marked by "DO NOT USE" in the documentation) should never be
called by you. Only the internal processes of PagedGeometry can safely use these
functions, so be careful. Using these functions will cause unpredictable results.
*/
class GeometryPageManager
{
public:
	/** \brief A std::list of pointers to geometry pages */
	typedef std::list<GeometryPage*> TPGeometryPages;

	/** \brief Internal function - DO NOT USE */
	GeometryPageManager(PagedGeometry *mainGeom);

	/** \brief Internal function - DO NOT USE */
	~GeometryPageManager();

	/**
	\brief Sets the near viewing range of this page manager.
	\param nearRange The distance where this page manager starts displaying geometry.

	All geometry displayed by this page manager is confined within a certain radius
	gap from the camera. This function sets the closest distance geometry is allowed
	near the camera.
	*/
	inline void setNearRange(Ogre::Real nearRange)
	{
		nearDist = nearRange;
		nearDistSq = nearDist * nearDist;
	}

	/**
	\brief Sets the far viewing range of this page manager.
	\param farRange The distance where this page manager stops displaying geometry.

	All geometry displayed by this page manager is confined within a certain radius
	gap from the camera. This function sets the farthest distance geometry is allowed
	from the camera.
	*/
	inline void setFarRange(Ogre::Real farRange)
	{
		farDist = farRange;
		farDistSq = farDist * farDist;

		farTransDist = farDist + fadeLength;
		farTransDistSq = farTransDist * farTransDist;
	}

	/**
	\brief Gets the near viewing range of this page manager.
	\returns The near viewing range of this page manager.

	\see setNearRange() for more info about the near viewing range.
	*/
	inline Ogre::Real getNearRange() const
	{
		return nearDist;
	}

	/**
	\brief Gets the far viewing range of this page manager.
	\returns The far viewing range of this page manager.

	\see setFarRange() for more info about the near viewing range.
	*/
	inline Ogre::Real getFarRange() const
	{
		return farDist;
	}

	/**
	\brief Customizes the cache behaviour (advanced).
	\param maxCacheInterval The maximum period of time (milliseconds) before another page is loaded.
	\param inactivePageLife The maximum period of time (milliseconds) a inactive (invisible) page
	is allowed to stay loaded.

	The GeometryPageManager automatically determines how fast pages should be cached to keep
	everything running as smooth as possible, but there are a few options that are adjustable.
	This function allows you to adjust these variables to fine-tune cache performance.

	The maxCacheInterval is basically a minimum rate at which pages are cached. Normally, a stopped
	camera would cause the cache rate prediction algorithm to say 0 pages-per-second must be cached.
	However, this is not optimal, since idle time should be taken advantage of to finish loading.
	By adjusting this value, you can set how much caching you want going on when the camera is
	stopped or moving very slowly.

	The inactivePageLife allows you to set how long inactive pages remain in memory. An inactive page
	is one that is out of the cache range and may not be immediately needed. By allowing these pages
	to remain in memory for a short period of time, the camera can return to it's previous position
	with no need to reload anything.

	\note Even with large inactivePageLife values, pages may be unloaded if the camera moves far enough
	from them, so setting extremely high inactivePageLife values won't result in massive memory usage.
	*/
	inline void setCacheSpeed(unsigned long maxCacheInterval = 200, unsigned long inactivePageLife = 2000)
	{
		GeometryPageManager::maxCacheInterval = maxCacheInterval;
		GeometryPageManager::inactivePageLife = inactivePageLife;
	}

	inline void setTransition(Ogre::Real transitionLength)
	{
		if (transitionLength > 0) {
			//Setup valid transition
			fadeLength = transitionLength;
			fadeLengthSq = fadeLength * fadeLength;
			fadeEnabled = true;
		} else {
			//<= 0 indicates disabled transition
			fadeLength = 0;
			fadeLengthSq = 0;
			fadeEnabled = false;
		}

		farTransDist = farDist + fadeLength;
		farTransDistSq = farTransDist * farTransDist;
	}

	inline Ogre::Real getTransition() const
	{
		return fadeLength;
	}


	/** \brief Internal function - DO NOT USE */
	inline TPGeometryPages getLoadedPages() const { return loadedList; }

	/** \brief Internal function - DO NOT USE */
	template <class PageType> void initPages(const TBounds& bounds, const Ogre::Any &data = Ogre::Any(), Ogre::uint32 queryFlag = 0);

	/** \brief Internal function - DO NOT USE */
	void update(unsigned long deltaTime, Ogre::Vector3 &camPos, Ogre::Vector3 &camSpeed, bool &enableCache, GeometryPageManager *prevManager);

	/** \brief Internal function - DO NOT USE */
	void reloadGeometry();

	/** \brief Internal function - DO NOT USE */
	void reloadGeometryPage(const Ogre::Vector3 &point);

	/** \brief Internal function - DO NOT USE */
	void reloadGeometryPages(const Ogre::Vector3 &center, Ogre::Real radius);

	/** \brief Internal function - DO NOT USE */
	void reloadGeometryPages(const TBounds & area);

	/** \brief Internal function - DO NOT USE */
	void preloadGeometry(const TBounds & area);

	/** \brief Internal function - DO NOT USE */
	void resetPreloadedGeometry();

private:
	PagedGeometry *mainGeom;

	//geomGrid is a 2D array storing all the GeometryPage instances managed by this object.
	GeometryPage **geomGrid;	//A dynamic 2D array of pointers (2D grid of GeometryPage's)
	GeometryPage **scrollBuffer; //A dynamic 1D array of pointers (temporary GeometryPage's used in scrolling geomGrid)
	int geomGridX, geomGridZ;	//The dimensions of the dynamic array
	TBounds		 gridBounds;		//Current grid bounds

	//Fade transitions
	Ogre::Real fadeLength, fadeLengthSq;
	bool fadeEnabled;

	//Inline function used to get geometry page tiles
	inline GeometryPage *_getGridPage(const int x, const int z)
	{
		#ifdef _DEBUG
		if(x >= geomGridX || z >= geomGridZ )
			OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS,
				"Grid dimension is out of bounds",
				"GeometryPageManager::_getGridPage()");
		#endif

		return geomGrid[z * geomGridX + x];
	}
	inline void _setGridPage(const int x, const int z, GeometryPage *page)
	{
		#ifdef _DEBUG
		if(x >= geomGridX || z >= geomGridZ )
			OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS,
				"Grid dimension is out of bounds",
				"GeometryPageManager::_setGridPage()");
		#endif

		geomGrid[z * geomGridX + x] = page;
	}

	//Utility functions for loading/unloading geometry pages (see source for detailed descriptions)
	void _loadPage(GeometryPage *page);
	void _unloadPage(GeometryPage *page);
	void _unloadPageDelayed(GeometryPage *page);

	//Utility function for scrolling pages in the grid by the given amount
	void _scrollGridPages(int shiftX, int shiftZ);


	//Timer counting how long it has been since the last page has been cached
	unsigned long cacheTimer;

	TPGeometryPages pendingList;	//Pages of geometry to be loaded
	TPGeometryPages loadedList;	//Pages of geometry already loaded

	//Cache settings
	unsigned long maxCacheInterval;
	unsigned long inactivePageLife;

	//Near and far visibility ranges for this type of geometry
	Ogre::Real nearDist, nearDistSq;
	Ogre::Real farDist, farDistSq;
	Ogre::Real farTransDist, farTransDistSq;	//farTransDist = farDist + fadeLength
};



//-------------------------------------------------------------------------------------

template <class PageType> inline GeometryPageManager* PagedGeometry::addDetailLevel(Ogre::Real maxRange, Ogre::Real transitionLength, const Ogre::Any &data, Ogre::uint32 queryFlag)
{
	//Create a new page manager
	GeometryPageManager *mgr = new GeometryPageManager(this);

	//If vertex shaders aren't supported, don't use transitions
	Ogre::Root *root = root->getSingletonPtr();	//Work-around for Linux compiler bug
	if (!root->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_VERTEX_PROGRAM))
		transitionLength = 0;

	//Add it to the list (also initializing maximum viewing distance)
	_addDetailLevel(mgr, maxRange, transitionLength);

	//And initialize the paged (dependent on maximum viewing distance)
	mgr->initPages<PageType>(getBounds(), data, queryFlag);

	return mgr;
}

template <class PageType> inline void GeometryPageManager::initPages(const TBounds& bounds, const Ogre::Any &data, Ogre::uint32 queryFlag)
{
	// Calculate grid size, if left is Real minimum, it means that bounds are infinite
	// scrollBuffer is used as a flag. If it is allocated than infinite bounds are used
	// !!! Two cases are required because of the way scrolling is implemented
	// if it is redesigned it would allow to use the same functionality.
	if(bounds.width() < 0.00001)
	{
		// In case of infinite bounds bounding rect needs to be calculated in a different manner, since
		// it represents local bounds, which are shifted along with the player's movements around the world.
		geomGridX = (2 * farTransDist / mainGeom->getPageSize()) + 4;
		gridBounds.top = 0;
		gridBounds.left = 0;
		gridBounds.right = geomGridX * mainGeom->getPageSize();
		gridBounds.bottom = geomGridX * mainGeom->getPageSize();
		// Allocate scroll buffer (used in scrolling the grid)
		scrollBuffer = new GeometryPage *[geomGridX];

		//Note: All this padding and transition preparation is performed because even in infinite
		//mode, a local grid size must be chosen. Unfortunately, this also means that view ranges
		//and transition lengths cannot be exceeded dynamically with set functions.
	}
	else
	{
		//Bounded mode
		gridBounds = bounds;
		// In case the devision does not give the round number use the next largest integer
		geomGridX = std::ceil(gridBounds.width() / mainGeom->getPageSize());
	}
	geomGridZ = geomGridX; //Note: geomGridX == geomGridZ; Need to merge.


	//Allocate grid array
	geomGrid = new GeometryPage *[geomGridX * geomGridZ];

	int xioffset = Ogre::Math::Floor(gridBounds.left / mainGeom->getPageSize());
	int zioffset = Ogre::Math::Floor(gridBounds.top / mainGeom->getPageSize());
	for (int x = 0; x < geomGridX; ++x)
	{
		for (int z = 0; z < geomGridZ; ++z)
		{
			GeometryPage* page = new PageType();
			page->init(mainGeom, data);
			// 0,0 page is located at (gridBounds.left,gridBounds.top) corner of the bounds
			page->_centerPoint.x = ((x + 0.5f) * mainGeom->getPageSize()) + gridBounds.left;
			page->_centerPoint.z = ((z + 0.5f) * mainGeom->getPageSize()) + gridBounds.top;
			page->_centerPoint.y = 0.0f;
			page->_xIndex = x + xioffset;
			page->_zIndex = z + zioffset;
			page->_inactiveTime = 0;
			page->_loaded = false;
			page->_needsUnload = false;
			page->_pending = false;
			page->_keepLoaded = false;
			page->_visible = false;
			page->_userData = 0;
			page->_fadeEnable = false;
			page->setQueryFlag(queryFlag);

			page->clearBoundingBox();

			_setGridPage(x, z, page);
		}
	}
}


}




#endif
