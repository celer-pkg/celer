// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TEXTUREITEM_H
#define TEXTUREITEM_H

#include <QQuickPaintedItem>
#include <QQuick3DTextureData>
#include <QImage>

class TextureItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(const QQuick3DTextureData* textureData READ textureData NOTIFY textureDataChanged FINAL)
    QML_ELEMENT

public:
    TextureItem(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;
    const QQuick3DTextureData *textureData() const;

    Q_INVOKABLE void setPoint(float x, float y, int id, bool pressed);
    Q_INVOKABLE void setUv(float u, float v, int id, bool pressed);
    Q_INVOKABLE void clear(const QColor &color);
    Q_INVOKABLE void setColor(const QColor &newColor, int id = 0);
    Q_INVOKABLE void setPenWidth(qreal newPenWidth);
    Q_INVOKABLE void resetPoint(int id);

public slots:
    void updateImage();

signals:
    void textureDataChanged();
    void colorChanged();
    void penWidthChanged();

private:

    struct TouchPointState {
        QColor color = "red";
        qreal penWidth = 10; // unused
        double alpha = 0.5;  // smoothing factor (0..1)
        bool initialized = false;
        QPointF prevFiltered;
        QPointF prevRaw;
        qint64 prevTime = 0; // timestamp in milliseconds

        void reset() {
            initialized = false;
            prevTime = 0;
        }

        QPointF filter(const QPointF &unfiltered, qint64 timestamp) {
            prevRaw = unfiltered;
            if (!initialized) {
                prevFiltered = unfiltered;
                prevTime = timestamp;
                initialized = true;
            } else {
                // Adjust alpha based on time delta
                qint64 dt = timestamp - prevTime;
                double adaptiveAlpha = qMin(1.0, alpha * (dt / 16.0)); // 16ms = ~60fps baseline

                prevFiltered.setX(adaptiveAlpha * unfiltered.x() + (1 - adaptiveAlpha) * prevFiltered.x());
                prevFiltered.setY(adaptiveAlpha * unfiltered.y() + (1 - adaptiveAlpha) * prevFiltered.y());
                prevTime = timestamp;
            }
            return prevFiltered;
        }
    };

    void reset(TouchPointState &s);

    QHash<int, TouchPointState> state;
    QQuick3DTextureData m_textureData;
    QImage m_image;
    qreal m_penWidth = 10;
};

#endif // TEXTUREITEM_H
