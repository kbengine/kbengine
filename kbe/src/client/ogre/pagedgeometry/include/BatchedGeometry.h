/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

//BatchedGeometry.h
//A "lightweight" version of Ogre::StaticGeometry, which gives you a little more control
//over the batch materials, etc.
//-------------------------------------------------------------------------------------

#ifndef __BatchedGeometry_H__
#define __BatchedGeometry_H__

#include <OgrePrerequisites.h>
#include <OgreMovableObject.h>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>

namespace Forests {

class BatchedGeometry: public Ogre::MovableObject
{
public:
	BatchedGeometry(Ogre::SceneManager *mgr, Ogre::SceneNode *rootSceneNode);
	~BatchedGeometry();

	virtual void addEntity(Ogre::Entity *ent, const Ogre::Vector3 &position, const Ogre::Quaternion &orientation = Ogre::Quaternion::IDENTITY, const Ogre::Vector3 &scale = Ogre::Vector3::UNIT_SCALE, const Ogre::ColourValue &color = Ogre::ColourValue::White);
	void build();
	void clear();

	Ogre::Vector3 _convertToLocal(const Ogre::Vector3 &globalVec) const;

	void _notifyCurrentCamera(Ogre::Camera *cam);
	void _updateRenderQueue(Ogre::RenderQueue *queue);
	bool isVisible();
	const Ogre::AxisAlignedBox &getBoundingBox(void) const { return bounds; }
	Ogre::Real getBoundingRadius(void) const { return radius; }
	const Ogre::String &getMovableType(void) const { static Ogre::String t = "BatchedGeometry"; return t; }

	#if !(OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR <= 4)
	//Shaggoth compatibility and upcoming versions
	void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables) {}
	#endif
	
	class SubBatch: public Ogre::Renderable
	{
	public:
		SubBatch(BatchedGeometry *parent, Ogre::SubEntity *ent);
		~SubBatch();

		void addSubEntity(Ogre::SubEntity *ent, const Ogre::Vector3 &position, const Ogre::Quaternion &orientation, const Ogre::Vector3 &scale, const Ogre::ColourValue &color = Ogre::ColourValue::White, void* userData = NULL);
		virtual void build();
		void clear();
		
		void setMaterial(Ogre::MaterialPtr &mat) { material = mat; }
		void setMaterialName(const Ogre::String &mat) { material = Ogre::MaterialManager::getSingleton().getByName(mat); }
		inline Ogre::String getMaterialName() const { return material->getName(); }

		void addSelfToRenderQueue(Ogre::RenderQueue *queue, Ogre::uint8 group);
		void getRenderOperation(Ogre::RenderOperation& op);
		Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
		const Ogre::LightList& getLights(void) const;

		Ogre::Technique *getTechnique() const { return bestTechnique; }
		const Ogre::MaterialPtr &getMaterial(void) const { return material; }
		void getWorldTransforms(Ogre::Matrix4* xform) const { *xform = parent->_getParentNodeFullTransform(); }
		const Ogre::Quaternion& getWorldOrientation(void) const { return parent->sceneNode->_getDerivedOrientation(); }
		const Ogre::Vector3& getWorldPosition(void) const { return parent->sceneNode->_getDerivedPosition(); }
		bool castsShadows(void) const { return parent->getCastShadows(); }

		Ogre::VertexData *vertexData;
		Ogre::IndexData *indexData;

	private:
		// This function is used to make a single clone of materials used, since the materials
		// will be modified by the batch system (and it wouldn't be good to modify the original materials
		// that the user may be using somewhere else).
		Ogre::Material *getMaterialClone(Ogre::Material &mat);

		Ogre::Technique *bestTechnique;	//This is recalculated every frame

	protected:
		bool built;
		bool requireVertexColors;
		Ogre::SubMesh *meshType;
		BatchedGeometry *parent;
		Ogre::MaterialPtr material;
		
		// A structure defining the desired position/orientation/scale of a batched mesh. The
		// SubMesh is not specified since that can be determined by which MeshQueue this belongs to.
		struct QueuedMesh
		{
			Ogre::SubMesh *mesh;
			Ogre::Vector3 position;
			Ogre::Quaternion orientation;
			Ogre::Vector3 scale;
			Ogre::ColourValue color;
			void* userData;
		};
		typedef std::vector<QueuedMesh>::iterator MeshQueueIterator;
		typedef std::vector<QueuedMesh> MeshQueue;
		MeshQueue meshQueue;	//The list of meshes to be added to this batch
	};


private:
	Ogre::Real radius;

	Ogre::SceneManager *sceneMgr;
	Ogre::SceneNode *sceneNode, *parentSceneNode;

	Ogre::Real minDistanceSquared;
	bool withinFarDistance;


protected:
	static void extractVertexDataFromShared(Ogre::MeshPtr mesh);

	Ogre::String getFormatString(Ogre::SubEntity *ent);
	typedef std::map<Ogre::String, SubBatch*> SubBatchMap;	//Stores a list of GeomBatch'es, using a format string (generated with getGeometryFormatString()) as the key value
	SubBatchMap subBatchMap;
	Ogre::Vector3 center;	
	Ogre::AxisAlignedBox bounds;
	bool boundsUndefined;

	bool built;

public:
	typedef Ogre::MapIterator<SubBatchMap> SubBatchIterator;
	SubBatchIterator getSubBatchIterator() const;
};


}

#endif
