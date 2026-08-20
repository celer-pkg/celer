// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qscxmlerror.h"

QT_BEGIN_NAMESPACE

class QScxmlError::ScxmlErrorPrivate
{
public:
    ScxmlErrorPrivate()
        : line(-1)
        , column(-1)
    {}

    QString fileName;
    int line;
    int column;
    QString description;
};

/*!
 * \class QScxmlError
 * \brief The QScxmlError class describes the errors returned by the Qt SCXML
 * state machine when parsing an SCXML file.
 * \since 5.7
 * \inmodule QtScxml
 *
 * \sa QScxmlStateMachine QScxmlCompiler
 */

/*!
    \property QScxmlError::column
    \brief The column number in which the SCXML error occurred.
*/

/*!
    \property QScxmlError::description
    \brief A description of the SCXML error.
*/

/*!
    \property QScxmlError::fileName
    \brief The name of the file in which the SCXML error occurred.
*/

/*!
    \property QScxmlError::line
    \brief The line number on which the SCXML error occurred.
*/

/*!
    \property QScxmlError::valid
    \brief Whether the SCXML error is valid.
*/

/*!
 * Creates a new invalid SCXML error.
 */
QScxmlError::QScxmlError()
    : d(nullptr)
{}

/*!
 * Creates a new valid SCXML error that contains the error message,
 * \a description, as well as the \a fileName, \a line, and \a column where the
 * error occurred.
 */
QScxmlError::QScxmlError(const QString &fileName, int line, int column, const QString &description)
    : d(new ScxmlErrorPrivate)
{
    d->fileName = fileName;
    d->line = line;
    d->column = column;
    d->description = description;
}

/*!
 * Constructs a copy of \a other.
 */
QScxmlError::QScxmlError(const QScxmlError &other)
    : d(nullptr)
{
    *this = other;
}

/*!
 * Assigns \a other to this SCXML error and returns a reference to this SCXML
 * error.
 */
QScxmlError &QScxmlError::operator=(const QScxmlError &other)
{
    if (other.d) {
        if (!d)
            d = new ScxmlErrorPrivate;
        d->fileName = other.d->fileName;
        d->line = other.d->line;
        d->column = other.d->column;
        d->description = other.d->description;
    } else {
        delete d;
        d = nullptr;
    }
    return *this;
}

/*!
 * Destroys the SCXML error.
 */
QScxmlError::~QScxmlError()
{
    delete d;
    d = nullptr;
}

/*!
 * Returns \c true if the error is valid, \c false otherwise. An invalid error
 * can only be created by calling the default constructor or by assigning an
 * invalid error.
 */
bool QScxmlError::isValid() const
{
    return d != nullptr;
}

/*!
 * Returns the name of the file in which the error occurred.
 */
QString QScxmlError::fileName() const
{
    return isValid() ? d->fileName : QString();
}

/*!
 * Returns the line on which the error occurred.
 */
int QScxmlError::line() const
{
    return isValid() ? d->line : -1;
}

/*!
 * Returns the column in which the error occurred.
 */
int QScxmlError::column() const
{
    return isValid() ? d->column : -1;
}

/*!
 * Returns the error message.
 */
QString QScxmlError::description() const
{
    return isValid() ? d->description : QString();
}

/*!
 * This convenience method converts an error to a string.
 * Returns the error message formatted as:
 * \e {"filename:line:column: error: description"}
 */
QString QScxmlError::toString() const
{
    QString str;
    if (!isValid())
        return str;

    if (d->fileName.isEmpty())
        str = QStringLiteral("<Unknown File>");
    else
        str = d->fileName;
    if (d->line != -1) {
        str += QStringLiteral(":%1").arg(d->line);
        if (d->column != -1)
            str += QStringLiteral(":%1").arg(d->column);
    }
    str += QStringLiteral(": error: ") + d->description;

    return str;
}

QT_END_NAMESPACE
