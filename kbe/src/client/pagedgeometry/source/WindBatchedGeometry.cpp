/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

//WindBatchedGeometry.h
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

#include "WindBatchedGeometry.h"
#include "PagedGeometry.h"

using namespace Ogre;
using namespace Forests;


//-----------------------------------------------------------------------------
///
WindBatchedGeometry::WindBatchedGeometry(SceneManager *mgr, SceneNode *rootSceneNode, const PagedGeometry *pagedGeom) : 
BatchedGeometry(mgr, rootSceneNode),
mGeom (pagedGeom)
{
   // empty
}


//-----------------------------------------------------------------------------
///
void WindBatchedGeometry::addEntity(Entity *ent, const Vector3 &position, const Quaternion &orientation, const Vector3 &scale, const ColourValue &color)
{
	MeshPtr mesh = ent->getMesh();
	if (mesh->sharedVertexData != NULL)
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Shared vertex data not allowed", "BatchedGeometry::addEntity()");

	//For each subentity
	for (unsigned int i = 0, cntSubEnt = ent->getNumSubEntities(); i < cntSubEnt; ++i)
   {
		//Get the subentity
		SubEntity *subEntity = ent->getSubEntity(i);
		SubMesh *subMesh = subEntity->getSubMesh();

		//Generate a format string that uniquely identifies this material & vertex/index format
		if (subMesh->vertexData == NULL)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "SubMesh vertex data not found!", "BatchedGeometry::addEntity()");

		String formatStr = getFormatString(subEntity);
		
		//If a batch using an identical format exists...
		WindSubBatch *batch;
		TSubBatchMap::iterator batchIter = m_mapSubBatch.find(formatStr);
		if (batchIter != m_mapSubBatch.end())
			batch = static_cast < WindBatchedGeometry::WindSubBatch* > (batchIter->second);  //Use the batch
      else
      {  //Otherwise create a new batch
			batch = new WindSubBatch(this, subEntity);
			m_mapSubBatch.insert(std::pair<String, SubBatch*>(formatStr, batch));
		}

		//Now add the submesh to the compatible batch
		batch->addSubEntity(subEntity, position, orientation, scale, color, ent);
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
/// Constructor
WindBatchedGeometry::WindSubBatch::WindSubBatch(WindBatchedGeometry *parent, SubEntity *ent) : 
BatchedGeometry::SubBatch(parent, ent)
{
   // empty
}


//-----------------------------------------------------------------------------
///
void WindBatchedGeometry::WindSubBatch::build()
{
	assert(!m_Built);

	//Misc. setup
	const Vector3 &batchCenter = static_cast < WindBatchedGeometry* > (m_pParentGeom)->m_vecCenter;

	HardwareIndexBuffer::IndexType srcIndexType = m_pSubMesh->indexData->indexBuffer->getType();
	HardwareIndexBuffer::IndexType destIndexType = 
      m_pVertexData->vertexCount > 0xFFFF || srcIndexType == HardwareIndexBuffer::IT_32BIT ?
      HardwareIndexBuffer::IT_32BIT : HardwareIndexBuffer::IT_16BIT;

	//Allocate the index buffer
	m_pIndexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
      destIndexType, m_pIndexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

	//Lock the index buffer
	uint32 *indexBuffer32 = 0;
	uint16 *indexBuffer16 = 0;
	if (destIndexType == HardwareIndexBuffer::IT_32BIT)
		indexBuffer32 = static_cast<uint32*>(m_pIndexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
	else
		indexBuffer16 = static_cast<uint16*>(m_pIndexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));

   VertexBufferBinding *vertBinding = m_pVertexData->vertexBufferBinding;
	VertexDeclaration *vertDecl = m_pVertexData->vertexDeclaration;

	unsigned short texCoordCount = 0;
   {
      const VertexDeclaration::VertexElementList &vlist = vertDecl->getElements();
      VertexDeclaration::VertexElementList::const_iterator it = vlist.begin(), iend = vlist.end();
      while (it != iend)
      {
         if ((*it).getSemantic() == VES_TEXTURE_COORDINATES)
            ++texCoordCount;
         ++it;
      }
   }

   Ogre::ushort numVertBuffs = (Ogre::ushort)vertBinding->getBufferCount();

   vertDecl->addElement(numVertBuffs - 1, vertDecl->getVertexSize(0), VET_FLOAT4 , VES_TEXTURE_COORDINATES, texCoordCount);
   vertDecl->addElement(numVertBuffs - 1, vertDecl->getVertexSize(0), VET_FLOAT4 , VES_TEXTURE_COORDINATES, texCoordCount+1);

   //Allocate & lock the vertex buffers
   std::vector<uchar*> vertexBuffers(numVertBuffs);
   std::vector<VertexDeclaration::VertexElementList> vertexBufferElements(numVertBuffs);

	for (Ogre::ushort i = 0; i < numVertBuffs; ++i)
	{
		HardwareVertexBufferSharedPtr buffer = HardwareBufferManager::getSingleton().createVertexBuffer(
         vertDecl->getVertexSize(i), m_pVertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

		vertBinding->setBinding(i, buffer);
		
		vertexBuffers[i] = static_cast<uchar*>(buffer->lock(HardwareBuffer::HBL_DISCARD));
		vertexBufferElements[i] = vertDecl->findElementsBySource(i);
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
   for (size_t iMesh = 0, meshCnt = m_queueMesh.size(); iMesh < meshCnt; ++iMesh)
   {
		const QueuedMesh &queuedMesh = m_queueMesh[iMesh];
      const Ogre::Vector3 &scale = queuedMesh.scale;

		const IndexData *sourceIndexData = queuedMesh.subMesh->indexData;
		const VertexData *sourceVertexData = queuedMesh.subMesh->vertexData;
		Entity * ent = static_cast<Ogre::Entity*>(queuedMesh.userData);

      static const Ogre::String c_windFactorX = "windFactorX", c_windFactorY = "windFactorY";
		float factorX = static_cast<WindBatchedGeometry*>(m_pParentGeom)->mGeom->getCustomParam(ent->getName(), c_windFactorX, 0.f);	// amplitude in X
		float factorY = static_cast<WindBatchedGeometry*>(m_pParentGeom)->mGeom->getCustomParam(ent->getName(), c_windFactorY, 0.f);	// amplitude in Y

      Ogre::Real invMaxHeight = Ogre::Real(1.) / ent->getBoundingBox().getMaximum().y;

		//Copy mesh vertex data into the vertex buffer
		VertexBufferBinding *sourceBinds = sourceVertexData->vertexBufferBinding;
		VertexBufferBinding *destBinds = m_pVertexData->vertexBufferBinding;

      // SVA speed up. Rotate 3d vector by matrix 3x3 instead of quaternion
      Ogre::Matrix3 m3MeshRotation;
      queuedMesh.orientation.ToRotationMatrix(m3MeshRotation);
      Ogre::Real *mat = m3MeshRotation[0]; // Ogre::Matrix is row major
      Ogre::Vector3 v3AddBatchCenter = queuedMesh.position - batchCenter;

		for (Ogre::ushort i = 0; i < destBinds->getBufferCount(); ++i)
		{
			if (i < sourceBinds->getBufferCount())
         {
				//Lock the input buffer
				const HardwareVertexBufferSharedPtr &sourceBuffer = sourceBinds->getBuffer(i);
				uchar *sourceBase = static_cast<uchar*>(sourceBuffer->lock(HardwareBuffer::HBL_READ_ONLY));

            size_t sourceVertexSize = sourceBuffer->getVertexSize();
            size_t destVertexSize   = vertDecl->getVertexSize(i);

				//Get the locked output buffer
				uchar *destBase = vertexBuffers[i];

            const VertexDeclaration::VertexElementList &elems = vertexBufferElements[i];
            VertexDeclaration::VertexElementList::const_iterator iBegin = elems.begin(), iEnd = elems.end();
            // vector to stock the original y value of every vertex because batchCenter doesn't take consider the height of the ground
            Ogre::Vector3 vertexPos;

            //Copy vertices
            float *sourcePtr = 0, *destPtr = 0;
				for (size_t v = 0, vertexCount = sourceVertexData->vertexCount; v < vertexCount; ++v)
				{
               VertexDeclaration::VertexElementList::const_iterator itElement = iBegin;
               // Iterate over vertex elements
					for ( ; itElement != iEnd; ++itElement)
					{
						const VertexElement &elem = *itElement;
						elem.baseVertexPointerToElement(sourceBase, &sourcePtr);
						elem.baseVertexPointerToElement(destBase, &destPtr);

						switch (elem.getSemantic())
						{
						case VES_POSITION:
                     {
                        Ogre::Vector3 tmp(sourcePtr[0] * scale.x, sourcePtr[1] * scale.y, sourcePtr[2] * scale.z);
                        // rotate vector by matrix. Ogre::Matrix3::operator* (const Vector3&) is not fast
                        vertexPos.x = mat[0] * tmp.x + mat[1] * tmp.y + mat[2] * tmp.z;
                        vertexPos.y = mat[3] * tmp.x + mat[4] * tmp.y + mat[5] * tmp.z;
                        vertexPos.z = mat[6] * tmp.x + mat[7] * tmp.y + mat[8] * tmp.z;
                        tmp = vertexPos + v3AddBatchCenter;
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
                        destPtr[0] = float(mat[0] * sourcePtr[0] + mat[1] * sourcePtr[1] + mat[2] * sourcePtr[2]);   // x
                        destPtr[1] = float(mat[3] * sourcePtr[0] + mat[4] * sourcePtr[1] + mat[5] * sourcePtr[2]);   // y
                        destPtr[2] = float(mat[6] * sourcePtr[0] + mat[6] * sourcePtr[1] + mat[6] * sourcePtr[2]);   // z
                     }
                     break;

						case VES_DIFFUSE:
                     {
                        Ogre::uint32 tmpColor = *(reinterpret_cast<uint32*>(sourcePtr));
                        Ogre::uint8 tmpR = static_cast<uint8>((tmpColor & 0xFF) * queuedMesh.color.r);
                        Ogre::uint8 tmpG = static_cast<uint8>(((tmpColor >> 8) & 0xFF)  * queuedMesh.color.g);
                        Ogre::uint8 tmpB = static_cast<uint8>(((tmpColor >> 16) & 0xFF) * queuedMesh.color.b);
                        Ogre::uint8 tmpA = static_cast<uint8>(((tmpColor >> 24) & 0xFF) * queuedMesh.color.a);

                        tmpColor = tmpR | (tmpG << 8) | (tmpB << 16) | (tmpA << 24);
                        *(reinterpret_cast<uint32*>(destPtr)) = tmpColor;
                     }
							break;

						case VES_TEXTURE_COORDINATES:
                     {
                        if (elem.getIndex() == texCoordCount)
                        {
                           // parameters to be passed to the shader
                           destPtr[0] = float(vertexPos.x);                // radius coefficient
                           destPtr[1] = float(vertexPos.y * invMaxHeight); // height coefficient
                           destPtr[2] = factorX;
                           destPtr[3] = factorY;
                        }
                        else if (elem.getIndex() == texCoordCount + 1)
                        {
                           // original position for each vertex
                           destPtr[0] = (float)queuedMesh.position.x;
                           destPtr[1] = (float)queuedMesh.position.y;
                           destPtr[2] = (float)queuedMesh.position.z;
                           destPtr[3] = 0.f;
                        }
                        else
                           //memcpy(destPtr, sourcePtr, VertexElement::getTypeSize(elem.getType()));
                           memcpy(destPtr, sourcePtr, s_vertexType2Size[elem.getType()]);
                     }                  
                     break;	

						default:
							//Raw copy
							//memcpy(destPtr, sourcePtr, VertexElement::getTypeSize(elem.getType()));
                     memcpy(destPtr, sourcePtr, s_vertexType2Size[elem.getType()]);
							break;
						};
					}

					// Increment both pointers
					destBase    += destVertexSize;
					sourceBase  += sourceVertexSize;
				}

				//Unlock the input buffer
				vertexBuffers[i] = destBase;
				sourceBuffer->unlock();
         }
         else
         {
				assert(m_RequireVertexColors);

				//Get the locked output buffer
				uint32 *startPtr = (uint32*)vertexBuffers[vertBinding->getBufferCount()-1];
				uint32 *endPtr = startPtr + sourceVertexData->vertexCount;
				
				//Generate color
				uint8 tmpR = static_cast<uint8>(queuedMesh.color.r * 255);
				uint8 tmpG = static_cast<uint8>(queuedMesh.color.g * 255);
				uint8 tmpB = static_cast<uint8>(queuedMesh.color.b * 255);
				uint32 tmpColor = tmpR | (tmpG << 8) | (tmpB << 16) | (0xFF << 24);

				//Copy colors
				while (startPtr < endPtr)
            {
					*startPtr++ = tmpColor;
				}

				vertexBuffers[vertBinding->getBufferCount()-1] += (sizeof(uint32) * sourceVertexData->vertexCount);
         }
		}


		//Copy mesh index data into the index buffer
		if (srcIndexType == HardwareIndexBuffer::IT_32BIT)
      {
			//Lock the input buffer
			uint32 *source = static_cast<uint32*>(sourceIndexData->indexBuffer->lock(
				sourceIndexData->indexStart, sourceIndexData->indexCount, HardwareBuffer::HBL_READ_ONLY));
			uint32 *sourceEnd = source + sourceIndexData->indexCount;

			//And copy it to the output buffer
			while (source != sourceEnd) {
				*indexBuffer32++ = static_cast<uint32>(*source++ + indexOffset);
			}
			
			//Unlock the input buffer
			sourceIndexData->indexBuffer->unlock();

			//Increment the index offset
			indexOffset += sourceVertexData->vertexCount;
		}
      else
      {
			if (destIndexType == HardwareIndexBuffer::IT_32BIT)
         {
				//-- Convert 16 bit to 32 bit indices --
				//Lock the input buffer
				uint16 *source = static_cast<uint16*>(sourceIndexData->indexBuffer->lock(
					sourceIndexData->indexStart, sourceIndexData->indexCount, HardwareBuffer::HBL_READ_ONLY
					));
				uint16 *sourceEnd = source + sourceIndexData->indexCount;

				//And copy it to the output buffer
				while (source != sourceEnd) {
					uint32 indx = *source++;
					*indexBuffer32++ = (indx + indexOffset);
				}

				//Unlock the input buffer
				sourceIndexData->indexBuffer->unlock();

				//Increment the index offset
				indexOffset += sourceVertexData->vertexCount;
			}
         else
         {
				//Lock the input buffer
				uint16 *source = static_cast<uint16*>(sourceIndexData->indexBuffer->lock(
					sourceIndexData->indexStart, sourceIndexData->indexCount, HardwareBuffer::HBL_READ_ONLY
					));
				uint16 *sourceEnd = source + sourceIndexData->indexCount;

				//And copy it to the output buffer
				while (source != sourceEnd)
            {
					*indexBuffer16++ = static_cast<uint16>(*source++ + indexOffset);
				}

				//Unlock the input buffer
				sourceIndexData->indexBuffer->unlock();

				//Increment the index offset
				indexOffset += sourceVertexData->vertexCount;
			}
		}
	}

	//Unlock buffers
	m_pIndexData->indexBuffer->unlock();
	for (Ogre::ushort i = 0; i < vertBinding->getBufferCount(); ++i)
		vertBinding->getBuffer(i)->unlock();

	m_queueMesh.clear();
	m_Built = true;
}
