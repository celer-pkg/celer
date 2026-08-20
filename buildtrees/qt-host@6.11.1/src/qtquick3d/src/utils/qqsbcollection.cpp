// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qqsbcollection_p.h"
#include <QtCore/QLockFile>
#include <QtCore/QSaveFile>
#include <QtCore/QCryptographicHash>
#include <QtCore/QLoggingCategory>
#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcQuick3DCache, "qt.quick3d.cache")

QQsbCollection::~QQsbCollection()
{
}

QDataStream &operator<<(QDataStream &stream, const QQsbCollection::Entry &entry)
{
    return (stream << entry.key << entry.value);
}

QDataStream &operator>>(QDataStream &stream, QQsbCollection::Entry &entry)
{
    QByteArray key;
    qint64 value;
    stream >> key >> value;
    entry = QQsbCollection::Entry(key, value);
    return stream;
}

size_t qHash(const QQsbCollection::Entry &entry, size_t)
{
    return entry.hashKey;
}

bool operator==(const QQsbCollection::Entry &l, const QQsbCollection::Entry &r)
{
    return (l.key == r.key);
}

QDataStream &operator<<(QDataStream &stream, const QQsbCollection::EntryDesc &entryDesc)
{
    return (stream << entryDesc.materialKey
            << entryDesc.featureSet
            << entryDesc.vertShader.serialized()
            << entryDesc.fragShader.serialized());
}

QDataStream &operator>>(QDataStream &stream, QQsbCollection::EntryDesc &entryDesc)
{
    QByteArray desc;
    QQsbCollection::FeatureSet fs;
    QByteArray vertData;
    QByteArray fragData;
    stream >> desc >> fs >> vertData >> fragData;
    entryDesc.materialKey = desc;
    entryDesc.featureSet = fs;
    entryDesc.vertShader = QShader::fromSerialized(vertData);
    entryDesc.fragShader = QShader::fromSerialized(fragData);
    return stream;
}

static constexpr quint64 MagicaDS = 0x3933333335346337;
static constexpr qint64 HeaderSize = sizeof(qint64 /*startOffs*/) + sizeof(quint8 /*version*/) + sizeof(quint32 /*qtVersion*/) + sizeof(MagicaDS);
static constexpr quint32 QtVersion = (QT_VERSION_MAJOR << 16) | (QT_VERSION_MINOR << 8) | (QT_VERSION_PATCH);

bool QQsbCollection::readEndHeader(QDataStream &ds, qint64 *startPos, quint8 *version)
{
    quint64 fileId = 0;
    quint32 qtver = 0;
    ds >> *startPos >> *version >> qtver >> fileId;
    if (fileId != MagicaDS) {
        qCDebug(lcQuick3DCache, "Corrupt qsbc file");
        return false;
    }
    if (*version != Version::Two) {
        qCDebug(lcQuick3DCache, "qsbc file has an unsupported version");
        return false;
    }
    if (qtver != QtVersion) {
        qCDebug(lcQuick3DCache, "qsbc file is for a different Qt version");
        return false;
    }
    return true;
}

bool QQsbCollection::readEndHeader(QIODevice *device, EntryMap *entries, quint8 *version)
{
    bool result = false;
    const qint64 size = device->size();
    if (device->seek(size - HeaderSize)) {
        QDataStream ds(device);
        ds.setVersion(QDataStream::Qt_6_0);
        qint64 startPos = 0;
        if (readEndHeader(ds, &startPos, version)) {
            if (startPos >= 0 && startPos < size && device->seek(startPos)) {
                ds >> *entries;
                result = true;
            }
        }
    }
    return result;
}

void QQsbCollection::writeEndHeader(QDataStream &ds, qint64 startPos, quint8 version, quint64 magic)
{
    ds << startPos << version << QtVersion << magic;
}

void QQsbCollection::writeEndHeader(QIODevice *device, const EntryMap &entries)
{
    if (!device->atEnd()) {
        device->seek(device->size() - 1);
        Q_ASSERT(device->atEnd());
    }
    QDataStream ds(device);
    ds.setVersion(QDataStream::Qt_6_0);
    const qint64 startPos = device->pos();
    ds << entries;
    writeEndHeader(ds, startPos, quint8(Version::Two), MagicaDS);
}

QByteArray QQsbCollection::EntryDesc::generateSha(const QByteArray &materialKey, const FeatureSet &featureSet)
{
    QCryptographicHash h(QCryptographicHash::Algorithm::Sha1);
    h.addData(materialKey);
    for (auto it = featureSet.cbegin(), end = featureSet.cend(); it != end; ++it) {
        if (it.value())
            h.addData(it.key());
    }
    return h.result().toHex();
}

QByteArray QQsbCollection::EntryDesc::generateSha() const
{
    return generateSha(materialKey, featureSet);
}

QQsbCollection::EntryMap QQsbInMemoryCollection::availableEntries() const
{
    return EntryMap(entries.keyBegin(), entries.keyEnd());
}

QQsbCollection::Entry QQsbInMemoryCollection::addEntry(const QByteArray &key, const EntryDesc &entryDesc)
{
    Entry e(key);
    if (!entries.contains(e)) {
        entries.insert(e, entryDesc);
        return e;
    }
    return {}; // can only add with a given key once
}

bool QQsbInMemoryCollection::extractEntry(Entry entry, EntryDesc &entryDesc)
{
    auto it = entries.constFind(entry);
    if (it != entries.constEnd()) {
        entryDesc = *it;
        return true;
    }
    return false;
}

void QQsbInMemoryCollection::clear()
{
    entries.clear();
}

static inline QString lockFileName(const QString &name)
{
    return name + QLatin1String(".lck");
}

bool QQsbInMemoryCollection::load(const QString &filename)
{
    QLockFile lock(lockFileName(filename));
    if (!lock.lock()) {
        qCDebug(lcQuick3DCache, "Could not create shader cache lock file '%s'",
                 qPrintable(lock.fileName()));
        return false;
    }

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
        qCDebug(lcQuick3DCache, "Failed to open qsbc file %s", qPrintable(filename));
        return false;
    }

    EntryMap entryMap;
    quint8 version = 0;
    if (!readEndHeader(&f, &entryMap, &version)) {
        qCDebug(lcQuick3DCache, "Ignoring qsbc file %s", qPrintable(filename));
        return false;
    }

    f.seek(0);
    const qint64 size = f.size();

    clear();

    const qsizetype entryCountBefore = entries.size();
    for (const Entry &e : entryMap) {
        const qint64 offset = e.value;
        if (e.isValid() && offset >= 0 && size > offset && f.seek(offset)) {
            QDataStream ds(&f);
            ds.setVersion(QDataStream::Qt_6_0);
            EntryDesc entryDesc;
            ds >> entryDesc;
            entries.insert(Entry(e.key), entryDesc);
        }
    }
    qCDebug(lcQuick3DCache, "Loaded %d material cache entries from %s",
            int(entries.size() - entryCountBefore),
            qPrintable(filename));

    return true;
}

bool QQsbInMemoryCollection::save(const QString &filename)
{
    QLockFile lock(lockFileName(filename));
    if (!lock.lock()) {
        qCDebug(lcQuick3DCache, "Could not create shader cache lock file '%s'",
                 qPrintable(lock.fileName()));
        return false;
    }

#if QT_CONFIG(temporaryfile)
    QSaveFile f(filename);
#else
    QFile f(filename);
#endif
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCDebug(lcQuick3DCache, "Failed to write qsbc file %s", qPrintable(filename));
        return false;
    }

    QDataStream ds(&f);
    ds.setVersion(QDataStream::Qt_6_0);

    EntryMap entryMap;
    for (auto it = entries.cbegin(), end = entries.cend(); it != end; ++it) {
        const qint64 offset = f.pos();
        ds << it.value();
        entryMap.insert(Entry(it.key().key, offset));
    }

    writeEndHeader(&f, entryMap);

    qCDebug(lcQuick3DCache, "Writing %d material cache entries to %s",
            int(entryMap.size()),
            qPrintable(filename));

#if QT_CONFIG(temporaryfile)
    return f.commit();
#else
    return true;
#endif
}

QQsbIODeviceCollection::QQsbIODeviceCollection(const QString &filePath)
    : file(filePath)
    , device(file)
{
}

QQsbIODeviceCollection::QQsbIODeviceCollection(QIODevice &dev)
    : device(dev)
    , devOwner(DeviceOwner::Extern)
{

}

QQsbIODeviceCollection::~QQsbIODeviceCollection()
{
    if (!entries.isEmpty() || device.isOpen())
        unmap();
}

bool QQsbIODeviceCollection::map(MapMode mode)
{
    if (device.isOpen()) {
        // Make sure Truncate is set if we're writing.
        if ((device.openMode() & QIODevice::WriteOnly) != 0) {
            if ((device.openMode() & QIODevice::Truncate) == 0) {
                qWarning("Open mode needs to have Truncate set for writing!");
                return false;
            }
            if ((device.openMode() & QIODevice::Text) != 0) {
                qWarning("Open mode can't have Text mode set!");
                return false;
            }
        }
    } else if (!device.open(QIODevice::OpenMode(mode))) {
        qWarning("Unable to open device!");
        return false;
    }

    if (mode == Write)
        return true;

    Q_ASSERT(mode == Read);

    const bool ret = readEndHeader(&device, &entries, &version);

    if (!ret)
        unmap();

    return ret;
}

void QQsbIODeviceCollection::unmap()
{
    if (device.isOpen() && ((device.openMode() & Write) == Write)) {
        if (!entries.isEmpty()) {
            writeEndHeader(&device, entries);
        } else {
            if (devOwner == DeviceOwner::Self)
                file.remove();
        }
    }
    device.close();
    entries.clear();
}

QQsbCollection::EntryMap QQsbIODeviceCollection::availableEntries() const
{
    return entries;
}

QQsbCollection::Entry QQsbIODeviceCollection::addEntry(const QByteArray &key, const EntryDesc &entryDesc)
{
    if (entries.contains(Entry(key)) || !map(MapMode::Write))
        return {};

    QDataStream ds(&device);
    ds.setVersion(QDataStream::Qt_6_0);
    const auto offset = device.pos();
    ds << entryDesc;
    Entry e(key, offset);
    entries.insert(e);
    return e;
}

bool QQsbIODeviceCollection::extractEntry(Entry entry, EntryDesc &entryDesc)
{
    if (device.isOpen() && device.isReadable()) {
        const qint64 offset = entry.value;
        if (entry.isValid() && offset >= 0) {
            const qint64 size = device.size();
            if (size > offset && device.seek(offset)) {
                QDataStream ds(&device);
                ds.setVersion(QDataStream::Qt_6_0);
                ds >> entryDesc;
                return true;
            }
        } else {
            qWarning("Entry not found id(%s), offset(%lld)", entry.key.constData(), entry.value);
        }
    } else {
        qWarning("Unable to open file for reading");
    }

    return false;
}

static const char *borderText() { return "--------------------------------------------------------------------------------"; }

void QQsbIODeviceCollection::dumpInfo()
{
    if (map(QQsbIODeviceCollection::Read)) {
        qDebug("Number of entries in collection: %zu\n", size_t(entries.size()));
        int i = 0;
        qDebug("Qsbc version: %u", version);
        for (const auto &e : std::as_const(entries)) {
            qDebug("%s\n"
                   "Entry %d\n%s\n"
                   "Key: %s\n"
                   "Offset: %llu", borderText(), i++, borderText(), e.key.constData(), e.value);

            QQsbCollection::EntryDesc ed;
            if (extractEntry(e, ed)) {
                qDebug() << ed.materialKey << Qt::endl
                         << ed.featureSet << Qt::endl
                         << ed.vertShader << Qt::endl
                         << ed.fragShader;
            } else {
                qWarning("Extracting Qsb entry failed!");
            }
        }
    }
    unmap();
}

void QQsbIODeviceCollection::dumpInfo(const QString &file)
{
    QQsbIODeviceCollection qsbc(file);
    qsbc.dumpInfo();
}

void QQsbIODeviceCollection::dumpInfo(QIODevice &device)
{
    QQsbIODeviceCollection qsbc(device);
    qsbc.dumpInfo();
}

QT_END_NAMESPACE
