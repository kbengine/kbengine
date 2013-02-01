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

// --------------------- SkyX skydome material ------------------------

float scale(float cos, float uScaleDepth)
{
	float x = 1.0 - cos;
	return uScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

void main_vp(
    // IN
	float4 iPosition	        : POSITION,
	float3 iNPosition           : TEXCOORD0,
	float2 iUV                  : TEXCOORD1,
	float  iOpacity             : TEXCOORD2,
	// OUT
	out float4 oPosition		: POSITION,
	out float2 oUV              : TEXCOORD0,
	out float3 oRayleighColor   : TEXCOORD1,
	out float3 oMieColor        : TEXCOORD2,
	out float3 oDirection       : TEXCOORD3,
	out float  oOpacity         : TEXCOORD4,
	out float  oHeight          : TEXCOORD5,
	// UNIFORM
	uniform float4x4 uWorldViewProj,
	// Global information
	uniform float3 uLightDir,
	// Position information
	uniform float3 uCameraPos,
	uniform float3 uInvWaveLength,
	uniform float  uInnerRadius,
	// Scattering parameters
	uniform float  uKrESun, // Kr * ESun
	uniform float  uKmESun, // Km * ESun
	uniform float  uKr4PI,  // Kr * 4 * PI
	uniform float  uKm4PI,  // Km * 4 * PI
	// Atmosphere properties
	uniform float uScale,               // 1 / (outerRadius - innerRadius)
	uniform float uScaleDepth,          // Where the average atmosphere density is found
	uniform float uScaleOverScaleDepth, // Scale / ScaleDepth
	// Number of samples
	uniform int   uNumberOfSamples,
	uniform float uSamples)
{
    // Clip space position
	oPosition   = mul(uWorldViewProj, iPosition);

	float3 v3Pos = iNPosition;
	v3Pos.y += uInnerRadius;

    float3 v3Ray = v3Pos - uCameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;
	
	// Calculate the ray's starting position, then calculate its scattering offset
	float3 v3Start = uCameraPos;
	float fHeight = uCameraPos.y;
	float fStartAngle = dot(v3Ray, v3Start) / fHeight;
	// NOTE: fDepth is not pased as parameter(like a constant) to avoid the little precission issue (Apreciable)
	float fDepth = exp(uScaleOverScaleDepth * (uInnerRadius - uCameraPos.y));
	float fStartOffset = fDepth * scale(fStartAngle, uScaleDepth);

    // Init loop variables
	float fSampleLength = fFar / uSamples;
	float fScaledLength = fSampleLength * uScale;
	float3 v3SampleRay = v3Ray * fSampleLength;
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5f;
	
	// Loop the ray
	float3 color;
	for (int i = 0; i < uNumberOfSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(uScaleOverScaleDepth * (uInnerRadius-fHeight));
		
		float fLightAngle = dot(uLightDir, v3SamplePoint) / fHeight;
		float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
		
		float fScatter = (fStartOffset + fDepth*(scale(fLightAngle, uScaleDepth) - scale(fCameraAngle, uScaleDepth)));
		float3 v3Attenuate = exp(-fScatter * (uInvWaveLength * uKr4PI + uKm4PI)); // <<< TODO
		
		// Accumulate color
		v3Attenuate *= (fDepth * fScaledLength);
		color += v3Attenuate;
		
		// Next sample point
		v3SamplePoint += v3SampleRay;
	}

    // Outputs
    oRayleighColor = color * (uInvWaveLength * uKrESun); // TODO <--- parameter
    oMieColor      = color * uKmESun; // TODO <--- *uInvMieWaveLength
    oDirection     = uCameraPos - v3Pos;
    oUV = iUV;
    oOpacity = iOpacity;
    oHeight = 1-iNPosition.y;
}

void main_fp(
    // IN
    float2 iUV              : TEXCOORD0,
	float3 iRayleighColor   : TEXCOORD1,
	float3 iMieColor        : TEXCOORD2,
	float3 iDirection       : TEXCOORD3,
	float  iOpacity         : TEXCOORD4,
	float  iHeight          : TEXCOORD5,
	// OUT 
	out float4 oColor		: COLOR,
	// UNIFORM
#ifdef STARFIELD
	uniform float  uTime,
#endif // STARFIELD
	uniform float3 uLightDir,
    // Phase function
	uniform float  uG,
	uniform float  uG2
	// Exposure
#ifdef LDR 
    ,
	uniform float  uExposure
#endif // LDR
#ifdef STARFIELD
    ,
	uniform sampler2D uStarfield : register(s0)
#endif // STARFIELD
	)
{
    float cos = dot(uLightDir, iDirection) / length(iDirection);
	float cos2 = cos*cos;
	
	float rayleighPhase = 0.75 * (1.0 + 0.5*cos2);
	
	float miePhase = 1.5f * ((1.0f - uG2) / (2.0f + uG2)) * // <<< TODO
					 (1.0f + cos2) / pow(1.0f + uG2 - 2.0f * uG * cos, 1.5f);

#ifdef LDR
	oColor = float4((1 - exp(-uExposure * (rayleighPhase * iRayleighColor + miePhase * iMieColor))), iOpacity);
#else // HDR
    oColor = float4(rayleighPhase * iRayleighColor + miePhase * iMieColor, iOpacity);
#endif // LDR
	
	// For night rendering
	float nightmult = saturate(1 - max(oColor.x, max(oColor.y, oColor.z))*10);
	
#ifdef STARFIELD
	oColor.xyz += nightmult *(float3(0.05, 0.05, 0.1)*(2-0.75*saturate(-uLightDir.y))*pow(iHeight,3) + tex2D(uStarfield, iUV+uTime)*(0.35f + saturate(-uLightDir.y*0.45f))); 
#else // NO STARFIELD
	oColor.xyz += nightmult *(float3(0.05, 0.05, 0.1)*(2-0.75*saturate(-uLightDir.y))*pow(iHeight,3)); 
#endif // STARFIELD	
}