// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QLOGVALUE3DAXISFORMATTER_H
#define QTDATAVIS3D_QLOGVALUE3DAXISFORMATTER_H

#include <QtDataVisualization/qvalue3daxisformatter.h>

QT_BEGIN_NAMESPACE

class QLogValue3DAxisFormatterPrivate;

class Q_DATAVISUALIZATION_EXPORT QLogValue3DAxisFormatter : public QValue3DAxisFormatter
{
    Q_OBJECT

    Q_PROPERTY(qreal base READ base WRITE setBase NOTIFY baseChanged)
    Q_PROPERTY(bool autoSubGrid READ autoSubGrid WRITE setAutoSubGrid NOTIFY autoSubGridChanged)
    Q_PROPERTY(bool showEdgeLabels READ showEdgeLabels WRITE setShowEdgeLabels NOTIFY showEdgeLabelsChanged)

protected:
    explicit QLogValue3DAxisFormatter(QLogValue3DAxisFormatterPrivate *d, QObject *parent = nullptr);
public:
    explicit QLogValue3DAxisFormatter(QObject *parent = nullptr);
    virtual ~QLogValue3DAxisFormatter();

    void setBase(qreal base);
    qreal base() const;
    void setAutoSubGrid(bool enabled);
    bool autoSubGrid() const;
    void setShowEdgeLabels(bool enabled);
    bool showEdgeLabels() const;

Q_SIGNALS:
    void baseChanged(qreal base);
    void autoSubGridChanged(bool enabled);
    void showEdgeLabelsChanged(bool enabled);

protected:
    QValue3DAxisFormatter *createNewInstance() const override;
    void recalculate() override;
    float positionAt(float value) const override;
    float valueAt(float position) const override;
    void populateCopy(QValue3DAxisFormatter &copy) const override;

    QLogValue3DAxisFormatterPrivate *dptr();
    const QLogValue3DAxisFormatterPrivate *dptrc() const;

private:
    Q_DISABLE_COPY(QLogValue3DAxisFormatter)

    friend class QLogValue3DAxisFormatterPrivate;
};

QT_END_NAMESPACE

#endif
