// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D

View3D {
    DefaultMaterialA {
        id: defAMatUnused
    }

    DefaultMaterialA {
        id: defAMatOne
    }

    DefaultMaterialA {
        id: defAMatTwo
    }

    // Model component using inherited material (1)
    ModelA {
        opacity: 0.5
    }

    // Model using referenced material component (1)
    Model {
        source: "#Cube"
        materials: defAMatOne // Script binding -> materials -> Identifier: defAMatOne
    }

    // Model using inline material component (1)
    Model {
        materials: DefaultMaterialA { } // Object binding -> materials: DefaultMaterialA
        source: "#Cube"
    }

    // Component model overriding materials with refed material (1)
    ModelA {
        materials: [ defAMatOne ] // Script binding -> materials -> [ -> Identifier: defAMatOne ]
    }

    // Component model overriding materials with inline material (1)
    ModelA {
        materials: [ DefaultMaterial { } ] // Array binding(s) -> materials: [ ... ]
    }

    // Component model overriding materials with refed materials (2)
    ModelA {
        materials: [ defAMatOne, defAMatTwo ] // Script binding -> materials -> [ -> Identifier: defAMatOne, (...) defAMatTwo ]
    }

    // Component model overriding materials with inline materials (2)
    ModelA {
        id: modelWithInlineMaterials
        materials: [ DefaultMaterialA { // Array binding(s) -> materials: [ ... ]
                id: oneInline
            }, DefaultMaterialA {
                id: twoInline
            } ]
    }
}
