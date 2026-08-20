// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "ambientproperties.h"

#include <QtWidgets/QColorDialog>
#include <QtWidgets/QFontDialog>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>

QT_BEGIN_NAMESPACE

AmbientProperties::AmbientProperties(QWidget *parent)
: QDialog(parent), container(nullptr)
{
    setupUi(this);

    connect(buttonClose, &QAbstractButton::clicked, this, &QWidget::close);
}

void AmbientProperties::setControl(QWidget *widget)
{
    container = widget;

    QColor c = container->palette().color(container->backgroundRole());
    QPalette p = backSample->palette(); p.setColor(backSample->backgroundRole(), c); backSample->setPalette(p);

    c = container->palette().color(container->foregroundRole());
    p = foreSample->palette(); p.setColor(foreSample->backgroundRole(), c); foreSample->setPalette(p);

    fontSample->setFont( container->font() );
    buttonEnabled->setChecked( container->isEnabled() );
    enabledSample->setEnabled( container->isEnabled() );
}

void AmbientProperties::on_buttonBackground_clicked()
{
    const QColor c = QColorDialog::getColor(backSample->palette().color(backSample->backgroundRole()), this);
    QPalette p = backSample->palette();
    p.setColor(backSample->backgroundRole(), c);
    backSample->setPalette(p);

    p = container->palette();
    p.setColor(container->backgroundRole(), c);
    container->setPalette(p);

    const QWidgetList widgets = mdiAreaWidgets();
    for (QWidget *widget : widgets) {
        p = widget->palette();
        p.setColor(widget->backgroundRole(), c);
        widget->setPalette(p);
    }
}

void AmbientProperties::on_buttonForeground_clicked()
{
    const QColor c = QColorDialog::getColor(foreSample->palette().color(foreSample->backgroundRole()), this);

    QPalette p = foreSample->palette();
    p.setColor(foreSample->backgroundRole(), c);
    foreSample->setPalette(p);

    p = container->palette();
    p.setColor(container->foregroundRole(), c);
    container->setPalette(p);

    const QWidgetList widgets = mdiAreaWidgets();
    for (QWidget *widget : widgets) {
        p = widget->palette();
        p.setColor(widget->foregroundRole(), c);
        widget->setPalette(p);
    }
}

void AmbientProperties::on_buttonFont_clicked()
{
    bool ok;
    QFont f = QFontDialog::getFont( &ok, fontSample->font(), this );
    if ( !ok )
        return;
    fontSample->setFont( f );
    container->setFont( f );

    const QWidgetList widgets = mdiAreaWidgets();
    for (QWidget *widget : widgets)
        widget->setFont( f );
}

void AmbientProperties::on_buttonEnabled_toggled(bool on)
{
    enabledSample->setEnabled( on );
    container->setEnabled( on );
}

QWidgetList AmbientProperties::mdiAreaWidgets() const
{
    QWidgetList result;

    if (QMdiArea *mdiArea = qobject_cast<QMdiArea*>(container)) {
        const auto mdiSubWindows = mdiArea->subWindowList();
        for (const QMdiSubWindow *subWindow : mdiSubWindows)
            result.push_back(subWindow->widget());
    }
    return result;
}

QT_END_NAMESPACE
