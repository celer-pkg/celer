// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QtTest>

#include <qplacecontentrequest.h>

QT_USE_NAMESPACE

class tst_QPlaceContentRequest : public QObject
{
    Q_OBJECT

public:
    tst_QPlaceContentRequest();

private Q_SLOTS:
    void contentTest();
    void clearTest();
};

tst_QPlaceContentRequest::tst_QPlaceContentRequest()
{
}

void tst_QPlaceContentRequest::contentTest()
{
    QPlaceContentRequest req;
    QCOMPARE(req.limit(), -1);
    QCOMPARE(req.contentType(), QPlaceContent::NoType);

    //check that we can set the request fields
    req.setLimit(100);
    req.setContentType(QPlaceContent::ImageType);
    QCOMPARE(req.limit(), 100);
    QCOMPARE(req.contentType(), QPlaceContent::ImageType);

    //check that we assignment works correctly
    QPlaceContentRequest otherReq;
    otherReq.setLimit(10);
    otherReq.setContentType(QPlaceContent::ReviewType);
    req = otherReq;
    QCOMPARE(req.limit(), 10);
    QCOMPARE(req.contentType(), QPlaceContent::ReviewType);
    QCOMPARE(req, otherReq);

   //check that comparison will fail if one the fields are different
    req.setLimit(9000);
    QVERIFY(req != otherReq);
}

void tst_QPlaceContentRequest::clearTest()
{
    QPlaceContentRequest req;
    req.setContentType(QPlaceContent::ReviewType);
    req.setLimit(9000);
    req.clear();
    QVERIFY(req.contentType() == QPlaceContent::NoType);
    QVERIFY(req.limit() == -1);
}

QTEST_APPLESS_MAIN(tst_QPlaceContentRequest)

#include "tst_qplacecontentrequest.moc"
