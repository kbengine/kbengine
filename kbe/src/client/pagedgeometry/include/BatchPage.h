/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

/// \file BatchPage.h
/// \brief BatchPage is an extension to PagedGeometry which displays entities as static geometry.
//-------------------------------------------------------------------------------------
#ifndef __BatchPage_H__
#define __BatchPage_H__

#include "PagedGeometry.h"
#include "BatchedGeometry.h"

#include <OgrePrerequisites.h>
#include <OgreStringConverter.h>

namespace Forests
{

   /**
   \brief The BatchPage class renders entities as StaticGeometry.

   This is one of the geometry page types included in the StaticGeometry engine. These
   page types should be added to a PagedGeometry object with PagedGeometry::addDetailLevel()
   so the PagedGeometry will know how you want your geometry displayed.

   To use this page type, use (the last parameter is optional):
   \code
   PagedGeometry::addDetailLevel<BatchPage>(farRange, transitionLength, Ogre::Any(LODLevel));
   \endcode

   This page type uses batched geometry (Ogre::StaticGeometry) to represent the entities.
   Batched geometry is generally much faster than plain entities, since video card state
   changes and transform calculations can be minimized. Batched geometry can be anywhere
   from 2 to 20 times faster than plain entities.

   "LODLevel" can be used to specify a certain LOD level to use from the added entities.
   This would be useful, for example, if you wanted to add high-res batched trees near the camera,
   and low-res batched trees farther away.
   */
   class BatchPage: public GeometryPage
   {
      typedef std::vector<Ogre::MaterialPtr> TMaterials;
   public:
      /// Default constructor
      BatchPage();
      ~BatchPage();

      /// Replace pure virtual GeometryPage::init
      void init(PagedGeometry *geom, const Ogre::Any &data);

      void addEntity(Ogre::Entity *ent, const Ogre::Vector3 &position, const Ogre::Quaternion &rotation,
         const Ogre::Vector3 &scale, const Ogre::ColourValue &color);
      void removeEntities();

      void build();

      void setVisible(bool visible);
      void setFade(bool enabled, Ogre::Real visibleDist, Ogre::Real invisibleDist);

      void addEntityToBoundingBox()
      {
         int a = 0;
         a++;
      }

      void clearBoundingBox()
      {
         int a = 0;
         a++;
      }

      const Ogre::AxisAlignedBox &getBoundingBox() { return m_pBatchGeom->getBoundingBox(); }

   protected :
      virtual void _updateShaders();

   private:
      static Ogre::String getUniqueID(const Ogre::String &prefix)
      {
         return prefix + Ogre::StringConverter::toString(++s_nGUID);
      }

      // Data section of class BatchPage
   protected:
      PagedGeometry*       m_pPagedGeom;
      Ogre::SceneManager*  m_pSceneMgr;
      BatchedGeometry*     m_pBatchGeom;
      size_t               m_nLODLevel;
      bool                 m_bFadeEnabled;
      bool                 m_bShadersSupported;
      Ogre::Real           m_fVisibleDist;
      Ogre::Real           m_fInvisibleDist;
      TMaterials           m_vecUnfadedMaterials;
      
   protected:
      static unsigned long s_nRefCount;
      static unsigned long s_nGUID;
   };

}

#endif
