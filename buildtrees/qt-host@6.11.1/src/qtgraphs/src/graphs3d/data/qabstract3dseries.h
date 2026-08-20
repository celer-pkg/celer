// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QABSTRACT3DSERIES_H
#define QTGRAPHS_QABSTRACT3DSERIES_H

#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qstring.h>
#include <QtGraphs/qgraphstheme.h>
#include <QtGui/qbrush.h>
#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

class QAbstract3DSeriesPrivate;

class Q_GRAPHS_EXPORT QAbstract3DSeries : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstract3DSeries)
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_PROPERTY(QAbstract3DSeries::SeriesType type READ type CONSTANT)
    Q_PROPERTY(QString itemLabelFormat READ itemLabelFormat WRITE setItemLabelFormat NOTIFY
                   itemLabelFormatChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(QAbstract3DSeries::Mesh mesh READ mesh WRITE setMesh NOTIFY meshChanged)
    Q_PROPERTY(bool meshSmooth READ isMeshSmooth WRITE setMeshSmooth NOTIFY meshSmoothChanged)
    Q_PROPERTY(
        QQuaternion meshRotation READ meshRotation WRITE setMeshRotation NOTIFY meshRotationChanged)
    Q_PROPERTY(QString userDefinedMesh READ userDefinedMesh WRITE setUserDefinedMesh NOTIFY
                   userDefinedMeshChanged)
    Q_PROPERTY(QGraphsTheme::ColorStyle colorStyle READ colorStyle WRITE setColorStyle NOTIFY
                   colorStyleChanged)
    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged)
    Q_PROPERTY(QLinearGradient baseGradient READ baseGradient WRITE setBaseGradient NOTIFY
                   baseGradientChanged)
    Q_PROPERTY(QColor singleHighlightColor READ singleHighlightColor WRITE setSingleHighlightColor
                   NOTIFY singleHighlightColorChanged)
    Q_PROPERTY(QLinearGradient singleHighlightGradient READ singleHighlightGradient WRITE
                   setSingleHighlightGradient NOTIFY singleHighlightGradientChanged)
    Q_PROPERTY(QColor multiHighlightColor READ multiHighlightColor WRITE setMultiHighlightColor
                   NOTIFY multiHighlightColorChanged)
    Q_PROPERTY(QLinearGradient multiHighlightGradient READ multiHighlightGradient WRITE
                   setMultiHighlightGradient NOTIFY multiHighlightGradientChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString itemLabel READ itemLabel NOTIFY itemLabelChanged)
    Q_PROPERTY(bool itemLabelVisible READ isItemLabelVisible WRITE setItemLabelVisible NOTIFY
                   itemLabelVisibleChanged)
    Q_PROPERTY(QAbstract3DSeries::LightingMode lightingMode READ lightingMode WRITE setLightingMode
               NOTIFY lightingModeChanged REVISION(6,10))
    QML_NAMED_ELEMENT(Abstract3DSeries)
    QML_UNCREATABLE("Uncreatable base type")
public:
    enum class SeriesType {
        None,
        Bar,
        Scatter,
        Surface,
    };
    Q_ENUM(SeriesType)

    enum class Mesh {
        UserDefined,
        Bar,
        Cube,
        Pyramid,
        Cone,
        Cylinder,
        BevelBar,
        BevelCube,
        Sphere,
        Minimal,
        Arrow,
        Point,
    };
    Q_ENUM(Mesh)

    enum class LightingMode : bool {
        Shaded,
        Unshaded,
    };
    Q_ENUM(LightingMode)

protected:
    explicit QAbstract3DSeries(QAbstract3DSeriesPrivate &d, QObject *parent = nullptr);

public:
    ~QAbstract3DSeries() override;

    QAbstract3DSeries::SeriesType type() const;

    void setItemLabelFormat(const QString &format);
    QString itemLabelFormat() const;

    void setVisible(bool visible);
    bool isVisible() const;

    void setMesh(QAbstract3DSeries::Mesh mesh);
    QAbstract3DSeries::Mesh mesh() const;

    void setMeshSmooth(bool enable);
    bool isMeshSmooth() const;

    void setMeshRotation(const QQuaternion &rotation);
    QQuaternion meshRotation() const;
    Q_INVOKABLE void setMeshAxisAndAngle(QVector3D axis, float angle);

    void setUserDefinedMesh(const QString &fileName);
    QString userDefinedMesh() const;

    void setColorStyle(QGraphsTheme::ColorStyle style);
    QGraphsTheme::ColorStyle colorStyle() const;
    void setBaseColor(QColor color);
    QColor baseColor() const;
    void setBaseGradient(const QLinearGradient &gradient);
    QLinearGradient baseGradient() const;
    void setSingleHighlightColor(QColor color);
    QColor singleHighlightColor() const;
    void setSingleHighlightGradient(const QLinearGradient &gradient);
    QLinearGradient singleHighlightGradient() const;
    void setMultiHighlightColor(QColor color);
    QColor multiHighlightColor() const;
    void setMultiHighlightGradient(const QLinearGradient &gradient);
    QLinearGradient multiHighlightGradient() const;

    LightingMode lightingMode() const;
    void setLightingMode(LightingMode lightingMode);

    void setName(const QString &name);
    QString name() const;

    QString itemLabel();
    void setItemLabelVisible(bool visible);
    bool isItemLabelVisible() const;

Q_SIGNALS:
    void itemLabelFormatChanged(const QString &format);
    void visibleChanged(bool visible);
    void meshChanged(QAbstract3DSeries::Mesh mesh);
    void meshSmoothChanged(bool enabled);
    void meshRotationChanged(const QQuaternion &rotation);
    void userDefinedMeshChanged(const QString &fileName);
    void colorStyleChanged(QGraphsTheme::ColorStyle style);
    void baseColorChanged(QColor color);
    void baseGradientChanged(const QLinearGradient &gradient);
    void singleHighlightColorChanged(QColor color);
    void singleHighlightGradientChanged(const QLinearGradient &gradient);
    void multiHighlightColorChanged(QColor color);
    void multiHighlightGradientChanged(const QLinearGradient &gradient);
    void nameChanged(const QString &name);
    void itemLabelChanged(const QString &label);
    void itemLabelVisibleChanged(bool visible);
    void lightingModeChanged(QAbstract3DSeries::LightingMode lightingMode);

private:
    Q_DISABLE_COPY(QAbstract3DSeries)

    friend class QQuickGraphsItem;
    friend class QQuickGraphsBars;
    friend class QQuickGraphsSurface;
    friend class QQuickGraphsScatter;
};

QT_END_NAMESPACE

#endif
