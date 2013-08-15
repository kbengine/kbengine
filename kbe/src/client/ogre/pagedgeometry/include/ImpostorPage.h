/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich
Modified 2008 by Erik Hjortsberg (erik.hjortsberg@iteam.se)

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
    1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

//ImpostorPage.h
//ImposterPage is an extension to PagedGeometry which displays entities as imposters.
//-------------------------------------------------------------------------------------

#ifndef __ImpostorPage_H__
#define __ImpostorPage_H__

#include "PagedGeometry.h"
#include "StaticBillboardSet.h"

#include <OgrePrerequisites.h>
#include <OgreTextureManager.h>
#include <OgreRenderTexture.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
// linux memory fix
#include <memory>
#endif

//The number of angle increments around the yaw axis to render impostor "snapshots" of trees
#define IMPOSTOR_YAW_ANGLES 8

//The number of angle increments around the pitch axis to render impostor "snapshots" of trees
#define IMPOSTOR_PITCH_ANGLES 4

//When IMPOSTOR_RENDER_ABOVE_ONLY is defined, impostor images will only be rendered from angles around and
//above entities. If this is disabled, bottom views of the entities will be rendered to the impostor atlas
//and therefore allow those angles to be viewed from a distance. However, this requires the IMPOSTOR_PITCH_ANGLES
//to be doubled to maintain an equal level of impostor angle correctness compared to when impostors are rendered
//from above only.
#define IMPOSTOR_RENDER_ABOVE_ONLY

//When IMPOSTOR_FILE_SAVE is defined, impostor textures will be read and saved to disc; if not, they will stay
//in memory and need to be regenerated each time the application is run (remove or comment out the line below if this
//is desired)
#define IMPOSTOR_FILE_SAVE

namespace Forests {

class ImpostorBatch;
class ImpostorTexture;

//Blend modes used by ImpostorPage::setBlendMode()
typedef enum ImpostorBlendMode {
	ALPHA_REJECT_IMPOSTOR,
	ALPHA_BLEND_IMPOSTOR
} ImpostorBlendMode;

//-------------------------------------------------------------------------------------
/**
\brief The ImpostorPage class renders entities as impostors (billboard images that look just like the real entity).

This is one of the geometry page types included in the StaticGeometry engine. These
page types should be added to a PagedGeometry object with PagedGeometry::addDetailLevel()
so the PagedGeometry will know how you want your geometry displayed.

To use this page type, use:
\code
PagedGeometry::addDetailLevel<ImpostorPage>(farRange);
\endcode	TexturePtr renderTexture;


Of all the page types included in the PagedGeometry engine, this one is the fastest. It
uses impostors (billboards that look just like the real entity) to represent entities.
This way, your video card only has to render a bunch of 2D images, instead of a full 3D
mesh. Imposters are generally used off in the distance slightly, since they don't always
look exactly like the real thing, especially up close (since they are flat, and sometimes
slightly pixelated).

\note Impostors are generated only once for each entity. If you make any changes to your
entities, you'll have to force impostor regeneration by deleting the prerender files
located in your executable's working directory (named "Impostor.[ResourceGroup].[Entity].png")
*/
class ImpostorPage: public GeometryPage
{
	friend class ImpostorBatch;
	friend class ImpostorTexture;

public:
	void init(PagedGeometry *geom, const Ogre::Any &data);
	~ImpostorPage();
	
	void setRegion(Ogre::Real left, Ogre::Real top, Ogre::Real right, Ogre::Real bottom);
	void addEntity(Ogre::Entity *ent, const Ogre::Vector3 &position, const Ogre::Quaternion &rotation, const Ogre::Vector3 &scale, const Ogre::ColourValue &color);
	void build();
	void removeEntities();
	
	void setVisible(bool visible);
	void setFade(bool enabled, Ogre::Real visibleDist, Ogre::Real invisibleDist);

	void update();

	/**
	\brief Sets the resolution for single impostor images.
	\param pixels The width/height of one square impostor render
	
	The default impostor resolution is 128x128. Note that 32 impostor images
	will be stored in a single texture (8 x 4), so a impostor resolution of 128,
	for example, results in final texture size of 1024 x 512.
	
	\warning Calling this function will have no effect unless it is done before
	any entities are added to any page.
	*/
	static void setImpostorResolution(int pixels) { impostorResolution = pixels; }

	/**
	\brief Sets the background color used when rendering impostor images.
	\param color The background color

	Choosing an impostor color that closely matches the main color of your objects
	is important to reduce mipmap artifacts. When distant objects are displayed as
	impostors, hardware mipmapping can cause the color surrounding your object (the
	background color) to "bleed" into the main image, which can result in incorrectly
	tinted distant objects.

	The default background color is ColourValue(0.0f, 0.3f, 0.0f, 0.0f), or dark green
	(this color was chosen because the main use of ImpostorPage is for trees, bushes, etc.)

	\warning Calling this function will have no effect unless it is done before
	any entities are added to any page. Also remember that you may have to
	delete the old impostor renders (located in your exe's directory) in
	order for the new ones to be generated.
	*/
	static void setImpostorColor(const Ogre::ColourValue &color)
	{
		impostorBackgroundColor = color;
		impostorBackgroundColor.a = 0.0f;
	}

	/**
	\brief Sets the billboard pivot point used when rendering camera-facing impostors

	This function can be used to set how impostors should rotate to face the camera. By default,
	impostors are pointed towards the camera by rotating around the impostor billboard's center.
	By choosing an alternate pivot point with this function, you can acheive better results under
	certain conditions. For example, when looking up or down very steep hills, you'll probably want
	to set BBO_BOTTOM_CENTER as the pivot point. For most other cases, however, the default pivot
	point of BBO_CENTER works best.

	\note Only BBO_CENTER and BBO_BOTTOM_CENTER is supported by this function currently.

	\warning Calling this function will have no effect unless it is done before
	any entities are added to any page.
	*/
	static void setImpostorPivot(Ogre::BillboardOrigin origin);

	/**
	\brief Regenerates the impostor texture for the specified entity
	\param ent The entity which will have it's impostor texture regenerated
	
	This function can be called to force the regeneration of a specific impostor.
	Normally, impostors are generated once (saved to a file), and simply preloaded
	from the file afterwards (unless you delete the file). Calling this will
	instantly regenerate the impostor and update it's saved image file.

	\note This function cannot regenerate an impostor unless it's already being used
	in the scene.

	\warning This is NOT a real-time operation - it may take a few seconds to complete.
	*/
	static void regenerate(Ogre::Entity *ent);

	/**
	\brief Regenerates all impostor textures currently being used in the scene

	This function can be called to force the regeneration of all impostors currently being
	used in your scene. Normally, impostors are generated once (saved to a file), and simply
	preloaded from the files afterwards (unless you delete the files). Calling this will
	instantly regenerate the impostors and update their saved image files.

	\warning This is NOT a real-time operation - it may take a few seconds to complete.
	*/
	static void regenerateAll();
	

	inline void setBlendMode(ImpostorBlendMode blendMode) { this->blendMode = blendMode; }
	inline ImpostorBlendMode getBlendMode() { return blendMode; }

protected:
	Ogre::SceneManager *sceneMgr;
	PagedGeometry *geom;

	ImpostorBlendMode blendMode;
	static int impostorResolution;
	static Ogre::ColourValue impostorBackgroundColor;
	static Ogre::BillboardOrigin impostorPivot;
	
	static Ogre::uint32 selfInstances;
	static Ogre::uint32 updateInstanceID;
	Ogre::uint32 instanceID;

	Ogre::Timer updateTimer;

	Ogre::Vector3 center;
	int aveCount;
	
	std::map<Ogre::String, ImpostorBatch *> impostorBatches;
};


//-------------------------------------------------------------------------------------
//This is used internally by ImpostorPage to store a "batch" of impostors. Similar
//impostors are all batched into ImpostorBatch'es, which contain a BillboardSet (where
//the actual billboards are), and a pointer to an existing ImpostorTexture.
class ImpostorBatch
{
public:
	static ImpostorBatch *getBatch(ImpostorPage *group, Ogre::Entity *entity);
	~ImpostorBatch();

	inline void build()
	{
		bbset->build();
	}

	inline void clear()
	{
		bbset->clear();
	}

	inline void setVisible(bool visible)
	{
		bbset->setVisible(visible);
	}

	inline void setFade(bool enabled, Ogre::Real visibleDist, Ogre::Real invisibleDist)
	{
		bbset->setFade(enabled, visibleDist, invisibleDist);
	}

	void setBillboardOrigin(Ogre::BillboardOrigin origin);
	inline void addBillboard(const Ogre::Vector3 &position, const Ogre::Quaternion &rotation, const Ogre::Vector3 &scale, const Ogre::ColourValue &color = Ogre::ColourValue::White);
	void setAngle(float pitchDeg, float yawDeg);

	static Ogre::String generateEntityKey(Ogre::Entity *entity);

protected:
	ImpostorBatch(ImpostorPage *group, Ogre::Entity *entity);

	ImpostorTexture *tex;
	StaticBillboardSet *bbset;

	Ogre::Vector3 entityBBCenter;

	ImpostorPage *igroup;

	Ogre::uint16 pitchIndex, yawIndex;

	static unsigned long GUID;
	static inline Ogre::String getUniqueID(const Ogre::String &prefix)
	{
		return prefix + Ogre::StringConverter::toString(++GUID);
	}
};

//-------------------------------------------------------------------------------------
//Responsible for making sure that the texture is rerendered when the texture resource needs to //be reloaded.
//
class ImpostorTextureResourceLoader : public Ogre::ManualResourceLoader
{
public:
	/**
	 *    Ctor.
	 * @param renderContext The ImpostorTexture to which this instance belongs.
	 */
	ImpostorTextureResourceLoader(ImpostorTexture& impostorTexture);
	
	
	/**
	 *    At load time the texture will be rerendered.
	 * @param resource 
	 */
	virtual void loadResource (Ogre::Resource *resource);
protected:
	ImpostorTexture& texture;
};

//-------------------------------------------------------------------------------------
//This is used internally by ImpostorPage. An ImpostorTexture is actually multiple
//images of an entity from various rotations. ImpostorTextures are applied
//to billboards to create the effect of 3D shapes, when in reality they are simply
//flat billboards.
class ImpostorTexture
{
	friend class ImpostorBatch;
	friend class ImpostorTextureResourceLoader;

public:
	/** Returns a pointer to an ImpostorTexture for the specified entity. If one does not
	already exist, one will automatically be created.
	*/
	static ImpostorTexture *getTexture(ImpostorPage *group, Ogre::Entity *entity);
	
	/** remove created texture, note that all of the ImposterTextures
	must be deleted at once, because there is no track if a texture is still
	being used by something else
	*/
	static void removeTexture(ImpostorTexture* Texture);

	void regenerate();
	static void regenerateAll();

	~ImpostorTexture();
	
protected:
	ImpostorTexture(ImpostorPage *group, Ogre::Entity *entity);

	void renderTextures(bool force);	// Renders the impostor texture grid
	void updateMaterials();				// Updates the materials to use the latest rendered impostor texture grid

	Ogre::String removeInvalidCharacters(Ogre::String s);

	static std::map<Ogre::String, ImpostorTexture *> selfList;
	Ogre::SceneManager *sceneMgr;
	Ogre::Entity *entity;
	Ogre::String entityKey;
	ImpostorPage *group;

	Ogre::MaterialPtr material[IMPOSTOR_PITCH_ANGLES][IMPOSTOR_YAW_ANGLES];
	Ogre::TexturePtr texture;

	Ogre::ResourceHandle sourceMesh;
	Ogre::AxisAlignedBox boundingBox;
	float entityDiameter, entityRadius;
	Ogre::Vector3 entityCenter;

	static unsigned long GUID;
	static inline Ogre::String getUniqueID(const Ogre::String &prefix)
	{
		return prefix + Ogre::StringConverter::toString(++GUID);
	}
	
	//This will only be used when IMPOSTOR_FILE_SAVE is set to 0
	std::auto_ptr<ImpostorTextureResourceLoader> loader;
};



//-------------------------------------------------------------------------------------
//This is an inline function from ImposterBatch that had to be defined down below the
//ImpostorTexture class, because it uses it.
void ImpostorBatch::addBillboard(const Ogre::Vector3 &position, const Ogre::Quaternion &rotation, const Ogre::Vector3 &scale, const Ogre::ColourValue &color)
{
	//float degrees = (Math::ACos(rotation.w)*2.0f).valueDegrees();
	const Ogre::Vector3 zVector = rotation * Ogre::Vector3::UNIT_Z;
	float degrees = Ogre::Math::ATan2(zVector.x, zVector.z).valueDegrees();
	if (degrees < 0) degrees += 360;

	int n = IMPOSTOR_YAW_ANGLES * (degrees / 360.0f) + 0.5f;
	Ogre::uint16 texCoordIndx = (IMPOSTOR_YAW_ANGLES - n) % IMPOSTOR_YAW_ANGLES;

	bbset->createBillboard(position + (rotation * entityBBCenter) * scale,
							tex->entityDiameter * 0.5f * (scale.x + scale.z),
							tex->entityDiameter * scale.y, color,
							texCoordIndx);
}
}

#endif
