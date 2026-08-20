// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QApplication>
#include <QMessageBox>
#include <QMainWindow>
#include <QScreen>
#include <QVariant>
#include <QSettings>
#include <QFileDialog>
#include <QCommandLineParser>

#include "ui_mainwindow.h"

static const char geometryKey[] = "Geometry";

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();
    void openMedia(const QString &mediaUrl);

public slots:
    void on_mediaPlayer_PlayStateChange(int newState);
    void on_actionOpen_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();

private:
    void updateWindowTitle(const QString &state);
    Ui::MainWindow m_ui;
};

MainWindow::MainWindow()
{
    m_ui.setupUi(this);

    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QByteArray restoredGeometry = settings.value(QLatin1String(geometryKey)).toByteArray();
    if (restoredGeometry.isEmpty() || !restoreGeometry(restoredGeometry)) {
        const QRect availableGeometry = screen()->availableGeometry();
        const QSize size = (availableGeometry.size() * 4) / 5;
        resize(size);
        move(availableGeometry.center() - QPoint(size.width(), size.height()) / 2);
    }

    m_ui.mediaPlayer->dynamicCall("enableContextMenu", false);
    m_ui.mediaPlayer->dynamicCall("stretchToFit", true);
    updateWindowTitle("");
}

MainWindow::~MainWindow()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue(QLatin1String(geometryKey), saveGeometry());
}

void MainWindow::on_mediaPlayer_PlayStateChange(int newState)
{
    static const QHash<int, const char *> stateMapping {
        {1,  "Stopped"},
        {2,  "Paused"},
        {3,  "Playing"},
        {4,  "Scanning Forwards"},
        {5,  "Scanning Backwards"},
        {6,  "Buffering"},
        {7,  "Waiting"},
        {8,  "Media Ended"},
        {9,  "Transitioning"},
        {10, "Ready"},
        {11, "Reconnecting"},
    };
    const char *stateStr = stateMapping.value(newState, "");
    updateWindowTitle(tr(stateStr));
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog fileDialog(this, tr("Open File"));
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setMimeTypeFilters({ "application/octet-stream", "video/x-msvideo", "video/mp4", "audio/mpeg", "audio/mp4" });
    if (fileDialog.exec() == QDialog::Accepted)
        openMedia(fileDialog.selectedFiles().first());
}

void MainWindow::on_actionExit_triggered()
{
    QCoreApplication::quit();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About Media Player"),
                tr("This Example has been created using the ActiveQt integration into Qt Designer.\n"
                   "It demonstrates the use of QAxWidget to embed the Windows Media Player ActiveX\n"
                   "control into a Qt application."));
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::openMedia(const QString &mediaUrl)
{
    if (!mediaUrl.isEmpty())
        m_ui.mediaPlayer->dynamicCall("URL", mediaUrl);
}

void MainWindow::updateWindowTitle(const QString &state)
{
    QString appName = QCoreApplication::applicationName();
    QString title = state.isEmpty() ? appName :
                    QString("%1 (%2)").arg(appName, state);
    setWindowTitle(title);
}

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QCoreApplication::setApplicationName(QLatin1String("Active Qt Media Player"));
    QCoreApplication::setOrganizationName(QLatin1String("QtProject"));

    MainWindow w;
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The media file to open.");
    parser.process(app);
    if (!parser.positionalArguments().isEmpty())
        w.openMedia(parser.positionalArguments().constFirst());
    w.show();
    return app.exec();
}
