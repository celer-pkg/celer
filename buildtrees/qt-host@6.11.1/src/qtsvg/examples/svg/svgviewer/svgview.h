// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SVGVIEW_H
#define SVGVIEW_H

#include <QWidget>
#include <QSvgRenderer>

class SvgView : public QWidget
{
    Q_OBJECT
public:
    enum Background {
        Black,
        White,
        Chequered
    };

    enum Feature {
        Tiny_12,
        Extended,
    };

    explicit SvgView(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;
    QImage renderAsImage(QSize imageSize);

    void setBackgroundType(Background type);
    void setFeature(Feature feature);
    void setScale(qreal scale);

    void restartAnimations();

    void load(QStringView fileName);
    void reload();
    QStringView currentFile() const;

    QSize fileSize() const;
    QSize sizeHint() const override;

public slots:
    void setAnimations(bool enable);
    void setAssumeTrustedSource(bool enable);
    void setOutline(bool enable);

private:
    QPixmap m_chequered;
    Background m_background;
    QSvgRenderer m_renderer;
    qreal m_scale = 1.0;
    QString m_fileName;
    bool m_outline = true;
};

#endif // SVGVIEW_H
