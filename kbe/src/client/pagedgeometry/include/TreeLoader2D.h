/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
    1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

#ifndef __TreeLoader2D_H__
#define __TreeLoader2D_H__

#include "PagedGeometry.h"
#include "PropertyMaps.h"

#include <OgrePrerequisites.h>


namespace Forests {

class TreeIterator3D;
class TreeIterator2D;

/** \brief A PageLoader-derived object you can use with PagedGeometry to easily place trees on your terrain. 

\note TreeLoader2D is derived from PageLoader - this implementation provides you with an easy way
to add trees to your scene. Remember that if the included PageLoader's aren't enough, you can easily
create your own to load geometry any way you want. You could even modify existing an PageLoader
to suit your needs.

Using a TreeLoader is simple - simply create an instance, attach it to your PagedGeometry object
with PagedGeometry::setPageLoader(), and add your trees.

To add trees, just call TreeLoader2D::addTree(), supplying the appropriate parameters. You may notice that
TreeLoader2D restricts trees to uniform scale, yaw rotation, and a position value with no vertical component.
This is done to conserve memory; TreeLoader2D packs trees into memory as effeciently as possible, taking only
6 bytes per tree. This means 1 million trees only takes 5.72 MB of RAM (additionally, adding 1 million trees
takes less than a second in a release build).

\note By default, TreeLoader2D doesn't know what shape your terrain is, so all trees will be placed
at 0 height. To inform TreeLoader2D of the shape of your terrain, you must specify a height function
that returns the height (vertical coordinate) of your terrain at the given coordinates. See the
TreeLoader2D::setHeightFunction() documentation for more information.

\warning If you attempt to use Ogre's scene queries to get the terrain height,
keep in mind that calculating the height of Ogre's built-in terrain this way can
be VERY slow if not done properly, and may cause stuttering due to long paging delays.

If the inability to supply a vertical coordinate to addTree() is too limiting, or you are unable to implement 
a fast enough height function, please refer to TreeLoader3D. TreeLoader3D allows you to provide full 3D x/y/z
coordinates, although 40% more memory is required per tree.
*/
class TreeLoader2D: public PageLoader
{
public:
	/** \brief Creates a new TreeLoader2D object.
	\param geom The PagedGeometry object that this TreeLoader2D will be assigned to.
	\param bounds The rectangular boundary in which all trees will be placed. */
	TreeLoader2D(PagedGeometry *geom, const TBounds &bounds);
	~TreeLoader2D();
	
	/** \brief Adds an entity to the scene with the specified location, rotation, and scale.
	\param entity The entity to be added to the scene.
	\param position The desired position of the tree
	\param yaw The desired rotation around the vertical axis in degrees
	\param scale The desired scale of the entity

	Trees added with TreeLoader2D are restricted to yaw rotation only, and uniform scale
	values. This conserves memory by avoiding storage of useless data (for example, storing
	the height of each tree when this can be easily calculated from the terrain's shape).

	Unlike TreeLoader3D, the vertical position of each tree is NOT stored (therefore
	TreeLoader2D takes less memory than TreeLoader3D), but this means you must provide
	a height function with setHeightFunction() in order for TreeLoader2D to be able to
	calculate the desired height values when the time comes. If you do not specify a
	height function, all trees will appear at 0 height.
	
	\warning By default, scale values may not exceed 2.0. If you need to use higher scale
	values than 2.0, use setMaximumScale() to reconfigure the maximum. */
	void addTree(Ogre::Entity *entity, const Ogre::Vector3 &position, Ogre::Degree yaw = Ogre::Degree(0), Ogre::Real scale = 1.0f, void* userData = NULL);

	/** \brief Deletes trees within a certain radius of the given coordinates.
	\param position The coordinate of the tree(s) to delete
	\param radius The radius from the given coordinate where trees will be deleted
	\param type The type of tree to delete (optional)

	\note If the "type" parameter is set to an entity, only trees created with that entity
	will be deleted. */
	#ifdef PAGEDGEOMETRY_USER_DATA
		std::vector<void*>
	#else
		void
	#endif
	deleteTrees(const Ogre::Vector3 &position, Ogre::Real radius = 0.001f, Ogre::Entity *type = NULL);

	/** \brief Deletes trees within a certain rectangular area.
	\param area The area where trees are to be deleted
	\param type The type of tree to delete (optional)

	\note If the "type" parameter is set to an entity, only trees created with that entity
	will be deleted. */
	#ifdef PAGEDGEOMETRY_USER_DATA
		std::vector<void*>
	#else
		void
	#endif
	deleteTrees(TBounds area, Ogre::Entity *type = NULL);

#ifdef PAGEDGEOMETRY_USER_DATA
	/** \brief Find trees within a certain radius of the given coordinates.
		\param position The coordinate of the tree(s) to look for
		\param radius The radius from the given coordinate where trees will be deleted
		\param type The type of tree to find (optional)

		\note If the "type" parameter is set to an entity, only trees created with that entity
		will be found. */
	std::vector<void*> findTrees(const Ogre::Vector3 &position, float radius, Ogre::Entity *type = NULL);
#endif

	/** \brief Sets the height function used to calculate tree height coordinates
	\param heightFunction A pointer to a height function
	\param userData Optional user data to be supplied to the height function
	
	Unless you want all your trees placed at 0 height, you need to specify a height function
	so TreeLoader2D will be able to calculate the height coordinate. The height function given
	to setHeightFunction() should use the following prototype (although you can name the
	function anything you want):

	\code
	Real getHeightAt(Real x, Real z, void *userData);
	\endcode

	\note If you're not using the default coordinate system (where x = right, z = back), the
	x/z parameters will actually be representing the appropriate equivalents.

	The userData parameter allows you to include any additional data you want when your height
	function is called, and is completely optional (although you can't actually omit it from the
	declaration, you can ignore it). Any userData value you choose to supply to setHeightFunction()
	will be passed on to your height function every time it is called.
	
	After you've defined a height function, using setHeightFunction is easy:
	
	\code
	pageLoader2D->setHeightFunction(&getHeightAt);
	//Or (if you want to pass additional data on to your height function)...
	pageLoader2D->setHeightFunction(&getHeightAt, myUserData);
	\endcode

	In most cases, you may not even need to use the extra "userData" parameter, but it's there in
	the event that your height function needs extra contextual data.
	*/
	void setHeightFunction(Ogre::Real (*heightFunction)(Ogre::Real x, Ogre::Real z, void *userData), void *userData = NULL)
	{
		this->heightFunction = heightFunction;
		heightFunctionUserData = userData;
	}

	/** \brief Gets an iterator which can be used to access all added trees.
	
	The returned TreeIterator can be used to iterate through every tree that was added
	to this TreeLoader fairly efficiently.

	\see The TreeIterator class documentation for more info.
	\warning Be sure to test TreeIterator3D::hasMoreElements() before calling other members of the
	TreeIterator3D class.
	*/
	TreeIterator2D getTrees();

	/** \brief Sets the color map used to color trees
	\param mapFile The color map image
	\param channel The color channel(s) to from the image to use

	A color map is simply a texture that allows you to vary the color and shading of trees
	across your world for a more realistic look. For example, adding a dark spot to the center
	of your color map will make trees near the center of your world look darker.

	The channel parameter allows you to extract the color information from the image's
	red, green, blue, alpha, or color values. For example, you may store the desired shade of your
	trees in the red channel of an image, in which case you would use CHANNEL_RED (when you choose
	a single channel, it is converted to a greyscale color). By default, CHANNEL_COLOR is used,
	which uses the full color information available in the image. */
	void setColorMap(const Ogre::String &mapFile, MapChannel channel = CHANNEL_COLOR);

	/** \brief Sets the color map used to color trees

	Overloaded to accept a Texture object. See the original setColorMap() documentation above
	for more detailed information on color maps.

	\note The texture data you provide is copied into RAM, so you can delete the texture after
	calling this function without risk of crashing. */
	void setColorMap(Ogre::TexturePtr map, MapChannel channel = CHANNEL_COLOR);

	/** \brief Gets a pointer to the color map being used

	You can use this function to access the internal color map object used by the TreeLoader2D.
	Through this object you can directly manipulate the pixels of the color map, among other
	things.

	Note that although you can edit the color map in real-time through this class, the changes
	won't be uploaded to your video card until you call PagedGeometry::reloadGeometry(). If you
	don't, the colors of the trees you see will remain unchanged. */
	ColorMap *getColorMap() { return colorMap; }

	/** \brief Sets the filtering mode used for the color map

	This function can be used to set the filtering mode used for your tree color map.
	By default, no filtering is used (MAPFILTER_NONE). If you enable bilinear filtering
	by using MAPFILTER_BILINEAR, the resulting tree coloration may appear more smooth
	(less pixelated), depending on the resolution of your color map.

	MAPFILTER_BILINEAR is slightly slowe than MAPFILTER_NONE, so don't use it unless you notice
	considerable pixelation. */
	void setColorMapFilter(MapFilter filter)
	{
		colorMapFilter = filter;
		if (colorMap)
			colorMap->setFilter(colorMapFilter);
	}

	/** \brief Sets the maximum tree scale value

	When calling addTree() to add trees, the scale values you are allowed to use are restricted
	to 2.0 maximum by default. With this function, you can adjust the maximum scale you are allowed
	to use for trees. However, keep in mind that the higher the maximum scale is, the less precision
	there will be in storing the tree scales (since scale values are actually packed into a single byte).

	\warning Be sure to call this before adding any trees - otherwise adjusting this value will cause
	the size of the currently added trees to change. */
	void setMaximumScale(Ogre::Real maxScale)	{ maximumScale = maxScale;	}

	/** \brief Gets the maximum tree scale value
	\returns The maximum tree scale value

	This function will return the maximum tree scale value as set by setMaximumScale(). By
	default this value will be 2.0. */
   Ogre::Real getMaximumScale() const  { return maximumScale; }

	/** \brief Sets the minimum tree scale value

	When calling addTree() to add trees, the scale values you are allowed to use are restricted
	in the range of 0.0 - 2.0 by default. With this function, you can adjust the minimum scale you are
	allowed to use for trees. The closer this minimum value is to the maximum tree scale, the more
	precision there will be when storing trees with addTree().

	\warning Be sure to call this before adding any trees - otherwise adjusting this value will cause
	the size of the currently added trees to change. */
	void setMinimumScale(Ogre::Real minScale)	{ minimumScale = minScale;	}

	/** \brief Gets the minimum tree scale value
	\returns The minimum tree scale value

	This function will return the minimum tree scale value as set by setMinimumScale(). By
	default this value will be 0. */
	Ogre::Real getMinimumScale() const { return minimumScale; }

	/** \brief Gets the tree boundary area
	\returns A TBounds boundary value

	This function returns the boundaries in which all trees are added. This value is set from the constructor. */
   const TBounds &getBounds() const { return actualBounds; }

	void loadPage(PageInfo &page);

private:
	friend class TreeIterator2D;

	struct TreeDef
	{
		Ogre::uint16 xPos, zPos;
		Ogre::uint8 scale, rotation;
#ifdef PAGEDGEOMETRY_USER_DATA
		void* userData;
#endif
	};

	//Information about the 2D grid of pages
	int pageGridX, pageGridZ;
	Ogre::Real pageSize;
	TBounds gridBounds, actualBounds;

	Ogre::Real maximumScale, minimumScale;

	//Colormap
	ColorMap *colorMap;
	MapFilter colorMapFilter;

	//Height data
	Ogre::Real (*heightFunction)(Ogre::Real x, Ogre::Real z, void *userData);	//Pointer to height function
	void *heightFunctionUserData;

	//Misc.
	PagedGeometry *geom;

	//A std::map of 2D grid arrays. Each array is the same size (pageGridX x pageGridZ), and
	//contains a std::vector list of trees. Each std::map entry corresponds to a single tree
	//type, and every tree defined in the std::map entry's grid should be of that tree type.
	std::map<Ogre::Entity*, std::vector<TreeDef>*> pageGridList;
	typedef std::map<Ogre::Entity*, std::vector<TreeDef>*>::iterator PageGridListIterator;
	typedef std::pair<Ogre::Entity*, std::vector<TreeDef>*> PageGridListValue;
	
	inline std::vector<TreeDef> &_getGridPage(std::vector<TreeDef> *grid, int x, int z)
	{
		#ifdef _DEBUG
		if(x >= pageGridX || z >= pageGridZ )
			OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS,
			"Grid dimension is out of bounds",
			"TreeLoader2D::_getGridPage()");
		#endif

		return grid[z * pageGridX + x];
	}

	inline void _setGridPage(std::vector<TreeDef> *grid, int x, int z, const std::vector<TreeDef> &page)
	{
		#ifdef _DEBUG
		if(x >= pageGridX || z >= pageGridZ )
			OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS,
			"Grid dimension is out of bounds",
			"TreeLoader2D::_getGridPage()");
		#endif

		grid[z * pageGridX + x] = page;
	}
};



//The TreeRef class is used by both TreeLoader2D and TreeLoader3D.
//This #ifndef makes sure TreeRef isn't declared twice in case both treeloaders are used.
#ifndef TreeRef_Declared
#define TreeRef_Declared

class TreeRef
{
public:
	/** Returns the tree's position */
	inline Ogre::Vector3 &getPosition() { return position; }

	/** Returns the tree's yaw as a degree value */
	inline Ogre::Degree &getYaw() { return yaw; }

	/** Returns the tree's uniform scale value */
	inline Ogre::Real &getScale() { return scale; }
	
	/** Returns the tree's orientation as a Quaternion */
	inline Ogre::Quaternion getOrientation() { return Ogre::Quaternion(yaw, Ogre::Vector3::UNIT_Y); }

	/** Returns the entity used to create the tree */
	inline Ogre::Entity *getEntity() { return entity; }

#ifdef PAGEDGEOMETRY_USER_DATA
	/** Returns the user-defined data associated with this tree */
	inline void* getUserData() { return userData; }
#endif

private:
	friend class TreeIterator3D;
	friend class TreeIterator2D;
	Ogre::Vector3 position;
	Ogre::Degree yaw;
	Ogre::Real scale;
	Ogre::Entity *entity;
#ifdef PAGEDGEOMETRY_USER_DATA
	void* userData;
#endif
};

#endif


class TreeIterator2D
{
public:
	TreeIterator2D(TreeLoader2D *trees);

	/** Returns true if there are more trees available to be read */
	inline bool hasMoreElements() const { return hasMore; }

	/** Returns the next tree, and advances to the next */
	TreeRef getNext();

	/** Returns the next tree, without advancing to the next */
	inline TreeRef &peekNext() { return prevTreeDat; }

	/** Returns a pointer to the next tree, without advancing to the next */
	inline TreeRef *peekNextPtr() { return &prevTreeDat; }
	
	/** Moves the iterator on to the next tree */
	void moveNext();

private:
	void _readTree();

	TreeLoader2D *trees;
	TreeLoader2D::PageGridListIterator currentGrid;
	int currentX, currentZ;
	std::vector<TreeLoader2D::TreeDef> *currentTreeList;
	std::vector<TreeLoader2D::TreeDef>::iterator currentTree;

	TreeRef currentTreeDat, prevTreeDat;
	bool hasMore;
};

}

#endif