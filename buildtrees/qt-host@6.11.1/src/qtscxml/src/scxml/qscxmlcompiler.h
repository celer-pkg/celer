// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSCXMLCOMPILER_H
#define QSCXMLCOMPILER_H

#include <QtScxml/qscxmlerror.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE
class QXmlStreamReader;
class QScxmlStateMachine;

class QScxmlCompilerPrivate;
class Q_SCXML_EXPORT QScxmlCompiler
{
public:
    class Q_SCXML_EXPORT Loader
    {
    public:
        Loader();
        virtual ~Loader();
        virtual QByteArray load(const QString &name,
                                const QString &baseDir,
                                QStringList *errors) = 0;
    };

public:
    QScxmlCompiler(QXmlStreamReader *xmlReader);
    ~QScxmlCompiler();

    QString fileName() const;
    void setFileName(const QString &fileName);

    Loader *loader() const;
    void setLoader(Loader *newLoader);

    QScxmlStateMachine *compile();
    QList<QScxmlError> errors() const;

private:
    friend class QScxmlCompilerPrivate;
    QScxmlCompilerPrivate *d;
};

QT_END_NAMESPACE

#endif // QSCXMLCOMPILER_H
