#define AI_BASECOLOR_FACTOR vec4(0.2549, 0.80392, 0.32157, 1.0)
#define AI_METALNESS 0.5
#define AI_ROUGHNESS 0.1

//
// References:
// https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
// https://google.github.io/filament/Filament.md.html
// https://github.com/KhronosGroup/glTF-Sample-Viewer#physically-based-materials-in-gltf-20
// https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile
//

SHARED_VARS {
    vec3 f0;
    vec3 albedo;
};
vec3 sRGBToLinear(vec3 rgbIn)
{
    return rgbIn * (rgbIn * (rgbIn * 0.305306011 + 0.682171111) + 0.012522878);
}
vec4 sRGBToLinear(vec4 rgbaIn)
{
    return vec4(sRGBToLinear(rgbaIn.rgb), rgbaIn.a);
}
void MAIN()
{
    BASE_COLOR = AI_BASECOLOR_FACTOR;
    METALNESS = AI_METALNESS;
    ROUGHNESS = AI_ROUGHNESS;

    SHARED.f0 = mix(vec3(0.04), BASE_COLOR.rgb, METALNESS);
    SHARED.albedo = BASE_COLOR.rgb * 0.96 * (1 - METALNESS);
}

void IBL_PROBE()
{
    if (IBL_EXPOSE < 0.005)
        return;
    vec3 smpDirDiffuse = NORMAL;
    smpDirDiffuse = IBL_ORIENTATION * smpDirDiffuse;

    vec3 diffuseSample = IBL_EXPOSE * AO_FACTOR * SHARED.albedo * sRGBToLinear(textureLod(IBL_TEXTURE, smpDirDiffuse, IBL_MAXMIPMAP).rgb);
    float NdotV = clamp(dot(NORMAL, VIEW_VECTOR), 0.0, 1.0);
    float lod = clamp(ROUGHNESS * IBL_MAXMIPMAP, 0.0, IBL_MAXMIPMAP);
    vec3 smpDirSpecular = normalize(reflect(-VIEW_VECTOR, NORMAL));
    smpDirSpecular = IBL_ORIENTATION * smpDirSpecular;
    vec4 specSample = textureLod(IBL_TEXTURE, smpDirSpecular, lod);
    const vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
    const vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);
    vec4 r = ROUGHNESS * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * NdotV)) * r.x + r.y;
    vec2 brdf = vec2(-1.04, 1.04) * a004 + r.zw;
    specSample.rgb = sRGBToLinear(specSample.rgb);

    specSample.rgb *= IBL_EXPOSE * AO_FACTOR * (SHARED.f0 * brdf.x + brdf.y);
    if (IBL_HORIZON > -1.0) {
        float ctr = 0.5 + 0.5 * IBL_HORIZON;
        vec2 vertWt;
        vertWt.x = smoothstep(ctr * 0.25, ctr + 0.25, smpDirDiffuse.y);
        vertWt.y = smoothstep(ctr * 0.25, ctr + 0.25, smpDirSpecular.y);
        vec2 wtScaled = mix(vec2(1.0), vertWt, IBL_HORIZON + 1.0);
        diffuseSample *= wtScaled.x;
        specSample.rgb *= wtScaled.y;
    }

    DIFFUSE += diffuseSample;
    SPECULAR += specSample.rgb;
}
