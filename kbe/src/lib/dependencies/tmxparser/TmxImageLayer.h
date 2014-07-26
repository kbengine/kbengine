//-----------------------------------------------------------------------------
// TmxTileset.h
//
// Copyright (c) 2010-2014, Tamir Atias
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL TAMIR ATIAS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: Tamir Atias
//-----------------------------------------------------------------------------
#ifndef __TMX_IMAGELAYER_H__
#define __TMX_IMAGELAYER_H__

#include <string>
#include <vector>

#include "TmxPropertySet.h"

class TiXmlNode;

namespace Tmx 
{
	class Map;
	class Image;

	//-------------------------------------------------------------------------
	// A class used for storing information about each of the tilesets.
	// A tileset is a collection of tiles, of whom each may contain properties.
	// The tileset class itself does not have properties.
	//-------------------------------------------------------------------------
	class ImageLayer 
	{
	public:
		ImageLayer(const Tmx::Map *_map);
		~ImageLayer();

		// Parse a ImageLayer element.
		void Parse(const TiXmlNode *imageLayerNode);

		// Returns the name of the ImageLayer.
		const std::string &GetName() const { return name; }

		// Get the width of the ImageLayer.
		int GetWidth() const { return width; } 

		// Get the height of the ImageLayer.
		int GetHeight() const { return height; }

		// Get the visibility of the ImageLayer.
		bool IsVisible() const { return visible; }

		// Returns a variable containing information 
		// about the image of the ImageLayer.
		const Tmx::Image* GetImage() const { return image; }

		// Get a set of properties regarding the ImageLayer.
		const Tmx::PropertySet &GetProperties() const { return properties; }

		// Get the zorder of the ImageLayer.
		int GetZOrder() const { return zOrder; }
		
		// Set the zorder of the ImageLayer.
		void SetZOrder( int z ) { zOrder = z; }

	private:
		const Tmx::Map *map;

		std::string name;
		
		int width;
		int height;

		float opacity;
		bool visible;
		int zOrder;
		
		Tmx::Image* image;

		Tmx::PropertySet properties;
	};
};

#endif

