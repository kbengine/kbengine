/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
    1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

//StaticBillboardSet.h
//Provides a method of displaying billboards faster than Ogre's built-in BillboardSet
//functions by taking advantage of the static nature of billboards (note: StaticBillboardSet
//does not allow billboards to be moved or deleted individually in real-time)
//-------------------------------------------------------------------------------------

#include "StaticBillboardSet.h"

#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreVector3.h>
#include <OgreMeshManager.h>
#include <OgreMesh.h>
#include <OgreSubMesh.h>
#include <OgreMaterialManager.h>
#include <OgreMaterial.h>
#include <OgreBillboardSet.h>
#include <OgreBillboard.h>
#include <OgreSceneNode.h>
#include <OgreString.h>
#include <OgreStringConverter.h>
#include <OgreRenderSystem.h>
#include <OgreRenderSystemCapabilities.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreHardwareBufferManager.h>
#include <OgreHardwareBuffer.h>
#include <OgreLogManager.h>
#include <OgreEntity.h>
using namespace Ogre;

namespace Forests {

//-------------------------------------------------------------------------------------

unsigned long StaticBillboardSet::GUID = 0;
uint32 StaticBillboardSet::selfInstances = 0;
StaticBillboardSet::FadedMaterialMap StaticBillboardSet::fadedMaterialMap;

StaticBillboardSet::StaticBillboardSet(SceneManager *mgr, SceneNode *rootSceneNode, BillboardMethod method)
{
	sceneMgr = mgr;
	renderMethod = method;
	visible = true;
	fadeEnabled = false;
	bbOrigin = BBO_CENTER;
	subMesh = NULL;
	
	//Fall back to BB_METHOD_COMPATIBLE if vertex shaders are not available
	if (renderMethod == BB_METHOD_ACCELERATED){
		const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();
		if (!caps->hasCapability(RSC_VERTEX_PROGRAM))
			renderMethod = BB_METHOD_COMPATIBLE;
	}
	
	node = rootSceneNode->createChildSceneNode();
	entityName = getUniqueID("SBSentity");

	if (renderMethod == BB_METHOD_ACCELERATED){
		//Accelerated billboard method
		entity = NULL;

		uFactor = 1.0f;
		vFactor = 1.0f;
		
		//Load vertex shader to align billboards to face the camera (if not loaded already)
		if (++selfInstances == 1){

			String shaderLanguage;
			if (Root::getSingleton().getRenderSystem()->getName() == "Direct3D9 Rendering Subsystem")
				shaderLanguage = "hlsl";
			else if(Root::getSingleton().getRenderSystem()->getName() == "OpenGL Rendering Subsystem")
				shaderLanguage = "glsl";
			else
				shaderLanguage = "cg";

			//First shader, simple camera-alignment
			HighLevelGpuProgramPtr vertexShader;
			vertexShader = HighLevelGpuProgramManager::getSingleton().getByName("Sprite_vp");
			if (vertexShader.isNull()){
				String vertexProg;
				if(!shaderLanguage.compare("hlsl") || !shaderLanguage.compare("cg"))
				{
					vertexProg =
					"void Sprite_vp(	\n"
					"	float4 position : POSITION,	\n"
					"	float3 normal   : NORMAL,	\n"
					"	float4 color	: COLOR,	\n"
					"	float2 uv       : TEXCOORD0,	\n"
					"	out float4 oPosition : POSITION,	\n"
					"	out float2 oUv       : TEXCOORD0,	\n"
					"	out float4 oColor    : COLOR, \n"
					"	out float oFog       : FOG,	\n"
					"	uniform float4x4 worldViewProj,	\n"
					"	uniform float    uScroll, \n"
					"	uniform float    vScroll, \n"
					"	uniform float4   preRotatedQuad[4] )	\n"
					"{	\n"
					//Face the camera
					"	float4 vCenter = float4( position.x, position.y, position.z, 1.0f );	\n"
					"	float4 vScale = float4( normal.x, normal.y, normal.x, 1.0f );	\n"
					"	oPosition = mul( worldViewProj, vCenter + (preRotatedQuad[normal.z] * vScale) );  \n"
					
					//Color
					"	oColor = color;   \n"
					
					//UV Scroll
					"	oUv = uv;	\n"
					"	oUv.x += uScroll; \n"
					"	oUv.y += vScroll; \n"
					
					//Fog
					"	oFog = oPosition.z; \n"
					"}";
				}
				else
				{
					// Must be GLSL
					vertexProg =
					"uniform float uScroll; \n"
					"uniform float vScroll; \n"
					"uniform vec4  preRotatedQuad[4]; \n"

					"void main() { \n"
					//Face the camera
					"	vec4 vCenter = vec4( gl_Vertex.x, gl_Vertex.y, gl_Vertex.z, 1.0 ); \n"
					"	vec4 vScale = vec4( gl_Normal.x, gl_Normal.y, gl_Normal.x , 1.0 ); \n"
					"	gl_Position = gl_ModelViewProjectionMatrix * (vCenter + (preRotatedQuad[int(gl_Normal.z)] * vScale) ); \n"

					//Color
					"	gl_FrontColor = gl_Color; \n"

					//UV Scroll
					"	gl_TexCoord[0] = gl_MultiTexCoord0; \n"
					"	gl_TexCoord[0].x += uScroll; \n"
					"	gl_TexCoord[0].y += vScroll; \n"

					//Fog
					"	gl_FogFragCoord = gl_Position.z; \n"
					"}";
				}

				vertexShader = HighLevelGpuProgramManager::getSingleton()
					.createProgram("Sprite_vp",
					ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
					shaderLanguage, GPT_VERTEX_PROGRAM);
				vertexShader->setSource(vertexProg);

				if (shaderLanguage == "hlsl")
				{
					vertexShader->setParameter("target", "vs_1_1");
					vertexShader->setParameter("entry_point", "Sprite_vp");
				}
				else if(shaderLanguage == "cg")
				{
					vertexShader->setParameter("profiles", "vs_1_1 arbvp1");
					vertexShader->setParameter("entry_point", "Sprite_vp");
				}
				// GLSL can only have one entry point "main".

				vertexShader->load();
			}

			//Second shader, camera alignment and distance based fading
			HighLevelGpuProgramPtr vertexShader2;
			vertexShader2 = HighLevelGpuProgramManager::getSingleton().getByName("SpriteFade_vp");
			if (vertexShader2.isNull()){
				String vertexProg2;
				if(!shaderLanguage.compare("hlsl") || !shaderLanguage.compare("cg"))
				{
					vertexProg2 =
					"void SpriteFade_vp(	\n"
					"	float4 position : POSITION,	\n"
					"	float3 normal   : NORMAL,	\n"
					"	float4 color	: COLOR,	\n"
					"	float2 uv       : TEXCOORD0,	\n"
					"	out float4 oPosition : POSITION,	\n"
					"	out float2 oUv       : TEXCOORD0,	\n"
					"	out float4 oColor    : COLOR, \n"
					"	out float oFog       : FOG,	\n"
					"	uniform float4x4 worldViewProj,	\n"

					"	uniform float3 camPos, \n"
					"	uniform float fadeGap, \n"
					"   uniform float invisibleDist, \n"

					"	uniform float    uScroll, \n"
					"	uniform float    vScroll, \n"
					"	uniform float4   preRotatedQuad[4] )	\n"
					"{	\n"
					//Face the camera
					"	float4 vCenter = float4( position.x, position.y, position.z, 1.0f );	\n"
					"	float4 vScale = float4( normal.x, normal.y, normal.x, 1.0f );	\n"
					"	oPosition = mul( worldViewProj, vCenter + (preRotatedQuad[normal.z] * vScale) );  \n"

					"	oColor.rgb = color.rgb;   \n"

					//Fade out in the distance
					"	float dist = distance(camPos.xz, position.xz);	\n"
					"	oColor.a = (invisibleDist - dist) / fadeGap;   \n"

					//UV scroll
					"	oUv = uv;	\n"
					"	oUv.x += uScroll; \n"
					"	oUv.y += vScroll; \n"

					//Fog
					"	oFog = oPosition.z; \n"
					"}";
				}
				else
				{
					// Must be GLSL
					vertexProg2 =
					"uniform vec3  camPos; \n"
					"uniform float fadeGap; \n"
					"uniform float invisibleDist; \n"
					"uniform float uScroll; \n"
					"uniform float vScroll; \n"
					"uniform vec4  preRotatedQuad[4]; \n"

					"void main() { \n"
					//Face the camera
					"	vec4 vCenter = vec4( gl_Vertex.x, gl_Vertex.y, gl_Vertex.z, 1.0 ); \n"
					"	vec4 vScale = vec4( gl_Normal.x, gl_Normal.y, gl_Normal.x , 1.0 ); \n"
					"	gl_Position = gl_ModelViewProjectionMatrix * (vCenter + (preRotatedQuad[int(gl_Normal.z)] * vScale) ); \n"

					"	gl_FrontColor.xyz = gl_Color.xyz; \n"

					//Fade out in the distance
					"	vec4 position = gl_Vertex; \n"
					"	float dist = distance(camPos.xz, position.xz); \n"
					"	gl_FrontColor.w = (invisibleDist - dist) / fadeGap; \n"

					//UV scroll
					"	gl_TexCoord[0] = gl_MultiTexCoord0; \n"
					"	gl_TexCoord[0].x += uScroll; \n"
					"	gl_TexCoord[0].y += vScroll; \n"

					//Fog
					"	gl_FogFragCoord = gl_Position.z; \n"
					"}";
				}
				vertexShader2 = HighLevelGpuProgramManager::getSingleton()
					.createProgram("SpriteFade_vp",
					ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
					shaderLanguage, GPT_VERTEX_PROGRAM);
				vertexShader2->setSource(vertexProg2);

				if (shaderLanguage == "hlsl")
				{
					vertexShader2->setParameter("target", "vs_1_1");
					vertexShader2->setParameter("entry_point", "SpriteFade_vp");
				}
				else if(shaderLanguage == "cg")
				{
					vertexShader2->setParameter("profiles", "vs_1_1 arbvp1");
					vertexShader2->setParameter("entry_point", "SpriteFade_vp");
				}
				// GLSL can only have one entry point "main".

				vertexShader2->load();
			}

		}
	} else {
		//Compatible billboard method
		fallbackSet = sceneMgr->createBillboardSet(getUniqueID("SBS"), 100);
		node->attachObject(fallbackSet);
		uFactor = 0;
		vFactor = 0;
	}
}

StaticBillboardSet::~StaticBillboardSet()
{
	if (renderMethod == BB_METHOD_ACCELERATED){
		//Delete mesh data
		clear();

		//Delete scene node
		sceneMgr->destroySceneNode(node->getName());

		//Update material reference list
		if (!materialPtr.isNull()) SBMaterialRef::removeMaterialRef(materialPtr);
		if (!fadeMaterialPtr.isNull()) SBMaterialRef::removeMaterialRef(fadeMaterialPtr);

		//Delete vertex shaders and materials if no longer in use
		if (--selfInstances == 0){
			//Delete fade materials
			fadedMaterialMap.clear();
		}
	} else {
		//Delete scene node
		sceneMgr->destroySceneNode(node->getName());

		//Remove billboard set
		sceneMgr->destroyBillboardSet(fallbackSet);
	}
}

void StaticBillboardSet::clear()
{
	if (renderMethod == BB_METHOD_ACCELERATED){
		//Delete the entity and mesh data
		if (entity){
			//Delete entity
			node->detachAllObjects();
			sceneMgr->destroyEntity(entity);
			entity = 0;
			
			//Delete mesh
			assert(!mesh.isNull());
			String meshName(mesh->getName());
			mesh.setNull();
			if (MeshManager::getSingletonPtr())
				MeshManager::getSingleton().remove(meshName);
		}

		//Remove any billboard data which might be left over if the user forgot to call build()
		std::vector<StaticBillboard*>::iterator i1, i2;
		i1 = billboardBuffer.begin();
		i2 = billboardBuffer.end();
		while (i1 != i2)
		{
			delete (*i1);
			++i1;
		}
		billboardBuffer.clear();
	} else {
		fallbackSet->clear();
	}
}

void StaticBillboardSet::build()
{
	if (renderMethod == BB_METHOD_ACCELERATED){
		//Delete old entity and mesh data
		if (entity){
			//Delete entity
			node->detachAllObjects();
			sceneMgr->destroyEntity(entity);
			entity = 0;
			
			//Delete mesh
			assert(!mesh.isNull());
			String meshName(mesh->getName());
			mesh.setNull();
			if (MeshManager::getSingletonPtr())
				MeshManager::getSingleton().remove(meshName);
		}

		//If there are no billboards to create, exit
		if (billboardBuffer.empty())
			return;

		//Create manual mesh to store billboard quads
		mesh = MeshManager::getSingleton().createManual(getUniqueID("SBSmesh"), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		subMesh = mesh->createSubMesh();
		subMesh->useSharedVertices = false;

		//Setup vertex format information
		subMesh->vertexData = new VertexData;
		subMesh->vertexData->vertexStart = 0;
		subMesh->vertexData->vertexCount = 4 * billboardBuffer.size();
		
		VertexDeclaration* dcl = subMesh->vertexData->vertexDeclaration;
		size_t offset = 0;
		dcl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		dcl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		dcl->addElement(0, offset, VET_COLOUR, VES_DIFFUSE);
		offset += VertexElement::getTypeSize(VET_COLOUR);
		dcl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);
		offset += VertexElement::getTypeSize(VET_FLOAT2);

		//Populate a new vertex buffer
		HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton()
											.createVertexBuffer(offset, subMesh->vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		float* pReal = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
		
		float minX = Math::POS_INFINITY, minY = Math::POS_INFINITY, minZ = Math::POS_INFINITY;
		float maxX = Math::NEG_INFINITY, maxY = Math::NEG_INFINITY, maxZ = Math::NEG_INFINITY;

		std::vector<StaticBillboard*>::iterator i1, i2;
		i1 = billboardBuffer.begin();
		i2 = billboardBuffer.end();
		while (i1 != i2)
		{
			StaticBillboard *bb = (*i1);
			float halfXScale = bb->xScale * 0.5f;
			float halfYScale = bb->yScale * 0.5f;
			
			// position
			*pReal++ = bb->position.x;
			*pReal++ = bb->position.y;
			*pReal++ = bb->position.z;
			// normals (actually used as scale / translate info for vertex shader)
			*pReal++ = halfXScale;
			*pReal++ = halfYScale;
			*pReal++ = 0.0f;
			// color
			*((uint32*)pReal++) = bb->color;
			// uv
			*pReal++ = (bb->texcoordIndexU * uFactor);
			*pReal++ = (bb->texcoordIndexV * vFactor);
			
			// position
			*pReal++ = bb->position.x;
			*pReal++ = bb->position.y;
			*pReal++ = bb->position.z;
			// normals (actually used as scale / translate info for vertex shader)
			*pReal++ = halfXScale;
			*pReal++ = halfYScale;
			*pReal++ = 1.0f;
			// color
			*((uint32*)pReal++) = bb->color;
			// uv
			*pReal++ = ((bb->texcoordIndexU + 1) * uFactor);
			*pReal++ = (bb->texcoordIndexV * vFactor);

			// position
			*pReal++ = bb->position.x;
			*pReal++ = bb->position.y;
			*pReal++ = bb->position.z;
			// normals (actually used as scale / translate info for vertex shader)
			*pReal++ = halfXScale;
			*pReal++ = halfYScale;
			*pReal++ = 2.0f;
			// color
			*((uint32*)pReal++) = bb->color;
			// uv
			*pReal++ = (bb->texcoordIndexU * uFactor);
			*pReal++ = ((bb->texcoordIndexV + 1) * vFactor);

			// position
			*pReal++ = bb->position.x;
			*pReal++ = bb->position.y;
			*pReal++ = bb->position.z;
			// normals (actually used as scale / translate info for vertex shader)
			*pReal++ = halfXScale;
			*pReal++ = halfYScale;
			*pReal++ = 3.0f;
			// color
			*((uint32*)pReal++) = bb->color;
			// uv
			*pReal++ = ((bb->texcoordIndexU + 1) * uFactor);
			*pReal++ = ((bb->texcoordIndexV + 1) * vFactor);
			
			//Update bounding box
			if (bb->position.x - halfXScale < minX) minX = bb->position.x - halfXScale;
			if (bb->position.x + halfXScale > maxX) maxX = bb->position.x + halfXScale;
			if (bb->position.y - halfYScale < minY) minY = bb->position.y - halfYScale;
			if (bb->position.y + halfYScale > maxY) maxY = bb->position.y + halfYScale;
			if (bb->position.z - halfXScale < minZ) minZ = bb->position.z - halfXScale;
			if (bb->position.z + halfXScale > maxZ) maxZ = bb->position.z + halfXScale;
			
			delete bb;
			++i1;
		}
		AxisAlignedBox bounds(minX, minY, minZ, maxX, maxY, maxZ);
		
		vbuf->unlock();
		subMesh->vertexData->vertexBufferBinding->setBinding(0, vbuf);
		
		//Populate index buffer
		subMesh->indexData->indexStart = 0;
		subMesh->indexData->indexCount = 6 * billboardBuffer.size();
		subMesh->indexData->indexBuffer = HardwareBufferManager::getSingleton()
							.createIndexBuffer(HardwareIndexBuffer::IT_16BIT, subMesh->indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		Ogre::uint16* pI = static_cast<Ogre::uint16*>(subMesh->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
		for (Ogre::uint16 i = 0; i < billboardBuffer.size(); ++i)
		{
			Ogre::uint16 offset = i * 4;

			*pI++ = 0 + offset;
			*pI++ = 2 + offset;
			*pI++ = 1 + offset;

			*pI++ = 1 + offset;
			*pI++ = 2 + offset;
			*pI++ = 3 + offset;
		}
		
		subMesh->indexData->indexBuffer->unlock();

		//Finish up mesh
		mesh->_setBounds(bounds);
		Vector3 temp = bounds.getMaximum() - bounds.getMinimum();
		mesh->_setBoundingSphereRadius(temp.length() * 0.5f);

		LogManager::getSingleton().setLogDetail(static_cast<LoggingLevel>(0));
		mesh->load();
		LogManager::getSingleton().setLogDetail(LL_NORMAL);

		//Empty the billboardBuffer now, because all billboards have been built
		billboardBuffer.clear();
		
		//Create an entity for the mesh
		entity = sceneMgr->createEntity(entityName, mesh->getName());
		entity->setCastShadows(false);

		//Apply texture
		if (fadeEnabled) {
			assert(!fadeMaterialPtr.isNull());
			entity->setMaterialName(fadeMaterialPtr->getName());
		} else {
			assert(!materialPtr.isNull());
			entity->setMaterialName(materialPtr->getName());
		}

		//Add to scene
		node->attachObject(entity);
		entity->setVisible(visible);
	}
}

void StaticBillboardSet::setMaterial(const String &materialName)
{
	if (renderMethod == BB_METHOD_ACCELERATED){
		if (materialPtr.isNull() || materialPtr->getName() != materialName){
			//Update material reference list
			if (fadeEnabled) {
				assert(!fadeMaterialPtr.isNull());
				SBMaterialRef::removeMaterialRef(fadeMaterialPtr);
			} else {
				if (!materialPtr.isNull())
					SBMaterialRef::removeMaterialRef(materialPtr);
			}

			materialPtr = MaterialManager::getSingleton().getByName(materialName);
			if (fadeEnabled) {
				fadeMaterialPtr = getFadeMaterial(fadeVisibleDist, fadeInvisibleDist);
				SBMaterialRef::addMaterialRef(fadeMaterialPtr, bbOrigin);
			} else {
				SBMaterialRef::addMaterialRef(materialPtr, bbOrigin);
			}
			
			//Apply material to entity
			if (entity){
				if (fadeEnabled){
					entity->setMaterialName(fadeMaterialPtr->getName());
				} else {
					entity->setMaterialName(materialPtr->getName());
				}
			}
		}
	} else {
		if (materialPtr.isNull() || materialPtr->getName() != materialName){
			materialPtr = MaterialManager::getSingleton().getByName(materialName);
			fallbackSet->setMaterialName(materialPtr->getName());
		}
	}
}

void StaticBillboardSet::setFade(bool enabled, Real visibleDist, Real invisibleDist)
{
	if (renderMethod == BB_METHOD_ACCELERATED){
		if (enabled){
			if (materialPtr.isNull())
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Billboard fading cannot be enabled without a material applied first", "StaticBillboardSet::setFade()");

			//Update material reference list
			if (fadeEnabled) {
				assert(!fadeMaterialPtr.isNull());
				SBMaterialRef::removeMaterialRef(fadeMaterialPtr);
			} else {
				assert(!materialPtr.isNull());
				SBMaterialRef::removeMaterialRef(materialPtr);
			}

			fadeMaterialPtr = getFadeMaterial(visibleDist, invisibleDist);
			SBMaterialRef::addMaterialRef(fadeMaterialPtr, bbOrigin);
			
			//Apply material to entity
			if (entity)
				entity->setMaterialName(fadeMaterialPtr->getName());

			fadeEnabled = enabled;
			fadeVisibleDist = visibleDist;
			fadeInvisibleDist = invisibleDist;
		} else {
			if (fadeEnabled){
				//Update material reference list
				assert(!fadeMaterialPtr.isNull());
				assert(!materialPtr.isNull());
				SBMaterialRef::removeMaterialRef(fadeMaterialPtr);
				SBMaterialRef::addMaterialRef(materialPtr, bbOrigin);
				
				//Apply material to entity
				if (entity)
					entity->setMaterialName(materialPtr->getName());

				fadeEnabled = enabled;
				fadeVisibleDist = visibleDist;
				fadeInvisibleDist = invisibleDist;
			}
		}
	}
}

void StaticBillboardSet::setTextureStacksAndSlices(Ogre::uint16 stacks, Ogre::uint16 slices)
{
	uFactor = 1.0f / slices;
	vFactor = 1.0f / stacks;
}

MaterialPtr StaticBillboardSet::getFadeMaterial(Real visibleDist, Real invisibleDist)
{
	StringUtil::StrStreamType materialSignature;
	materialSignature << entityName << "|";
	materialSignature << visibleDist << "|";
	materialSignature << invisibleDist << "|";
	materialSignature << materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureUScroll() << "|";
	materialSignature << materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureVScroll() << "|";

	FadedMaterialMap::iterator it = fadedMaterialMap.find(materialSignature.str());
	MaterialPtr fadeMaterial;

	//If a correctly faded version of the material exists...
	if (it != fadedMaterialMap.end()){
		//Use the existing fade material
		fadeMaterial = it->second;
	} else {
		//Otherwise clone the material
		fadeMaterial = materialPtr->clone(getUniqueID("ImpostorFade"));

		bool isglsl = false;
		if(Root::getSingleton().getRenderSystem()->getName() == "OpenGL Rendering Subsystem")
			isglsl = true;

		//And apply the fade shader
		for (unsigned short t = 0; t < fadeMaterial->getNumTechniques(); ++t){
			Technique *tech = fadeMaterial->getTechnique(t);
			for (unsigned short p = 0; p < tech->getNumPasses(); ++p){
				Pass *pass = tech->getPass(p);

				//Setup vertex program
				pass->setVertexProgram("SpriteFade_vp");
				GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();

				//glsl can use the built in gl_ModelViewProjectionMatrix
				if(!isglsl) params->setNamedAutoConstant("worldViewProj", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
				params->setNamedAutoConstant("uScroll", GpuProgramParameters::ACT_CUSTOM);
				params->setNamedAutoConstant("vScroll", GpuProgramParameters::ACT_CUSTOM);
				params->setNamedAutoConstant("preRotatedQuad[0]", GpuProgramParameters::ACT_CUSTOM);
				params->setNamedAutoConstant("preRotatedQuad[1]", GpuProgramParameters::ACT_CUSTOM);
				params->setNamedAutoConstant("preRotatedQuad[2]", GpuProgramParameters::ACT_CUSTOM);
				params->setNamedAutoConstant("preRotatedQuad[3]", GpuProgramParameters::ACT_CUSTOM);

				params->setNamedAutoConstant("camPos", GpuProgramParameters::ACT_CAMERA_POSITION_OBJECT_SPACE);
				params->setNamedAutoConstant("fadeGap", GpuProgramParameters::ACT_CUSTOM);
				params->setNamedAutoConstant("invisibleDist", GpuProgramParameters::ACT_CUSTOM);

				//Set fade ranges
				params->setNamedConstant("invisibleDist", invisibleDist);
				params->setNamedConstant("fadeGap", invisibleDist - visibleDist);
				
				pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
				//pass->setAlphaRejectFunction(CMPF_ALWAYS_PASS);
				//pass->setDepthWriteEnabled(false);
			}
		}

		//Add it to the list so it can be reused later
		fadedMaterialMap.insert(std::pair<String, MaterialPtr>(materialSignature.str(), fadeMaterial));
	}

	return fadeMaterial;
}

void StaticBillboardSet::updateAll(const Vector3 &cameraDirection)
{
	if (selfInstances > 0){  //selfInstances will only be greater than 0 if one or more StaticBillboardSet's are using BB_METHOD_ACCELERATED
		//Set shader parameter so material will face camera
		Vector3 forward = cameraDirection;
		Vector3 vRight = forward.crossProduct(Vector3::UNIT_Y);
		Vector3 vUp = forward.crossProduct(vRight);
		vRight.normalise();
		vUp.normalise();

		bool isglsl = false;
		if(Root::getSingleton().getRenderSystem()->getName() == "OpenGL Rendering Subsystem")
			isglsl = true;

		//Even if camera is upside down, the billboards should remain upright
		if (vUp.y < 0) vUp *= -1;

		//For each material in use by the billboard system..
		SBMaterialRefList::iterator i1, i2;
		i1 = SBMaterialRef::getList().begin();
		i2 = SBMaterialRef::getList().end();
		while (i1 != i2){
			Material *mat = i1->second->getMaterial();
			BillboardOrigin bbOrigin = i1->second->getOrigin();

			Vector3 vPoint0, vPoint1, vPoint2, vPoint3;
			if (bbOrigin == BBO_CENTER){
				vPoint0 = (-vRight + vUp);
				vPoint1 = ( vRight + vUp);
				vPoint2 = (-vRight - vUp);
				vPoint3 = ( vRight - vUp);
			}
			else if (bbOrigin == BBO_BOTTOM_CENTER){
				vPoint0 = (-vRight + vUp + vUp);
				vPoint1 = ( vRight + vUp + vUp);
				vPoint2 = (-vRight);
				vPoint3 = ( vRight);
			}

			//single prerotated quad oriented towards the camera
			float preRotatedQuad[16] = {
				vPoint0.x, vPoint0.y, vPoint0.z, 0.0f,
				vPoint1.x, vPoint1.y, vPoint1.z, 0.0f,
				vPoint2.x, vPoint2.y, vPoint2.z, 0.0f,
				vPoint3.x, vPoint3.y, vPoint3.z, 0.0f
			};

			//Ensure material is set up with the vertex shader
			Pass *p = mat->getTechnique(0)->getPass(0);
			if(!p->hasVertexProgram()){
				p->setVertexProgram("Sprite_vp");

				//glsl can use the built in gl_ModelViewProjectionMatrix
				if(!isglsl)	p->getVertexProgramParameters()->setNamedAutoConstant("worldViewProj", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
				p->getVertexProgramParameters()->setNamedAutoConstant("uScroll", GpuProgramParameters::ACT_CUSTOM);
				p->getVertexProgramParameters()->setNamedAutoConstant("vScroll", GpuProgramParameters::ACT_CUSTOM);
				p->getVertexProgramParameters()->setNamedAutoConstant("preRotatedQuad[0]", GpuProgramParameters::ACT_CUSTOM);
				p->getVertexProgramParameters()->setNamedAutoConstant("preRotatedQuad[1]", GpuProgramParameters::ACT_CUSTOM);
				p->getVertexProgramParameters()->setNamedAutoConstant("preRotatedQuad[2]", GpuProgramParameters::ACT_CUSTOM);
				p->getVertexProgramParameters()->setNamedAutoConstant("preRotatedQuad[3]", GpuProgramParameters::ACT_CUSTOM);
			}
			
			//Update the vertex shader parameters
			GpuProgramParametersSharedPtr params = p->getVertexProgramParameters();
			params->setNamedConstant("preRotatedQuad[0]", preRotatedQuad, 4);
			params->setNamedConstant("uScroll", p->getTextureUnitState(0)->getTextureUScroll());
			params->setNamedConstant("vScroll", p->getTextureUnitState(0)->getTextureVScroll());

			++i1;
		}
	}
}

void StaticBillboardSet::setBillboardOrigin(BillboardOrigin origin)
{
	if (origin != BBO_CENTER && origin != BBO_BOTTOM_CENTER)
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid origin - only BBO_CENTER and BBO_BOTTOM_CENTER is supported", "StaticBillboardSet::setBillboardOrigin()");

	if (renderMethod == BB_METHOD_ACCELERATED){
		bbOrigin = origin;
	} else {
		bbOrigin = origin;
		fallbackSet->setBillboardOrigin(origin);
	}
}


//-------------------------------------------------------------------------------------

SBMaterialRefList SBMaterialRef::selfList;

void SBMaterialRef::addMaterialRef(const MaterialPtr &matP, Ogre::BillboardOrigin o)
{
	Material *mat = matP.getPointer();

	SBMaterialRef *matRef;
	SBMaterialRefList::iterator it;
	it = selfList.find(mat);

	if (it != selfList.end()){
		//Material already exists in selfList - increment refCount
		matRef = it->second;
		++matRef->refCount;
	} else {
		//Material does not exist in selfList - add it
		matRef = new SBMaterialRef(mat, o);
		selfList[mat] = matRef;
		//No need to set refCount to 1 here because the SBMaterialRef
		//constructor sets refCount to 1.
	}
}

void SBMaterialRef::removeMaterialRef(const MaterialPtr &matP)
{
	Material *mat = matP.getPointer();

	SBMaterialRef *matRef;
	SBMaterialRefList::iterator it;

	//Find material in selfList
	it = selfList.find(mat);
	if (it != selfList.end()){
		//Decrease the reference count, and remove the item if refCount == 0
		matRef = it->second;
		if (--matRef->refCount == 0){
			delete matRef;
			selfList.erase(it);
		}
	}
}

SBMaterialRef::SBMaterialRef(Material *mat, Ogre::BillboardOrigin o)
{
	material = mat;
	origin = o;
	refCount = 1;
}
}
