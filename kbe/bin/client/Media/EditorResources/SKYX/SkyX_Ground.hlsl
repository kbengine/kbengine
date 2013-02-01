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

// --------------------- SkyX ground material ------------------------

float scale(float cos, float uScaleDepth)
{
	float x = 1.0 - cos;
	return uScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

void main_vp(
    // IN
	float4 iPosition	        : POSITION,
	// OUT
	out float4 oPosition		: POSITION,
	out float3 oRayleighColor   : TEXCOORD0,
	// UNIFORM
	uniform float4x4 uWorldViewProj,
    uniform float4x4 uWorld,
	// Global information
	uniform float3 uLightDir,
	// Position information
	uniform float3 uCameraPos,
	uniform float3 uCameraPos_,
	uniform float3 uInvWaveLength,
	uniform float  uInnerRadius,
	// Scattering parameters
	uniform float  uKrESun, // Kr * ESun
	uniform float  uKr4PI,  // Kr * 4 * PI
	uniform float  uKm4PI,  // Km * 4 * PI
	// Atmosphere properties
	uniform float uScale,               // 1 / (outerRadius - innerRadius)
	uniform float uScaleDepth,          // Where the average atmosphere density is found
	uniform float uScaleOverScaleDepth, // Scale / ScaleDepth
	uniform float uSkydomeRadius,
	// Number of samples
	uniform int   uNumberOfSamples,
	uniform float uSamples)
{
    // Clip space position
	oPosition = mul(uWorldViewProj, iPosition);
	
	// Calculate vertex world position
	float3 vertexWorldPos = mul(uWorld, iPosition);
	
	// Get the ray from the camera to the vertex, and its length (which is the far point of the ray passing through the atmosphere)
	float3 v3Pos;
	v3Pos.xz = (vertexWorldPos.xz-uCameraPos_.xz) / uSkydomeRadius;
	v3Pos.y = uCameraPos.y + vertexWorldPos.y / uSkydomeRadius;
	
	float3 v3Ray = v3Pos - uCameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;
	
	// Calculate the ray's starting position, then calculate its scattering offset
	float3 v3Start = uCameraPos; // Creo k ai k ajustar la posicion y del pixel
	float fDepth = exp((uInnerRadius - uCameraPos.y) / uScaleDepth);
	float fCameraAngle = dot(v3Ray, uCameraPos) / length(v3Pos);
	float fLightAngle = dot(normalize(uLightDir), v3Pos) / length(v3Pos);
	float fCameraScale = scale(fCameraAngle, uScaleDepth);
	float fLightScale = scale(fLightAngle, uScaleDepth);
	float fCameraOffset = fDepth*fCameraScale;
	float fTemp = (fLightScale + fCameraScale);
	
    // Init loop variables
	float fSampleLength = fFar / uSamples;
	float fScaledLength = fSampleLength * uScale;
	float3 v3SampleRay = v3Ray * fSampleLength;
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5f;
	
	// Loop the ray
	float3 color = float3(0,0,0);
	for (int i = 0; i < uNumberOfSamples; i++)
	{
    	float fHeight = length(v3SamplePoint);
		float fDepth = exp(uScaleOverScaleDepth * (uInnerRadius-fHeight));
		float fScatter = fDepth*fTemp - fCameraOffset;
		float3 v3Attenuate = exp(-fScatter * (uInvWaveLength * uKr4PI + uKm4PI)); // <<< TODO
		
		color += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}

    // Outputs
    oRayleighColor = color * (uInvWaveLength * uKrESun); // TODO <--- parameter
}

void main_fp(
    // IN
	float3 iRayleighColor   : TEXCOORD0,
	// OUT 
	out float4 oColor		: COLOR
	// UNIFORM
#ifdef LDR
	,uniform float  uExposure
#endif // LDR
	)
{	
	oColor = float4(iRayleighColor,1);
	
#ifdef LDR
	oColor.xyz = 1 - exp(-uExposure * oColor);
#endif // LDR
}