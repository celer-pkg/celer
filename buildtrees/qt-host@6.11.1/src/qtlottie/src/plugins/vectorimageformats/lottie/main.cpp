// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQuickVectorImageGenerator/private/qquickvectorimageplugin_p.h>
#include <QtLottieVectorImageGenerator/private/qlottievisitor_p.h>
#include <QtLottie/private/qlottieroot_p.h>
#include <QtCore/qfile.h>
#include <QtCore/qscopeguard.h>

#include <QtQuick/private/qquickanimation_p.h>

QT_BEGIN_NAMESPACE

class QLottieVectorImagePlugin : public QObject, public QQuickVectorImagePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQuickVectorImageFormatsPluginFactory_iid FILE "lottie.json")
    Q_INTERFACES(QQuickVectorImagePlugin)
public:
    QLottieVectorImagePlugin();
    ~QLottieVectorImagePlugin();

    bool generate(const QString &fileName, QQuickItemGenerator *generator) override;

private:
    bool canRead(QIODevice &input) const;
};

QLottieVectorImagePlugin::QLottieVectorImagePlugin()
{
}

QLottieVectorImagePlugin::~QLottieVectorImagePlugin()
{
}

bool QLottieVectorImagePlugin::generate(const QString &fileName, QQuickItemGenerator *generator)
{
    QFile f(fileName);
    QLottieRoot root;

    if (f.open(QIODevice::ReadOnly) && canRead(f)) {
        QByteArray jsonSource = f.readAll();

        static int frameNo = qEnvironmentVariableIntValue("QLT_FRAMENO");

        if (root.parseSource(jsonSource, QUrl::fromLocalFile(fileName)) >= 0) {
            if (frameNo < 0)
                frameNo = root.startFrame() + (root.endFrame() - root.startFrame()) / 2;

            root.setStructureDumping(true);
            for (QLottieBase *elem : root.children()) {
                if (elem->active(0))
                    elem->updateProperties(0);
            }

            generator->addExtraImport(QStringLiteral("Qt.labs.lottieqt.VectorImageHelpers"));
            QLottieVisitor visitor(fileName, generator);
            visitor.render(root);

            if (frameNo > 0) {
                QQuickItem *item = generator->parentItem();
                const int seekTime = qRound(1000.0 * frameNo / root.frameRate());

                QList<QQuickAbstractAnimation *> animations = item->findChildren<QQuickAbstractAnimation *>();
                for (QQuickAbstractAnimation *animation : animations) {
                    if (animation->group() == nullptr)
                        animation->setCurrentTime(seekTime);
                }
            }

            return true;
        }
    }

    return false;
}

bool QLottieVectorImagePlugin::canRead(QIODevice &input) const
{
    const qint64 pos = input.pos();
    auto cleanup = qScopeGuard([&] { input.seek(pos); });
    QTextStream s(&input);
    const QString head = s.read(256);
    bool res = QStringView(head).trimmed().startsWith(QChar::fromLatin1('{'));
    return res;
}

QT_END_NAMESPACE

#include "main.moc"
