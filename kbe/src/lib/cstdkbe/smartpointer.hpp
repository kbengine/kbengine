/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __SMARTPOINTER__
#define __SMARTPOINTER__
namespace KBEngine { 

template<class T>
inline void incrementReferenceCount(const T* obj)
{
};

template<class T>
inline void decrementReferenceCount(const T* obj)
{
	delete obj;
};

template<class T>
class ConstSmartPointer
{
public:
	enum REF_TAG {STEAL_REF, NEW_REF};
	ConstSmartPointer(const T* obj, REF_TAG tag = ConstSmartPointer::NEW_REF):
	m_obj_(obj)
	{
		if(tag == ConstSmartPointer::STEAL_REF)
		{
			incrementReferenceCount(m_obj_);
		}
	}
	
	~ConstSmartPointer()
	{
		if (m_obj_)
			decrementReferenceCount(m_obj_);

		m_obj_ = 0;
	}
	
	void clear()
	{
		if (m_obj_)
			decrementReferenceCount(m_obj_);
		
		m_obj_ = 0;
	}
	
	const T* get()
	{
		return m_obj_;
	}

	const T* operator()()
	{
		return m_obj_;
	}

	ConstSmartPointer( const ConstSmartPointer<T>& P )
	{
		m_obj_ = P.get();
		if (m_obj_) incrementReferenceCount( *m_obj_ );
	}

	ConstSmartPointer<T>& operator=( const ConstSmartPointer<T>& X )
	{
		if (m_obj_ != X.get())
		{
			const T* pOldObj = m_obj_;
			m_obj_ = X.get();
			if (m_obj_) incrementReferenceCount( *m_obj_ );
			if (pOldObj) decrementReferenceCount( *pOldObj );
		}
		return *this;
	}

	const T* operator->() const
	{
		return m_obj_;
	}
protected:
	const T* m_obj_;
};

template <class T>
class SmartPointer : public ConstSmartPointer<T>
{
public:
	typedef ConstSmartPointer<T> ConstProxy;

	SmartPointer(T* obj, REF_TAG tag = ConstSmartPointer::NEW_REF):
	ConstProxy(obj, tag)
	{
	}
	
	T * get() const
	{
		return const_cast<T *>( this->m_obj_ );
	}
};

}
#endif // __SMARTPOINTER__