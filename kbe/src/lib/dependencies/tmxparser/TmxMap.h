//-----------------------------------------------------------------------------
// TmxMap.h
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
#ifndef __TMX_MAP_H__
#define __TMX_MAP_H__

#include <vector>
#include <string>

#include "TmxPropertySet.h"

namespace Tmx 
{
	class Layer;
	class ImageLayer;
	class ObjectGroup;
	class Tileset;

	//-------------------------------------------------------------------------
	// Error in handling of the Map class.
	//-------------------------------------------------------------------------
	enum MapError 
	{
		// A file could not be opened. (usually due to permission problems)
		TMX_COULDNT_OPEN = 0x01,

		// There was an error in parsing the TMX file.
		// This is being caused by TinyXML parsing problems.
		TMX_PARSING_ERROR = 0x02,
		
		// The size of the file is invalid.
		TMX_INVALID_FILE_SIZE = 0x04
	};

	//-------------------------------------------------------------------------
	// The way the map is viewed.
	//-------------------------------------------------------------------------
	enum MapOrientation 
	{
		// This map is an orthogonal map.
		TMX_MO_ORTHOGONAL = 0x01,

		// This map is an isometric map.
		TMX_MO_ISOMETRIC = 0x02,

		// This map is an isometric staggered map.
		TMX_MO_STAGGERED = 0x03
	};

	//-------------------------------------------------------------------------
	// This class is the root class of the parser.
	// It has all of the information in regard to the TMX file.
	// This class has a property set.
	//-------------------------------------------------------------------------
	class Map 
	{
	public:
		Map(const Map &_map);

	public:
		Map();
		~Map();

		// Read a file and parse it.
		// Note: use '/' instead of '\\' as it is using '/' to find the path.
		void ParseFile(const std::string &fileName);
		
		// Parse text containing TMX formatted XML.
		void ParseText(const std::string &text);

		// Get the filename used to read the map.
		const std::string &GetFilename() { return file_name; }

		// Get a path to the directory of the map file if any.
		const std::string &GetFilepath() const { return file_path; }

		// Get the version of the map.
		double GetVersion() const { return version; }

		// Get the orientation of the map.
		Tmx::MapOrientation GetOrientation() const { return orientation; }

		// Get the width of the map, in tiles.
		int GetWidth() const { return width; }

		// Get the height of the map, in tiles.
		int GetHeight() const { return height; }

		// Get the width of a tile, in pixels.
		int GetTileWidth() const { return tile_width; }

		// Get the height of a tile, in pixels.
		int GetTileHeight() const { return tile_height; }

		// Get the layer at a certain index.
		Tmx::Layer *GetLayer(int index) { return layers.at(index); }

		// Get the amount of layers.
		int GetNumLayers() const { return layers.size(); }

		// Get the whole layers collection.
		const std::vector< Tmx::Layer* > &GetLayers() const { return layers; }

		// Get the object group at a certain index.
		const Tmx::ObjectGroup *GetObjectGroup(int index) const { return object_groups.at(index); }

		// Get the amount of object groups.
		int GetNumObjectGroups() const { return object_groups.size(); }

		// Get the whole object group collection.
		const std::vector< Tmx::ObjectGroup* > &GetObjectGroups() const { return object_groups; }

		// Get the layer at a certain index.
		const Tmx::ImageLayer *GetImageLayer(int index) const { return image_layers.at(index); }

		// Get the amount of layers.
		int GetNumImageLayers() const { return image_layers.size(); }

		// Get the whole layers collection.
		const std::vector< Tmx::ImageLayer* > &GetImageLayers() const { return image_layers; }

		// Find the tileset index for a tileset using a tile gid.
		int FindTilesetIndex(int gid) const;

		// Find a tileset for a specific gid.
		const Tmx::Tileset *FindTileset(int gid) const;

		// Get a tileset by an index.
		const Tmx::Tileset *GetTileset(int index) const { return tilesets.at(index); }

		// Get the amount of tilesets.
		int GetNumTilesets() const { return tilesets.size(); }

		// Get the collection of tilesets.
		const std::vector< Tmx::Tileset* > &GetTilesets() const { return tilesets; }

		// Get whether there was an error or not.
		bool HasError() const { return has_error; }

		// Get an error string containing the error in text format.
		const std::string &GetErrorText() const { return error_text; }

		// Get a number that identifies the error. (TMX_ preceded constants)
		unsigned char GetErrorCode() const { return error_code; }

		// Get the property set.
		const Tmx::PropertySet &GetProperties() const { return properties; }

	private:
		std::string file_name;
		std::string file_path;

		double version;
		Tmx::MapOrientation orientation;

		int width;
		int height;
		int tile_width;
		int tile_height;

		std::vector< Tmx::Layer* > layers;
		std::vector< Tmx::ImageLayer* > image_layers;
		std::vector< Tmx::ObjectGroup* > object_groups;
		std::vector< Tmx::Tileset* > tilesets;

		bool has_error;
		unsigned char error_code;
		std::string error_text;

		Tmx::PropertySet properties;
	};
};

#endif

