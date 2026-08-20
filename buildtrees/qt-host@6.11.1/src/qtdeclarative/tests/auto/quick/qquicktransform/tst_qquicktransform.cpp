// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QtQuick>
#include <QtQuick/private/qquicktranslate_p.h>

QT_BEGIN_NAMESPACE

class tst_qquicktransform : public QObject
{
    Q_OBJECT
private slots:
    void shearTransform();
    void rotationDistanceToPlane();
};

void tst_qquicktransform::shearTransform()
{
    const QRect rect(10, 20, 100, 200);

    {
        QQuickShear shear;
        shear.setXFactor(1.0);
        shear.setOrigin(QVector3D(rect.topLeft()));

        QMatrix4x4 matrix;
        shear.applyTo(&matrix);

        QRect mappedRect = matrix.mapRect(rect);

        QCOMPARE(mappedRect.topLeft(), rect.topLeft());
        QCOMPARE(mappedRect.bottomRight(), rect.bottomRight() + QPoint(rect.height(), 0.0));
    }

    {
        QQuickShear shear;
        shear.setOrigin(QVector3D(rect.topLeft()));
        shear.setYFactor(0.5);

        QMatrix4x4 matrix;
        shear.applyTo(&matrix);

        QRect mappedRect = matrix.mapRect(rect);

        QCOMPARE(mappedRect.topLeft(), rect.topLeft());
        QCOMPARE(mappedRect.bottomRight(), rect.bottomRight() + QPoint(0.0, rect.width() * 0.5));
    }

    {
        QQuickShear shear;
        shear.setOrigin(QVector3D(rect.topLeft()));
        shear.setXFactor(1.0);
        shear.setYFactor(0.5);

        QMatrix4x4 matrix;
        shear.applyTo(&matrix);

        QRect mappedRect = matrix.mapRect(rect);

        QCOMPARE(mappedRect.topLeft(), rect.topLeft());
        QCOMPARE(mappedRect.bottomRight(), rect.bottomRight() + QPoint(rect.height(), rect.width() * 0.5));
    }

    {
        QQuickShear shear;
        shear.setOrigin(QVector3D(rect.topLeft()));
        shear.setXFactor(0.5);
        shear.setXAngle(qAtan(0.5) * 180.0 / M_PI);

        QMatrix4x4 matrix;
        shear.applyTo(&matrix);

        QRect mappedRect = matrix.mapRect(rect);

        QCOMPARE(mappedRect.topLeft(), rect.topLeft());
        QCOMPARE(mappedRect.bottomRight(), rect.bottomRight() + QPoint(rect.height(), 0.0));
    }

    {
        QQuickShear shear;
        shear.setOrigin(QVector3D(rect.topLeft()));
        shear.setYFactor(0.25);
        shear.setYAngle(qAtan(0.25) * 180.0 / M_PI);

        QMatrix4x4 matrix;
        shear.applyTo(&matrix);

        QRect mappedRect = matrix.mapRect(rect);

        QCOMPARE(mappedRect.topLeft(), rect.topLeft());
        QCOMPARE(mappedRect.bottomRight(), rect.bottomRight() + QPoint(0.0, rect.width() * 0.5));
    }
}

void tst_qquicktransform::rotationDistanceToPlane()
{
    const QRect testRect(100, 100, 3840, 2160);
    const QPoint topLeft = testRect.topLeft();
    const QPoint topRight = testRect.topRight();
    const QPoint bottomLeft = testRect.bottomLeft();
    const QPoint bottomRight = testRect.bottomRight();

    { // distanceToPlane = 1024
        QQuickRotation rotation;
        rotation.setAngle(-45.0);
        rotation.setAxis(QVector3D(0, 1, 0));
        rotation.setOrigin(QVector3D(2020, 1180, 0));

        QMatrix4x4 matrix;
        rotation.applyTo(&matrix);

        QRect mappedRect = matrix.mapRect(testRect);
        QRect expectedRect(-2147, -2135, 3583, 6630);
        QCOMPARE(mappedRect, expectedRect);

        QPoint mappedTopLeft = matrix.map(topLeft);
        QPoint mappedTopRight = matrix.map(topRight);
        QPoint mappedBottomLeft = matrix.map(bottomLeft);
        QPoint mappedBottomRight = matrix.map(bottomRight);

        QCOMPARE(mappedTopLeft, QPoint(1436, 716));
        QCOMPARE(mappedTopRight, QPoint(-2153, 4502));
        QCOMPARE(mappedBottomLeft, QPoint(1436, 1644));
        QCOMPARE(mappedBottomRight, QPoint(-2153, -2139));
    }

    { // distanceToPlane = 2048
        QQuickRotation rotation;
        rotation.setAngle(-45.0);
        rotation.setAxis(QVector3D(0, 1, 0));
        rotation.setOrigin(QVector3D(2020, 1180, 0));
        rotation.setDistanceToPlane(2048.0);

        QMatrix4x4 matrix;
        rotation.applyTo(&matrix);

        QRect mappedRect = matrix.mapRect(testRect);
        QRect expectedRect(1204, -2024, 4844, 6408);
        QCOMPARE(mappedRect, expectedRect);

        QPoint mappedTopLeft = matrix.map(topLeft);
        QPoint mappedTopRight = matrix.map(topRight);
        QPoint mappedBottomLeft = matrix.map(bottomLeft);
        QPoint mappedBottomRight = matrix.map(bottomRight);

        QCOMPARE(mappedTopLeft, QPoint(1204, 531));
        QCOMPARE(mappedTopRight, QPoint(6041, -2021));
        QCOMPARE(mappedBottomLeft, QPoint(1204, 1829));
        QCOMPARE(mappedBottomRight, QPoint(6041, 4378));
    }

    { // distanceToPlane = 512
        QQuickRotation rotation;
        rotation.setAngle(-45.0);
        rotation.setAxis(QVector3D(0, 1, 0));
        rotation.setOrigin(QVector3D(2020, 1180, 0));
        rotation.setDistanceToPlane(512.0);

        QMatrix4x4 matrix;
        rotation.applyTo(&matrix);

        QRect mappedRect = matrix.mapRect(testRect);
        QRect expectedRect(1198, 526, 450, 1308);
        QCOMPARE(mappedRect, expectedRect);

        QPoint mappedTopLeft = matrix.map(topLeft);
        QPoint mappedTopRight = matrix.map(topRight);
        QPoint mappedBottomLeft = matrix.map(bottomLeft);
        QPoint mappedBottomRight = matrix.map(bottomRight);

        QCOMPARE(mappedTopLeft, QPoint(1648, 884));
        QCOMPARE(mappedTopRight, QPoint(1198, 1834));
        QCOMPARE(mappedBottomLeft, QPoint(1648, 1475));
        QCOMPARE(mappedBottomRight, QPoint(1198, 526));
    }

    { // distanceToPlane = 0
        QQuickRotation rotation;
        rotation.setAngle(-45.0);
        rotation.setAxis(QVector3D(0, 1, 0));
        rotation.setOrigin(QVector3D(2020, 1180, 0));
        rotation.setDistanceToPlane(0.0);

        QMatrix4x4 matrix;
        rotation.applyTo(&matrix);

        QRect mappedRect = matrix.mapRect(testRect);
        QRect expectedRect(662, 100, 2716, 2160);
        QCOMPARE(mappedRect, expectedRect);

        QPoint mappedTopLeft = matrix.map(topLeft);
        QPoint mappedTopRight = matrix.map(topRight);
        QPoint mappedBottomLeft = matrix.map(bottomLeft);
        QPoint mappedBottomRight = matrix.map(bottomRight);

        QCOMPARE(mappedTopLeft, QPoint(662, 100));
        QCOMPARE(mappedTopRight, QPoint(3377, 100));
        QCOMPARE(mappedBottomLeft, QPoint(662, 2259));
        QCOMPARE(mappedBottomRight, QPoint(3377, 2259));
    }

    // Test Z-axis rotation - distanceToPlane should have no effect
    {
        QQuickRotation rotation1;
        rotation1.setAngle(45.0);
        rotation1.setAxis(QVector3D(0, 0, 1));
        rotation1.setOrigin(QVector3D(2020, 1180, 0));
        // Use default distanceToPlane = 1024.0

        QMatrix4x4 matrix1;
        rotation1.applyTo(&matrix1);

        QRect mappedRect1 = matrix1.mapRect(testRect);
        QPoint mappedTopLeft1 = matrix1.map(topLeft);
        QPoint mappedTopRight1 = matrix1.map(topRight);
        QPoint mappedBottomLeft1 = matrix1.map(bottomLeft);
        QPoint mappedBottomRight1 = matrix1.map(bottomRight);

        // Test with different distanceToPlane values
        qreal distanceValues[] = {2048.0, 512.0, 0.0};
        for (int i = 0; i < 3; i++) {
            QQuickRotation rotation2;
            rotation2.setAngle(45.0);
            rotation2.setAxis(QVector3D(0, 0, 1));
            rotation2.setOrigin(QVector3D(2020, 1180, 0));
            rotation2.setDistanceToPlane(distanceValues[i]);

            QMatrix4x4 matrix2;
            rotation2.applyTo(&matrix2);

            QRect mappedRect2 = matrix2.mapRect(testRect);
            QPoint mappedTopLeft2 = matrix2.map(topLeft);
            QPoint mappedTopRight2 = matrix2.map(topRight);
            QPoint mappedBottomLeft2 = matrix2.map(bottomLeft);
            QPoint mappedBottomRight2 = matrix2.map(bottomRight);

            // Z-axis rotation should be identical regardless of distanceToPlane
            QCOMPARE(mappedRect2, mappedRect1);
            QCOMPARE(mappedTopLeft2, mappedTopLeft1);
            QCOMPARE(mappedTopRight2, mappedTopRight1);
            QCOMPARE(mappedBottomLeft2, mappedBottomLeft1);
            QCOMPARE(mappedBottomRight2, mappedBottomRight1);
        }

        // Verify the actual Z-axis rotation result
        QRect expectedRect(-101, -941, 4242, 4242);
        QCOMPARE(mappedRect1, expectedRect);
        QCOMPARE(mappedTopLeft1, QPoint(1426, -941));
        QCOMPARE(mappedTopRight1, QPoint(4141, 1773));
        QCOMPARE(mappedBottomLeft1, QPoint(-101, 585));
        QCOMPARE(mappedBottomRight1, QPoint(2614, 3300));
    }
}

QT_END_NAMESPACE

QTEST_MAIN(tst_qquicktransform)

#include "tst_qquicktransform.moc"
