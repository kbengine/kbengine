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

// ------------------------- SkyX volumetric clouds -----------------------------

void main_vp(
    // IN
	float4 iPosition	        : POSITION,
	float3 i3DCoord             : TEXCOORD0,
	float2 iNoiseUV             : TEXCOORD1,
	float  iOpacity             : TEXCOORD2,
	// OUT
	out float4 oPosition		: POSITION,
	out float3 o3DCoord         : TEXCOORD0,
	out float2 oNoiseUV         : TEXCOORD1,
	out float  oOpacity         : TEXCOORD2,
	out float3 oEyePixel        : TEXCOORD3,
	out float  oDistance        : TEXCOORD4,
	// UNIFORM
	uniform float4x4 uWorldViewProj,
	uniform float3   uCameraPosition,
	uniform float    uRadius)
{
    // Clip space position
	oPosition   = mul(uWorldViewProj, iPosition);

	// 3D coords
	o3DCoord = i3DCoord;
    // Noise coords
    oNoiseUV = iNoiseUV;
    // Opacity
    oOpacity = iOpacity;
    // Eye to pixel vector
    oEyePixel = normalize(iPosition.xyz - uCameraPosition);
    
    // Distance in [0,1] range
    oDistance = length(float2(iPosition.x, iPosition.z)) / uRadius;
}

void main_fp(
    // IN
    float3 i3DCoord  : TEXCOORD0,
    float2 iNoiseUV  : TEXCOORD1,
    float  iOpacity  : TEXCOORD2,
    float3 iEyePixel : TEXCOORD3,
    float  iDistance : TEXCOORD4,
	// OUT 
	out float4 oColor		: COLOR,
	// UNIFORM
	uniform float     uInterpolation,
	uniform float3    uSunDirection,
	uniform float3    uAmbientColor,
	uniform float3    uSunColor,
	uniform float4    uLightResponse,
	uniform float4    uAmbientFactors,
	uniform sampler3D uDensity0 : register(s0),
	uniform sampler3D uDensity1 : register(s1),
	uniform sampler2D uNoise    : register(s2))
{    
    // x - Sun light power
    // y - Sun beta multiplier
    // z - Ambient color multiplier
    // w - Distance attenuation
	// uLightResponse = float4(0.25,0.2,1,0.1);
	
	// Ambient light factors
	// x - constant, y - linear, z - cuadratic, w - cubic
	// float4 uAmbientFactors = float4(0.4,1,1,1);

	float3 Noise = tex2D(uNoise, iNoiseUV*5);
	float3 Final3DCoord = i3DCoord+0.002575*(Noise-0.5f)*2;
	Final3DCoord.z = saturate(Final3DCoord.z);
	
	float3 Density0 = tex3D(uDensity0, Final3DCoord);
	float3 Density1 = tex3D(uDensity1, Final3DCoord);
	float3 Density  = Density0*(1-uInterpolation) + Density1*uInterpolation;
	
	float3 finalcolor = float3(0,0,0);
	float  Opacity    = 0;
	
	if (Density.x > 0)
	{
	    float cos0 = saturate(dot(uSunDirection,iEyePixel));
	    float c3=cos0*cos0;
	    c3*=c3;
	
		float Beta = c3*uLightResponse.y*(0.5f+0.5*Density.y);

		float sunaccumulation = saturate( Beta+Density.y*uLightResponse.x+pow(iDistance,1.5)*uLightResponse.w );
		float ambientaccumulation = 
			  saturate(uAmbientFactors.x + uAmbientFactors.y*i3DCoord.z + uAmbientFactors.z*pow(i3DCoord.z,2) + uAmbientFactors.w*pow(i3DCoord.z,3))               *uLightResponse.z;
	    
		finalcolor = uAmbientColor*ambientaccumulation + uSunColor*sunaccumulation;
		Opacity = (1 - exp(-Density.x*(7.5-6.5*i3DCoord.z)))*iOpacity;
	}
	
    oColor = float4(finalcolor, Opacity);
    

 //oColor.xyz*=0.0001;
 // oColor.a = saturate(oColor.a+1)*iOpacity;
 // oColor.xyz+=Noise;
//oColor.rgb*=0.0001;oColor.r = dot(uLightDirection,iEyePixel);
}