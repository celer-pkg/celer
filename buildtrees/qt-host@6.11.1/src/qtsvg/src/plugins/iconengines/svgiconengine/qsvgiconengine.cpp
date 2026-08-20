// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qsvgiconengine.h"

#ifndef QT_NO_SVGRENDERER

#include "qpainter.h"
#include "qpixmap.h"
#include "qsvgrenderer.h"
#include "qpixmapcache.h"
#include "qfileinfo.h"
#if QT_CONFIG(mimetype)
#include <qmimedatabase.h>
#include <qmimetype.h>
#endif
#include <QAtomicInt>
#include "qdebug.h"
#include <private/qguiapplication_p.h>
#include <private/qhexstring_p.h>

QT_BEGIN_NAMESPACE

class QSvgIconEnginePrivate : public QSharedData
{
public:
    QSvgIconEnginePrivate()
    {
        stepSerialNum();
    }

    static int hashKey(QIcon::Mode mode, QIcon::State state)
    {
        return ((mode << 4) | state);
    }

    QString pmcKey(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale) const
    {
        return QLatin1String("$qt_svgicon_")
                % HexString<int>(serialNum)
                % HexString<qint8>(mode)
                % HexString<qint8>(state)
                % HexString<int>(size.width())
                % HexString<int>(size.height())
                % HexString<qint16>(qRound(scale * 1000));
    }

    void stepSerialNum()
    {
        serialNum = lastSerialNum.fetchAndAddRelaxed(1);
    }

    bool tryLoad(QSvgRenderer *renderer, QIcon::Mode mode, QIcon::State state);
    QIcon::Mode loadDataForModeAndState(QSvgRenderer *renderer, QIcon::Mode mode, QIcon::State state);

    QHash<int, QString> svgFiles;
    QHash<int, QByteArray> svgBuffers;
    QMultiHash<int, QPixmap> addedPixmaps;
    int serialNum = 0;
    static QAtomicInt lastSerialNum;
};

QAtomicInt QSvgIconEnginePrivate::lastSerialNum;

QSvgIconEngine::QSvgIconEngine()
    : d(new QSvgIconEnginePrivate)
{
}

QSvgIconEngine::QSvgIconEngine(const QSvgIconEngine &other)
    : QIconEngine(other), d(new QSvgIconEnginePrivate)
{
    d->svgFiles = other.d->svgFiles;
    d->svgBuffers = other.d->svgBuffers;
    d->addedPixmaps = other.d->addedPixmaps;
}


QSvgIconEngine::~QSvgIconEngine()
{
}


QSize QSvgIconEngine::actualSize(const QSize &size, QIcon::Mode mode,
                                 QIcon::State state)
{
    if (!d->addedPixmaps.isEmpty()) {
        const auto key = d->hashKey(mode, state);
        auto it = d->addedPixmaps.constFind(key);
        while (it != d->addedPixmaps.end() && it.key() == key) {
            const auto &pm = it.value();
            if (!pm.isNull() && pm.size() == size)
                return size;
            ++it;
        }
    }

    QPixmap pm = pixmap(size, mode, state);
    if (pm.isNull())
        return QSize();
    return pm.size();
}

static inline QByteArray maybeUncompress(const QByteArray &ba)
{
#ifndef QT_NO_COMPRESS
    return qUncompress(ba);
#else
    return ba;
#endif
}

bool QSvgIconEnginePrivate::tryLoad(QSvgRenderer *renderer, QIcon::Mode mode, QIcon::State state)
{
    const auto key = hashKey(mode, state);
    QByteArray buf = svgBuffers.value(key);
    if (!buf.isEmpty()) {
        if (renderer->load(maybeUncompress(buf)))
            return true;
        svgBuffers.remove(key);
    }
    QString svgFile = svgFiles.value(key);
    if (!svgFile.isEmpty()) {
        if (renderer->load(svgFile))
            return true;
    }
    return false;
}

QIcon::Mode QSvgIconEnginePrivate::loadDataForModeAndState(QSvgRenderer *renderer, QIcon::Mode mode, QIcon::State state)
{
    if (tryLoad(renderer, mode, state))
        return mode;

    const QIcon::State oppositeState = (state == QIcon::On) ? QIcon::Off : QIcon::On;
    if (mode == QIcon::Disabled || mode == QIcon::Selected) {
        const QIcon::Mode oppositeMode = (mode == QIcon::Disabled) ? QIcon::Selected : QIcon::Disabled;
        if (tryLoad(renderer, QIcon::Normal, state))
            return QIcon::Normal;
        if (tryLoad(renderer, QIcon::Active, state))
            return QIcon::Active;
        if (tryLoad(renderer, mode, oppositeState))
            return mode;
        if (tryLoad(renderer, QIcon::Normal, oppositeState))
            return QIcon::Normal;
        if (tryLoad(renderer, QIcon::Active, oppositeState))
            return QIcon::Active;
        if (tryLoad(renderer, oppositeMode, state))
            return oppositeMode;
        if (tryLoad(renderer, oppositeMode, oppositeState))
            return oppositeMode;
    } else {
        const QIcon::Mode oppositeMode = (mode == QIcon::Normal) ? QIcon::Active : QIcon::Normal;
        if (tryLoad(renderer, oppositeMode, state))
            return oppositeMode;
        if (tryLoad(renderer, mode, oppositeState))
            return mode;
        if (tryLoad(renderer, oppositeMode, oppositeState))
            return oppositeMode;
        if (tryLoad(renderer, QIcon::Disabled, state))
            return QIcon::Disabled;
        if (tryLoad(renderer, QIcon::Selected, state))
            return QIcon::Selected;
        if (tryLoad(renderer, QIcon::Disabled, oppositeState))
            return QIcon::Disabled;
        if (tryLoad(renderer, QIcon::Selected, oppositeState))
            return QIcon::Selected;
    }
    return QIcon::Normal;
}

QPixmap QSvgIconEngine::pixmap(const QSize &size, QIcon::Mode mode,
                               QIcon::State state)
{
    return scaledPixmap(size, mode, state, 1.0);
}

QPixmap QSvgIconEngine::scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state,
                                     qreal scale)
{
    QPixmap pm;

    QString pmckey(d->pmcKey(size, mode, state, scale));
    if (QPixmapCache::find(pmckey, &pm))
        return pm;

    if (!d->addedPixmaps.isEmpty()) {
        const auto realSize = size * scale;
        const auto key = d->hashKey(mode, state);
        auto it = d->addedPixmaps.constFind(key);
        while (it != d->addedPixmaps.end() && it.key() == key) {
            const auto &pm = it.value();
            if (!pm.isNull()) {
                // we don't care about dpr here - don't use QSvgIconEngine when
                // there are a lot of raster images are to handle.
                if (pm.size() == realSize)
                    return pm;
            }
            ++it;
        }
    }

    QSvgRenderer renderer;
    const QIcon::Mode loadmode = d->loadDataForModeAndState(&renderer, mode, state);
    if (!renderer.isValid())
        return pm;

    QSize actualSize = renderer.defaultSize();
    if (!actualSize.isNull())
        actualSize.scale(size * scale, Qt::KeepAspectRatio);

    if (actualSize.isEmpty())
        return pm;

    pm = QPixmap(actualSize);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    renderer.render(&p);
    p.end();
    if (qobject_cast<QGuiApplication *>(QCoreApplication::instance())) {
        if (loadmode != mode && mode != QIcon::Normal) {
            const QPixmap generated = QGuiApplicationPrivate::instance()->applyQIconStyleHelper(mode, pm);
            if (!generated.isNull())
                pm = generated;
        }
    }

    if (!pm.isNull()) {
        pm.setDevicePixelRatio(scale);
        QPixmapCache::insert(pmckey, pm);
    }

    return pm;
}


void QSvgIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode,
                               QIcon::State state)
{
    d->stepSerialNum();
    d->addedPixmaps.insert(d->hashKey(mode, state), pixmap);
}

enum FileType { OtherFile, SvgFile, CompressedSvgFile };

static FileType fileType(const QFileInfo &fi)
{
    const QString &suffix = fi.completeSuffix();
    if (suffix.endsWith(QLatin1String("svg"), Qt::CaseInsensitive))
        return SvgFile;
    if (suffix.endsWith(QLatin1String("svgz"), Qt::CaseInsensitive)
        || suffix.endsWith(QLatin1String("svg.gz"), Qt::CaseInsensitive)) {
        return CompressedSvgFile;
    }
#if QT_CONFIG(mimetype)
    const QString &mimeTypeName = QMimeDatabase().mimeTypeForFile(fi).name();
    if (mimeTypeName == QLatin1String("image/svg+xml"))
        return SvgFile;
    if (mimeTypeName == QLatin1String("image/svg+xml-compressed"))
        return CompressedSvgFile;
#endif
    return OtherFile;
}

void QSvgIconEngine::addFile(const QString &fileName, const QSize &,
                             QIcon::Mode mode, QIcon::State state)
{
    if (!fileName.isEmpty()) {
         const QFileInfo fi(fileName);
         const QString abs = fi.absoluteFilePath();
         const FileType type = fileType(fi);
#ifndef QT_NO_COMPRESS
         if (type == SvgFile || type == CompressedSvgFile) {
#else
         if (type == SvgFile) {
#endif
             QSvgRenderer renderer(abs);
             if (renderer.isValid()) {
                 d->stepSerialNum();
                 d->svgFiles.insert(d->hashKey(mode, state), abs);
             }
         } else if (type == OtherFile) {
             QPixmap pm(abs);
             if (!pm.isNull())
                 addPixmap(pm, mode, state);
         }
    }
}

void QSvgIconEngine::paint(QPainter *painter, const QRect &rect,
                           QIcon::Mode mode, QIcon::State state)
{
    QSize pixmapSize = rect.size();
    if (painter->device())
        pixmapSize *= painter->device()->devicePixelRatio();
    painter->drawPixmap(rect, pixmap(pixmapSize, mode, state));
}

bool QSvgIconEngine::isNull()
{
    return d->svgFiles.isEmpty() && d->addedPixmaps.isEmpty() && d->svgBuffers.isEmpty();
}

QString QSvgIconEngine::key() const
{
    return QLatin1String("svg");
}

QIconEngine *QSvgIconEngine::clone() const
{
    return new QSvgIconEngine(*this);
}


bool QSvgIconEngine::read(QDataStream &in)
{
    d = new QSvgIconEnginePrivate;

    if (in.version() >= QDataStream::Qt_4_4) {
        int isCompressed;
        QHash<int, QString> fileNames;  // For memoryoptimization later
        in >> fileNames >> isCompressed >> d->svgBuffers;
#ifndef QT_NO_COMPRESS
        if (!isCompressed) {
            for (auto &svgBuf : d->svgBuffers)
                svgBuf = qCompress(svgBuf);
        }
#else
        if (isCompressed) {
            qWarning("QSvgIconEngine: Can not decompress SVG data");
            d->svgBuffers.clear();
        }
#endif
        int hasAddedPixmaps;
        in >> hasAddedPixmaps;
        if (hasAddedPixmaps) {
            in >> d->addedPixmaps;
        }
    }
    else {
        QPixmap pixmap;
        QByteArray data;
        uint mode;
        uint state;
        int num_entries;

        in >> data;
        if (!data.isEmpty()) {
#ifndef QT_NO_COMPRESS
            data = qUncompress(data);
#endif
            if (!data.isEmpty())
                d->svgBuffers.insert(d->hashKey(QIcon::Normal, QIcon::Off), data);
        }
        in >> num_entries;
        for (int i=0; i<num_entries; ++i) {
            if (in.atEnd())
                return false;
            in >> pixmap;
            in >> mode;
            in >> state;
            // The pm list written by 4.3 is buggy and/or useless, so ignore.
            //addPixmap(pixmap, QIcon::Mode(mode), QIcon::State(state));
        }
    }

    return true;
}


bool QSvgIconEngine::write(QDataStream &out) const
{
    if (out.version() >= QDataStream::Qt_4_4) {
        int isCompressed = 0;
#ifndef QT_NO_COMPRESS
        isCompressed = 1;
#endif
        QHash<int, QByteArray> svgBuffers = d->svgBuffers;
        for (const auto &it : d->svgFiles.asKeyValueRange()) {
            QByteArray buf;
            QFile f(it.second);
            if (f.open(QIODevice::ReadOnly))
                buf = f.readAll();
#ifndef QT_NO_COMPRESS
            buf = qCompress(buf);
#endif
            svgBuffers.insert(it.first, buf);
        }
        out << d->svgFiles << isCompressed << svgBuffers;
        if (d->addedPixmaps.isEmpty())
            out << 0;
        else
            out << 1 << d->addedPixmaps;
    }
    else {
        const auto key = d->hashKey(QIcon::Normal, QIcon::Off);
        QByteArray buf = d->svgBuffers.value(key);
        if (buf.isEmpty()) {
            QString svgFile = d->svgFiles.value(key);
            if (!svgFile.isEmpty()) {
                QFile f(svgFile);
                if (f.open(QIODevice::ReadOnly))
                    buf = f.readAll();
            }
        }
#ifndef QT_NO_COMPRESS
        buf = qCompress(buf);
#endif
        out << buf;
        // 4.3 has buggy handling of added pixmaps, so don't write any
        out << (int)0;
    }
    return true;
}

QT_END_NAMESPACE

#endif // QT_NO_SVGRENDERER
