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

#include "BatchedGeometry.h"
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
#include <string>
using namespace Ogre;

namespace Forests {

//-------------------------------------------------------------------------------------

BatchedGeometry::BatchedGeometry(SceneManager *mgr, SceneNode *rootSceneNode)
 :	withinFarDistance(0),
	minDistanceSquared(0),
	sceneNode(NULL),
	sceneMgr(mgr),
	built(false),
	boundsUndefined(true),
	parentSceneNode(rootSceneNode)
{
	clear();
}

BatchedGeometry::~BatchedGeometry()
{
	clear();
}

void BatchedGeometry::addEntity(Entity *ent, const Vector3 &position, const Quaternion &orientation, const Vector3 &scale, const Ogre::ColourValue &color)
{
    setCastShadows(ent->getCastShadows());
    setVisibilityFlags(ent->getVisibilityFlags());

	MeshPtr mesh = ent->getMesh();

	//If shared vertex data is used, extract into non-shared data
	extractVertexDataFromShared(mesh);	

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
		SubBatch *batch;
		SubBatchMap::iterator batchIter = subBatchMap.find(formatStr);
		if (batchIter != subBatchMap.end()){
			//Use the batch
			batch = batchIter->second;
		} else {
			//Otherwise create a new batch
			batch = new SubBatch(this, subEntity);
			subBatchMap.insert(std::pair<String, SubBatch*>(formatStr, batch));
		}

		//Now add the submesh to the compatible batch
		batch->addSubEntity(subEntity, position, orientation, scale, color);
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

void BatchedGeometry::extractVertexDataFromShared(MeshPtr mesh)
{
	if (mesh->sharedVertexData == NULL)
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


BatchedGeometry::SubBatchIterator BatchedGeometry::getSubBatchIterator() const
{
	return BatchedGeometry::SubBatchIterator((SubBatchMap&)subBatchMap);
}

String BatchedGeometry::getFormatString(SubEntity *ent)
{
	StringUtil::StrStreamType str;

	str << ent->getMaterialName() << "|";
	str << ent->getSubMesh()->indexData->indexBuffer->getType() << "|";

	const VertexDeclaration::VertexElementList &elemList = ent->getSubMesh()->vertexData->vertexDeclaration->getElements();
	VertexDeclaration::VertexElementList::const_iterator i;
	for (i = elemList.begin(); i != elemList.end(); ++i)
	{
		const VertexElement &element = *i;
		str << element.getSource() << "|";
		str << element.getSemantic() << "|";
		str << element.getType() << "|";
	}

	return str.str();
}

void BatchedGeometry::build()
{
	///Make sure the batch hasn't already been built
	if (built)
		OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "Invalid call to build() - geometry is already batched (call clear() first)", "BatchedGeometry::GeomBatch::build()");

	if (subBatchMap.size() != 0) {
		//Finish bounds information
		center = bounds.getCenter();			//Calculate bounds center
		bounds.setMinimum(bounds.getMinimum() - center);	//Center the bounding box
		bounds.setMaximum(bounds.getMaximum() - center);	//Center the bounding box
		radius = bounds.getMaximum().length();	//Calculate BB radius
		
		//Create scene node
		sceneNode = parentSceneNode->createChildSceneNode(center);

		//Build each batch
		for (SubBatchMap::iterator i = subBatchMap.begin(); i != subBatchMap.end(); ++i){
			i->second->build();
		}

		//Attach the batch to the scene node
		sceneNode->attachObject(this);

		//Debug
		//sceneNode->showBoundingBox(true);

		built = true;
	}
	
}

void BatchedGeometry::clear()
{
	//Remove the batch from the scene
	if (sceneNode){
		sceneNode->removeAllChildren();
		sceneMgr->destroySceneNode(sceneNode->getName());
		sceneNode = NULL;
	}

	//Reset bounds information
	boundsUndefined = true;
	center = Vector3::ZERO;
	radius = 0;

	//Delete each batch
	for (SubBatchMap::iterator i = subBatchMap.begin(); i != subBatchMap.end(); ++i){
		delete i->second;
	}
	subBatchMap.clear();

	built = false;
}

void BatchedGeometry::_updateRenderQueue(RenderQueue *queue)
{
	//If visible...
	if (isVisible()){
		//Ask each batch to add itself to the render queue if appropriate
		for (SubBatchMap::iterator i = subBatchMap.begin(); i != subBatchMap.end(); ++i){
			i->second->addSelfToRenderQueue(queue, getRenderQueueGroup());
		}
	}
}

bool BatchedGeometry::isVisible()
{
	return mVisible && withinFarDistance;
}

void BatchedGeometry::_notifyCurrentCamera(Camera *cam)
{
	if (getRenderingDistance() == 0) {
		withinFarDistance = true;
	} else {
		//Calculate camera distance
		Vector3 camVec = _convertToLocal(cam->getDerivedPosition()) - center;
		Real centerDistanceSquared = camVec.squaredLength();
		minDistanceSquared = std::max(0.0f, centerDistanceSquared - (radius * radius));
		//Note: centerDistanceSquared measures the distance between the camera and the center of the GeomBatch,
		//while minDistanceSquared measures the closest distance between the camera and the closest edge of the
		//geometry's bounding sphere.

		//Determine whether the BatchedGeometry is within the far rendering distance
		withinFarDistance = minDistanceSquared <= Math::Sqr(getRenderingDistance());
	}
}

Ogre::Vector3 BatchedGeometry::_convertToLocal(const Vector3 &globalVec) const
{
	assert(parentSceneNode);
	//Convert from the given global position to the local coordinate system of the parent scene node.
	return (parentSceneNode->getOrientation().Inverse() * globalVec);
}




BatchedGeometry::SubBatch::SubBatch(BatchedGeometry *parent, SubEntity *ent)
{
	meshType = ent->getSubMesh();
	this->parent = parent;
	built = false;
	requireVertexColors = false;

	// Material must always exist
	Material *origMat = ((MaterialPtr)MaterialManager::getSingleton().getByName(ent->getMaterialName())).getPointer();
	if (origMat) {
		material = MaterialManager::getSingleton().getByName(getMaterialClone(*origMat)->getName());
	} else {
		MaterialManager::ResourceCreateOrRetrieveResult result = MaterialManager::getSingleton().createOrRetrieve("PagedGeometry_Batched_Material", "General");
		if (result.first.isNull()) {
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "BatchedGeometry failed to create a material for entity with invalid material.", "BatchedGeometry::SubBatch::SubBatch(BatchedGeometry *parent, SubEntity *ent)");
		}
		material = result.first;
	}

	//Setup vertex/index data structure
	vertexData = meshType->vertexData->clone(false);
	indexData = meshType->indexData->clone(false);

	//Remove blend weights from vertex format
	const VertexElement* blendIndices = vertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_INDICES);
	const VertexElement* blendWeights = vertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_WEIGHTS);
	if (blendIndices && blendWeights)
	{
		//Check for format errors
		assert(blendIndices->getSource() == blendWeights->getSource()
			&& "Blend indices and weights should be in the same buffer");
		assert(blendIndices->getSize() + blendWeights->getSize() == vertexData->vertexBufferBinding->getBuffer(blendIndices->getSource())->getVertexSize()
			&& "Blend indices and blend buffers should have buffer to themselves!");

		//Remove the blend weights
		vertexData->vertexBufferBinding->unsetBinding(blendIndices->getSource());
		vertexData->vertexDeclaration->removeElement(VES_BLEND_INDICES);
		vertexData->vertexDeclaration->removeElement(VES_BLEND_WEIGHTS);
		#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR > 2
		vertexData->closeGapsInBindings();
		#endif
	}

	//Reset vertex/index count
	vertexData->vertexStart = 0;
	vertexData->vertexCount = 0;
	indexData->indexStart = 0;
	indexData->indexCount = 0;
}

BatchedGeometry::SubBatch::~SubBatch()
{
	clear();

	delete vertexData;
	delete indexData;
}

Material *BatchedGeometry::SubBatch::getMaterialClone(Material &mat)
{
	String clonedName = mat.getName() + "_Batched";
	MaterialPtr clonedMat = MaterialManager::getSingleton().getByName(clonedName);
	if (clonedMat.isNull())
		clonedMat = mat.clone(clonedName);
	
	return clonedMat.getPointer();
}

void BatchedGeometry::SubBatch::addSubEntity(SubEntity *ent, const Vector3 &position, const Quaternion &orientation, const Vector3 &scale, const Ogre::ColourValue &color, void* userData)
{
	assert(!built);

	//Add this submesh to the queue
	QueuedMesh newMesh;
	newMesh.mesh = ent->getSubMesh();
	newMesh.position = position;
	newMesh.orientation = orientation;
	newMesh.scale = scale;
	newMesh.userData = userData;

	newMesh.color = color;
	if (newMesh.color != ColourValue::White) {
		requireVertexColors = true;
		VertexElementType format = Root::getSingleton().getRenderSystem()->getColourVertexElementType();
		switch (format){
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

	meshQueue.push_back(newMesh);

	//Increment the vertex/index count so the buffers will have room for this mesh
	vertexData->vertexCount += ent->getSubMesh()->vertexData->vertexCount;
	indexData->indexCount += ent->getSubMesh()->indexData->indexCount;
}

void BatchedGeometry::SubBatch::build()
{
	assert(!built);

	//Misc. setup
	Vector3 batchCenter = parent->center;

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

	//For each queued mesh...
	MeshQueueIterator it;
	size_t indexOffset = 0;
	for (it = meshQueue.begin(); it != meshQueue.end(); ++it) {
		const QueuedMesh queuedMesh = (*it);
		const IndexData *sourceIndexData = queuedMesh.mesh->indexData;
		const VertexData *sourceVertexData = queuedMesh.mesh->vertexData;

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

						default:
							//Raw copy
							memcpy(destPtr, sourcePtr, VertexElement::getTypeSize(elem.getType()));
							break;
						};
					}

					// Increment both pointers
					destBase += sourceBuffer->getVertexSize();
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

void BatchedGeometry::SubBatch::clear()
{
	//If built, delete the batch
	if (built){
		//Delete buffers
		indexData->indexBuffer.setNull();
		vertexData->vertexBufferBinding->unsetAllBindings();

		//Reset vertex/index count
		vertexData->vertexStart = 0;
		vertexData->vertexCount = 0;
		indexData->indexStart = 0;
		indexData->indexCount = 0;
	}

	//Clear mesh queue
	meshQueue.clear();

	built = false;
}

void BatchedGeometry::SubBatch::addSelfToRenderQueue(RenderQueue *queue, uint8 group)
{
	if (built){
		//Update material technique based on camera distance
		assert(!material.isNull());
		bestTechnique = material->getBestTechnique(material->getLodIndex(parent->minDistanceSquared * parent->minDistanceSquared));
			
		//Add to render queue
		queue->addRenderable(this, group);
	}
}

void BatchedGeometry::SubBatch::getRenderOperation(RenderOperation& op)
{
	op.operationType = RenderOperation::OT_TRIANGLE_LIST;
	op.srcRenderable = this;
	op.useIndexes = true;
	op.vertexData = vertexData;
	op.indexData = indexData;
}

Real BatchedGeometry::SubBatch::getSquaredViewDepth(const Camera* cam) const
{
	Vector3 camVec = parent->_convertToLocal(cam->getDerivedPosition()) - parent->center;
	return camVec.squaredLength();
}

#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR <= 2
//Dagon-compatible getLights()
const Ogre::LightList& BatchedGeometry::SubBatch::getLights(void) const
{
	return parent->sceneNode->findLights(parent->radius);
}
#else
//Eihort-compatible getLights()
const Ogre::LightList& BatchedGeometry::SubBatch::getLights(void) const
{
	return parent->queryLights();
}
}

#endif
