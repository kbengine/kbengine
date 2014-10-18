//-----------------------------------------------------------------------------
// TmxTileset.cpp
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
#include <tinyxml.h>

#include "TmxImageLayer.h"
#include "TmxImage.h"

using std::vector;
using std::string;

namespace Tmx 
{
	ImageLayer::ImageLayer(const Tmx::Map *_map) 
		: map(_map)
		, name()
		, width(0)
		, height(0)
		, opacity(0)
		, visible(0)
		, zOrder(0)
		, image(NULL)
		, properties()
	{
	}

	ImageLayer::~ImageLayer() 
	{
		delete image;
	}

	void ImageLayer::Parse(const TiXmlNode *imageLayerNode) 
	{
		const TiXmlElement *imagenLayerElem = imageLayerNode->ToElement();

		// Read all the attributes into local variables.
		name = imagenLayerElem->Attribute("name");

		imagenLayerElem->Attribute("width", &width);
		imagenLayerElem->Attribute("height", &height);

		const char *opacityStr = imagenLayerElem->Attribute("opacity");
		if (opacityStr) 
		{
			opacity = (float)atof(opacityStr);
		}

		const char *visibleStr = imagenLayerElem->Attribute("visible");
		if (visibleStr) 
		{
			visible = atoi(visibleStr) != 0; // to prevent visual c++ from complaining..
		}

		// Parse the image.
		const TiXmlNode *imageNode = imagenLayerElem->FirstChild("image");
		
		if (imageNode) 
		{
			image = new Image();
			image->Parse(imageNode);
		}

		// Parse the properties if any.
		const TiXmlNode *propertiesNode = imagenLayerElem->FirstChild("properties");
		
		if (propertiesNode) 
		{
			properties.Parse(propertiesNode);
		}
	}

};
