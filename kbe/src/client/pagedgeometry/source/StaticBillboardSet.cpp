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

#include "StaticBillboardSet.h"

using namespace Ogre;
using namespace Forests;


// Static data initialization
StaticBillboardSet::FadedMaterialMap StaticBillboardSet::s_mapFadedMaterial;
bool StaticBillboardSet::s_isGLSL                  = false;
unsigned int StaticBillboardSet::s_nSelfInstances  = 0;
unsigned long StaticBillboardSet::GUID             = 0;


//-----------------------------------------------------------------------------
/// Constructor
StaticBillboardSet::StaticBillboardSet(SceneManager *mgr, SceneNode *rootSceneNode, BillboardMethod method) :
mVisible                (true),
mFadeEnabled            (false),
mRenderMethod           (method),
mpSceneMgr              (mgr),
mpSceneNode             (NULL),
mpEntity                (NULL),
mfUFactor               (1.f),
mfVFactor               (1.f),
mpFallbackBillboardSet  (NULL),
mBBOrigin               (BBO_CENTER),
mFadeVisibleDist        (0.f),
mFadeInvisibleDist      (0.f)
{
   assert(rootSceneNode);

   //Fall back to BB_METHOD_COMPATIBLE if vertex shaders are not available
   if (method == BB_METHOD_ACCELERATED)
   {
      const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();
      if (!caps->hasCapability(RSC_VERTEX_PROGRAM))
         mRenderMethod = BB_METHOD_COMPATIBLE;
   }

   mpSceneNode = rootSceneNode->createChildSceneNode();
   mEntityName = getUniqueID("SBSentity");

   if (mRenderMethod == BB_METHOD_ACCELERATED)
   {
      //Load vertex shader to align billboards to face the camera (if not loaded already)
      if (s_nSelfInstances == 0)
      {
         const Ogre::String &renderName = Root::getSingleton().getRenderSystem()->getName();
         s_isGLSL = renderName == "OpenGL Rendering Subsystem" ? true : false;
         Ogre::String shaderLanguage = s_isGLSL ? "glsl" : renderName == "Direct3D9 Rendering Subsystem" ? "hlsl" : "cg";

         //First shader, simple camera-alignment
         String vertexProg;
         if (!s_isGLSL) // DirectX HLSL or nVidia CG
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
         else     // OpenGL GLSL
         {
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

         HighLevelGpuProgramPtr vertexShader = HighLevelGpuProgramManager::getSingleton().getByName("Sprite_vp");
         assert(vertexShader.isNull() && "Sprite_vp already exist");

         vertexShader = HighLevelGpuProgramManager::getSingleton().createProgram(
            "Sprite_vp", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, shaderLanguage, GPT_VERTEX_PROGRAM);
         vertexShader->setSource(vertexProg);

         // Set entry point for vertex program. GLSL can only have one entry point "main".
         if (!s_isGLSL)
         {
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
            else
            {
               assert(false && "Unknown shader language");
            }
         }

         // compile vertex shader
         vertexShader->load();


         //====================================================================
         //Second shader, camera alignment and distance based fading
         String vertexProg2;
         if (!s_isGLSL) // DirectX HLSL or nVidia CG
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
         else        // OpenGL GLSL
         {
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

         HighLevelGpuProgramPtr vertexShader2 = HighLevelGpuProgramManager::getSingleton().getByName("SpriteFade_vp");
         assert(vertexShader2.isNull() && "SpriteFade_vp already exist");
         vertexShader2 = HighLevelGpuProgramManager::getSingleton().createProgram("SpriteFade_vp",
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, shaderLanguage, GPT_VERTEX_PROGRAM);
         vertexShader2->setSource(vertexProg2);

         // Set entry point. GLSL can only have one entry point "main".
         if (!s_isGLSL)
         {

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
         }
         // compile it
         vertexShader2->load();
      }
   }
   else
   {
      //Compatible billboard method
      mpFallbackBillboardSet = mgr->createBillboardSet(getUniqueID("SBS"), 100);
      mpSceneNode->attachObject(mpFallbackBillboardSet);
      mfUFactor = mfVFactor = Ogre::Real(0.);
   }


   ++s_nSelfInstances;
}


//-----------------------------------------------------------------------------
/// Destructor
StaticBillboardSet::~StaticBillboardSet()
{
   if (mRenderMethod == BB_METHOD_ACCELERATED)
   {
      clear(); // Delete mesh data

      //Update material reference list
      if (!mPtrMaterial.isNull())
         SBMaterialRef::removeMaterialRef(mPtrMaterial);
      if (!mPtrFadeMaterial.isNull())
         SBMaterialRef::removeMaterialRef(mPtrFadeMaterial);

      //Delete vertex shaders and materials if no longer in use
      if (--s_nSelfInstances == 0)
         s_mapFadedMaterial.clear();  //Delete fade materials
   }
   else
      //Remove billboard set
      mpSceneMgr->destroyBillboardSet(mpFallbackBillboardSet);

   //Delete scene node
   if (mpSceneNode->getParent())
      mpSceneNode->getParentSceneNode()->removeAndDestroyChild(mpSceneNode->getName());
   else
      mpSceneNode->getCreator()->destroySceneNode(mpSceneNode);
}


//-----------------------------------------------------------------------------
///
void StaticBillboardSet::clear()
{
   if (mRenderMethod == BB_METHOD_ACCELERATED)
   {
      //Delete the entity and mesh data
      if (mpEntity)
      {
         //Delete entity
         mpSceneNode->detachAllObjects();
         mpEntity->_getManager()->destroyEntity(mpEntity);
         mpEntity = NULL;

         //Delete mesh
         String meshName(mPtrMesh->getName());
         mPtrMesh.setNull();
         MeshManager::getSingleton().remove(meshName);
      }

      if (!mBillboardBuffer.empty())
      {
         //Remove any billboard data which might be left over if the user forgot to call build()
         for (int i = mBillboardBuffer.size() - 1; i > 0; /* empty */ )
            delete mBillboardBuffer[--i];
         mBillboardBuffer.clear();
      }
   }
   else
      mpFallbackBillboardSet->clear();
}


//-----------------------------------------------------------------------------
/// Performs final steps required for the created billboards to appear in the scene.
void StaticBillboardSet::build()
{
   if (mRenderMethod == BB_METHOD_ACCELERATED)
   {
      //Delete old entity and mesh data
      if (mpEntity)
      {
         //Delete entity
         mpSceneNode->detachAllObjects();
         mpEntity->_getManager()->destroyEntity(mpEntity);
         mpEntity = NULL;

         //Delete mesh
         assert(!mPtrMesh.isNull());
         String meshName(mPtrMesh->getName());
         mPtrMesh.setNull();
         MeshManager::getSingleton().remove(meshName);
      }

      //If there are no billboards to create, exit
      if (mBillboardBuffer.empty())
         return;

      //Create manual mesh to store billboard quads
      mPtrMesh = MeshManager::getSingleton().createManual(getUniqueID("SBSmesh"), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
      Ogre::SubMesh *pSubMesh = mPtrMesh->createSubMesh();
      pSubMesh->useSharedVertices = false;

      //Setup vertex format information
      pSubMesh->vertexData = new VertexData;
      pSubMesh->vertexData->vertexStart = 0;
      pSubMesh->vertexData->vertexCount = 4 * mBillboardBuffer.size();

      VertexDeclaration* dcl = pSubMesh->vertexData->vertexDeclaration;
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
      HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
         offset, pSubMesh->vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
      float* pReal = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));

      float minX = (float)Math::POS_INFINITY, minY = (float)Math::POS_INFINITY, minZ = (float)Math::POS_INFINITY;
      float maxX = (float)Math::NEG_INFINITY, maxY = (float)Math::NEG_INFINITY, maxZ = (float)Math::NEG_INFINITY;

      // For each billboard
      size_t billboardCount = mBillboardBuffer.size();
      for (size_t ibb = 0; ibb < billboardCount; ++ibb)
      {
         const StaticBillboard *bb = mBillboardBuffer[ibb];

         // position
         ////*pReal++ = bb->xPos;
         ////*pReal++ = bb->yPos;
         ////*pReal++ = bb->zPos;

         ////// normals (actually used as scale / translate info for vertex shader)
         ////*pReal++ = bb->xScaleHalf;
         ////*pReal++ = bb->yScaleHalf;
         memcpy(pReal, &(bb->xPos), 5 * sizeof(float));
         pReal += 5; // move to next float value
         *pReal++ = 0.0f;

         // color
         *(reinterpret_cast< uint32* >(pReal++)) = bb->color;
         // uv
         *pReal++ = (bb->texcoordIndexU * mfUFactor);
         *pReal++ = (bb->texcoordIndexV * mfVFactor);


         // position
         //////*pReal++ = bb->xPos;
         //////*pReal++ = bb->yPos;
         //////*pReal++ = bb->zPos;
         //////// normals (actually used as scale / translate info for vertex shader)
         //////*pReal++ = bb->xScaleHalf;
         //////*pReal++ = bb->yScaleHalf;
         memcpy(pReal, &(bb->xPos), 5 * sizeof(float));
         pReal += 5; // move to next float value
         *pReal++ = 1.0f;
         // color
         *(reinterpret_cast< uint32* >(pReal++)) = bb->color;
         // uv
         *pReal++ = ((bb->texcoordIndexU + 1) * mfUFactor);
         *pReal++ = (bb->texcoordIndexV * mfVFactor);


         // position
         //////*pReal++ = bb->xPos;
         //////*pReal++ = bb->yPos;
         //////*pReal++ = bb->zPos;
         //////// normals (actually used as scale / translate info for vertex shader)
         //////*pReal++ = bb->xScaleHalf;
         //////*pReal++ = bb->yScaleHalf;
         memcpy(pReal, &(bb->xPos), 5 * sizeof(float));
         pReal += 5; // move to next float value
         *pReal++ = 2.0f;
         // color
         *(reinterpret_cast< uint32* > (pReal++)) = bb->color;
         // uv
         *pReal++ = (bb->texcoordIndexU * mfUFactor);
         *pReal++ = ((bb->texcoordIndexV + 1) * mfVFactor);


         // position
         //////*pReal++ = bb->xPos;
         //////*pReal++ = bb->yPos;
         //////*pReal++ = bb->zPos;
         //////// normals (actually used as scale / translate info for vertex shader)
         //////*pReal++ = bb->xScaleHalf;
         //////*pReal++ = bb->yScaleHalf;
         memcpy(pReal, &(bb->xPos), 5 * sizeof(float));
         pReal += 5; // move to next float value
         *pReal++ = 3.0f;
         // color
         *(reinterpret_cast< uint32* >(pReal++)) = bb->color;
         // uv
         *pReal++ = ((bb->texcoordIndexU + 1) * mfUFactor);
         *pReal++ = ((bb->texcoordIndexV + 1) * mfVFactor);

         //Update bounding box
         if (bb->xPos - bb->xScaleHalf < minX) minX = bb->xPos - bb->xScaleHalf;
         if (bb->xPos + bb->xScaleHalf > maxX) maxX = bb->xPos + bb->xScaleHalf;
         if (bb->yPos - bb->yScaleHalf < minY) minY = bb->yPos - bb->yScaleHalf;
         if (bb->yPos + bb->yScaleHalf > maxY) maxY = bb->yPos + bb->yScaleHalf;
         //if (bb->zPos - halfXScale < minZ) minZ = bb->zPos - halfXScale;
         //if (bb->zPos + halfXScale > maxZ) maxZ = bb->zPos + halfXScale;
         if (bb->zPos < minZ) minZ = bb->zPos - 0.5f;
         if (bb->zPos > maxZ) maxZ = bb->zPos + 0.5f;

         delete bb;
      }
      mBillboardBuffer.clear(); //Empty the mBillboardBuffer now, because all billboards have been built

      vbuf->unlock();
      pSubMesh->vertexData->vertexBufferBinding->setBinding(0, vbuf);

      // Populate index buffer
      {
         pSubMesh->indexData->indexStart = 0;
         pSubMesh->indexData->indexCount = 6 * billboardCount;
         assert(pSubMesh->indexData->indexCount <= std::numeric_limits<Ogre::uint16>::max() && "To many indices. Use 32 bit indices");
         Ogre::HardwareIndexBufferSharedPtr &ptrIndBuf = pSubMesh->indexData->indexBuffer;
         ptrIndBuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT,
            pSubMesh->indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

         Ogre::uint16 *pIBuf = static_cast < Ogre::uint16* > (ptrIndBuf->lock(HardwareBuffer::HBL_DISCARD));
         for (Ogre::uint16 i = 0; i < billboardCount; ++i)
         {
            Ogre::uint16 offset = i * 4;

            *pIBuf++ = 0 + offset;
            *pIBuf++ = 2 + offset;
            *pIBuf++ = 1 + offset;

            *pIBuf++ = 1 + offset;
            *pIBuf++ = 2 + offset;
            *pIBuf++ = 3 + offset;
         }
         ptrIndBuf->unlock(); // Unlock buffer and update GPU
      }

      // Finish up mesh
      {
         AxisAlignedBox bounds(minX, minY, minZ, maxX, maxY, maxZ);
         mPtrMesh->_setBounds(bounds);
         Vector3 temp = bounds.getMaximum() - bounds.getMinimum();
         mPtrMesh->_setBoundingSphereRadius(temp.length() * 0.5f);

         // Loading mesh
         Ogre::LoggingLevel logLev = LogManager::getSingleton().getDefaultLog()->getLogDetail();
         LogManager::getSingleton().setLogDetail(LL_LOW);
         mPtrMesh->load();
         LogManager::getSingleton().setLogDetail(logLev);
      }

      // Create an entity for the mesh
      mpEntity = mpSceneMgr->createEntity(mEntityName, mPtrMesh->getName(), mPtrMesh->getGroup());
      mpEntity->setCastShadows(false);
      mpEntity->setMaterial(mFadeEnabled ? mPtrFadeMaterial : mPtrMaterial);

      // Add to scene
      mpSceneNode->attachObject(mpEntity);
      mpEntity->setVisible(mVisible);
   }
}


//-----------------------------------------------------------------------------
///
void StaticBillboardSet::setMaterial(const String &materialName, const Ogre::String &resourceGroup)
{
   bool needUpdateMat = mPtrMaterial.isNull() || mPtrMaterial->getName() != materialName || mPtrMaterial->getGroup() != resourceGroup;
   if (!needUpdateMat)
      return;

   if (mRenderMethod == BB_METHOD_ACCELERATED)
   {
      //Update material reference list
      if (mFadeEnabled)
      {
         assert(!mPtrFadeMaterial.isNull());
         SBMaterialRef::removeMaterialRef(mPtrFadeMaterial);
      }
      else if (!mPtrMaterial.isNull())
         SBMaterialRef::removeMaterialRef(mPtrMaterial);

      mPtrMaterial = MaterialManager::getSingleton().getByName(materialName, resourceGroup);

      if (mFadeEnabled)
      {
         mPtrFadeMaterial = getFadeMaterial(mPtrMaterial, mFadeVisibleDist, mFadeInvisibleDist);
         SBMaterialRef::addMaterialRef(mPtrFadeMaterial, mBBOrigin);
      }
      else 
         SBMaterialRef::addMaterialRef(mPtrMaterial, mBBOrigin);

      //Apply material to entity
      if (mpEntity)
         mpEntity->setMaterial(mFadeEnabled ? mPtrFadeMaterial : mPtrMaterial);
   }
   else  // old GPU compatibility
   {
      mPtrMaterial = MaterialManager::getSingleton().getByName(materialName, resourceGroup);
      mpFallbackBillboardSet->setMaterialName(mPtrMaterial->getName(), mPtrMaterial->getGroup());
      // SVA. Since Ogre 1.7.3 Ogre::BillboardSet have setMaterial(const MaterialPtr&) method
   }
}


//-----------------------------------------------------------------------------
///
void StaticBillboardSet::setFade(bool enabled, Real visibleDist, Real invisibleDist)
{
   if (mRenderMethod == BB_METHOD_ACCELERATED)
   {
      if (enabled)
      {
         if (mPtrMaterial.isNull())
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Billboard fading cannot be enabled without a material applied first", "StaticBillboardSet::setFade()");

         //Update material reference list
         if (mFadeEnabled)
         {
            assert(!mPtrFadeMaterial.isNull());
            SBMaterialRef::removeMaterialRef(mPtrFadeMaterial);
         }
         else
         {
            assert(!mPtrMaterial.isNull());
            SBMaterialRef::removeMaterialRef(mPtrMaterial);
         }

         mPtrFadeMaterial = getFadeMaterial(mPtrMaterial, visibleDist, invisibleDist);
         SBMaterialRef::addMaterialRef(mPtrFadeMaterial, mBBOrigin);

         //Apply material to entity
         if (mpEntity)
            mpEntity->setMaterial(mPtrFadeMaterial);

         mFadeEnabled = true;
         mFadeVisibleDist = visibleDist;
         mFadeInvisibleDist = invisibleDist;
      }
      else  // disable
      {
         if (mFadeEnabled)
         {
            //Update material reference list
            assert(!mPtrFadeMaterial.isNull());
            assert(!mPtrMaterial.isNull());
            SBMaterialRef::removeMaterialRef(mPtrFadeMaterial);
            SBMaterialRef::addMaterialRef(mPtrMaterial, mBBOrigin);

            //Apply material to entity
            if (mpEntity)
               mpEntity->setMaterial(mPtrMaterial);

            mFadeEnabled = false;
            mFadeVisibleDist = visibleDist;
            mFadeInvisibleDist = invisibleDist;
         }
      }
   }
}


//-----------------------------------------------------------------------------
///
void StaticBillboardSet::setTextureStacksAndSlices(Ogre::uint16 stacks, Ogre::uint16 slices)
{
   assert(stacks != 0 && slices != 0 && "division by zero");
   mfUFactor = 1.0f / slices;
   mfVFactor = 1.0f / stacks;
}


//-----------------------------------------------------------------------------
///
MaterialPtr StaticBillboardSet::getFadeMaterial(const Ogre::MaterialPtr &protoMaterial,
                                                Real visibleDist_, Real invisibleDist_)
{
   assert(!protoMaterial.isNull());

   StringUtil::StrStreamType materialSignature;
   materialSignature << mEntityName << "|";
   materialSignature << visibleDist_ << "|";
   materialSignature << invisibleDist_ << "|";
   materialSignature << protoMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureUScroll() << "|";
   materialSignature << protoMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureVScroll() << "|";

   FadedMaterialMap::iterator it = s_mapFadedMaterial.find(materialSignature.str());
   if (it != s_mapFadedMaterial.end())
      return it->second; //Use the existing fade material
   else
   {
      MaterialPtr fadeMaterial = protoMaterial->clone(getUniqueID("ImpostorFade"));

      bool isglsl = Root::getSingleton().getRenderSystem()->getName() == "OpenGL Rendering Subsystem" ? true : false;

      //And apply the fade shader
      for (unsigned short t = 0; t < fadeMaterial->getNumTechniques(); ++t)
      {
         Technique *tech = fadeMaterial->getTechnique(t);
         for (unsigned short p = 0; p < tech->getNumPasses(); ++p)
         {
            Pass *pass = tech->getPass(p);

            //Setup vertex program
            pass->setVertexProgram("SpriteFade_vp");
            GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();

            //glsl can use the built in gl_ModelViewProjectionMatrix
            if (!isglsl)
               params->setNamedAutoConstant("worldViewProj", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);

            static const Ogre::String uScroll = "uScroll", vScroll = "vScroll",
               preRotatedQuad0 = "preRotatedQuad[0]", preRotatedQuad1 = "preRotatedQuad[1]",
               preRotatedQuad2 = "preRotatedQuad[2]", preRotatedQuad3 = "preRotatedQuad[3]",
               camPos = "camPos", fadeGap = "fadeGap", invisibleDist = "invisibleDist";

            params->setNamedAutoConstant(uScroll, GpuProgramParameters::ACT_CUSTOM);
            params->setNamedAutoConstant(vScroll, GpuProgramParameters::ACT_CUSTOM);
            params->setNamedAutoConstant(preRotatedQuad0, GpuProgramParameters::ACT_CUSTOM);
            params->setNamedAutoConstant(preRotatedQuad1, GpuProgramParameters::ACT_CUSTOM);
            params->setNamedAutoConstant(preRotatedQuad2, GpuProgramParameters::ACT_CUSTOM);
            params->setNamedAutoConstant(preRotatedQuad3, GpuProgramParameters::ACT_CUSTOM);

            params->setNamedAutoConstant(camPos, GpuProgramParameters::ACT_CAMERA_POSITION_OBJECT_SPACE);
            params->setNamedAutoConstant(fadeGap, GpuProgramParameters::ACT_CUSTOM);
            params->setNamedAutoConstant(invisibleDist, GpuProgramParameters::ACT_CUSTOM);

            //Set fade ranges
            params->setNamedConstant(invisibleDist, invisibleDist_);
            params->setNamedConstant(fadeGap, invisibleDist_ - visibleDist_);

            pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
            //pass->setAlphaRejectFunction(CMPF_ALWAYS_PASS);
            //pass->setDepthWriteEnabled(false);

         }  // for Pass

      }  // for Technique

      //Add it to the list so it can be reused later
      s_mapFadedMaterial.insert(std::pair<String, MaterialPtr>(materialSignature.str(), fadeMaterial));

      return fadeMaterial;
   }
}


//-----------------------------------------------------------------------------
///
void StaticBillboardSet::updateAll(const Vector3 &cameraDirection)
{
   // s_nSelfInstances will only be greater than 0 if one or more StaticBillboardSet's are using BB_METHOD_ACCELERATED
   if (s_nSelfInstances == 0)
      return;

   //Set shader parameter so material will face camera
   Vector3 forward = cameraDirection;
   Vector3 vRight = forward.crossProduct(Vector3::UNIT_Y);
   Vector3 vUp = forward.crossProduct(vRight);
   vRight.normalise();
   vUp.normalise();

   //Even if camera is upside down, the billboards should remain upright
   if (vUp.y < 0)
      vUp *= -1;

   // Precompute preRotatedQuad for both cases (BBO_CENTER, BBO_BOTTOM_CENTER)
   
   Vector3 vPoint0 = (-vRight + vUp);
   Vector3 vPoint1 = ( vRight + vUp);
   Vector3 vPoint2 = (-vRight - vUp);
   Vector3 vPoint3 = ( vRight - vUp);

   float preRotatedQuad_BBO_CENTER[16] = // single prerotated quad oriented towards the camera
   {
      (float)vPoint0.x, (float)vPoint0.y, (float)vPoint0.z, 0.0f,
      (float)vPoint1.x, (float)vPoint1.y, (float)vPoint1.z, 0.0f,
      (float)vPoint2.x, (float)vPoint2.y, (float)vPoint2.z, 0.0f,
      (float)vPoint3.x, (float)vPoint3.y, (float)vPoint3.z, 0.0f
   };

   vPoint0 = (-vRight + vUp + vUp);
   vPoint1 = ( vRight + vUp + vUp);
   vPoint2 = (-vRight);
   vPoint3 = ( vRight);
   float preRotatedQuad_BBO_BOTTOM_CENTER[16] =
   {
      (float)vPoint0.x, (float)vPoint0.y, (float)vPoint0.z, 0.0f,
      (float)vPoint1.x, (float)vPoint1.y, (float)vPoint1.z, 0.0f,
      (float)vPoint2.x, (float)vPoint2.y, (float)vPoint2.z, 0.0f,
      (float)vPoint3.x, (float)vPoint3.y, (float)vPoint3.z, 0.0f
   };

   // Shaders uniform variables
   static const Ogre::String uScroll = "uScroll", vScroll = "vScroll", preRotatedQuad0 = "preRotatedQuad[0]",
      preRotatedQuad1 = "preRotatedQuad[1]", preRotatedQuad2 = "preRotatedQuad[2]", preRotatedQuad3 = "preRotatedQuad[3]";

   // SVA for Ogre::Material hack
   const GpuConstantDefinition *pGPU_ConstDef_preRotatedQuad0 = 0,
      *pGPU_ConstDef_uScroll = 0, *pGPU_ConstDef_vScroll = 0;

   // For each material in use by the billboard system..
   bool firstIteraion = true;
   SBMaterialRefList::iterator i1 = SBMaterialRef::getList().begin(), iend = SBMaterialRef::getList().end();
   while (i1 != iend)
   {
      Ogre::Material *mat = i1->second->getMaterial();

      // Ensure material is set up with the vertex shader
      Pass *p = mat->getTechnique(0)->getPass(0);
      if (!p->hasVertexProgram())
      {
         static const Ogre::String Sprite_vp = "Sprite_vp";
         p->setVertexProgram(Sprite_vp);

         // glsl can use the built in gl_ModelViewProjectionMatrix
         if (!s_isGLSL)
            p->getVertexProgramParameters()->setNamedAutoConstant("worldViewProj", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);

         GpuProgramParametersSharedPtr params = p->getVertexProgramParameters();
         params->setNamedAutoConstant(uScroll, GpuProgramParameters::ACT_CUSTOM);
         params->setNamedAutoConstant(vScroll, GpuProgramParameters::ACT_CUSTOM);
         params->setNamedAutoConstant(preRotatedQuad0, GpuProgramParameters::ACT_CUSTOM);
         params->setNamedAutoConstant(preRotatedQuad1, GpuProgramParameters::ACT_CUSTOM);
         params->setNamedAutoConstant(preRotatedQuad2, GpuProgramParameters::ACT_CUSTOM);
         params->setNamedAutoConstant(preRotatedQuad3, GpuProgramParameters::ACT_CUSTOM);
      }

      // Which prerotated quad use
      const float *pQuad = i1->second->getOrigin() == BBO_CENTER ? preRotatedQuad_BBO_CENTER : preRotatedQuad_BBO_BOTTOM_CENTER;

      // Update the vertex shader parameters
      GpuProgramParametersSharedPtr params = p->getVertexProgramParameters();
      //params->setNamedConstant(preRotatedQuad0, pQuad, 4);
      //params->setNamedConstant(uScroll, p->getTextureUnitState(0)->getTextureUScroll());
      //params->setNamedConstant(vScroll, p->getTextureUnitState(0)->getTextureVScroll());

      // SVA some hack of Ogre::Material.
      // Since material are cloned and use same vertex shader "Sprite_vp" hardware GPU indices
      // must be same. I don`t know planes of Ogre Team to change this behaviour.
      // Therefore this may be unsafe code. Instead of 3 std::map lookups(map::find(const Ogre::String&)) do only 1
      {
         const GpuConstantDefinition *def = params->_findNamedConstantDefinition(preRotatedQuad0, true);
         if (def != pGPU_ConstDef_preRotatedQuad0) // new material, reread
         {
            pGPU_ConstDef_preRotatedQuad0 = def;
            pGPU_ConstDef_uScroll         = params->_findNamedConstantDefinition(uScroll, true);
            pGPU_ConstDef_vScroll         = params->_findNamedConstantDefinition(vScroll, true);
         }
      }

      float fUScroll = (float)p->getTextureUnitState(0)->getTextureUScroll(),
         fVScroll = (float)p->getTextureUnitState(0)->getTextureVScroll();
      params->_writeRawConstants(pGPU_ConstDef_preRotatedQuad0->physicalIndex, pQuad, 16);
      params->_writeRawConstants(pGPU_ConstDef_uScroll->physicalIndex, &fUScroll, 1);
      params->_writeRawConstants(pGPU_ConstDef_vScroll->physicalIndex, &fVScroll, 1);
      
      ++i1; // next material in billboard system
   }
}


//-----------------------------------------------------------------------------
///
void StaticBillboardSet::setBillboardOrigin(BillboardOrigin origin)
{
   assert((origin == BBO_CENTER || origin == BBO_BOTTOM_CENTER) && "Invalid origin - only BBO_CENTER and BBO_BOTTOM_CENTER is supported");
   mBBOrigin = origin;
   if (mRenderMethod != BB_METHOD_ACCELERATED)
      mpFallbackBillboardSet->setBillboardOrigin(origin);
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
