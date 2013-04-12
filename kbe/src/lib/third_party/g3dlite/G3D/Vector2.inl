/**
 @file Vector2.inl
 
 @maintainer Morgan McGuire, matrix@graphics3d.com
 @cite Portions by Laura Wollstadt, graphics3d.com
 
 @cite Portions based on Dave Eberly'x Magic Software Library
 at http://www.magic-software.com
 
 
 @created 2001-06-02
 @edited  2006-01-14

  Copyright 2000-2006, Morgan McGuire.
  All rights reserved.
 */
namespace G3D {
//----------------------------------------------------------------------------
inline void Vector2::normalise()
{
    float fLength = length();

	if (!(fLength < 0.00000001f && fLength > -0.00000001f))
	{
        float fInvLength = 1.f / fLength;
        x *= fInvLength;
        y *= fInvLength;
	}
}


}

