/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/
#ifndef __BatchedGeometry_H__
#define __BatchedGeometry_H__

#include <OgrePrerequisites.h>
#include <OgreMovableObject.h>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>

namespace Forests
{
   //--------------------------------------------------------------------------
   /// A "lightweight" version of Ogre::StaticGeometry, which gives you a little
   /// more control over the batch materials, etc.
   class BatchedGeometry : public Ogre::MovableObject
   {
   public:
      //--------------------------------------------------------------------------
      /// Visible chunk of geometry.
      class SubBatch : public Ogre::Renderable
      {
      protected:
         // A structure defining the desired position/orientation/scale of a batched mesh. The
         // SubMesh is not specified since that can be determined by which MeshQueue this belongs to.
         struct QueuedMesh
         {
            QueuedMesh(Ogre::SubMesh* sm, const Ogre::Vector3 &pos, const Ogre::Quaternion &ori,
               const Ogre::Vector3 &scl, const Ogre::ColourValue &clr, void *userData_ = 0) :
            subMesh     (sm),
               position    (pos),
               orientation (ori),
               scale       (scl),
               color       (clr),
               userData    (userData_)
            {
               // empty
            }

            Ogre::SubMesh*    subMesh;
            Ogre::Vector3     position;
            Ogre::Quaternion  orientation;
            Ogre::Vector3     scale;
            Ogre::ColourValue color;
            void*             userData;
         };

         /// Queue of meshes for build batch of geometry
         typedef std::vector<QueuedMesh> TMeshQueue;


         // Function section

      public:
         /// Constructor
         SubBatch(BatchedGeometry *parent, Ogre::SubEntity *ent);
         /// Destructor
         ~SubBatch();

         /// 
         void addSubEntity(Ogre::SubEntity *ent, const Ogre::Vector3 &position,
            const Ogre::Quaternion &orientation, const Ogre::Vector3 &scale,
            const Ogre::ColourValue &color = Ogre::ColourValue::White, void* userData = NULL);

         /// Build (assemble a vertex/index buffers) geometry for rendering
         virtual void build();
         ///
         void clear();

         /// 
         void addSelfToRenderQueue(Ogre::RenderQueueGroup *rqg);
         ///
         void getRenderOperation(Ogre::RenderOperation& op);
         ///
         Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
         ///
         const Ogre::LightList& getLights(void) const;

         ///
         void setMaterial(Ogre::MaterialPtr &mat)                 { m_ptrMaterial = mat; }
         void setMaterialName(const Ogre::String &mat, const Ogre::String &rg =
            Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME)
         {
            m_ptrMaterial = Ogre::MaterialManager::getSingleton().getByName(mat, rg);
         }

         /// Get material name. Be careful, resource group name missing
         const Ogre::String& getMaterialName() const              { return m_ptrMaterial->getName(); }
         Ogre::Technique *getTechnique() const                    { return m_pBestTechnique; }
         const Ogre::MaterialPtr& getMaterial(void) const         { return m_ptrMaterial; }
         void getWorldTransforms(Ogre::Matrix4* xform) const      { *xform = m_pParentGeom->_getParentNodeFullTransform(); }
         const Ogre::Quaternion& getWorldOrientation(void) const  { return m_pParentGeom->m_pSceneNode->_getDerivedOrientation(); }
         const Ogre::Vector3& getWorldPosition(void) const        { return m_pParentGeom->m_pSceneNode->_getDerivedPosition(); }
         bool castsShadows(void) const                            { return m_pParentGeom->getCastShadows(); }

         // internal fuctions
      private:
         /// Build vertex of QueuedMesh if it have identity orientation
         static void _buildIdentiryOrientation(const QueuedMesh &queuedMesh, const Ogre::Vector3 &parentGeomCenter,
            const std::vector<Ogre::VertexDeclaration::VertexElementList> &vertexBufferElements, std::vector<Ogre::uchar*> &vertexBuffers,
            Ogre::VertexData *dst);
         /// Build vertex of QueuedMesh if it have some orientation
         static void _buildFullTransform(const QueuedMesh &queuedMesh, const Ogre::Vector3 &parentGeomCenter,
            const std::vector<Ogre::VertexDeclaration::VertexElementList> &vertexBufferElements, std::vector<Ogre::uchar*> &vertexBuffers,
            Ogre::VertexData *dst);


         // Data section class SubBatch

      public:
         Ogre::VertexData* m_pVertexData;          ///<
         Ogre::IndexData*  m_pIndexData;           ///<

      protected:
         bool              m_Built;                ///<
         bool              m_RequireVertexColors;  ///<
         Ogre::SubMesh*    m_pSubMesh;             ///< Ogre::SubMesh for Index/Vertex buffers manipulation
         BatchedGeometry*  m_pParentGeom;          ///<
         Ogre::MaterialPtr m_ptrMaterial;          ///<
         TMeshQueue        m_queueMesh;            ///< The list of meshes to be added to this batch

      private:
         Ogre::Technique*  m_pBestTechnique;       ///< Technique recalculated every frame

      }; // end class SubBatch
      //-----------------------------------------------------------------------

      /// Stores a list of GeomBatch'es, using a format string (generated with getGeometryFormatString()) as the key value
      typedef std::map<Ogre::String, SubBatch*>    TSubBatchMap;
      typedef Ogre::MapIterator<TSubBatchMap>      TSubBatchIterator;
      typedef Ogre::ConstMapIterator<TSubBatchMap> TConstSubBatchIterator;

   public:

      /// Constructor
      BatchedGeometry(Ogre::SceneManager *mgr, Ogre::SceneNode *rootSceneNode);
      ~BatchedGeometry();

      TConstSubBatchIterator getSubBatchIterator() const    { return TConstSubBatchIterator(m_mapSubBatch); }
      TSubBatchIterator getSubBatchIterator()               { return TSubBatchIterator(m_mapSubBatch); }

      virtual void addEntity(Ogre::Entity *ent, const Ogre::Vector3 &position,
         const Ogre::Quaternion &orientation = Ogre::Quaternion::IDENTITY,
         const Ogre::Vector3 &scale = Ogre::Vector3::UNIT_SCALE,
         const Ogre::ColourValue &color = Ogre::ColourValue::White);

      void build();
      void clear();

      Ogre::Vector3 _convertToLocal(const Ogre::Vector3 &globalVec) const;

      const Ogre::AxisAlignedBox &getBoundingBox(void) const   { return m_boundsAAB; }
      Ogre::Real getBoundingRadius(void) const                 { return m_fRadius; }

   private:
      bool isVisible();
      const Ogre::String& getMovableType(void) const;
      void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables) { /* empty */ }     
      void _notifyCurrentCamera(Ogre::Camera *cam);
      void _updateRenderQueue(Ogre::RenderQueue *queue);

   protected:
      static Ogre::String getFormatString(Ogre::SubEntity *ent);
      static void extractVertexDataFromShared(const Ogre::MeshPtr &mesh);


      // Data section of BatchedGeometry class
   protected:
      bool                    m_Built;
      bool                    m_BoundsUndefined;
      Ogre::Vector3           m_vecCenter;
      Ogre::AxisAlignedBox    m_boundsAAB;
      TSubBatchMap            m_mapSubBatch;
      /// Internal matrix for remap vertex type to vertex size instead call VertexElement::getTypeSize
      static const size_t     s_vertexType2Size[Ogre::VET_COLOUR_ABGR + 1];

   private:
      bool                    m_bWithinFarDistance;
      Ogre::Real              m_fRadius;
      Ogre::Real              m_fMinDistanceSquared;
      Ogre::SceneManager*     m_pSceneMgr;
      Ogre::SceneNode*        m_pSceneNode;
      Ogre::SceneNode*        m_pParentSceneNode;
   };

}

#endif
