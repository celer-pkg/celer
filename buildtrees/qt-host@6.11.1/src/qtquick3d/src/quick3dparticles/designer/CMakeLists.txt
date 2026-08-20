# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_path_join(destination ${QT_INSTALL_DIR} "${INSTALL_QMLDIR}/QtQuick3D/Particles3D/designer")
qt_copy_or_install(
    FILES
        Particle3DSection.qml
        Particle3DSpecifics.qml
        Affector3DSection.qml
        Affector3DSpecifics.qml
        Attractor3DSection.qml
        Attractor3DSpecifics.qml
        EmitBurst3DSection.qml
        EmitBurst3DSpecifics.qml
        DynamicBurst3DSection.qml
        DynamicBurst3DSpecifics.qml
        ParticleEmitter3DSection.qml
        ParticleEmitter3DSpecifics.qml
        Gravity3DSection.qml
        Gravity3DSpecifics.qml
        LineParticle3DSection.qml
        LineParticle3DSpecifics.qml
        ModelBlendParticle3DSection.qml
        ModelBlendParticle3DSpecifics.qml
        ModelParticle3DSection.qml
        ModelParticle3DSpecifics.qml
        NodeSection.qml
        NodeSpecifics.qml
        ParticleCustomShape3DSection.qml
        ParticleCustomShape3DSpecifics.qml
        ParticleModelShape3DSection.qml
        ParticleModelShape3DSpecifics.qml
        PointRotator3DSection.qml
        PointRotator3DSpecifics.qml
        ParticleShape3DSection.qml
        ParticleShape3DSpecifics.qml
        Repeller3DSection.qml
        Repeller3DSpecifics.qml
        SpriteParticle3DSection.qml
        SpriteParticle3DSpecifics.qml
        SpriteSequence3DSection.qml
        SpriteSequence3DSpecifics.qml
        ScaleAffector3DSection.qml
        ScaleAffector3DSpecifics.qml
        ParticleSystem3DSection.qml
        ParticleSystem3DSpecifics.qml
        TargetDirection3DSection.qml
        TargetDirection3DSpecifics.qml
        TrailEmitter3DSection.qml
        TrailEmitter3DSpecifics.qml
        VectorDirection3DSection.qml
        VectorDirection3DSpecifics.qml
        Wander3DSection.qml
        Wander3DSpecifics.qml
        particles3d.metainfo
    DESTINATION "${destination}"
)

add_subdirectory(images)
add_subdirectory(source)
