// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QCATEGORY3DAXIS_H
#define QTGRAPHS_QCATEGORY3DAXIS_H

#include <QtGraphs/qabstract3daxis.h>

QT_BEGIN_NAMESPACE

class QCategory3DAxisPrivate;

class Q_GRAPHS_EXPORT QCategory3DAxis : public QAbstract3DAxis
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QCategory3DAxis)
    Q_PROPERTY(QStringList labels READ labels WRITE setLabels NOTIFY labelsChanged FINAL)
    QML_NAMED_ELEMENT(Category3DAxis)

public:
    explicit QCategory3DAxis(QObject *parent = nullptr);
    ~QCategory3DAxis() override;

    void setLabels(const QStringList &labels);
    QStringList labels() const;

Q_SIGNALS:
    void rowLabelsChanged();
    void columnLabelsChanged();

private:
    Q_DISABLE_COPY(QCategory3DAxis)

    friend class QQuickGraphsBars;
    friend class QQuickGraphsItem;
};

QT_END_NAMESPACE

#endif
