/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
    1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

//WindBatchPage.h
//WindBatchPage is child of BatchPage to include a wind effect.
//-------------------------------------------------------------------------------------

#ifndef __WindBatchPage_H__
#define __WindBatchPage_H__

#include "BatchPage.h"
#include "WindBatchedGeometry.h"

#include <OgrePrerequisites.h>
#include <OgreStringConverter.h>

namespace Forests {

class PagedGeometry;

/**
\brief The WindBatchPage class renders entities as StaticGeometry with hardware accelerated wind animation capability.

This is one of the geometry page types included in the StaticGeometry engine. These
page types should be added to a PagedGeometry object with PagedGeometry::addDetailLevel()
so the PagedGeometry will know how you want your geometry displayed.

To use this page type, use:
\code
PagedGeometry::addDetailLevel<WindBatchPage>(farRange, transitionLength, Ogre::Any(LODLevel));
\endcode

This page type (WindBatchPage) is almost identical to BatchPage, except it includes additional
code to support a hardware accelerated wind animation technique (through a vertex shader). To
enable animation on your tree(s), use PagedGeometry::setCustomParam() to set the following
parameters:

windFactorX - Horizontal tree sway magnitude
windFactorY - Vertical tree sway magnitude

See Example 8 for a practical example of using WindBatchPage.

Special thanks to Wendigo Studios (www.wendigostudios.com) for donating this extension of
BatchPage to the PagedGeometry project.
*/
class WindBatchPage: public BatchPage
{
public:
	inline WindBatchPage() { mGeom = NULL; }
	void init(PagedGeometry *geom, const Ogre::Any &data);

protected:
	void _updateShaders();

private :
	std::string entityName;
	const PagedGeometry  * mGeom;
};

}

#endif
