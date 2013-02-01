/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/
#ifndef __WindBatchedGeometry_H__
#define __WindBatchedGeometry_H__

#include <OgrePrerequisites.h>
#include <OgreMovableObject.h>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>

#include "BatchedGeometry.h"

namespace Forests
{
   // Forward declaration
   class PagedGeometry;


   //-----------------------------------------------------------------------------
   /// A variation of BatchedGeometry designed for WindBatchPage.
   class WindBatchedGeometry: public BatchedGeometry
   {
   public:
      //--------------------------------------------------------------------------
      /// Wind chunck of Renderable geometry
      class WindSubBatch: public BatchedGeometry::SubBatch
      {
      public:
         WindSubBatch(WindBatchedGeometry *parent, Ogre::SubEntity *ent);
         void build();
      };

   public:
      WindBatchedGeometry(Ogre::SceneManager *mgr, Ogre::SceneNode *rootSceneNode, const PagedGeometry *pagedGeom);

      void addEntity(Ogre::Entity *ent, const Ogre::Vector3 &position, const Ogre::Quaternion &orientation = Ogre::Quaternion::IDENTITY, const Ogre::Vector3 &scale = Ogre::Vector3::UNIT_SCALE, const Ogre::ColourValue &color = Ogre::ColourValue::White);
      void setGeom(const PagedGeometry * geom) { mGeom = geom; }

   private:
      const PagedGeometry  * mGeom;
   };

}

#endif
