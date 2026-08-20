// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: root

    property var matLib

    position: Qt.vector3d(0.18901094794273376, 0.34249943494796753, -0.004048084374517202)
    rotation: Qt.quaternion(0.718825, -0.136648, -0.681451, -0.0155715)

    CustomModel {
        id: annoyedEyes
        position: Qt.vector3d(-0.0003483230248093605, -0.025516914203763008, -0.012772757560014725)
        rotation: Qt.quaternion(0.98654, 0.14128, -0.0284962, -0.0772424)
        scale: Qt.vector3d(0.0396734, 0.0396735, 0.0396735)
        visible: AccessoryState.annoyedEyesVisible
        source: "meshes/mesh_005_mesh.mesh"
        materials: root.matLib.annoyedEyes
        CustomModel {
            id: eyebrows_005
            position: Qt.vector3d(0.0007747262716293335, -2.0784430503845215, -4.479295253753662)
            scale: Qt.vector3d(6.43903, 6.43903, 6.43903)
            source: "meshes/mesh_007_mesh.mesh"
            materials: root.matLib.eyeBrows_Black
        }
    }
    CustomModel {
        id: confusedEyes
        position: Qt.vector3d(0.0028726072050631046, 0.006814511958509684, -0.03231776878237724)
        rotation: Qt.quaternion(0.987609, 0.137125, -0.0128826, -0.0752226)
        scale: Qt.vector3d(0.344056, 0.344056, 0.344056)
        visible: AccessoryState.confusedEyesVisible
        source: "meshes/plane_014_mesh.mesh"
        materials: root.matLib.eyeBrows_Black
    }
    CustomModel {
        id: cuteEyes
        position: Qt.vector3d(0.004, 0.004, -0.039)
        eulerRotation: Qt.vector3d(14, 0, -9)
        scale: Qt.vector3d(0.0396734, 0.0396735, 0.0396735)
        visible: AccessoryState.cuteEyesVisible
        source: "meshes/mesh_003_mesh.mesh"
        materials: root.matLib.quteEyes
        CustomModel {
            id: eyebrows_006
            position: Qt.vector3d(0, -3, -4.5)
            eulerRotation: Qt.vector3d(0, 0, 0)
            scale: Qt.vector3d(6.43903, 6.43903, 6.43903)
            source: "meshes/mesh_004_mesh.mesh"
            materials: root.matLib.eyeBrows_Black_004
        }
    }
    CustomModel {
        id: powerpuffEyes
        position: Qt.vector3d(-0.002, -0.029, -0.0413)
        rotation: Qt.quaternion(0.776248, -0.624027, 0.0418585, -0.0792301)
        visible: AccessoryState.powerpuffEyesVisible
        source: "meshes/mesh_011_mesh.mesh"
        materials: root.matLib.powerpuffEyes_001
        CustomModel {
            id: eyebrows_007
            position: Qt.vector3d(0, 0.19136035442352295, -0.08120261132717133)
            rotation: Qt.quaternion(0.707107, 0.707107, 0, 0)
            scale: Qt.vector3d(0.255459, 0.255459, 0.255459)
            source: "meshes/mesh_014_mesh.mesh"
            materials: root.matLib.eyeBrows_Black_001
        }
    }
    CustomModel {
        id: smallEyes
        position: Qt.vector3d(0.013645559549331665, 0.014445514418184757, -0.028406282886862755)
        rotation: Qt.quaternion(0.710811, -0.696372, 0.0550266, -0.0823777)
        visible: AccessoryState.smallEyesVisible
        source: "meshes/mesh_124_mesh.mesh"
        materials: root.matLib.defaultEyes
        CustomModel {
            id: eyebrows_008
            position: Qt.vector3d(-0.003959749825298786, 0.16850973665714264, -0.08624143153429031)
            rotation: Qt.quaternion(0.707107, 0.707107, 0, 0)
            scale: Qt.vector3d(0.255459, 0.255459, 0.255459)
            source: "meshes/mesh_016_mesh.mesh"
            materials: root.matLib.eyeBrows_Black_005
        }
    }
    CustomModel {
        id: surprisedEyes
        position: Qt.vector3d(0.0008958298712968826, -0.022206032648682594, -0.0632537454366684)
        rotation: Qt.quaternion(0.995711, 0.0301848, -0.0157461, -0.086022)
        scale: Qt.vector3d(0.0396735, 0.0396735, 0.0396735)
        visible: AccessoryState.surprisedEyesVisible
        source: "meshes/mesh_008_mesh.mesh"
        materials: root.matLib.defaultEyes
        Model {
            id: eyebrows
            position: Qt.vector3d(0, -2.889869451522827, -6.205251216888428)
            scale: Qt.vector3d(6.43903, 6.43903, 6.43903)
            source: "meshes/mesh_009_mesh.mesh"
            materials: root.matLib.eyeBrows_Black
        }
    }
    CustomModel {
        id: wideEyes
        position: Qt.vector3d(-0.012811146676540375, -0.09096617251634598, -0.02168007753789425)
        rotation: Qt.quaternion(0.989466, 0.11526, -0.00849352, -0.0871826)
        scale: Qt.vector3d(0.213261, 0.213261, 0.213261)
        visible: AccessoryState.wideEyesVisible
        source: "meshes/mesh_010_mesh.mesh"
        materials: root.matLib.eyeBrows_Black

        Model {
            id: plane_002
            position: Qt.vector3d(0, -0.19275015592575073, 1.4294742345809937)
            materials: root.matLib.eyeBrows_Black_002
            source: "meshes/plane_015_mesh.mesh"
        }
        Model {
            id: plane_003
            position: Qt.vector3d(0, -0.23832258582115173, 1.4294743537902832)
            materials: root.matLib.eyeBrows_Black_003
            source: "meshes/plane_016_mesh.mesh"
        }
        CustomModel {
            id: plane_004
            position: Qt.vector3d(0.00478070043027401, 0.39956894516944885, -0.13168218731880188)
            scale: Qt.vector3d(0.303691, 0.303691, 0.303691)
            materials: root.matLib.eyeBrows_Black_004
            source: "meshes/plane_017_mesh.mesh"
        }
    }
}
