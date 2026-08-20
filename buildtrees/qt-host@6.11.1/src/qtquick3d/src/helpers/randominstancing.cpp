// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "randominstancing_p.h"
#include <QRandomGenerator>
#include <QObject>

QT_BEGIN_NAMESPACE

/*!
    \qmltype InstanceRange
    \inherits Object3D
    \inqmlmodule QtQuick3D.Helpers
    \since 6.2
    \brief Specifies a range for RandomInstancing.

    The InstanceRange QML type is used to specify the range of variation for
    RandomInstancing attributes.
*/

/*!
    \qmlproperty Variant InstanceRange::from

    This property specifies the lower bound of the range. The type needs to match the type of the attribute that this range is applied to.
*/

/*!
    \qmlproperty Variant InstanceRange::to

    This property specifies the upper bound of the range. The type needs to match the type of the attribute that this range is applied to.
*/

/*!
    \qmlproperty bool InstanceRange::proportional

    This property determines whether the components of the attribute vary proportionally or independently.
    The default value is \c true, meaning that all components are independent.

    For example, the following defines a scaling range that preserves the aspect ratio of the model:
    \qml
        InstanceRange {
            from: Qt.vector3d(1, 1, 1)
            to: Qt.vector3d(5, 5, 5)
            proportional: true
        }
    \endqml

    This defines a greyscale color range:
    \qml
        InstanceRange {
            from: "black"
            to: "white"
            proportional: true
        }
    \endqml

    While the following defines a range that covers all colors
    \qml
        InstanceRange {
            from: "black"
            to: "white"
        }
    \endqml
*/

/*!
    \qmltype RandomInstancing
    \inherits Instancing
    \inqmlmodule QtQuick3D.Helpers
    \since 6.2
    \brief Generates a random instancing table.

    The RandomInstancing type provides an easy way to generate a large number of
    random instances within defined bounds. The number of instances is defined by the
    \l instanceCount property. The bounds are defined by the properties
    \l position, \l scale, \l rotation, \l color, and \l customData.

    \sa InstanceList
*/

/*!
    \qmlproperty int RandomInstancing::instanceCount

    The instanceCount property specifies the number of instances to generate. Changing this value will regenerate the whole table.

    \sa randomSeed
*/

/*!
    \qmlproperty int RandomInstancing::randomSeed

    This property defines the seed for the random number generator. Setting this to a value
    different from -1 guarantees that the instance table will have the same content each time it is generated.
    Note that adding or changing attributes may cause a completely different table to be generated.

    The default value is -1, causing the table to get a new random value each time it is generated.
*/

/*!
    \qmlproperty InstanceRange RandomInstancing::position

    The position property defines the geometrical bounds of the generated instances.
    The default value is empty, causing a generated position of \c{[0, 0, 0]}.

    \sa color, rotation, scale, customData
*/

/*!
    \qmlproperty vector3d RandomInstancing::gridSpacing
    \since 6.9

    The gridSpacing property defines the minimum spacing between instances, ensuring they do not overlap.
    Each position will be separated by at least the value specified in \c gridSpacing.

    If the specified \c gridSpacing cannot accommodate the requested number of instances, the \c instanceCount
    property will be reduced to the number of instances that can be placed without overlap.

    \note The gridSpacing property affects only the position of instances.
    Rotation and scaling applied to instances are not considered in the spacing algorithm.

    The default value is \c{[0, 0, 0]},  which imposes no restriction on overlapping instances.

    \sa position
*/

/*!
    \qmlproperty InstanceRange RandomInstancing::scale

    The scale property defines the scaling limits for the generated instances. The type is
    \l vector3d.
    Set \l {InstanceRange::proportional}{InstanceRange.proportional} to \c true for uniform scaling.
    The default value is empty, causing no scaling to be applied.

    \sa position, color, rotation, customData
*/

/*!
    \qmlproperty InstanceRange RandomInstancing::rotation

    The rotation property defines the rotation range for the generated instances. The type is
    \l vector3d, corresponding to a Euler rotation vector \c{[xRotation, yRotation, zRotation]}.
    The default value is empty, causing no rotation to be applied.

    \sa position, color, scale, customData
*/

/*!
    \qmlproperty InstanceRange RandomInstancing::color


    The color property defines the color variation range for the generated instances. The type is \l color.
    The default value is empty, causing the color to be white.

    Setting the colorModel property makes it possible to select only saturated colors, for example.

    \sa position, rotation, scale, customData
*/

/*!
    \qmlproperty InstanceRange RandomInstancing::customData

    The customData property defines the custom data variation range for the generated instances.
    The type is \l vector4d.
    The default value is empty, causing causing the generated data to be \c{[0, 0, 0, 0]}.

    \sa position, color, rotation, scale
*/
/*!
    \qmlproperty enumeration RandomInstancing::colorModel

    This property controls how the color range is interpreted.

    The instance colors are generated component by component within the range determined by the
    \e from and \e to colors. The color model determines how those components are defined.

    \value RandomInstancing.RGB
        The components are red, green, blue, and alpha, according to the RGB color model.
    \value RandomInstancing.HSV
        The components are hue, saturation, value, and alpha, according to the \l{QColor#The HSV Color Model}{HSV Color Model}.
    \value RandomInstancing.HSL
        The components are hue, saturation, lightness, and alpha,, according to the \l{QColor#The HSL Color Model}{HSL Color Model}.

    As an example, the following color range
    \qml
        color: InstanceRange {
            from: Qt.hsva(0, 0.1, 0.8, 1)
            to: Qt.hsva(1, 0.3, 1, 1)
        }
    \endqml
    will generate a full range of pastel colors when using the \c HSV color model, but only shades of pink
    when using the \c RGB color model.

    The default value is \c RandomInstancing.RGB

    \sa RandomInstancing::color
*/

QQuick3DRandomInstancing::QQuick3DRandomInstancing(QQuick3DObject *parent)
    : QQuick3DInstancing(parent)
{

}

QQuick3DRandomInstancing::~QQuick3DRandomInstancing()
{
}

void QQuick3DRandomInstancing::setInstanceCount(int instanceCount)
{
    if (instanceCount == m_randomCount)
        return;
    m_randomCount = instanceCount;
    emit instanceCountChanged();
    m_dirty = true;
    markDirty();
}

void QQuick3DRandomInstancing::setRandomSeed(int randomSeed)
{
    if (m_randomSeed == randomSeed)
        return;

    m_randomSeed = randomSeed;
    emit randomSeedChanged();
    m_dirty = true;
    markDirty();
}

void QQuick3DRandomInstancing::setPosition(QQuick3DInstanceRange *position)
{
    if (m_position == position)
        return;

    if (m_position)
        disconnect(m_position,  &QQuick3DInstanceRange::changed, this, &QQuick3DRandomInstancing::handleChange);
    m_position = position;
    emit positionChanged();
    m_dirty = true;
    markDirty();
    if (m_position) {
        connect(m_position, &QQuick3DInstanceRange::changed, this, &QQuick3DRandomInstancing::handleChange);
        connect(m_position, &QObject::destroyed, this, [this](QObject *obj){ if (obj == m_position) m_position = nullptr; });
    }
}

void QQuick3DRandomInstancing::setScale(QQuick3DInstanceRange *scale)
{
    if (m_scale == scale)
        return;

    if (m_scale)
        disconnect(m_scale,  &QQuick3DInstanceRange::changed, this, &QQuick3DRandomInstancing::handleChange);
    m_scale = scale;
    emit scaleChanged();
    m_dirty = true;
    markDirty();
    if (m_scale) {
        connect(m_scale, &QQuick3DInstanceRange::changed, this, &QQuick3DRandomInstancing::handleChange);
        connect(m_scale, &QObject::destroyed, this, [this](QObject *obj){ if (obj == m_scale) m_scale = nullptr; });
    }
}

void QQuick3DRandomInstancing::setRotation(QQuick3DInstanceRange *rotation)
{
    if (m_rotation == rotation)
        return;

    if (m_rotation)
        disconnect(m_rotation,  &QQuick3DInstanceRange::changed, this, &QQuick3DRandomInstancing::handleChange);
    m_rotation = rotation;
    emit rotationChanged();
    m_dirty = true;
    markDirty();
    if (m_rotation) {
        connect(m_rotation, &QQuick3DInstanceRange::changed, this, &QQuick3DRandomInstancing::handleChange);
        connect(m_rotation, &QObject::destroyed, this, [this](QObject *obj){ if (obj == m_rotation) m_rotation = nullptr; });
    }
}

void QQuick3DRandomInstancing::setColor(QQuick3DInstanceRange *color)
{
    if (m_color == color)
        return;

    if (m_color)
        disconnect(m_color,  &QQuick3DInstanceRange::changed, this, &QQuick3DRandomInstancing::handleChange);
    m_color = color;
    emit colorChanged();
    m_dirty = true;
    markDirty();
    if (m_color) {
        connect(m_color, &QQuick3DInstanceRange::changed, this, &QQuick3DRandomInstancing::handleChange);
        connect(m_color, &QObject::destroyed, this, [this](QObject *obj){ if (obj == m_color) m_color = nullptr; });
    }

}

void QQuick3DRandomInstancing::setCustomData(QQuick3DInstanceRange *customData)
{
    if (m_customData == customData)
        return;

    if (m_customData)
        disconnect(m_customData,  &QQuick3DInstanceRange::changed, this, &QQuick3DRandomInstancing::handleChange);
    m_customData = customData;
    emit customDataChanged();
    m_dirty = true;
    markDirty();
    if (m_customData) {
        connect(m_customData, &QQuick3DInstanceRange::changed, this, &QQuick3DRandomInstancing::handleChange);
        connect(m_customData, &QObject::destroyed, this, [this](QObject *obj){ if (obj == m_customData) m_customData = nullptr; });
    }
}

void QQuick3DRandomInstancing::setColorModel(QQuick3DRandomInstancing::ColorModel colorModel)
{
    if (m_colorModel == colorModel)
        return;
    m_colorModel = colorModel;
    emit colorModelChanged();
    m_dirty = true;
    markDirty();
}

void QQuick3DRandomInstancing::handleChange()
{
    m_dirty = true;
    markDirty();
}

static inline float genRandom(float from, float to, QRandomGenerator *rgen)
{
    float c = rgen->bounded(1.0);
    return from + c * (to - from);
}

static QVector3D genRandom(const QVector3D &from, const QVector3D &to, bool proportional, QRandomGenerator *rgen)
{
    if (proportional) {
        float c = rgen->bounded(1.0);
        return from + c * (to - from);
    }
    return { genRandom(from.x(), to.x(), rgen), genRandom(from.y(), to.y(), rgen), genRandom(from.z(), to.z(), rgen) };
}

static QVector4D genRandom(const QVector4D &from, const QVector4D &to, bool proportional, QRandomGenerator *rgen)
{
    if (proportional) {
        float c = rgen->bounded(1.0);
        return from + c * (to - from);
    }
    return { genRandom(from.x(), to.x(), rgen), genRandom(from.y(), to.y(), rgen), genRandom(from.z(), to.z(), rgen), genRandom(from.w(), to.w(), rgen) };
}

static QColor genRandom(const QColor &from, const QColor &to, bool proportional, QQuick3DRandomInstancing::ColorModel colorModel, QRandomGenerator *rgen)
{
    QVector4D v1, v2;
    switch (colorModel) {
    case QQuick3DRandomInstancing::ColorModel::HSL:
        from.getHslF(&v1[0], &v1[1], &v1[2], &v1[3]);
        to.getHslF(&v2[0], &v2[1], &v2[2], &v2[3]);
        break;
    case QQuick3DRandomInstancing::ColorModel::HSV:
        from.getHsvF(&v1[0], &v1[1], &v1[2], &v1[3]);
        to.getHsvF(&v2[0], &v2[1], &v2[2], &v2[3]);
        break;
    case QQuick3DRandomInstancing::ColorModel::RGB:
    default:
        from.getRgbF(&v1[0], &v1[1], &v1[2], &v1[3]);
        to.getRgbF(&v2[0], &v2[1], &v2[2], &v2[3]);
        break;
    }
    QVector4D r = genRandom(v1, v2, proportional, rgen);

    switch (colorModel) {
    case QQuick3DRandomInstancing::ColorModel::HSL:
        return QColor::fromHslF(r[0], r[1], r[2], r[3]);
        break;
    case QQuick3DRandomInstancing::ColorModel::HSV:
        return QColor::fromHsvF(r[0], r[1], r[2], r[3]);
        break;
    case QQuick3DRandomInstancing::ColorModel::RGB:
    default:
        return QColor::fromRgbF(r[0], r[1], r[2], r[3]);
    }
}

QByteArray QQuick3DRandomInstancing::getInstanceBuffer(int *instanceCount)
{
    if (m_dirty)
        generateInstanceTable();
    if (instanceCount)
        *instanceCount = m_randomCount;
    return m_instanceData;
}

namespace   {

struct GridPosition {
    int x, y, z;
};

inline bool operator==(const GridPosition &e1, const GridPosition &e2)
{
    return e1.x == e2.x && e1.y == e2.y && e1.z == e2.z;
}

inline size_t qHash(const GridPosition &key, size_t seed)
{
    return qHashMulti(seed, key.x, key.y, key.z);
}

class PositionGenerator {
public:
    void init(QVector3D fromPos, QVector3D toPos, bool proportional, bool gridMode, QVector3D gridSize) {
        m_from = fromPos;
        m_to = toPos;
        m_proportional = proportional;
        m_gridMode = gridMode;
        if (gridMode) {
            int nx, ny, nz;
            float cellWidth;
            float cellDepth;
            float cellHeight;
            float width  = toPos.x() - fromPos.x();
            float height = toPos.y() - fromPos.y();
            float depth  = toPos.z() - fromPos.z();
            if (qFuzzyIsNull(width)) {
                cellWidth = 0;
                nx = 1;
            } else {
                cellWidth = gridSize.x() > 0 ? gridSize.x() : width;
                nx = width / cellWidth;
            }
            if (qFuzzyIsNull(height)) {
                cellHeight = 0;
                ny = 1;
            } else {
                cellHeight = gridSize.y() > 0 ? gridSize.y() : height;
                ny = height / cellHeight;
            }
            if (qFuzzyIsNull(depth)) {
                cellDepth = 0;
                nz = 1;
            } else {
                cellDepth = gridSize.z() > 0 ? gridSize.z() : depth;
                nz = depth / cellDepth;
            }
            m_xGrid = nx > 1;
            m_yGrid = ny > 1;
            m_zGrid = nz > 1;
            m_gridSize = {cellWidth, cellHeight, cellDepth};
        }
        m_remainingAttempts = 1000000;
    }

    inline GridPosition gridPos(QVector3D pos) {
        int ix = m_xGrid ? int(pos.x() / m_gridSize.x()) : 0;
        int iy = m_yGrid ? int(pos.y() / m_gridSize.y()) : 0;
        int iz = m_zGrid ? int(pos.z() / m_gridSize.z()) : 0;
        return {ix, iy, iz};
    }

    inline bool collision(const GridPosition &gp) {
        for (int x = gp.x - m_xGrid; x <= gp.x + m_xGrid; ++x)
            for (int y = gp.y - m_yGrid; y <= gp.y + m_yGrid; ++y)
                for (int z = gp.z - m_zGrid; z <= gp.z + m_zGrid; ++z )
                    if (m_occupiedCells.contains({x,y,z}))
                        return true;
        return false;
    }

    QVector3D generate(QRandomGenerator *rgen) {
        if (m_gridMode) {
            while (m_remainingAttempts > 0) {
                auto pos = genRandom(m_from, m_to, m_proportional, rgen);
                auto gPos = gridPos(pos);
                if (!collision(gPos)) {
                    m_occupiedCells.insert(gPos);
                    return pos;
                }
                m_remainingAttempts--;
            }
            return {};
        }
        return genRandom(m_from, m_to, m_proportional, rgen);
    }

    bool isFull() const { return m_remainingAttempts <= 0; }

private:
    QVector3D m_from;
    QVector3D m_to;
    QVector3D m_gridSize;
    QSet<GridPosition> m_occupiedCells; // TODO: We could use a Bloom filter (no, not the graphics effect;) to save memory
    int m_remainingAttempts;
    bool m_proportional = false;
    bool m_gridMode = false;
    bool m_xGrid = false;
    bool m_yGrid = false;
    bool m_zGrid = false;
};
}


void QQuick3DRandomInstancing::generateInstanceTable()
{
    m_dirty = false;
    const int count = m_randomCount;

    QRandomGenerator rgen(m_randomSeed);
    if (m_randomSeed == -1)
        rgen.seed(QRandomGenerator::global()->generate());

    qsizetype tableSize = count * sizeof(InstanceTableEntry);
    m_instanceData.resize(tableSize);

    auto *array = reinterpret_cast<InstanceTableEntry*>(m_instanceData.data());

    PositionGenerator posGen;
    if (m_position)
        posGen.init(m_position->from().value<QVector3D>(), m_position->to().value<QVector3D>(), m_position->proportional(), m_gridMode, m_gridSpacing);

    for (int i = 0; i < count; ++i) {
        QVector3D pos;
        QVector3D scale{1, 1, 1};
        QVector3D eulerRotation;
        QColor color(Qt::white);
        QVector4D customData;

        if (m_position)
            pos = posGen.generate(&rgen);
        if (m_scale)
            scale = genRandom(m_scale->from().value<QVector3D>(), m_scale->to().value<QVector3D>(), m_scale->proportional(), &rgen);
        if (m_rotation) //TODO: quaternion rotation???
            eulerRotation = genRandom(m_rotation->from().value<QVector3D>(), m_rotation->to().value<QVector3D>(), m_rotation->proportional(), &rgen);
        if (m_color)
            color = genRandom(m_color->from().value<QColor>(), m_color->to().value<QColor>(), m_color->proportional(), m_colorModel, &rgen);
        if (m_customData)
            customData = genRandom(m_customData->from().value<QVector4D>(), m_customData->to().value<QVector4D>(), m_customData->proportional(), &rgen);

        if (Q_UNLIKELY(posGen.isFull())) {
            qWarning() << "RandomInstancing: Could not find free cell, truncating instance array" << i;
            qsizetype newSize = i * sizeof(InstanceTableEntry);
            m_instanceData.truncate(newSize);
            m_randomCount = i;
            emit instanceCountChanged();
            break;
        }

        array[i] = calculateTableEntry(pos, scale, eulerRotation, color, customData);
    }
}

QQuick3DInstanceRange::QQuick3DInstanceRange(QQuick3DObject *parent)
    : QQuick3DObject(parent)
{

}

void QQuick3DInstanceRange::setFrom(QVariant from)
{
    if (m_from == from)
        return;

    m_from = from;
    emit fromChanged();
    emit changed();
}

void QQuick3DInstanceRange::setTo(QVariant to)
{
    if (m_to == to)
        return;

    m_to = to;
    emit toChanged();
    emit changed();
}

void QQuick3DInstanceRange::setProportional(bool proportional)
{
    if (m_proportional == proportional)
        return;

    m_proportional = proportional;
    emit proportionalChanged();
    emit changed();
}

QVector3D QQuick3DRandomInstancing::gridSpacing() const
{
    return m_gridSpacing;
}

void QQuick3DRandomInstancing::setGridSpacing(const QVector3D &newGridSpacing)
{
    if (m_gridSpacing == newGridSpacing)
        return;
    m_gridSpacing = newGridSpacing;
    emit gridSpacingChanged();
    m_gridMode = (newGridSpacing.x() > 0 || newGridSpacing.y() > 0 || newGridSpacing.z() > 0) && !(newGridSpacing.x() < 0 || newGridSpacing.y() < 0 || newGridSpacing.z() < 0);
    m_dirty = true;
    markDirty();
}

QT_END_NAMESPACE
