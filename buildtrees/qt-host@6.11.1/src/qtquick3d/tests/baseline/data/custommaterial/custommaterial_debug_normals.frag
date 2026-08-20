// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

void MAIN()
{
    // Apply TBN-space normal map. In debug Normals mode the NORMAL assignment
    // must take effect so the debug visualization reflects the perturbed normal.
    vec3 normalValue = texture(normalMap, UV0).rgb;
    normalValue.xy = normalValue.xy * 2.0 - 1.0;
    normalValue.z = sqrt(max(0.0, 1.0 - dot(normalValue.xy, normalValue.xy)));
    NORMAL = normalize(TANGENT * normalValue.x + BINORMAL * normalValue.y + NORMAL * normalValue.z);
}
