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

#ifndef ENUMTOSTRINGMAP_P_H
#define ENUMTOSTRINGMAP_P_H

//#define VERBOSE_STATE_STORE

#ifdef VERBOSE_STATE_STORE

#include <QtGui/qopengl.h>
#include <QMap>
#include <QString>
#include <private/qglobal_p.h>

class EnumToStringMap
{
public:
    static EnumToStringMap *newInstance();
    static void deleteInstance();
    QString lookUp(const GLuint value) const;

private:
    EnumToStringMap();
    QMap<int, QString> m_map;
    QString m_unknown;
};
#endif
#endif
