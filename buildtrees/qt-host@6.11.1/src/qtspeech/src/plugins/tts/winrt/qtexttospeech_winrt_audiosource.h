// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECHENGINE_WINRT_AUDIOSOURCE_H
#define QTEXTTOSPEECHENGINE_WINRT_AUDIOSOURCE_H

#include <QtCore/QIODevice>
#include <QtMultimedia/QAudioFormat>

#include <robuffer.h>
#include <winrt/base.h>
#include <QtCore/private/qfactorycacheregistration_p.h>
#include <QtCore/private/qcomobject_p.h>
#include <windows.foundation.h>
#include <windows.media.speechsynthesis.h>
#include <windows.storage.streams.h>

#include <wrl.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Media::SpeechSynthesis;
using namespace ABI::Windows::Storage::Streams;
using namespace Microsoft::WRL;

QT_BEGIN_NAMESPACE

using StreamReadyHandler = IAsyncOperationCompletedHandler<SpeechSynthesisStream*>;
using BytesReadyHandler = IAsyncOperationWithProgressCompletedHandler<IBuffer*, UINT32>;

class AudioSource final : public QIODevice, public QComObject<StreamReadyHandler, BytesReadyHandler>
{
    Q_OBJECT
public:
    AudioSource(ComPtr<IAsyncOperation<SpeechSynthesisStream*>> synthOperation);

    bool isSequential() const override { return true; }

    void close() override;
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override { return 0; }

    bool atEnd() const override;
    qint64 bytesAvailable() const override;

    QAudioFormat format() const { return audioFormat; }

    enum PauseState {
        NoPause,
        PauseRequested,
        Paused
    } m_pause = NoPause;

    void pause(quint64 atByte)
    {
        m_pause = PauseRequested;
        m_pauseRequestedAt = atByte;
    }

    void resume()
    {
        m_pause = NoPause;
        m_pauseRequestedAt = 0;
        if (bytesAvailable())
            emit readyRead();
    }

    struct Boundary {
        enum Type { Word, Sentence, Unknown } type;
        QString text;
        int beginIndex;
        int endIndex;
        qint64 startTime;
        friend inline bool operator<(const Boundary &lhs, const Boundary &rhs)
        {
            return lhs.startTime < rhs.startTime;
        }
    };

    QList<Boundary> boundaryData() const
    {
        return boundaries;
    }

    // completion handler for synthesising the stream
    HRESULT STDMETHODCALLTYPE Invoke(IAsyncOperation<SpeechSynthesisStream*> *operation,
                                     AsyncStatus status) override;
    // completion handler for reading from the stream
    HRESULT STDMETHODCALLTYPE Invoke(IAsyncOperationWithProgress<IBuffer*, unsigned int> *read,
                                     AsyncStatus status) override;

Q_SIGNALS:
    void streamReady(const QAudioFormat &format);
    void errorInStream();

private:
    // lifetime is controlled via IUnknown reference counting, make sure
    // we don't destroy by accident, polymorphically, or via a QObject parent
    ~AudioSource() override;

    qint64 bytesInBuffer() const;
    bool fetchMore();

    QAudioFormat audioFormat;

    // The input stream that gives access to the synthesis stream. We keep the
    // async operation so that we can cancel it if we get destroyed prematurely.
    ComPtr<IAsyncOperation<SpeechSynthesisStream*>> synthOperation;
    ComPtr<IInputStream> inputStream;
    ComPtr<IRandomAccessStream> randomAccessStream;
    // the current ReadAsync operation that yields an IBuffer
    ComPtr<IAsyncOperationWithProgress<IBuffer*, UINT32>> readOperation;
    ComPtr<IBuffer> m_buffer;
    // access to the raw pcm bytes in the IBuffer; this took much reading of Windows header files...
    ComPtr<::Windows::Storage::Streams::IBufferByteAccess> bufferByteAccess;
    // The data in the IBuffer might be paritally consumed
    UINT32 m_bufferOffset = 0;
    // RIFF header has been checked at the beginning of the stream
    bool m_riffHeaderChecked = false;
    quint64 m_bytesRead = 0;
    quint64 m_pauseRequestedAt = 0;
    std::optional<qint64> m_pauseDetectionSilenceCount;

    void populateBoundaries();
    QList<Boundary> boundaries;
};

QT_END_NAMESPACE

#endif
