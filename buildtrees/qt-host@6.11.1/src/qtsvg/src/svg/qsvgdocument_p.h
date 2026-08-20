// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSVGDOCUMENT_P_H
#define QSVGDOCUMENT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qsvgstructure_p.h"
#include "qtsvgglobal.h"
#include "qtsvgglobal_p.h"

#include "QtCore/qrect.h"
#include "QtCore/qhash.h"
#include "QtCore/qxmlstream.h"
#include "QtCore/qscopedvaluerollback.h"
#include "QtCore/qsharedpointer.h"
#include "qsvgstyle_p.h"
#include "qsvgfont_p.h"
#include "private/qsvganimator_p.h"

QT_BEGIN_NAMESPACE

class QPainter;
class QByteArray;
class QSvgFont;
class QTransform;

class Q_SVG_EXPORT QSvgDocument : public QSvgStructureNode
{
public:
    static std::unique_ptr<QSvgDocument> load(const QString &file, QtSvg::Options options = {},
                                  QtSvg::AnimatorType type = QtSvg::AnimatorType::Automatic);
    static std::unique_ptr<QSvgDocument> load(const QByteArray &contents, QtSvg::Options options = {},
                                  QtSvg::AnimatorType type = QtSvg::AnimatorType::Automatic);
    static std::unique_ptr<QSvgDocument> load(QXmlStreamReader *contents, QtSvg::Options options = {},
                                  QtSvg::AnimatorType type = QtSvg::AnimatorType::Automatic);
    static bool isLikelySvg(QIODevice *device, bool *isCompressed = nullptr);
public:
    QSvgDocument(QtSvg::Options options, QtSvg::AnimatorType type);
    ~QSvgDocument();
    Type type() const override;

    inline QSize size() const;
    void setWidth(int len, bool percent);
    void setHeight(int len, bool percent);
    inline int width() const;
    inline int height() const;
    inline bool widthPercent() const;
    inline bool heightPercent() const;

    inline bool preserveAspectRatio() const;
    void setPreserveAspectRatio(bool on);

    inline QRectF viewBox() const;
    void setViewBox(const QRectF &rect);
    bool isCalculatingImplicitViewBox() { return m_calculatingImplicitViewBox; }

    QtSvg::Options options() const;

    void drawCommand(QPainter *, QSvgExtraStates &) override;

    void draw(QPainter *p);
    void draw(QPainter *p, const QRectF &bounds);
    void draw(QPainter *p, const QString &id,
              const QRectF &bounds=QRectF());

    QTransform transformForElement(const QString &id) const;
    QRectF boundsOnElement(const QString &id) const;
    bool   elementExists(const QString &id) const;

    void addSvgFont(QSvgFont *);
    QSvgFont *svgFont(const QString &family) const;
    void addNamedNode(const QString &id, QSvgNode *node);
    QSvgNode *namedNode(const QString &id) const;
    void addNamedStyle(const QString &id, QSvgPaintStyleProperty *style);
    QSvgPaintStyleProperty *namedStyle(const QString &id) const;

    void restartAnimation();
    inline qint64 currentElapsed() const;
    bool animated() const;
    void setAnimated(bool a);
    inline int animationDuration() const;
    int currentFrame() const;
    void setCurrentFrame(int);
    void setFramesPerSecond(int num);

    QSharedPointer<QSvgAbstractAnimator> animator() const;

private:
    void mapSourceToTarget(QPainter *p, const QRectF &targetRect, const QRectF &sourceRect = QRectF());
private:
    QSize  m_size;
    bool   m_widthPercent;
    bool   m_heightPercent;

    mutable bool m_calculatingImplicitViewBox = false;
    mutable bool m_implicitViewBox = true;
    mutable QRectF m_viewBox;
    bool m_preserveAspectRatio = false;

    QHash<QString, QSvgRefCounter<QSvgFont> > m_fonts;
    QHash<QString, QSvgNode *> m_namedNodes;
    QHash<QString, QSvgRefCounter<QSvgPaintStyleProperty> > m_namedStyles;

    bool  m_animated;
    int   m_fps;

    QSvgExtraStates m_states;

    const QtSvg::Options m_options;
    QSharedPointer<QSvgAbstractAnimator> m_animator;
};

Q_SVG_EXPORT QDebug operator<<(QDebug debug, const QSvgDocument &doc);

inline std::optional<int> calculateSizeValue(bool isPercent, int sizeValue, qreal viewBoxSizeValue)
{
    if (!isPercent)
        return sizeValue;

    const double valueAsDouble = 0.01 * sizeValue * viewBoxSizeValue;
    if (valueAsDouble < (std::numeric_limits<int>::min)() || valueAsDouble > (std::numeric_limits<int>::max)())
        return {};
    return qRound(valueAsDouble);
}

inline QSize QSvgDocument::size() const
{
    if (m_size.isEmpty())
        return viewBox().size().toSize();
    if (m_widthPercent || m_heightPercent) {
        const std::optional<int> width = calculateSizeValue(m_widthPercent, m_size.width(), viewBox().size().width());
        const std::optional<int> height = calculateSizeValue(m_heightPercent, m_size.height(), viewBox().size().height());
        if (!width || !height)
            return {};

        return QSize(*width, *height);
    }
    return m_size;
}

inline int QSvgDocument::width() const
{
    return size().width();
}

inline int QSvgDocument::height() const
{
    return size().height();
}

inline bool QSvgDocument::widthPercent() const
{
    return m_widthPercent;
}

inline bool QSvgDocument::heightPercent() const
{
    return m_heightPercent;
}

inline QRectF QSvgDocument::viewBox() const
{
    if (m_viewBox.isNull()) {
        QScopedValueRollback<bool> guard(m_calculatingImplicitViewBox, true);
        m_viewBox = bounds();
        m_implicitViewBox = true;
    }

    return m_viewBox;
}

inline bool QSvgDocument::preserveAspectRatio() const
{
    return m_preserveAspectRatio;
}

inline qint64 QSvgDocument::currentElapsed() const
{
    return m_animator->currentElapsed();
}

inline int QSvgDocument::animationDuration() const
{
    return m_animator->animationDuration();
}

QT_END_NAMESPACE

#endif // QSVGDOCUMENT_P_H
