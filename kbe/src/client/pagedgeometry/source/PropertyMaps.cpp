/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

#include "PropertyMaps.h"
#include "PagedGeometry.h"

#include <OgreRoot.h>
#include <OgrePrerequisites.h>
#include <OgrePixelFormat.h>
#include <OgreTexture.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRenderSystem.h>
#include <OgreString.h>
#include <OgreStringConverter.h>
using namespace Ogre;

namespace Forests {

std::map<String, DensityMap*> DensityMap::selfList;

DensityMap *DensityMap::load(const String &fileName, MapChannel channel)
{
	//Load image
	TexturePtr map = TextureManager::getSingleton().load(fileName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	//Copy image to pixelbox
	return load(map, channel);
}

DensityMap *DensityMap::load(TexturePtr texture, MapChannel channel)
{
	const String key = texture->getName() + StringConverter::toString((int)channel);

	std::map<String, DensityMap*>::iterator i;
	i = selfList.find(key);

	DensityMap *m;
	if (i != selfList.end())
		m = i->second;
	else
		m = new DensityMap(texture, channel);

	++(m->refCount);
	return m;
}

void DensityMap::unload()
{
	--refCount;
	if (refCount == 0)
		delete this;
}

DensityMap::~DensityMap()
{
	assert(pixels);
	delete[] static_cast<uint8*>(pixels->data);
	delete pixels;

	//Remove self from selfList
	selfList.erase(selfKey);
}

DensityMap::DensityMap(TexturePtr map, MapChannel channel)
{
	assert(map.isNull() == false);
	filter = MAPFILTER_BILINEAR;

	//Add self to selfList
	selfKey = map->getName() + StringConverter::toString((int)channel);
	selfList.insert(std::pair<String, DensityMap*>(selfKey, this));
	refCount = 0;

	//Get the texture buffer
	HardwarePixelBufferSharedPtr buff = map->getBuffer();

	//Prepare a PixelBox (8-bit greyscale) to receive the density values
	pixels = new PixelBox(Box(0, 0, buff->getWidth(), buff->getHeight()), PF_BYTE_L);
	pixels->data = new uint8[pixels->getConsecutiveSize()];

	if (channel == CHANNEL_COLOR){
		//Copy to the greyscale density map directly if no channel extraction is necessary
		buff->blitToMemory(*pixels);
	} else {
		//If channel extraction is necessary, first convert to a PF_R8G8B8A8 format PixelBox
		//This is necessary for the code below to properly extract the desired channel
		PixelBox tmpPixels(Box(0, 0, buff->getWidth(), buff->getHeight()), PF_R8G8B8A8);
		tmpPixels.data = new uint8[tmpPixels.getConsecutiveSize()];
		buff->blitToMemory(tmpPixels);

        unsigned char rgba_shift[4];
        Ogre::PixelUtil::getBitShifts(tmpPixels.format, rgba_shift);

		//Pick out a channel from the pixel buffer
		size_t channelOffset;
		switch (channel){
			case CHANNEL_RED: channelOffset = rgba_shift[0] / 8; break;
			case CHANNEL_GREEN: channelOffset = rgba_shift[1] / 8; break;
			case CHANNEL_BLUE: channelOffset = rgba_shift[2] / 8; break;
			case CHANNEL_ALPHA: channelOffset = rgba_shift[3] / 8; break;
			default: OGRE_EXCEPT(0, "Invalid channel", "GrassLayer::setDensityMap()"); break;
		}

		//And copy that channel into the density map
		uint8 *inputPtr = (uint8*)tmpPixels.data + channelOffset;
		uint8 *outputPtr = (uint8*)pixels->data;
		uint8 *outputEndPtr = outputPtr + pixels->getConsecutiveSize();
		while (outputPtr != outputEndPtr){
			*outputPtr++ = *inputPtr;
			inputPtr += 4;
		}

		//Finally, delete the temporary PF_R8G8B8A8 pixel buffer
		delete[] static_cast<uint8*>(tmpPixels.data);
	}
}


//Returns the density map value at the given location
//Make sure a density map exists before calling this.
float DensityMap::_getDensityAt_Unfiltered(float x, float z, const TRect<Real> &mapBounds)
{
	assert(pixels);

	// Early out if the coordinates are outside map bounds.
	if(x < mapBounds.left || x >= mapBounds.right || z < mapBounds.top || z >= mapBounds.bottom)
	{
		return 0.0f;
	}

	uint32 mapWidth = (uint32)pixels->getWidth();
	uint32 mapHeight = (uint32)pixels->getHeight();
	float boundsWidth = mapBounds.width();
	float boundsHeight = mapBounds.height();

	uint32 xindex = mapWidth * (x - mapBounds.left) / boundsWidth;
	uint32 zindex = mapHeight * (z - mapBounds.top) / boundsHeight;

	uint8 *data = (uint8*)pixels->data;
	float val = data[mapWidth * zindex + xindex] / 255.0f;

	return val;
}

//Returns the density map value at the given location with bilinear filtering
//Make sure a density map exists before calling this.
float DensityMap::_getDensityAt_Bilinear(float x, float z, const TRect<Real> &mapBounds)
{
	assert(pixels);

	// Early out if the coordinates are outside map bounds.
	if(x < mapBounds.left || x >= mapBounds.right || z < mapBounds.top || z >= mapBounds.bottom)
	{
		return 0.0f;
	}

	uint32 mapWidth = (uint32)pixels->getWidth();
	uint32 mapHeight = (uint32)pixels->getHeight();
	float boundsWidth = mapBounds.width();
	float boundsHeight = mapBounds.height();

	float xIndexFloat = (mapWidth * (x - mapBounds.left) / boundsWidth) - 0.5f;
	float zIndexFloat = (mapHeight * (z - mapBounds.top) / boundsHeight) - 0.5f;

	uint32 xIndex = xIndexFloat;
	uint32 zIndex = zIndexFloat;
	if (xIndex < 0 || zIndex < 0 || xIndex >= mapWidth-1 || zIndex >= mapHeight-1)
		return 0.0f;

	float xRatio = xIndexFloat - xIndex;
	float xRatioInv = 1 - xRatio;
	float zRatio = zIndexFloat - zIndex;
	float zRatioInv = 1 - zRatio;

	uint8 *data = (uint8*)pixels->data;

	float val11 = data[mapWidth * zIndex + xIndex] / 255.0f;
	float val21 = data[mapWidth * zIndex + xIndex + 1] / 255.0f;
	float val12 = data[mapWidth * ++zIndex + xIndex] / 255.0f;
	float val22 = data[mapWidth * zIndex + xIndex + 1] / 255.0f;

	float val1 = xRatioInv * val11 + xRatio * val21;
	float val2 = xRatioInv * val12 + xRatio * val22;

	float val = zRatioInv * val1 + zRatio * val2;

	return val;
}



//----------------------------------------------------------------------------------------------

std::map<String, ColorMap*> ColorMap::selfList;

ColorMap *ColorMap::load(const String &fileName, MapChannel channel)
{
	//Load image
	TexturePtr map = TextureManager::getSingleton().load(fileName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	//Copy image to pixelbox
	return load(map, channel);
}

ColorMap *ColorMap::load(TexturePtr texture, MapChannel channel)
{
	const String key = texture->getName() + StringConverter::toString((int)channel);

	std::map<String, ColorMap*>::iterator i;
	i = selfList.find(key);

	ColorMap *m;
	if (i != selfList.end())
		m = i->second;
	else
		m = new ColorMap(texture, channel);

	++(m->refCount);
	return m;
}

void ColorMap::unload()
{
	--refCount;
	if (refCount == 0)
		delete this;
}

ColorMap::~ColorMap()
{
	assert(pixels);
	delete[] static_cast<uint8*>(pixels->data);
	delete pixels;

	//Remove self from selfList
	selfList.erase(selfKey);
}

ColorMap::ColorMap(TexturePtr map, MapChannel channel)
{
	assert(map.isNull() == false);
	filter = MAPFILTER_BILINEAR;

	//Add self to selfList
	selfKey = map->getName() + StringConverter::toString((int)channel);
	selfList.insert(std::pair<String, ColorMap*>(selfKey, this));
	refCount = 0;

	//Get the texture buffer
	HardwarePixelBufferSharedPtr buff = map->getBuffer();

	//Prepare a PixelBox (24-bit RGB) to receive the color values
	VertexElementType format = Root::getSingleton().getRenderSystem()->getColourVertexElementType();
	switch (format){
		case VET_COLOUR_ARGB:
			//DirectX9
			pixels = new PixelBox(Box(0, 0, buff->getWidth(), buff->getHeight()), PF_A8R8G8B8);
			break;
		case VET_COLOUR_ABGR:
			//OpenGL
			pixels = new PixelBox(Box(0, 0, buff->getWidth(), buff->getHeight()), PF_A8B8G8R8);
			//Patch for Ogre's incorrect blitToMemory() when copying from PF_L8 in OpenGL
			if (buff->getFormat() == PF_L8)
				channel = CHANNEL_RED;
			break;
		default:
			OGRE_EXCEPT(0, "Unknown RenderSystem color format", "GrassLayer::setColorMap()");
			break;
	}

	pixels->data = new uint8[pixels->getConsecutiveSize()];

	if (channel == CHANNEL_COLOR){
		//Copy to the color map directly if no channel extraction is necessary
		buff->blitToMemory(*pixels);
	} else {
		//If channel extraction is necessary, first convert to a PF_R8G8B8A8 format PixelBox
		//This is necessary for the code below to properly extract the desired channel
		PixelBox tmpPixels(Box(0, 0, buff->getWidth(), buff->getHeight()), PF_R8G8B8A8);
		tmpPixels.data = new uint8[tmpPixels.getConsecutiveSize()];
		buff->blitToMemory(tmpPixels);

		//Pick out a channel from the pixel buffer
		size_t channelOffset;
		switch (channel){
			case CHANNEL_RED: channelOffset = 3; break;
			case CHANNEL_GREEN: channelOffset = 2; break;
			case CHANNEL_BLUE: channelOffset = 1; break;
			case CHANNEL_ALPHA: channelOffset = 0; break;
			default: OGRE_EXCEPT(0, "Invalid channel", "ColorMap::ColorMap()"); break;
		}

		//And copy that channel into the density map
		uint8 *inputPtr = (uint8*)tmpPixels.data + channelOffset;
		uint8 *outputPtr = (uint8*)pixels->data;
		uint8 *outputEndPtr = outputPtr + pixels->getConsecutiveSize();
		while (outputPtr != outputEndPtr){
			*outputPtr++ = *inputPtr;
			*outputPtr++ = *inputPtr;
			*outputPtr++ = *inputPtr;
			*outputPtr++ = 0xFF;	//Full alpha
			inputPtr += 4;
		}

		//Finally, delete the temporary PF_R8G8B8A8 pixel buffer
		delete[] static_cast<uint8*>(tmpPixels.data);
	}
}

//Returns the color map value at the given location
uint32 ColorMap::_getColorAt(float x, float z, const TRect<Real> &mapBounds)
{
	assert(pixels);

	// Early out if the coordinates are outside map bounds.
	if(x < mapBounds.left || x >= mapBounds.right || z < mapBounds.top || z >= mapBounds.bottom)
	{
		return 0xFFFFFFFF;
	}

	uint32 mapWidth = (uint32)pixels->getWidth();
	uint32 mapHeight = (uint32)pixels->getHeight();
	float boundsWidth = mapBounds.width();
	float boundsHeight = mapBounds.height();

	uint32 xindex = mapWidth * (x - mapBounds.left) / boundsWidth;
	uint32 zindex = mapHeight * (z - mapBounds.top) / boundsHeight;

	uint32 *data = (uint32*)pixels->data;
	return data[mapWidth * zindex + xindex];
}

uint32 ColorMap::_interpolateColor(uint32 color1, uint32 color2, float ratio, float ratioInv)
{
	uint8 a1, b1, c1, d1;
	a1 = (color1 & 0xFF);
	b1 = (color1 >> 8 & 0xFF);
	c1 = (color1 >> 16 & 0xFF);
	d1 = (color1 >> 24 & 0xFF);

	uint8 a2, b2, c2, d2;
	a2 = (color2 & 0xFF);
	b2 = (color2 >> 8 & 0xFF);
	c2 = (color2 >> 16 & 0xFF);
	d2 = (color2 >> 24 & 0xFF);

	uint8 a, b, c, d;
	a = ratioInv * a1 + ratio * a2;
	b = ratioInv * b1 + ratio * b2;
	c = ratioInv * c1 + ratio * c2;
	d = ratioInv * d1 + ratio * d2;

	uint32 clr = a | (b << 8) | (c << 16) | (d << 24);
	return clr;
}

uint32 ColorMap::_getColorAt_Bilinear(float x, float z, const TRect<Real> &mapBounds)
{
	assert(pixels);

	// Early out if the coordinates are outside map bounds.
	if(x < mapBounds.left || x >= mapBounds.right || z < mapBounds.top || z >= mapBounds.bottom)
	{
		return 0xFFFFFFFF;
	}

	uint32 mapWidth = (uint32)pixels->getWidth();
	uint32 mapHeight = (uint32)pixels->getHeight();
	float boundsWidth = mapBounds.width();
	float boundsHeight = mapBounds.height();

	float xIndexFloat = (mapWidth * (x - mapBounds.left) / boundsWidth) - 0.5f;
	float zIndexFloat = (mapHeight * (z - mapBounds.top) / boundsHeight) - 0.5f;

	uint32 xIndex = xIndexFloat;
	uint32 zIndex = zIndexFloat;
	if (xIndex < 0 || zIndex < 0 || xIndex > mapWidth-1 || zIndex > mapHeight-1)
		return 0xFFFFFFFF;

	float xRatio = xIndexFloat - xIndex;
	float xRatioInv = 1 - xRatio;
	float zRatio = zIndexFloat - zIndex;
	float zRatioInv = 1 - zRatio;

	uint32 *data = (uint32*)pixels->data;

	uint32 val11 = data[mapWidth * zIndex + xIndex];
	uint32 val21 = data[mapWidth * zIndex + xIndex + 1];
	uint32 val12 = data[mapWidth * ++zIndex + xIndex];
	uint32 val22 = data[mapWidth * zIndex + xIndex + 1];

	uint32 val1 = _interpolateColor(val11, val21, xRatio, xRatioInv);
	uint32 val2 = _interpolateColor(val12, val22, xRatio, xRatioInv);

	uint32 val = _interpolateColor(val1, val2, zRatio, zRatioInv);

	return val;
}
}
