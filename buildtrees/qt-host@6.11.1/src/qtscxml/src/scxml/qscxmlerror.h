// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSCXMLERROR_H
#define QSCXMLERROR_H

#include <QtScxml/qscxmlglobals.h>
#include <QtCore/qobjectdefs.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class Q_SCXML_EXPORT QScxmlError
{
#ifndef BUILD_QSCXMLC
    Q_GADGET
    Q_PROPERTY(bool valid READ isValid CONSTANT)
    Q_PROPERTY(QString fileName READ fileName CONSTANT)
    Q_PROPERTY(int line READ line CONSTANT)
    Q_PROPERTY(int column READ column CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
#endif // BUILD_QSCXMLC

public:
    QScxmlError();
    QScxmlError(const QString &fileName, int line, int column, const QString &description);
    QScxmlError(const QScxmlError &);
    QScxmlError &operator=(const QScxmlError &);
    ~QScxmlError();

    bool isValid() const;

    QString fileName() const;
    int line() const;
    int column() const;
    QString description() const;

    QString toString() const;

private:
    class ScxmlErrorPrivate;
    ScxmlErrorPrivate *d;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QScxmlError)

#endif // QSCXMLERROR_H
