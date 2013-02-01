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

#include <OgreRoot.h>
#include <OgreRenderSystem.h>
#include <OgreCamera.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreSceneNode.h>
#include <OgreString.h>
#include <OgreStringConverter.h>
#include <OgreEntity.h>
#include <OgreSubMesh.h>
#include <OgreSubEntity.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreHardwareBufferManager.h>
#include <OgreHardwareBuffer.h>
#include <OgreMaterialManager.h>
#include <OgreMaterial.h>
#include <string>

#include "BatchedGeometry.h"
#include "PagedGeometry.h"

using namespace Ogre;
using namespace Forests;


/// For Ogre 1.7.2 and 1.7.3 VertexElementType enum writed as
///
/// VET_FLOAT1 = 0
/// VET_FLOAT2 = 1
/// VET_FLOAT3 = 2
/// VET_FLOAT4 = 3
/// VET_COLOUR = 4
/// VET_SHORT1 = 5
/// VET_SHORT2 = 6
/// VET_SHORT3 = 7
/// VET_SHORT4 = 8
/// VET_UBYTE4 = 9
/// VET_COLOUR_ARGB = 10
/// VET_COLOUR_ABGR = 11
const size_t BatchedGeometry::s_vertexType2Size[VET_COLOUR_ABGR + 1] = {
   VertexElement::getTypeSize(VET_FLOAT1),
   VertexElement::getTypeSize(VET_FLOAT2),
   VertexElement::getTypeSize(VET_FLOAT3),
   VertexElement::getTypeSize(VET_FLOAT4),
   VertexElement::getTypeSize(VET_COLOUR),
   VertexElement::getTypeSize(VET_SHORT1),
   VertexElement::getTypeSize(VET_SHORT2),
   VertexElement::getTypeSize(VET_SHORT3),
   VertexElement::getTypeSize(VET_SHORT4),
   VertexElement::getTypeSize(VET_UBYTE4),
   VertexElement::getTypeSize(VET_COLOUR_ARGB),
   VertexElement::getTypeSize(VET_COLOUR_ABGR)
};


//-------------------------------------------------------------------------------------
///
BatchedGeometry::BatchedGeometry(Ogre::SceneManager *mgr, Ogre::SceneNode *rootSceneNode) :
m_fRadius            (0.f),
m_fMinDistanceSquared(0.f),
m_pSceneMgr          (mgr),
m_pSceneNode         (NULL),
m_pParentSceneNode   (rootSceneNode),
m_bWithinFarDistance (false),
m_Built              (false),
m_vecCenter          (Ogre::Vector3::ZERO),
m_BoundsUndefined    (true)
{
   assert(rootSceneNode);
}

//-----------------------------------------------------------------------------
///
BatchedGeometry::~BatchedGeometry()
{
   clear();
}

//-----------------------------------------------------------------------------
///
const Ogre::String& BatchedGeometry::getMovableType() const
{
   static const Ogre::String strType = "BatchedGeometry";
   return strType;
}


//-----------------------------------------------------------------------------
///
void BatchedGeometry::addEntity(Entity *ent, const Vector3 &position,
                                const Quaternion &orientation,
                                const Vector3 &scale,
                                const Ogre::ColourValue &color)
{
   const MeshPtr &mesh = ent->getMesh();

   //If shared vertex data is used, extract into non-shared data
   extractVertexDataFromShared(mesh);	

   //For each subentity
   for (uint32 i = 0; i < ent->getNumSubEntities(); ++i)
   {
      //Get the subentity
      SubEntity *subEntity = ent->getSubEntity(i);
      SubMesh *subMesh = subEntity->getSubMesh();

      //Generate a format string that uniquely identifies this material & vertex/index format
      if (subMesh->vertexData == NULL)
         OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "SubMesh vertex data not found!", "BatchedGeometry::addEntity()");
      String formatStr = getFormatString(subEntity);

      //If a batch using an identical format exists...
      SubBatch *batch = 0;
      TSubBatchMap::iterator batchIter = m_mapSubBatch.find(formatStr);
      if (batchIter != m_mapSubBatch.end())
         batch = batchIter->second; //Use the batch
      else  // Otherwise create a new batch
      {
         batch = new SubBatch(this, subEntity);
         m_mapSubBatch.insert(std::pair<String, SubBatch*>(formatStr, batch));
      }

      //Now add the submesh to the compatible batch
      batch->addSubEntity(subEntity, position, orientation, scale, color);
   }

   //Update bounding box
   Matrix4 mat(orientation);
   mat.setScale(scale);
   AxisAlignedBox entBounds = ent->getBoundingBox();
   entBounds.transform(mat);

   if (m_BoundsUndefined)
   {
      m_boundsAAB.setMinimum(entBounds.getMinimum() + position);
      m_boundsAAB.setMaximum(entBounds.getMaximum() + position);
      m_BoundsUndefined = false;
   }
   else
   {
      Vector3 min = m_boundsAAB.getMinimum();
      Vector3 max = m_boundsAAB.getMaximum();
      min.makeFloor(entBounds.getMinimum() + position);
      max.makeCeil(entBounds.getMaximum() + position);
      m_boundsAAB.setMinimum(min);
      m_boundsAAB.setMaximum(max);
   }
}


//-----------------------------------------------------------------------------
///
uint32 CountUsedVertices(IndexData *id, std::map<uint32, uint32> &ibmap)
{
   uint32 i, count;
   switch (id->indexBuffer->getType()) {
      case HardwareIndexBuffer::IT_16BIT:
         {
            uint16 *data = (uint16*)id->indexBuffer->lock(id->indexStart * sizeof(uint16), 
               id->indexCount * sizeof(uint16), HardwareBuffer::HBL_READ_ONLY);

            for (i = 0; i < id->indexCount; i++) {
               uint16 index = data[i];
               if (ibmap.find(index) == ibmap.end()) ibmap[index] = (uint32)(ibmap.size());
            }
            count = (uint32)ibmap.size();
            id->indexBuffer->unlock();
         }
         break;

      case HardwareIndexBuffer::IT_32BIT:
         {
            uint32 *data = (uint32*)id->indexBuffer->lock(id->indexStart * sizeof(uint32), 
               id->indexCount * sizeof(uint32), HardwareBuffer::HBL_READ_ONLY);

            for (i = 0; i < id->indexCount; i++) {
               uint32 index = data[i];
               if (ibmap.find(index) == ibmap.end()) ibmap[index] = (uint32)(ibmap.size());
            }
            count = (uint32)ibmap.size();
            id->indexBuffer->unlock();
         }
         break;

      default:
         throw new Ogre::Exception(0, "Unknown index buffer type", "Converter.cpp::CountVertices");
         break;
   }

   return count;
}


//-----------------------------------------------------------------------------
///
void BatchedGeometry::extractVertexDataFromShared(const Ogre::MeshPtr &mesh)
{
   if (mesh.isNull() || !mesh->sharedVertexData)
      return;

   Mesh::SubMeshIterator subMeshIterator = mesh->getSubMeshIterator();

   // Get shared vertex data
   VertexData *oldVertexData = mesh->sharedVertexData;

   while (subMeshIterator.hasMoreElements()) {
      SubMesh *subMesh = subMeshIterator.getNext();

      // Get index data
      IndexData *indexData = subMesh->indexData;
      HardwareIndexBufferSharedPtr ib = indexData->indexBuffer;

      // Create new nonshared vertex data
      std::map<uint32, uint32> indicesMap;
      VertexData *newVertexData = new VertexData();
      newVertexData->vertexCount = CountUsedVertices(indexData, indicesMap);
      //delete newVertexData->vertexDeclaration;
      newVertexData->vertexDeclaration = oldVertexData->vertexDeclaration->clone();

      // Create new vertex buffers
      uint32 buffersCount = (uint32)oldVertexData->vertexBufferBinding->getBufferCount();
      for (uint32 bufferIndex = 0; bufferIndex < buffersCount; bufferIndex++) {

         // Lock shared vertex buffer
         HardwareVertexBufferSharedPtr oldVertexBuffer = oldVertexData->vertexBufferBinding->getBuffer(bufferIndex);
         size_t vertexSize = oldVertexBuffer->getVertexSize();
         uint8 *oldLock = (uint8*)oldVertexBuffer->lock(0, oldVertexData->vertexCount * vertexSize, HardwareBuffer::HBL_READ_ONLY);

         // Create and lock nonshared vertex buffer
         HardwareVertexBufferSharedPtr newVertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
            vertexSize, newVertexData->vertexCount, oldVertexBuffer->getUsage(), oldVertexBuffer->hasShadowBuffer());
         uint8 *newLock = (uint8*)newVertexBuffer->lock(0, newVertexData->vertexCount * vertexSize, HardwareBuffer::HBL_NORMAL);

         // Copy vertices from shared vertex buffer into nonshared vertex buffer
         std::map<uint32, uint32>::iterator i, iend = indicesMap.end();
         for (i = indicesMap.begin(); i != iend; i++) {
            memcpy(newLock + vertexSize * i->second, oldLock + vertexSize * i->first, vertexSize);
         }

         // Unlock vertex buffers
         oldVertexBuffer->unlock();
         newVertexBuffer->unlock();

         // Bind new vertex buffer
         newVertexData->vertexBufferBinding->setBinding(bufferIndex, newVertexBuffer);
      }

      // Re-create index buffer
      switch (indexData->indexBuffer->getType()) {
         case HardwareIndexBuffer::IT_16BIT:
            {
               uint16 *data = (uint16*)indexData->indexBuffer->lock(indexData->indexStart * sizeof(uint16), 
                  indexData->indexCount * sizeof(uint16), HardwareBuffer::HBL_NORMAL);

               for (uint32 i = 0; i < indexData->indexCount; i++) {
                  data[i] = (uint16)indicesMap[data[i]];
               }

               indexData->indexBuffer->unlock();
            }
            break;

         case HardwareIndexBuffer::IT_32BIT:
            {
               uint32 *data = (uint32*)indexData->indexBuffer->lock(indexData->indexStart * sizeof(uint32), 
                  indexData->indexCount * sizeof(uint32), HardwareBuffer::HBL_NORMAL);

               for (uint32 i = 0; i < indexData->indexCount; i++) {
                  data[i] = (uint32)indicesMap[data[i]];
               }

               indexData->indexBuffer->unlock();
            }
            break;

         default:
            throw new Ogre::Exception(0, "Unknown index buffer type", "Converter.cpp::CountVertices");
            break;
      }

      // Store new attributes
      subMesh->useSharedVertices = false;
      subMesh->vertexData = newVertexData;
   }

   // Release shared vertex data
   delete mesh->sharedVertexData;
   mesh->sharedVertexData = NULL;
}


//-----------------------------------------------------------------------------
///
String BatchedGeometry::getFormatString(SubEntity *ent)
{
   static char buf[1024];
   // add materialname and buffer type
   int countWritten =  sprintf(buf, "%s|%d", ent->getMaterialName().c_str(), ent->getSubMesh()->indexData->indexBuffer->getType());

   // now add vertex decl
   const VertexDeclaration::VertexElementList &elemList = ent->getSubMesh()->vertexData->vertexDeclaration->getElements();
   for (VertexDeclaration::VertexElementList::const_iterator i = elemList.begin(), iend = elemList.end(); i != iend; ++i)
   {
      const VertexElement &el = *i;
      countWritten += sprintf(buf + countWritten, "|%d|%d|%d", el.getSource(), el.getSemantic(), el.getType());
   }

   return buf;
}


//-----------------------------------------------------------------------------
///
void BatchedGeometry::build()
{
   ///Make sure the batch hasn't already been built
   if (m_Built)
      OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "Invalid call to build() - geometry is already batched (call clear() first)", "BatchedGeometry::GeomBatch::build()");

   if (!m_mapSubBatch.empty())
   {
      // Finish bounds information
      m_vecCenter = m_boundsAAB.getCenter();                            // Calculate bounds center
      m_boundsAAB.setMinimum(m_boundsAAB.getMinimum() - m_vecCenter);   // Center the bounding box
      m_boundsAAB.setMaximum(m_boundsAAB.getMaximum() - m_vecCenter);	// Center the bounding box
      m_fRadius = m_boundsAAB.getMaximum().length();                    // Calculate BB radius

      // Create scene node
      m_pSceneNode = m_pParentSceneNode->createChildSceneNode(m_vecCenter);

      //Build each batch
      for (TSubBatchMap::iterator i = m_mapSubBatch.begin(), iend = m_mapSubBatch.end(); i != iend; ++i)
         i->second->build();

      m_pSceneNode->attachObject(this);   // Attach the batch to the scene node

      //Debug
      //sceneNode->showBoundingBox(true);

      m_Built = true;
   }

}


//-----------------------------------------------------------------------------
///
void BatchedGeometry::clear()
{
   //Remove the batch from the scene
   if (m_pSceneNode)
   {
      m_pSceneNode->removeAllChildren();
      if (m_pSceneNode->getParent())
         m_pSceneNode->getParentSceneNode()->removeAndDestroyChild(m_pSceneNode->getName());
      else
         m_pSceneMgr->destroySceneNode(m_pSceneNode);

      m_pSceneNode = 0;
   }

   //Reset bounds information
   m_BoundsUndefined = true;
   m_vecCenter = Vector3::ZERO;
   m_fRadius = 0.f;

   //Delete each batch
   for (TSubBatchMap::iterator i = m_mapSubBatch.begin(), iend = m_mapSubBatch.end(); i != iend; ++i)
      delete i->second;
   m_mapSubBatch.clear();

   m_Built = false;
}


//-----------------------------------------------------------------------------
///
void BatchedGeometry::_updateRenderQueue(RenderQueue *queue)
{
   assert(isVisible() && "Ogre core code must detect that this MovableObject invisible");

   // SVA speed up adding
   Ogre::RenderQueueGroup *rqg = queue->getQueueGroup(getRenderQueueGroup());
   for (TSubBatchMap::const_iterator i = m_mapSubBatch.begin(), iend = m_mapSubBatch.end(); i != iend; ++i)
      i->second->addSelfToRenderQueue(rqg);

   ////If visible...
   //if (isVisible()){
   //   //Ask each batch to add itself to the render queue if appropriate
   //   for (SubBatchMap::iterator i = subBatchMap.begin(); i != subBatchMap.end(); ++i){
   //      i->second->addSelfToRenderQueue(queue, getRenderQueueGroup());
   //   }
   //}
}


//-----------------------------------------------------------------------------
///
bool BatchedGeometry::isVisible()
{
   return mVisible && m_bWithinFarDistance;
}


//-----------------------------------------------------------------------------
///
void BatchedGeometry::_notifyCurrentCamera(Camera *cam)
{
   if (getRenderingDistance() == Ogre::Real(0.))
      m_bWithinFarDistance = true;
   else
   {
      //Calculate camera distance
      Vector3 camVec = _convertToLocal(cam->getDerivedPosition()) - m_vecCenter;
      Real centerDistanceSquared = camVec.squaredLength();
      m_fMinDistanceSquared = std::max(Ogre::Real(0.), centerDistanceSquared - (m_fRadius * m_fRadius));
      //Note: centerDistanceSquared measures the distance between the camera and the center of the GeomBatch,
      //while minDistanceSquared measures the closest distance between the camera and the closest edge of the
      //geometry's bounding sphere.

      //Determine whether the BatchedGeometry is within the far rendering distance
      m_bWithinFarDistance = m_fMinDistanceSquared <= Math::Sqr(getRenderingDistance());
   }
}


//-----------------------------------------------------------------------------
///
Ogre::Vector3 BatchedGeometry::_convertToLocal(const Vector3 &globalVec) const
{
   //Convert from the given global position to the local coordinate system of the parent scene node.
   return (m_pParentSceneNode->getOrientation().Inverse() * globalVec);
}



//=============================================================================
// BatchedGeometry::SubBatch implementation
//=============================================================================


//-----------------------------------------------------------------------------
///
BatchedGeometry::SubBatch::SubBatch(BatchedGeometry *parent, SubEntity *ent) :
m_pBestTechnique        (NULL),
m_pVertexData           (0),
m_pIndexData            (0),
m_Built                 (false),
m_RequireVertexColors   (false),
m_pSubMesh              (0),
m_pParentGeom           (parent)
{
   assert(ent);
   m_pSubMesh = ent->getSubMesh();

   const Ogre::MaterialPtr &parentMaterial = ent->getMaterial();
   if (parentMaterial.isNull())
      OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "BatchedGeometry. Empty parent material", "BatchedGeometry::SubBatch::SubBatch");

   // SVA clone material
   // This function is used to make a single clone of materials used, since the materials
   // will be modified by the batch system (and it wouldn't be good to modify the original materials
   // that the user may be using somewhere else).
   {
      Ogre::String newName = parentMaterial->getName() + "_Batched";
      m_ptrMaterial = MaterialManager::getSingleton().getByName(newName, parentMaterial->getGroup());
      if (m_ptrMaterial.isNull())
         m_ptrMaterial = parentMaterial->clone(newName);
   }

   //Setup vertex/index data structure
   m_pVertexData = m_pSubMesh->vertexData->clone(false);
   m_pIndexData = m_pSubMesh->indexData->clone(false);

   //Remove blend weights from vertex format
   const VertexElement* blendIndices = m_pVertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_INDICES);
   const VertexElement* blendWeights = m_pVertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_WEIGHTS);
   if (blendIndices && blendWeights)
   {
      //Check for format errors
      assert(blendIndices->getSource() == blendWeights->getSource()
         && "Blend indices and weights should be in the same buffer");
      assert(blendIndices->getSize() + blendWeights->getSize() == m_pVertexData->vertexBufferBinding->getBuffer(blendIndices->getSource())->getVertexSize()
         && "Blend indices and blend buffers should have buffer to themselves!");

      //Remove the blend weights
      m_pVertexData->vertexBufferBinding->unsetBinding(blendIndices->getSource());
      m_pVertexData->vertexDeclaration->removeElement(VES_BLEND_INDICES);
      m_pVertexData->vertexDeclaration->removeElement(VES_BLEND_WEIGHTS);
      m_pVertexData->closeGapsInBindings();
   }

   //Reset vertex/index count
   m_pVertexData->vertexStart = 0;
   m_pVertexData->vertexCount = 0;
   m_pIndexData->indexStart   = 0;
   m_pIndexData->indexCount   = 0;
}


//-----------------------------------------------------------------------------
///
BatchedGeometry::SubBatch::~SubBatch()
{
   clear();

   OGRE_DELETE m_pVertexData;
   OGRE_DELETE m_pIndexData;
}


//-----------------------------------------------------------------------------
///
void BatchedGeometry::SubBatch::addSubEntity(SubEntity *ent, const Vector3 &position,
                                             const Quaternion &orientation, const Vector3 &scale,
                                             const Ogre::ColourValue &color, void* userData)
{
   assert(!m_Built);

   //Add this submesh to the queue
   QueuedMesh newMesh(ent->getSubMesh(), position, orientation, scale, color, userData);
   if (color != ColourValue::White)
   {
      m_RequireVertexColors = true;
      VertexElementType format = Root::getSingleton().getRenderSystem()->getColourVertexElementType();
      switch (format)
      {
      case VET_COLOUR_ARGB:
         std::swap(newMesh.color.r, newMesh.color.b);
         break;
      case VET_COLOUR_ABGR:
         break;
      default:
         OGRE_EXCEPT(0, "Unknown RenderSystem color format", "BatchedGeometry::SubBatch::addSubMesh()");
         break;
      }
   }

   m_queueMesh.push_back(newMesh);

   //Increment the vertex/index count so the buffers will have room for this mesh
   m_pVertexData->vertexCount += newMesh.subMesh->vertexData->vertexCount;
   m_pIndexData->indexCount   += newMesh.subMesh->indexData->indexCount;
}


//-----------------------------------------------------------------------------
///
void BatchedGeometry::SubBatch::build()
{
   assert(!m_Built);

   HardwareIndexBuffer::IndexType srcIndexType = m_pSubMesh->indexData->indexBuffer->getType();
   HardwareIndexBuffer::IndexType destIndexType =                             // type of index buffer
      m_pVertexData->vertexCount > 0xFFFF || srcIndexType == HardwareIndexBuffer::IT_32BIT ? 
      HardwareIndexBuffer::IT_32BIT : HardwareIndexBuffer::IT_16BIT;

   //Allocate the index buffer
   m_pIndexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
      destIndexType, m_pIndexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

   //Lock the index buffer
   uint32 *indexBuffer32;
   uint16 *indexBuffer16;
   if (destIndexType == HardwareIndexBuffer::IT_32BIT)
      indexBuffer32 = static_cast<uint32*>(m_pIndexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
   else
      indexBuffer16 = static_cast<uint16*>(m_pIndexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));

   //Allocate & lock the vertex buffers
   std::vector<uchar*> vertexBuffers;
   std::vector<VertexDeclaration::VertexElementList> vertexBufferElements;

   VertexBufferBinding *vertBinding = m_pVertexData->vertexBufferBinding;
   VertexDeclaration *vertDecl = m_pVertexData->vertexDeclaration;

   for (Ogre::ushort i = 0; i < vertBinding->getBufferCount(); ++i)
   {
      HardwareVertexBufferSharedPtr buffer = HardwareBufferManager::getSingleton().createVertexBuffer(
         vertDecl->getVertexSize(i), m_pVertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
      vertBinding->setBinding(i, buffer);

      vertexBuffers.push_back(static_cast<uchar*>(buffer->lock(HardwareBuffer::HBL_DISCARD)));
      vertexBufferElements.push_back(vertDecl->findElementsBySource(i));
   }

   //If no vertex colors are used, make sure the final batch includes them (so the shade values work)
   if (m_RequireVertexColors)
   {
      if (!m_pVertexData->vertexDeclaration->findElementBySemantic(VES_DIFFUSE))
      {
         Ogre::ushort i = (Ogre::ushort)vertBinding->getBufferCount();
         vertDecl->addElement(i, 0, VET_COLOUR, VES_DIFFUSE);

         HardwareVertexBufferSharedPtr buffer = HardwareBufferManager::getSingleton().createVertexBuffer(
            vertDecl->getVertexSize(i), m_pVertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
         vertBinding->setBinding(i, buffer);

         vertexBuffers.push_back(static_cast<uchar*>(buffer->lock(HardwareBuffer::HBL_DISCARD)));
         vertexBufferElements.push_back(vertDecl->findElementsBySource(i));
      }

      m_ptrMaterial->getTechnique(0)->getPass(0)->setVertexColourTracking(TVC_AMBIENT);
   }

   //For each queued mesh...
   size_t indexOffset = 0;
   for (size_t iMesh = 0, cntMeshes = m_queueMesh.size(); iMesh < cntMeshes; ++iMesh)
   {
      const QueuedMesh &queuedMesh = m_queueMesh[iMesh]; // <-- std::vector

      // If orientation identity we can skip many operation in vertex processing
      if (queuedMesh.orientation == Ogre::Quaternion::IDENTITY)
         _buildIdentiryOrientation(queuedMesh, m_pParentGeom->m_vecCenter, vertexBufferElements, vertexBuffers, m_pVertexData);
      else
         _buildFullTransform(queuedMesh, m_pParentGeom->m_vecCenter, vertexBufferElements, vertexBuffers, m_pVertexData);

      //Copy mesh index data into the index buffer
      Ogre::IndexData *sourceIndexData = queuedMesh.subMesh->indexData;

      if (srcIndexType == HardwareIndexBuffer::IT_32BIT)
      {
         //Lock the input buffer
         uint32 *source = static_cast<uint32*>(sourceIndexData->indexBuffer->lock(
            sourceIndexData->indexStart, sourceIndexData->indexCount, HardwareBuffer::HBL_READ_ONLY));
         uint32 *sourceEnd = source + sourceIndexData->indexCount;

         //And copy it to the output buffer
         while (source != sourceEnd)
            *indexBuffer32++ = static_cast<uint32>(*source++ + indexOffset);

         sourceIndexData->indexBuffer->unlock();                     // Unlock the input buffer
         indexOffset += queuedMesh.subMesh->vertexData->vertexCount; // Increment the index offset
      }
      else
      {
         if (destIndexType == HardwareIndexBuffer::IT_32BIT)
         {
            //-- Convert 16 bit to 32 bit indices --
            //Lock the input buffer
            uint16 *source = static_cast<uint16*>(sourceIndexData->indexBuffer->lock(
               sourceIndexData->indexStart, sourceIndexData->indexCount, HardwareBuffer::HBL_READ_ONLY));
            uint16 *sourceEnd = source + sourceIndexData->indexCount;

            //And copy it to the output buffer
            while (source != sourceEnd)
            {
               uint32 indx = *source++;
               *indexBuffer32++ = (indx + indexOffset);
            }

            sourceIndexData->indexBuffer->unlock();                  // Unlock the input buffer
            indexOffset += queuedMesh.subMesh->vertexData->vertexCount; // Increment the index offset
         }
         else
         {
            //Lock the input buffer
            uint16 *source = static_cast<uint16*>(sourceIndexData->indexBuffer->lock(
               sourceIndexData->indexStart, sourceIndexData->indexCount, HardwareBuffer::HBL_READ_ONLY));
            uint16 *sourceEnd = source + sourceIndexData->indexCount;

            //And copy it to the output buffer
            while (source != sourceEnd)
               *indexBuffer16++ = static_cast<uint16>(*source++ + indexOffset);

            sourceIndexData->indexBuffer->unlock();                  // Unlock the input buffer
            indexOffset += queuedMesh.subMesh->vertexData->vertexCount; // Increment the index offset
         }
      }

   }  // For each queued mesh

   //Unlock buffers
   m_pIndexData->indexBuffer->unlock();
   for (Ogre::ushort i = 0; i < vertBinding->getBufferCount(); ++i)
      vertBinding->getBuffer(i)->unlock();

   m_queueMesh.clear();   // Clear mesh queue
   m_Built = true;
}


//-----------------------------------------------------------------------------
///
void BatchedGeometry::SubBatch::_buildIdentiryOrientation(const QueuedMesh &queuedMesh,
                                                          const Ogre::Vector3 &parentGeomCenter,
                                                          const std::vector<VertexDeclaration::VertexElementList> &vertexBufferElements,
                                                          std::vector<Ogre::uchar*> &vertexBuffers,
                                                          Ogre::VertexData *dstVertexData)
{
   const VertexData *sourceVertexData = queuedMesh.subMesh->vertexData;
   Ogre::Vector3 v3AddBatchPosition = queuedMesh.position - parentGeomCenter;

   //Copy mesh vertex data into the vertex buffer
   VertexBufferBinding *sourceBinds = sourceVertexData->vertexBufferBinding;
   VertexBufferBinding *destBinds = dstVertexData->vertexBufferBinding;

   // For each vertex buffer
   for (unsigned short ibuffer = 0, bufCnt = destBinds->getBufferCount(); ibuffer < bufCnt; ++ibuffer)
   {
      if (ibuffer < sourceBinds->getBufferCount()) // destanation buffer index smaller than source buffers count
      {
         //Lock the input buffer
         const HardwareVertexBufferSharedPtr &sourceBuffer = sourceBinds->getBuffer(ibuffer);
         uchar *sourceBase = static_cast<uchar*>(sourceBuffer->lock(HardwareBuffer::HBL_READ_ONLY));
         uchar *destBase = vertexBuffers[ibuffer]; //Get the locked output buffer

         const VertexDeclaration::VertexElementList &elems = vertexBufferElements[ibuffer];
         VertexDeclaration::VertexElementList::const_iterator iBegin = elems.begin(), iEnd = elems.end();
         float *sourcePtr = 0, *destPtr = 0;
         size_t sourceVertexBuffer = sourceBuffer->getVertexSize();

         //Copy vertices
         for (size_t v = 0, vertexCounter = sourceVertexData->vertexCount; v < vertexCounter; ++v)
         {
            // Iterate over vertex elements
            VertexDeclaration::VertexElementList::const_iterator it = iBegin;
            while (it != iEnd)
            {
               const VertexElement &elem = *it;
               elem.baseVertexPointerToElement(sourceBase, &sourcePtr);
               elem.baseVertexPointerToElement(destBase, &destPtr);

               switch (elem.getSemantic())
               {
               case VES_POSITION:
                  {
                     Vector3 tmp(sourcePtr[0], sourcePtr[1], sourcePtr[2]);
                     tmp *= queuedMesh.scale;
                     tmp += v3AddBatchPosition;

                     destPtr[0] = (float)tmp.x; destPtr[1] = (float)tmp.y; destPtr[2] = (float)tmp.z;
                  }
                  break;

               case VES_NORMAL:
               case VES_BINORMAL:
               case VES_TANGENT:
                  {
                     // Raw copy
                     destPtr[0] = sourcePtr[0];
                     destPtr[1] = sourcePtr[1];
                     destPtr[2] = sourcePtr[2];
                  }
                  break;

               case VES_DIFFUSE:
                  {
                     uint32 tmpColor = *((uint32*)sourcePtr++);
                     uint8 tmpR = static_cast<uint8>(((tmpColor) & 0xFF) * queuedMesh.color.r);
                     uint8 tmpG = static_cast<uint8>(((tmpColor >> 8) & 0xFF)  * queuedMesh.color.g);
                     uint8 tmpB = static_cast<uint8>(((tmpColor >> 16) & 0xFF) * queuedMesh.color.b);
                     //uint8 tmpA = 0xFF; //static_cast<uint8>(((tmpColor >> 24) & 0xFF) * queuedMesh.color.a);  // always alpha 1.f
                     //*((uint32*)destPtr) = tmpR | (tmpG << 8) | (tmpB << 16) | (tmpA << 24);
                     *((uint32*)destPtr) = tmpR | (tmpG << 8) | (tmpB << 16) | (0xFF << 24);
                  }
                  break;

               default:
                  // Raw copy
                  //memcpy(destPtr, sourcePtr, VertexElement::getTypeSize(elem.getType()));
                  memcpy(destPtr, sourcePtr, s_vertexType2Size[elem.getType()]);
                  break;
               };

               ++it;
            }  // while Iterate over vertex elements

            // Increment both pointers
            destBase    += sourceVertexBuffer;
            sourceBase  += sourceVertexBuffer;
         }

         vertexBuffers[ibuffer] = destBase;
         sourceBuffer->unlock(); // unlock the input buffer
      }
      else
      {
         size_t bufferNumber = destBinds->getBufferCount()-1;

         //Get the locked output buffer
         uint32 *startPtr = (uint32*)vertexBuffers[bufferNumber];
         uint32 *endPtr = startPtr + sourceVertexData->vertexCount;

         //Generate color
         uint8 tmpR = static_cast<uint8>(queuedMesh.color.r * 255);
         uint8 tmpG = static_cast<uint8>(queuedMesh.color.g * 255);
         uint8 tmpB = static_cast<uint8>(queuedMesh.color.b * 255);
         uint32 tmpColor = tmpR | (tmpG << 8) | (tmpB << 16) | (0xFF << 24);

         //Copy colors
         while (startPtr < endPtr)
            *startPtr++ = tmpColor;

         vertexBuffers[bufferNumber] += sizeof(uint32) * sourceVertexData->vertexCount;
      }

   }  // For each vertex buffer

}

//-----------------------------------------------------------------------------
///
void BatchedGeometry::SubBatch::_buildFullTransform(const QueuedMesh &queuedMesh,
                                                    const Ogre::Vector3 &parentGeomCenter,
                                                    const std::vector<VertexDeclaration::VertexElementList> &vertexBufferElements,
                                                    std::vector<Ogre::uchar*> &vertexBuffers,
                                                    Ogre::VertexData *dstVertexData)
{
   const VertexData *sourceVertexData = queuedMesh.subMesh->vertexData;
   // Get rotation matrix for vertex rotation
   Ogre::Matrix3 m3MeshOrientation;
   queuedMesh.orientation.ToRotationMatrix(m3MeshOrientation);
   const Ogre::Real *mat = m3MeshOrientation[0];   // Ogre::Matrix3 is row major
   Ogre::Vector3 v3AddBatchPosition = queuedMesh.position - parentGeomCenter;
   const Ogre::Vector3 &scale = queuedMesh.scale;

   //Copy mesh vertex data into the vertex buffer
   VertexBufferBinding *sourceBinds = sourceVertexData->vertexBufferBinding;
   VertexBufferBinding *destBinds = dstVertexData->vertexBufferBinding;

   // For each vertex buffer
   for (unsigned short ibuffer = 0, bufCnt = destBinds->getBufferCount(); ibuffer < bufCnt; ++ibuffer)
   {
      if (ibuffer < sourceBinds->getBufferCount()) // destanation buffer index smaller than source buffers count
      {
         //Lock the input buffer
         const HardwareVertexBufferSharedPtr &sourceBuffer = sourceBinds->getBuffer(ibuffer);
         uchar *sourceBase = static_cast<uchar*>(sourceBuffer->lock(HardwareBuffer::HBL_READ_ONLY));

         //Get the locked output buffer
         uchar *destBase = vertexBuffers[ibuffer];


         const VertexDeclaration::VertexElementList &elems = vertexBufferElements[ibuffer];
         VertexDeclaration::VertexElementList::const_iterator iBegin = elems.begin(), iEnd = elems.end();
         float *sourcePtr = 0, *destPtr = 0;
         size_t sourceVertexSize = sourceBuffer->getVertexSize();

         //Copy vertices
         for (size_t v = 0, vertexCounter = sourceVertexData->vertexCount; v < vertexCounter; ++v)
         {
            VertexDeclaration::VertexElementList::const_iterator it = iBegin;
            // Iterate over vertex elements
            while (it != iEnd)
            {
               const VertexElement &elem = *it;
               elem.baseVertexPointerToElement(sourceBase, &sourcePtr);
               elem.baseVertexPointerToElement(destBase, &destPtr);

               switch (elem.getSemantic())
               {
               case VES_POSITION:
                  {
                     Vector3 tmp(sourcePtr[0] * scale.x, sourcePtr[1] * scale.x, sourcePtr[2] * scale.x);
                     // rotate vector by matrix. Ogre::Matrix3::operator* (const Vector3&) is not fast
                     tmp = Ogre::Vector3(
                        mat[0] * tmp.x + mat[1] * tmp.y + mat[2] * tmp.z,
                        mat[3] * tmp.x + mat[4] * tmp.y + mat[5] * tmp.z,
                        mat[6] * tmp.x + mat[7] * tmp.y + mat[8] * tmp.z);
                     tmp += v3AddBatchPosition;
                     destPtr[0] = (float)tmp.x;
                     destPtr[1] = (float)tmp.y;
                     destPtr[2] = (float)tmp.z;
                  }
                  break;

               case VES_NORMAL:
               case VES_BINORMAL:
               case VES_TANGENT:
                  {
                     // rotate vector by matrix. Ogre::Matrix3::operator* (const Vector3&) is not fast
                     destPtr[0] = float(mat[0] * sourcePtr[0] + mat[1] * sourcePtr[1] + mat[2] * sourcePtr[2]); // x
                     destPtr[1] = float(mat[3] * sourcePtr[0] + mat[4] * sourcePtr[1] + mat[5] * sourcePtr[2]); // y
                     destPtr[2] = float(mat[6] * sourcePtr[0] + mat[6] * sourcePtr[1] + mat[6] * sourcePtr[2]); // z
                  }
                  break;

               case VES_DIFFUSE:
                  {
                     uint32 tmpColor = *((uint32*)sourcePtr);
                     uint8 tmpR = static_cast<uint8>(((tmpColor) & 0xFF)       * queuedMesh.color.r);
                     uint8 tmpG = static_cast<uint8>(((tmpColor >> 8) & 0xFF)  * queuedMesh.color.g);
                     uint8 tmpB = static_cast<uint8>(((tmpColor >> 16) & 0xFF) * queuedMesh.color.b);
                     //uint8 tmpA = 0xFF; //static_cast<uint8>(((tmpColor >> 24) & 0xFF) * queuedMesh.color.a);  // always alpha 1.f

                     //*((uint32*)destPtr) = tmpR | (tmpG << 8) | (tmpB << 16) | (tmpA << 24);
                     *((uint32*)destPtr) = tmpR | (tmpG << 8) | (tmpB << 16) | (0xFF << 24);
                  }
                  break;

               default:
                  //memcpy(destPtr, sourcePtr, VertexElement::getTypeSize(elem.getType()));
                  memcpy(destPtr, sourcePtr, s_vertexType2Size[elem.getType()]);
                  break;
               };

               ++it;
            }  // for VertexElementList

            // Increment both pointers
            destBase    += sourceVertexSize;
            sourceBase  += sourceVertexSize;
         }

         //Unlock the input buffer
         vertexBuffers[ibuffer] = destBase;
         sourceBuffer->unlock();
      }
      else
      {
         size_t bufferNumber = destBinds->getBufferCount()-1;

         //Get the locked output buffer
         uint32 *startPtr = (uint32*)vertexBuffers[bufferNumber];
         uint32 *endPtr = startPtr + sourceVertexData->vertexCount;

         //Generate color
         uint8 tmpR = static_cast<uint8>(queuedMesh.color.r * 255);
         uint8 tmpG = static_cast<uint8>(queuedMesh.color.g * 255);
         uint8 tmpB = static_cast<uint8>(queuedMesh.color.b * 255);
         uint32 tmpColor = tmpR | (tmpG << 8) | (tmpB << 16) | (0xFF << 24);

         //Copy colors
         while (startPtr < endPtr)
            *startPtr++ = tmpColor;

         vertexBuffers[bufferNumber] += sizeof(uint32) * sourceVertexData->vertexCount;
      }
   }
}


//-----------------------------------------------------------------------------
///
void BatchedGeometry::SubBatch::clear()
{
   //If built, delete the batch
   if (m_Built)
   {
      m_Built = false;

      //Delete buffers
      m_pIndexData->indexBuffer.setNull();
      m_pVertexData->vertexBufferBinding->unsetAllBindings();

      //Reset vertex/index count
      m_pVertexData->vertexStart = 0;
      m_pVertexData->vertexCount = 0;
      m_pIndexData->indexStart = 0;
      m_pIndexData->indexCount = 0;
   }

   m_queueMesh.clear(); // Clear mesh queue
}


//-----------------------------------------------------------------------------
///
void BatchedGeometry::SubBatch::addSelfToRenderQueue(RenderQueueGroup *rqg)
{
   if (!m_Built)
      return;

   //Update material technique based on camera distance
   m_pBestTechnique = m_ptrMaterial->getBestTechnique(m_ptrMaterial->getLodIndex(
      m_pParentGeom->m_fMinDistanceSquared * m_pParentGeom->m_fMinDistanceSquared));
   rqg->addRenderable(this, m_pBestTechnique, OGRE_RENDERABLE_DEFAULT_PRIORITY);
}


//-----------------------------------------------------------------------------
///
void BatchedGeometry::SubBatch::getRenderOperation(RenderOperation& op)
{
   op.operationType = RenderOperation::OT_TRIANGLE_LIST;
   op.srcRenderable = this;
   op.useIndexes = true;
   op.vertexData = m_pVertexData;
   op.indexData = m_pIndexData;
}


//-----------------------------------------------------------------------------
///
Real BatchedGeometry::SubBatch::getSquaredViewDepth(const Camera* cam) const
{
   Vector3 camVec = m_pParentGeom->_convertToLocal(cam->getDerivedPosition()) - m_pParentGeom->m_vecCenter;
   return camVec.squaredLength();
}

//-----------------------------------------------------------------------------
///
const Ogre::LightList& BatchedGeometry::SubBatch::getLights(void) const
{
   return m_pParentGeom->queryLights();
}
