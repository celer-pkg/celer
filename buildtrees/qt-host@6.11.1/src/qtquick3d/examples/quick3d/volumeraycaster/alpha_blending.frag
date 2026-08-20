// Copyright (C) 2018 Martino Pilia <martino.pilia@gmail.com>
// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: MIT

VARYING vec3 ray_direction_model; // The direction from camera eye to vertex in model space

// Slab method for ray-box intersection
bool ray_box_intersection(vec3 ray_origin, vec3 ray_direction, vec3 box_bottom, vec3 box_top, out float t_0, out float t_1)
{
    vec3 direction_inv = 1.0 / ray_direction;
    vec3 t_top = direction_inv * (box_top - ray_origin);
    vec3 t_bottom = direction_inv * (box_bottom - ray_origin);
    vec3 t_min = min(t_top, t_bottom);
    vec2 t = max(t_min.xx, t_min.yz);
    t_0 = max(0.0, max(t.x, t.y));
    vec3 t_max = max(t_top, t_bottom);
    t = min(t_max.xx, t_max.yz);
    t_1 = min(t.x, t.y);
    return t_1 >= 0 && t_1 >= t_0;
}

//! [main]
void MAIN()
{
    FRAGCOLOR = vec4(0);

    // The camera position (eye) in model space
    const vec3 ray_origin_model = (inverse(MODEL_MATRIX) * vec4(CAMERA_POSITION, 1)).xyz;

    // Get the ray intersection with the sliced box
    float t_0, t_1;
    const vec3 top_sliced = vec3(100)*sliceMax - vec3(50);
    const vec3 bottom_sliced = vec3(100)*sliceMin - vec3(50);
    if (!ray_box_intersection(ray_origin_model, ray_direction_model, bottom_sliced, top_sliced, t_0, t_1))
        return; // No ray intersection with sliced box, nothing to render

    // Get the start/end points of the ray in original box
    const vec3 top = vec3(50, 50, 50);
    const vec3 bottom = vec3(-50, -50, -50);
    const vec3 ray_start = (ray_origin_model + ray_direction_model * t_0 - bottom) / (top - bottom);
    const vec3 ray_stop =  (ray_origin_model + ray_direction_model * t_1 - bottom) / (top - bottom);

    vec3 ray = ray_stop - ray_start;
    float ray_length = length(ray);
    vec3 step_vector = stepLength * ray / ray_length;

    vec3 position = ray_start;

    // Ray march until reaching the end of the volume, or color saturation
    while (ray_length > 0) {
        ray_length -= stepLength;
        position += step_vector;

        float val = textureLod(volume, position, 0).r;
        if (val == 0 || val < tMin || val > tMax)
            continue;

        const float alpha = multipliedAlpha ? val * stepAlpha : stepAlpha;
        vec4 val_color = vec4(textureLod(colormap, vec2(val, 0.5), 0).rgb, alpha);
        // Opacity correction
        val_color.a = 1.0 - pow(max(0.0, 1.0 - val_color.a), 1.0);
        FRAGCOLOR.rgb += (1.0 - FRAGCOLOR.a) * val_color.a * val_color.rgb;
        FRAGCOLOR.a += (1.0 - FRAGCOLOR.a) * val_color.a;
        if (FRAGCOLOR.a >= 0.95)
            break;
    }
}
//! [main]
