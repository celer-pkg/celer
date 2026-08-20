// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qwebphandler_p.h"
#include "webp/mux.h"
#include "webp/encode.h"
#include <qcolor.h>
#include <qimage.h>
#include <qdebug.h>
#include <qpainter.h>
#include <qvariant.h>
#include <QtEndian>

static const int riffHeaderSize = 12; // RIFF_HEADER_SIZE from webp/format_constants.h

QT_BEGIN_NAMESPACE

QWebpHandler::QWebpHandler() :
    m_quality(75),
    m_scanState(ScanNotScanned),
    m_features(),
    m_formatFlags(0),
    m_loop(0),
    m_frameCount(0),
    m_demuxer(NULL),
    m_composited(NULL)
{
    memset(&m_iter, 0, sizeof(m_iter));
}

QWebpHandler::~QWebpHandler()
{
    WebPDemuxReleaseIterator(&m_iter);
    WebPDemuxDelete(m_demuxer);
    delete m_composited;
}

bool QWebpHandler::canRead() const
{
    if (m_scanState == ScanNotScanned && !canRead(device()))
        return false;

    if (m_scanState != ScanError) {
        setFormat(QByteArrayLiteral("webp"));

        if (m_features.has_animation && m_iter.frame_num >= m_frameCount)
            return false;

        return true;
    }
    return false;
}

bool QWebpHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("QWebpHandler::canRead() called with no device");
        return false;
    }

    QByteArray header = device->peek(riffHeaderSize);
    return header.startsWith("RIFF") && header.endsWith("WEBP");
}

bool QWebpHandler::ensureScanned() const
{
    if (m_scanState != ScanNotScanned)
        return m_scanState == ScanSuccess;

    m_scanState = ScanError;

    QWebpHandler *that = const_cast<QWebpHandler *>(this);
    const int headerBytesNeeded = sizeof(WebPBitstreamFeatures);
    QByteArray header = device()->peek(headerBytesNeeded);
    if (header.size() < headerBytesNeeded)
        return false;

    // We do no random access during decoding, just a readAll() of the whole image file. So if
    // if it is all available already, we can accept a sequential device. The riff header contains
    // the file size minus 8 bytes header
    qint64 byteSize = qFromLittleEndian<quint32>(header.constData() + 4);
    if (device()->isSequential() && device()->bytesAvailable() < byteSize + 8) {
        qWarning() << "QWebpHandler: Insufficient data available in sequential device";
        return false;
    }
    if (WebPGetFeatures((const uint8_t*)header.constData(), header.size(), &(that->m_features)) == VP8_STATUS_OK) {
        if (m_features.has_animation) {
            // For animation, we have to read and scan whole file to determine loop count and images count
            if (that->ensureDemuxer()) {
                that->m_loop = WebPDemuxGetI(m_demuxer, WEBP_FF_LOOP_COUNT);
                that->m_frameCount = WebPDemuxGetI(m_demuxer, WEBP_FF_FRAME_COUNT);
                that->m_bgColor = QColor::fromRgba(QRgb(WebPDemuxGetI(m_demuxer, WEBP_FF_BACKGROUND_COLOR)));

                QSize sz(that->m_features.width, that->m_features.height);
                that->m_composited = new QImage;
                if (!QImageIOHandler::allocateImage(sz, QImage::Format_ARGB32, that->m_composited))
                    return false;
                if (that->m_features.has_alpha)
                    that->m_composited->fill(Qt::transparent);

                m_scanState = ScanSuccess;
            }
        } else {
            m_scanState = ScanSuccess;
        }
    }

    return m_scanState == ScanSuccess;
}

bool QWebpHandler::ensureDemuxer()
{
    if (m_demuxer)
        return true;

    m_rawData = device()->readAll();
    m_webpData.bytes = reinterpret_cast<const uint8_t *>(m_rawData.constData());
    m_webpData.size = m_rawData.size();

    m_demuxer = WebPDemux(&m_webpData);
    if (m_demuxer == NULL)
        return false;

    m_formatFlags = WebPDemuxGetI(m_demuxer, WEBP_FF_FORMAT_FLAGS);
    return true;
}

bool QWebpHandler::read(QImage *image)
{
    if (!ensureScanned() || !ensureDemuxer())
        return false;

    QRect prevFrameRect;
    if (m_iter.frame_num == 0) {
        // Read global meta-data chunks first
        WebPChunkIterator metaDataIter;
        if ((m_formatFlags & ICCP_FLAG) && WebPDemuxGetChunk(m_demuxer, "ICCP", 1, &metaDataIter)) {
            QByteArray iccProfile = QByteArray::fromRawData(reinterpret_cast<const char *>(metaDataIter.chunk.bytes),
                                                            metaDataIter.chunk.size);
            // Ensure the profile is 4-byte aligned.
            if (reinterpret_cast<qintptr>(iccProfile.constData()) & 0x3)
                iccProfile.detach();
            m_colorSpace = QColorSpace::fromIccProfile(iccProfile);
            // ### consider parsing EXIF and/or XMP metadata too.
            WebPDemuxReleaseChunkIterator(&metaDataIter);
        }

        // Go to first frame
        if (!WebPDemuxGetFrame(m_demuxer, 1, &m_iter))
            return false;
    } else {
        if (m_iter.has_alpha && m_iter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND)
            prevFrameRect = currentImageRect();

        // Go to next frame
        if (!WebPDemuxNextFrame(&m_iter))
            return false;
    }

    WebPBitstreamFeatures features;
    VP8StatusCode status = WebPGetFeatures(m_iter.fragment.bytes, m_iter.fragment.size, &features);
    if (status != VP8_STATUS_OK)
        return false;

    QImage::Format format = m_features.has_alpha ? QImage::Format_ARGB32 : QImage::Format_RGB32;
    QImage frame;
    if (!QImageIOHandler::allocateImage(QSize(m_iter.width, m_iter.height), format, &frame))
        return false;
    uint8_t *output = frame.bits();
    size_t output_size = frame.sizeInBytes();
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    if (!WebPDecodeBGRAInto(
        reinterpret_cast<const uint8_t*>(m_iter.fragment.bytes), m_iter.fragment.size,
        output, output_size, frame.bytesPerLine()))
#else
    if (!WebPDecodeARGBInto(
        reinterpret_cast<const uint8_t*>(m_iter.fragment.bytes), m_iter.fragment.size,
        output, output_size, frame.bytesPerLine()))
#endif
        return false;

    if (!m_features.has_animation) {
        // Single image
        *image = frame;
    } else {
        // Animation
        QPainter painter(m_composited);
        if (!prevFrameRect.isEmpty()) {
            painter.setCompositionMode(QPainter::CompositionMode_Clear);
            painter.fillRect(prevFrameRect, Qt::black);
        }
        if (m_features.has_alpha) {
            if (m_iter.blend_method == WEBP_MUX_NO_BLEND)
                painter.setCompositionMode(QPainter::CompositionMode_Source);
            else
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
        painter.drawImage(currentImageRect(), frame);

        *image = *m_composited;
    }
    image->setColorSpace(m_colorSpace);

    return true;
}

bool QWebpHandler::write(const QImage &image)
{
    if (image.isNull()) {
        qWarning() << "source image is null.";
        return false;
    }
    if (std::max(image.width(), image.height()) > WEBP_MAX_DIMENSION) {
        qWarning() << "QWebpHandler::write() source image too large for WebP: " << image.size();
        return false;
    }

    const bool alpha = image.hasAlphaChannel();
    QImage::Format newFormat = alpha ? QImage::Format_RGBA8888 : QImage::Format_RGB888;
    const QImage srcImage = (image.format() == newFormat) ? image : image.convertedTo(newFormat);

    WebPPicture picture;
    WebPConfig config;

    if (!WebPPictureInit(&picture) || !WebPConfigInit(&config)) {
        qWarning() << "failed to init webp picture and config";
        return false;
    }

    picture.width = srcImage.width();
    picture.height = srcImage.height();
    picture.use_argb = 1;
    bool failed = false;
    if (alpha)
        failed = !WebPPictureImportRGBA(&picture, srcImage.constBits(), srcImage.bytesPerLine());
    else
        failed = !WebPPictureImportRGB(&picture, srcImage.constBits(), srcImage.bytesPerLine());

    if (failed) {
        qWarning() << "failed to import image data to webp picture.";
        WebPPictureFree(&picture);
        return false;
    }

    int reqQuality = m_quality < 0 ? 75 : qMin(m_quality, 100);
    if (reqQuality < 100) {
        config.lossless = 0;
        config.quality = reqQuality;
    } else {
        config.lossless = 1;
        config.quality = 70;  // For lossless, specifies compression effort; 70 is libwebp default
    }
    config.alpha_quality = config.quality;
    WebPMemoryWriter writer;
    WebPMemoryWriterInit(&writer);
    picture.writer = WebPMemoryWrite;
    picture.custom_ptr = &writer;

    if (!WebPEncode(&config, &picture)) {
        qWarning() << "failed to encode webp picture, error code: " << picture.error_code;
        WebPPictureFree(&picture);
        WebPMemoryWriterClear(&writer);
        return false;
    }

    bool res = false;
    if (image.colorSpace().isValid()) {
        int copy_data = 0;
        WebPMux *mux = WebPMuxNew();
        WebPData image_data = { writer.mem, writer.size };
        WebPMuxSetImage(mux, &image_data, copy_data);
        uint8_t vp8xChunk[10];
        uint8_t flags = 0x20; // Has ICCP chunk, no XMP, EXIF or animation.
        if (image.hasAlphaChannel())
            flags |= 0x10;
        vp8xChunk[0] = flags;
        vp8xChunk[1] = 0;
        vp8xChunk[2] = 0;
        vp8xChunk[3] = 0;
        const unsigned width = image.width() - 1;
        const unsigned height = image.height() - 1;
        vp8xChunk[4] = width & 0xff;
        vp8xChunk[5] = (width >> 8) & 0xff;
        vp8xChunk[6] = (width >> 16) & 0xff;
        vp8xChunk[7] = height & 0xff;
        vp8xChunk[8] = (height >> 8) & 0xff;
        vp8xChunk[9] = (height >> 16) & 0xff;
        WebPData vp8x_data = { vp8xChunk, 10 };
        if (WebPMuxSetChunk(mux, "VP8X", &vp8x_data, copy_data) == WEBP_MUX_OK) {
            QByteArray iccProfile = image.colorSpace().iccProfile();
            WebPData iccp_data = {
                    reinterpret_cast<const uint8_t *>(iccProfile.constData()),
                    static_cast<size_t>(iccProfile.size())
            };
            if (WebPMuxSetChunk(mux, "ICCP", &iccp_data, copy_data) == WEBP_MUX_OK) {
                WebPData output_data;
                if (WebPMuxAssemble(mux, &output_data) == WEBP_MUX_OK) {
                    res = (output_data.size ==
                               static_cast<size_t>(device()->write(reinterpret_cast<const char *>(output_data.bytes), output_data.size)));
                }
                WebPDataClear(&output_data);
            }
        }
        WebPMuxDelete(mux);
    }
    if (!res) {
        res = (writer.size ==
                   static_cast<size_t>(device()->write(reinterpret_cast<const char *>(writer.mem), writer.size)));
    }
    WebPPictureFree(&picture);
    WebPMemoryWriterClear(&writer);

    return res;
}

QVariant QWebpHandler::option(ImageOption option) const
{
    if (!supportsOption(option) || !ensureScanned())
        return QVariant();

    switch (option) {
    case Quality:
        return m_quality;
    case Size:
        return QSize(m_features.width, m_features.height);
    case Animation:
        return m_features.has_animation;
    case BackgroundColor:
        return m_bgColor;
    default:
        return QVariant();
    }
}

void QWebpHandler::setOption(ImageOption option, const QVariant &value)
{
    switch (option) {
    case Quality:
        m_quality = value.toInt();
        return;
    default:
        break;
    }
    QImageIOHandler::setOption(option, value);
}

bool QWebpHandler::supportsOption(ImageOption option) const
{
    return option == Quality
        || option == Size
        || option == Animation
        || option == BackgroundColor;
}

int QWebpHandler::imageCount() const
{
    if (!ensureScanned())
        return 0;

    if (!m_features.has_animation)
        return 1;

    return m_frameCount;
}

int QWebpHandler::currentImageNumber() const
{
    if (!ensureScanned() || !m_features.has_animation)
        return 0;

    // Frame number in WebP starts from 1
    return m_iter.frame_num - 1;
}

QRect QWebpHandler::currentImageRect() const
{
    if (!ensureScanned())
        return QRect();

    return QRect(m_iter.x_offset, m_iter.y_offset, m_iter.width, m_iter.height);
}

int QWebpHandler::loopCount() const
{
    if (!ensureScanned() || !m_features.has_animation)
        return 0;

    // Loop count in WebP starts from 0
    return m_loop - 1;
}

int QWebpHandler::nextImageDelay() const
{
    if (!ensureScanned() || !m_features.has_animation)
        return 0;

    return m_iter.duration;
}

QT_END_NAMESPACE
