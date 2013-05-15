#include "DotSceneLoader.h"
#include <Ogre.h>
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>
#include <Terrain/OgreTerrainMaterialGeneratorA.h>

#include "PagedGeometry.h"
#include "BatchPage.h"
#include "ImpostorPage.h"
#include "TreeLoader3D.h"


#pragma warning(disable:4390)
#pragma warning(disable:4305)


using namespace Forests;

DotSceneLoader::DotSceneLoader(Ogre::uint32 queryflags) : mSceneMgr(0), mTerrainGroup(0) , mQueryflags(queryflags)
{
    mTerrainGlobalOptions = OGRE_NEW Ogre::TerrainGlobalOptions();
}


DotSceneLoader::~DotSceneLoader()
{
    if(mTerrainGroup)
    {
        OGRE_DELETE mTerrainGroup;
    }

    OGRE_DELETE mTerrainGlobalOptions;
}

void ParseStringVector(Ogre::String &str, Ogre::StringVector &list)
{
    list.clear();
    Ogre::StringUtil::trim(str,true,true);
    if(str == "") 
        return;

    int pos = str.find(";");
    while(pos != -1)
    {
        list.push_back(str.substr(0,pos));
        str.erase(0,pos + 1);
        pos = str.find(";");
    }
  
    if(str != "") 
        list.push_back(str);
}

void DotSceneLoader::parseDotScene(const Ogre::String &SceneName, const Ogre::String &groupName, Ogre::SceneManager *yourSceneMgr, Ogre::SceneNode *pAttachNode, const Ogre::String &sPrependNode)
{
    // set up shared object values
    m_sGroupName = groupName;
    mSceneMgr = yourSceneMgr;
    m_sPrependNode = sPrependNode;
    staticObjects.clear();
    dynamicObjects.clear();

    rapidxml::xml_document<> XMLDoc;    // character type defaults to char

    rapidxml::xml_node<>* XMLRoot;

    Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(SceneName, groupName );
    char* scene = strdup(stream->getAsString().c_str());
    XMLDoc.parse<0>(scene);

    // Grab the scene node
    XMLRoot = XMLDoc.first_node("scene");

    // Validate the File
    if( getAttrib(XMLRoot, "formatVersion", "") == "")
    {
        Ogre::LogManager::getSingleton().logMessage( "[DotSceneLoader] Error: Invalid .scene File. Missing <scene>" );
        delete scene;
        return;
    }

    // figure out where to attach any nodes we create
    mAttachNode = pAttachNode;
    if(!mAttachNode)
        mAttachNode = mSceneMgr->getRootSceneNode();

    // Process the scene
    processScene(XMLRoot);

    delete scene;
}

void DotSceneLoader::processScene(rapidxml::xml_node<>* XMLRoot)
{
    // Process the scene parameters
    Ogre::String version = getAttrib(XMLRoot, "formatVersion", "unknown");

    Ogre::String message = "[DotSceneLoader] Parsing dotScene file with version " + version;
    if(XMLRoot->first_attribute("ID"))
        message += ", id " + Ogre::String(XMLRoot->first_attribute("ID")->value());
    if(XMLRoot->first_attribute("sceneManager"))
        message += ", scene manager " + Ogre::String(XMLRoot->first_attribute("sceneManager")->value());
    if(XMLRoot->first_attribute("minOgreVersion"))
        message += ", min. Ogre version " + Ogre::String(XMLRoot->first_attribute("minOgreVersion")->value());
    if(XMLRoot->first_attribute("author"))
        message += ", author " + Ogre::String(XMLRoot->first_attribute("author")->value());

    Ogre::LogManager::getSingleton().logMessage(message);

    rapidxml::xml_node<>* pElement;

    // Process environment (?)
    pElement = XMLRoot->first_node("environment");
    if(pElement)
        processEnvironment(pElement);

    // Process nodes (?)
    pElement = XMLRoot->first_node("nodes");
    if(pElement)
        processNodes(pElement);

    // Process externals (?)
    pElement = XMLRoot->first_node("externals");
    if(pElement)
        processExternals(pElement);

    // Process userDataReference (?)
    pElement = XMLRoot->first_node("userDataReference");
    if(pElement)
        processUserDataReference(pElement);

    // Process octree (?)
    pElement = XMLRoot->first_node("octree");
    if(pElement)
        processOctree(pElement);

    // Process light (?)
    pElement = XMLRoot->first_node("light");
    while(pElement)
    {
        processLight(pElement);
        pElement = pElement->next_sibling("light");
    }

    // Process camera (?)
    pElement = XMLRoot->first_node("camera");
    while(pElement)
    {
        processCamera(pElement);
        pElement = pElement->next_sibling("camera");
    }

    // Process terrain (?)
    pElement = XMLRoot->first_node("terrain");
    if(pElement)
        processTerrain(pElement);
}

void DotSceneLoader::processNodes(rapidxml::xml_node<>* XMLNode)
{
    rapidxml::xml_node<>* pElement;

    // Process node (*)
    pElement = XMLNode->first_node("node");
    while(pElement)
    {
        processNode(pElement);
        pElement = pElement->next_sibling("node");
    }

    // Process position (?)
    pElement = XMLNode->first_node("position");
    if(pElement)
    {
        mAttachNode->setPosition(parseVector3(pElement));
        mAttachNode->setInitialState();
    }

    // Process rotation (?)
    pElement = XMLNode->first_node("rotation");
    if(pElement)
    {
        mAttachNode->setOrientation(parseQuaternion(pElement));
        mAttachNode->setInitialState();
    }

    // Process scale (?)
    pElement = XMLNode->first_node("scale");
    if(pElement)
    {
        mAttachNode->setScale(parseVector3(pElement));
        mAttachNode->setInitialState();
    }
}

void DotSceneLoader::processExternals(rapidxml::xml_node<>* XMLNode)
{
    //! @todo Implement this
}

void DotSceneLoader::processEnvironment(rapidxml::xml_node<>* XMLNode)
{
    rapidxml::xml_node<>* pElement;

    // Process camera (?)
    pElement = XMLNode->first_node("camera");
    if(pElement)
        processCamera(pElement);

    // Process fog (?)
    pElement = XMLNode->first_node("fog");
    if(pElement)
        processFog(pElement);

    // Process skyBox (?)
    pElement = XMLNode->first_node("skyBox");
    if(pElement)
        processSkyBox(pElement);

    // Process skyDome (?)
    pElement = XMLNode->first_node("skyDome");
    if(pElement)
        processSkyDome(pElement);

    // Process skyPlane (?)
    pElement = XMLNode->first_node("skyPlane");
    if(pElement)
        processSkyPlane(pElement);

    // Process clipping (?)
    pElement = XMLNode->first_node("clipping");
    if(pElement)
        processClipping(pElement);

    // Process colourAmbient (?)
    pElement = XMLNode->first_node("colourAmbient");
    if(pElement)
        mSceneMgr->setAmbientLight(parseColour(pElement));

    // Process colourBackground (?)
    //! @todo Set the background colour of all viewports (RenderWindow has to be provided then)
    pElement = XMLNode->first_node("colourBackground");
    if(pElement)
        ;//mSceneMgr->set(parseColour(pElement));

    // Process userDataReference (?)
    pElement = XMLNode->first_node("userDataReference");
    if(pElement)
        processUserDataReference(pElement);
}

void DotSceneLoader::processTerrain(rapidxml::xml_node<>* XMLNode)
{
    Ogre::Real worldSize = getAttribReal(XMLNode, "worldSize");
    int mapSize = Ogre::StringConverter::parseInt(XMLNode->first_attribute("mapSize")->value());
    bool colourmapEnabled = getAttribBool(XMLNode, "colourmapEnabled");
    int colourMapTextureSize = Ogre::StringConverter::parseInt(XMLNode->first_attribute("colourMapTextureSize")->value());
    int compositeMapDistance = Ogre::StringConverter::parseInt(XMLNode->first_attribute("tuningCompositeMapDistance")->value());
    int maxPixelError = Ogre::StringConverter::parseInt(XMLNode->first_attribute("tuningMaxPixelError")->value());
    
    Ogre::Vector3 lightdir(0, -0.3, 0.75);
    lightdir.normalise();
    Ogre::Light* l = mSceneMgr->createLight("tstLight");
    l->setType(Ogre::Light::LT_DIRECTIONAL);
    l->setDirection(lightdir);
    l->setDiffuseColour(Ogre::ColourValue(1.0, 1.0, 1.0));
    l->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.6, 0.6, 0.6));

    mTerrainGlobalOptions->setMaxPixelError((Ogre::Real)maxPixelError);
    mTerrainGlobalOptions->setCompositeMapDistance((Ogre::Real)compositeMapDistance);
    mTerrainGlobalOptions->setLightMapDirection(lightdir);
    mTerrainGlobalOptions->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
    mTerrainGlobalOptions->setCompositeMapDiffuse(l->getDiffuseColour());

    mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(mSceneMgr, Ogre::Terrain::ALIGN_X_Z, mapSize, worldSize);
    mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);

    mTerrainGroup->setResourceGroup(m_sGroupName);

    rapidxml::xml_node<>* pElement;
    rapidxml::xml_node<>* pPageElement;

    // Process terrain pages (*)
    pElement = XMLNode->first_node("terrainPages");
    if(pElement)
    {
        pPageElement = pElement->first_node("terrainPage");
        while(pPageElement)
        {
            processTerrainPage(pPageElement);
            pPageElement = pPageElement->next_sibling("terrainPage");
        }
    }
    mTerrainGroup->loadAllTerrains(true);

    mTerrainGroup->freeTemporaryResources();
    //mTerrain->setPosition(mTerrainPosition);
}

void DotSceneLoader::processTerrainPage(rapidxml::xml_node<>* XMLNode)
{
    Ogre::String name = getAttrib(XMLNode, "name");
    int pageX = Ogre::StringConverter::parseInt(XMLNode->first_attribute("pageX")->value());
    int pageY = Ogre::StringConverter::parseInt(XMLNode->first_attribute("pageY")->value());

    if (Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), name))
    {
        mTerrainGroup->defineTerrain(pageX, pageY, name);
    }
}

void DotSceneLoader::processUserDataReference(rapidxml::xml_node<>* XMLNode, Ogre::SceneNode *pParent)
{
    //! @todo Implement this
}

void DotSceneLoader::processOctree(rapidxml::xml_node<>* XMLNode)
{
    //! @todo Implement this
}

void DotSceneLoader::processLight(rapidxml::xml_node<>* XMLNode, Ogre::SceneNode *pParent)
{
    // Process attributes
    Ogre::String name = getAttrib(XMLNode, "name");
    Ogre::String id = getAttrib(XMLNode, "id");

    // Create the light
    Ogre::Light *pLight = mSceneMgr->createLight(name);
    if(pParent)
        pParent->attachObject(pLight);

    Ogre::String sValue = getAttrib(XMLNode, "type");
    if(sValue == "point")
        pLight->setType(Ogre::Light::LT_POINT);
    else if(sValue == "directional")
        pLight->setType(Ogre::Light::LT_DIRECTIONAL);
    else if(sValue == "spot")
        pLight->setType(Ogre::Light::LT_SPOTLIGHT);
    else if(sValue == "radPoint")
        pLight->setType(Ogre::Light::LT_POINT);

    pLight->setVisible(getAttribBool(XMLNode, "visible", true));
    pLight->setCastShadows(getAttribBool(XMLNode, "castShadows", true));

    rapidxml::xml_node<>* pElement;

    // Process position (?)
    pElement = XMLNode->first_node("position");
    if(pElement)
        pLight->setPosition(parseVector3(pElement));

    // Process normal (?)
    pElement = XMLNode->first_node("normal");
    if(pElement)
        pLight->setDirection(parseVector3(pElement));

    pElement = XMLNode->first_node("directionVector");
    if(pElement)
    {
        pLight->setDirection(parseVector3(pElement));
        mLightDirection = parseVector3(pElement);
    }

    // Process colourDiffuse (?)
    pElement = XMLNode->first_node("colourDiffuse");
    if(pElement)
        pLight->setDiffuseColour(parseColour(pElement));

    // Process colourSpecular (?)
    pElement = XMLNode->first_node("colourSpecular");
    if(pElement)
        pLight->setSpecularColour(parseColour(pElement));

    if(sValue != "directional")
    {
        // Process lightRange (?)
        pElement = XMLNode->first_node("lightRange");
        if(pElement)
            processLightRange(pElement, pLight);

        // Process lightAttenuation (?)
        pElement = XMLNode->first_node("lightAttenuation");
        if(pElement)
            processLightAttenuation(pElement, pLight);
    }
    // Process userDataReference (?)
    pElement = XMLNode->first_node("userDataReference");
    if(pElement)
        ;//processUserDataReference(pElement, pLight);
}

void DotSceneLoader::processCamera(rapidxml::xml_node<>* XMLNode, Ogre::SceneNode *pParent)
{
    // Process attributes
    Ogre::String name = getAttrib(XMLNode, "name");
    Ogre::String id = getAttrib(XMLNode, "id");
    Ogre::Real fov = getAttribReal(XMLNode, "fov", 45);
    Ogre::Real aspectRatio = getAttribReal(XMLNode, "aspectRatio", 1.3333);
    Ogre::String projectionType = getAttrib(XMLNode, "projectionType", "perspective");

    // Create the camera
    Ogre::Camera *pCamera = mSceneMgr->createCamera(name);
    
    //TODO: make a flag or attribute indicating whether or not the camera should be attached to any parent node.
    //if(pParent)
    //    pParent->attachObject(pCamera);

    // Set the field-of-view
    //! @todo Is this always in degrees?
    //pCamera->setFOVy(Ogre::Degree(fov));

    // Set the aspect ratio
    //pCamera->setAspectRatio(aspectRatio);
    
    // Set the projection type
    if(projectionType == "perspective")
        pCamera->setProjectionType(Ogre::PT_PERSPECTIVE);
    else if(projectionType == "orthographic")
        pCamera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);

    rapidxml::xml_node<>* pElement;

    // Process clipping (?)
    pElement = XMLNode->first_node("clipping");
    if(pElement)
    {
        Ogre::Real nearDist = getAttribReal(pElement, "near");
        pCamera->setNearClipDistance(nearDist);

        Ogre::Real farDist =  getAttribReal(pElement, "far");
        pCamera->setFarClipDistance(farDist);
    }

    // Process position (?)
    pElement = XMLNode->first_node("position");
    if(pElement)
        pCamera->setPosition(parseVector3(pElement));

    // Process rotation (?)
    pElement = XMLNode->first_node("rotation");
    if(pElement)
        pCamera->setOrientation(parseQuaternion(pElement));

    // Process normal (?)
    pElement = XMLNode->first_node("normal");
    if(pElement)
        ;//!< @todo What to do with this element?

    // Process lookTarget (?)
    pElement = XMLNode->first_node("lookTarget");
    if(pElement)
        ;//!< @todo Implement the camera look target

    // Process trackTarget (?)
    pElement = XMLNode->first_node("trackTarget");
    if(pElement)
        ;//!< @todo Implement the camera track target

    // Process userDataReference (?)
    pElement = XMLNode->first_node("userDataReference");
    if(pElement)
        ;//!< @todo Implement the camera user data reference
/*
    // construct a scenenode is no parent
    if(!pParent)
    {
        Ogre::SceneNode* pNode = mAttachNode->createChildSceneNode(name);
        pNode->setPosition(pCamera->getPosition());
        pNode->setOrientation(pCamera->getOrientation());
        pNode->scale(1,1,1);
    }
*/
}

void DotSceneLoader::processNode(rapidxml::xml_node<>* XMLNode, Ogre::SceneNode *pParent)
{
    // Construct the node's name
    Ogre::String name = m_sPrependNode + getAttrib(XMLNode, "name");

    // Create the scene node
    Ogre::SceneNode *pNode;
    if(name.empty())
    {
        // Let Ogre choose the name
        if(pParent)
            pNode = pParent->createChildSceneNode();
        else
            pNode = mAttachNode->createChildSceneNode();
    }
    else
    {
        // Provide the name
        if(pParent)
            pNode = pParent->createChildSceneNode(name);
        else
            pNode = mAttachNode->createChildSceneNode(name);
    }

    // Process other attributes
    Ogre::String id = getAttrib(XMLNode, "id");
    bool isTarget = getAttribBool(XMLNode, "isTarget");

    rapidxml::xml_node<>* pElement;

    // Process position (?)
    pElement = XMLNode->first_node("position");
    if(pElement)
    {
        pNode->setPosition(parseVector3(pElement));
        pNode->setInitialState();
    }

    // Process rotation (?)
    pElement = XMLNode->first_node("rotation");
    if(pElement)
    {
        pNode->setOrientation(parseQuaternion(pElement));
        pNode->setInitialState();
    }

    // Process scale (?)
    pElement = XMLNode->first_node("scale");
    if(pElement)
    {
        pNode->setScale(parseVector3(pElement));
        pNode->setInitialState();
    }

    // Process lookTarget (?)
    pElement = XMLNode->first_node("lookTarget");
    if(pElement)
        processLookTarget(pElement, pNode);

    // Process trackTarget (?)
    pElement = XMLNode->first_node("trackTarget");
    if(pElement)
        processTrackTarget(pElement, pNode);

    // Process node (*)
    pElement = XMLNode->first_node("node");
    while(pElement)
    {
        processNode(pElement, pNode);
        pElement = pElement->next_sibling("node");
    }

    // Process entity (*)
    pElement = XMLNode->first_node("entity");
    while(pElement)
    {
        processEntity(pElement, pNode);
        pElement = pElement->next_sibling("entity");
    }

    // Process light (*)
    //pElement = XMLNode->first_node("light");
    //while(pElement)
    //{
    //    processLight(pElement, pNode);
    //    pElement = pElement->next_sibling("light");
    //}

    // Process camera (*)
    pElement = XMLNode->first_node("camera");
    while(pElement)
    {
        processCamera(pElement, pNode);
        pElement = pElement->next_sibling("camera");
    }

    // Process particleSystem (*)
    pElement = XMLNode->first_node("particleSystem");
    while(pElement)
    {
        processParticleSystem(pElement, pNode);
        pElement = pElement->next_sibling("particleSystem");
    }

    // Process billboardSet (*)
    pElement = XMLNode->first_node("billboardSet");
    while(pElement)
    {
        processBillboardSet(pElement, pNode);
        pElement = pElement->next_sibling("billboardSet");
    }

    // Process plane (*)
    pElement = XMLNode->first_node("plane");
    while(pElement)
    {
        processPlane(pElement, pNode);
        pElement = pElement->next_sibling("plane");
    }

    // Process userDataReference (?)
    pElement = XMLNode->first_node("userDataReference");
    if(pElement)
        processUserDataReference(pElement, pNode);

    // Process entity (*)
    pElement = XMLNode->first_node("pagedgeometry");
    while(pElement)
    {
        processPagedGeometry(pElement, pNode);
        pElement = pElement->next_sibling("pagedgeometry");
    }
}

void DotSceneLoader::processLookTarget(rapidxml::xml_node<>* XMLNode, Ogre::SceneNode *pParent)
{
    //! @todo Is this correct? Cause I don't have a clue actually

    // Process attributes
    Ogre::String nodeName = getAttrib(XMLNode, "nodeName");

    Ogre::Node::TransformSpace relativeTo = Ogre::Node::TS_PARENT;
    Ogre::String sValue = getAttrib(XMLNode, "relativeTo");
    if(sValue == "local")
        relativeTo = Ogre::Node::TS_LOCAL;
    else if(sValue == "parent")
        relativeTo = Ogre::Node::TS_PARENT;
    else if(sValue == "world")
        relativeTo = Ogre::Node::TS_WORLD;

    rapidxml::xml_node<>* pElement;

    // Process position (?)
    Ogre::Vector3 position;
    pElement = XMLNode->first_node("position");
    if(pElement)
        position = parseVector3(pElement);

    // Process localDirection (?)
    Ogre::Vector3 localDirection = Ogre::Vector3::NEGATIVE_UNIT_Z;
    pElement = XMLNode->first_node("localDirection");
    if(pElement)
        localDirection = parseVector3(pElement);

    // Setup the look target
    try
    {
        if(!nodeName.empty())
        {
            Ogre::SceneNode *pLookNode = mSceneMgr->getSceneNode(nodeName);
            position = pLookNode->_getDerivedPosition();
        }

        pParent->lookAt(position, relativeTo, localDirection);
    }
    catch(Ogre::Exception &/*e*/)
    {
        Ogre::LogManager::getSingleton().logMessage("[DotSceneLoader] Error processing a look target!");
    }
}

void DotSceneLoader::processTrackTarget(rapidxml::xml_node<>* XMLNode, Ogre::SceneNode *pParent)
{
    // Process attributes
    Ogre::String nodeName = getAttrib(XMLNode, "nodeName");

    rapidxml::xml_node<>* pElement;

    // Process localDirection (?)
    Ogre::Vector3 localDirection = Ogre::Vector3::NEGATIVE_UNIT_Z;
    pElement = XMLNode->first_node("localDirection");
    if(pElement)
        localDirection = parseVector3(pElement);

    // Process offset (?)
    Ogre::Vector3 offset = Ogre::Vector3::ZERO;
    pElement = XMLNode->first_node("offset");
    if(pElement)
        offset = parseVector3(pElement);

    // Setup the track target
    try
    {
        Ogre::SceneNode *pTrackNode = mSceneMgr->getSceneNode(nodeName);
        pParent->setAutoTracking(true, pTrackNode, localDirection, offset);
    }
    catch(Ogre::Exception &/*e*/)
    {
        Ogre::LogManager::getSingleton().logMessage("[DotSceneLoader] Error processing a track target!");
    }
}

void DotSceneLoader::processEntity(rapidxml::xml_node<>* XMLNode, Ogre::SceneNode *pParent)
{
    // Process attributes
    Ogre::String name = getAttrib(XMLNode, "name");
    Ogre::String id = getAttrib(XMLNode, "id");
    Ogre::String meshFile = getAttrib(XMLNode, "meshFile");
    Ogre::String materialFile = getAttrib(XMLNode, "materialFile");
    bool isStatic = getAttribBool(XMLNode, "static", false);;
    bool castShadows = getAttribBool(XMLNode, "castShadows", true);

    // TEMP: Maintain a list of static and dynamic objects
    if(isStatic)
        staticObjects.push_back(name);
    else
        dynamicObjects.push_back(name);

    rapidxml::xml_node<>* pElement;

    // Process vertexBuffer (?)
    pElement = XMLNode->first_node("vertexBuffer");
    if(pElement)
        ;//processVertexBuffer(pElement);

    // Process indexBuffer (?)
    pElement = XMLNode->first_node("indexBuffer");
    if(pElement)
        ;//processIndexBuffer(pElement);

    // Create the entity
    Ogre::Entity *pEntity = 0;
    try
    {
        Ogre::MeshManager::getSingleton().load(meshFile, m_sGroupName);
        pEntity = mSceneMgr->createEntity(name, meshFile);
        pEntity->setCastShadows(castShadows);
        pParent->attachObject(pEntity);

		pEntity->setQueryFlags(mQueryflags);
		
        if(!materialFile.empty())
            pEntity->setMaterialName(materialFile);
		
		// Process subentity (*)
		/* if materials defined within subentities, those
		   materials will be used instead of the materialFile */
		pElement = XMLNode->first_node("subentities");

		if(pElement)
		{
			processSubEntity(pElement, pEntity);
		}else{
			// if the .scene file contains the subentites without
			// the <subentities> </subentities>
			processSubEntity(XMLNode, pEntity);
		}
    }
    catch(Ogre::Exception &/*e*/)
    {
        Ogre::LogManager::getSingleton().logMessage("[DotSceneLoader] Error loading an entity!");
    }

    // Process userDataReference (?)
    pElement = XMLNode->first_node("userDataReference");
    if(pElement)
        processUserDataReference(pElement, pEntity);

    
}

void DotSceneLoader::processSubEntity(rapidxml::xml_node<>* XMLNode, Ogre::Entity *pEntity){
    rapidxml::xml_node<>* pElement;
	int index = 0;
	Ogre::String materialName;
	Ogre::String sIndex;

    // Process subentity
    pElement = XMLNode->first_node("subentity");
	
	while(pElement){
		
		sIndex.clear();
		materialName.clear();

		sIndex = getAttrib(pElement, "index");				// submesh index
		materialName = getAttrib(pElement, "materialName");	// new material for submesh
		
		if(!sIndex.empty() && !materialName.empty()){
			
			index = Ogre::StringConverter::parseInt(sIndex);
			try{
				pEntity->getSubEntity(index)->setMaterialName(materialName);
			} catch (...){
				Ogre::LogManager::getSingleton().logMessage("[DotSceneLoader] Subentity material index invalid!");
			}
		}
		pElement = pElement->next_sibling("subentity");
	}
}

void DotSceneLoader::processParticleSystem(rapidxml::xml_node<>* XMLNode, Ogre::SceneNode *pParent)
{
    // Process attributes
    Ogre::String name = getAttrib(XMLNode, "name");
    Ogre::String id = getAttrib(XMLNode, "id");
    Ogre::String file = getAttrib(XMLNode, "file");

    // Create the particle system
    try
    {
        Ogre::ParticleSystem *pParticles = mSceneMgr->createParticleSystem(name, file);
        pParent->attachObject(pParticles);
    }
    catch(Ogre::Exception &/*e*/)
    {
        Ogre::LogManager::getSingleton().logMessage("[DotSceneLoader] Error creating a particle system!");
    }
}

void DotSceneLoader::processBillboardSet(rapidxml::xml_node<>* XMLNode, Ogre::SceneNode *pParent)
{
    //! @todo Implement this
}

void DotSceneLoader::processPlane(rapidxml::xml_node<>* XMLNode, Ogre::SceneNode *pParent)
{
    Ogre::String name = getAttrib(XMLNode, "name");
    Ogre::Real distance = getAttribReal(XMLNode, "distance");
    Ogre::Real width = getAttribReal(XMLNode, "width");
    Ogre::Real height = getAttribReal(XMLNode, "height");
    int xSegments = Ogre::StringConverter::parseInt(getAttrib(XMLNode, "xSegments"));
    int ySegments = Ogre::StringConverter::parseInt(getAttrib(XMLNode, "ySegments"));
    int numTexCoordSets = Ogre::StringConverter::parseInt(getAttrib(XMLNode, "numTexCoordSets"));
    Ogre::Real uTile = getAttribReal(XMLNode, "uTile");
    Ogre::Real vTile = getAttribReal(XMLNode, "vTile");
    Ogre::String material = getAttrib(XMLNode, "material");
    bool hasNormals = getAttribBool(XMLNode, "hasNormals");
    Ogre::Vector3 normal = parseVector3(XMLNode->first_node("normal"));
    Ogre::Vector3 up = parseVector3(XMLNode->first_node("upVector"));

    Ogre::Plane plane(normal, distance);
    Ogre::MeshPtr res = Ogre::MeshManager::getSingletonPtr()->createPlane(
                        name + "mesh", m_sGroupName, plane, width, height, xSegments, ySegments, hasNormals,
    numTexCoordSets, uTile, vTile, up);
    Ogre::Entity* ent = mSceneMgr->createEntity(name, name + "mesh");
	ent->setQueryFlags(mQueryflags);
    ent->setMaterialName(material);

    pParent->attachObject(ent);
}

struct PGInstanceInfo
{
    Ogre::Vector3 pos;
    Ogre::Real    scale;
    Ogre::Real    yaw;
};

typedef std::vector<PGInstanceInfo> PGInstanceList;

void DotSceneLoader::processPagedGeometry(rapidxml::xml_node<>* XMLNode, Ogre::SceneNode *pParent)
{
    Ogre::String filename = "../Projects/SampleScene3/" + getAttrib(XMLNode, "fileName");
    Ogre::String model = getAttrib(XMLNode, "model");
    Ogre::Real pagesize = getAttribReal(XMLNode, "pageSize");
    Ogre::Real batchdistance = getAttribReal(XMLNode, "batchDistance");
    Ogre::Real impostordistance = getAttribReal(XMLNode, "impostorDistance");
    Ogre::Vector4 bounds = Ogre::StringConverter::parseVector4(getAttrib(XMLNode, "bounds"));

    PagedGeometry *mPGHandle = new PagedGeometry();
    mPGHandle->setCamera(mSceneMgr->getCameraIterator().begin()->second);
    mPGHandle->setPageSize(pagesize);
    mPGHandle->setInfinite();

    mPGHandle->addDetailLevel<BatchPage>(batchdistance,0);
    mPGHandle->addDetailLevel<ImpostorPage>(impostordistance,0);

    TreeLoader3D *mHandle = new TreeLoader3D(mPGHandle, Forests::TBounds(bounds.x, bounds.y, bounds.z, bounds.w));
    mPGHandle->setPageLoader(mHandle);

    mPGHandles.push_back(mPGHandle);
    mTreeHandles.push_back(mHandle);

    std::ifstream stream(filename.c_str());

    if(!stream.is_open())
        return;

    Ogre::StringVector list;

    char res[128];

    PGInstanceList mInstanceList;

    while(!stream.eof())
    {
        stream.getline(res, 128);
        Ogre::String resStr(res);

        ParseStringVector(resStr, list);

        if(list.size() == 3)
        {
            PGInstanceInfo info;
            
            info.pos = Ogre::StringConverter::parseVector3(list[0]);
            info.scale = Ogre::StringConverter::parseReal(list[1]);
            info.yaw = Ogre::StringConverter::parseReal(list[2]);

            mInstanceList.push_back(info);
        }
        else if(list.size() == 4)
        {
            PGInstanceInfo info;
            
            info.pos = Ogre::StringConverter::parseVector3(list[1]);
            info.scale = Ogre::StringConverter::parseReal(list[2]);
            info.yaw = Ogre::StringConverter::parseReal(list[3]);

            mInstanceList.push_back(info);
        }
    }

    stream.close();

    if(model != "")
    {
        Ogre::Entity *mEntityHandle = mSceneMgr->createEntity(model + ".mesh");
		mEntityHandle->setQueryFlags(mQueryflags);
        PGInstanceList::iterator it = mInstanceList.begin();

        while(it != mInstanceList.end())
        {
            mHandle->addTree(mEntityHandle, it->pos, Ogre::Degree(it->yaw), it->scale);

            it++;
        }
    }
}

void DotSceneLoader::processFog(rapidxml::xml_node<>* XMLNode)
{
    // Process attributes
    Ogre::Real expDensity = getAttribReal(XMLNode, "density", 0.001);
    Ogre::Real linearStart = getAttribReal(XMLNode, "start", 0.0);
    Ogre::Real linearEnd = getAttribReal(XMLNode, "end", 1.0);

    Ogre::FogMode mode = Ogre::FOG_NONE;
    Ogre::String sMode = getAttrib(XMLNode, "mode");
    if(sMode == "none")
        mode = Ogre::FOG_NONE;
    else if(sMode == "exp")
        mode = Ogre::FOG_EXP;
    else if(sMode == "exp2")
        mode = Ogre::FOG_EXP2;
    else if(sMode == "linear")
        mode = Ogre::FOG_LINEAR;
    else
        mode = (Ogre::FogMode)Ogre::StringConverter::parseInt(sMode);

    rapidxml::xml_node<>* pElement;

    // Process colourDiffuse (?)
    Ogre::ColourValue colourDiffuse = Ogre::ColourValue::White;
    pElement = XMLNode->first_node("colour");
    if(pElement)
        colourDiffuse = parseColour(pElement);

    // Setup the fog
    mSceneMgr->setFog(mode, colourDiffuse, expDensity, linearStart, linearEnd);
}

void DotSceneLoader::processSkyBox(rapidxml::xml_node<>* XMLNode)
{
    // Process attributes
    Ogre::String material = getAttrib(XMLNode, "material", "BaseWhite");
    Ogre::Real distance = getAttribReal(XMLNode, "distance", 5000);
    bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);
    bool active = getAttribBool(XMLNode, "active", false);
    if(!active)
        return;

    rapidxml::xml_node<>* pElement;

    // Process rotation (?)
    Ogre::Quaternion rotation = Ogre::Quaternion::IDENTITY;
    pElement = XMLNode->first_node("rotation");
    if(pElement)
        rotation = parseQuaternion(pElement);

    // Setup the sky box
    mSceneMgr->setSkyBox(true, material, distance, drawFirst, rotation, m_sGroupName);
}

void DotSceneLoader::processSkyDome(rapidxml::xml_node<>* XMLNode)
{
    // Process attributes
    Ogre::String material = XMLNode->first_attribute("material")->value();
    Ogre::Real curvature = getAttribReal(XMLNode, "curvature", 10);
    Ogre::Real tiling = getAttribReal(XMLNode, "tiling", 8);
    Ogre::Real distance = getAttribReal(XMLNode, "distance", 4000);
    bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);
    bool active = getAttribBool(XMLNode, "active", false);
    if(!active)
        return;

    rapidxml::xml_node<>* pElement;

    // Process rotation (?)
    Ogre::Quaternion rotation = Ogre::Quaternion::IDENTITY;
    pElement = XMLNode->first_node("rotation");
    if(pElement)
        rotation = parseQuaternion(pElement);

    // Setup the sky dome
    mSceneMgr->setSkyDome(true, material, curvature, tiling, distance, drawFirst, rotation, 16, 16, -1, m_sGroupName);
}

void DotSceneLoader::processSkyPlane(rapidxml::xml_node<>* XMLNode)
{
    // Process attributes
    Ogre::String material = getAttrib(XMLNode, "material");
    Ogre::Real planeX = getAttribReal(XMLNode, "planeX", 0);
    Ogre::Real planeY = getAttribReal(XMLNode, "planeY", -1);
    Ogre::Real planeZ = getAttribReal(XMLNode, "planeX", 0);
    Ogre::Real planeD = getAttribReal(XMLNode, "planeD", 5000);
    Ogre::Real scale = getAttribReal(XMLNode, "scale", 1000);
    Ogre::Real bow = getAttribReal(XMLNode, "bow", 0);
    Ogre::Real tiling = getAttribReal(XMLNode, "tiling", 10);
    bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);

    // Setup the sky plane
    Ogre::Plane plane;
    plane.normal = Ogre::Vector3(planeX, planeY, planeZ);
    plane.d = planeD;
    mSceneMgr->setSkyPlane(true, plane, material, scale, tiling, drawFirst, bow, 1, 1, m_sGroupName);
}

void DotSceneLoader::processClipping(rapidxml::xml_node<>* XMLNode)
{
    //! @todo Implement this

    // Process attributes
    Ogre::Real fNear = getAttribReal(XMLNode, "near", 0);
    Ogre::Real fFar = getAttribReal(XMLNode, "far", 1);
}

void DotSceneLoader::processLightRange(rapidxml::xml_node<>* XMLNode, Ogre::Light *pLight)
{
    // Process attributes
    Ogre::Real inner = getAttribReal(XMLNode, "inner");
    Ogre::Real outer = getAttribReal(XMLNode, "outer");
    Ogre::Real falloff = getAttribReal(XMLNode, "falloff", 1.0);

    // Setup the light range
    pLight->setSpotlightRange(Ogre::Angle(inner), Ogre::Angle(outer), falloff);
}

void DotSceneLoader::processLightAttenuation(rapidxml::xml_node<>* XMLNode, Ogre::Light *pLight)
{
    // Process attributes
    Ogre::Real range = getAttribReal(XMLNode, "range");
    Ogre::Real constant = getAttribReal(XMLNode, "constant");
    Ogre::Real linear = getAttribReal(XMLNode, "linear");
    Ogre::Real quadratic = getAttribReal(XMLNode, "quadratic");

    // Setup the light attenuation
    pLight->setAttenuation(range, constant, linear, quadratic);
}


Ogre::String DotSceneLoader::getAttrib(rapidxml::xml_node<>* XMLNode, const Ogre::String &attrib, const Ogre::String &defaultValue)
{
    if(XMLNode->first_attribute(attrib.c_str()))
        return XMLNode->first_attribute(attrib.c_str())->value();
    else
        return defaultValue;
}

Ogre::Real DotSceneLoader::getAttribReal(rapidxml::xml_node<>* XMLNode, const Ogre::String &attrib, Ogre::Real defaultValue)
{
    if(XMLNode->first_attribute(attrib.c_str()))
        return Ogre::StringConverter::parseReal(XMLNode->first_attribute(attrib.c_str())->value());
    else
        return defaultValue;
}

bool DotSceneLoader::getAttribBool(rapidxml::xml_node<>* XMLNode, const Ogre::String &attrib, bool defaultValue)
{
    if(!XMLNode->first_attribute(attrib.c_str()))
        return defaultValue;

    if(Ogre::String(XMLNode->first_attribute(attrib.c_str())->value()) == "true")
        return true;

    return false;
}

Ogre::Vector3 DotSceneLoader::parseVector3(rapidxml::xml_node<>* XMLNode)
{
    return Ogre::Vector3(
        Ogre::StringConverter::parseReal(XMLNode->first_attribute("x")->value()),
        Ogre::StringConverter::parseReal(XMLNode->first_attribute("y")->value()),
        Ogre::StringConverter::parseReal(XMLNode->first_attribute("z")->value())
    );
}

Ogre::Quaternion DotSceneLoader::parseQuaternion(rapidxml::xml_node<>* XMLNode)
{
    //! @todo Fix this crap!

    Ogre::Quaternion orientation;

    if(XMLNode->first_attribute("qx"))
    {
        orientation.x = Ogre::StringConverter::parseReal(XMLNode->first_attribute("qx")->value());
        orientation.y = Ogre::StringConverter::parseReal(XMLNode->first_attribute("qy")->value());
        orientation.z = Ogre::StringConverter::parseReal(XMLNode->first_attribute("qz")->value());
        orientation.w = Ogre::StringConverter::parseReal(XMLNode->first_attribute("qw")->value());
    }
    if(XMLNode->first_attribute("qw"))
    {
        orientation.w = Ogre::StringConverter::parseReal(XMLNode->first_attribute("qw")->value());
        orientation.x = Ogre::StringConverter::parseReal(XMLNode->first_attribute("qx")->value());
        orientation.y = Ogre::StringConverter::parseReal(XMLNode->first_attribute("qy")->value());
        orientation.z = Ogre::StringConverter::parseReal(XMLNode->first_attribute("qz")->value());
    }
    else if(XMLNode->first_attribute("axisX"))
    {
        Ogre::Vector3 axis;
        axis.x = Ogre::StringConverter::parseReal(XMLNode->first_attribute("axisX")->value());
        axis.y = Ogre::StringConverter::parseReal(XMLNode->first_attribute("axisY")->value());
        axis.z = Ogre::StringConverter::parseReal(XMLNode->first_attribute("axisZ")->value());
        Ogre::Real angle = Ogre::StringConverter::parseReal(XMLNode->first_attribute("angle")->value());;
        orientation.FromAngleAxis(Ogre::Angle(angle), axis);
    }
    else if(XMLNode->first_attribute("angleX"))
    {
        Ogre::Vector3 axis;
        axis.x = Ogre::StringConverter::parseReal(XMLNode->first_attribute("angleX")->value());
        axis.y = Ogre::StringConverter::parseReal(XMLNode->first_attribute("angleY")->value());
        axis.z = Ogre::StringConverter::parseReal(XMLNode->first_attribute("angleZ")->value());
        //orientation.FromAxes(&axis);
        //orientation.F
    }
    else if(XMLNode->first_attribute("x"))
    {
        orientation.x = Ogre::StringConverter::parseReal(XMLNode->first_attribute("x")->value());
        orientation.y = Ogre::StringConverter::parseReal(XMLNode->first_attribute("y")->value());
        orientation.z = Ogre::StringConverter::parseReal(XMLNode->first_attribute("z")->value());
        orientation.w = Ogre::StringConverter::parseReal(XMLNode->first_attribute("w")->value());
    }
    else if(XMLNode->first_attribute("w"))
    {
        orientation.w = Ogre::StringConverter::parseReal(XMLNode->first_attribute("w")->value());
        orientation.x = Ogre::StringConverter::parseReal(XMLNode->first_attribute("x")->value());
        orientation.y = Ogre::StringConverter::parseReal(XMLNode->first_attribute("y")->value());
        orientation.z = Ogre::StringConverter::parseReal(XMLNode->first_attribute("z")->value());
    }

    return orientation;
}

Ogre::ColourValue DotSceneLoader::parseColour(rapidxml::xml_node<>* XMLNode)
{
    return Ogre::ColourValue(
        Ogre::StringConverter::parseReal(XMLNode->first_attribute("r")->value()),
        Ogre::StringConverter::parseReal(XMLNode->first_attribute("g")->value()),
        Ogre::StringConverter::parseReal(XMLNode->first_attribute("b")->value()),
        XMLNode->first_attribute("a") != NULL ? Ogre::StringConverter::parseReal(XMLNode->first_attribute("a")->value()) : 1
    );
}

Ogre::String DotSceneLoader::getProperty(const Ogre::String &ndNm, const Ogre::String &prop)
{
    for ( unsigned int i = 0 ; i < nodeProperties.size(); i++ )
    {
        if ( nodeProperties[i].nodeName == ndNm && nodeProperties[i].propertyNm == prop )
        {
            return nodeProperties[i].valueName;
        }
    }

    return "";
}

void DotSceneLoader::processUserDataReference(rapidxml::xml_node<>* XMLNode, Ogre::Entity *pEntity)
{
    Ogre::String str = XMLNode->first_attribute("id")->value();
    pEntity->setUserAny(Ogre::Any(str));
}
