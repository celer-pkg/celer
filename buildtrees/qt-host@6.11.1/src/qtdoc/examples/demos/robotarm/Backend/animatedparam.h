// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ANIMATEDPARAM_H
#define ANIMATEDPARAM_H

#include <QProperty>
#include <QVariantAnimation>

//! [class definition]
class AnimatedParam : public QVariantAnimation
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    //! [class definition]

public:
    AnimatedParam(QObject *parent = nullptr);

    int value() const;
    void setValue(int newValue);

    bool isRunning() const;

signals:
    void valueChanged();

private:
    QProperty<bool> m_isRunning;
};

#endif // ANIMATEDPARAM_H
