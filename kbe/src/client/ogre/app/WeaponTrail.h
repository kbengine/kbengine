#pragma once
#include "Ogre.h"
#include<list>


class WeaponTrail
{
public:

	WeaponTrail(Ogre::String  name, Ogre::SceneManager* s );

	virtual ~WeaponTrail();

	/// Set the weapon entity to which the trail is attached
	void setWeaponEntity(Ogre::Entity* p_WeaponEntity);
	/// update the weapon trail
	void onUpdate(float p_DeltaT);
	/// Set the name of the material to use for the trail
	void setMaterialName(const Ogre::String& p_MaterialName);
	/// set the initial color of the start of segments
	void setSegmentStartInitialColor(const Ogre::ColourValue& p_Color);
	/// get the initial color of the start of segments
	const Ogre::ColourValue& getSegmentStartInitialColor() const;
	/// set the initial color of the end of segments
	void setSegmentEndInitialColor(const Ogre::ColourValue& p_Color);
	/// get the initial color of the end of segments
	const Ogre::ColourValue& getSegmentEndInitialColor() const;
	/// set how the color of the start of segments will change over time
	void setSegmentStartColorChange(const Ogre::ColourValue& p_ColorChange);
	/// get how the color of the start of segments will change over time
	const Ogre::ColourValue& getSegmentStartColorChange() const;
	/// set how the color of the start of segments will change over time
	void setSegmentEndColorChange(const Ogre::ColourValue& p_ColorChange);
	/// get how the color of the start of segments will change over time
	const Ogre::ColourValue& getSegmentEndColorChange() const;
	/// Return the max vertex count of the trail
	inline int getMaxSegmentCount() const {return m_MaxSegmentCount;}
	/// set the width
	void setWidth(float p_Width) {m_Width = p_Width;}
	/// get the width
	float getWidth() const {return m_Width;}
	/// Set whether new segments are added
	void setActive(bool p_Active = true);
	/// Get whether new segments are added
	bool isActive() const;
	/// Get whether there are currently segments in the list
	bool isVisible() const;

protected:

	/// a trail segment
	struct TrailSegment
	{
		/// start position
		Ogre::Vector3 segmentStart;
		/// end position
		Ogre::Vector3 segmentEnd;
		/// current segment start color
		Ogre::ColourValue segmentStartColor;
		/// current segment end color
		Ogre::ColourValue segmentEndColor;
	}; // end TrailSegment struct declaration

	/// typedef for a list of trail segments
	typedef std::list<TrailSegment> TrailSegmentList;
	/// the list of currently active trail segments
	TrailSegmentList m_SegmentList;

	/// Initializes the manual object
	void init();
	/// Uninitializes the manual object
	void uninit();

	Ogre::ManualObject* m_TrailObject;        //!< the dynamically changed mesh representing the trail
	Ogre::Entity* m_WeaponEntity;             //!< the entity representing the weapon;
	Ogre::Node* m_WeaponNode;                 //!< the node the tracked entity is attached to
	Ogre::SceneNode* m_TrailNode;             //!< the node the manual object is attached to
	Ogre::String m_MaterialName;              //!< the name of the material to use
	const int m_MaxSegmentCount;        //!< the maximum number of segments the trail will consist of
	Ogre::ColourValue m_SegmentStartInitialColor;   //!< the initial color of start segments
	Ogre::ColourValue m_SegmentEndInitialColor;     //!< the initial color of end segments
	Ogre::ColourValue m_SegmentStartColorChange;    //!< how the color of start segments will change over time
	Ogre::ColourValue m_SegmentEndColorChange;      //!< how the color of end segments will change over time
	float m_Width;                      //!< the width of the trail
	bool m_IsActive;                    //!< flag indicating whether new segments are generated

	Ogre::SceneManager* mSceneMgr;
	Ogre::String  mName;

}; // end of WeaponTrail class declaration

