/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KBE_SMARTPOINTER_H
#define KBE_SMARTPOINTER_H

#include "common/refcountable.h"

namespace KBEngine { 

template<class T>
inline void incrementReferenceCount(const T& obj)
{
	obj.incRef();
};

template<class T>
inline void decrementReferenceCount(const T& obj)
{
	obj.decRef();
};

template<class T>
class ConstSmartPointer
{
public:
	enum REF_TAG {STEAL_REF, NEW_REF};
	
	ConstSmartPointer(const T* obj = 0, REF_TAG tag = ConstSmartPointer::NEW_REF):
	obj_(obj)
	{
		if(obj)
		{
			if(tag == ConstSmartPointer::NEW_REF)
			{
				incrementReferenceCount(*obj_);
			}
		}
	}

	ConstSmartPointer( const ConstSmartPointer<T>& P )
	{
		obj_ = P.get();
		if (obj_) 
			incrementReferenceCount(*obj_);
	}

	~ConstSmartPointer()
	{
		if (obj_)
			decrementReferenceCount(*obj_);

		obj_ = 0;
	}
	
	void clear()
	{
		if (obj_)
			decrementReferenceCount(*obj_);
		
		obj_ = 0;
	}
	
	const T* get() const
	{
		return obj_;
	}

	const T* operator()()
	{
		return obj_;
	}

	ConstSmartPointer<T>& operator=( const ConstSmartPointer<T>& X )
	{
		if (obj_ != X.get())
		{
			const T* pOldObj = obj_;
			obj_ = X.get();
			if (obj_) incrementReferenceCount( *obj_ );
			if (pOldObj) decrementReferenceCount( *pOldObj );
		}
		return *this;
	}

	const T* operator->() const
	{
		return obj_;
	}

	const T& operator*() const
	{
		return *obj_;
	}

	/**
	 *	These functions return whether or not the input objects refer to the same
	 *	object.
	 */
	friend bool operator==( const ConstSmartPointer<T>& A,
		const ConstSmartPointer<T>& B )
	{
		return A.obj_ == B.obj_;
	}

	friend bool operator==( const ConstSmartPointer<T>& A,
		const T* B )
	{
		return A.obj_ == B;
	}

	friend bool operator==( const T* A,
		const ConstSmartPointer<T>& B )
	{
		return A == B.obj_;
	}
	/**
	 *	These functions return not or whether the input objects refer to the same
	 *	object.
	 */
	friend bool operator!=( const ConstSmartPointer<T>& A,
		const ConstSmartPointer<T>& B )
	{
		return A.obj_ != B.obj_;
	}

	friend bool operator!=( const ConstSmartPointer<T>& A,
		const T* B )
	{
		return A.obj_ != B;
	}

	friend bool operator!=( const T* A,
		const ConstSmartPointer<T>& B )
	{
		return A != B.obj_;
	}

	/**
	 *	These functions give an ordering on smart pointers so that they can be
	 *	placed in sorted containers.
	 */
	friend bool operator<( const ConstSmartPointer<T>& A,
		const ConstSmartPointer<T>& B )
	{
		return A.obj_ < B.obj_;
	}

	friend bool operator<( const ConstSmartPointer<T>& A,
		const T* B )
	{
		return A.obj_ < B;
	}

	friend bool operator<( const T* A,
		const ConstSmartPointer<T>& B )
	{
		return A < B.obj_;
	}

	/**
	 *	These functions give an ordering on smart pointers so that they can be
	 *	compared.
	 */
	friend bool operator>( const ConstSmartPointer<T>& A,
		const ConstSmartPointer<T>& B )
	{
		return A.obj_ > B.obj_;
	}

	friend bool operator>( const ConstSmartPointer<T>& A,
		const T* B )
	{
		return A.obj_ > B;
	}

	friend bool operator>( const T* A,
		const ConstSmartPointer<T>& B )
	{
		return A > B.obj_;
	}

	/**
	 *	This method returns whether or not this pointers points to anything.
	 */
	typedef const T * ConstSmartPointer<T>::*unspecified_bool_type;
	operator unspecified_bool_type() const
	{
		return obj_ == 0? 0 : &ConstSmartPointer<T>::obj_;
	}
protected:
	const T* obj_;
};

template <class T>
class SmartPointer : public ConstSmartPointer<T>
{
public:
	typedef ConstSmartPointer<T> ConstProxy;
		
	SmartPointer(T* obj = 0, typename ConstProxy::REF_TAG tag = ConstProxy::NEW_REF):
	ConstProxy(obj, tag)
	{
	}
	
	SmartPointer( const SmartPointer<T>& P ) : ConstProxy( P ) { }

	template<class DerivedType>
	SmartPointer( ConstSmartPointer<DerivedType>& dt ) :
		ConstProxy( dt.get() )
	{
	}

	SmartPointer<T>& operator=( const SmartPointer<T>& P )
	{
		ConstProxy::operator=(P);
		return *this;
	}

	template<class DerivedType>
	SmartPointer<T>& operator=( ConstSmartPointer<DerivedType>& dt )
	{
		ConstProxy::operator=(dt.get());
		return *this;
	}

	T * get() const
	{
		return const_cast<T *>( this->obj_ );
	}

	T* operator->() const
	{
		return const_cast<T *>( this->obj_ );
	}

	T& operator*() const
	{
		return *const_cast<T *>( this->obj_ );
	}
};

}
#endif // KBE_SMARTPOINTER_H
