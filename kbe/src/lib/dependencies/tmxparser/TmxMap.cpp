//-----------------------------------------------------------------------------
// TmxMap.cpp
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
#include <stdio.h>

#include "TmxMap.h"
#include "TmxTileset.h"
#include "TmxLayer.h"
#include "TmxObjectGroup.h"
#include "TmxImageLayer.h"

#ifdef USE_SDL2_LOAD
#include <SDL.h>
#endif

using std::vector;
using std::string;

namespace Tmx 
{
	Map::Map() 
		: file_name()
		, file_path()
		, version(0.0)
		, orientation(TMX_MO_ORTHOGONAL)
		, width(0)
		, height(0)
		, tile_width(0)
		, tile_height(0)
		, layers()
		, object_groups()
		, tilesets() 
		, has_error(false)
		, error_code(0)
		, error_text()
	{}

	Map::Map(const Map &_map)
		: file_name(_map.file_name)
		, file_path(_map.file_path)
		, version(_map.version)
		, orientation(_map.orientation)
		, width(_map.width)
		, height(_map.height)
		, tile_width(_map.tile_width)
		, tile_height(_map.tile_height)
		, layers()
		, object_groups()
		, tilesets() 
		, has_error(_map.has_error)
		, error_code(_map.error_code)
		, error_text(_map.error_text)
	{
		std::vector< Tmx::Layer* >::const_iterator iter = _map.layers.begin();
		for(; iter != _map.layers.end(); iter++)
		{
			layers.push_back(new Tmx::Layer(*(*iter)));
		}

		std::vector< Tmx::Tileset* >::const_iterator iter1 = _map.tilesets.begin();
		for(; iter1 != _map.tilesets.end(); iter1++)
		{
			tilesets.push_back(new Tmx::Tileset(*(*iter1)));
		}

	}

	Map::~Map() 
	{
		// Iterate through all of the object groups and delete each of them.
		vector< ObjectGroup* >::iterator ogIter;
		for (ogIter = object_groups.begin(); ogIter != object_groups.end(); ++ogIter) 
		{
			ObjectGroup *objectGroup = (*ogIter);
			
			if (objectGroup)
			{
				delete objectGroup;
				objectGroup = NULL;
			}
		}

		// Iterate through all of the layers and delete each of them.
		vector< Layer* >::iterator lIter;
		for (lIter = layers.begin(); lIter != layers.end(); ++lIter) 
		{
			Layer *layer = (*lIter);

			if (layer) 
			{
				delete layer;
				layer = NULL;
			}
		}

		// Iterate through all of the layers and delete each of them.
		vector< ImageLayer* >::iterator ilIter;
		for (ilIter = image_layers.begin(); ilIter != image_layers.end(); ++ilIter) 
		{
			ImageLayer *layer = (*ilIter);

			if (layer) 
			{
				delete layer;
				layer = NULL;
			}
		}

		// Iterate through all of the tilesets and delete each of them.
		vector< Tileset* >::iterator tsIter;
		for (tsIter = tilesets.begin(); tsIter != tilesets.end(); ++tsIter) 
		{
			Tileset *tileset = (*tsIter);
			
			if (tileset) 
			{
				delete tileset;
				tileset = NULL;
			}
		}
	}

	void Map::ParseFile(const string &fileName) 
	{
		file_name = fileName;

		int lastSlash = fileName.find_last_of("/");

		// Get the directory of the file using substring.
		if (lastSlash > 0) 
		{
			file_path = fileName.substr(0, lastSlash + 1);
		} 
		else 
		{
			file_path = "";
		}

		char* fileText;
		int fileSize;

		// Open the file for reading.
#ifdef USE_SDL2_LOAD
		SDL_RWops * file = SDL_RWFromFile (fileName.c_str(), "rb");
#else
		FILE *file = fopen(fileName.c_str(), "rb");
#endif

		// Check if the file could not be opened.
		if (!file) 
		{
			has_error = true;
			error_code = TMX_COULDNT_OPEN;
			error_text = "Could not open the file.";
			return;
		}
	
		// Find out the file size.	
#ifdef USE_SDL2_LOAD
		fileSize = file->size(file);
#else
		fseek(file, 0, SEEK_END);
		fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);
#endif
		
		// Check if the file size is valid.
		if (fileSize <= 0)
		{
			has_error = true;
			error_code = TMX_INVALID_FILE_SIZE;
			error_text = "The size of the file is invalid.";
			return;
		}

		// Allocate memory for the file and read it into the memory.
		fileText = new char[fileSize + 1];
		fileText[fileSize] = 0;
#ifdef USE_SDL2_LOAD
		file->read(file, fileText, 1, fileSize);
#else
		if (fread(fileText, 1, fileSize, file) < 1)
		{
			error_text = "Error in reading or end of file.\n";
			return;
		}
#endif

#ifdef USE_SDL2_LOAD
		file->close(file);
#else
		fclose(file);
#endif

		// Copy the contents into a C++ string and delete it from memory.
		std::string text(fileText, fileText+fileSize);
		delete [] fileText;

		ParseText(text);		
	}

	void Map::ParseText(const string &text) 
	{
		// Create a tiny xml document and use it to parse the text.
		TiXmlDocument doc;
		doc.Parse(text.c_str());
	
		// Check for parsing errors.
		if (doc.Error()) 
		{
			has_error = true;
			error_code = TMX_PARSING_ERROR;
			error_text = doc.ErrorDesc();
			return;
		}

		TiXmlNode *mapNode = doc.FirstChild("map");
		TiXmlElement* mapElem = mapNode->ToElement();

		// Read the map attributes.
		mapElem->Attribute("version", &version);
		mapElem->Attribute("width", &width);
		mapElem->Attribute("height", &height);
		mapElem->Attribute("tilewidth", &tile_width);
		mapElem->Attribute("tileheight", &tile_height);

		// Read the orientation
		std::string orientationStr = mapElem->Attribute("orientation");

		if (!orientationStr.compare("orthogonal")) 
		{
			orientation = TMX_MO_ORTHOGONAL;
		} 
		else if (!orientationStr.compare("isometric")) 
		{
			orientation = TMX_MO_ISOMETRIC;
		}
		else if (!orientationStr.compare("staggered")) 
		{
			orientation = TMX_MO_STAGGERED;
		}
		

		const TiXmlNode *node = mapElem->FirstChild();
		int zOrder = 0;
		while( node )
		{
			// Read the map properties.
			if( strcmp( node->Value(), "properties" ) == 0 )
			{
				properties.Parse(node);			
			}

			// Iterate through all of the tileset elements.
			if( strcmp( node->Value(), "tileset" ) == 0 )
			{
				// Allocate a new tileset and parse it.
				Tileset *tileset = new Tileset();
				tileset->Parse(node->ToElement());

				// Add the tileset to the list.
				tilesets.push_back(tileset);
			}

			// Iterate through all of the layer elements.			
			if( strcmp( node->Value(), "layer" ) == 0 )
			{
				// Allocate a new layer and parse it.
				Layer *layer = new Layer(this);
				layer->Parse(node);
				layer->SetZOrder( zOrder );
				++zOrder;

				// Add the layer to the list.
				layers.push_back(layer);
			}

			// Iterate through all of the imagen layer elements.			
			if( strcmp( node->Value(), "imagelayer" ) == 0 )
			{
				// Allocate a new layer and parse it.
				ImageLayer *imageLayer = new ImageLayer(this);
				imageLayer->Parse(node);
				imageLayer->SetZOrder( zOrder );
				++zOrder;

				// Add the layer to the list.
				image_layers.push_back(imageLayer);
			}

			// Iterate through all of the objectgroup elements.
			if( strcmp( node->Value(), "objectgroup" ) == 0 )
			{
				// Allocate a new object group and parse it.
				ObjectGroup *objectGroup = new ObjectGroup();
				objectGroup->Parse(node);
				objectGroup->SetZOrder( zOrder );
				++zOrder;
		
				// Add the object group to the list.
				object_groups.push_back(objectGroup);
			}

			node = node->NextSibling();
		}
	}

	int Map::FindTilesetIndex(int gid) const
	{
		// Clean up the flags from the gid (thanks marwes91).
		gid &= ~(FlippedHorizontallyFlag | FlippedVerticallyFlag | FlippedDiagonallyFlag);

		for (int i = tilesets.size() - 1; i > -1; --i) 
		{
			// If the gid beyond the tileset gid return its index.
			if (gid >= tilesets[i]->GetFirstGid()) 
			{
				return i;
			}
		}
		
		return -1;
	}

	const Tileset *Map::FindTileset(int gid) const 
	{
		for (int i = tilesets.size() - 1; i > -1; --i) 
		{
			// If the gid beyond the tileset gid return it.
			if (gid >= tilesets[i]->GetFirstGid()) 
			{
				return tilesets[i];
			}
		}
		
		return NULL;
	}
};
