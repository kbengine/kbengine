#include "Ogre.h"
#include "OgreFontManager.h"
#include "OgreMovableText.h"

using namespace Ogre;

#define POS_TEX_BINDING    0
#define COLOUR_BINDING     1
#define UNICODE_NEL 0x0085   ///< next line
#define UNICODE_CR 0x000D   ///< carriage return
#define UNICODE_LF 0x000A   ///< line feed
#define UNICODE_SPACE 0x0020 ///< space
#define UNICODE_ZERO 0x0030   ///< 0

MovableText::MovableText(const String &name,
						 const DisplayString &caption,
						 const String &fontName,
						 Real charHeight,
						 const ColourValue &color)
						 : mpCam(NULL)
						 , mpWin(NULL)
						 , mpFont(NULL)
						 , mName(name)
						 , mCaption(caption)
						 , mFontName(fontName)
						 , mCharHeight(charHeight)
						 , mColor(color)
						 , mType("MovableText")
						 , mTimeUntilNextToggle(0)
						 , mSpaceWidth(0)
						 , mUpdateColors(true)
						 , mOnTop(false)
						 , mHorizontalAlignment(H_LEFT)
						 , mVerticalAlignment(V_BELOW)
						 , mAdditionalHeight(0.0)
						 , mPositionOffset(Vector3::ZERO)
						 , mScaleOffset(Vector3::UNIT_SCALE)
{
	if (name == StringUtil::BLANK )
		throw Exception(Exception::ERR_INVALIDPARAMS, "Trying to create MovableText without name", "MovableText::MovableText");

	if (caption == StringUtil::BLANK )
		throw Exception(Exception::ERR_INVALIDPARAMS, "Trying to create MovableText without caption", "MovableText::MovableText");

	mRenderOp.vertexData = NULL;
	this->setFontName(mFontName);
	this->_setupGeometry();
}

MovableText::~MovableText()
{
	if (mRenderOp.vertexData)
		delete mRenderOp.vertexData;
}

// Add to build on Shoggoth:
void MovableText::visitRenderables(Ogre::Renderable::Visitor* visitor, 
								   bool debugRenderables)
{
}

void MovableText::setFontName(const String &fontName)
{
	if((Ogre::MaterialManager::getSingletonPtr()->resourceExists(mName + "Material")))
	{
		Ogre::MaterialManager::getSingleton().remove(mName + "Material");
	}

	if (mFontName != fontName || mpMaterial.isNull() || mpFont.isNull())
	{
		mFontName = fontName;
		mpFont = FontManager::getSingleton().getByName(mFontName);
		if (mpFont.isNull())
			throw Exception(Exception::ERR_ITEM_NOT_FOUND, "Could not find font " + fontName, "MovableText::setFontName");

		mpFont->load();
		if (!mpMaterial.isNull())
		{
			MaterialManager::getSingletonPtr()->remove(mpMaterial->getName());
			mpMaterial.setNull();
		}

		mpMaterial = mpFont->getMaterial()->clone(mName + "Material");
		if (!mpMaterial->isLoaded())
			mpMaterial->load();

		mpMaterial->setDepthCheckEnabled(!mOnTop);
#if 1
		mpMaterial->setDepthBias(1.0,1.0);
#else
		mpMaterial->setDepthBias(1.0f);
#endif
		mpMaterial->setDepthWriteEnabled(mOnTop);
		mpMaterial->setLightingEnabled(false);
		mNeedUpdate = true;
	}
}

void MovableText::setCaption(const DisplayString &caption)
{
	if (caption != mCaption)
	{
		mCaption = caption;
		mNeedUpdate = true;
	}
}

void MovableText::setColor(const ColourValue &color)
{
	if (color != mColor)
	{
		mColor = color;
		mUpdateColors = true;
	}
}

void MovableText::setCharacterHeight(Real height)
{
	if (height != mCharHeight)
	{
		mCharHeight = height;
		mSpaceWidth = 0;
		mNeedUpdate = true;
	}
}

void MovableText::setSpaceWidth(Real width)
{
	if (width != mSpaceWidth)
	{
		mSpaceWidth = width;
		mNeedUpdate = true;
	}
}

void MovableText::setTextAlignment(const HorizontalAlignment& horizontalAlignment,
								   const VerticalAlignment& verticalAlignment)
{
	if(mHorizontalAlignment != horizontalAlignment)
	{
		mHorizontalAlignment = horizontalAlignment;
		mNeedUpdate = true;
	}
	if(mVerticalAlignment != verticalAlignment)
	{
		mVerticalAlignment = verticalAlignment;
		mNeedUpdate = true;
	}
}

void MovableText::setAdditionalHeight( Real height )
{
	if( mAdditionalHeight != height )
	{
		mAdditionalHeight = height;
		mNeedUpdate = true;
	}
}

void MovableText::showOnTop(bool show)
{
	if( mOnTop != show && !mpMaterial.isNull() )
	{
		mOnTop = show;
#if 1
		mpMaterial->setDepthBias(1.0f, 1.0f);
#else
		mpMaterial->setDepthBias(1.0f);
#endif
		mpMaterial->setDepthCheckEnabled(!mOnTop);
		mpMaterial->setDepthWriteEnabled(mOnTop);
	}
}

void MovableText::_setupGeometry()
{
	if (mpFont.isNull())
	{
		return;
	}

	if (mpMaterial.isNull())
	{
		return;
	}

	size_t charlen = mCaption.size();

	size_t vertexCount = charlen * 6;

	if (mRenderOp.vertexData)
	{
		// Removed this test as it causes problems when replacing a caption
		// of the same size: replacing "Hello" with "hello"
		// as well as when changing the text alignment
		//if (mRenderOp.vertexData->vertexCount != vertexCount)
		{
			delete mRenderOp.vertexData;
			mRenderOp.vertexData = NULL;
			mUpdateColors = true;
		}
	}

	if (!mRenderOp.vertexData)
		mRenderOp.vertexData = new VertexData();

	mRenderOp.indexData = 0;
	mRenderOp.vertexData->vertexStart = 0;
	mRenderOp.vertexData->vertexCount = vertexCount;
	mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;
	mRenderOp.useIndexes = false;

	VertexDeclaration *decl = mRenderOp.vertexData->vertexDeclaration;
	VertexBufferBinding   *bind = mRenderOp.vertexData->vertexBufferBinding;
	size_t offset = 0;

	// create/bind positions/tex.ccord. buffer
	if (!decl->findElementBySemantic(VES_POSITION))
		decl->addElement(POS_TEX_BINDING, offset, VET_FLOAT3, VES_POSITION);

	offset += VertexElement::getTypeSize(VET_FLOAT3);

	if (!decl->findElementBySemantic(VES_TEXTURE_COORDINATES))
		decl->addElement(POS_TEX_BINDING, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0);

	HardwareVertexBufferSharedPtr ptbuf = HardwareBufferManager::getSingleton().createVertexBuffer(decl->getVertexSize(POS_TEX_BINDING),
		mRenderOp.vertexData->vertexCount,
		HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
	bind->setBinding(POS_TEX_BINDING, ptbuf);

	// Colours - store these in a separate buffer because they change less often
	if (!decl->findElementBySemantic(VES_DIFFUSE))
		decl->addElement(COLOUR_BINDING, 0, VET_COLOUR, VES_DIFFUSE);

	HardwareVertexBufferSharedPtr cbuf = HardwareBufferManager::getSingleton().createVertexBuffer(decl->getVertexSize(COLOUR_BINDING),
		mRenderOp.vertexData->vertexCount,
		HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
	bind->setBinding(COLOUR_BINDING, cbuf);

	float *pPCBuff = static_cast<float*>(ptbuf->lock(HardwareBuffer::HBL_DISCARD));

	float largestWidth = 0;
	float left = 0 * 2.0 - 1.0;
	float top = -((0 * 2.0) - 1.0);

	// Derive space width from a capital A
	if (mSpaceWidth == 0)
		mSpaceWidth = mpFont->getGlyphAspectRatio('A') * mCharHeight * 2.0;

	// for calculation of AABB
	Ogre::Vector3 min, max, currPos;
	Ogre::Real maxSquaredRadius = 0;
	bool first = true;

	// Use iterator
	DisplayString::iterator i, iend;
	iend = mCaption.end();
	bool newLine = true;
	Real len = 0.0f;

	Real verticalOffset = 0;
	switch (mVerticalAlignment)
	{
	case MovableText::V_ABOVE:
		verticalOffset = mCharHeight;
		break;
	case MovableText::V_CENTER:
		verticalOffset = 0.5*mCharHeight;
		break;
	case MovableText::V_BELOW:
		verticalOffset = 0;
		break;
	}
	// Raise the first line of the caption
	top += verticalOffset;
	for (i = mCaption.begin(); i != iend; ++i)
	{
		Font::CodePoint character = OGRE_DEREF_DISPLAYSTRING_ITERATOR(i);
		if (character == UNICODE_CR
			|| character == UNICODE_NEL
			|| character == UNICODE_LF)
		{
			top += verticalOffset * 2.0f;
		}
	}

	for (i = mCaption.begin(); i != iend; ++i)
	{
		Font::CodePoint character = OGRE_DEREF_DISPLAYSTRING_ITERATOR(i);

		if (newLine)
		{
			len = 0.0f;
			for (DisplayString::iterator j = i; j != iend; j++)
			{
				Font::CodePoint cr = OGRE_DEREF_DISPLAYSTRING_ITERATOR(j);

				if (cr == UNICODE_CR
					|| cr == UNICODE_NEL
					|| cr == UNICODE_LF)
				{
					break;
				}
				else if (cr == UNICODE_SPACE) // space
				{
					len += mSpaceWidth;
				}
				else
				{
					len += mpFont->getGlyphAspectRatio(character) * mCharHeight * 2.0f;
				}
			}

			newLine = false;
		}

		if (character == UNICODE_CR
			|| character == UNICODE_NEL
			|| character == UNICODE_LF)
		{
			left = 0 * 2.0 - 1.0;
			top -= mCharHeight * 2.0;
			newLine = true;
			continue;
		}

		if (character == UNICODE_SPACE)
		{
			// Just leave a gap, no tris
			left += mSpaceWidth;
			// Also reduce tri count
			mRenderOp.vertexData->vertexCount -= 6;
			continue;
		}

		Real horiz_height = mpFont->getGlyphAspectRatio(*i);
		Real u1, u2, v1, v2;
		Ogre::Font::UVRect utmp;
		utmp = mpFont->getGlyphTexCoords(character);
		u1 = utmp.left;
		u2 = utmp.right;
		v1 = utmp.top;
		v2 = utmp.bottom;

		// each vert is (x, y, z, u, v)
		//-------------------------------------------------------------------------------------
		// First tri
		//
		// Upper left
		if(mHorizontalAlignment == MovableText::H_LEFT)
			*pPCBuff++ = left;
		else
			*pPCBuff++ = left - (len / 2);
		*pPCBuff++ = top;
		*pPCBuff++ = -1.0;
		*pPCBuff++ = u1;
		*pPCBuff++ = v1;

		// Deal with bounds
		if(mHorizontalAlignment == MovableText::H_LEFT)
			currPos = Ogre::Vector3(left, top, -1.0);
		else
			currPos = Ogre::Vector3(left - (len / 2), top, -1.0);
		if (first)
		{
			min = max = currPos;
			maxSquaredRadius = currPos.squaredLength();
			first = false;
		}
		else
		{
			min.makeFloor(currPos);
			max.makeCeil(currPos);
			maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());
		}

		top -= mCharHeight * 2.0;

		// Bottom left
		if(mHorizontalAlignment == MovableText::H_LEFT)
			*pPCBuff++ = left;
		else
			*pPCBuff++ = left - (len / 2);
		*pPCBuff++ = top;
		*pPCBuff++ = -1.0;
		*pPCBuff++ = u1;
		*pPCBuff++ = v2;

		// Deal with bounds
		if(mHorizontalAlignment == MovableText::H_LEFT)
			currPos = Ogre::Vector3(left, top, -1.0);
		else
			currPos = Ogre::Vector3(left - (len / 2), top, -1.0);
		min.makeFloor(currPos);
		max.makeCeil(currPos);
		maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());

		top += mCharHeight * 2.0;
		left += horiz_height * mCharHeight * 2.0;

		// Top right
		if(mHorizontalAlignment == MovableText::H_LEFT)
			*pPCBuff++ = left;
		else
			*pPCBuff++ = left - (len / 2);
		*pPCBuff++ = top;
		*pPCBuff++ = -1.0;
		*pPCBuff++ = u2;
		*pPCBuff++ = v1;
		//-------------------------------------------------------------------------------------

		// Deal with bounds
		if(mHorizontalAlignment == MovableText::H_LEFT)
			currPos = Ogre::Vector3(left, top, -1.0);
		else
			currPos = Ogre::Vector3(left - (len / 2), top, -1.0);
		min.makeFloor(currPos);
		max.makeCeil(currPos);
		maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());

		//-------------------------------------------------------------------------------------
		// Second tri
		//
		// Top right (again)
		if(mHorizontalAlignment == MovableText::H_LEFT)
			*pPCBuff++ = left;
		else
			*pPCBuff++ = left - (len / 2);
		*pPCBuff++ = top;
		*pPCBuff++ = -1.0;
		*pPCBuff++ = u2;
		*pPCBuff++ = v1;

		currPos = Ogre::Vector3(left, top, -1.0);
		min.makeFloor(currPos);
		max.makeCeil(currPos);
		maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());

		top -= mCharHeight * 2.0;
		left -= horiz_height * mCharHeight * 2.0;

		// Bottom left (again)
		if(mHorizontalAlignment == MovableText::H_LEFT)
			*pPCBuff++ = left;
		else
			*pPCBuff++ = left - (len / 2);
		*pPCBuff++ = top;
		*pPCBuff++ = -1.0;
		*pPCBuff++ = u1;
		*pPCBuff++ = v2;

		currPos = Ogre::Vector3(left, top, -1.0);
		min.makeFloor(currPos);
		max.makeCeil(currPos);
		maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());

		left += horiz_height * mCharHeight * 2.0;

		// Bottom right
		if(mHorizontalAlignment == MovableText::H_LEFT)
			*pPCBuff++ = left;
		else
			*pPCBuff++ = left - (len / 2);
		*pPCBuff++ = top;
		*pPCBuff++ = -1.0;
		*pPCBuff++ = u2;
		*pPCBuff++ = v2;
		//-------------------------------------------------------------------------------------

		currPos = Ogre::Vector3(left, top, -1.0);
		min.makeFloor(currPos);
		max.makeCeil(currPos);
		maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());

		// Go back up with top
		top += mCharHeight * 2.0;

		float currentWidth = (left + 1)/2 - 0;
		if (currentWidth > largestWidth)
			largestWidth = currentWidth;
	}

	// Unlock vertex buffer
	ptbuf->unlock();

	min.makeFloor(max); //When only spaces are typed in min can exceed max, this rectifies this

	// update AABB/Sphere radius
	mAABB = Ogre::AxisAlignedBox(min, max);
	mRadius = Ogre::Math::Sqrt(maxSquaredRadius);

	if (mUpdateColors)
		this->_updateColors();

	mNeedUpdate = false;
}

void MovableText::_updateColors(void)
{
	assert(!mpFont.isNull());
	assert(!mpMaterial.isNull());

	if (mpFont.isNull())
	{
		return;
	}

	if (mpMaterial.isNull())
	{
		return;
	}

	// Convert to system-specific
	RGBA color;
	Root::getSingleton().convertColourValue(mColor, &color);
	HardwareVertexBufferSharedPtr vbuf = mRenderOp.vertexData->vertexBufferBinding->getBuffer(COLOUR_BINDING);
	RGBA *pDest = static_cast<RGBA*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
	for (uint i = 0; i < mRenderOp.vertexData->vertexCount; ++i)
		*pDest++ = color;
	vbuf->unlock();
	mUpdateColors = false;
}

const Quaternion& MovableText::getWorldOrientation(void) const
{
	assert(mpCam);
	return const_cast<Quaternion&>(mpCam->getDerivedOrientation());
}

const Vector3& MovableText::getWorldPosition(void) const
{
	assert(mParentNode);
	return mParentNode->_getDerivedPosition();
}

void MovableText::getWorldTransforms(Matrix4 *xform) const
{
	if (this->isVisible() && mpCam)
	{
		Matrix3 rot3x3, scale3x3 = Matrix3::IDENTITY;

		// store rotation in a matrix
		mpCam->getDerivedOrientation().ToRotationMatrix(rot3x3);

		// parent node position
		Vector3 ppos = mParentNode->_getDerivedPosition() + Vector3::UNIT_Y*mAdditionalHeight;
		Quaternion pori = mParentNode->_getDerivedOrientation();
		Vector3 alignmentOffset = Vector3::ZERO;

		//ppos += pori * (mPositionOffset + alignmentOffset * 0.01);

		// apply scale
		scale3x3[0][0] = mParentNode->_getDerivedScale().x / 2;
		scale3x3[1][1] = mParentNode->_getDerivedScale().y / 2;
		scale3x3[2][2] = mParentNode->_getDerivedScale().z / 2;

		// apply all transforms to xform      
		*xform = (rot3x3 * scale3x3);
		xform->setTrans(ppos);
	}
}

void MovableText::getRenderOperation(RenderOperation &op)
{
	if (this->isVisible())
	{
		if (mNeedUpdate)
			this->_setupGeometry();
		if (mUpdateColors)
			this->_updateColors();
		op = mRenderOp;
	}
}

void MovableText::_notifyCurrentCamera(Camera *cam)
{
	mpCam = cam;
}

void MovableText::_updateRenderQueue(RenderQueue* queue)
{
	if (this->isVisible())
	{
		if (mNeedUpdate)
			this->_setupGeometry();
		if (mUpdateColors)
			this->_updateColors();

		queue->addRenderable(this, mRenderQueueID, OGRE_RENDERABLE_DEFAULT_PRIORITY);
	}
}

void Ogre::MovableText::setPositionOffset( const Ogre::Vector3& offset )
{
	mPositionOffset = offset;
}

void Ogre::MovableText::setScaleOffset( const Ogre::Vector3& offset )
{
	mScaleOffset = offset;
}

