// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QLOGVALUE3DAXISFORMATTER_H
#define QTGRAPHS_QLOGVALUE3DAXISFORMATTER_H

#include <QtGraphs/qvalue3daxisformatter.h>

QT_BEGIN_NAMESPACE

class QLogValue3DAxisFormatterPrivate;

class Q_GRAPHS_EXPORT QLogValue3DAxisFormatter : public QValue3DAxisFormatter
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QLogValue3DAxisFormatter)
    Q_PROPERTY(qreal base READ base WRITE setBase NOTIFY baseChanged FINAL)
    Q_PROPERTY(bool autoSubGrid READ autoSubGrid WRITE setAutoSubGrid NOTIFY autoSubGridChanged FINAL)
    Q_PROPERTY(bool edgeLabelsVisible READ edgeLabelsVisible WRITE setEdgeLabelsVisible NOTIFY
                   edgeLabelsVisibleChanged FINAL)
    QML_NAMED_ELEMENT(LogValue3DAxisFormatter)

protected:
    explicit QLogValue3DAxisFormatter(QLogValue3DAxisFormatterPrivate &d, QObject *parent = nullptr);

public:
    explicit QLogValue3DAxisFormatter(QObject *parent = nullptr);
    ~QLogValue3DAxisFormatter() override;

    void setBase(qreal base);
    qreal base() const;
    void setAutoSubGrid(bool enabled);
    bool autoSubGrid() const;
    void setEdgeLabelsVisible(bool enabled);
    bool edgeLabelsVisible() const;

Q_SIGNALS:
    void baseChanged(qreal base);
    void autoSubGridChanged(bool enabled);
    void edgeLabelsVisibleChanged(bool enabled);

protected:
    QValue3DAxisFormatter *createNewInstance() const override;
    void recalculate() override;
    float positionAt(float value) const override;
    float valueAt(float position) const override;
    void populateCopy(QValue3DAxisFormatter &copy) override;

private:
    Q_DISABLE_COPY(QLogValue3DAxisFormatter)
};

QT_END_NAMESPACE

#endif
