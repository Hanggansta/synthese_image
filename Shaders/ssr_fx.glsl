
#version 330

#ifdef VERTEX_SHADER
const vec2 quadVertices[4] = vec2[4]( vec2( -1.0, -1.0), vec2( 1.0, -1.0), vec2( -1.0, 1.0), vec2( 1.0, 1.0));

out vec2 vtexcoord;

void main( )
{
	gl_Position = vec4(quadVertices[gl_VertexID], 0.0, 1.0);
	vtexcoord = (quadVertices[gl_VertexID] + 1.0) / 2.0;
}
#endif




#ifdef FRAGMENT_SHADER
uniform sampler2D colorBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D depthBuffer;
uniform vec2 renderSize;
uniform mat4 viewMatrix;
uniform mat4 projToPixel;
uniform mat4 invProj;
uniform float nearZ;
uniform float farZ;

const float maxSteps = 256;
const float binarySearchIterations = 5;
const float maxDistance = 100.0;
const float stride = 8.0;
const float zThickness = 1.0;
const float strideZCutoff = 100.0f;
const float screenEdgeFadeStart = 0.75f;
const float eyeFadeStart = 0.5f;
const float eyeFadeEnd = 1.0f;

in vec2 vtexcoord;

out vec4 pixelColor;

float DistanceSquared(vec2 a, vec2 b) 
{ 
	a -= b; 
	return dot(a, a); 
}

float Linear01Depth(float z)
{
	float temp = farZ / nearZ;
	return 1.0 / ((1.0 - temp) * z + temp);
}

float LinearEyeDepth(float z)
{
	float temp = farZ / nearZ;
	return 1.0 / ((1.0 - temp) / farZ * z + temp / farZ);
}
 
bool FindSSRHit(vec3 csOrig, vec3 csDir, float jitter,
	out vec2 hitPixel, out vec3 hitPoint, out float iterations) 
{
	// Clip to the near plane
	float rayLength = ((csOrig.z + csDir.z * maxDistance) > -nearZ) ?
		(-nearZ - csOrig.z) / csDir.z : maxDistance;
	vec3 csEndPoint = csOrig + csDir * rayLength;
	//if(csEndPoint.z > csOrig.z)
	//	return false;

	// Project into homogeneous clip space
	vec4 H0 = projToPixel * vec4(csOrig, 1.0);
	vec4 H1 = projToPixel * vec4(csEndPoint, 1.0);
	float k0 = 1.0 / H0.w, k1 = 1.0 / H1.w;

	// The interpolated homogeneous version of the camera-space points  
	vec3 Q0 = csOrig * k0, Q1 = csEndPoint * k1;

	// Screen-space endpoints
	vec2 P0 = H0.xy * k0, P1 = H1.xy * k1;

	// If the line is degenerate, make it cover at least one pixel
	// to avoid handling zero-pixel extent as a special case later
	P1 += vec2((DistanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);
	vec2 delta = P1 - P0;

	// Permute so that the primary iteration is in x to collapse
	// all quadrant-specific DDA cases later
	bool permute = false;
	if (abs(delta.x) < abs(delta.y)) 
	{
		// This is a more-vertical line
		permute = true;
		delta = delta.yx;
		P0 = P0.yx;
		P1 = P1.yx; 
	}
	
	float stepDir = sign(delta.x);
	float invdx = stepDir / delta.x;
	
	// Track the derivatives of Q and k
	vec3  dQ = (Q1 - Q0) * invdx;
	float dk = (k1 - k0) * invdx;
	vec2  dP = vec2(stepDir, delta.y * invdx);

	float strideScaler = 1.0 - min( 1.0, -csOrig.z / strideZCutoff);
	float pixelStride = 1.0 + strideScaler * stride;

	// Scale derivatives by the desired pixel stride and then
	// offset the starting values by the jitter fraction
	dP *= pixelStride; dQ *= pixelStride; dk *= pixelStride;
	P0 += dP * jitter; Q0 += dQ * jitter; k0 += dk * jitter;

	float end = P1.x * stepDir;
	float i, zA = csOrig.z, zB = csOrig.z;
	vec4 pqk = vec4(P0, Q0.z, k0);
	vec4 dPQK = vec4(dP, dQ.z, dk);
	bool intersect = false;
	for (i = 0; i < maxSteps && intersect == false && pqk.x * stepDir <= end; i++) 
	{
		pqk += dPQK;

		zA = zB;
		zB = (dPQK.z * 0.5 + pqk.z) / (dPQK.w * 0.5 + pqk.w);
		/*if (zB > zA) 
		{ 
			float t = zA;
			zA = zB;
			zB = t;
		}*/
 
		hitPixel = permute ? pqk.yx : pqk.xy;
		//hitPixel.y = renderSize.y - hitPixel.y;

		hitPixel = hitPixel / renderSize;
		float currentZ = Linear01Depth(texture(depthBuffer, hitPixel).x) * -farZ;
		intersect = zA >= currentZ - zThickness && zB <= currentZ;
	}

	// Advance Q based on the number of steps
	Q0.xy += dQ.xy * i;
	Q0.z = pqk.z;
	hitPoint = Q0 / pqk.w;
	iterations = i;
	return intersect;
}

float ComputeBlendFactorForIntersection(
		float iterationCount, 
		vec2 hitPixel,
		vec3 hitPoint,
		vec3 vsRayOrigin,
		vec3 vsRayDirection)
{
	float alpha = 1.0f;

	// Fade ray hits that approach the maximum iterations
	alpha *= 1.0 - (iterationCount / maxSteps);

	// Fade ray hits that approach the screen edge
	float screenFade = screenEdgeFadeStart;
	vec2 hitPixelNDC = (hitPixel * 2.0 - 1.0);
	float maxDimension = min( 1.0, max( abs( hitPixelNDC.x), abs( hitPixelNDC.y)));
	alpha *= 1.0 - (max( 0.0, maxDimension - screenFade) / (1.0 - screenFade));

	// Fade ray hits base on how much they face the camera
	float eyeDirection = clamp(vsRayDirection.z, eyeFadeStart, eyeFadeEnd);
	alpha *= 1.0 - ((eyeDirection - eyeFadeStart) / (eyeFadeEnd - eyeFadeStart));

	// Fade ray hits based on distance from ray origin
	alpha *= 1.0 - clamp(distance(vsRayOrigin, hitPoint) / maxDistance, 0.0, 1.0);

	return alpha;
}

void main()
{
	// Sample original color
	pixelColor = texture(colorBuffer, vtexcoord.xy);


	// Calculate world pixel pos and normal
	float z = texture(depthBuffer, vtexcoord).x;
	if(z >= 0.9999f)
		return;
	vec4 clipSpacePosition = vec4(vtexcoord.xy * 2.0 - 1.0, z * 2.0 - 1.0, 1);
	vec4 viewSpacePosition = invProj * clipSpacePosition;
	viewSpacePosition /= viewSpacePosition.w;
	vec3 vsPos = viewSpacePosition.xyz;
	vec3 vsNormal = mat3(viewMatrix) * (texture(normalBuffer, vtexcoord).xyz * 2.0f - 1.0f);

	// Screen Space Reflection Test
	vec3 vsRayDir = normalize(vsPos);
	vec3 vsReflect = reflect(vsRayDir, vsNormal);
	//vsReflect = vec3(0,0,1);
	vec2 hitPixel = vec2(0, 0);
	vec3 hitPoint = vec3(0, 0, 0);
	vec2 uv2 = vtexcoord * renderSize;
	float jitter = mod((uv2.x + uv2.y) * 0.25, 1.0);
	float iterations = 0;
	bool hit = FindSSRHit(vsPos, vsReflect, jitter, hitPixel, hitPoint, iterations);

	// Move hit pixel from pixel position to UVs
	if(hit == false || hitPixel.x > 1.0f || hitPixel.x < 0.0f || hitPixel.y > 1.0f || hitPixel.y < 0.0f)
		return;

	// Calculate blend factor
	float blend = ComputeBlendFactorForIntersection(iterations, hitPixel, hitPoint, vsPos, vsReflect) * 0.5f;

	// Combine colors
	vec4 hitColor = texture(colorBuffer, hitPixel.xy);
	pixelColor = mix(pixelColor, hitColor, blend);
}
#endif
