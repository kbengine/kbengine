/* ------------------------------------------------------------------------- */
 // File       : LensFlare.h
 // Project    : Long Forgotten Earth
 // Author     : David de Lorenzo
 /* ------------------------------------------------------------------------- */
 #ifndef _LENSFLARE_H_
 #define _LENSFLARE_H_
 
 #if _MSC_VER > 1000
 #pragma once
 #endif
 
 #include "ogre.h"
 
 using namespace Ogre;
 
 /* ------------------------------------------------------------------------- */
 /// A lens Flare effect.
 /**
 This class will create a lensflare effect, between The light position and the 
 camera position.
 Some functions will allow to change the lensflare color effect, in case of coloured 
 light, for instance.
 */
 /* ------------------------------------------------------------------------- */
 class LensFlare
 {
 public:
     LensFlare(Vector3 LightPosition, Camera* camera, SceneManager* SceneMgr);
     virtual ~LensFlare();
     void    createLensFlare();
     void    update();
     void    setVisible(bool visible);
     void    setLightPosition(Vector3 pos);
     void    setHaloColour(ColourValue color);
     void    setBurstColour(ColourValue color);
 
 
 protected:
     SceneManager* mSceneMgr;
     Camera*       mCamera;
     ColourValue   mColour;
     SceneNode*    mNode;
        BillboardSet* mHaloSet;
       BillboardSet* mBurstSet;
     Vector3       mLightPosition;
     bool          mHidden;
 };
 
 #endif
 
 