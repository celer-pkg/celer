// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "texturesample.glsllib"

vec3 tonemap_filmic(vec3 color, float white) {
    // exposure bias: input scale (color *= bias, white *= bias) to make the brightness consistent with other tonemappers
    // also useful to scale the input to the range that the tonemapper is designed for (some require very high input values)
    // has no effect on the curve's general shape or visual properties
    const float exposure_bias = 2.0;
    const float A = 0.22 * exposure_bias * exposure_bias; // bias baked into constants for performance
    const float B = 0.30 * exposure_bias;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.01;
    const float F = 0.30;

    vec3 color_tonemapped = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    float white_tonemapped = ((white * (A * white + C * B) + D * E) / (white * (A * white + B) + D * F)) - E / F;

    return clamp(color_tonemapped / white_tonemapped, vec3(0.0), vec3(1.0));
}

vec3 tonemap_aces(vec3 color, float white) {
    const float exposure_bias = 0.85;
    const float A = 2.51 * exposure_bias * exposure_bias;
    const float B = 0.03 * exposure_bias;
    const float C = 2.43 * exposure_bias * exposure_bias;
    const float D = 0.59 * exposure_bias;
    const float E = 0.14;

    vec3 color_tonemapped = (color * (A * color + B)) / (color * (C * color + D) + E);
    float white_tonemapped = (white * (A * white + B)) / (white * (C * white + D) + E);

    return clamp(color_tonemapped / white_tonemapped, vec3(0.0), vec3(1.0));
}

vec3 tonemap_reinhard(vec3 color, float white) {
    return clamp((white * color + color) / (color * white + white), vec3(0.0), vec3(1.0));
}

vec3 linear_to_srgb(vec3 color) { // convert linear rgb to srgb, assumes clamped input in range [0;1]
    const vec3 a = vec3(0.055);
    return mix((vec3(1.0) + a) * pow(color.rgb, vec3(1.0 / 2.4)) - a, 12.92 * color.rgb, lessThan(color.rgb, vec3(0.0031308)));
}

vec3 apply_fxaa(vec3 color, float exposure, vec2 pixelSize) {
    const float FXAA_REDUCE_MIN = (1.0 / 128.0);
    const float FXAA_REDUCE_MUL = (1.0 / 8.0);
    const float FXAA_SPAN_MAX = 8.0;

    vec3 rgbNW = SAMPLE_LOD(INPUT, INPUT_UV + vec2(-1.0, -1.0) * pixelSize, 0.0).xyz * exposure;
    vec3 rgbNE = SAMPLE_LOD(INPUT, INPUT_UV + vec2(1.0, -1.0) * pixelSize, 0.0).xyz * exposure;
    vec3 rgbSW = SAMPLE_LOD(INPUT, INPUT_UV + vec2(-1.0, 1.0) * pixelSize, 0.0).xyz * exposure;
    vec3 rgbSE = SAMPLE_LOD(INPUT, INPUT_UV + vec2(1.0, 1.0) * pixelSize, 0.0).xyz * exposure;
    vec3 rgbM = color;
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM = dot(rgbM, luma);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
                          (0.25 * FXAA_REDUCE_MUL),
                          FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
              max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
                  dir * rcpDirMin)) *
            pixelSize;

    vec3 rgbA = 0.5 * exposure * (SAMPLE_LOD(INPUT, INPUT_UV + dir * (1.0 / 3.0 - 0.5), 0.0).xyz + SAMPLE_LOD(INPUT, INPUT_UV + dir * (2.0 / 3.0 - 0.5), 0.0).xyz);
    vec3 rgbB = rgbA * 0.5 + 0.25 * exposure * (SAMPLE_LOD(INPUT, INPUT_UV + dir * -0.5, 0.0).xyz + SAMPLE_LOD(INPUT, INPUT_UV + dir * 0.5, 0.0).xyz);

    float lumaB = dot(rgbB, luma);
    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        return rgbA;
    } else {
        return rgbB;
    }
}

// From http://alex.vlachos.com/graphics/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
// and https://www.shadertoy.com/view/MslGR8 (5th one starting from the bottom)
// NOTE: `frag_coord` is in pixels (i.e. not normalized UV).
vec3 screen_space_dither(vec2 frag_coord) {
    // Iestyn's RGB dither (7 asm instructions) from Portal 2 X360, slightly modified for VR.
    vec3 dither = vec3(dot(vec2(171.0, 231.0), frag_coord));
    dither.rgb = fract(dither.rgb / vec3(103.0, 71.0, 97.0));

    // Subtract 0.5 to avoid slightly brightening the whole viewport.
    return (dither.rgb - 0.5) / 255.0;
}

// Adapted from https://github.com/DadSchoorse/vkBasalt/blob/b929505ba71dea21d6c32a5a59f2d241592b30c4/src/shader/cas.frag.glsl
// (MIT license).
vec3 apply_cas(vec3 color, float exposure, float sharpen_intensity) {
    // Fetch a 3x3 neighborhood around the pixel 'e',
    //  a b c
    //  d(e)f
    //  g h i
    vec3 a = SAMPLE_LOD_OFFSET(INPUT, INPUT_UV, 0.0, ivec2(-1, -1)).rgb * exposure;
    vec3 b = SAMPLE_LOD_OFFSET(INPUT, INPUT_UV, 0.0, ivec2(0, -1)).rgb * exposure;
    vec3 c = SAMPLE_LOD_OFFSET(INPUT, INPUT_UV, 0.0, ivec2(1, -1)).rgb * exposure;
    vec3 d = SAMPLE_LOD_OFFSET(INPUT, INPUT_UV, 0.0, ivec2(-1, 0)).rgb * exposure;
    vec3 e = color.rgb;
    vec3 f = SAMPLE_LOD_OFFSET(INPUT, INPUT_UV, 0.0, ivec2(1, 0)).rgb * exposure;
    vec3 g = SAMPLE_LOD_OFFSET(INPUT, INPUT_UV, 0.0, ivec2(-1, 1)).rgb * exposure;
    vec3 h = SAMPLE_LOD_OFFSET(INPUT, INPUT_UV, 0.0, ivec2(0, 1)).rgb * exposure;
    vec3 i = SAMPLE_LOD_OFFSET(INPUT, INPUT_UV, 0.0, ivec2(1, 1)).rgb * exposure;

    // Soft min and max.
    //  a b c             b
    //  d e f * 0.5  +  d e f * 0.5
    //  g h i             h
    // These are 2.0x bigger (factored out the extra multiply).
    vec3 min_rgb = min(min(min(d, e), min(f, b)), h);
    vec3 min_rgb2 = min(min(min(min_rgb, a), min(g, c)), i);
    min_rgb += min_rgb2;

    vec3 max_rgb = max(max(max(d, e), max(f, b)), h);
    vec3 max_rgb2 = max(max(max(max_rgb, a), max(g, c)), i);
    max_rgb += max_rgb2;

    // Smooth minimum distance to signal limit divided by smooth max.
    vec3 rcp_max_rgb = vec3(1.0) / max_rgb;
    vec3 amp_rgb = clamp((min(min_rgb, 2.0 - max_rgb) * rcp_max_rgb), 0.0, 1.0);

    // Shaping amount of sharpening.
    amp_rgb = inversesqrt(amp_rgb);
    float peak = 8.0 - 3.0 * sharpen_intensity;
    vec3 w_rgb = -vec3(1) / (amp_rgb * peak);
    vec3 rcp_weight_rgb = vec3(1.0) / (1.0 + 4.0 * w_rgb);

    //                          0 w 0
    //  Filter shape:           w 1 w
    //                          0 w 0
    vec3 window = b + d + f + h;

    return max(vec3(0.0), (window * w_rgb + e) * rcp_weight_rgb);
}

vec3 apply_bcs(vec3 color) {
    color = mix(vec3(0.0), color, adjustmentBrightness);
    color = mix(vec3(0.5), color, adjustmentContrast);
    color = mix(vec3(dot(vec3(1.0), color) * 0.33333), color, adjustmentSaturation);

    return color;
}

vec3 apply_tonemapping(vec3 color, float white) {
    if (tonemapMode == 0) {
        // None
    } else if (tonemapMode == 1) {
        // Linear
        color = clamp(color, vec3(0.0f), vec3(1.0f));
        color = linear_to_srgb(color);
    } else if (tonemapMode == 2) {
        // ACES
        color = max(vec3(0.0), color);
        color = tonemap_aces(color, white);
        color = linear_to_srgb(color);
    } else if (tonemapMode == 3) {
        // HejlDawson/Reinhard
        color = max(vec3(0.0), color);
        color = tonemap_reinhard(color, white);
        color = linear_to_srgb(color);
    } else {
        // Filmic
        color = max(vec3(0.0), color);
        color = tonemap_filmic(color, white);
        color = linear_to_srgb(color);
    }
    return color;
}

// w0, w1, w2, and w3 are the four cubic B-spline basis functions
float w0(float a) {
    return (1.0f / 6.0f) * (a * (a * (-a + 3.0f) - 3.0f) + 1.0f);
}

float w1(float a) {
    return (1.0f / 6.0f) * (a * a * (3.0f * a - 6.0f) + 4.0f);
}

float w2(float a) {
    return (1.0f / 6.0f) * (a * (a * (-3.0f * a + 3.0f) + 3.0f) + 1.0f);
}

float w3(float a) {
    return (1.0f / 6.0f) * (a * a * a);
}

// g0 and g1 are the two amplitude functions
float g0(float a) {
    return w0(a) + w1(a);
}

float g1(float a) {
    return w2(a) + w3(a);
}

// h0 and h1 are the two offset functions
float h0(float a) {
    return -1.0f + w1(a) / (w0(a) + w1(a));
}

float h1(float a) {
    return 1.0f + w3(a) / (w2(a) + w3(a));
}

#if QSHADER_VIEW_COUNT >= 2
vec3 sampleGlowBuffer(sampler2DArray glowBuffer, int level)
#else
vec3 sampleGlowBuffer(sampler2D glowBuffer, int level)
#endif
{
    if (glowUseBicubicUpscale) {
        vec2 tex_size = vec2(ivec2(INPUT_SIZE) >> level);
        vec2 texel_size = vec2(1.0f) / tex_size;

        vec2 uv = INPUT_UV * tex_size + vec2(0.5f);

        vec2 iuv = floor(uv);
        vec2 fuv = fract(uv);

        float g0x = g0(fuv.x);
        float g1x = g1(fuv.x);
        float h0x = h0(fuv.x);
        float h1x = h1(fuv.x);
        float h0y = h0(fuv.y);
        float h1y = h1(fuv.y);

        vec2 p0 = (vec2(iuv.x + h0x, iuv.y + h0y) - vec2(0.5f)) * texel_size;
        vec2 p1 = (vec2(iuv.x + h1x, iuv.y + h0y) - vec2(0.5f)) * texel_size;
        vec2 p2 = (vec2(iuv.x + h0x, iuv.y + h1y) - vec2(0.5f)) * texel_size;
        vec2 p3 = (vec2(iuv.x + h1x, iuv.y + h1y) - vec2(0.5f)) * texel_size;

        return ((g0(fuv.y) * (g0x * SAMPLE_LOD(glowBuffer, p0, 0) + g1x * SAMPLE_LOD(glowBuffer, p1, 0))) +
                (g1(fuv.y) * (g0x * SAMPLE_LOD(glowBuffer, p2, 0) + g1x * SAMPLE_LOD(glowBuffer, p3, 0)))).rgb;
    } else {
        return SAMPLE_LOD(glowBuffer, INPUT_UV, 0.0).rgb;
    }
}

float fixY(float y) {
    if (qt_normalAdjustViewportFactor > 0.0)
        return y;
    const float lut_div = lutSize - 1.0;
    return lut_div - y;
}

// Gets interpolation percentage for color  channel using floor and diff values.
float get_interp_percent_channel(float channel_value, float floor_value, float diff_value){
    // Workaround to avoid division by zero and return zero
    float div_sign = abs(sign(diff_value));
    return (channel_value-floor_value)*div_sign/(diff_value + (div_sign-1.0));
}

// Gets interpolation percentage for color using floor and diff values.
vec3 get_interp_percent_color(vec3 color, vec3 floorc, vec3 diff){
    vec3 res = vec3(0.0);
    res.r = get_interp_percent_channel(color.r, floorc.r, diff.r);
    res.g = get_interp_percent_channel(color.g, floorc.g, diff.g);
    res.b = get_interp_percent_channel(color.b, floorc.b, diff.b);
    return res;
}

// Get interpolated color using color floor, diff and channel percentage.
vec3 get_interpolated_color(vec3 floorc, vec3 diff, float perc){
    return floorc.rgb + diff.rgb * perc;
}

// Gets LUT mapped color using trilinear interpolation.
vec3 get_lut_mapping_trilinear(vec3 old_color){
    float lut_div = lutSize - 1.0;
    // Get floor and ceil colors and diff from identity lut
    vec3 old_color_lut_base = lut_div * old_color;
    vec3 old_color_floor_vec = floor(old_color_lut_base);
    vec3 old_color_ceil_vec = ceil(old_color_lut_base);
    vec3 old_color_diff = (old_color_floor_vec - old_color_ceil_vec)/lut_div;
    vec3 old_color_percentages = get_interp_percent_color(old_color, old_color_floor_vec/lut_div, old_color_diff);
    // Get the surrounding 8 samples positions
    vec3 lut_color_fff_vec = vec3(old_color_floor_vec.r, old_color_floor_vec.g, old_color_floor_vec.b);
    vec3 lut_color_ffc_vec = vec3(old_color_floor_vec.r, old_color_floor_vec.g, old_color_ceil_vec.b);
    vec3 lut_color_fcf_vec = vec3(old_color_floor_vec.r, old_color_ceil_vec.g, old_color_floor_vec.b);
    vec3 lut_color_fcc_vec = vec3(old_color_floor_vec.r, old_color_ceil_vec.g, old_color_ceil_vec.b);
    vec3 lut_color_cff_vec = vec3(old_color_ceil_vec.r, old_color_floor_vec.g, old_color_floor_vec.b);
    vec3 lut_color_cfc_vec = vec3(old_color_ceil_vec.r, old_color_floor_vec.g, old_color_ceil_vec.b);
    vec3 lut_color_ccf_vec = vec3(old_color_ceil_vec.r, old_color_ceil_vec.g, old_color_floor_vec.b);
    vec3 lut_color_ccc_vec = vec3(old_color_ceil_vec.r, old_color_ceil_vec.g, old_color_ceil_vec.b);
    ivec2 lut_color_fff_pos = ivec2(int(lutSize*lut_color_fff_vec.b + lut_color_fff_vec.r), int(fixY(lut_color_fff_vec.g)));
    ivec2 lut_color_ffc_pos = ivec2(int(lutSize*lut_color_ffc_vec.b + lut_color_ffc_vec.r), int(fixY(lut_color_ffc_vec.g)));
    ivec2 lut_color_fcf_pos = ivec2(int(lutSize*lut_color_fcf_vec.b + lut_color_fcf_vec.r), int(fixY(lut_color_fcf_vec.g)));
    ivec2 lut_color_fcc_pos = ivec2(int(lutSize*lut_color_fcc_vec.b + lut_color_fcc_vec.r), int(fixY(lut_color_fcc_vec.g)));
    ivec2 lut_color_cff_pos = ivec2(int(lutSize*lut_color_cff_vec.b + lut_color_cff_vec.r), int(fixY(lut_color_cff_vec.g)));
    ivec2 lut_color_cfc_pos = ivec2(int(lutSize*lut_color_cfc_vec.b + lut_color_cfc_vec.r), int(fixY(lut_color_cfc_vec.g)));
    ivec2 lut_color_ccf_pos = ivec2(int(lutSize*lut_color_ccf_vec.b + lut_color_ccf_vec.r), int(fixY(lut_color_ccf_vec.g)));
    ivec2 lut_color_ccc_pos = ivec2(int(lutSize*lut_color_ccc_vec.b + lut_color_ccc_vec.r), int(fixY(lut_color_ccc_vec.g)));
    // Get gamma corrected color from LUT.
    vec3 lut_color_fff = texelFetch(lut, lut_color_fff_pos, 0).rgb;
    vec3 lut_color_ffc = texelFetch(lut, lut_color_ffc_pos, 0).rgb;
    vec3 lut_color_fcf = texelFetch(lut, lut_color_fcf_pos, 0).rgb;
    vec3 lut_color_fcc = texelFetch(lut, lut_color_fcc_pos, 0).rgb;
    vec3 lut_color_cff = texelFetch(lut, lut_color_cff_pos, 0).rgb;
    vec3 lut_color_cfc = texelFetch(lut, lut_color_cfc_pos, 0).rgb;
    vec3 lut_color_ccf = texelFetch(lut, lut_color_ccf_pos, 0).rgb;
    vec3 lut_color_ccc = texelFetch(lut, lut_color_ccc_pos, 0).rgb;
    // Calculate first level interpolations.
    vec3 lut_color_iff = get_interpolated_color(lut_color_fff, lut_color_fff - lut_color_cff , old_color_percentages.r);
    vec3 lut_color_ifc = get_interpolated_color(lut_color_ffc, lut_color_ffc - lut_color_cfc, old_color_percentages.r);
    vec3 lut_color_icf = get_interpolated_color(lut_color_fcf, lut_color_fcf - lut_color_ccf, old_color_percentages.r);
    vec3 lut_color_icc = get_interpolated_color(lut_color_fcc, lut_color_fcc - lut_color_ccc, old_color_percentages.r);
    // Calculate second level interpolations.
    vec3 lut_color_iif = get_interpolated_color(lut_color_iff, lut_color_iff - lut_color_icf, old_color_percentages.g);
    vec3 lut_color_iic = get_interpolated_color(lut_color_ifc, lut_color_ifc - lut_color_icc, old_color_percentages.g);
    // Calculate third and final interpolation.
    vec3 lut_color_iii = get_interpolated_color(lut_color_iif, lut_color_iif - lut_color_iic, old_color_percentages.b);

    return lut_color_iii;
}

void MAIN()
{
    vec4 sourceColor = SAMPLE(INPUT, INPUT_UV);
    vec3 color = sourceColor.rgb;
    // Exposure
    float fullExposure = exposure;

    color *= fullExposure;

    // FXAA
    if (applyFXAA)
        color = apply_fxaa(color, fullExposure, vec2(1.0 / OUTPUT_SIZE.x, 1.0 / OUTPUT_SIZE.y));

    // Sharpening
    if (sharpnessAmount >= 0.001)
        color = apply_cas(color, fullExposure, sharpnessAmount);

    // Debanding
    if (ditheringEnabled)
        color += screen_space_dither(gl_FragCoord.xy);

    // Lens Flare
    if (lensFlareEnabled) {
        vec3 lensMod = vec3(1.0);

        if (lensFlareApplyDirtTexture) {
            lensMod = texture(lensDirtTexture, INPUT_UV).rgb;
        }

        vec3 lensFlare = SAMPLE(lensFlareTexture, INPUT_UV).rgb * lensMod;

        if (lensFlareApplyStarburstTexture) {
            vec2 centerVec = INPUT_UV - vec2(0.5);
            float d = length(centerVec);
            float radial = acos(centerVec.x / d);

            float starOffset = dot(lensFlareCameraDirection, vec3(1.0)) * 10.0;
            float star = texture(starburstTexture, vec2(radial + starOffset)).r
                      * texture(starburstTexture, vec2(radial + starOffset * 0.5)).r;
            star = clamp(star + (1.0 - smoothstep(0.0, 0.3, d)), 0.0, 1.0);
            lensFlare *= star;
        }

        if (!lensFlareDebug)
            color += lensFlare;
        else {
            FRAGCOLOR = vec4(apply_tonemapping(lensFlare, white), 1.0);
            return;
        }
    }

    // Tonemapping
    color = apply_tonemapping(color, white);

    // Glow
    if (isGlowEnabled) {
        // Read glow from the glowBuffers
        const int GLOW_LEVEL_1 = 0x1;
        const int GLOW_LEVEL_2 = 0x2;
        const int GLOW_LEVEL_3 = 0x4;
        const int GLOW_LEVEL_4 = 0x8;
        const int GLOW_LEVEL_5 = 0x10;
        const int GLOW_LEVEL_6 = 0x20;
        const int GLOW_LEVEL_7 = 0x40;

        vec3 glow = vec3(0.0);
        if ((glowLevel & GLOW_LEVEL_1) != 0)
            glow += sampleGlowBuffer(glowBuffer1, 0);
        if ((glowLevel & GLOW_LEVEL_2) != 0)
            glow += sampleGlowBuffer(glowBuffer2, 1);
        if ((glowLevel & GLOW_LEVEL_3) != 0)
            glow += sampleGlowBuffer(glowBuffer3, 2);
        if ((glowLevel & GLOW_LEVEL_4) != 0)
            glow += sampleGlowBuffer(glowBuffer4, 3);
        if ((glowLevel & GLOW_LEVEL_5) != 0)
            glow += sampleGlowBuffer(glowBuffer5, 4);
        if ((glowLevel & GLOW_LEVEL_6) != 0)
            glow += sampleGlowBuffer(glowBuffer6, 5);
        if ((glowLevel & GLOW_LEVEL_7) != 0)
            glow += sampleGlowBuffer(glowBuffer7, 6);
        glow *= glowIntensity;

        // Tonemap the resulting glow
        glow = apply_tonemapping(glow, white);

        // Blend in the Glow
        if (glowBlendMode == 0) {
            // Additive
            color += glow;
        } else if (glowBlendMode == 1) {
            // Screen
            color = clamp(color, vec3(0.0), vec3(1.0));
            color = max((color + glow) - (color * glow), vec3(0.0));
        } else if (glowBlendMode == 2) {
            // Softlight
            color = clamp(color, vec3(0.0), vec3(1.0));
            glow = glow * vec3(0.5) + vec3(0.5);

            color.r = (glow.r <= 0.5) ? (color.r - (1.0 - 2.0 * glow.r) * color.r * (1.0 - color.r)) : (((glow.r > 0.5) && (color.r <= 0.25)) ? (color.r + (2.0 * glow.r - 1.0) * (4.0 * color.r * (4.0 * color.r + 1.0) * (color.r - 1.0) + 7.0 * color.r)) : (color.r + (2.0 * glow.r - 1.0) * (sqrt(color.r) - color.r)));
            color.g = (glow.g <= 0.5) ? (color.g - (1.0 - 2.0 * glow.g) * color.g * (1.0 - color.g)) : (((glow.g > 0.5) && (color.g <= 0.25)) ? (color.g + (2.0 * glow.g - 1.0) * (4.0 * color.g * (4.0 * color.g + 1.0) * (color.g - 1.0) + 7.0 * color.g)) : (color.g + (2.0 * glow.g - 1.0) * (sqrt(color.g) - color.g)));
            color.b = (glow.b <= 0.5) ? (color.b - (1.0 - 2.0 * glow.b) * color.b * (1.0 - color.b)) : (((glow.b > 0.5) && (color.b <= 0.25)) ? (color.b + (2.0 * glow.b - 1.0) * (4.0 * color.b * (4.0 * color.b + 1.0) * (color.b - 1.0) + 7.0 * color.b)) : (color.b + (2.0 * glow.b - 1.0) * (sqrt(color.b) - color.b)));
        } else {
            // Replace
            color = glow;
        }
    }

    // Brightness, Contrast, Saturation Adjustments
    if (colorAdjustmentsEnabled)
        color = apply_bcs(color);

    // Color Grading (LUT)
    if (enableLut) {
        vec3 filteredColor = get_lut_mapping_trilinear(color);
       // Calculate filter alpha.
        vec3 diffColor = filteredColor - color;
        color = get_interpolated_color(color, diffColor, lutFilterAlpha);
    }

    // Vignette
    if (vignetteEnabled) {
        vec2 uv = INPUT_UV;
        vec3 bg = color;

        uv *= 1.0 - uv.yx;
        float vig = uv.x * uv.y * vignetteStrength;
        vig = pow(vig, vignetteRadius);

        vec4 vigColor = vec4(vignetteColor.rgb, vig) * vig;
        color = mix(vigColor, vec4(color, 1.0), vig).rgb;
    }

    // Final Output
    FRAGCOLOR = vec4(color, sourceColor.a);
}
