// Copyright (C) 2015 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>

#include <QFile>

#include <QSvgGenerator>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QPainterPath>
#include <QRect>
#include <QSize>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    const QStringList arguments = app.arguments();
    if (arguments.size() < 2) {
        qWarning("Missing file name");
        return 0;
    }

    QFile output(arguments[1]);
    if (!output.open(QIODevice::WriteOnly))
        qFatal("Cannot open output file name");

    QSvgGenerator generator(QSvgGenerator::SvgVersion::Svg11);
    generator.setOutputDevice(&output);
    generator.setSize(QSize(1000, 500));
    generator.setViewBox(QRect(0, 0, 1000, 500));

    {
        QPainter painter(&generator);
        QFont f = painter.font();
        f.setPointSize(48);
        painter.setFont(f);

        {
            painter.save();
            // clipped rectangle
            painter.setClipRect(QRect(100, 100, 250, 200));

            painter.setBrush(QColorConstants::Blue);
            painter.drawEllipse(QRect(0, 100, 400, 200));

            {
                painter.save();

                // transformed element within clip
                painter.setBrush(QColorConstants::Green);
                painter.translate(300, 150);
                painter.rotate(45);
                painter.drawEllipse(QPointF(0, 0), 100, 50);

                painter.restore();
            }

            painter.drawText(200, 200, "A very long clipped text");

            painter.restore();
        }
        {
            // unclipped
            painter.setBrush(QColorConstants::Red);
            painter.drawEllipse(0, 0, 200, 150);
        }
        {
            painter.save();

            // transformed clip (by transforming the painter before setting the clip);
            painter.translate(500, 0);
            painter.rotate(45);

            painter.setClipRect(QRect(50, 50, 150, 200));

            painter.setBrush(QColorConstants::Green);
            QPainterPath path;
            path.addRect(50, 50, 100, 100);
            path.moveTo(0, 0);
            path.cubicTo(300, 0,  150, 150,  300, 300);
            path.cubicTo(0, 300,  150, 150,  0, 0);
            painter.drawPath(path);

            painter.setBrush(QColorConstants::Blue);
            painter.drawEllipse(QPointF(125, 125), 100, 50);

            painter.restore();
        }

        {
            painter.save();

            // transformed clip
            painter.translate(700, 50);

            // clip by path
            QPainterPath path;
            path.moveTo(0, 0);
            path.cubicTo(300, 0,  150, 150,  300, 300);
            path.cubicTo(0, 300,  150, 150,  0, 0);

            painter.setBrush(QColorConstants::Svg::red);
            painter.drawPath(path);

            painter.setClipPath(path);

            painter.setBrush(QColorConstants::Svg::purple);
            painter.drawEllipse(QPointF(150, 50), 150, 50);

            // transform and remove clipping
            painter.translate(0, 100);
            painter.setClipping(false);
            painter.setBrush(QColorConstants::Svg::darkblue);
            painter.drawEllipse(QPointF(150, 50), 150, 50);

            // transform and clip again
            painter.translate(0, 100);
            painter.setClipping(true);
            painter.setBrush(QColorConstants::Svg::green);
            painter.drawEllipse(QPointF(150, 50), 150, 50);


            painter.restore();
        }

    }

    output.close();
}
