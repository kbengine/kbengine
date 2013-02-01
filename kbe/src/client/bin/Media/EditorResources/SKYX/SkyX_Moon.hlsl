/*
--------------------------------------------------------------------------------
This source file is part of SkyX.
Visit ---

Copyright (C) 2009 Xavier Verguín González <xavierverguin@hotmail.com>
                                           <xavyiy@gmail.com>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
--------------------------------------------------------------------------------
*/

// --------------------- SkyX moon material ------------------------

void main_vp(
    // IN
	float4 iPosition	        : POSITION,
	float2 iUV                  : TEXCOORD0,
	// OUT
	out float4 oPosition		: POSITION,
	out float4 oUVYLength       : TEXCOORD0,
	// UNIFORM
	uniform float4x4 uWorldViewProj,
	uniform float4x4 uWorld,
	uniform float3   uSkydomeCenter)
{
    // Clip space position
	oPosition   = mul(uWorldViewProj, iPosition);
	// World position
	float3 ObjectSpacePosition = mul(uWorld, iPosition) - uSkydomeCenter;

    // UV
    oUVYLength.xy = iUV;
    // Y
    oUVYLength.z  = ObjectSpacePosition.y;
    // Length
    oUVYLength.w  = length(ObjectSpacePosition);
}

void main_fp(
    // IN
    float4 iUVYLength       : TEXCOORD0,
	// OUT 
	out float4 oColor		: COLOR,
	// UNIFORM
	uniform sampler2D uMoon : register(s0))
{
    // Output color
    oColor = tex2D(uMoon, iUVYLength.xy);
    oColor.w *= saturate((iUVYLength.z/iUVYLength.w)*3.5f);
}