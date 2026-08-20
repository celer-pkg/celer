// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquickstyleitemsearchfield.h"
#include <QtQuickTemplates2/private/qquickindicatorbutton_p.h>

QT_BEGIN_NAMESPACE

QFont QQuickStyleItemSearchField::styleFont(QQuickItem *control) const
{
    return style()->font(QStyle::CE_PushButtonLabel, controlSize(control));
}

void QQuickStyleItemSearchField::connectToControl() const
{
    QQuickStyleItem::connectToControl();
    auto searchField = control<QQuickSearchField>();
    connect(searchField->searchIndicator(), &QQuickIndicatorButton::pressedChanged, this, &QQuickStyleItem::markImageDirty);
    connect(searchField->clearIndicator(), &QQuickIndicatorButton::pressedChanged, this, &QQuickStyleItem::markImageDirty);
    connect(searchField, &QQuickSearchField::textChanged, this, &QQuickStyleItem::markImageDirty);

}

void QQuickStyleItemSearchField::paintEvent(QPainter *painter) const
{
    QStyleOptionSearchField styleOption;
    initStyleOption(styleOption);
    style()->drawComplexControl(QStyle::CC_SearchField, &styleOption, painter);
}

StyleItemGeometry QQuickStyleItemSearchField::calculateGeometry() {
    QStyleOptionSearchField styleOption;
    initStyleOption(styleOption);
    StyleItemGeometry geometry;

    geometry.minimumSize = style()->sizeFromContents(QStyle::CT_SearchField, &styleOption, QSize(0, 0));

    if (styleOption.subControls == QStyle::SC_SearchFieldFrame) {
        geometry.implicitSize = style()->sizeFromContents(QStyle::CT_SearchField, &styleOption, contentSize());
        styleOption.rect = QRect(QPoint(0, 0), geometry.implicitSize);
        geometry.contentRect = style()->subControlRect(QStyle::CC_SearchField, &styleOption, QStyle::SC_SearchFieldEditField);
        geometry.layoutRect = style()->subElementRect(QStyle::SE_SearchFieldLayoutItem, &styleOption);
        geometry.ninePatchMargins = style()->ninePatchMargins(QStyle::CC_SearchField, &styleOption, geometry.minimumSize);
        geometry.focusFrameRadius = style()->pixelMetric(QStyle::PM_SearchFieldFocusFrameRadius, &styleOption);
    } else {
        geometry.implicitSize = geometry.minimumSize;
    }

    return geometry;
}

void QQuickStyleItemSearchField::initStyleOption(QStyleOptionSearchField &styleOption) const {
    initStyleOptionBase(styleOption);
    auto searchField = control<QQuickSearchField>();

    styleOption.text = searchField->text();

    switch (m_subControl) {
    case Frame:
        styleOption.subControls = QStyle::SC_SearchFieldFrame;
        styleOption.frame = true;
        break;
    case Search:
        styleOption.subControls = QStyle::SC_SearchFieldSearch;
        break;
    case Clear:
        styleOption.subControls = QStyle::SC_SearchFieldClear;
        break;
    }

    if (searchField->searchIndicator()->isPressed()) {
        styleOption.activeSubControls = QStyle::SC_SearchFieldSearch;
        styleOption.state |= QStyle::State_Sunken;
    } else if (searchField->clearIndicator()->isPressed()) {
        styleOption.activeSubControls = QStyle::SC_SearchFieldClear;
        styleOption.state |= QStyle::State_Sunken;
    }
}

QT_END_NAMESPACE

#include "moc_qquickstyleitemsearchfield.cpp"
