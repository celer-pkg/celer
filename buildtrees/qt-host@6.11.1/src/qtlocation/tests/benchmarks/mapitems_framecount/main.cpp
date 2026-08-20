// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QApplication>
#include <QQuickWidget>
#include <QTime>
#include <QList>

class TimerWidget : public QQuickWidget
{
    Q_OBJECT
public:
    TimerWidget(const QList<QString> &scripts, QWidget *parent = nullptr)
        : QQuickWidget(parent)
    {
        qmlFiles = scripts;
        QObject::connect(this->quickWindow(), &QQuickWindow::beforeFrameBegin, this, &TimerWidget::startTimer);
        QObject::connect(this->quickWindow(), &QQuickWindow::afterFrameEnd, this, &TimerWidget::endTimer);
        QObject::connect(this, &TimerWidget::finishedSampleSize, this, &TimerWidget::loadNext);
    }

public slots:
    void startTimer()
    {
        renderStartAt = QTime::currentTime().msecsSinceStartOfDay();
    }

    void endTimer()
    {
        if (renderStartAt < 0) return;

        const int now = QTime::currentTime().msecsSinceStartOfDay();
        const int renderTime = now-renderStartAt;

        numberOfRenders++;
        sumOfRenderTime += renderTime;

        qInfo() << "    Rendering took" << renderTime << "msec. Average:" << sumOfRenderTime/numberOfRenders << "msec (sample size:" << numberOfRenders << ")";

        renderStartAt = -1;

        if (numberOfRenders >= runSampleSize)
        {
            executionDuration.insert(currentScript, sumOfRenderTime);
            emit finishedSampleSize();
        }
    }

    void loadNext()
    {
        if (qmlFiles.isEmpty())
        {
            writeResult();
            return;
        }
        currentScript = qmlFiles.takeFirst();
        numberOfRenders = 0;
        sumOfRenderTime = 0;
        QUrl url(currentScript);
        setSource(url);
    }

    void runScripts()
    {
        loadNext();
    }

    void writeResult()
    {
        for (const auto &i : executionDuration.keys())
        {
            qInfo() << "Render duration for" << runSampleSize << "Frames of script" << i << ":" << executionDuration[i] << "msec.";
        }
        this->deleteLater();
        QApplication::exit(0);
    }

signals:
    void finishedSampleSize();

private:
    int runSampleSize = 100;
    int renderStartAt = -1;
    int sumOfRenderTime = 0;
    int numberOfRenders = 0;

    QString currentScript;
    QList<QString> qmlFiles;
    QMap<QString, int> executionDuration;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TimerWidget *w = new TimerWidget({
                                         "qrc:/circles.qml",
                                         "qrc:/rectangles.qml",
                                         "qrc:/polylines.qml",
                                         "qrc:/polygons.qml"
                                     });
    w->show();
    w->runScripts();
    return a.exec();
}

#include "main.moc"
