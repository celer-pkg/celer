// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>
#include <QPixmapStyle>


using namespace Qt::StringLiterals;

class QCustomPixmapStyle : public QPixmapStyle
{
    Q_OBJECT
public:
    QCustomPixmapStyle();
    ~QCustomPixmapStyle() override;
};

//! [0]
QCustomPixmapStyle::QCustomPixmapStyle() :
    QPixmapStyle()
{
    //! [1]
    addDescriptor(PB_Enabled,
                  "://button/core_button_inactive.png"_L1,
                  QMargins(13, 13, 13, 13),
                  QTileRules(Qt::RepeatTile, Qt::StretchTile));
    //! [1]
    addDescriptor(PB_Checked,
                  "://button/core_button_enabled_selected.png"_L1,
                  QMargins(13, 13, 13, 13),
                  QTileRules(Qt::RepeatTile, Qt::StretchTile));
    addDescriptor(PB_Pressed,
                  "://button/core_button_pressed.png"_L1,
                  QMargins(13, 13, 13, 13),
                  QTileRules(Qt::RepeatTile, Qt::StretchTile));
    addDescriptor(PB_Disabled,
                  "://button/core_button_disabled.png"_L1,
                  QMargins(13, 13, 13, 13),
                  QTileRules(Qt::RepeatTile, Qt::StretchTile));
    addDescriptor(PB_PressedDisabled,
                  "://button/core_button_disabled_selected.png"_L1,
                  QMargins(13, 13, 13, 13),
                  QTileRules(Qt::RepeatTile, Qt::StretchTile));

    //! [2]
    addDescriptor(LE_Enabled,
                  "://lineedit/core_textinput_bg.png"_L1,
                  QMargins(8, 8, 8, 8));
    addDescriptor(LE_Disabled,
                  "://lineedit/core_textinput_bg_disabled.png"_L1,
                  QMargins(8, 8, 8, 8));
    addDescriptor(LE_Focused,
                  "://lineedit/core_textinput_bg_highlight.png"_L1,
                  QMargins(8, 8, 8, 8));

    copyDescriptor(LE_Enabled, TE_Enabled);
    copyDescriptor(LE_Disabled, TE_Disabled);
    copyDescriptor(LE_Focused, TE_Focused);
    //! [2]
}
//! [0]

QCustomPixmapStyle::~QCustomPixmapStyle()
{
}
