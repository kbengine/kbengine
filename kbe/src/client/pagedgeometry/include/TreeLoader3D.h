/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

#ifndef __TreeLoader3D_H__
#define __TreeLoader3D_H__

#include "PagedGeometry.h"
#include "PropertyMaps.h"

#include <OgrePrerequisites.h>

namespace Forests {

class TreeIterator3D;
class TreeIterator2D;

/** \brief A PageLoader-derived object you can use with PagedGeometry to easily place trees on your terrain. 

\note TreeLoader3D is derived from PageLoader - this implementation provides you with an easy way
to add trees to your scene. Remember that if the included PageLoader's aren't enough, you can easily
create your own to load geometry any way you want. You could even modify existing an PageLoader
to suit your needs.

Using a TreeLoader is simple - simply create an instance, attach it to your PagedGeometry object
with PagedGeometry::setPageLoader(), and add your trees.

To add trees, just call TreeLoader3D::addTree(), supplying the appropriate parameters. You may notice that
TreeLoader3D restricts trees to uniform scale and yaw rotation. This is done to conserve memory; TreeLoader3D
packs trees into memory as effeciently as possible, taking only 10 bytes per tree. This means 1 million trees
takes 9.53 MB of RAM (additionally, adding 1 million trees takes less than a second in a release build).

\note Using TreeLoader2D uses 40% less memory than TreeLoader3D.
*/
class TreeLoader3D: public PageLoader
{
public:
	/** \brief Creates a new TreeLoader3D object.
	\param geom The PagedGeometry object that this TreeLoader3D will be assigned to.
	\param bounds The rectangular boundary in which all trees will be placed. */
	TreeLoader3D(PagedGeometry *geom, const TBounds &bounds);
	~TreeLoader3D();

	/** \brief Adds an entity to the scene with the specified location, rotation, and scale.
	\param entity The entity to be added to the scene.
	\param position The desired position of the tree
	\param yaw The desired rotation around the vertical axis in degrees
	\param scale The desired scale of the entity

	While TreeLoader3D allows you to provide full 3-dimensional x/y/z coordinates,
	you are restricted to only yaw rotation, and only uniform scale. 
	
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
		deleteTrees(const Ogre::Vector3 &position, Ogre::Real radius, Ogre::Entity *type = NULL);

#ifdef PAGEDGEOMETRY_USER_DATA
	/** \brief Find trees within a certain radius of the given coordinates.
		\param position The coordinate of the tree(s) to look for
		\param radius The radius from the given coordinate where trees will be deleted
		\param type The type of tree to find (optional)

		\note If the "type" parameter is set to an entity, only trees created with that entity
		will be found. */
	std::vector<void*> findTrees(const Ogre::Vector3 &position, float radius, Ogre::Entity *type = NULL);
#endif


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

	/** \brief Gets an iterator which can be used to access all added trees.

	The returned TreeIterator3D can be used to iterate through every tree that was added
	to this TreeLoader3D fairly efficiently.

	\see The TreeIterator class documentation for more info.
	\warning Be sure to test TreeIterator3D::hasMoreElements() before calling other members of the
	TreeIterator3D class. */
	TreeIterator3D getTrees();

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

	You can use this function to access the internal color map object used by the TreeLoader3D.
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
	void setMaximumScale(Ogre::Real maxScale)
	{
		maximumScale = maxScale;
	}

	/** \brief Gets the maximum tree scale value
	\returns The maximum tree scale value

	This function will return the maximum tree scale value as set by setMaximumScale(). By
	default this value will be 2.0. */
	float getMaximumScale()
	{
		return maximumScale;
	}

	/** \brief Sets the minimum tree scale value

	When calling addTree() to add trees, the scale values you are allowed to use are restricted
	in the range of 0.0 - 2.0 by default. With this function, you can adjust the minimum scale you are
	allowed to use for trees. The closer this minimum value is to the maximum tree scale, the more
	precision there will be when storing trees with addTree().

	\warning Be sure to call this before adding any trees - otherwise adjusting this value will cause
	the size of the currently added trees to change. */
	void setMinimumScale(Ogre::Real minScale)
	{
		minimumScale = minScale;
	}

	/** \brief Gets the minimum tree scale value
	\returns The minimum tree scale value

	This function will return the minimum tree scale value as set by setMinimumScale(). By
	default this value will be 0. */
	float getMinimumScale()
	{
		return minimumScale;
	}

	/** \brief Gets the tree boundary area
	\returns A TBounds boundary value

	This function returns the boundaries in which all trees are added. This value is set from the constructor. */
	inline const TBounds &getBounds()
	{
		return actualBounds;
	}

	void loadPage(PageInfo &page);

private:
	friend class TreeIterator3D;

	struct TreeDef
	{
		float yPos;
		Ogre::uint16 xPos, zPos;
		Ogre::uint8 scale, rotation;
#ifdef PAGEDGEOMETRY_USER_DATA
		void *userData;
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
	friend class TreeIterator2D;
	friend class TreeIterator3D;
	Ogre::Vector3 position;
	Ogre::Degree yaw;
	Ogre::Real scale;
	Ogre::Entity *entity;
#ifdef PAGEDGEOMETRY_USER_DATA
	void* userData;
#endif
};

#endif


class TreeIterator3D
{
public:
	TreeIterator3D(TreeLoader3D *trees);

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

	TreeLoader3D *trees;
	TreeLoader3D::PageGridListIterator currentGrid;
	int currentX, currentZ;
	std::vector<TreeLoader3D::TreeDef> *currentTreeList;
	std::vector<TreeLoader3D::TreeDef>::iterator currentTree;

	TreeRef currentTreeDat, prevTreeDat;
	bool hasMore;
};

}

#endif