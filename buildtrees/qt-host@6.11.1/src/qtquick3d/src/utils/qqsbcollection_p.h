// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QQSBCOLLECTION_H
#define QQSBCOLLECTION_H

#include <QtQuick3DUtils/private/qtquick3dutilsglobal_p.h>

#include <QtCore/qfile.h>
#include <QtCore/qset.h>
#include <QtCore/qmap.h>

#include <rhi/qshader.h>

QT_BEGIN_NAMESPACE

class QRhiShaderStage;

class Q_QUICK3DUTILS_EXPORT QQsbCollection
{
public:
    virtual ~QQsbCollection();

    struct Q_QUICK3DUTILS_EXPORT Entry
    {
        // 'value' is optional. hashing and comparison are based solely on 'key'.
        Entry() = default;
        explicit Entry(const QByteArray &key) : key(key)
        {
            hashKey = qHash(key);
        }
        Entry(const QByteArray &key, qint64 value) : key(key), value(value)
        {
            hashKey = qHash(key);
        }
        bool isValid() const { return !key.isEmpty(); }
        QByteArray key;
        qint64 value = -1;
        size_t hashKey;
    };

    using FeatureSet = QMap<QByteArray, bool>; // QMap so it is ordered by key

    template<typename T>
    static FeatureSet toFeatureSet(const T &ssgFeatureSet)
    {
        FeatureSet ret;
        for (quint32 i = 0, end = T::Count; i != end; ++i) {
            auto def = T::fromIndex(i);
            if (ssgFeatureSet.isSet(def))
                ret.insert(T::asDefineString(def), true);
        }
        return ret;
    }

    struct Q_QUICK3DUTILS_EXPORT EntryDesc {
        QByteArray materialKey;
        FeatureSet featureSet;
        QShader vertShader;
        QShader fragShader;
        QByteArray generateSha() const;
        static QByteArray generateSha(const QByteArray &materialKey, const FeatureSet &featureSet);
    };

    using EntryMap = QSet<Entry>;
    virtual EntryMap availableEntries() const = 0;
    virtual Entry addEntry(const QByteArray &key, const EntryDesc &entryDesc) = 0;
    virtual bool extractEntry(Entry entry, EntryDesc &entryDesc) = 0;

protected:
    enum Version : quint8
    {
        Unknown,
        One = 0x10,
        Two = 0x20
    };
    bool readEndHeader(QDataStream &ds, qint64 *startPos, quint8 *version);
    void writeEndHeader(QDataStream &ds, qint64 startPos, quint8 version, quint64 magic);
    bool readEndHeader(QIODevice *device, EntryMap *entries, quint8 *version);
    void writeEndHeader(QIODevice *device, const EntryMap &entries);
};

Q_DECLARE_TYPEINFO(QQsbCollection::Entry, Q_PRIMITIVE_TYPE);

Q_QUICK3DUTILS_EXPORT QDataStream &operator<<(QDataStream &stream, const QQsbCollection::Entry &entry);
Q_QUICK3DUTILS_EXPORT QDataStream &operator>>(QDataStream &stream, QQsbCollection::Entry &entry);
Q_QUICK3DUTILS_EXPORT QDataStream &operator<<(QDataStream &stream, const QQsbCollection::EntryDesc &entryDesc);
Q_QUICK3DUTILS_EXPORT QDataStream &operator>>(QDataStream &stream, QQsbCollection::EntryDesc &entryDesc);

Q_QUICK3DUTILS_EXPORT size_t qHash(const QQsbCollection::Entry &entry, size_t);
Q_QUICK3DUTILS_EXPORT bool operator==(const QQsbCollection::Entry &l, const QQsbCollection::Entry &r);

// Simple implementation backed by a hash table. Save and load are explicit and
// all data is read and written. The file format is compatible with other
// implementations.
class Q_QUICK3DUTILS_EXPORT QQsbInMemoryCollection : public QQsbCollection
{
public:
    QQsbInMemoryCollection() = default;

    EntryMap availableEntries() const override;
    Entry addEntry(const QByteArray &key, const EntryDesc &entryDesc) override;
    bool extractEntry(Entry entry, EntryDesc &entryDesc) override;

    void clear();

    bool load(const QString &filename);
    bool save(const QString &filename);

private:
    Q_DISABLE_COPY(QQsbInMemoryCollection);

    QHash<Entry, EntryDesc> entries;
};

// Serial, direct-to/from-QIODevice implementation.
class Q_QUICK3DUTILS_EXPORT QQsbIODeviceCollection : public QQsbCollection
{
public:
    enum MapMode
    {
        Read = QIODevice::ReadOnly,
        Write = (QIODevice::WriteOnly | QIODevice::Truncate)
    };

    explicit QQsbIODeviceCollection(const QString &filePath);
    explicit QQsbIODeviceCollection(QIODevice &dev);
    ~QQsbIODeviceCollection();

    bool map(MapMode mode);
    void unmap();

    EntryMap availableEntries() const override;
    Entry addEntry(const QByteArray &key, const EntryDesc &entryDesc) override;
    bool extractEntry(Entry entry, EntryDesc &entryDesc) override;

    void dumpInfo();
    static void dumpInfo(const QString &device);
    static void dumpInfo(QIODevice &device);

private:
    Q_DISABLE_COPY(QQsbIODeviceCollection);

    enum class DeviceOwner : quint8
    {
        Self,
        Extern
    };
    QFile file;
    QIODevice &device;
    DeviceOwner devOwner = DeviceOwner::Self;
    quint8 version = Version::Unknown;
    EntryMap entries;
};

QT_END_NAMESPACE

#endif // QQSBCOLLECTION_H
