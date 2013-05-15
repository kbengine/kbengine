void shadow_caster_vs(
	float4 position			: POSITION,

	out float4 oPosition	: SV_POSITION,
	out float2 oDepth		: TEXCOORD0,

	uniform float4x4 wvpMat)
{
	// this is the view space position
	oPosition = mul(wvpMat, position);

	// depth info for the fragment.
	oDepth.x = oPosition.z;
	oDepth.y = oPosition.w;

	// clamp z to zero. seem to do the trick. :-/
	//oPosition.z = max(oPosition.z, 0);
}

void shadow_caster_ps(
  float4 Position	: SV_POSITION,
	float2 depth		: TEXCOORD0,

	out float4 oColour	: COLOR,

	uniform float4 pssmSplitPoints)
{
	float finalDepth = depth.x / depth.y;
//	oColour = float4(finalDepth, finalDepth, finalDepth, 1);
	oColour = float4(0, 0, 0, 1);
}



void shadow_receiver_vs(
	float4 position				: POSITION,
	float3 normal				: NORMAL,
	float2 uv					: TEXCOORD0,

	out float4 oPosition		: SV_POSITION,
	out float3 oUv				: TEXCOORD0,
	out float3 oLightDir		: TEXCOORD1,
	out float3 oHalfAngle		: TEXCOORD2,
	out float4 oLightPosition0	: TEXCOORD3,
	out float4 oLightPosition1	: TEXCOORD4,
	out float4 oLightPosition2	: TEXCOORD5,
	out float3 oNormal			: TEXCOORD6,

	uniform float4 lightPosition,				// object space
	uniform float3 eyePosition,					// object space
	uniform float4x4 worldViewProjMatrix,
	uniform float4x4 texWorldViewProjMatrix0,
	uniform float4x4 texWorldViewProjMatrix1,
	uniform float4x4 texWorldViewProjMatrix2)
{
	// calculate output position
	oPosition = mul(worldViewProjMatrix, position);

	// pass the main uvs straight through unchanged
	oUv.xy = uv;
	oUv.z = oPosition.z;

	// calculate tangent space light vector
	// Get object space light direction
	oLightDir = normalize(lightPosition.xyz - (position * lightPosition.w).xyz);

	// Calculate half-angle in tangent space
	float3 eyeDir = normalize(eyePosition - position.xyz);
	oHalfAngle = normalize(eyeDir + oLightDir);	

	// Calculate the position of vertex in light space
	oLightPosition0 = mul(texWorldViewProjMatrix0, position);
	oLightPosition1 = mul(texWorldViewProjMatrix1, position);
	oLightPosition2 = mul(texWorldViewProjMatrix2, position);

	oNormal = normal;
}

float shadowPCF(sampler2D shadowMap, float4 shadowMapPos, float2 offset)
{
	shadowMapPos = shadowMapPos / shadowMapPos.w;
	float2 uv = shadowMapPos.xy;
	float3 o = float3(offset, -offset.x) * 0.3f;

	// Note: We using 2x2 PCF. Good enough and is alot faster.
	float c =	(shadowMapPos.z <= tex2D(shadowMap, uv.xy - o.xy).r)>0 ? 1 : 0; // top left
	c +=		(shadowMapPos.z <= tex2D(shadowMap, uv.xy + o.xy).r)>0 ? 1 : 0; // bottom right
	c +=		(shadowMapPos.z <= tex2D(shadowMap, uv.xy + o.zy).r)>0 ? 1 : 0; // bottom left
	c +=		(shadowMapPos.z <= tex2D(shadowMap, uv.xy - o.zy).r)>0 ? 1 : 0; // top right
	//float c =	(shadowMapPos.z <= tex2Dlod(shadowMap, uv.xyyy - o.xyyy).r) ? 1 : 0; // top left
	//c +=		(shadowMapPos.z <= tex2Dlod(shadowMap, uv.xyyy + o.xyyy).r) ? 1 : 0; // bottom right
	//c +=		(shadowMapPos.z <= tex2Dlod(shadowMap, uv.xyyy + o.zyyy).r) ? 1 : 0; // bottom left
	//c +=		(shadowMapPos.z <= tex2Dlod(shadowMap, uv.xyyy - o.zyyy).r) ? 1 : 0; // top right
	return c / 4;
}

// to put it simply, this does 100% per pixel diffuse lighting
void shadow_receiver_ps(
  float4 oPosition		: SV_POSITION,
	float3 uv				: TEXCOORD0,
	float3 OSlightDir			: TEXCOORD1,
	float3 OShalfAngle		: TEXCOORD2,
	float4 LightPosition0	: TEXCOORD3,
	float4 LightPosition1	: TEXCOORD4,
	float4 LightPosition2	: TEXCOORD5,
	float3 normal			: TEXCOORD6,

	out float4 oColour		: COLOR,

	uniform float4 invShadowMapSize0,
	uniform float4 invShadowMapSize1,
	uniform float4 invShadowMapSize2,
	uniform float4 pssmSplitPoints,
	uniform sampler2D diffuse:register(s3),
	uniform sampler2D specular:register(s4),
	uniform sampler2D normalMap:register(s5),
	uniform sampler2D shadowMap0:register(s0),
	uniform sampler2D shadowMap1:register(s1),
	uniform sampler2D shadowMap2:register(s2),
	uniform float4 lightDiffuse,
	uniform float4 lightSpecular,
	uniform float4 ambient
	)
{
	// calculate shadow
	float shadowing = 1.0f;
	float4 splitColour;
	if (uv.z <= pssmSplitPoints.y)
	{
		splitColour = float4(0.1, 0, 0, 1);
		shadowing = shadowPCF(shadowMap0, LightPosition0, invShadowMapSize0.xy);
	}
	else if (uv.z <= pssmSplitPoints.z)
	{
		splitColour = float4(0, 0.1, 0, 1);
		shadowing = shadowPCF(shadowMap1, LightPosition1, invShadowMapSize1.xy);
	}
	else
	{
		splitColour = float4(0.1, 0.1, 0, 1);
		shadowing = shadowPCF(shadowMap2, LightPosition2, invShadowMapSize2.xy);
	}

	// retrieve normalised light vector, expand from range-compressed
	float3 lightVec = normalize(OSlightDir);

	// retrieve half angle and normalise through cube map
	float3 halfAngle = normalize(OShalfAngle);

	// get diffuse colour
	float4 diffuseColour = tex2D(diffuse, uv.xy);

	// specular
	float4 specularColour = tex2D(specular, uv.xy);
	float shininess = specularColour.w;
	specularColour.w = 1;

	// calculate lit value.
	float4 lighting = lit(dot(normal, lightVec), dot(normal, halfAngle), shininess * 128) * shadowing;

	// final lighting with diffuse and spec
	oColour = (diffuseColour * clamp(ambient + lightDiffuse * lighting.y, 0, 1)) + (lightSpecular * specularColour * lighting.z);
	oColour.w = diffuseColour.w;

	//oColour += splitColour;
}

