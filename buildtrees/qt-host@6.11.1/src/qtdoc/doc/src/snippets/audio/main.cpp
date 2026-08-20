// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>

#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QAudioInput>

class Window2 : public QWidget
{
    Q_OBJECT

public slots:
//![0]
    void stateChanged(QAudio::State newState)
    {
        switch(newState) {
            case QAudio::StopState:
            if (input->error() != QAudio::NoError) {
                // Error handling
            } else {

            }
            break;
//![0]
        default:
            ;
        }
    }

private:
    QAudioInput *input;

};

class Window : public QWidget
{
    Q_OBJECT

public:
    Window()
    {
        output = new QAudioOutput;
        connect(output, SIGNAL(stateChanged(QAudio::State)),
            this, SLOT(stateChanged(QAudio::State)));
    }

private:
    void setupFormat()
    {
//![1]
        QAudioFormat format;
        format.setFrequency(44100);
//![1]
        format.setChannels(2);
        format.setSampleSize(16);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
//![2]
        format.setSampleType(QAudioFormat::SignedInt);

        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

        if (!info.isFormatSupported(format))
            format = info.nearestFormat(format);
//![2]
    }

public slots:
//![3]
    void stateChanged(QAudio::State newState)
    {
        switch (newState) {
            case QAudio::StopState:
                if (output->error() != QAudio::NoError) {
                    // Perform error handling
                } else {
                    // Normal stop
                }
                break;
//![3]

            // Handle
            case QAudio::ActiveState:
                // Handle active state...
                break;
            break;
        default:
            ;
        }
    }

private:
    QAudioOutput *output;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Window window;
    window.show();

    return app.exec();
}


#include "main.moc"

