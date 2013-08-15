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

#include "WindBatchedGeometry.h"
#include "PagedGeometry.h"

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
using namespace Ogre;

namespace Forests {

//-------------------------------------------------------------------------------------

WindBatchedGeometry::WindBatchedGeometry(SceneManager *mgr, SceneNode *rootSceneNode):BatchedGeometry(mgr, rootSceneNode)
{
	mGeom = NULL;
}

void WindBatchedGeometry::addEntity(Entity *ent, const Vector3 &position, const Quaternion &orientation, const Vector3 &scale, const Ogre::ColourValue &color)
{
	MeshPtr mesh = ent->getMesh();
	if (mesh->sharedVertexData != NULL)
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Shared vertex data not allowed", "BatchedGeometry::addEntity()");

	//For each subentity
	for (uint32 i = 0; i < ent->getNumSubEntities(); ++i){
		//Get the subentity
		SubEntity *subEntity = ent->getSubEntity(i);
		SubMesh *subMesh = subEntity->getSubMesh();

		//Generate a format string that uniquely identifies this material & vertex/index format
		if (subMesh->vertexData == NULL)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "SubMesh vertex data not found!", "BatchedGeometry::addEntity()");

		String formatStr = getFormatString(subEntity);
		
		//If a batch using an identical format exists...
		WindSubBatch *batch;
		SubBatchMap::iterator batchIter = subBatchMap.find(formatStr);
		if (batchIter != subBatchMap.end()){
			//Use the batch
			batch = dynamic_cast<WindBatchedGeometry::WindSubBatch*>(batchIter->second);
		} else {
			//Otherwise create a new batch
			batch = new WindSubBatch(this, subEntity);
			subBatchMap.insert(std::pair<String, WindSubBatch*>(formatStr, batch));
		}

		//Now add the submesh to the compatible batch
		batch->addSubEntity(subEntity, position, orientation, scale, color, ent);
	}

	//Update bounding box
	Matrix4 mat(orientation);
	mat.setScale(scale);
	AxisAlignedBox entBounds = ent->getBoundingBox();
	entBounds.transform(mat);

	if (boundsUndefined){
		bounds.setMinimum(entBounds.getMinimum() + position);
		bounds.setMaximum(entBounds.getMaximum() + position);
		boundsUndefined = false;
	} else {
		Vector3 min = bounds.getMinimum();
		Vector3 max = bounds.getMaximum();
		min.makeFloor(entBounds.getMinimum() + position);
		max.makeCeil(entBounds.getMaximum() + position);
		bounds.setMinimum(min);
		bounds.setMaximum(max);
	}
	
}

WindBatchedGeometry::WindSubBatch::WindSubBatch(WindBatchedGeometry *parent, SubEntity *ent):BatchedGeometry::SubBatch(parent, ent)
{}

void WindBatchedGeometry::WindSubBatch::build()
{
	assert(!built);

	//Misc. setup
	Vector3 batchCenter = dynamic_cast<WindBatchedGeometry*>(parent)->center;

	HardwareIndexBuffer::IndexType srcIndexType = meshType->indexData->indexBuffer->getType();
	HardwareIndexBuffer::IndexType destIndexType;
	if (vertexData->vertexCount > 0xFFFF || srcIndexType == HardwareIndexBuffer::IT_32BIT)
		destIndexType = HardwareIndexBuffer::IT_32BIT;
	else
		destIndexType = HardwareIndexBuffer::IT_16BIT;

	//Allocate the index buffer
	indexData->indexBuffer = HardwareBufferManager::getSingleton()
		.createIndexBuffer(destIndexType, indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

	//Lock the index buffer
	uint32 *indexBuffer32;
	uint16 *indexBuffer16;
	if (destIndexType == HardwareIndexBuffer::IT_32BIT)
		indexBuffer32 = static_cast<uint32*>(indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
	else
		indexBuffer16 = static_cast<uint16*>(indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));

	//Allocate & lock the vertex buffers
	std::vector<uchar*> vertexBuffers;
	std::vector<VertexDeclaration::VertexElementList> vertexBufferElements;

	VertexBufferBinding *vertBinding = vertexData->vertexBufferBinding;
	VertexDeclaration *vertDecl = vertexData->vertexDeclaration;

	unsigned short texCoordCount = 0;
		for (unsigned short j = 0; j < vertexData->vertexDeclaration->getElementCount(); ++j) 
		{
			const VertexElement *el = vertexData->vertexDeclaration->getElement(j);
			if (el->getSemantic() == VES_TEXTURE_COORDINATES) 
			{
				++ texCoordCount;
			}
		}
		Ogre::ushort k = (Ogre::ushort)vertBinding->getBufferCount();

		vertDecl->addElement(k-1, vertDecl->getVertexSize(0), VET_FLOAT4 , VES_TEXTURE_COORDINATES, texCoordCount);
		vertDecl->addElement(k-1, vertDecl->getVertexSize(0), VET_FLOAT4 , VES_TEXTURE_COORDINATES, texCoordCount+1);

	for (Ogre::ushort i = 0; i < vertBinding->getBufferCount(); ++i)
	{
		HardwareVertexBufferSharedPtr buffer = HardwareBufferManager::getSingleton()
			.createVertexBuffer(vertDecl->getVertexSize(i), vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		vertBinding->setBinding(i, buffer);
		
		vertexBuffers.push_back(static_cast<uchar*>(buffer->lock(HardwareBuffer::HBL_DISCARD)));
		vertexBufferElements.push_back(vertDecl->findElementsBySource(i));
	}

	//If no vertex colors are used, make sure the final batch includes them (so the shade values work)
	if (requireVertexColors) {
		if (!vertexData->vertexDeclaration->findElementBySemantic(VES_DIFFUSE)) {
			Ogre::ushort i = (Ogre::ushort)vertBinding->getBufferCount();

			vertDecl->addElement(i, 0, VET_COLOUR, VES_DIFFUSE);

			HardwareVertexBufferSharedPtr buffer = HardwareBufferManager::getSingleton()
				.createVertexBuffer(vertDecl->getVertexSize(i), vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
			vertBinding->setBinding(i, buffer);
			
			vertexBuffers.push_back(static_cast<uchar*>(buffer->lock(HardwareBuffer::HBL_DISCARD)));
			vertexBufferElements.push_back(vertDecl->findElementsBySource(i));

		}

		Pass *p = material->getTechnique(0)->getPass(0);

		p->setVertexColourTracking(TVC_AMBIENT);
	}

	std::string entityName;
	Ogre::AxisAlignedBox entityBounds;

	//For each queued mesh...
	MeshQueueIterator it;
	size_t indexOffset = 0;
	for (it = meshQueue.begin(); it != meshQueue.end(); ++it) {
		const QueuedMesh queuedMesh =  (*it);
		//const QueuedMesh queuedMesh =  dynamic_cast<WindBatchedGeometry::WindSubBatch::QueuedMesh>((*it));
		const IndexData *sourceIndexData = queuedMesh.mesh->indexData;
		const VertexData *sourceVertexData = queuedMesh.mesh->vertexData;

		Entity * ent = static_cast<Ogre::Entity*>(queuedMesh.userData);
		entityName = ent->getName();
		entityBounds = ent->getBoundingBox();

		// vector to stock the original y value of every vertex because batchCenter doesn't take consider the height of the ground
		Vector3 vertexPos;
		float maxHeight = entityBounds.getMaximum().y;
		float factorX = dynamic_cast<WindBatchedGeometry*>(parent)->mGeom->getCustomParam(entityName, "windFactorX", 0);	// amplitude in X
		float factorY = dynamic_cast<WindBatchedGeometry*>(parent)->mGeom->getCustomParam(entityName, "windFactorY", 0);	// amplitude in Y

		//Copy mesh vertex data into the vertex buffer
		VertexBufferBinding *sourceBinds = sourceVertexData->vertexBufferBinding;
		VertexBufferBinding *destBinds = vertexData->vertexBufferBinding;

		for (Ogre::ushort i = 0; i < destBinds->getBufferCount(); ++i)
		{
			if (i < sourceBinds->getBufferCount()){
				//Lock the input buffer
				HardwareVertexBufferSharedPtr sourceBuffer = sourceBinds->getBuffer(i);
				uchar *sourceBase = static_cast<uchar*>(sourceBuffer->lock(HardwareBuffer::HBL_READ_ONLY));

				//Get the locked output buffer
				uchar *destBase = vertexBuffers[i];

				//Copy vertices
				float *sourcePtr, *destPtr;
				for (size_t v = 0; v < sourceVertexData->vertexCount; ++v)
				{
					// Iterate over vertex elements
					VertexDeclaration::VertexElementList &elems = vertexBufferElements[i];
					VertexDeclaration::VertexElementList::iterator ei;
					for (ei = elems.begin(); ei != elems.end(); ++ei)
					{
						VertexElement &elem = *ei;
						elem.baseVertexPointerToElement(sourceBase, &sourcePtr);
						elem.baseVertexPointerToElement(destBase, &destPtr);

						Vector3 tmp;
						uint32 tmpColor;
						uint8 tmpR, tmpG, tmpB, tmpA;

						switch (elem.getSemantic())
						{
						case VES_POSITION:
							tmp.x = *sourcePtr++;
							tmp.y = *sourcePtr++;
							tmp.z = *sourcePtr++;
							
							//Transform
							tmp = (queuedMesh.orientation * (tmp * queuedMesh.scale)) + queuedMesh.position;
							
							vertexPos = tmp - queuedMesh.position;

							tmp -= batchCenter;		//Adjust for batch center

							*destPtr++ = tmp.x;
							*destPtr++ = tmp.y;
							*destPtr++ = tmp.z;
							break;

						case VES_NORMAL:
							tmp.x = *sourcePtr++;
							tmp.y = *sourcePtr++;
							tmp.z = *sourcePtr++;

							//Rotate
							tmp = queuedMesh.orientation * tmp;

							*destPtr++ = tmp.x;
							*destPtr++ = tmp.y;
							*destPtr++ = tmp.z;
							break;

						case VES_DIFFUSE:
							tmpColor = *((uint32*)sourcePtr++);
							tmpR = ((tmpColor) & 0xFF) * queuedMesh.color.r;
							tmpG = ((tmpColor >> 8) & 0xFF) * queuedMesh.color.g;
							tmpB = ((tmpColor >> 16) & 0xFF) * queuedMesh.color.b;
							tmpA = (tmpColor >> 24) & 0xFF;

							tmpColor = tmpR | (tmpG << 8) | (tmpB << 16) | (tmpA << 24);
							*((uint32*)destPtr++) = tmpColor;
							break;

						case VES_TANGENT:
						case VES_BINORMAL:
							tmp.x = *sourcePtr++;
							tmp.y = *sourcePtr++;
							tmp.z = *sourcePtr++;

							//Rotate
							tmp = queuedMesh.orientation * tmp;

							*destPtr++ = tmp.x;
							*destPtr++ = tmp.y;
							*destPtr++ = tmp.z;
							break;
						
						case VES_TEXTURE_COORDINATES:
							if (elem.getIndex() == texCoordCount)
							{
								// parameters to be passed to the shader
								*destPtr++ = vertexPos.x;	// radius coefficient
								*destPtr++ = vertexPos.y / maxHeight;  // height coefficient
								*destPtr++ = factorX;
								*destPtr++ = factorY;
							}
							else
							{
								if (elem.getIndex() == texCoordCount + 1 )
								{
									// original position for each vertex
									*destPtr++ = queuedMesh.position.x;
									*destPtr++ = queuedMesh.position.y;
									*destPtr++ = queuedMesh.position.z;
									*destPtr++ = 0;
								}
								else
								{
									memcpy(destPtr, sourcePtr, VertexElement::getTypeSize(elem.getType()));
								}
							}
							break;	
						default:
							//Raw copy
							memcpy(destPtr, sourcePtr, VertexElement::getTypeSize(elem.getType()));
							break;
						};
					}

					// Increment both pointers
					destBase += vertDecl->getVertexSize(i);
					sourceBase += sourceBuffer->getVertexSize();
				}

				//Unlock the input buffer
				vertexBuffers[i] = destBase;
				sourceBuffer->unlock();
			} else {
				assert(requireVertexColors);

				//Get the locked output buffer
				uint32 *startPtr = (uint32*)vertexBuffers[vertBinding->getBufferCount()-1];
				uint32 *endPtr = startPtr + sourceVertexData->vertexCount;
				
				//Generate color
				uint8 tmpR = queuedMesh.color.r * 255;
				uint8 tmpG = queuedMesh.color.g * 255;
				uint8 tmpB = queuedMesh.color.b * 255;
				uint32 tmpColor = tmpR | (tmpG << 8) | (tmpB << 16) | (0xFF << 24);

				//Copy colors
				while (startPtr < endPtr) {
					*startPtr++ = tmpColor;
				}

				vertexBuffers[vertBinding->getBufferCount()-1] += (sizeof(uint32) * sourceVertexData->vertexCount);
			}
		}


		//Copy mesh index data into the index buffer
		if (srcIndexType == HardwareIndexBuffer::IT_32BIT) {
			//Lock the input buffer
			uint32 *source = static_cast<uint32*>(sourceIndexData->indexBuffer->lock(
				sourceIndexData->indexStart, sourceIndexData->indexCount, HardwareBuffer::HBL_READ_ONLY
				));
			uint32 *sourceEnd = source + sourceIndexData->indexCount;

			//And copy it to the output buffer
			while (source != sourceEnd) {
				*indexBuffer32++ = static_cast<uint32>(*source++ + indexOffset);
			}
			
			//Unlock the input buffer
			sourceIndexData->indexBuffer->unlock();

			//Increment the index offset
			indexOffset += sourceVertexData->vertexCount;
		} else {
			if (destIndexType == HardwareIndexBuffer::IT_32BIT){
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
			} else {
				//Lock the input buffer
				uint16 *source = static_cast<uint16*>(sourceIndexData->indexBuffer->lock(
					sourceIndexData->indexStart, sourceIndexData->indexCount, HardwareBuffer::HBL_READ_ONLY
					));
				uint16 *sourceEnd = source + sourceIndexData->indexCount;

				//And copy it to the output buffer
				while (source != sourceEnd) {
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
	indexData->indexBuffer->unlock();
	for (Ogre::ushort i = 0; i < vertBinding->getBufferCount(); ++i)
		vertBinding->getBuffer(i)->unlock();

	//Clear mesh queue
	meshQueue.clear();

	built = true;
}

}
