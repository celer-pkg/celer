// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qndeffilter.h"
#include "qndefmessage.h"

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QVarLengthArray>

QT_BEGIN_NAMESPACE

/*!
    \class QNdefFilter
    \brief The QNdefFilter class provides a filter for matching NDEF messages.

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \since 5.2

    The QNdefFilter encapsulates the structure of an NDEF message and is used for
    matching messages that have a particular structure.

    The following filter matches NDEF messages that contain a single smart poster record:

    \code
    QNdefFilter filter;
    filter.append(QNdefRecord::NfcRtd, "Sp");
    \endcode

    The following filter matches NDEF messages that contain a URI, a localized piece of text and an
    optional JPEG image.  The order of the records must be in the order specified:

    \code
    QNdefFilter filter;
    filter.setOrderMatch(true);
    filter.appendRecord(QNdefRecord::NfcRtd, "U");
    filter.appendRecord<QNdefNfcTextRecord>();
    filter.appendRecord(QNdefRecord::Mime, "image/jpeg", 0, 1);
    \endcode

    The \l match() method can be used to check if a message matches the filter.

    \section1 Matching Algorithms

    The filter behavior depends on the value of \l orderMatch() parameter.

    \note In the discussion below we will consider the filter records to be
    equal if their \c typeNameFormat and \c type parameters match. Joining
    two records means adding their \c minimum and \c maximum values,
    respectively.

    \section2 Unordered Matching

    If the record order is not taken into account, all the equal records in the
    filter can be joined. The resulting filter will contain only unique records,
    each with the updated \c minimum and \c maximum value.

    Consider the following example:

    \code
    QNdefFilter filter;
    filter.appendRecord<QNdefNfcTextRecord>(0, 1);
    filter.appendRecord<QNdefNfcTextRecord>(0, 1);
    filter.appendRecord(QNdefRecord::Mime, "", 1, 1);
    filter.appendRecord<QNdefNfcTextRecord>(1, 1);
    filter.setOrderMatch(false);
    \endcode

    With the unordered matching, the filter will be simplified to the following:

    \code
    QNdefFilter filter;
    filter.appendRecord<QNdefNfcTextRecord>(1, 3);
    filter.appendRecord(QNdefRecord::Mime, "", 1, 1);
    filter.setOrderMatch(false);
    \endcode

    Once the filter contains only the unique records, the matching algorithm
    iterates through the message and calculates the actual amount of records of
    each type. If all the actual amounts fit in the corresponding
    [minimum, maximum] ranges, the matching algorithm returns \c true.

    \section2 Ordered Matching

    If the record order is important, a different approach is applied. In this
    case the equal records can't be simply joined together. However, the
    consecutive equal records can still be joined. Then, the matching
    algorithm iterates through the message, this time also taking the positions
    of the records into account.

    \section2 Handling Empty Type in Filter Record

    It's possible to add a filter record with an empty \c type. In this case
    the empty type will act as a wildcard for any type.

    For example, the filter can be defined as follows:

    \code
    QNdefFilter filter;
    filter.addRecord(QNdefRecord::Mime, "", 1, 1);
    \endcode

    This filter specifies that the message must contain exactly one NDEF record
    with \l {QNdefRecord::}{Mime} \l {QNdefRecord::}{typeNameFormat}(), and any
    \l {QNdefRecord::}{type}().

    \section2 Handling Extra Records in the Message

    If the message contains some records that do not match \e any record in the
    filter, the matching algorithm will return \c false.

    \section2 Filter Examples

    In the table below, each filter record is specified by the following
    parameters (in the given order):

    \list
    \li \c typeNameFormat - contains the \l {QNdefRecord::}{typeNameFormat}() of
    the record.
    \li \c type - contains the \l {QNdefRecord::}{type}() of the record.
    \li \c minimum - contains the minimum amount of occurrences of the record
    in the message.
    \li \c maximum - contains the maximum amount of occurrences of the record
    in the message.
    \endlist

    The filter contains multiple records.

    The message consists of multiple \l {QNdefRecord}s. In the table below, only
    the \l {QNdefRecord::}{typeNameFormat}() and \l {QNdefRecord::}{type}() of
    each record will be shown, because the other parameters do not matter for
    filtering.

    \table
    \header
        \li Filter
        \li Message
        \li Match Result
        \li Comment
    \row
        \li Empty filter
        \li Empty message
        \li Match
        \li
    \row
        \li Empty filter
        \li Non-empty message
        \li No match
        \li
    \row
        \li Non-empty filter
        \li Empty message
        \li No match
        \li
    \row
        \li {1, 2}[(QNdefRecord::NfcRtd, "T", 1, 2),
            (QNdefRecord::Mime, "", 1, 1), (QNdefRecord::Empty, "", 0, 100)]
        \li {1, 2}[(QNdefRecord::Mime, "image/jpeg"), (QNdefRecord::Empty, ""),
            (QNdefRecord::NfcRtd, "T"), (QNdefRecord::Empty, ""),
            (QNdefRecord::NfcRtd, "T")]
        \li Unordered: match
        \li {1, 2} Ordered filter does not match because the message must start
            with a QNdefRecord::NfcRtd record, but it starts with
            QNdefRecord::Mime.
    \row
        \li Ordered: no match
    \row
        \li {1, 2}[(QNdefRecord::NfcRtd, "T", 0, 2),
            (QNdefRecord::Mime, "", 1, 1), (QNdefRecord::NfcRtd, "T", 1, 1)]
        \li {1, 2}[(QNdefRecord::NfcRtd, "T"), (QNdefRecord::NfcRtd, "T"),
            (QNdefRecord::Mime, "image/jpeg")]
        \li Unordered: match
        \li {1, 2} Ordered filter does not match because an QNdefRecord::NfcRtd
            record is expected after QNdefRecord::Mime, but the message does not
            have it.
    \row
        \li Ordered: no match
    \row
        \li {1, 2}[(QNdefRecord::NfcRtd, "T", 0, 2),
            (QNdefRecord::NfcRtd, "T", 1, 1), (QNdefRecord::Mime, "", 1, 1)]
        \li {1, 2}[(QNdefRecord::NfcRtd, "T"),
            (QNdefRecord::Mime, "image/jpeg")]
        \li Unordered: match
        \li {1, 2} Both cases match because the message contains the required
            minimum of records in the correct order.
    \row
        \li Ordered: match
    \endtable

*/

/*!
    \class QNdefFilter::Record
    \brief The QNdefFilter::Record struct contains the information about a
    filter record.

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \since 5.2

    The QNdefFilter::Record struct is used to populate the QNdefFilter object.
    Each record contains the following information:
    \list
    \li \l {QNdefRecord::TypeNameFormat} of the corresponding \l QNdefRecord.
    \li Type of the \l QNdefRecord.
    \li Minimum and maximum amount of the records with such parameters in the
        filter.
    \endlist
*/

/*!
    \fn template<typename T> bool QNdefFilter::appendRecord(unsigned int min, unsigned int max)

    Appends a record matching the template parameter to the NDEF filter.
    The record must occur between \a min and \a max times in the NDEF message.

    Returns \c true if the record was appended successfully. Otherwise returns
    \c false.
*/

class QNdefFilterPrivate : public QSharedData
{
public:
    QNdefFilterPrivate();

    bool orderMatching;
    QList<QNdefFilter::Record> filterRecords;
};

QNdefFilterPrivate::QNdefFilterPrivate()
:   orderMatching(false)
{
}

/*!
    Constructs a new NDEF filter.
*/
QNdefFilter::QNdefFilter()
:   d(new QNdefFilterPrivate)
{
}

/*!
    Constructs a new NDEF filter that is a copy of \a other.
*/
QNdefFilter::QNdefFilter(const QNdefFilter &other)
:   d(other.d)
{
}

/*!
    Destroys the NDEF filter.
*/
QNdefFilter::~QNdefFilter()
{
}

/*!
    Assigns \a other to this filter and returns a reference to this filter.
*/
QNdefFilter &QNdefFilter::operator=(const QNdefFilter &other)
{
    if (d != other.d)
        d = other.d;

    return *this;
}

/*!
    \since 6.2

    Returns \c true if the \a message matches the given filter.
    Otherwise returns \c false.

    See \l {Matching Algorithms} for more detailed explanation of matching.
*/
bool QNdefFilter::match(const QNdefMessage &message) const
{
    // empty filter matches only empty message
    if (d->filterRecords.isEmpty())
        return message.isEmpty();

    bool matched = true;
    int totalCount = 0;

    if (!d->orderMatching) {
        // Order is not important. The most reliable way is to merge all the
        // similar records, and then simply check the amount of occurrences.
        using MapKey = QPair<QNdefRecord::TypeNameFormat, QByteArray>;

        // Creating a map from the list of filter records.
        QMap<MapKey, Record> joinedFilterRecords;
        for (const auto &rec : d->filterRecords) {
            const auto key = qMakePair(rec.typeNameFormat, rec.type);
            if (joinedFilterRecords.contains(key)) {
                joinedFilterRecords[key].minimum += rec.minimum;
                joinedFilterRecords[key].maximum += rec.maximum;
            } else {
                joinedFilterRecords.insert(key, rec);
            }
        }
        // Checking the message, calculate occurrences.
        QMap<MapKey, unsigned int> counts; // current number of occurrences
        for (const auto &record : message) {
            const auto key = qMakePair(record.typeNameFormat(), record.type());
            // Do not forget that we handle an empty type as "any type".
            const auto emptyTypeKey = qMakePair(record.typeNameFormat(), QByteArray());

            if (joinedFilterRecords.contains(key))
                counts[key] += 1;
            else if (joinedFilterRecords.contains(emptyTypeKey))
                counts[emptyTypeKey] += 1;
        }
        // Check that the occurrences match [min; max] range.
        for (auto it = joinedFilterRecords.cbegin(); it != joinedFilterRecords.cend(); ++it) {
            const auto count = counts.value(it.key(), 0);
            totalCount += count;
            if (count < it.value().minimum || count > it.value().maximum) {
                matched = false;
                break;
            }
        }
    } else {
        // Order *is* important. Need to iterate the list.

        // Here we need to merge consecutive records with the same parameters.
        QList<Record> mergedRecords;
        // we know that filer is not empty here
        Record currentRecord = d->filterRecords.first();
        for (qsizetype i = 1; i < d->filterRecords.size(); ++i) {
            const auto &rec = d->filterRecords.at(i);
            if (rec.typeNameFormat == currentRecord.typeNameFormat
                && rec.type == currentRecord.type) {
                currentRecord.minimum += rec.minimum;
                currentRecord.maximum += rec.maximum;
            } else {
                mergedRecords.push_back(currentRecord);
                currentRecord = rec;
            }
        }
        mergedRecords.push_back(currentRecord);

        // The list contains the current number of occurrences of each record.
        QVarLengthArray<unsigned int> counts(mergedRecords.size(), 0);

        // Iterate through the messages and calculate the number of occurrences.
        qsizetype filterIndex = 0;
        for (qsizetype messageIndex = 0; matched && messageIndex < message.size(); ++messageIndex) {
            const auto &messageRec = message.at(messageIndex);
            // Try to find a filter record that matches the message record.
            // We start from the last processed filter record, not from the very
            // beginning (because the order matters).
            qsizetype idx = filterIndex;
            for (; idx < mergedRecords.size(); ++idx) {
                const auto &filterRec = mergedRecords.at(idx);
                if (filterRec.typeNameFormat == messageRec.typeNameFormat()
                    && (filterRec.type == messageRec.type() || filterRec.type.isEmpty())) {
                    counts[idx] += 1;
                    break;
                } else if (counts[idx] < filterRec.minimum || counts[idx] > filterRec.maximum) {
                    // The current message record does not match the current
                    // filter record, but we didn't get enough records to
                    // fulfill the filter => that's an error.
                    matched = false;
                    break;
                }
            }
            filterIndex = idx;
        }

        if (matched) {
            // Check that the occurrences match [min; max] range.
            for (qsizetype i = 0; i < mergedRecords.size(); ++i) {
                const auto &rec = mergedRecords.at(i);
                totalCount += counts[i];
                if (counts[i] < rec.minimum || counts[i] > rec.maximum) {
                    matched = false;
                    break;
                }
            }
        }
    }

    // Check if the message has records that do not match any record from the
    // filter. To do it we just need to compare the total count of records,
    // detected by filter, with the full message size. If they do not match,
    // we have some records, that are not covered by the filter.
    if (matched && (totalCount != message.size())) {
        matched = false;
    }

    return matched;
}

/*!
    Clears the filter.
*/
void QNdefFilter::clear()
{
    d->orderMatching = false;
    d->filterRecords.clear();
}

/*!
    Sets the ordering requirements of the filter. If \a on is \c {true}, the
    filter will only match if the order of records in the filter matches the
    order of the records in the NDEF message. If \a on is \c {false}, the order
    of the records is not taken into account when matching.

    By default record order is not taken into account.
*/
void QNdefFilter::setOrderMatch(bool on)
{
    d->orderMatching = on;
}

/*!
    Returns \c true if the filter takes NDEF record order into account when
    matching. Otherwise returns \c false.
*/
bool QNdefFilter::orderMatch() const
{
    return d->orderMatching;
}

/*!
    Appends a record with type name format \a typeNameFormat and type \a type
    to the NDEF filter. The record must occur between \a min and \a max times
    in the NDEF message.

    Returns \c true if the record was appended successfully. Otherwise returns
    \c false.
*/
bool QNdefFilter::appendRecord(QNdefRecord::TypeNameFormat typeNameFormat, const QByteArray &type,
                               unsigned int min, unsigned int max)
{
    QNdefFilter::Record record;

    record.typeNameFormat = typeNameFormat;
    record.type = type;
    record.minimum = min;
    record.maximum = max;

    return appendRecord(record);
}

static bool verifyRecord(const QNdefFilter::Record &record)
{
    return record.minimum <= record.maximum;
}

/*!
    Verifies the \a record and appends it to the NDEF filter.

    Returns \c true if the record was appended successfully. Otherwise returns
    \c false.
*/
bool QNdefFilter::appendRecord(const Record &record)
{
    if (verifyRecord(record)) {
        d->filterRecords.append(record);
        return true;
    }
    return false;
}

/*!
    Returns the NDEF record at index \a i.

    \a i must be a valid index (i.e. 0 <= i < \l recordCount()).

    \sa recordCount()
*/
QNdefFilter::Record QNdefFilter::recordAt(qsizetype i) const
{
    return d->filterRecords.at(i);
}

/*!
    Returns the number of NDEF records in the filter.
*/
qsizetype QNdefFilter::recordCount() const
{
    return d->filterRecords.size();
}

QT_END_NAMESPACE
