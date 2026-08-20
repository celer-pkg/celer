#version 440

layout(location = 0) out vec4 fragOutput;

#if QSHADER_VIEW_COUNT >= 2
layout(location = 0) flat in uint v_viewIndex;
#endif

layout(std140, binding = 0) uniform buf {
    vec4 aoProperties;
    vec4 aoProperties2;
    vec4 aoScreenConst;
    vec4 uvToEyeConst;
    vec2 cameraProperties;
} ubuf;

#if QSHADER_VIEW_COUNT >= 2
layout(binding = 1) uniform sampler2DArray depthTextureArray;
#else
layout(binding = 1) uniform sampler2D depthTexture;
#endif

float calculateVertexDepth( vec2 cameraProperties, vec4 position )
{
    float camera_range = cameraProperties.y - cameraProperties.x;
    return 1.0 - ((position.w - cameraProperties.x) / (camera_range));
}

vec4 outputDepth( float vert_depth )
{
    float integer_portion = 0.0;
    float fraction = modf((vert_depth * 255.0), integer_portion);
    return vec4( integer_portion / 255.0, fraction, 0, 1.0 );
}

float getDepthValue( vec4 depth_texture_sample, vec2 cameraProperties )
{
    float zNear = cameraProperties.x;
    float zFar = cameraProperties.y;
    float zRange = zFar - zNear;
    float z_b = depth_texture_sample.x;
    float z_n = 2.0 * z_b - 1.0;
    float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zRange));
    return 1.0 - ((z_e - cameraProperties.x) / (zRange));
}

float depthValueToLinearDistance( float depth_value, vec2 cameraProperties )
{
    float FarClipDistance = cameraProperties.y;
    float NearClipDistance = cameraProperties.x;
    float DepthRange = FarClipDistance - NearClipDistance;
    float linearDepth = NearClipDistance + (DepthRange * (1.0 - depth_value));
    return linearDepth;
}

float hashRot(vec2 pos)
{
    // Basically an odd-even hash.
    float px = 2.0 * fract(floor(pos.x) * 0.5);
    float py = fract(floor(pos.y) * 0.5);

    return px + py;
}

vec3 quatRotate( vec4 q, vec3 v )
{
    return v + 2.0 * cross( cross( v, q.xyz ) + q.w * v, q.xyz );
}

#if QSHADER_VIEW_COUNT >= 2
vec3 getViewSpacePos( sampler2DArray depthSampler, vec2 camProps, vec2 UV, vec4 UvToEye )
{
    float sampleDepth = getDepthValue( texture(depthSampler, vec3(UV, v_viewIndex)), camProps );
#else
vec3 getViewSpacePos( sampler2D depthSampler, vec2 camProps, vec2 UV, vec4 UvToEye )
{
    float sampleDepth = getDepthValue( texture(depthSampler, UV), camProps );
#endif
    sampleDepth = depthValueToLinearDistance( sampleDepth, camProps );

    vec2 scaledUV = (UV * UvToEye.xy) + UvToEye.zw;
    return vec3(scaledUV * sampleDepth, sampleDepth);
}

vec2 computeDir( vec2 baseDir, int v )
{
    float ang = 3.1415926535 * hashRot( gl_FragCoord.xy ) + float(v - 1);
    vec2 vX = vec2(cos(ang), sin(ang));
    vec2 vY = vec2(-sin(ang), cos(ang));

    return vec2( dot(baseDir, vX), dot(baseDir, vY) );
}

vec2 offsetDir( vec2 baseDir, int v )
{
    float ang = float(v - 1);
    vec2 vX = vec2(cos(ang), sin(ang));
    vec2 vY = vec2(-sin(ang), cos(ang));

    return vec2( dot(baseDir, vX), dot(baseDir, vY) );
}

#if QSHADER_VIEW_COUNT >= 2
float calculateAo(int j, vec3 kernel[9], vec4 aoParams, vec4 aoParams2, vec2 camProps, vec2 centerUV, vec4 aoScreen, vec4 UvToEye,
                  vec3 viewPos, vec3 viewNorm, float radStep, sampler2DArray depthSampler)
#else
float calculateAo(int j, vec3 kernel[9], vec4 aoParams, vec4 aoParams2, vec2 camProps, vec2 centerUV, vec4 aoScreen, vec4 UvToEye,
                  vec3 viewPos, vec3 viewNorm, float radStep, sampler2D depthSampler)
#endif
{
    float ret = 0.0;
    // manually unroll the loop 0..8
    {
        int i = 0;
        float curRange = aoParams.y * radStep * float(j);
        float curRadius = curRange * kernel[i].z;

        vec3 smpDir;
        smpDir.xy = computeDir(kernel[i].xy, j) * aoParams2.y + (1.0 - aoParams2.y) * offsetDir(kernel[i].xy, j);
        smpDir.z = kernel[i].z;
        smpDir *= curRange;

        vec2 smpUV = centerUV.xy + smpDir.xy * aoScreen.zw;

        // First method is based on Horizon-Based AO
        vec3 samplePos = getViewSpacePos( depthSampler, camProps, smpUV, UvToEye );
        vec3 smpVec = samplePos - viewPos;

        float lenRad = dot(smpVec, smpVec);
        smpVec = normalize(smpVec);
        float lenDot = dot(smpVec, viewNorm);

        lenRad /= aoParams.y*aoParams.y;
        float falloff = smoothstep(8.0, 0.0, (lenRad - 1.0) * 0.125);
        float occl = 1.0 - clamp(lenDot * falloff, 0.0, 1.0);

        ret += occl * occl;
    }

    {
        int i = 1;
        float curRange = aoParams.y * radStep * float(j);
        float curRadius = curRange * kernel[i].z;

        vec3 smpDir;
        smpDir.xy = computeDir(kernel[i].xy, j) * aoParams2.y + (1.0 - aoParams2.y) * offsetDir(kernel[i].xy, j);
        smpDir.z = kernel[i].z;
        smpDir *= curRange;

        vec2 smpUV = centerUV.xy + smpDir.xy * aoScreen.zw;

        // First method is based on Horizon-Based AO
        vec3 samplePos = getViewSpacePos( depthSampler, camProps, smpUV, UvToEye );
        vec3 smpVec = samplePos - viewPos;

        float lenRad = dot(smpVec, smpVec);
        smpVec = normalize(smpVec);
        float lenDot = dot(smpVec, viewNorm);

        lenRad /= aoParams.y*aoParams.y;
        float falloff = smoothstep(8.0, 0.0, (lenRad - 1.0) * 0.125);
        float occl = 1.0 - clamp(lenDot * falloff, 0.0, 1.0);

        ret += occl * occl;
    }


    {
        int i = 2;
        float curRange = aoParams.y * radStep * float(j);
        float curRadius = curRange * kernel[i].z;

        vec3 smpDir;
        smpDir.xy = computeDir(kernel[i].xy, j) * aoParams2.y + (1.0 - aoParams2.y) * offsetDir(kernel[i].xy, j);
        smpDir.z = kernel[i].z;
        smpDir *= curRange;

        vec2 smpUV = centerUV.xy + smpDir.xy * aoScreen.zw;

        // First method is based on Horizon-Based AO
        vec3 samplePos = getViewSpacePos( depthSampler, camProps, smpUV, UvToEye );
        vec3 smpVec = samplePos - viewPos;

        float lenRad = dot(smpVec, smpVec);
        smpVec = normalize(smpVec);
        float lenDot = dot(smpVec, viewNorm);

        lenRad /= aoParams.y*aoParams.y;
        float falloff = smoothstep(8.0, 0.0, (lenRad - 1.0) * 0.125);
        float occl = 1.0 - clamp(lenDot * falloff, 0.0, 1.0);

        ret += occl * occl;
    }


    {
        int i = 3;
        float curRange = aoParams.y * radStep * float(j);
        float curRadius = curRange * kernel[i].z;

        vec3 smpDir;
        smpDir.xy = computeDir(kernel[i].xy, j) * aoParams2.y + (1.0 - aoParams2.y) * offsetDir(kernel[i].xy, j);
        smpDir.z = kernel[i].z;
        smpDir *= curRange;

        vec2 smpUV = centerUV.xy + smpDir.xy * aoScreen.zw;

        // First method is based on Horizon-Based AO
        vec3 samplePos = getViewSpacePos( depthSampler, camProps, smpUV, UvToEye );
        vec3 smpVec = samplePos - viewPos;

        float lenRad = dot(smpVec, smpVec);
        smpVec = normalize(smpVec);
        float lenDot = dot(smpVec, viewNorm);

        lenRad /= aoParams.y*aoParams.y;
        float falloff = smoothstep(8.0, 0.0, (lenRad - 1.0) * 0.125);
        float occl = 1.0 - clamp(lenDot * falloff, 0.0, 1.0);

        ret += occl * occl;
    }


    {
        int i = 4;
        float curRange = aoParams.y * radStep * float(j);
        float curRadius = curRange * kernel[i].z;

        vec3 smpDir;
        smpDir.xy = computeDir(kernel[i].xy, j) * aoParams2.y + (1.0 - aoParams2.y) * offsetDir(kernel[i].xy, j);
        smpDir.z = kernel[i].z;
        smpDir *= curRange;

        vec2 smpUV = centerUV.xy + smpDir.xy * aoScreen.zw;

        // First method is based on Horizon-Based AO
        vec3 samplePos = getViewSpacePos( depthSampler, camProps, smpUV, UvToEye );
        vec3 smpVec = samplePos - viewPos;

        float lenRad = dot(smpVec, smpVec);
        smpVec = normalize(smpVec);
        float lenDot = dot(smpVec, viewNorm);

        lenRad /= aoParams.y*aoParams.y;
        float falloff = smoothstep(8.0, 0.0, (lenRad - 1.0) * 0.125);
        float occl = 1.0 - clamp(lenDot * falloff, 0.0, 1.0);

        ret += occl * occl;
    }


    {
        int i = 5;
        float curRange = aoParams.y * radStep * float(j);
        float curRadius = curRange * kernel[i].z;

        vec3 smpDir;
        smpDir.xy = computeDir(kernel[i].xy, j) * aoParams2.y + (1.0 - aoParams2.y) * offsetDir(kernel[i].xy, j);
        smpDir.z = kernel[i].z;
        smpDir *= curRange;

        vec2 smpUV = centerUV.xy + smpDir.xy * aoScreen.zw;

        // First method is based on Horizon-Based AO
        vec3 samplePos = getViewSpacePos( depthSampler, camProps, smpUV, UvToEye );
        vec3 smpVec = samplePos - viewPos;

        float lenRad = dot(smpVec, smpVec);
        smpVec = normalize(smpVec);
        float lenDot = dot(smpVec, viewNorm);

        lenRad /= aoParams.y*aoParams.y;
        float falloff = smoothstep(8.0, 0.0, (lenRad - 1.0) * 0.125);
        float occl = 1.0 - clamp(lenDot * falloff, 0.0, 1.0);

        ret += occl * occl;
    }


    {
        int i = 6;
        float curRange = aoParams.y * radStep * float(j);
        float curRadius = curRange * kernel[i].z;

        vec3 smpDir;
        smpDir.xy = computeDir(kernel[i].xy, j) * aoParams2.y + (1.0 - aoParams2.y) * offsetDir(kernel[i].xy, j);
        smpDir.z = kernel[i].z;
        smpDir *= curRange;

        vec2 smpUV = centerUV.xy + smpDir.xy * aoScreen.zw;

        // First method is based on Horizon-Based AO
        vec3 samplePos = getViewSpacePos( depthSampler, camProps, smpUV, UvToEye );
        vec3 smpVec = samplePos - viewPos;

        float lenRad = dot(smpVec, smpVec);
        smpVec = normalize(smpVec);
        float lenDot = dot(smpVec, viewNorm);

        lenRad /= aoParams.y*aoParams.y;
        float falloff = smoothstep(8.0, 0.0, (lenRad - 1.0) * 0.125);
        float occl = 1.0 - clamp(lenDot * falloff, 0.0, 1.0);

        ret += occl * occl;
    }


    {
        int i = 7;
        float curRange = aoParams.y * radStep * float(j);
        float curRadius = curRange * kernel[i].z;

        vec3 smpDir;
        smpDir.xy = computeDir(kernel[i].xy, j) * aoParams2.y + (1.0 - aoParams2.y) * offsetDir(kernel[i].xy, j);
        smpDir.z = kernel[i].z;
        smpDir *= curRange;

        vec2 smpUV = centerUV.xy + smpDir.xy * aoScreen.zw;

        // First method is based on Horizon-Based AO
        vec3 samplePos = getViewSpacePos( depthSampler, camProps, smpUV, UvToEye );
        vec3 smpVec = samplePos - viewPos;

        float lenRad = dot(smpVec, smpVec);
        smpVec = normalize(smpVec);
        float lenDot = dot(smpVec, viewNorm);

        lenRad /= aoParams.y*aoParams.y;
        float falloff = smoothstep(8.0, 0.0, (lenRad - 1.0) * 0.125);
        float occl = 1.0 - clamp(lenDot * falloff, 0.0, 1.0);

        ret += occl * occl;
    }


    {
        int i = 8;
        float curRange = aoParams.y * radStep * float(j);
        float curRadius = curRange * kernel[i].z;

        vec3 smpDir;
        smpDir.xy = computeDir(kernel[i].xy, j) * aoParams2.y + (1.0 - aoParams2.y) * offsetDir(kernel[i].xy, j);
        smpDir.z = kernel[i].z;
        smpDir *= curRange;

        vec2 smpUV = centerUV.xy + smpDir.xy * aoScreen.zw;

        // First method is based on Horizon-Based AO
        vec3 samplePos = getViewSpacePos( depthSampler, camProps, smpUV, UvToEye );
        vec3 smpVec = samplePos - viewPos;

        float lenRad = dot(smpVec, smpVec);
        smpVec = normalize(smpVec);
        float lenDot = dot(smpVec, viewNorm);

        lenRad /= aoParams.y*aoParams.y;
        float falloff = smoothstep(8.0, 0.0, (lenRad - 1.0) * 0.125);
        float occl = 1.0 - clamp(lenDot * falloff, 0.0, 1.0);

        ret += occl * occl;
    }

    return ret;
}

#if QSHADER_VIEW_COUNT >= 2
float SSambientOcclusion(sampler2DArray depthSampler, vec3 viewNorm, vec4 aoParams, vec4 aoParams2, vec2 camProps, vec4 aoScreen, vec4 UvToEye)
#else
float SSambientOcclusion(sampler2D depthSampler, vec3 viewNorm, vec4 aoParams, vec4 aoParams2, vec2 camProps, vec4 aoScreen, vec4 UvToEye)
#endif
{
    vec2 centerUV = gl_FragCoord.xy * aoScreen.zw;
    vec3 viewPos = getViewSpacePos( depthSampler, camProps, centerUV, UvToEye );
    viewPos += viewNorm * aoParams.w;

    float screenRadius = aoParams.y * aoScreen.y / viewPos.z;
    if (screenRadius < 1.0) { return 1.0; }

    vec3 kernel[9];

    // The X and Y are the 2d direction, while the Z is the height of the sphere at that point.
    // In essence, it normalizes the 3d vector, but we're really interested in the 2D offset.
    kernel[0] = vec3(-0.1376476, 0.2842022, 0.948832);
    kernel[1] = vec3(-0.626618, 0.4594115, 0.629516);
    kernel[2] = vec3(-0.8903138, -0.05865424, 0.451554);
    kernel[3] = vec3(0.2871419, 0.8511679, 0.439389);
    kernel[4] = vec3(-0.1525251, -0.3870117, 0.909372);
    kernel[5] = vec3(0.6978705, -0.2176773, 0.682344);
    kernel[6] = vec3(0.7343006, 0.3774331, 0.5642);
    kernel[7] = vec3(0.1408805, -0.88915, 0.4353);
    kernel[8] = vec3(-0.6642616, -0.543601, 0.5130);

    float sampleRate = clamp(aoParams2.x, 2.0, 4.0);
    int radLevels = int(floor(sampleRate));
    float radStep = 1.0 / sampleRate;

    float ret = 0.0;
    // Manually unroll the loop 1..radLevels, and so the same inside
    // calculateAo. This is because HLSL cannot deal with automatic unrolling
    // without the [unroll] annotation which we have no means to add.
    ret += calculateAo(1, kernel, aoParams, aoParams2, camProps, centerUV, aoScreen, UvToEye, viewPos, viewNorm, radStep, depthSampler);
    ret += calculateAo(2, kernel, aoParams, aoParams2, camProps, centerUV, aoScreen, UvToEye, viewPos, viewNorm, radStep, depthSampler);
    if (radLevels >= 3)
        ret += calculateAo(3, kernel, aoParams, aoParams2, camProps, centerUV, aoScreen, UvToEye, viewPos, viewNorm, radStep, depthSampler);
    if (radLevels >= 4)
        ret += calculateAo(4, kernel, aoParams, aoParams2, camProps, centerUV, aoScreen, UvToEye, viewPos, viewNorm, radStep, depthSampler);

    ret = (ret) / (9.0 * float(radLevels));

    // Blend between soft and hard based on softness param
    // NOTE : the 0.72974 is actually an gamma-inverted 0.5 (assuming gamma 2.2)
    // Would not need this if we linearized color instead.
    float hardCut = (1.0 - aoParams.z) * 0.72974;
    ret = smoothstep(0.0, 1.0, (ret - hardCut) / (1.0 - hardCut));

    // Blend between full and no occlusion based on strength param
    ret = aoParams.x * ret + (1.0 - aoParams.x);

    return ret;
}

void main()
{
    ivec2 iCoords = ivec2(gl_FragCoord.xy);
#if QSHADER_VIEW_COUNT >= 2
    float depth = getDepthValue(texelFetch(depthTextureArray, ivec3(iCoords, v_viewIndex), 0), ubuf.cameraProperties);
#else
    float depth = getDepthValue(texelFetch(depthTexture, iCoords, 0), ubuf.cameraProperties);
#endif
    depth = depthValueToLinearDistance( depth, ubuf.cameraProperties );
    depth = (depth - ubuf.cameraProperties.x) / (ubuf.cameraProperties.y - ubuf.cameraProperties.x);
#if QSHADER_VIEW_COUNT >= 2
    float depth2 = getDepthValue(texelFetch(depthTextureArray, ivec3(iCoords+ivec2(1), v_viewIndex), 0), ubuf.cameraProperties);
#else
    float depth2 = getDepthValue(texelFetch(depthTexture, iCoords+ivec2(1), 0), ubuf.cameraProperties);
#endif
    depth2 = depthValueToLinearDistance( depth, ubuf.cameraProperties );
#if QSHADER_VIEW_COUNT >= 2
    float depth3 = getDepthValue(texelFetch(depthTextureArray, ivec3(iCoords-ivec2(1), v_viewIndex), 0), ubuf.cameraProperties);
#else
    float depth3 = getDepthValue(texelFetch(depthTexture, iCoords-ivec2(1), 0), ubuf.cameraProperties);
#endif

    depth3 = depthValueToLinearDistance( depth, ubuf.cameraProperties );

    vec3 tanU = vec3(10, 0, dFdx(depth));
    vec3 tanV = vec3(0, 10, dFdy(depth));
    vec3 screenNorm = normalize(cross(tanU, tanV));
    tanU = vec3(10, 0, dFdx(depth2));
    tanV = vec3(0, 10, dFdy(depth2));
    screenNorm += normalize(cross(tanU, tanV));
    tanU = vec3(10, 0, dFdx(depth3));
    tanV = vec3(0, 10, dFdy(depth3));
    screenNorm += normalize(cross(tanU, tanV));
    screenNorm = -normalize(screenNorm);

#if QSHADER_VIEW_COUNT >= 2
    float aoFactor = SSambientOcclusion( depthTextureArray,
#else
    float aoFactor = SSambientOcclusion( depthTexture,
#endif
                                         screenNorm, ubuf.aoProperties, ubuf.aoProperties2,
                                         ubuf.cameraProperties, ubuf.aoScreenConst, ubuf.uvToEyeConst );

    fragOutput = vec4(aoFactor, aoFactor, aoFactor, 1.0);
}
