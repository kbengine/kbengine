/**
* File: MovableText.h
*
* description: This create create a billboarding object that display a text.
* 
* @author	2003 by cTh see gavocanov@rambler.ru
* @update	2006 by barraq see nospam@barraquand.com
* @update	2012 to work with newer versions of OGRE by MindCalamity <mindcalamity@gmail.com>
*	- See "Notes" on: http://www.ogre3d.org/tikiwiki/tiki-editpage.php?page=MovableText
*/
 
#ifndef __include_MovableText_H__
#define __include_MovableText_H__
 
namespace Ogre {
 
	class MovableText : public MovableObject, public Renderable
	{
		/******************************** MovableText data ****************************/
	public:
		enum HorizontalAlignment    {H_LEFT, H_CENTER};
		enum VerticalAlignment      {V_BELOW, V_ABOVE, V_CENTER};
 
	protected:
		String					mFontName;
		String					mType;
		String					mName;
		String					mCaption;
		HorizontalAlignment		mHorizontalAlignment;
		VerticalAlignment		mVerticalAlignment;
 
		ColourValue				mColor;
		RenderOperation			mRenderOp;
		AxisAlignedBox			mAABB;
		LightList				mLList;
 
		Real					mCharHeight;
		Real					mSpaceWidth;
 
		bool					mNeedUpdate;
		bool					mUpdateColors;
		bool					mOnTop;
 
		Real					mTimeUntilNextToggle;
		Real					mRadius;
 
		Vector3					mGlobalTranslation;
		Vector3					mLocalTranslation;
 
		Camera					*mpCam;
		RenderWindow			*mpWin;
		Font					*mpFont;
		MaterialPtr				mpMaterial;
		MaterialPtr				mpBackgroundMaterial;
 
		/******************************** public methods ******************************/
	public:
		MovableText(const String &name, const String &caption, const String &fontName = "BlueHighway-8", Real charHeight = 1.0, const ColourValue &color = ColourValue::White);
		virtual ~MovableText();
 
		// Add to build on Shoggoth:
		virtual void				visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables = false);
 
		// Set settings
		void						setFontName(const String &fontName);
		void						setCaption(const String &caption);
		void						setColor(const ColourValue &color);
		void						setCharacterHeight(Real height);
		void						setSpaceWidth(Real width);
		void						setTextAlignment(const HorizontalAlignment& horizontalAlignment, const VerticalAlignment& verticalAlignment);
		void						setGlobalTranslation( Vector3 trans );
		void						setLocalTranslation( Vector3 trans );
		void						showOnTop(bool show=true);
 
		// Get settings
		const   String				&getFontName()	const {return mFontName;}
		const   String				&getCaption()	const {return mCaption;}
		const   ColourValue			&getColor()		const {return mColor;}
 
		Real						getCharacterHeight() const {return mCharHeight;}
		Real						getSpaceWidth() const {return mSpaceWidth;}
		Vector3						getGlobalTranslation() const {return mGlobalTranslation;}
		Vector3						getLocalTranslation() const {return mLocalTranslation;}
		bool						getShowOnTop() const {return mOnTop;}
		AxisAlignedBox				GetAABB(void) { return mAABB; }
 
		/******************************** protected methods and overload **************/
	protected:
 
		// from MovableText, create the object
		void						_setupGeometry();
		void						_updateColors();
 
		// from MovableObject
		void						getWorldTransforms(Matrix4 *xform) const;
		Real						getBoundingRadius(void) const {return mRadius;};
		Real						getSquaredViewDepth(const Camera *cam) const {return 0;};
		const   Quaternion			&getWorldOrientation(void) const;
		const   Vector3				&getWorldPosition(void) const;
		const   AxisAlignedBox		&getBoundingBox(void) const {return mAABB;};
		const   String				&getName(void) const {return mName;};
		const   String				&getMovableType(void) const {static Ogre::String movType = "MovableText"; return movType;};
 
		void						_notifyCurrentCamera(Camera *cam);
		void						_updateRenderQueue(RenderQueue* queue);
 
		// from renderable
		void						getRenderOperation(RenderOperation &op);
		const   MaterialPtr			&getMaterial(void) const {assert(!mpMaterial.isNull());return mpMaterial;};
		const   LightList			&getLights(void) const {return mLList;};
	};
 
} //end namespace Ogre
 
#endif
