#ifndef __MovableText_H__
#define __MovableText_H__

#include <Ogre.h>
#include <OgreFont.h>
#include <OgreFontManager.h>

namespace Ogre
{

	class MovableText : public MovableObject, public Renderable
	{
	public:
		/// 水平对齐方式
		enum HorizontalAlignment
		{
			H_LEFT,   ///< 左对齐
			H_CENTER, ///< 居中
			H_RIGHT   ///< 右对齐
		};

		/// 垂直对齐方式
		enum VerticalAlignment
		{
			V_BELOW,///< 低端对齐
			V_ABOVE,///< 顶端对齐
			V_CENTER///< 居中
		};

	protected:
		String     mFontName;///<
		String     mType;///<
		String     mName;///<
		DisplayString   mCaption;///<
		HorizontalAlignment mHorizontalAlignment;///< 水平对齐方式
		VerticalAlignment mVerticalAlignment;///< 垂直对齐方式

		ColourValue    mColor;///<
		RenderOperation   mRenderOp;///<
		AxisAlignedBox   mAABB;///<
		LightList    mLList;///<

		Real     mCharHeight;///<
		Real     mSpaceWidth;///<

		bool     mNeedUpdate;///<
		bool     mUpdateColors;///<
		bool     mOnTop;///<

		Real     mTimeUntilNextToggle;///<
		Real     mRadius;///< 包围半径
		Real     mAdditionalHeight;///<

		Camera*     mpCam;///< 摄像机指针
		RenderWindow*   mpWin;///< 渲染窗口指针
		FontPtr     mpFont;///< 字体指针
		MaterialPtr    mpMaterial;///<
		MaterialPtr    mpBackgroundMaterial;///< 背景材质

		Vector3     mPositionOffset;///<
		Vector3     mScaleOffset;///<

	public:
		/// 构造函数
		/// @param[in] name 标识名
		/// @param[in] caption 字幕字符串
		/// @param[in] fontName 字体名
		/// @param[in] charHeight 字符高度
		/// @param[in] colour 字符颜色
		MovableText(const Ogre::String& name,
			const Ogre::DisplayString& caption,
			const Ogre::String& fontName = "BlueHighway",
			Ogre::Real charHeight = 1.0f,
			const Ogre::ColourValue& colour = Ogre::ColourValue::White);

		virtual ~MovableText(void);

		// Add to build on Shoggoth:
		virtual void				visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables = false);

		/// 设置字体名
		void    setFontName(const String &fontName);

		/// 设置显示字幕
		void    setCaption(const DisplayString &caption);

		/// 设置文字颜色
		void    setColor(const ColourValue &color);

		/// 设置文字高度
		void    setCharacterHeight(Real height);

		/// 设置间隔宽度
		void    setSpaceWidth(Real width);

		/// 设置文字对齐方式
		void    setTextAlignment(const HorizontalAlignment& horizontalAlignment,
			const VerticalAlignment& verticalAlignment);

		/// 设置
		void    setAdditionalHeight( Real height );

		/// 是否最前方显示
		void    showOnTop(bool show=true);

		///
		void setPositionOffset(const Ogre::Vector3& offset);

		///
		Ogre::Vector3 getPositionOffset() const { return mPositionOffset; }

		///
		void setScaleOffset(const Ogre::Vector3& offset);

		///
		Ogre::Vector3 getScaleOffset() const { return mScaleOffset; }

		/// 获取字体名
		const   String&    getFontName() const {return mFontName;}

		/// 获取字幕字符串
		const   DisplayString& getCaption() const {return mCaption;}

		/// 获取字体颜色
		const   ColourValue& getColor() const {return mColor;}

		/// 获取字符高度
		Real    getCharacterHeight() const {return mCharHeight;}

		/// 获取间隔宽度
		Real    getSpaceWidth() const {return mSpaceWidth;}

		///
		Real    getAdditionalHeight() const {return mAdditionalHeight;}

		/// 获取是否在最前方显示
		bool    getShowOnTop() const {return mOnTop;}

		/// 获取包围盒
		AxisAlignedBox         GetAABB(void) { return mAABB; }

	protected:
		// from MovableText, create the object
		void _setupGeometry();
		void _updateColors();

		/// 获取世界坐标系中的变换
		void getWorldTransforms(Matrix4 *xform) const;

		/// 获取包围半径
		Real getBoundingRadius(void) const {return mRadius;};

		/// 获取摄像机的视深
		Real getSquaredViewDepth(const Camera *cam) const {return 0;};

		/// 获取世界坐标系中的朝向
		/// @note 一直面朝摄像机
		const   Quaternion&    getWorldOrientation(void) const;

		/// 获取在世界坐标系中的坐标
		const   Vector3&    getWorldPosition(void) const;

		/// 获取包围盒
		const   AxisAlignedBox&   getBoundingBox(void) const {return mAABB;};

		/// 获取标识名
		const   String&     getName(void) const {return mName;};

		/// 获取类型名
		const   String&     getMovableType(void) const {static Ogre::String movType = "MovableText"; return movType;};

		void    _notifyCurrentCamera(Camera *cam);
		void    _updateRenderQueue(RenderQueue* queue);

		// from renderable
		void    getRenderOperation(RenderOperation &op);
		const   MaterialPtr& getMaterial(void) const {assert(!mpMaterial.isNull());return mpMaterial;};
		const   LightList&   getLights(void) const {return mLList;};
	};

}

#endif
