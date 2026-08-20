// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QBAR3DSERIES_H
#define QTGRAPHS_QBAR3DSERIES_H

#include <QtCore/qpoint.h>
#include <QtGraphs/qabstract3dseries.h>
#include <QtGraphs/qbardataproxy.h>
#include <QtGraphs/qvalue3daxis.h>
#include <QtGraphs/qcategory3daxis.h>

QT_BEGIN_NAMESPACE

class QBar3DSeriesPrivate;
class QQuaternion;

class Q_GRAPHS_EXPORT QBar3DSeries : public QAbstract3DSeries
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QBar3DSeries)
    Q_PROPERTY(
        QBarDataProxy *dataProxy READ dataProxy WRITE setDataProxy NOTIFY dataProxyChanged FINAL)
    Q_PROPERTY(QPoint selectedBar READ selectedBar WRITE setSelectedBar NOTIFY selectedBarChanged)
    Q_PROPERTY(float meshAngle READ meshAngle WRITE setMeshAngle NOTIFY meshAngleChanged FINAL)
    Q_PROPERTY(QList<QColor> rowColors READ rowColors WRITE setRowColors NOTIFY rowColorsChanged)
    Q_PROPERTY(QStringList rowLabels READ rowLabels WRITE setRowLabels NOTIFY rowLabelsChanged FINAL)
    Q_PROPERTY(QStringList columnLabels READ columnLabels WRITE setColumnLabels NOTIFY
                   columnLabelsChanged FINAL)
    Q_PROPERTY(QStringList customRowLabels READ customRowLabels WRITE setCustomRowLabels NOTIFY
                   customRowLabelsChanged REVISION(6, 11))
    Q_PROPERTY(QStringList customColumnLabels READ customColumnLabels WRITE setCustomColumnLabels
                   NOTIFY customColumnLabelsChanged REVISION(6, 11))
    Q_PROPERTY(
        QBarDataArray dataArray READ dataArray WRITE setDataArray NOTIFY dataArrayChanged FINAL)
    Q_PROPERTY(bool valueColoringEnabled READ isValueColoringEnabled WRITE setValueColoringEnabled
                   NOTIFY valueColoringEnabledChanged REVISION(6, 9))
    Q_PROPERTY(QCategory3DAxis *rowAxis READ rowAxis WRITE setRowAxis RESET resetRowAxis NOTIFY
                       rowAxisChanged REVISION(6, 11))
    Q_PROPERTY(QValue3DAxis *valueAxis READ valueAxis WRITE setValueAxis RESET resetValueAxis NOTIFY
                       valueAxisChanged REVISION(6, 11))
    Q_PROPERTY(QCategory3DAxis *columnAxis READ columnAxis WRITE setColumnAxis RESET resetColumnAxis
                       NOTIFY columnAxisChanged REVISION(6, 11))
    QML_ELEMENT
    QML_UNCREATABLE("Trying to create uncreatable: QBar3DSeries, use Bar3DSeries instead.")
public:
    explicit QBar3DSeries(QObject *parent = nullptr);
    explicit QBar3DSeries(QBarDataProxy *dataProxy, QObject *parent = nullptr);
    ~QBar3DSeries() override;

    void setDataProxy(QBarDataProxy *proxy);
    QBarDataProxy *dataProxy() const;

    void setSelectedBar(QPoint position);
    QPoint selectedBar() const;
    static QPoint invalidSelectionPosition();

    void setMeshAngle(float angle);
    float meshAngle() const;

    QList<QColor> rowColors() const;
    void setRowColors(const QList<QColor> &colors);

    bool isValueColoringEnabled() const;
    void setValueColoringEnabled(bool enabled);

    void setDataArray(const QBarDataArray &newDataArray);
    void clearRow(qsizetype rowIndex);
    void clearArray();
    const QBarDataArray &dataArray() const &;
    QBarDataArray dataArray() &&;
    QStringList rowLabels() const;
    void setRowLabels(const QStringList &labels);
    QStringList columnLabels() const;
    void setColumnLabels(const QStringList &labels);

    QStringList customRowLabels() const;
    void setCustomRowLabels(const QStringList &labels);
    QStringList customColumnLabels() const;
    void setCustomColumnLabels(const QStringList &labels);

    void setRowAxis(QCategory3DAxis *axis);
    void setValueAxis(QValue3DAxis *axis);
    void setColumnAxis(QCategory3DAxis *axis);
    QCategory3DAxis *rowAxis() const;
    QValue3DAxis *valueAxis() const;
    QCategory3DAxis *columnAxis() const;
    void resetRowAxis();
    void resetValueAxis();
    void resetColumnAxis();


Q_SIGNALS:
    void dataProxyChanged(QBarDataProxy *proxy);
    void selectedBarChanged(QPoint position);
    void meshAngleChanged(float angle);
    void rowColorsChanged(const QList<QColor> &rowcolors);
    void rowLabelsChanged();
    void columnLabelsChanged();
    void dataArrayChanged(const QBarDataArray &array);
    Q_REVISION(6, 9) void valueColoringEnabledChanged(bool enabled);
    Q_REVISION(6, 11) void customRowLabelsChanged(const QStringList &newCustomRowLabels);
    Q_REVISION(6, 11) void customColumnLabelsChanged(const QStringList &newCustomColumnLabels);
    Q_REVISION(6, 11) void rowAxisChanged(QCategory3DAxis *axis);
    Q_REVISION(6, 11) void valueAxisChanged(QValue3DAxis *axis);
    Q_REVISION(6, 11) void columnAxisChanged(QCategory3DAxis *axis);

private:
    Q_DISABLE_COPY(QBar3DSeries)

    void connectSignals();
    void handleMeshRotationChanged(const QQuaternion &rotation);

    friend class QQuickGraphsBars;
};

QT_END_NAMESPACE

#endif
