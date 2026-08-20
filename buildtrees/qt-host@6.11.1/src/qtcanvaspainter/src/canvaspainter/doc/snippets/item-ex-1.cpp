// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//![0]
class MyItem : public QCanvasPainterItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MyItem) // exposed to QML, instantiate as MyItem { ... }

    // a custom property
    Q_PROPERTY(float value READ value WRITE setValue NOTIFY valueChanged)

public:
    HelloItem(QQuickItem *parent = nullptr)
        : QCanvasPainterItem(parent)
    {
    }

    QCanvasPainterItemRenderer *createItemRenderer() const override
    {
        return new MyRenderer;
    }

    float value() const { return m_value; }
    void setValue(float newValue)
    {
        if (m_value != newValue) {
            m_value = newValue;
            emit valueChanged();
        }
    }
    float m_value = 0.0f;
};
//![0]
