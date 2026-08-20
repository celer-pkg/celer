// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

RWTexture3D<float4> dstMip : register(u0);
Texture3D<float4> srcMip : register(t0);
SamplerState samp : register(s0);

cbuffer cbuf : register(b0)
{
    float3 texelSize;
    uint srcMipLevel;
}

[numthreads( 8, 8, 8 )]
void csMipmap3D( uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID )
{
    float3 uv = texelSize * (DTid.xyz + float3(0.5, 0.5, 0.5));
    float4 result = srcMip.SampleLevel(samp, uv, srcMipLevel);
    dstMip[DTid.xyz] = result;
}
