// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef IMAGETESTINGITEM_H
#define IMAGETESTINGITEM_H

#include "qcanvaspainteritem.h"
#include "imagetestingrenderer.h"

class ImageTestingItem : public QCanvasPainterItem
{
    Q_OBJECT
    Q_PROPERTY(int dataAmount READ dataAmount NOTIFY dataAmountChanged FINAL)
    Q_PROPERTY(int imageAmount READ imageAmount NOTIFY imageAmountChanged FINAL)
    Q_PROPERTY(bool showGradient READ showGradient WRITE setShowGradient NOTIFY showGradientChanged FINAL)
    Q_PROPERTY(bool animateGradient READ animateGradient WRITE setAnimateGradient NOTIFY animateGradientChanged FINAL)
    QML_ELEMENT

public:
    enum class Action {
        NoActions = 0,
        Generate = 1,
        RemoveNewest = 2,
        HideNewest = 4,
        CleanupResources = 8,
        RemoveHidden = 16,
        ShowHidden = 32,
        RemoveAll = 64,
        GenerateCanvas = 128,
        UnregisterNewestCanvas = 256,
        ReregisterCanvases = 512,
        RemoveNewestCanvas = 1024
    };
    Q_DECLARE_FLAGS(Actions, Action)

    ImageTestingItem(QQuickItem *parent = nullptr);
    Q_INVOKABLE void generate() {
        m_actions ^= Action::Generate;
        update();
    }
    Q_INVOKABLE void removeNewestImage() {
        m_actions ^= Action::RemoveNewest;
        update();
    }
    Q_INVOKABLE void removeHiddenImages() {
        m_actions ^= Action::RemoveHidden;
        update();
    }
    Q_INVOKABLE void hideNewestImage() {
        m_actions ^= Action::HideNewest;
        update();
    }
    Q_INVOKABLE void showHiddenImages() {
        m_actions ^= Action::ShowHidden;
        update();
    }
    Q_INVOKABLE void clearImages() {
        m_actions ^= Action::RemoveAll;
        update();
    }
    Q_INVOKABLE void clearResources() {
        m_actions ^= Action::CleanupResources;
        update();
    }
    Q_INVOKABLE void generateCanvas() {
        m_actions ^= Action::GenerateCanvas;
        update();
    }
    Q_INVOKABLE void unregisterNewestCanvas() {
        m_actions ^= Action::UnregisterNewestCanvas;
        update();
    }
    Q_INVOKABLE void reregisterCanvases() {
        m_actions ^= Action::ReregisterCanvases;
        update();
    }
    Q_INVOKABLE void removeNewestCanvas() {
        m_actions ^= Action::RemoveNewestCanvas;
        update();
    }

    int dataAmount() const { return m_dataAmount; }
    int imageAmount() const { return m_imageAmount; }

    bool showGradient() const { return m_showGradient; }
    void setShowGradient(bool show) {
        if (m_showGradient == show)
            return;
        m_showGradient = show;
        Q_EMIT showGradientChanged();
        update();
    }

    bool animateGradient() const { return m_animateGradient; }
    void setAnimateGradient(bool animate) {
        if (m_animateGradient == animate)
            return;
        m_animateGradient = animate;
        Q_EMIT animateGradientChanged();
        update();
    }

protected:
    QCanvasPainterItemRenderer *createItemRenderer() const override;

signals:
    void dataAmountChanged();
    void imageAmountChanged();
    void showGradientChanged();
    void animateGradientChanged();

private:
    friend class ImageTestingRenderer;
    Actions m_actions;
    int m_dataAmount = 0;
    int m_imageAmount = 0;
    bool m_showGradient = false;
    bool m_animateGradient = false;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ImageTestingItem::Actions)

#endif // IMAGETESTINGITEM_H
