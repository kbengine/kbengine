/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
    1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

//WindBatchPage.cpp
//WindBatchPage is an extension to PagedGeometry which displays entities as static geometry but that is affected by wind.
//-------------------------------------------------------------------------------------

#include "WindBatchPage.h"
#include "WindBatchedGeometry.h"

#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreEntity.h>
#include <OgreRenderSystem.h>
#include <OgreRenderSystemCapabilities.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreHighLevelGpuProgramManager.h>

// to dump the shader source in a file
#include <fstream>
using namespace Ogre;

namespace Forests {

//-------------------------------------------------------------------------------------


void WindBatchPage::init(PagedGeometry *geom, const Any &data)
{
	int datacast = data.isEmpty() ? 0 : Ogre::any_cast<int>(data);
#ifdef _DEBUG
	if ( datacast < 0)
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,"Data of WindBatchPage must be a positive integer. It representing the LOD level this detail level stores.","WindBatchPage::WindBatchPage");
#endif
	mLODLevel = datacast; 

	mGeom = geom;
	sceneMgr = geom->getSceneManager();
	batch = new WindBatchedGeometry(sceneMgr, geom->getSceneNode());
	dynamic_cast<WindBatchedGeometry*>(batch)->setGeom(geom);
	fadeEnabled = false;

	const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();
	if (caps->hasCapability(RSC_VERTEX_PROGRAM))
		shadersSupported = true;
	else
		shadersSupported = false;

	++refCount;
}

void WindBatchPage::_updateShaders()
{
	if (!shadersSupported)
		return;

	uint32 i = 0;
	BatchedGeometry::SubBatchIterator it = batch->getSubBatchIterator();
	while (it.hasMoreElements()){
		WindBatchedGeometry::WindSubBatch *subBatch = dynamic_cast<WindBatchedGeometry::WindSubBatch*>(it.getNext());
		MaterialPtr mat = unfadedMaterials[i++];

		//Check if lighting should be enabled
		bool lightingEnabled = false;
		for (unsigned short t = 0; t < mat->getNumTechniques(); ++t){
			Technique *tech = mat->getTechnique(t);
			for (unsigned short p = 0; p < tech->getNumPasses(); ++p){
				Pass *pass = tech->getPass(p);
				if (pass->getLightingEnabled()) {
					lightingEnabled = true;
					break;
				}
			}
			if (lightingEnabled)
				break;
		}

		//Compile the shader script based on various material / fade options
		StringUtil::StrStreamType tmpName;
		tmpName << "BatchPage_";
		if (fadeEnabled)
			tmpName << "fade_";
		if (lightingEnabled)
			tmpName << "lit_";
		if (subBatch->vertexData->vertexDeclaration->findElementBySemantic(VES_DIFFUSE) != NULL)
			tmpName << "clr_";

		for (unsigned short i = 0; i < subBatch->vertexData->vertexDeclaration->getElementCount(); ++i) {
			const VertexElement *el = subBatch->vertexData->vertexDeclaration->getElement(i);
			if (el->getSemantic() == VES_TEXTURE_COORDINATES) {
				String uvType = "";
				switch (el->getType()) {
						case VET_FLOAT1: uvType = "1"; break;
						case VET_FLOAT2: uvType = "2"; break;
						case VET_FLOAT3: uvType = "3"; break;
						case VET_FLOAT4: uvType = "4"; break;
				}
				tmpName << uvType << '_';
			}
		}

		tmpName << "vp";

		const String vertexProgName = tmpName.str();

		String shaderLanguage;
		if (Root::getSingleton().getRenderSystem()->getName() == "Direct3D9 Rendering Subsystem")
			shaderLanguage = "hlsl";
		else if(Root::getSingleton().getRenderSystem()->getName() == "OpenGL Rendering Subsystem")
			shaderLanguage = "glsl";
		else
			shaderLanguage = "cg";

		//If the shader hasn't been created yet, create it
		if (HighLevelGpuProgramManager::getSingleton().getByName(vertexProgName).isNull())
		{
			Pass *pass = mat->getTechnique(0)->getPass(0);
			String vertexProgSource;

			if(!shaderLanguage.compare("hlsl") || !shaderLanguage.compare("cg"))
			{

				vertexProgSource =
					"void main( \n"
					"	float4 iPosition : POSITION, \n"
					"	float3 normal	 : NORMAL, \n"
					"	out float4 oPosition : POSITION, \n";

				if (subBatch->vertexData->vertexDeclaration->findElementBySemantic(VES_DIFFUSE) != NULL)
				{
					vertexProgSource += 
						"	float4 iColor	 : COLOR, \n";
				}

				int texNum = 0;

				unsigned short texCoordCount = 0;
				for (unsigned short j = 0; j < subBatch->vertexData->vertexDeclaration->getElementCount(); ++j) 
				{
					const VertexElement *el = subBatch->vertexData->vertexDeclaration->getElement(j);
					if (el->getSemantic() == VES_TEXTURE_COORDINATES) 
					{
						++ texCoordCount;
					}
				}

				for (unsigned short i = 0; i < subBatch->vertexData->vertexDeclaration->getElementCount(); ++i)
				{
					const VertexElement *el = subBatch->vertexData->vertexDeclaration->getElement(i);
					if (el->getSemantic() == VES_TEXTURE_COORDINATES)
					{
						if (el->getIndex() == texCoordCount - 2)
						{
							vertexProgSource += 
								"	float4 params 	: TEXCOORD" + StringConverter::toString(texCoordCount-2) + ", \n";
						}
						else
						{
							if (el->getIndex() == texCoordCount - 1)
							{
								vertexProgSource += 
									"	float4 originPos 	: TEXCOORD" + StringConverter::toString(texCoordCount-1) + ", \n";
							}
							else
							{
								String uvType = "";
								switch (el->getType())
								{
									case VET_FLOAT1: uvType = "float"; break;
									case VET_FLOAT2: uvType = "float2"; break;
									case VET_FLOAT3: uvType = "float3"; break;
									case VET_FLOAT4: uvType = "float4"; break;
								}

								vertexProgSource += 
									"	" + uvType + " iUV" + StringConverter::toString(texNum) + "			: TEXCOORD" + StringConverter::toString(texNum) + ", \n"
									"	out " + uvType + " oUV" + StringConverter::toString(texNum) + "		: TEXCOORD" + StringConverter::toString(texNum) + ", \n";
							}
							++texNum;
						}
					}
				}

				vertexProgSource +=
					"	out float oFog : FOG, \n"
					"	out float4 oColor : COLOR, \n";

				if (lightingEnabled)
				{
					 vertexProgSource +=
						"	uniform float4 objSpaceLight, \n"
						"	uniform float4 lightDiffuse, \n"
						"	uniform float4 lightAmbient, \n";
				}

				if (fadeEnabled)
				{
					vertexProgSource +=
						"	uniform float3 camPos, \n"
						"	uniform float fadeGap, \n"
						"	uniform float invisibleDist, \n";
				}

				vertexProgSource +=
					"	uniform float4x4 worldViewProj,\n"
					"	uniform float time) \n "
					"{	\n";

				if (lightingEnabled)
				{
					//Perform lighting calculations (no specular)
					vertexProgSource +=
						"	float3 light = normalize(objSpaceLight.xyz - (iPosition.xyz * objSpaceLight.w)); \n"
						"	float diffuseFactor = max(dot(normal, light), 0); \n";

					if (subBatch->vertexData->vertexDeclaration->findElementBySemantic(VES_DIFFUSE) != NULL)
					{
						vertexProgSource +=
							"	oColor = (lightAmbient + diffuseFactor * lightDiffuse) * iColor; \n";
					}
					else
					{
						vertexProgSource +=
							"	oColor = (lightAmbient + diffuseFactor * lightDiffuse); \n";
					}
				}
				else
				{
					if (subBatch->vertexData->vertexDeclaration->findElementBySemantic(VES_DIFFUSE) != NULL)
					{
						vertexProgSource +=
							"	oColor = iColor; \n";
					}
					else
					{
						vertexProgSource +=
							"	oColor = float4(1, 1, 1, 1); \n";
					}
				}

				if (fadeEnabled)
				{
					//Fade out in the distance
					 vertexProgSource +=
						"	float dist = distance(camPos.xz, iPosition.xz); \n"
						"	oColor.a *= (invisibleDist - dist) / fadeGap; \n";
				}

				for (unsigned short i = 0; i < texCoordCount - 2; ++i)
				{
					vertexProgSource += 
						"	oUV" + StringConverter::toString(i) + " = iUV" + StringConverter::toString(i) + "; \n";
				}

				vertexProgSource +=
					"	float radiusCoeff = params.x; \n"
					"	float heightCoeff = params.y; \n"
					"	float factorX = params.z; \n"
					"	float factorY = params.w; \n"
					"	float4 tmpPos = iPosition; \n"

					/* 
					2 different methods are used to for the sin calculation :
					- the first one gives a better effect but at the cost of a few fps because of the 2 sines
					- the second one uses less ressources but is a bit less realistic

						a sin approximation could be use to optimize performances
					*/
	#if 1
					"	tmpPos.y += sin(time + originPos.z + tmpPos.y + tmpPos.x) * radiusCoeff * radiusCoeff * factorY; \n"
					"	tmpPos.x += sin(time + originPos.z ) * heightCoeff * heightCoeff * factorX ; \n"
	#else
					"	float sinval = sin(time + originPos.z ); \n"
					"	tmpPos.y += sinval * radiusCoeff * radiusCoeff * factorY; \n"
					"	tmpPos.x += sinval * heightCoeff * heightCoeff * factorX ; \n"
	#endif
					"	oPosition = mul(worldViewProj, tmpPos); \n"
					"	oFog = oPosition.z; \n"
					"}";
			}

			if(!shaderLanguage.compare("glsl"))
			{
				unsigned short texCoordCount = 0;
				for (unsigned short j = 0; j < subBatch->vertexData->vertexDeclaration->getElementCount(); ++j) 
				{
					const VertexElement *el = subBatch->vertexData->vertexDeclaration->getElement(j);
					if (el->getSemantic() == VES_TEXTURE_COORDINATES) 
					{
						++ texCoordCount;
					}
				}

				if (lightingEnabled)
				{
					 vertexProgSource +=
						"uniform vec4 objSpaceLight; \n"
						"uniform vec4 lightDiffuse; \n"
						"uniform vec4 lightAmbient; \n";
				}

				if (fadeEnabled)
				{
					 vertexProgSource +=
						"uniform vec3 camPos; \n"
						"uniform float fadeGap; \n"
						"uniform float invisibleDist; \n";
				}

				vertexProgSource +=
					"uniform float time; \n"
					"void main() \n"
					"{ \n";

				int texNum = 0;

				for (unsigned short i = 0; i < subBatch->vertexData->vertexDeclaration->getElementCount(); ++i)
				{
					const VertexElement *el = subBatch->vertexData->vertexDeclaration->getElement(i);
					if (el->getSemantic() == VES_TEXTURE_COORDINATES)
					{
						if (el->getIndex() == texCoordCount - 2)
						{
							vertexProgSource += 
								"	vec4 params = gl_MultiTexCoord" + StringConverter::toString(texCoordCount-2) + "; \n";
						}
						else
						{
							if (el->getIndex() == texCoordCount - 1)
							{
								vertexProgSource += 
									"	vec4 originPos = gl_MultiTexCoord" + StringConverter::toString(texCoordCount-1) + "; \n";
							}
							else
							{
								vertexProgSource += 
								"	gl_TexCoord[" + StringConverter::toString(texNum) + "]	= gl_MultiTexCoord" + StringConverter::toString(texNum) + "; \n";
							}
							++texNum;
						}
					}
				}

				if (lightingEnabled)
				{
					//Perform lighting calculations (no specular)
					vertexProgSource +=
						"	vec3 light = normalize(objSpaceLight.xyz - (gl_Vertex.xyz * objSpaceLight.w)); \n"
						"	float diffuseFactor = max(dot(gl_Normal.xyz, light), 0.0); \n";

					if (subBatch->vertexData->vertexDeclaration->findElementBySemantic(VES_DIFFUSE) != NULL)
					{
						vertexProgSource +=
							"	gl_FrontColor = (lightAmbient + diffuseFactor * lightDiffuse) * gl_Color; \n";
					}
					else
					{
						vertexProgSource +=
							"	gl_FrontColor = (lightAmbient + diffuseFactor * lightDiffuse); \n";
					}
				}
				else
				{
					if (subBatch->vertexData->vertexDeclaration->findElementBySemantic(VES_DIFFUSE) != NULL)
					{
						vertexProgSource += "	gl_FrontColor = gl_Color; \n";
					}
					else
					{
						vertexProgSource += "	gl_FrontColor = vec4(1.0, 1.0, 1.0, 1.0); \n";
					}
				}

				if (fadeEnabled)
				{
					//Fade out in the distance
					vertexProgSource +=
						"	float dist = distance(camPos.xz, gl_Vertex.xz);	\n"
						"	gl_FrontColor.a *= (invisibleDist - dist) / fadeGap; \n";
				}

				vertexProgSource +=
					"	float radiusCoeff = params.x; \n"
					"	float heightCoeff = params.y; \n"
					"	float factorX = params.z; \n"
					"	float factorY = params.w; \n"
					"	vec4 tmpPos = gl_Vertex; \n"
					
					/* 
					2 different methods are used to for the sin calculation :
					- the first one gives a better effect but at the cost of a few fps because of the 2 sines
					- the second one uses less ressources but is a bit less realistic

					a sin approximation could be use to optimize performances
					*/
	#if 1
					"	tmpPos.y += sin(time + originPos.z + tmpPos.y + tmpPos.x) * radiusCoeff * radiusCoeff * factorY; \n"
					"	tmpPos.x += sin(time + originPos.z ) * heightCoeff * heightCoeff * factorX; \n"
	#else
	 				
					"	float sinval = sin(time + originPos.z ); \n"
					"	tmpPos.y += sinval * radiusCoeff * radiusCoeff * factorY; \n"
					"	tmpPos.x += sinval * heightCoeff * heightCoeff * factorX; \n"
	#endif
					"	gl_Position = gl_ModelViewProjectionMatrix * tmpPos; \n"
					"	gl_FogFragCoord = gl_Position.z; \n"
					"}";
			}

			// test for shader source
			//std::ofstream shaderOutput;
			//shaderOutput.open((vertexProgName+std::string(".cg")).c_str());
			//shaderOutput << vertexProgSource;
			//shaderOutput.close();

			// end test for shader source

			HighLevelGpuProgramPtr vertexShader = HighLevelGpuProgramManager::getSingleton().createProgram(
				vertexProgName,
				ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
				shaderLanguage, GPT_VERTEX_PROGRAM);

			vertexShader->setSource(vertexProgSource);

			if (shaderLanguage == "hlsl")
			{
				vertexShader->setParameter("target", "vs_1_1");
				vertexShader->setParameter("entry_point", "main");
			}
			else if(shaderLanguage == "cg")
			{
				vertexShader->setParameter("profiles", "vs_1_1 arbvp1");
				vertexShader->setParameter("entry_point", "main");
			}
			// GLSL can only have one entry point "main".

			vertexShader->load();
		}

		//Now that the shader is ready to be applied, apply it
		StringUtil::StrStreamType materialSignature;
		materialSignature << "BatchMat|";
		materialSignature << mat->getName() << "|";
		if (fadeEnabled){
			materialSignature << visibleDist << "|";
			materialSignature << invisibleDist << "|";
		}

		//Search for the desired material
		MaterialPtr generatedMaterial = MaterialManager::getSingleton().getByName(materialSignature.str());
		if (generatedMaterial.isNull()){
			//Clone the material
			generatedMaterial = mat->clone(materialSignature.str());

			//And apply the fade shader
			for (unsigned short t = 0; t < generatedMaterial->getNumTechniques(); ++t){
				Technique *tech = generatedMaterial->getTechnique(t);
				for (unsigned short p = 0; p < tech->getNumPasses(); ++p){
					Pass *pass = tech->getPass(p);

					//Setup vertex program
					if (pass->getVertexProgramName() == "")
						pass->setVertexProgram(vertexProgName);

					try{
						GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();

						if (lightingEnabled) {
							params->setNamedAutoConstant("objSpaceLight", GpuProgramParameters::ACT_LIGHT_POSITION_OBJECT_SPACE);
							params->setNamedAutoConstant("lightDiffuse", GpuProgramParameters::ACT_DERIVED_LIGHT_DIFFUSE_COLOUR);
							params->setNamedAutoConstant("lightAmbient", GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR);
							//params->setNamedAutoConstant("matAmbient", GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR);
						}

						params->setNamedConstantFromTime("time", 1);

						if(shaderLanguage.compare("glsl"))
						{
							//glsl can use the built in gl_ModelViewProjectionMatrix
							params->setNamedAutoConstant("worldViewProj", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
						}

						if (fadeEnabled){
							params->setNamedAutoConstant("camPos", GpuProgramParameters::ACT_CAMERA_POSITION_OBJECT_SPACE);

							//Set fade ranges
							params->setNamedAutoConstant("invisibleDist", GpuProgramParameters::ACT_CUSTOM);
							params->setNamedConstant("invisibleDist", invisibleDist);

							params->setNamedAutoConstant("fadeGap", GpuProgramParameters::ACT_CUSTOM);
							params->setNamedConstant("fadeGap", invisibleDist - visibleDist);

							if (pass->getAlphaRejectFunction() == CMPF_ALWAYS_PASS)
								pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
						}
					}
					catch (Ogre::Exception e)
					{
						// test for shader source	
						std::ofstream shaderOutput;
						shaderOutput.open("exception.log");
						shaderOutput << e.getDescription();
						shaderOutput.close();
					}
					catch (...) {
						OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Error configuring batched geometry transitions. If you're using materials with custom vertex shaders, they will need to implement fade transitions to be compatible with BatchPage.", "BatchPage::_updateShaders()");
					}
				}
			}

		}

		//Apply the material
		subBatch->setMaterial(generatedMaterial);
	}

}
}
