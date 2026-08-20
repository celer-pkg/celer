// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qtiffhandler_p.h"

#include <qcolorspace.h>
#include <qdebug.h>
#include <qfloat16.h>
#include <qimage.h>
#include <qloggingcategory.h>
#include <qvariant.h>
#include <qvarlengtharray.h>
#include <qbuffer.h>
#include <qfiledevice.h>
#include <qimagereader.h>

extern "C" {
#include "tiffio.h"
}

#include <memory>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcTiff, "qt.imageformats.tiff")

tsize_t qtiffReadProc(thandle_t fd, tdata_t buf, tsize_t size)
{
    QIODevice *device = static_cast<QIODevice *>(fd);
    return device->isReadable() ? device->read(static_cast<char *>(buf), size) : -1;
}

tsize_t qtiffWriteProc(thandle_t fd, tdata_t buf, tsize_t size)
{
    return static_cast<QIODevice *>(fd)->write(static_cast<char *>(buf), size);
}

toff_t qtiffSeekProc(thandle_t fd, toff_t off, int whence)
{
    QIODevice *device = static_cast<QIODevice *>(fd);
    switch (whence) {
    case SEEK_SET:
        device->seek(off);
        break;
    case SEEK_CUR:
        device->seek(device->pos() + off);
        break;
    case SEEK_END:
        device->seek(device->size() + off);
        break;
    }

    return device->pos();
}

int qtiffCloseProc(thandle_t /*fd*/)
{
    return 0;
}

toff_t qtiffSizeProc(thandle_t fd)
{
    return static_cast<QIODevice *>(fd)->size();
}

int qtiffMapProc(thandle_t fd, void **base, toff_t *size)
{
    QIODevice *device = static_cast<QIODevice *>(fd);

    QFileDevice *file = qobject_cast<QFileDevice *>(device);
    if (file) {
        *base = file->map(0, file->size());
        if (*base != nullptr) {
            *size = file->size();
            return 1;
        }
    } else {
        QBuffer *buf = qobject_cast<QBuffer *>(device);
        if (buf) {
            *base = const_cast<char *>(buf->data().constData());
            *size = buf->size();
            return 1;
        }
    }
    return 0;
}

void qtiffUnmapProc(thandle_t fd, void *base, toff_t /*size*/)
{
    QFileDevice *file = qobject_cast<QFileDevice *>(static_cast<QIODevice *>(fd));
    if (file && base)
        file->unmap(static_cast<uchar *>(base));
}


class QTiffHandlerPrivate
{
public:
    QTiffHandlerPrivate();
    ~QTiffHandlerPrivate();

    static bool canRead(QIODevice *device);
    bool openForRead(QIODevice *device);
    bool readHeaders(QIODevice *device);
    void close();
    TIFF *openInternal(const char *mode, QIODevice *device);
#if TIFFLIB_VERSION >= 20221213
    static int tiffErrorHandler(TIFF *tif, void *user_data, const char *,
                                const char *fmt, va_list ap);
    static int tiffWarningHandler(TIFF *tif, void *user_data, const char *,
                                  const char *fmt, va_list ap);
#endif

    TIFF *tiff;
    int compression;
    QImageIOHandler::Transformations transformation;
    QImage::Format format;
    QSize size;
    uint16_t photometric;
    bool grayscale;
    bool floatingPoint;
    bool headersRead;
    int currentDirectory;
    int directoryCount;
};

static QImageIOHandler::Transformations exif2Qt(int exifOrientation)
{
    switch (exifOrientation) {
    case 1: // normal
        return QImageIOHandler::TransformationNone;
    case 2: // mirror horizontal
        return QImageIOHandler::TransformationMirror;
    case 3: // rotate 180
        return QImageIOHandler::TransformationRotate180;
    case 4: // mirror vertical
        return QImageIOHandler::TransformationFlip;
    case 5: // mirror horizontal and rotate 270 CW
        return QImageIOHandler::TransformationFlipAndRotate90;
    case 6: // rotate 90 CW
        return QImageIOHandler::TransformationRotate90;
    case 7: // mirror horizontal and rotate 90 CW
        return QImageIOHandler::TransformationMirrorAndRotate90;
    case 8: // rotate 270 CW
        return QImageIOHandler::TransformationRotate270;
    }
    qCWarning(lcTiff, "Invalid EXIF orientation");
    return QImageIOHandler::TransformationNone;
}

static int qt2Exif(QImageIOHandler::Transformations transformation)
{
    switch (transformation) {
    case QImageIOHandler::TransformationNone:
        return 1;
    case QImageIOHandler::TransformationMirror:
        return 2;
    case QImageIOHandler::TransformationRotate180:
        return 3;
    case QImageIOHandler::TransformationFlip:
        return 4;
    case QImageIOHandler::TransformationFlipAndRotate90:
        return 5;
    case QImageIOHandler::TransformationRotate90:
        return 6;
    case QImageIOHandler::TransformationMirrorAndRotate90:
        return 7;
    case QImageIOHandler::TransformationRotate270:
        return 8;
    }
    qCWarning(lcTiff, "Invalid Qt image transformation");
    return 1;
}

QTiffHandlerPrivate::QTiffHandlerPrivate()
    : tiff(0)
    , compression(QTiffHandler::NoCompression)
    , transformation(QImageIOHandler::TransformationNone)
    , format(QImage::Format_Invalid)
    , photometric(false)
    , grayscale(false)
    , headersRead(false)
    , currentDirectory(0)
    , directoryCount(0)
{
}

QTiffHandlerPrivate::~QTiffHandlerPrivate()
{
    close();
}

void QTiffHandlerPrivate::close()
{
    if (tiff)
        TIFFClose(tiff);
    tiff = 0;
}

TIFF *QTiffHandlerPrivate::openInternal(const char *mode, QIODevice *device)
{
// TIFFLIB_VERSION 20221213 -> 4.5.0
#if TIFFLIB_VERSION >= 20221213
    TIFFOpenOptions *opts = TIFFOpenOptionsAlloc();
    TIFFOpenOptionsSetErrorHandlerExtR(opts, &tiffErrorHandler, this);
    TIFFOpenOptionsSetWarningHandlerExtR(opts, &tiffWarningHandler, this);

#if TIFFLIB_AT_LEAST(4, 7, 0)
    quint64 maxAlloc = quint64(QImageReader::allocationLimit()) << 20;
    if (maxAlloc) {
        maxAlloc = qMin(maxAlloc, quint64(std::numeric_limits<tmsize_t>::max()));
        TIFFOpenOptionsSetMaxCumulatedMemAlloc(opts, tmsize_t(maxAlloc));
    }
#endif

    auto handle = TIFFClientOpenExt("foo",
                                    mode,
                                    device,
                                    qtiffReadProc,
                                    qtiffWriteProc,
                                    qtiffSeekProc,
                                    qtiffCloseProc,
                                    qtiffSizeProc,
                                    qtiffMapProc,
                                    qtiffUnmapProc,
                                    opts);
    TIFFOpenOptionsFree(opts);
#else
    auto handle = TIFFClientOpen("foo",
                                 mode,
                                 device,
                                 qtiffReadProc,
                                 qtiffWriteProc,
                                 qtiffSeekProc,
                                 qtiffCloseProc,
                                 qtiffSizeProc,
                                 qtiffMapProc,
                                 qtiffUnmapProc);
#endif
    return handle;
}


#if TIFFLIB_VERSION >= 20221213
int QTiffHandlerPrivate::tiffErrorHandler(TIFF *tif, void *user_data, const char *,
                                          const char *fmt, va_list ap)
{
    const auto priv = static_cast<QTiffHandlerPrivate *>(user_data);
    if (!priv || priv->tiff != tif)
        return 0;
    qCCritical(lcTiff) << QString::vasprintf(fmt, ap);
    return 1;
}

int QTiffHandlerPrivate::tiffWarningHandler(TIFF *tif, void *user_data, const char *,
                                            const char *fmt, va_list ap)
{
    const auto priv = static_cast<QTiffHandlerPrivate *>(user_data);
    if (!priv || priv->tiff != tif)
        return 0;
    qCWarning(lcTiff) << QString::vasprintf(fmt, ap);
    return 1;
}
#endif

bool QTiffHandlerPrivate::canRead(QIODevice *device)
{
    if (!device) {
        qCWarning(lcTiff, "QTiffHandler::canRead() called with no device");
        return false;
    }

    // current implementation uses TIFFClientOpen which needs to be
    // able to seek, so sequential devices are not supported
    char h[4];
    if (device->peek(h, 4) != 4)
        return false;
    if ((h[0] == 0x49 && h[1] == 0x49) && (h[2] == 0x2a || h[2] == 0x2b) && h[3] == 0)
        return true; // Little endian, classic or bigtiff
    if ((h[0] == 0x4d && h[1] == 0x4d) && h[2] == 0 && (h[3] == 0x2a || h[3] == 0x2b))
        return true; // Big endian, classic or bigtiff
    return false;
}

bool QTiffHandlerPrivate::openForRead(QIODevice *device)
{
    if (tiff)
        return true;

    if (!canRead(device))
        return false;

    tiff = openInternal("rh", device);
    return tiff != nullptr;
}

bool QTiffHandlerPrivate::readHeaders(QIODevice *device)
{
    if (headersRead)
        return true;

    if (!openForRead(device))
        return false;

    if (!TIFFSetDirectory(tiff, currentDirectory)) {
        close();
        return false;
    }

    uint32_t width;
    uint32_t height;
    if (!TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width)
        || !TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height)
        || !TIFFGetField(tiff, TIFFTAG_PHOTOMETRIC, &photometric)) {
        close();
        return false;
    }
    size = QSize(width, height);

    uint16_t orientationTag;
    if (TIFFGetField(tiff, TIFFTAG_ORIENTATION, &orientationTag))
        transformation = exif2Qt(orientationTag);

    // BitsPerSample defaults to 1 according to the TIFF spec.
    uint16_t bitPerSample;
    if (!TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bitPerSample))
        bitPerSample = 1;
    uint16_t samplesPerPixel; // they may be e.g. grayscale with 2 samples per pixel
    if (!TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel))
        samplesPerPixel = 1;
    uint16_t sampleFormat;
    if (!TIFFGetField(tiff, TIFFTAG_SAMPLEFORMAT, &sampleFormat))
        sampleFormat = SAMPLEFORMAT_VOID;
    floatingPoint = (sampleFormat == SAMPLEFORMAT_IEEEFP);

    grayscale = photometric == PHOTOMETRIC_MINISBLACK || photometric == PHOTOMETRIC_MINISWHITE;

    if (grayscale && bitPerSample == 1 && samplesPerPixel == 1)
        format = QImage::Format_Mono;
    else if (photometric == PHOTOMETRIC_MINISBLACK && bitPerSample == 8 && samplesPerPixel == 1)
        format = QImage::Format_Grayscale8;
    else if (photometric == PHOTOMETRIC_MINISBLACK && bitPerSample == 16 && samplesPerPixel == 1 && !floatingPoint)
        format = QImage::Format_Grayscale16;
    else if ((grayscale || photometric == PHOTOMETRIC_PALETTE) && bitPerSample == 8 && samplesPerPixel == 1)
        format = QImage::Format_Indexed8;
    else if (samplesPerPixel < 4) {
        bool regular = (samplesPerPixel != 2) && (photometric == PHOTOMETRIC_RGB || photometric == PHOTOMETRIC_MINISBLACK);
        if (bitPerSample == 16 && regular)
            format = floatingPoint ? QImage::Format_RGBX16FPx4 : QImage::Format_RGBX64;
        else if (bitPerSample == 32 && floatingPoint && regular)
            format = QImage::Format_RGBX32FPx4;
        else
            format = QImage::Format_RGB32;
    } else {
        uint16_t count;
        uint16_t *extrasamples;
        // If there is any definition of the alpha-channel, libtiff will return premultiplied
        // data to us. If there is none, libtiff will not touch it and  we assume it to be
        // non-premultiplied, matching behavior of tested image editors, and how older Qt
        // versions used to save it.
        bool premultiplied = true;
        bool gotField = TIFFGetField(tiff, TIFFTAG_EXTRASAMPLES, &count, &extrasamples);
        if (!gotField || !count || extrasamples[0] == EXTRASAMPLE_UNSPECIFIED)
            premultiplied = false;

        if (bitPerSample == 16 && photometric == PHOTOMETRIC_RGB) {
            // We read 64-bit raw, so unassoc remains unpremultiplied.
            if (gotField && count && extrasamples[0] == EXTRASAMPLE_UNASSALPHA)
                premultiplied = false;
            if (premultiplied)
                format = floatingPoint ? QImage::Format_RGBA16FPx4_Premultiplied : QImage::Format_RGBA64_Premultiplied;
            else
                format = floatingPoint ? QImage::Format_RGBA16FPx4 : QImage::Format_RGBA64;
        } else if (bitPerSample == 32 && floatingPoint && photometric == PHOTOMETRIC_RGB) {
            if (gotField && count && extrasamples[0] == EXTRASAMPLE_UNASSALPHA)
                premultiplied = false;
            if (premultiplied)
                format = QImage::Format_RGBA32FPx4_Premultiplied;
            else
                format = QImage::Format_RGBA32FPx4;
        } else if (samplesPerPixel == 4 && bitPerSample == 8 && photometric == PHOTOMETRIC_SEPARATED) {
            uint16_t inkSet;
            const bool gotInkSetField = TIFFGetField(tiff, TIFFTAG_INKSET, &inkSet);
            if (!gotInkSetField || inkSet == INKSET_CMYK) {
                format = QImage::Format_CMYK8888;
            } else {
                close();
                return false;
            }
        } else {
            if (premultiplied)
                format = QImage::Format_ARGB32_Premultiplied;
            else
                format = QImage::Format_ARGB32;
        }
    }

    headersRead = true;
    return true;
}

QTiffHandler::QTiffHandler()
    : QImageIOHandler()
    , d(new QTiffHandlerPrivate)
{
}

bool QTiffHandler::canRead() const
{
    if (d->tiff)
        return true;
    if (QTiffHandlerPrivate::canRead(device())) {
        setFormat("tiff");
        return true;
    }
    return false;
}

bool QTiffHandler::canRead(QIODevice *device)
{
    return QTiffHandlerPrivate::canRead(device);
}

bool QTiffHandler::read(QImage *image)
{
    // Open file and read headers if it hasn't already been done.
    if (!d->readHeaders(device()))
        return false;

    QImage::Format format = d->format;

    if (!QImageIOHandler::allocateImage(d->size, format, image)) {
        d->close();
        return false;
    }

    TIFF *const tiff = d->tiff;
    if (TIFFIsTiled(tiff) && TIFFTileSize64(tiff) > uint64_t(image->sizeInBytes())) // Corrupt image
        return false;
    const quint32 width = d->size.width();
    const quint32 height = d->size.height();

    // Setup color tables
    if (format == QImage::Format_Mono || format == QImage::Format_Indexed8) {
        if (format == QImage::Format_Mono) {
            QList<QRgb> colortable(2);
            if (d->photometric == PHOTOMETRIC_MINISBLACK) {
                colortable[0] = 0xff000000;
                colortable[1] = 0xffffffff;
            } else {
                colortable[0] = 0xffffffff;
                colortable[1] = 0xff000000;
            }
            image->setColorTable(colortable);
        } else if (format == QImage::Format_Indexed8) {
            const uint16_t tableSize = 256;
            QList<QRgb> qtColorTable(tableSize);
            if (d->grayscale) {
                for (int i = 0; i<tableSize; ++i) {
                    const int c = (d->photometric == PHOTOMETRIC_MINISBLACK) ? i : (255 - i);
                    qtColorTable[i] = qRgb(c, c, c);
                }
            } else {
                // create the color table
                uint16_t *redTable = 0;
                uint16_t *greenTable = 0;
                uint16_t *blueTable = 0;
                if (!TIFFGetField(tiff, TIFFTAG_COLORMAP, &redTable, &greenTable, &blueTable)) {
                    d->close();
                    return false;
                }
                if (!redTable || !greenTable || !blueTable) {
                    d->close();
                    return false;
                }

                for (int i = 0; i<tableSize ;++i) {
                    // emulate libtiff behavior for 16->8 bit color map conversion: just ignore the lower 8 bits
                    const int red = redTable[i] >> 8;
                    const int green = greenTable[i] >> 8;
                    const int blue = blueTable[i] >> 8;
                    qtColorTable[i] = qRgb(red, green, blue);
                }
            }
            image->setColorTable(qtColorTable);
            // free redTable, greenTable and greenTable done by libtiff
        }
    }
    bool format8bit = (format == QImage::Format_Mono || format == QImage::Format_Indexed8 || format == QImage::Format_Grayscale8);
    bool format16bit = (format == QImage::Format_Grayscale16);
    bool formatCmyk32bit = (format == QImage::Format_CMYK8888);
    bool format64bit = (format == QImage::Format_RGBX64 || format == QImage::Format_RGBA64 || format == QImage::Format_RGBA64_Premultiplied);
    bool format64fp = (format == QImage::Format_RGBX16FPx4 || format == QImage::Format_RGBA16FPx4 || format == QImage::Format_RGBA16FPx4_Premultiplied);
    bool format128fp = (format == QImage::Format_RGBX32FPx4 || format == QImage::Format_RGBA32FPx4 || format == QImage::Format_RGBA32FPx4_Premultiplied);

    // Formats we read directly, instead of over RGBA32:
    if (format8bit || format16bit || formatCmyk32bit || format64bit || format64fp || format128fp) {
        int bytesPerPixel = image->depth() / 8;
        if (format == QImage::Format_RGBX64 || format == QImage::Format_RGBX16FPx4)
            bytesPerPixel = d->photometric == PHOTOMETRIC_RGB ? 6 : 2;
        else if (format == QImage::Format_RGBX32FPx4)
            bytesPerPixel = d->photometric == PHOTOMETRIC_RGB ? 12 : 4;
        if (TIFFIsTiled(tiff)) {
            quint32 tileWidth, tileLength;
            TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &tileWidth);
            TIFFGetField(tiff, TIFFTAG_TILELENGTH, &tileLength);
            if (!tileWidth || !tileLength || tileWidth % 16 || tileLength % 16) {
                d->close();
                return false;
            }
            quint32 byteWidth = (format == QImage::Format_Mono) ? (width + 7)/8 : (width * bytesPerPixel);
            quint32 byteTileWidth = (format == QImage::Format_Mono) ? tileWidth/8 : (tileWidth * bytesPerPixel);
            tmsize_t byteTileSize = TIFFTileSize(tiff);
            if (byteTileSize > image->sizeInBytes() || byteTileSize / tileLength < byteTileWidth) {
                d->close();
                return false;
            }
            uchar *buf = (uchar *)_TIFFmalloc(byteTileSize);
            if (!buf) {
                d->close();
                return false;
            }
            for (quint32 y = 0; y < height; y += tileLength) {
                for (quint32 x = 0; x < width; x += tileWidth) {
                    if (TIFFReadTile(tiff, buf, x, y, 0, 0) < 0) {
                        _TIFFfree(buf);
                        d->close();
                        return false;
                    }
                    quint32 linesToCopy = qMin(tileLength, height - y);
                    quint32 byteOffset = (format == QImage::Format_Mono) ? x/8 : (x * bytesPerPixel);
                    quint32 widthToCopy = qMin(byteTileWidth, byteWidth - byteOffset);
                    for (quint32 i = 0; i < linesToCopy; i++) {
                        ::memcpy(image->scanLine(y + i) + byteOffset, buf + (i * byteTileWidth), widthToCopy);
                    }
                }
            }
            _TIFFfree(buf);
        } else {
            if (image->bytesPerLine() < TIFFScanlineSize(tiff)) {
                d->close();
                return false;
            }
            for (uint32_t y=0; y<height; ++y) {
                if (TIFFReadScanline(tiff, image->scanLine(y), y, 0) < 0) {
                    d->close();
                    return false;
                }
            }
        }
        if (format == QImage::Format_RGBX64 || format == QImage::Format_RGBX16FPx4) {
            if (d->photometric == PHOTOMETRIC_RGB)
                rgb48fixup(image, d->floatingPoint);
            else
                rgbFixup(image);
        } else if (format == QImage::Format_RGBX32FPx4) {
            if (d->photometric == PHOTOMETRIC_RGB)
                rgb96fixup(image);
            else
                rgbFixup(image);
        }
    } else {
        const int stopOnError = 1;
        if (TIFFReadRGBAImageOriented(tiff, width, height, reinterpret_cast<uint32_t *>(image->bits()), qt2Exif(d->transformation), stopOnError)) {
            for (uint32_t y=0; y<height; ++y)
                convert32BitOrder(image->scanLine(y), width);
        } else {
            d->close();
            return false;
        }
    }


    float resX = 0;
    float resY = 0;
    uint16_t resUnit;
    if (!TIFFGetField(tiff, TIFFTAG_RESOLUTIONUNIT, &resUnit))
        resUnit = RESUNIT_INCH;

    if (TIFFGetField(tiff, TIFFTAG_XRESOLUTION, &resX)
        && TIFFGetField(tiff, TIFFTAG_YRESOLUTION, &resY)) {

        switch(resUnit) {
        case RESUNIT_CENTIMETER:
            image->setDotsPerMeterX(qRound(resX * 100));
            image->setDotsPerMeterY(qRound(resY * 100));
            break;
        case RESUNIT_INCH:
            image->setDotsPerMeterX(qRound(resX * (100 / 2.54)));
            image->setDotsPerMeterY(qRound(resY * (100 / 2.54)));
            break;
        default:
            // do nothing as defaults have already
            // been set within the QImage class
            break;
        }
    }

    uint32_t count;
    void *profile;
    if (TIFFGetField(tiff, TIFFTAG_ICCPROFILE, &count, &profile)) {
        QByteArray iccProfile(reinterpret_cast<const char *>(profile), count);
        image->setColorSpace(QColorSpace::fromIccProfile(iccProfile));
    }
    // We do not handle colorimetric metadat not on ICC profile form, it seems to be a lot
    // less common, and would need additional API in QColorSpace.

    return true;
}

static bool checkGrayscale(const QList<QRgb> &colorTable)
{
    if (colorTable.size() != 256)
        return false;

    const bool increasing = (colorTable.at(0) == 0xff000000);
    for (int i = 0; i < 256; ++i) {
        if ((increasing && colorTable.at(i) != qRgb(i, i, i))
            || (!increasing && colorTable.at(i) != qRgb(255 - i, 255 - i, 255 - i)))
            return false;
    }
    return true;
}

static QList<QRgb> effectiveColorTable(const QImage &image)
{
    QList<QRgb> colors;
    switch (image.format()) {
    case QImage::Format_Indexed8:
        colors = image.colorTable();
        break;
    case QImage::Format_Alpha8:
        colors.resize(256);
        for (int i = 0; i < 256; ++i)
            colors[i] = qRgba(0, 0, 0, i);
        break;
    case QImage::Format_Grayscale8:
    case QImage::Format_Grayscale16:
        colors.resize(256);
        for (int i = 0; i < 256; ++i)
            colors[i] = qRgb(i, i, i);
        break;
    default:
        Q_UNREACHABLE();
    }
    return colors;
}

static quint32 defaultStripSize(TIFF *tiff)
{
    // Aim for 4MB strips
    qint64 scanSize = qMax(qint64(1), qint64(TIFFScanlineSize(tiff)));
    qint64 numRows = (4 * 1024 * 1024) / scanSize;
    quint32 reqSize = static_cast<quint32>(qBound(qint64(1), numRows, qint64(UINT_MAX)));
    return TIFFDefaultStripSize(tiff, reqSize);
}

bool QTiffHandler::write(const QImage &image)
{
    if (!device()->isWritable())
        return false;

    TIFF *const tiff = d->openInternal("wB", device());
    if (!tiff)
        return false;

    const int width = image.width();
    const int height = image.height();
    const int compression = d->compression;

    if (!TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, width)
        || !TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, height)
        || !TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG)) {
        TIFFClose(tiff);
        return false;
    }

    // set the resolution
    bool  resolutionSet = false;
    const int dotPerMeterX = image.dotsPerMeterX();
    const int dotPerMeterY = image.dotsPerMeterY();
    if ((dotPerMeterX % 100) == 0
        && (dotPerMeterY % 100) == 0) {
        resolutionSet = TIFFSetField(tiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_CENTIMETER)
                        && TIFFSetField(tiff, TIFFTAG_XRESOLUTION, dotPerMeterX/100.0)
                        && TIFFSetField(tiff, TIFFTAG_YRESOLUTION, dotPerMeterY/100.0);
    } else {
        resolutionSet = TIFFSetField(tiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH)
                        && TIFFSetField(tiff, TIFFTAG_XRESOLUTION, static_cast<float>(image.logicalDpiX()))
                        && TIFFSetField(tiff, TIFFTAG_YRESOLUTION, static_cast<float>(image.logicalDpiY()));
    }
    if (!resolutionSet) {
        TIFFClose(tiff);
        return false;
    }
    // set the orienataion
    bool orientationSet = false;
    orientationSet = TIFFSetField(tiff, TIFFTAG_ORIENTATION, qt2Exif(d->transformation));
    if (!orientationSet) {
        TIFFClose(tiff);
        return false;
    }
    // set color space
    const QByteArray iccProfile = image.colorSpace().iccProfile();
    if (!iccProfile.isEmpty()) {
        if (!TIFFSetField(tiff, TIFFTAG_ICCPROFILE, iccProfile.size(), reinterpret_cast<const void *>(iccProfile.constData()))) {
            TIFFClose(tiff);
            return false;
        }
    }
    // configure image depth
    const QImage::Format format = image.format();
    if (format == QImage::Format_Mono || format == QImage::Format_MonoLSB) {
        uint16_t photometric = PHOTOMETRIC_MINISBLACK;
        if (image.colorTable().at(0) == 0xffffffff)
            photometric = PHOTOMETRIC_MINISWHITE;
        if (!TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, photometric)
            || !TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression == NoCompression ? COMPRESSION_NONE : COMPRESSION_LZW)
            || !TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 1)
            || !TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, defaultStripSize(tiff))) {
            TIFFClose(tiff);
            return false;
        }

        // try to do the conversion in chunks no greater than 16 MB
        const int chunks = int(image.sizeInBytes() / (1024 * 1024 * 16)) + 1;
        const int chunkHeight = qMax(height / chunks, 1);

        int y = 0;
        while (y < height) {
            QImage chunk = image.copy(0, y, width, qMin(chunkHeight, height - y)).convertToFormat(QImage::Format_Mono);

            int chunkStart = y;
            int chunkEnd = y + chunk.height();
            while (y < chunkEnd) {
                if (TIFFWriteScanline(tiff, reinterpret_cast<uint32_t *>(chunk.scanLine(y - chunkStart)), y) != 1) {
                    TIFFClose(tiff);
                    return false;
                }
                ++y;
            }
        }
        TIFFClose(tiff);
    } else if (format == QImage::Format_Indexed8
               || format == QImage::Format_Grayscale8
               || format == QImage::Format_Grayscale16
               || format == QImage::Format_Alpha8) {
        QList<QRgb> colorTable = effectiveColorTable(image);
        bool isGrayscale = checkGrayscale(colorTable);
        if (isGrayscale) {
            uint16_t photometric = PHOTOMETRIC_MINISBLACK;
            if (colorTable.at(0) == 0xffffffff)
                photometric = PHOTOMETRIC_MINISWHITE;
            if (!TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, photometric)
                    || !TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression == NoCompression ? COMPRESSION_NONE : COMPRESSION_LZW)
                    || !TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, image.depth())
                    || !TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT)
                    || !TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, defaultStripSize(tiff))) {
                TIFFClose(tiff);
                return false;
            }
        } else {
            if (!TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE)
                    || !TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression == NoCompression ? COMPRESSION_NONE : COMPRESSION_LZW)
                    || !TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8)
                    || !TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, defaultStripSize(tiff))) {
                TIFFClose(tiff);
                return false;
            }
            //// write the color table
            // allocate the color tables
            const int tableSize = colorTable.size();
            Q_ASSERT(tableSize <= 256);
            QVarLengthArray<uint16_t> redTable(tableSize);
            QVarLengthArray<uint16_t> greenTable(tableSize);
            QVarLengthArray<uint16_t> blueTable(tableSize);

            // set the color table
            for (int i = 0; i<tableSize; ++i) {
                const QRgb color = colorTable.at(i);
                redTable[i] = qRed(color) * 257;
                greenTable[i] = qGreen(color) * 257;
                blueTable[i] = qBlue(color) * 257;
            }

            const bool setColorTableSuccess = TIFFSetField(tiff, TIFFTAG_COLORMAP, redTable.data(), greenTable.data(), blueTable.data());

            if (!setColorTableSuccess) {
                TIFFClose(tiff);
                return false;
            }
        }

        //// write the data
        for (int y = 0; y < height; ++y) {
            if (TIFFWriteScanline(tiff, const_cast<uchar *>(image.scanLine(y)), y) != 1) {
                TIFFClose(tiff);
                return false;
            }
        }
        TIFFClose(tiff);
    } else if (format == QImage::Format_RGBX64 || format == QImage::Format_RGBX16FPx4) {
        if (!TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB)
            || !TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression == NoCompression ? COMPRESSION_NONE : COMPRESSION_LZW)
            || !TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 3)
            || !TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 16)
            || !TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT,
                             format == QImage::Format_RGBX64
                                ? SAMPLEFORMAT_UINT
                                : SAMPLEFORMAT_IEEEFP)
            || !TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tiff, 0))) {
            TIFFClose(tiff);
            return false;
        }
        std::unique_ptr<quint16[]> rgb48line(new quint16[width * 3]);
        for (int y = 0; y < height; ++y) {
            const quint16 *srcLine = reinterpret_cast<const quint16 *>(image.constScanLine(y));
            for (int x = 0; x < width; ++x) {
                rgb48line[x * 3 + 0] = srcLine[x * 4 + 0];
                rgb48line[x * 3 + 1] = srcLine[x * 4 + 1];
                rgb48line[x * 3 + 2] = srcLine[x * 4 + 2];
            }

            if (TIFFWriteScanline(tiff, (void*)rgb48line.get(), y) != 1) {
                TIFFClose(tiff);
                return false;
            }
        }
        TIFFClose(tiff);
    } else if (format == QImage::Format_RGBA64
               || format == QImage::Format_RGBA64_Premultiplied) {
        const bool premultiplied = image.format() != QImage::Format_RGBA64;
        const uint16_t extrasamples = premultiplied ? EXTRASAMPLE_ASSOCALPHA : EXTRASAMPLE_UNASSALPHA;
        if (!TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB)
            || !TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression == NoCompression ? COMPRESSION_NONE : COMPRESSION_LZW)
            || !TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 4)
            || !TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 16)
            || !TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT)
            || !TIFFSetField(tiff, TIFFTAG_EXTRASAMPLES, 1, &extrasamples)
            || !TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tiff, 0))) {
            TIFFClose(tiff);
            return false;
        }
        for (int y = 0; y < height; ++y) {
            if (TIFFWriteScanline(tiff, (void*)image.scanLine(y), y) != 1) {
                TIFFClose(tiff);
                return false;
            }
        }
        TIFFClose(tiff);
    } else if (format == QImage::Format_RGBX32FPx4) {
        if (!TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB)
            || !TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression == NoCompression ? COMPRESSION_NONE : COMPRESSION_LZW)
            || !TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 3)
            || !TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 32)
            || !TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP)
            || !TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tiff, 0))) {
            TIFFClose(tiff);
            return false;
        }
        std::unique_ptr<float[]> line(new float[width * 3]);
        for (int y = 0; y < height; ++y) {
            const float *srcLine = reinterpret_cast<const float *>(image.constScanLine(y));
            for (int x = 0; x < width; ++x) {
                line[x * 3 + 0] = srcLine[x * 4 + 0];
                line[x * 3 + 1] = srcLine[x * 4 + 1];
                line[x * 3 + 2] = srcLine[x * 4 + 2];
            }

            if (TIFFWriteScanline(tiff, (void*)line.get(), y) != 1) {
                TIFFClose(tiff);
                return false;
            }
        }
        TIFFClose(tiff);
    } else if (format == QImage::Format_RGBA16FPx4 || format == QImage::Format_RGBA32FPx4
               || format == QImage::Format_RGBA16FPx4_Premultiplied
               || format == QImage::Format_RGBA32FPx4_Premultiplied) {
        const bool premultiplied = image.format() != QImage::Format_RGBA16FPx4 && image.format() != QImage::Format_RGBA32FPx4;
        const uint16_t extrasamples = premultiplied ? EXTRASAMPLE_ASSOCALPHA : EXTRASAMPLE_UNASSALPHA;
        if (!TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB)
            || !TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression == NoCompression ? COMPRESSION_NONE : COMPRESSION_LZW)
            || !TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 4)
            || !TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, image.depth() == 64 ? 16 : 32)
            || !TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP)
            || !TIFFSetField(tiff, TIFFTAG_EXTRASAMPLES, 1, &extrasamples)
            || !TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tiff, 0))) {
            TIFFClose(tiff);
            return false;
        }
        for (int y = 0; y < height; ++y) {
            if (TIFFWriteScanline(tiff, (void*)image.scanLine(y), y) != 1) {
                TIFFClose(tiff);
                return false;
            }
        }
        TIFFClose(tiff);
    } else if (format == QImage::Format_CMYK8888) {
        if (!TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED)
            || !TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression == NoCompression ? COMPRESSION_NONE : COMPRESSION_LZW)
            || !TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 4)
            || !TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8)
            || !TIFFSetField(tiff, TIFFTAG_INKSET, INKSET_CMYK)
            || !TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, defaultStripSize(tiff))) {
            TIFFClose(tiff);
            return false;
        }

        for (int y = 0; y < image.height(); ++y) {
            if (TIFFWriteScanline(tiff, (void*)image.scanLine(y), y) != 1) {
                TIFFClose(tiff);
                return false;
            }
        }

        TIFFClose(tiff);
    } else if (!image.hasAlphaChannel()) {
        if (!TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB)
            || !TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression == NoCompression ? COMPRESSION_NONE : COMPRESSION_LZW)
            || !TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 3)
            || !TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8)
            || !TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, defaultStripSize(tiff))) {
            TIFFClose(tiff);
            return false;
        }
        // try to do the RGB888 conversion in chunks no greater than 16 MB
        const int chunks = int(image.sizeInBytes() / (1024 * 1024 * 16)) + 1;
        const int chunkHeight = qMax(height / chunks, 1);

        int y = 0;
        while (y < height) {
            const QImage chunk = image.copy(0, y, width, qMin(chunkHeight, height - y)).convertToFormat(QImage::Format_RGB888);

            int chunkStart = y;
            int chunkEnd = y + chunk.height();
            while (y < chunkEnd) {
                if (TIFFWriteScanline(tiff, (void*)chunk.scanLine(y - chunkStart), y) != 1) {
                    TIFFClose(tiff);
                    return false;
                }
                ++y;
            }
        }
        TIFFClose(tiff);
    } else {
        const bool premultiplied = image.format() != QImage::Format_ARGB32
                                && image.format() != QImage::Format_RGBA8888;
        const uint16_t extrasamples = premultiplied ? EXTRASAMPLE_ASSOCALPHA : EXTRASAMPLE_UNASSALPHA;
        if (!TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB)
            || !TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression == NoCompression ? COMPRESSION_NONE : COMPRESSION_LZW)
            || !TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 4)
            || !TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8)
            || !TIFFSetField(tiff, TIFFTAG_EXTRASAMPLES, 1, &extrasamples)
            || !TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, defaultStripSize(tiff))) {
            TIFFClose(tiff);
            return false;
        }
        // try to do the RGBA8888 conversion in chunks no greater than 16 MB
        const int chunks = int(image.sizeInBytes() / (1024 * 1024 * 16)) + 1;
        const int chunkHeight = qMax(height / chunks, 1);

        const QImage::Format format = premultiplied ? QImage::Format_RGBA8888_Premultiplied
                                                    : QImage::Format_RGBA8888;
        int y = 0;
        while (y < height) {
            const QImage chunk = image.copy(0, y, width, qMin(chunkHeight, height - y)).convertToFormat(format);

            int chunkStart = y;
            int chunkEnd = y + chunk.height();
            while (y < chunkEnd) {
                if (TIFFWriteScanline(tiff, (void*)chunk.scanLine(y - chunkStart), y) != 1) {
                    TIFFClose(tiff);
                    return false;
                }
                ++y;
            }
        }
        TIFFClose(tiff);
    }

    return true;
}

QVariant QTiffHandler::option(ImageOption option) const
{
    if (option == Size && canRead()) {
        if (d->readHeaders(device()))
            return d->size;
    } else if (option == CompressionRatio) {
        return d->compression;
    } else if (option == ImageFormat) {
        if (d->readHeaders(device()))
            return d->format;
    } else if (option == ImageTransformation) {
        if (d->readHeaders(device()))
            return int(d->transformation);
    }
    return QVariant();
}

void QTiffHandler::setOption(ImageOption option, const QVariant &value)
{
    if (option == CompressionRatio && value.metaType().id() == QMetaType::Int)
        d->compression = qBound(0, value.toInt(), 1);
    if (option == ImageTransformation) {
        int transformation = value.toInt();
        if (transformation > 0 && transformation < 8)
            d->transformation = QImageIOHandler::Transformations(transformation);
    }
}

bool QTiffHandler::supportsOption(ImageOption option) const
{
    return option == CompressionRatio
            || option == Size
            || option == ImageFormat
            || option == ImageTransformation;
}

bool QTiffHandler::jumpToNextImage()
{
    if (!ensureHaveDirectoryCount())
        return false;
    if (d->currentDirectory >= d->directoryCount - 1)
        return false;

    d->headersRead = false;
    ++d->currentDirectory;
    return true;
}

bool QTiffHandler::jumpToImage(int imageNumber)
{
    if (!ensureHaveDirectoryCount())
        return false;
    if (imageNumber < 0 || imageNumber >= d->directoryCount)
        return false;

    if (d->currentDirectory != imageNumber) {
        d->headersRead = false;
        d->currentDirectory = imageNumber;
    }
    return true;
}

int QTiffHandler::imageCount() const
{
    if (!ensureHaveDirectoryCount())
        return 1;

    return d->directoryCount;
}

int QTiffHandler::currentImageNumber() const
{
    return d->currentDirectory;
}

void QTiffHandler::convert32BitOrder(void *buffer, int width)
{
    uint32_t *target = reinterpret_cast<uint32_t *>(buffer);
    for (int32_t x=0; x<width; ++x) {
        uint32_t p = target[x];
        // convert between ARGB and ABGR
        target[x] = (p & 0xff000000)
                    | ((p & 0x00ff0000) >> 16)
                    | (p & 0x0000ff00)
                    | ((p & 0x000000ff) << 16);
    }
}

void QTiffHandler::rgb48fixup(QImage *image, bool floatingPoint)
{
    Q_ASSERT(image->depth() == 64);
    const int h = image->height();
    const int w = image->width();
    uchar *scanline = image->bits();
    const qsizetype bpl = image->bytesPerLine();
    quint16 mask = 0xffff;
    const qfloat16 fp_mask = qfloat16(1.0f);
    if (floatingPoint)
        memcpy(&mask, &fp_mask, 2);
    for (int y = 0; y < h; ++y) {
        quint16 *dst = reinterpret_cast<uint16_t *>(scanline);
        for (int x = w - 1; x >= 0; --x) {
            dst[x * 4 + 3] = mask;
            dst[x * 4 + 2] = dst[x * 3 + 2];
            dst[x * 4 + 1] = dst[x * 3 + 1];
            dst[x * 4 + 0] = dst[x * 3 + 0];
        }
        scanline += bpl;
    }
}

void QTiffHandler::rgb96fixup(QImage *image)
{
    Q_ASSERT(image->depth() == 128);
    const int h = image->height();
    const int w = image->width();
    uchar *scanline = image->bits();
    const qsizetype bpl = image->bytesPerLine();
    for (int y = 0; y < h; ++y) {
        float *dst = reinterpret_cast<float *>(scanline);
        for (int x = w - 1; x >= 0; --x) {
            dst[x * 4 + 3] = 1.0f;
            dst[x * 4 + 2] = dst[x * 3 + 2];
            dst[x * 4 + 1] = dst[x * 3 + 1];
            dst[x * 4 + 0] = dst[x * 3 + 0];
        }
        scanline += bpl;
    }
}

void QTiffHandler::rgbFixup(QImage *image)
{
    Q_ASSERT(d->floatingPoint);
    if (image->depth() == 64) {
        const int h = image->height();
        const int w = image->width();
        uchar *scanline = image->bits();
        const qsizetype bpl = image->bytesPerLine();
        for (int y = 0; y < h; ++y) {
            qfloat16 *dst = reinterpret_cast<qfloat16 *>(scanline);
            for (int x = w - 1; x >= 0; --x) {
                dst[x * 4 + 3] = qfloat16(1.0f);
                dst[x * 4 + 2] = dst[x];
                dst[x * 4 + 1] = dst[x];
                dst[x * 4 + 0] = dst[x];
            }
            scanline += bpl;
        }
    } else {
        const int h = image->height();
        const int w = image->width();
        uchar *scanline = image->bits();
        const qsizetype bpl = image->bytesPerLine();
        for (int y = 0; y < h; ++y) {
            float *dst = reinterpret_cast<float *>(scanline);
            for (int x = w - 1; x >= 0; --x) {
                dst[x * 4 + 3] = 1.0f;
                dst[x * 4 + 2] = dst[x];
                dst[x * 4 + 1] = dst[x];
                dst[x * 4 + 0] = dst[x];
            }
            scanline += bpl;
        }
    }
}

bool QTiffHandler::ensureHaveDirectoryCount() const
{
    if (d->directoryCount > 0)
        return true;

    TIFF *tiff = d->openInternal("rh", device());

    if (!tiff) {
        device()->reset();
        return false;
    }

    while (TIFFReadDirectory(tiff))
      ++d->directoryCount;
    TIFFClose(tiff);
    device()->reset();
    return true;
}

QT_END_NAMESPACE
