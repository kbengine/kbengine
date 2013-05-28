/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

#ifndef __PropertyMaps_H__
#define __PropertyMaps_H__

#include <OgrePrerequisites.h>
#include <OgrePixelFormat.h>
#include <OgreColourValue.h>
#include <OgreRoot.h>
#include <OgreRenderSystem.h>

namespace Forests {

/** \brief Specifies which color channel(s) to extract from an image */
enum MapChannel {
	/// Use only the image's red color channel
	CHANNEL_RED,
	/// Use only the image's green color channel
	CHANNEL_GREEN,
	/// Use only the image's blue color channel
	CHANNEL_BLUE,
	/// Use only the image's alpha channel
	CHANNEL_ALPHA,
	/// Use the image's full RGB color information
	CHANNEL_COLOR
};

/** \brief Specifies the filtering method used to interpret property maps */
enum MapFilter {
	/// Use no filtering - fast, but may appear blocky
	MAPFILTER_NONE,
	/// Use bilinear filtering - slower, but will appear less blocky
	MAPFILTER_BILINEAR
};

/** \brief A 2D greyscale image that is assigned to a certain region of your world to represent density levels.

This class is used by various PagedLoader's internally, so it's not necessary to learn anything about this class.
However, you can achieve more advanced effects through the DensityMap class interface than you can with the standard
GrassLayer density map functions, for example.

Probably the most useful function in this class is getPixelBox(), which you can use to directly manipulate the
density map pixels in real-time. */
class DensityMap
{
public:
	static DensityMap *load(const Ogre::String &fileName, MapChannel channel = CHANNEL_COLOR);
	static DensityMap *load(Ogre::TexturePtr texture, MapChannel channel = CHANNEL_COLOR);
	void unload();

	/** \brief Sets the filtering mode used for this density map

	This function can be used to set the filtering mode used for your density map. By default,
	bilinear filtering is used (MAPFILTER_BILINEAR). If you disable filtering by using MAPFILTER_NONE,
	the resulting effect of the density map may look square and blocky, depending on the resolution of
	the map.

	MAPFILTER_NONE is slightly faster than MAPFILTER_BILINEAR, so use it if you don't notice any
	considerable blockyness. */
	void setFilter(MapFilter filter) { this->filter = filter; }

	/** \brief Returns the filtering mode being used for this density map */
	MapFilter getFilter() { return filter; }

	/** \brief Gets a pointer to the pixel data of the density map

	You can use this function to access the pixel data of the density map. The PixelBox
	returned is an image in PF_BYTE_L (aka. PF_L8) byte format. You can modify this image
	any way you like in real-time, so long as you do not change the byte format.

	This function is useful in editors where the density map needs to be changed dynamically.
	Note that although you can change this map in real-time, the changes won't be uploaded to your
	video card until you call PagedGeometry::reloadGeometry(). If you don't, the grass you see
	will remain unchanged. */
	Ogre::PixelBox getPixelBox()
	{
		assert(pixels);
		return *pixels;
	}
	
	/** \brief Gets the density level at the specified position

	The boundary given defines the area where this density map takes effect.
	Normally this is set to your terrain's bounds so the density map is aligned
	to your heightmap, but you could apply it anywhere you want. */
	inline float getDensityAt(float x, float z, const Ogre::TRect<Ogre::Real> &mapBounds)
	{
		if (filter == MAPFILTER_NONE)
			return _getDensityAt_Unfiltered(x, z, mapBounds);
		else
			return _getDensityAt_Bilinear(x, z, mapBounds);
	}

	float _getDensityAt_Unfiltered(float x, float z, const Ogre::TRect<Ogre::Real> &mapBounds);
	float _getDensityAt_Bilinear(float x, float z, const Ogre::TRect<Ogre::Real> &mapBounds);

private:
	DensityMap(Ogre::TexturePtr texture, MapChannel channel);
	~DensityMap();

	static std::map<Ogre::String, DensityMap*> selfList;
	Ogre::String selfKey;
	Ogre::uint32 refCount;

	MapFilter filter;
	Ogre::PixelBox *pixels;
};

/** \brief A 2D greyscale image that is assigned to a certain region of your world to represent color levels.

This class is used by various PagedLoader's internally, so it's not necessary to learn anything about this class.
However, you can achieve more advanced effects through the ColorMap class interface than you can with the standard
GrassLayer color map functions, for example.

Probably the most useful function in this class is getPixelBox(), which you can use to directly manipulate the
color map pixels in real-time. */
class ColorMap
{
public:
	static ColorMap *load(const Ogre::String &fileName, MapChannel channel = CHANNEL_COLOR);
	static ColorMap *load(Ogre::TexturePtr texture, MapChannel channel = CHANNEL_COLOR);
	void unload();

	/** \brief Sets the filtering mode used for this color map

	This function can be used to set the filtering mode used for your color map. By default,
	bilinear filtering is used (MAPFILTER_BILINEAR). If you disable filtering by using
	MAPFILTER_NONE, the resulting coloration may appear slightly pixelated, depending on the 
	resolution of the map.

	MAPFILTER_NONE is slightly faster than MAPFILTER_BILINEAR, so use it if you don't notice any
	considerable pixelation. */
	void setFilter(MapFilter filter) { this->filter = filter; }

	/** \brief Returns the filtering mode being used for this color map */
	MapFilter getFilter() { return filter; }

	/** \brief Gets a pointer to the pixel data of the color map

	You can use this function to access the pixel data of the color map. The PixelBox
	returned is an image in PF_A8R8G8B8 format when running with DirectX, and PF_A8B8G8R8
	when running with OpenGL. You can modify this image any way you like in
	real-time, so long as you do not change the byte format.

	This function is useful in editors where the color map needs to be changed dynamically.
	Note that although you can change this map in real-time, the changes won't be uploaded to your
	video card until you call PagedGeometry::reloadGeometry(). If you don't, the grass you see
	will remain unchanged. */
	Ogre::PixelBox getPixelBox()
	{
		assert(pixels);
		return *pixels;
	}

	/** \brief Gets the color value at the specified position

	A RenderSystem-specific 32-bit packed color value is used, so it can be fed directly to
	the video card.

	The boundary given defines the area where this color map takes effect.
	Normally this is set to your terrain's bounds so the color map is aligned
	to your heightmap, but you could apply it anywhere you want. */
	inline Ogre::uint32 getColorAt(float x, float z, const Ogre::TRect<Ogre::Real> &mapBounds)
	{
		if (filter == MAPFILTER_NONE)
			return _getColorAt(x, z, mapBounds);
		else
			return _getColorAt_Bilinear(x, z, mapBounds);
	}

	/** \brief Gets the color value at the specified position

	The unpacks the 32-bit color value into an Ogre::ColourValue and returns it. */
	inline Ogre::ColourValue getColorAt_Unpacked(float x, float z, const Ogre::TRect<Ogre::Real> &mapBounds)
	{
		Ogre::uint32 c;

		if (filter == MAPFILTER_NONE)
			c = _getColorAt(x, z, mapBounds);
		else
			c = _getColorAt_Bilinear(x, z, mapBounds);
		
		Ogre::Real r, g, b, a;
		static Ogre::VertexElementType format = Ogre::Root::getSingleton().getRenderSystem()->getColourVertexElementType();
		if (format == Ogre::VET_COLOUR_ARGB){
			b = ((c) & 0xFF) / 255.0f;
			g = ((c >> 8) & 0xFF) / 255.0f;
			r = ((c >> 16) & 0xFF) / 255.0f;
			a = ((c >> 24) & 0xFF) / 255.0f;
		} else {
			r = ((c) & 0xFF) / 255.0f;
			g = ((c >> 8) & 0xFF) / 255.0f;
			b = ((c >> 16) & 0xFF) / 255.0f;
			a = ((c >> 24) & 0xFF) / 255.0f;
		}
		
		return Ogre::ColourValue(r, g, b, a);
	}

private:
	ColorMap(Ogre::TexturePtr map, MapChannel channel);
	~ColorMap();

	static std::map<Ogre::String, ColorMap*> selfList;
	Ogre::String selfKey;
	Ogre::uint32 refCount;

	//Directly interpolates two Ogre::uint32 colors
	Ogre::uint32 _interpolateColor(Ogre::uint32 color1, Ogre::uint32 color2, float ratio, float ratioInv);

	//Returns the color map value at the given location
	Ogre::uint32 _getColorAt(float x, float z, const Ogre::TRect<Ogre::Real> &mapBounds);

	//Returns the color map value at the given location with bilinear filtering
	Ogre::uint32 _getColorAt_Bilinear(float x, float z, const Ogre::TRect<Ogre::Real> &mapBounds);

	MapFilter filter;
	Ogre::PixelBox *pixels;
	Ogre::TRect<Ogre::Real> mapBounds;
};

}

#endif