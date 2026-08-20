// Adapted from https://github.com/Twinklebear/webgl-volume-raycaster
//
// The MIT License (MIT)
// Copyright (c) 2018 Will Usher
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions: The above copyright
// notice and this permission notice shall be included in all copies or
// substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS",
// WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

VARYING vec3 vray_dir;
VARYING vec3 transformed_eye;

vec2 intersect_box(vec3 orig, vec3 dir)
{
    const vec3 box_min = vec3(0);
    const vec3 box_max = vec3(1);
    vec3 inv_dir = 1.0 / dir;
    vec3 tmin_tmp = (box_min - orig) * inv_dir;
    vec3 tmax_tmp = (box_max - orig) * inv_dir;
    vec3 tmin = min(tmin_tmp, tmax_tmp);
    vec3 tmax = max(tmin_tmp, tmax_tmp);
    float t0 = max(tmin.x, max(tmin.y, tmin.z));
    float t1 = min(tmax.x, min(tmax.y, tmax.z));
    return vec2(t0, t1);
}

void MAIN()
{
    const vec3 volume_dims = vec3(256, 256, 256);

    vec3 ray_dir = normalize(vray_dir);
    vec2 t_hit = intersect_box(transformed_eye, ray_dir);

    if (t_hit.x > t_hit.y)
        discard;

    vec3 dt_vec = 1.0 / (vec3(volume_dims) * abs(ray_dir));
    float dt = min(dt_vec.x, min(dt_vec.y, dt_vec.z));

    float offset = 0;

    vec3 p = transformed_eye + (t_hit.x + offset * dt) * ray_dir;

    const float dt_scale = 1.0;

    FRAGCOLOR = vec4(0);
    for (float t = t_hit.x; t < t_hit.y; t += dt) {
        // Use textureLod, to play nice with HLSL
        float val = textureLod(volume, vec3(p.x, p.y, p.z), 0).r;
        vec4 val_color = vec4(textureLod(colormap, vec2(val, 0.5), 0).rgb, val);
        // Opacity correction
        val_color.a = 1.0 - pow(max(0.0, 1.0 - val_color.a), dt_scale);
        FRAGCOLOR.rgb += (1.0 - FRAGCOLOR.a) * val_color.a * val_color.rgb;
        FRAGCOLOR.a += (1.0 - FRAGCOLOR.a) * val_color.a;
        if (FRAGCOLOR.a >= 0.95)
            break;
        p += ray_dir * dt;
    }
}
