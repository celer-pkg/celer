// Copyright (C) 2016 - 2012 Research In Motion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <qndefnfcsmartposterrecord.h>
#include "qndefnfcsmartposterrecord_p.h"
#include <qndefmessage.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

/*!
    \class QNdefNfcIconRecord
    \brief The QNdefNfcIconRecord class provides an NFC MIME record to hold an
    icon.

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \since Qt 5.2

    This class wraps the image data into an NDEF message.
    It provides an NDEF record of type \l QNdefRecord::Mime.
    The \l {QNdefRecord::}{payload}() contains the raw image data.
*/

/*!
    \fn QNdefNfcIconRecord::QNdefNfcIconRecord()

    Constructs an empty NDEF record of type \l QNdefRecord::Mime.
*/

/*!
    \fn QNdefNfcIconRecord::QNdefNfcIconRecord(const QNdefRecord &other)

    Constructs an NDEF icon record that is a copy of \a other.
*/

/*!
    \class QNdefNfcSmartPosterRecord
    \brief The QNdefNfcSmartPosterRecord class provides an NFC RTD-SmartPoster.

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \since Qt 5.2

    RTD-SmartPoster encapsulates a Smart Poster.
 */

/*!
    \enum QNdefNfcSmartPosterRecord::Action

    This enum describes the course of action that a device should take with the content.

    \value UnspecifiedAction    The action is not defined.
    \value DoAction             Do the action (send the SMS, launch the browser, make the telephone call).
    \value SaveAction           Save for later (store the SMS in INBOX, put the URI in a bookmark, save the telephone number in contacts).
    \value EditAction           Open for editing (open an SMS in the SMS editor, open the URI in a URI editor, open the telephone number for editing).
 */

QNdefNfcSmartPosterRecordPrivate::~QNdefNfcSmartPosterRecordPrivate()
{
    cleanup();
}

void QNdefNfcSmartPosterRecordPrivate::cleanup()
{
    // Clean-up existing internal structure
    m_titleList.clear();
    delete m_uri;
    delete m_action;
    m_iconList.clear();
    delete m_size;
    delete m_type;
}

/*!
    Constructs a new empty smart poster.
*/
QNdefNfcSmartPosterRecord::QNdefNfcSmartPosterRecord()
    : QNdefRecord(QNdefRecord::NfcRtd, "Sp"),
      d(new QNdefNfcSmartPosterRecordPrivate)
{
}

/*!
    Constructs a new smart poster that is a copy of \a other.
*/
QNdefNfcSmartPosterRecord::QNdefNfcSmartPosterRecord(const QNdefRecord &other)
    : QNdefRecord(other, QNdefRecord::NfcRtd, "Sp"),
      d(new QNdefNfcSmartPosterRecordPrivate)
{
    // Need to set payload again to create internal structure
    setPayload(other.payload());
}

/*!
    Constructs a new smart poster that is a copy of \a other.
*/
QNdefNfcSmartPosterRecord::QNdefNfcSmartPosterRecord(const QNdefNfcSmartPosterRecord &other)
    : QNdefRecord(other, QNdefRecord::NfcRtd, "Sp"), d(other.d)
{
}

/*!
    Assigns the \a other smart poster record to this record and returns a reference to
    this record.
*/
QNdefNfcSmartPosterRecord &QNdefNfcSmartPosterRecord::operator=(const QNdefNfcSmartPosterRecord &other)
{
    if (this != &other)
        d = other.d;

    return *this;
}

/*!
    Destroys the smart poster.
*/
QNdefNfcSmartPosterRecord::~QNdefNfcSmartPosterRecord()
{
}

void QNdefNfcSmartPosterRecord::cleanup()
{
    if (d)
        d->cleanup();
}

/*!
    \internal
    Sets the payload of the NDEF record to \a payload
*/
void QNdefNfcSmartPosterRecord::setPayload(const QByteArray &payload)
{
    QNdefRecord::setPayload(payload);

    cleanup();

    if (!payload.isEmpty()) {
        // Create new structure
        const QNdefMessage message = QNdefMessage::fromByteArray(payload);

        // Iterate through all the records contained in the payload's message.
        for (const QNdefRecord& record : message) {
            // Title
            if (record.isRecordType<QNdefNfcTextRecord>()) {
                addTitleInternal(record);
            }

            // URI
            else if (record.isRecordType<QNdefNfcUriRecord>()) {
                d->m_uri = new QNdefNfcUriRecord(record);
            }

            // Action
            else if (record.isRecordType<QNdefNfcActRecord>()) {
                d->m_action = new QNdefNfcActRecord(record);
            }

            // Icon
            else if (record.isRecordType<QNdefNfcIconRecord>()) {
                addIconInternal(record);
            }

            // Size
            else if (record.isRecordType<QNdefNfcSizeRecord>()) {
                d->m_size = new QNdefNfcSizeRecord(record);
            }

            // Type
            else if (record.isRecordType<QNdefNfcTypeRecord>()) {
                d->m_type = new QNdefNfcTypeRecord(record);
            }
        }
    }
}

void QNdefNfcSmartPosterRecord::convertToPayload()
{
    QNdefMessage message;

    // Title
    for (qsizetype t = 0; t < titleCount(); t++)
        message.append(titleRecord(t));

    // URI
    if (d->m_uri)
        message.append(*(d->m_uri));

    // Action
    if (d->m_action)
        message.append(*(d->m_action));

    // Icon
    for (qsizetype i = 0; i < iconCount(); i++)
        message.append(iconRecord(i));

    // Size
    if (d->m_size)
        message.append(*(d->m_size));

    // Type
    if (d->m_type)
        message.append(*(d->m_type));

    QNdefRecord::setPayload(message.toByteArray());
}

/*!
    Returns \c true if the smart poster contains a title record using the locale
    \a locale. If \a locale is empty, then \c true is returned if the smart
    poster contains at least one title record. In all other cases, \c false is
    returned.
 */
bool QNdefNfcSmartPosterRecord::hasTitle(const QString &locale) const
{
    for (qsizetype i = 0; i < d->m_titleList.size(); ++i) {
        const QNdefNfcTextRecord &text = d->m_titleList[i];

        if (locale.isEmpty() || text.locale() == locale)
            return true;
    }

    return false;
}

/*!
    Returns \c true if the smart poster contains an action record, otherwise
    returns \c false.
 */
bool QNdefNfcSmartPosterRecord::hasAction() const
{
    return d->m_action != nullptr;
}

/*!
    Returns \c true if the smart poster contains an icon record using the type
    \a mimetype. If \a mimetype is empty, then \c true is returned if the smart
    poster contains at least one icon record.
    In all other cases, \c false is returned.
 */
bool QNdefNfcSmartPosterRecord::hasIcon(const QByteArray &mimetype) const
{
    for (qsizetype i = 0; i < d->m_iconList.size(); ++i) {
        const QNdefNfcIconRecord &icon = d->m_iconList[i];

        if (mimetype.isEmpty() || icon.type() == mimetype)
            return true;
    }

    return false;
}

/*!
    Returns \c true if the smart poster contains a size record, otherwise
    returns \c false.
 */
bool QNdefNfcSmartPosterRecord::hasSize() const
{
    return d->m_size != nullptr;
}

/*!
    Returns \c true if the smart poster contains a type record, otherwise
    returns \c false.
 */
bool QNdefNfcSmartPosterRecord::hasTypeInfo() const
{
    return d->m_type != nullptr;
}

/*!
    Returns the number of title records contained inside the smart poster.
 */
qsizetype QNdefNfcSmartPosterRecord::titleCount() const
{
    return d->m_titleList.size();
}

/*!
    Returns the title record corresponding to the index \a index inside the
    smart poster, where \a index is a value between 0 and titleCount() - 1.
    Values outside of this range return an empty record.
 */
QNdefNfcTextRecord QNdefNfcSmartPosterRecord::titleRecord(qsizetype index) const
{
    if (index >= 0 && index < d->m_titleList.size())
        return d->m_titleList[index];

    return QNdefNfcTextRecord();
}

/*!
    Returns the title record text associated with locale \a locale if available. If \a locale
    is empty then the title text of the first available record is returned. In all other
    cases an empty string is returned.
 */
QString QNdefNfcSmartPosterRecord::title(const QString &locale) const
{
    for (qsizetype i = 0; i < d->m_titleList.size(); ++i) {
        const QNdefNfcTextRecord &text = d->m_titleList[i];

        if (locale.isEmpty() || text.locale() == locale)
            return text.text();
    }

    return QString();
}

/*!
    Returns a copy of all title records inside the smart poster.
 */
QList<QNdefNfcTextRecord> QNdefNfcSmartPosterRecord::titleRecords() const
{
    return d->m_titleList;
}

/*!
    Attempts to add a title record \a text to the smart poster. If the smart poster does not already
    contain a title record with the same locale as title record \a text, then the title record is added
    and the function returns \c true. Otherwise \c false is returned.
 */
bool QNdefNfcSmartPosterRecord::addTitle(const QNdefNfcTextRecord &text)
{
    const bool status = addTitleInternal(text);

    // Convert to payload if the title is added
    if (status)
        convertToPayload();

    return status;
}

bool QNdefNfcSmartPosterRecord::addTitleInternal(const QNdefNfcTextRecord &text)
{
    for (qsizetype i = 0; i < d->m_titleList.size(); ++i) {
        const QNdefNfcTextRecord &rec = d->m_titleList[i];

        if (rec.locale() == text.locale())
            return false;
    }

    d->m_titleList.append(text);
    return true;
}

/*!
    Attempts to add a new title record with title \a text, locale \a locale and encoding \a encoding.
    If the smart poster does not already contain a title record with locale \a locale, then the title record
    is added and the function returns \c true. Otherwise \c false is returned.
 */
bool QNdefNfcSmartPosterRecord::addTitle(const QString &text, const QString &locale, QNdefNfcTextRecord::Encoding encoding)
{
    QNdefNfcTextRecord rec;
    rec.setText(text);
    rec.setLocale(locale);
    rec.setEncoding(encoding);

    return addTitle(rec);
}

/*!
    Attempts to remove the title record \a text from the smart poster. Removes
    the record and returns \c true if the smart poster contains a matching
    record, otherwise \c false is returned.
 */
bool QNdefNfcSmartPosterRecord::removeTitle(const QNdefNfcTextRecord &text)
{
    bool status = false;

    for (qsizetype i = 0; i < d->m_titleList.size(); ++i) {
        const QNdefNfcTextRecord &rec = d->m_titleList[i];

        if (rec.text() == text.text() && rec.locale() == text.locale() && rec.encoding() == text.encoding()) {
            d->m_titleList.removeAt(i);
            status = true;
            break;
        }
    }

    // Convert to payload if the title list has changed
    if (status)
        convertToPayload();

    return status;
}

/*!
    Attempts to remove a title record with the locale \a locale from the smart
    poster. Removes the record and returns \c true if the smart poster contains
    a matching record, otherwise \c false is returned.
 */
bool QNdefNfcSmartPosterRecord::removeTitle(const QString &locale)
{
    bool status = false;

    for (qsizetype i = 0; i < d->m_titleList.size(); ++i) {
        const QNdefNfcTextRecord &rec = d->m_titleList[i];

        if (rec.locale() == locale) {
            d->m_titleList.removeAt(i);
            status = true;
            break;
        }
    }

    // Convert to payload if the title list has changed
    if (status)
        convertToPayload();

    return status;
}

/*!
    Adds the title record list \a titles to the smart poster. Any existing records are overwritten.
 */
void QNdefNfcSmartPosterRecord::setTitles(const QList<QNdefNfcTextRecord> &titles)
{
    d->m_titleList = titles;

    // Convert to payload
    convertToPayload();
}

/*!
    Returns the URI from the smart poster's URI record if set. Otherwise an empty URI is returned.
 */
QUrl QNdefNfcSmartPosterRecord::uri() const
{
    if (d->m_uri)
        return d->m_uri->uri();

    return QUrl();
}

/*!
    Returns the smart poster's URI record if set. Otherwise an empty URI is returned.
 */
QNdefNfcUriRecord QNdefNfcSmartPosterRecord::uriRecord() const
{
    if (d->m_uri)
        return *(d->m_uri);

    return QNdefNfcUriRecord();
}

/*!
    Sets the URI record to \a url
 */
void QNdefNfcSmartPosterRecord::setUri(const QNdefNfcUriRecord &url)
{
    if (d->m_uri)
        delete d->m_uri;

    d->m_uri = new QNdefNfcUriRecord(url);

    // Convert to payload
    convertToPayload();
}

/*!
    Constructs a URI record and sets its content inside the smart poster to \a url
 */
void QNdefNfcSmartPosterRecord::setUri(const QUrl &url)
{
    QNdefNfcUriRecord rec;
    rec.setUri(url);

    setUri(rec);
}

/*!
    Returns the action from the action record if available. Otherwise \l UnspecifiedAction is returned.
 */
QNdefNfcSmartPosterRecord::Action QNdefNfcSmartPosterRecord::action() const
{
    if (d->m_action)
        return d->m_action->action();

    return UnspecifiedAction;
}

/*!
    Sets the action record to \a act
 */
void QNdefNfcSmartPosterRecord::setAction(Action act)
{
    if (!d->m_action)
        d->m_action = new QNdefNfcActRecord();

    d->m_action->setAction(act);

    // Convert to payload
    convertToPayload();
}

/*!
    Returns the number of icon records contained inside the smart poster.
 */
qsizetype QNdefNfcSmartPosterRecord::iconCount() const
{
    return d->m_iconList.size();
}

/*!
    Returns the icon record corresponding to the index \a index inside the smart
    poster, where \a index is a value between 0 and \l iconCount() - 1.
    Values outside of this range return an empty record.
 */
QNdefNfcIconRecord QNdefNfcSmartPosterRecord::iconRecord(qsizetype index) const
{
    if (index >= 0 && index < d->m_iconList.size())
        return d->m_iconList[index];

    return QNdefNfcIconRecord();
}

/*!
    Returns the associated icon record data if the smart poster contains an icon record with MIME type \a mimetype.
    If \a mimetype is omitted or empty then the first icon's record data is returned. In all other cases, an empty array is returned.
 */
QByteArray QNdefNfcSmartPosterRecord::icon(const QByteArray& mimetype) const
{
    for (qsizetype i = 0; i < d->m_iconList.size(); ++i) {
        const QNdefNfcIconRecord &icon = d->m_iconList[i];

        if (mimetype.isEmpty() || icon.type() == mimetype)
            return icon.data();
    }

    return QByteArray();
}

/*!
    Returns a copy of all icon records inside the smart poster.
 */
QList<QNdefNfcIconRecord> QNdefNfcSmartPosterRecord::iconRecords() const
{
    return d->m_iconList;
}

/*!
    Adds an icon record \a icon to the smart poster. If the smart poster already contains an icon
    record with the same type then the existing icon record is replaced.
 */
void QNdefNfcSmartPosterRecord::addIcon(const QNdefNfcIconRecord &icon)
{
    addIconInternal(icon);

    // Convert to payload
    convertToPayload();
}

void QNdefNfcSmartPosterRecord::addIconInternal(const QNdefNfcIconRecord &icon)
{
    for (qsizetype i = 0; i < d->m_iconList.size(); ++i) {
        const QNdefNfcIconRecord &rec = d->m_iconList[i];

        if (rec.type() == icon.type())
            d->m_iconList.removeAt(i);
    }

    d->m_iconList.append(icon);
}

/*!
    Adds an icon record with type \a type and data \a data to the smart poster. If the smart poster
    already contains an icon record with the same type then the existing icon record is replaced.
 */
void QNdefNfcSmartPosterRecord::addIcon(const QByteArray &type, const QByteArray &data)
{
    QNdefNfcIconRecord rec;
    rec.setType(type);
    rec.setData(data);

    addIcon(rec);
}

/*!
    Attempts to remove the icon record \a icon from the smart poster.
    Removes the record and returns \c true if the smart poster contains
    a matching record, otherwise \c false is returned.
 */
bool QNdefNfcSmartPosterRecord::removeIcon(const QNdefNfcIconRecord &icon)
{
    bool status = false;

    for (qsizetype i = 0; i < d->m_iconList.size(); ++i) {
        const QNdefNfcIconRecord &rec = d->m_iconList[i];

        if (rec.type() == icon.type() && rec.data() == icon.data()) {
            d->m_iconList.removeAt(i);
            status = true;
            break;
        }
    }

    // Convert to payload if the icon list has changed
    if (status)
        convertToPayload();

    return status;
}

/*!
    Attempts to remove the icon record with the type \a type from the smart
    poster. Removes the record and returns \c true if the smart poster contains
    a matching record, otherwise \c false is returned.
 */
bool QNdefNfcSmartPosterRecord::removeIcon(const QByteArray &type)
{
    bool status = false;

    for (qsizetype i = 0; i < d->m_iconList.size(); ++i) {
        const QNdefNfcIconRecord &rec = d->m_iconList[i];

        if (rec.type() == type) {
            d->m_iconList.removeAt(i);
            status = true;
            break;
        }
    }

    // Convert to payload if the icon list has changed
    if (status)
        convertToPayload();

    return status;
}

/*!
    Adds the icon record list \a icons to the smart poster.
    Any existing records are overwritten.

    \sa hasIcon(), icon()
 */
void QNdefNfcSmartPosterRecord::setIcons(const QList<QNdefNfcIconRecord> &icons)
{
    d->m_iconList = icons;

    // Convert to payload
    convertToPayload();
}

/*!
    Returns the size from the size record if available; otherwise returns 0.

    The value is optional and contains the size in bytes of the object
    that the URI refers to. It may be used by the device to determine
    whether it can accommodate the object.

    \sa setSize()
 */
quint32 QNdefNfcSmartPosterRecord::size() const
{
    if (d->m_size)
        return d->m_size->size();

    return 0;
}

/*!
    Sets the record \a size. The value contains the size in bytes of
    the object that the URI refers to.

    \sa size(), hasSize()
 */
void QNdefNfcSmartPosterRecord::setSize(quint32 size)
{
    if (!d->m_size)
        d->m_size = new QNdefNfcSizeRecord();

    d->m_size->setSize(size);

    // Convert to payload
    convertToPayload();
}

/*!
    Returns the MIME type that describes the type of the objects that can be
    reached via uri().

    If the type is not known, the returned QString is empty.

    \sa setTypeInfo(), hasTypeInfo()
 */
QString QNdefNfcSmartPosterRecord::typeInfo() const
{
    if (d->m_type)
        return d->m_type->typeInfo();

    return QString();
}

/*!
    Sets the type record to \a type. \a type describes the type of the object
    referenced by uri().

    \sa typeInfo()
 */
void QNdefNfcSmartPosterRecord::setTypeInfo(const QString &type)
{
    if (d->m_type)
        delete d->m_type;

    d->m_type = new QNdefNfcTypeRecord();
    d->m_type->setTypeInfo(type);

    // Convert to payload
    convertToPayload();
}

void QNdefNfcActRecord::setAction(QNdefNfcSmartPosterRecord::Action action)
{
    QByteArray data(1, action);

    setPayload(data);
}

QNdefNfcSmartPosterRecord::Action QNdefNfcActRecord::action() const
{
    const QByteArray p = payload();
    QNdefNfcSmartPosterRecord::Action value =
            QNdefNfcSmartPosterRecord::UnspecifiedAction;

    if (!p.isEmpty())
        value = QNdefNfcSmartPosterRecord::Action(static_cast<signed char>(p[0]));

    return value;
}

/*!
    Sets the contents of the icon record to \a data.
*/
void QNdefNfcIconRecord::setData(const QByteArray &data)
{
    setPayload(data);
}

/*!
    Returns the icon data as \l QByteArray.
*/
QByteArray QNdefNfcIconRecord::data() const
{
    return payload();
}

void QNdefNfcSizeRecord::setSize(quint32 size)
{
    QByteArray data(4, 0);

    data[0] = (int) ((size & 0xFF000000) >> 24);
    data[1] = (int) ((size & 0x00FF0000) >> 16);
    data[2] = (int) ((size & 0x0000FF00) >> 8);
    data[3] = (int) ((size & 0x000000FF));

    setPayload(data);
}

quint32 QNdefNfcSizeRecord::size() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return 0;

    return ((p[0] << 24) & 0xFF000000) + ((p[1] << 16) & 0x00FF0000)
            + ((p[2] << 8) & 0x0000FF00) + (p[3] & 0x000000FF);
}

void QNdefNfcTypeRecord::setTypeInfo(const QString &type)
{
    setPayload(type.toUtf8());
}

QString QNdefNfcTypeRecord::typeInfo() const
{
    return QString::fromUtf8(payload());
}

QT_END_NAMESPACE
