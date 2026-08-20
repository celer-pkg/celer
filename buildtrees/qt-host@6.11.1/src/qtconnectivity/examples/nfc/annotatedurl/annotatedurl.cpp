// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include "annotatedurl.h"

#include <QDebug>
#include <QLocale>
#include <QUrl>

#include <QDesktopServices>
#include <QEvent>

#include <QNdefMessage>
#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>
#include <QNdefRecord>
#include <QNearFieldManager>
#include <QNearFieldTarget>

#include <QGridLayout>
#include <QLabel>

AnnotatedUrl::AnnotatedUrl(QObject *parent)
    : QObject(parent),
      manager(new QNearFieldManager(this))
{
    connect(manager, &QNearFieldManager::targetDetected,
            this, &AnnotatedUrl::targetDetected);
    connect(manager, &QNearFieldManager::targetLost,
            this, &AnnotatedUrl::targetLost);
    connect(manager, &QNearFieldManager::adapterStateChanged,
            this, &AnnotatedUrl::handleAdapterStateChange);

//! [populateFilter]
    messageFilter.setOrderMatch(false);
    messageFilter.appendRecord<QNdefNfcTextRecord>(1, 100);
    messageFilter.appendRecord<QNdefNfcUriRecord>(1, 1);
    messageFilter.appendRecord(QNdefRecord::Mime, "", 0, 1);
//! [populateFilter]
}

AnnotatedUrl::~AnnotatedUrl()
{

}

void AnnotatedUrl::startDetection()
{
    if (!manager->isEnabled()) {
        qWarning() << "NFC not enabled";
        emit nfcStateChanged(false);
        return;
    }

    if (manager->startTargetDetection(QNearFieldTarget::NdefAccess))
        emit nfcStateChanged(true);
}

void AnnotatedUrl::targetDetected(QNearFieldTarget *target)
{
    if (!target)
        return;

    connect(target, &QNearFieldTarget::ndefMessageRead,
            this, &AnnotatedUrl::handlePolledNdefMessage);
    connect(target, &QNearFieldTarget::error, this,
            [this]() { emit tagError("Tag read error"); });
    target->readNdefMessages();
}

void AnnotatedUrl::targetLost(QNearFieldTarget *target)
{
    if (target)
        target->deleteLater();
}

void AnnotatedUrl::handlePolledNdefMessage(QNdefMessage message)
{
    QNearFieldTarget *target = qobject_cast<QNearFieldTarget *>(sender());
    handleMessage(message, target);
}

//! [handleAdapterState]
void AnnotatedUrl::handleAdapterStateChange(QNearFieldManager::AdapterState state)
{
    if (state == QNearFieldManager::AdapterState::Online) {
        startDetection();
    } else if (state == QNearFieldManager::AdapterState::Offline) {
        manager->stopTargetDetection();
        emit nfcStateChanged(false);
    }
}
//! [handleAdapterState]

//! [handleMessage 1]
void AnnotatedUrl::handleMessage(const QNdefMessage &message, QNearFieldTarget *target)
{
//! [handleMessage 1]
    Q_UNUSED(target);

//! [handleMessage 2]
    if (!messageFilter.match(message)) {
        emit tagError("Invalid message format");
        return;
    }
//! [handleMessage 2]

    enum {
        MatchedNone,
        MatchedFirst,
        MatchedEnglish,
        MatchedLanguage,
        MatchedLanguageAndCountry
    } bestMatch = MatchedNone;

    QLocale defaultLocale;

    QString title;
    QUrl url;
    QPixmap pixmap;

//! [handleMessage 3]
    for (const QNdefRecord &record : message) {
        if (record.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord textRecord(record);

            title = textRecord.text();
            QLocale locale(textRecord.locale());
//! [handleMessage 3]
            // already found best match
            if (bestMatch == MatchedLanguageAndCountry) {
                // do nothing
            } else if (bestMatch <= MatchedLanguage && locale == defaultLocale) {
                bestMatch = MatchedLanguageAndCountry;
            } else if (bestMatch <= MatchedEnglish &&
                       locale.language() == defaultLocale.language()) {
                bestMatch = MatchedLanguage;
            } else if (bestMatch <= MatchedFirst && locale.language() == QLocale::English) {
                bestMatch = MatchedEnglish;
            } else if (bestMatch == MatchedNone) {
                bestMatch = MatchedFirst;
            }
//! [handleMessage 4]
        } else if (record.isRecordType<QNdefNfcUriRecord>()) {
            QNdefNfcUriRecord uriRecord(record);

            url = uriRecord.uri();
        } else if (record.typeNameFormat() == QNdefRecord::Mime &&
                   record.type().startsWith("image/")) {
            pixmap = QPixmap::fromImage(QImage::fromData(record.payload()));
        }
//! [handleMessage 4]
//! [handleMessage 5]
    }

    emit annotatedUrl(url, title, pixmap);
}
//! [handleMessage 5]
