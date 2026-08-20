// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qctextlayout_p.h"
#include <QTransform>

QT_BEGIN_NAMESPACE

QTextOption::WrapMode QCTextLayout::convertToQtWrapMode(QCanvasPainter::WrapMode mode)
{
    switch (mode) {
    case QCanvasPainter::WrapMode::NoWrap:
        return QTextOption::NoWrap;
        break;
    case QCanvasPainter::WrapMode::Wrap:
        return QTextOption::WrapAtWordBoundaryOrAnywhere;
        break;
    case QCanvasPainter::WrapMode::WordWrap:
        return QTextOption::WordWrap;
        break;
    case QCanvasPainter::WrapMode::WrapAnywhere:
        return QTextOption::WrapAnywhere;
        break;
    }
    return QTextOption::NoWrap;
}

Qt::Alignment QCTextLayout::convertToQtAlignment(QCanvasPainter::TextAlign alignment)
{
    switch (alignment) {
    case QCanvasPainter::TextAlign::Left:
        return Qt::AlignLeft;
        break;
    case QCanvasPainter::TextAlign::Center:
        return Qt::AlignCenter;
        break;
    case QCanvasPainter::TextAlign::Right:
        return Qt::AlignRight;
        break;
    default:
        break;
    }
    return Qt::AlignLeft;
}

float QCTextLayout::calculateVerticalAlignment(QCanvasPainter::TextBaseline baseline, const QRectF &rect,
                                               const QFontMetrics &metrics, const QRectF &layoutRect)
{
    float offset = 0;
    switch (baseline) {
    case QCanvasPainter::TextBaseline::Top:
        break;
    case QCanvasPainter::TextBaseline::Hanging:
        offset = -metrics.height() + metrics.ascent();
        break;
    case QCanvasPainter::TextBaseline::Middle:
        offset = rect.height() * 0.5 - layoutRect.height() * 0.5;
        break;
    case QCanvasPainter::TextBaseline::Alphabetic:
        offset = rect.height() - layoutRect.height() + metrics.descent();
        break;
    case QCanvasPainter::TextBaseline::Bottom:
        offset = rect.height() - layoutRect.height();
        break;
    }

    return offset;
}

Qt::LayoutDirection QCTextLayout::convertToQtDirection(QCanvasPainter::TextDirection direction)
{
    switch (direction) {
    case QCanvasPainter::TextDirection::LeftToRight:
        return Qt::LeftToRight;
    case QCanvasPainter::TextDirection::RightToLeft:
        return Qt::RightToLeft;
    case QCanvasPainter::TextDirection::Inherit:
        return QGuiApplication::layoutDirection();
    case QCanvasPainter::TextDirection::Auto:
        return Qt::LayoutDirectionAuto;
    }
    Q_UNREACHABLE_RETURN(Qt::LayoutDirectionAuto);
}

QT_END_NAMESPACE
