// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <QtTest/QtTest>
#include <QAxObject>
#include <QByteArray>

class tst_QAxObject : public QObject
{
    Q_OBJECT

private slots:
    void propertyByRefWritable();
    void setPropertyByRef();
    void multiplePropertiesDuplicateName();
};

void tst_QAxObject::propertyByRefWritable()
{
    const QAxObject speak("SAPI.SPVoice");
    const QMetaObject *metaObject = speak.metaObject();

    for (int i = 0; i < metaObject->propertyCount(); ++i) {
        if (qstrcmp(metaObject->property(i).name(), "Voice") == 0) {
            QVERIFY(metaObject->property(i).isWritable());
            break;
        }
    }
}

void tst_QAxObject::setPropertyByRef()
{
    QAxObject speak("SAPI.SPVoice");

    QVERIFY(speak.setProperty("Voice", speak.property("Voice")));
}

void tst_QAxObject::multiplePropertiesDuplicateName()
{
    // Remote desktop client control has two instances for most properties,
    // one for the setter and one for the getter
    QAxObject ax("MsTscAx.MsTscAx.4");
    if (ax.isNull())
        QSKIP("MsTscAx control was not found so test cannot be run");
    int newDesktopHeight = 768;
    QVERIFY(ax.setProperty("DesktopHeight", newDesktopHeight));
    QCOMPARE(ax.property("DesktopHeight").toInt(), newDesktopHeight);
}

QTEST_MAIN(tst_QAxObject)
#include "tst_qaxobject.moc"
