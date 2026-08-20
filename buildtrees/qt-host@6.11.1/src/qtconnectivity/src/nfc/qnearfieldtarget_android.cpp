// Copyright (C) 2016 Centria research and development
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qnearfieldtarget_android_p.h"
#include "android/androidjninfc_p.h"
#include "qdebug.h"

#define NDEFTECHNOLOGY              QStringLiteral("android.nfc.tech.Ndef")
#define NDEFFORMATABLETECHNOLOGY    QStringLiteral("android.nfc.tech.NdefFormatable")
#define ISODEPTECHNOLOGY            QStringLiteral("android.nfc.tech.IsoDep")
#define NFCATECHNOLOGY              QStringLiteral("android.nfc.tech.NfcA")
#define NFCBTECHNOLOGY              QStringLiteral("android.nfc.tech.NfcB")
#define NFCFTECHNOLOGY              QStringLiteral("android.nfc.tech.NfcF")
#define NFCVTECHNOLOGY              QStringLiteral("android.nfc.tech.NfcV")
#define MIFARECLASSICTECHNOLOGY     QStringLiteral("android.nfc.tech.MifareClassic")
#define MIFARECULTRALIGHTTECHNOLOGY QStringLiteral("android.nfc.tech.MifareUltralight")

#define MIFARETAG   QStringLiteral("com.nxp.ndef.mifareclassic")
#define NFCTAGTYPE1 QStringLiteral("org.nfcforum.ndef.type1")
#define NFCTAGTYPE2 QStringLiteral("org.nfcforum.ndef.type2")
#define NFCTAGTYPE3 QStringLiteral("org.nfcforum.ndef.type3")
#define NFCTAGTYPE4 QStringLiteral("org.nfcforum.ndef.type4")

QNearFieldTargetPrivateImpl::QNearFieldTargetPrivateImpl(QJniObject intent,
                                                         const QByteArray uid,
                                                         QObject *parent)
:   QNearFieldTargetPrivate(parent),
    targetIntent(intent),
    targetUid(uid)
{
    updateTechList();
    updateType();
    setupTargetCheckTimer();
}

QNearFieldTargetPrivateImpl::~QNearFieldTargetPrivateImpl()
{
    releaseIntent();
    Q_EMIT targetDestroyed(targetUid);
}

QByteArray QNearFieldTargetPrivateImpl::uid() const
{
    return targetUid;
}

QNearFieldTarget::Type QNearFieldTargetPrivateImpl::type() const
{
    return tagType;
}

QNearFieldTarget::AccessMethods QNearFieldTargetPrivateImpl::accessMethods() const
{
    QNearFieldTarget::AccessMethods result = QNearFieldTarget::UnknownAccess;

    if (techList.contains(NDEFTECHNOLOGY)
            || techList.contains(NDEFFORMATABLETECHNOLOGY))
        result |= QNearFieldTarget::NdefAccess;

    if (techList.contains(ISODEPTECHNOLOGY)
            || techList.contains(NFCATECHNOLOGY)
            || techList.contains(NFCBTECHNOLOGY)
            || techList.contains(NFCFTECHNOLOGY)
            || techList.contains(NFCVTECHNOLOGY))
        result |= QNearFieldTarget::TagTypeSpecificAccess;

    return result;
}

bool QNearFieldTargetPrivateImpl::disconnect()
{
    if (!tagTech.isValid())
        return false;
    QJniEnvironment env;
    bool connected = tagTech.callMethod<jboolean>("isConnected");
    if (!connected)
        return false;
    auto methodId = env.findMethod<void>(tagTech.objectClass(), "close");
    if (!methodId)
        return false;
    env->CallVoidMethod(tagTech.object(), methodId);
    return !env.checkAndClearExceptions();
}

bool QNearFieldTargetPrivateImpl::hasNdefMessage()
{
    return techList.contains(NDEFTECHNOLOGY);
}

QNearFieldTarget::RequestId QNearFieldTargetPrivateImpl::readNdefMessages()
{
    // Making sure that target has NDEF messages
    if (!hasNdefMessage())
        return QNearFieldTarget::RequestId();

    // Making sure that target is still in range
    QNearFieldTarget::RequestId requestId(new QNearFieldTarget::RequestIdPrivate);
    if (!targetIntent.isValid()) {
        reportError(QNearFieldTarget::TargetOutOfRangeError, requestId);
        return requestId;
    }

    // Getting Ndef technology object
    if (!setTagTechnology({NDEFTECHNOLOGY})) {
        reportError(QNearFieldTarget::UnsupportedError, requestId);
        return requestId;
    }

    // Connect
    if (!connect()) {
        reportError(QNearFieldTarget::ConnectionError, requestId);
        return requestId;
    }

    // Get NdefMessage object
    QJniObject ndefMessage = tagTech.callMethod<QtJniTypes::NdefMessage>("getNdefMessage");
    if (!ndefMessage.isValid()) {
        reportError(QNearFieldTarget::NdefReadError, requestId);
        return requestId;
    }

    QByteArray ndefMessageQBA = ndefMessage.callMethod<jbyte[]>("toByteArray").toContainer();

    // Sending QNdefMessage, requestCompleted and exit.
    QNdefMessage qNdefMessage = QNdefMessage::fromByteArray(ndefMessageQBA);
    QMetaObject::invokeMethod(this, [this, qNdefMessage]() {
        Q_EMIT this->q_ptr->ndefMessageRead(qNdefMessage);
    }, Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, [this, requestId]() {
        Q_EMIT this->requestCompleted(requestId);
    }, Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, [this, qNdefMessage, requestId]() {
        //TODO This is an Android specific signal in NearFieldTarget.
        //     We need to check if it is still necessary.
        Q_EMIT this->ndefMessageRead(qNdefMessage, requestId);
    }, Qt::QueuedConnection);
    return requestId;
}

int QNearFieldTargetPrivateImpl::maxCommandLength() const
{
    QJniObject tagTech;
    if (techList.contains(ISODEPTECHNOLOGY))
        tagTech = getTagTechnology(ISODEPTECHNOLOGY);
    else if (techList.contains(NFCATECHNOLOGY))
        tagTech = getTagTechnology(NFCATECHNOLOGY);
    else if (techList.contains(NFCBTECHNOLOGY))
        tagTech = getTagTechnology(NFCBTECHNOLOGY);
    else if (techList.contains(NFCFTECHNOLOGY))
        tagTech = getTagTechnology(NFCFTECHNOLOGY);
    else if (techList.contains(NFCVTECHNOLOGY))
        tagTech = getTagTechnology(NFCVTECHNOLOGY);
    else
        return 0;

    return tagTech.callMethod<jint>("getMaxTransceiveLength");
}

QNearFieldTarget::RequestId QNearFieldTargetPrivateImpl::sendCommand(const QByteArray &command)
{
    const int maxLength = maxCommandLength();
    if (maxLength == 0) {
        // Similar to the handling of "transceive" below, some devices (especially Samsung)
        // fail to report a lost card right away but consider it still present.
        // Thus, checkIsTargetLost() does not emit the targetLost signal. When actually
        // trying to communicate with the alleged card, the JNI call will fail with
        // an SecurityException, indicating that the card was, in fact, lost before.
        handleTargetLost();
        Q_EMIT error(QNearFieldTarget::ConnectionError, QNearFieldTarget::RequestId());
        return QNearFieldTarget::RequestId();
    }

    if (command.size() == 0 || command.size() > maxLength) {
        Q_EMIT error(QNearFieldTarget::InvalidParametersError, QNearFieldTarget::RequestId());
        return QNearFieldTarget::RequestId();
    }

    // Making sure that target has commands
    if (!(accessMethods() & QNearFieldTarget::TagTypeSpecificAccess))
        return QNearFieldTarget::RequestId();

    QJniEnvironment env;

    if (!setTagTechnology({ISODEPTECHNOLOGY, NFCATECHNOLOGY, NFCBTECHNOLOGY, NFCFTECHNOLOGY, NFCVTECHNOLOGY})) {
        Q_EMIT error(QNearFieldTarget::UnsupportedError, QNearFieldTarget::RequestId());
        return QNearFieldTarget::RequestId();
    }

    // Connecting
    QNearFieldTarget::RequestId requestId = QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate());
    if (!connect()) {
        reportError(QNearFieldTarget::ConnectionError, requestId);
        return requestId;
    }

    // Writing
    const QJniArray myNewVal = tagTech.callMethod<jbyte[]>("transceive", command);
    if (!myNewVal.isValid()) {
        // Some devices (Samsung, Huawei) throw an exception when the card is lost:
        // "android.nfc.TagLostException: Tag was lost". But there seems to be a bug that
        // isConnected still reports true. So we need to invalidate the target as soon as
        // possible and treat the card as lost.
        handleTargetLost();

        reportError(QNearFieldTarget::CommandError, requestId);
        return requestId;
    }
    QByteArray result = myNewVal.toContainer();

    setResponseForRequest(requestId, result, false);

    QMetaObject::invokeMethod(this, [this, requestId]() {
        Q_EMIT this->requestCompleted(requestId);
    }, Qt::QueuedConnection);

    return requestId;
}

QNearFieldTarget::RequestId QNearFieldTargetPrivateImpl::writeNdefMessages(const QList<QNdefMessage> &messages)
{
    if (messages.size() == 0)
        return QNearFieldTarget::RequestId();

    if (messages.size() > 1)
        qWarning("QNearFieldTarget::writeNdefMessages: Android supports writing only one NDEF message per tag.");

    QJniEnvironment env;
    const char *writeMethod;

    if (!setTagTechnology({NDEFFORMATABLETECHNOLOGY, NDEFTECHNOLOGY}))
        return QNearFieldTarget::RequestId();

    // Getting write method
    if (selectedTech == NDEFFORMATABLETECHNOLOGY)
        writeMethod = "format";
    else
        writeMethod = "writeNdefMessage";

    // Connecting
    QNearFieldTarget::RequestId requestId = QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate());
    if (!connect()) {
        reportError(QNearFieldTarget::ConnectionError, requestId);
        return requestId;
    }

    // Making NdefMessage object
    const QNdefMessage &message = messages.first();
    QtJniTypes::NdefMessage jmessage(message.toByteArray());
    if (!jmessage.isValid()) {
        reportError(QNearFieldTarget::UnknownError, requestId);
        return requestId;
    }

    // Writing
    auto methodId =
            env.findMethod<void, QtJniTypes::NdefMessage>(tagTech.objectClass(), writeMethod);
    if (methodId)
        env->CallVoidMethod(tagTech.object(), methodId, jmessage.object<jobject>());
    if (!methodId || env.checkAndClearExceptions()) {
        reportError(QNearFieldTarget::NdefWriteError, requestId);
        return requestId;
    }

    QMetaObject::invokeMethod(this, [this, requestId]() {
        Q_EMIT this->requestCompleted(requestId);
    }, Qt::QueuedConnection);
    return requestId;
}

void QNearFieldTargetPrivateImpl::setIntent(QJniObject intent)
{
    if (targetIntent == intent)
        return;

    releaseIntent();
    targetIntent = intent;
    if (targetIntent.isValid()) {
        // Updating tech list and type in case of there is another tag with same UID as one before.
        updateTechList();
        updateType();
        targetCheckTimer->start();
    }
}

void QNearFieldTargetPrivateImpl::checkIsTargetLost()
{
    if (!targetIntent.isValid() || !setTagTechnology({selectedTech})) {
        handleTargetLost();
        return;
    }

    QJniEnvironment env;
    bool connected = false;
    auto methodId = env.findMethod<jboolean>(tagTech.objectClass(), "isConnected");
    if (methodId)
        connected = env->CallBooleanMethod(tagTech.object(), methodId);
    if (!methodId || env.checkAndClearExceptions()) {
        handleTargetLost();
        return;
    }

    if (connected)
        return;

    methodId = env.findMethod<void>(tagTech.objectClass(), "connect");
    if (methodId)
        env->CallVoidMethod(tagTech.object(), methodId);
    if (!methodId || env.checkAndClearExceptions(QJniEnvironment::OutputMode::Silent)) {
        handleTargetLost();
        return;
    }
    methodId = env.findMethod<void>(tagTech.objectClass(), "close");
    if (methodId)
        env->CallVoidMethod(tagTech.object(), methodId);
    if (!methodId || env.checkAndClearExceptions(QJniEnvironment::OutputMode::Silent))
        handleTargetLost();
}

void QNearFieldTargetPrivateImpl::releaseIntent()
{
    targetCheckTimer->stop();

    targetIntent = QJniObject();
}

void QNearFieldTargetPrivateImpl::updateTechList()
{
    if (!targetIntent.isValid())
        return;

    // Getting tech list
    QJniEnvironment env;
    QJniObject tag = QtNfc::getTag(targetIntent);
    Q_ASSERT_X(tag.isValid(), "updateTechList", "could not get Tag object");

    const QJniArray techListArray = tag.callMethod<QtJniTypes::String[]>("getTechList");
    if (!techListArray.isValid()) {
        handleTargetLost();
        return;
    }

    // Converting tech list array to QStringList.
    techList.clear();
    for (const auto &tech : techListArray)
        techList.append(tech.toString());
}

void QNearFieldTargetPrivateImpl::updateType()
{
    tagType = getTagType();
}

QNearFieldTarget::Type QNearFieldTargetPrivateImpl::getTagType() const
{
    if (techList.contains(NDEFTECHNOLOGY)) {
        QJniObject ndef = getTagTechnology(NDEFTECHNOLOGY);
        QString qtype = ndef.callMethod<jstring>("getType").toString();

        if (qtype.compare(MIFARETAG) == 0)
            return QNearFieldTarget::MifareTag;
        if (qtype.compare(NFCTAGTYPE1) == 0)
            return QNearFieldTarget::NfcTagType1;
        if (qtype.compare(NFCTAGTYPE2) == 0)
            return QNearFieldTarget::NfcTagType2;
        if (qtype.compare(NFCTAGTYPE3) == 0)
            return QNearFieldTarget::NfcTagType3;
        if (qtype.compare(NFCTAGTYPE4) == 0)
            return QNearFieldTarget::NfcTagType4;
        return QNearFieldTarget::ProprietaryTag;
    } else if (techList.contains(NFCATECHNOLOGY)) {
        if (techList.contains(MIFARECLASSICTECHNOLOGY))
            return QNearFieldTarget::MifareTag;

        // Checking ATQA/SENS_RES
        // xxx0 0000  xxxx xxxx: Identifies tag Type 1 platform
        QJniObject nfca = getTagTechnology(NFCATECHNOLOGY);
        const QByteArray atqaQBA = nfca.callMethod<QByteArray>("getAtqa");
        if (atqaQBA.isEmpty())
            return QNearFieldTarget::ProprietaryTag;
        if ((atqaQBA[0] & 0x1F) == 0x00)
            return QNearFieldTarget::NfcTagType1;

        // Checking SAK/SEL_RES
        // xxxx xxxx  x00x x0xx: Identifies tag Type 2 platform
        // xxxx xxxx  x01x x0xx: Identifies tag Type 4 platform
        jshort sakS = nfca.callMethod<jshort>("getSak");
        if ((sakS & 0x0064) == 0x0000)
            return QNearFieldTarget::NfcTagType2;
        else if ((sakS & 0x0064) == 0x0020)
            return QNearFieldTarget::NfcTagType4A;
        return QNearFieldTarget::ProprietaryTag;
    } else if (techList.contains(NFCBTECHNOLOGY)) {
        return QNearFieldTarget::NfcTagType4B;
    } else if (techList.contains(NFCFTECHNOLOGY)) {
        return QNearFieldTarget::NfcTagType3;
    }

    return QNearFieldTarget::ProprietaryTag;
}

void QNearFieldTargetPrivateImpl::setupTargetCheckTimer()
{
    targetCheckTimer = new QTimer(this);
    targetCheckTimer->setInterval(1000);
    QObject::connect(targetCheckTimer, &QTimer::timeout, this, &QNearFieldTargetPrivateImpl::checkIsTargetLost);
    targetCheckTimer->start();
}

void QNearFieldTargetPrivateImpl::handleTargetLost()
{
    releaseIntent();
    Q_EMIT targetLost(this);
}

QJniObject QNearFieldTargetPrivateImpl::getTagTechnology(const QString &tech) const
{
    QString techClass(tech);
    techClass.replace(QLatin1Char('.'), QLatin1Char('/'));

    // Getting requested technology
    QJniObject tag = QtNfc::getTag(targetIntent);
    Q_ASSERT_X(tag.isValid(), "getTagTechnology", "could not get Tag object");

    const QString sig = QString::fromUtf8("(Landroid/nfc/Tag;)L%1;");
    QJniObject tagTech = QJniObject::callStaticObjectMethod(techClass.toUtf8().constData(), "get",
            sig.arg(techClass).toUtf8().constData(), tag.object<jobject>());

    return tagTech;
}

bool QNearFieldTargetPrivateImpl::setTagTechnology(const QStringList &technologies)
{
    for (const QString &tech : technologies) {
        if (techList.contains(tech)) {
            if (selectedTech == tech) {
                return true;
            }
            selectedTech = tech;
            tagTech = getTagTechnology(tech);
            return tagTech.isValid();
        }
    }

    return false;
}

bool QNearFieldTargetPrivateImpl::connect()
{
    if (!tagTech.isValid())
        return false;

    QJniEnvironment env;
    auto methodId = env.findMethod<jboolean>(tagTech.objectClass(), "isConnected");
    bool connected = false;
    if (methodId)
        connected = env->CallBooleanMethod(tagTech.object(), methodId);
    if (!methodId || env.checkAndClearExceptions()) {
        handleTargetLost();
        return false;
    }

    if (connected)
        return true;

    setCommandTimeout(3000);
    methodId = env.findMethod<void>(tagTech.objectClass(), "connect");
    if (!methodId)
        return false;
    env->CallVoidMethod(tagTech.object(), methodId);
    return !env.checkAndClearExceptions();
}

bool QNearFieldTargetPrivateImpl::setCommandTimeout(int timeout)
{
    if (!tagTech.isValid())
        return false;

    QJniEnvironment env;
    auto methodId = env.findMethod<void, jint>(tagTech.objectClass(), "setTimeout");
    if (methodId)
        env->CallVoidMethod(tagTech.object(), methodId, timeout);
    return methodId && !env.checkAndClearExceptions();
}
