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
//functions by taking advantage of the static nature of tree billboards (note:
//StaticBillboardSet does not allow billboards to be moved or deleted individually in
//real-time)
//-------------------------------------------------------------------------------------

#ifndef __StaticBillboardSet_H__
#define __StaticBillboardSet_H__

#include <OgrePrerequisites.h>
#include <OgreRoot.h>
#include <OgreRenderSystem.h>
#include <OgreVector3.h>
#include <OgreMesh.h>
#include <OgreMaterial.h>
#include <OgreBillboard.h>
#include <OgreBillboardSet.h>
#include <OgreMaterialManager.h>
#include <OgreSceneNode.h>
#include <OgreStringConverter.h>

namespace Forests
{

   class SBMaterialRef;
   typedef std::map<Ogre::Material*, SBMaterialRef*> SBMaterialRefList;

   /** Different methods used to render billboards. This can be supplied as a parameter
   to the StaticBillboardSet constructor to manually select how you want billboards
   rendered (although in almost all cases BB_METHOD_ACCELERATED is the best choice).*/
   enum BillboardMethod
   {
      /** This mode accelerates the performance of billboards by using vertex shaders
      to keep billboards facing the camera. Note: If the computer's hardware is not
      capable	of vertex shaders, it will automatically fall back to BB_METHOD_COMPATIBLE
      mode.*/
      BB_METHOD_ACCELERATED = 1,

      /** Unlike BB_METHOD_ACCELERATED, this does not use vertex shaders to align
      billboards to the camera. This is more compatible with old video cards,
      although it can result in poor performance with high amounts of billboards.*/
      BB_METHOD_COMPATIBLE = 0,
   };


   //--------------------------------------------------------------------------
   /// A faster alternative to Ogre's built-in BillboardSet class.
   ///
   /// This class provides a method of displaying billboards faster than Ogre's built-in
   /// BillboardSet functions by taking advantage of the static nature of tree billboards.
   /// However, if your video card does not support vertex shaders, using this class over
   /// Ogre's built-in Billboard class will have no performance benefit.
   ///
   /// @note StaticBillboardSet does not allow billboards to be moved or deleted individually in real-time
   class StaticBillboardSet
   {
      //-----------------------------------------------------------------------------
      //Internal class - do not use
      class StaticBillboard
      {
      public:
         // Constructor
         StaticBillboard(const Ogre::Vector3 &pos, float xScale, float yScale,
            const Ogre::ColourValue &clr, Ogre::uint16 texcrdIndexU, Ogre::uint16 texcrdIndexV) :
         xPos((float)pos.x), yPos((float)pos.y), zPos((float)pos.z), xScaleHalf(0.5f * xScale), yScaleHalf(0.5f * yScale),
            texcoordIndexU(texcrdIndexU), texcoordIndexV(texcrdIndexV)
         {
            Ogre::Root::getSingletonPtr()->getRenderSystem()->convertColourValue(clr, &color);
         }


         // SVA mem friendly aligment
         float xPos, yPos, zPos;
         //float xScale, yScale;
         float xScaleHalf, yScaleHalf;
         Ogre::uint32 color;
         Ogre::uint16 texcoordIndexU, texcoordIndexV;
      };

   public:
      /**
      \brief Initializes a StaticBillboardSet object.
      \param mgr The SceneManager to be used to display the billboards.
      \param method The method used when rendering billboards. See the BillboardMethod
      documentation for more information. In almost all cases, this should be set to
      BB_METHOD_ACCELERATED for optimal speed and efficiency.
      */
      StaticBillboardSet(Ogre::SceneManager *mgr, Ogre::SceneNode *rootSceneNode, BillboardMethod method = BB_METHOD_ACCELERATED);
      ~StaticBillboardSet();

      /**
      \brief Adds a billboard to the StaticBillboardSet at the specified position.
      \param position The desired position of the billboard.
      \param xScale The width scale of the billboard.
      \param yScale The height scale of the billboard.
      \param texcoordIndex The texture tile this billboard will use. This value shoud be 
      0..n, where n is the number of slices set with setTextureSlices()

      The texcoordIndex option is only applicable if you have used setTextureSlices()
      to divide the applied material into a number of horizontal segments. texcoordIndex selects
      which segment is applied to the billboard as a texture.

      \note Any billboards created will not appear in the scene until you call build()
      */
      void createBillboard(const Ogre::Vector3 &position, float xScale = 1.0f, float yScale = 1.0f,
         const Ogre::ColourValue &color = Ogre::ColourValue::White, Ogre::uint16 texcoordIndexU = 0, Ogre::uint16 texcoordIndexV = 0)
      {
         if (mRenderMethod == BB_METHOD_ACCELERATED)
         {
            mBillboardBuffer.push_back(new StaticBillboard(position, xScale, yScale, color, texcoordIndexU, texcoordIndexV));

            //mBillboardBuffer.push_back(bb);

            ////bb->position = position;
            //bb->xPos = (float)position.x;
            //bb->yPos = (float)position.y;
            //bb->zPos = (float)position.z;
            //bb->xScaleHalf = xScale * 0.5f;
            //bb->yScaleHalf = yScale * 0.5f;

            //bb->texcoordIndexU = texcoordIndexU;
            //bb->texcoordIndexV = texcoordIndexV;

            //Ogre::uint32 packedColor;
            //Ogre::Root::getSingleton().getRenderSystem()->convertColourValue(color, &packedColor);
            //bb->color = packedColor;
         }
         else
         {
            Ogre::Billboard *bb = mpFallbackBillboardSet->createBillboard(position, color);
            bb->setDimensions(xScale, yScale);
            bb->setTexcoordRect(texcoordIndexU * mfUFactor, texcoordIndexV * mfVFactor,
               (texcoordIndexU + 1) * mfUFactor, (texcoordIndexV + 1) * mfVFactor);
         }
      }

      /**
      \brief Sets the billboard's origin (pivotal point)

      This function can be used to set what part of the billboard image is considered the
      origin, or "center". By default, the center of the image is used, so billboards will
      pivot around the center and positioning a billboard will place it's center at the desired
      location. Other origins, like BBO_BOTTOM_CENTER are good for trees, etc. BBO_CENTER is
      used by default.
      */
      void setBillboardOrigin(Ogre::BillboardOrigin origin);

      /// Returns the current billboard origin.
      /// This returns the current billboard origin as set by setBillboardOrigin().
      Ogre::BillboardOrigin getBillboardOrigin() const   { return mBBOrigin; }

      /// Returns the method used to render billboards.
      /// The billboard render method is set in the constructor. See the BillboardMethod enum
      /// documentation for more information on billboard render methods.
      BillboardMethod getRenderMethod() const            { return mRenderMethod; }

      ///Sets whether or not this StaticBillboardSet will be rendered.
      /// \param visible The desired visibility state of the StaticBillboardSet (true/false)
      void setVisible(bool visible)
      {
         if (mVisible != visible)
         {
            mVisible = visible;
            mpSceneNode->setVisible(visible);
         }
      }

      /**
      \brief Enables/disables distance fade-out for this billboard set
      \param enabled Whether or not to enable fading
      \param visibleDist The distance where billboards will be fully opaque (alpha 1)
      \param invisibleDist The distance where billboards will be invisible (alpha 0)

      You can use this function to enable distance based alpha fading, so billboards will
      smoothly fade out into the distance. Note that the fading is performed 2-dimensionally,
      which means height is not taken into account when fading - only xz distances. This works
      well with flat terrain maps, but may not be well suited for spherical worlds.

      The distance ranges given specify how the final alpha values should be calculated -
      billboards at visibleDist will have alpha values of 1, and geometry at invisibleDist
      will have alpha values of 0.

      \note invisibleDist may be greater than or less than visibleDist, depending on
      whether the geometry is fading out or in to the distance.

      \note setFade() only works in BB_MODE_ACCELERATED mode.
      */
      void setFade(bool enabled, Ogre::Real visibleDist, Ogre::Real invisibleDist);

      /// Performs final steps required for the created billboards to appear in the scene.
      /// Until this is called, any billboards created with createBillboard() will not appear.
      void build();

      /// Deletes all billboards from the scene.
      /// The only way to delete a billboard in a StaticBillboardSet is to delete them all,
      /// which this function does.
      void clear();

      /**
      \brief Applies a material to the billboards in this set.
      \param materialName The name of the material to apply to this StaticBillboardSet.

      This may actually modify the material to include a vertex shader (which
      is used to keep the billboards facing the camera).
      */
      void setMaterial(const Ogre::String &materialName, const Ogre::String &resourceGroup =
         Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

      /**
      \brief Sets how many horizontal slices and vertical stacks the currently applied material is using.
      \param stacks The number of vertical stacks.
      \param slices The number of horizontal slices.

      If the applied material contains multiple images all merged together into a grid
      you can use this function to gain access to the individual images. Simply set how
      many tiles are contained within the material horizontally and vertically, and use
      the texcoordIndexU and texcoordIndexV parameters of createBillboard() to specify
      which image is to be used for the billboard.
      */
      void setTextureStacksAndSlices(Ogre::uint16 stacks, Ogre::uint16 slices);

      /**
      \brief Manually updates all StaticBillboardSet objects for a frame.

      \note If you are using root->startRendering() or root->renderOneFrame() to update
      your scene, there is no need to use this function at all. Doing so would be redundant
      and ineffient, as it will be called	automatically in this case.

      However, if you	update all your render targets yourself, you will have to call this
      manually per frame from your program loop. If updateAll() doesn't get called one way
      or another, your billboards will not be updated to face the camera.
      */
      static void updateAll(const Ogre::Vector3 &cameraDirection);

   private:
      ///
      Ogre::MaterialPtr getFadeMaterial(const Ogre::MaterialPtr &protoMaterial, Ogre::Real visibleDist, Ogre::Real invisibleDist);

   private:

      typedef std::vector < StaticBillboard* >  TBillBuf;

      bool                    mVisible;                  ///<
      bool                    mFadeEnabled;              ///<
      BillboardMethod         mRenderMethod;             ///<

      Ogre::SceneManager*     mpSceneMgr;                ///<
      Ogre::SceneNode*        mpSceneNode;               ///<
      Ogre::Entity*           mpEntity;                  ///<
      Ogre::MeshPtr           mPtrMesh;                  ///<
      Ogre::String            mEntityName;               ///<
      Ogre::MaterialPtr       mPtrMaterial;              ///<
      Ogre::MaterialPtr       mPtrFadeMaterial;          ///<
      float                   mfUFactor, mfVFactor;      ///<

      Ogre::BillboardSet*     mpFallbackBillboardSet;    ///<
      Ogre::BillboardOrigin   mBBOrigin;                 ///<
      Ogre::Real              mFadeVisibleDist;          ///<
      Ogre::Real              mFadeInvisibleDist;        ///<
      TBillBuf                mBillboardBuffer;          ///<


      // static data
   private:

      typedef std::map<Ogre::String, Ogre::MaterialPtr> FadedMaterialMap;

      static bool             s_isGLSL;                  ///< OpenGL
      //static bool             s_shadersGenerated;        ///< First instance generate shaders for billboard rendering
      static unsigned int     s_nSelfInstances;          ///< Instances counter
      static FadedMaterialMap s_mapFadedMaterial;        ///< 

      //static Ogre::uint32 selfInstances;
      static unsigned long GUID;
      static Ogre::String getUniqueID(const Ogre::String &prefix) { return prefix + Ogre::StringConverter::toString(++GUID); }
   };



   //-------------------------------------------------------------------------------------

   //SBMaterialRef::addMaterialRef() and ::removeMaterialRef() are used to keep track
   //of all the materials in use by the billboard system. This is necessary when keeping
   //the materials' vertex shaders up-to-date. To get the list of all materials in use,
   //use getList().
   class SBMaterialRef
   {
   public:
      static void addMaterialRef(const Ogre::MaterialPtr &matP, Ogre::BillboardOrigin o);
      static void removeMaterialRef(const Ogre::MaterialPtr &matP);

      inline static SBMaterialRefList &getList() { return selfList; }

      inline Ogre::Material *getMaterial() { return material; }
      inline Ogre::BillboardOrigin getOrigin() { return origin; }

   private:
      SBMaterialRef(Ogre::Material *mat, Ogre::BillboardOrigin o);

      static SBMaterialRefList selfList;

      Ogre::uint32 refCount;
      Ogre::Material *material;
      Ogre::BillboardOrigin origin;
   };
}

#endif
