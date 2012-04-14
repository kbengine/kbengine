/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __CIRCULAR_ARRAY_HPP__
#define __CIRCULAR_ARRAY_HPP__

#include "cstdkbe/cstdkbe.hpp"

namespace KBEngine { 
namespace Mercury
{

/** »·ÐÎ»º³å */
template <class T> class CircularBuffer
{
public:
	typedef CircularBuffer<T> OurType;

	CircularBuffer( uint size ) : data_( new T[size] ), mask_( size-1 ) 
	{ 
		memset( data_, 0, sizeof(T) * this->size() );
	}
	~CircularBuffer()	{ delete [] data_; }

	uint size() const	{ return mask_+1; }

	const T & operator[]( uint n ) const	{ return data_[n&mask_]; }
	T & operator[]( uint n )				{ return data_[n&mask_]; }
	void swap( OurType & other )
	{
		T * data = data_;
		uint mask = mask_;

		data_ = other.data_;
		mask_ = other.mask_;

		other.data_ = data;
		other.mask_ = mask;
	}

	void inflateToAtLeast( size_t newSize )
	{
		if (newSize > this->size())
		{
			size_t size = this->size();
			while (newSize > size)
			{
				size *= 2;
			}

			OurType newWindow( size );
			this->swap( newWindow );
		}
	}

	void doubleSize( uint32 startIndex )
	{
		OurType newWindow( this->size() * 2 );

		for (size_t i = 0; i < this->size(); ++i)
		{
			newWindow[ startIndex + i ] = (*this)[ startIndex + i ];
		}

		this->swap( newWindow );
	}
private:
	CircularBuffer( const OurType & other );
	OurType & operator=( const OurType & other );

	T* data_;
	uint mask_;
};

} 
}
#endif // __CIRCULAR_ARRAY_HPP__
