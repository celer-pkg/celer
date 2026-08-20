// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef FOREIGNTYPES_P_H
#define FOREIGNTYPES_P_H

#include <QtQml/qqml.h>

#include <QtCore/qabstractitemmodel.h>

#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/q3dinputhandler.h>
#include <QtDataVisualization/q3dlight.h>
#include <QtDataVisualization/q3dobject.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QtDataVisualization/qabstract3daxis.h>
#include <QtDataVisualization/qabstract3dinputhandler.h>
#include <QtDataVisualization/qabstract3dseries.h>
#include <QtDataVisualization/qabstractdataproxy.h>
#include <QtDataVisualization/qbar3dseries.h>
#include <QtDataVisualization/qbardataproxy.h>
#include <QtDataVisualization/qcategory3daxis.h>
#include <QtDataVisualization/qcustom3ditem.h>
#include <QtDataVisualization/qcustom3dlabel.h>
#include <QtDataVisualization/qcustom3dvolume.h>
#include <QtDataVisualization/qheightmapsurfacedataproxy.h>
#include <QtDataVisualization/qitemmodelbardataproxy.h>
#include <QtDataVisualization/qitemmodelscatterdataproxy.h>
#include <QtDataVisualization/qitemmodelsurfacedataproxy.h>
#include <QtDataVisualization/qlogvalue3daxisformatter.h>
#include <QtDataVisualization/qscatter3dseries.h>
#include <QtDataVisualization/qscatterdataproxy.h>
#include <QtDataVisualization/qsurface3dseries.h>
#include <QtDataVisualization/qsurfacedataproxy.h>
#include <QtDataVisualization/qtouch3dinputhandler.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/qvalue3daxisformatter.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

#define DEFINE_FOREIGN_BASE_ATTRIBUTES(type, name, minor) \
        Q_GADGET \
        QML_NAMED_ELEMENT(name) \
        QML_FOREIGN(type) \
        QML_ADDED_IN_VERSION(1, minor) \

#define DEFINE_FOREIGN_UNCREATABLE_TYPE(type, name) \
    struct type##DataVisForeign \
    { \
        DEFINE_FOREIGN_BASE_ATTRIBUTES(type, name, 0) \
        QML_UNCREATABLE("Trying to create uncreatable: " #name ".") \
    };

#define DEFINE_FOREIGN_CREATABLE_TYPE(type, name, minor) \
    struct type##DataVisForeign \
    { \
        DEFINE_FOREIGN_BASE_ATTRIBUTES(type, name, minor) \
    };

#define DEFINE_FOREIGN_REPLACED_TYPE(type, name, better) \
    struct type##DataVisForeign \
    { \
        DEFINE_FOREIGN_BASE_ATTRIBUTES(type, name, 0) \
        QML_UNCREATABLE("Trying to create uncreatable: " #name ", use " #better " instead.") \
    };

struct Q3DSceneForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_ADDED_IN_VERSION(1, 0)
    QML_FOREIGN(Q3DScene)
};

DEFINE_FOREIGN_CREATABLE_TYPE(Q3DCamera, Camera3D, 0)
DEFINE_FOREIGN_CREATABLE_TYPE(Q3DLight, Light3D, 0)
DEFINE_FOREIGN_CREATABLE_TYPE(QCategory3DAxis, CategoryAxis3D, 0)
DEFINE_FOREIGN_CREATABLE_TYPE(QHeightMapSurfaceDataProxy, HeightMapSurfaceDataProxy, 0)
DEFINE_FOREIGN_CREATABLE_TYPE(QItemModelBarDataProxy, ItemModelBarDataProxy, 0)
DEFINE_FOREIGN_CREATABLE_TYPE(QItemModelScatterDataProxy, ItemModelScatterDataProxy, 0)
DEFINE_FOREIGN_CREATABLE_TYPE(QItemModelSurfaceDataProxy, ItemModelSurfaceDataProxy, 0)
DEFINE_FOREIGN_CREATABLE_TYPE(QValue3DAxis, ValueAxis3D, 0)

DEFINE_FOREIGN_CREATABLE_TYPE(QCustom3DItem, Custom3DItem, 1)
DEFINE_FOREIGN_CREATABLE_TYPE(QCustom3DLabel, Custom3DLabel, 1)
DEFINE_FOREIGN_CREATABLE_TYPE(QLogValue3DAxisFormatter, LogValueAxis3DFormatter, 1)
DEFINE_FOREIGN_CREATABLE_TYPE(QValue3DAxisFormatter, ValueAxis3DFormatter, 1)

DEFINE_FOREIGN_CREATABLE_TYPE(Q3DInputHandler, InputHandler3D, 2)
DEFINE_FOREIGN_CREATABLE_TYPE(QCustom3DVolume, Custom3DVolume, 2)
DEFINE_FOREIGN_CREATABLE_TYPE(QTouch3DInputHandler, TouchInputHandler3D, 2)

DEFINE_FOREIGN_REPLACED_TYPE(Q3DTheme, Q3DTheme, Theme3D)
DEFINE_FOREIGN_REPLACED_TYPE(QBar3DSeries, QBar3DSeries, Bar3DSeries)
DEFINE_FOREIGN_REPLACED_TYPE(QScatter3DSeries, QScatter3DSeries, Scatter3DSeries)
DEFINE_FOREIGN_REPLACED_TYPE(QSurface3DSeries, QSurface3DSeries, Surface3DSeries)

DEFINE_FOREIGN_UNCREATABLE_TYPE(Q3DObject, Object3D)
DEFINE_FOREIGN_UNCREATABLE_TYPE(QAbstract3DAxis, AbstractAxis3D)
DEFINE_FOREIGN_UNCREATABLE_TYPE(QAbstract3DInputHandler, AbstractInputHandler3D)
DEFINE_FOREIGN_UNCREATABLE_TYPE(QAbstract3DSeries, Abstract3DSeries)
DEFINE_FOREIGN_UNCREATABLE_TYPE(QAbstractDataProxy, AbstractDataProxy)
DEFINE_FOREIGN_UNCREATABLE_TYPE(QAbstractItemModel, AbstractItemModel)
DEFINE_FOREIGN_UNCREATABLE_TYPE(QBarDataProxy, BarDataProxy)
DEFINE_FOREIGN_UNCREATABLE_TYPE(QScatterDataProxy, ScatterDataProxy)
DEFINE_FOREIGN_UNCREATABLE_TYPE(QSurfaceDataProxy, SurfaceDataProxy)

QT_END_NAMESPACE

#endif // FOREIGNTYPES_P_H
