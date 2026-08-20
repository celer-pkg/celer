// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasdebugcountervisualizer_p.h"
#include "qcanvaspainter_p.h"

QT_BEGIN_NAMESPACE

QCanvasDebugCounterVisualizer::QCanvasDebugCounterVisualizer()
    : m_debugNsElapsed(0)
    , m_debugCounter(0)
    , m_debugMsElapsed(QLatin1String("0.000"))
{
}

void QCanvasDebugCounterVisualizer::start()
{
    m_debugTimer.start();
}

void QCanvasDebugCounterVisualizer::paint(QCanvasPainter *painter, float width, float height, const QCDebugCounters &debugCounters)
{
    if (!painter)
        return;

    qint64 elapsed = m_debugTimer.nsecsElapsed();
    m_debugNsElapsed += elapsed;
    m_debugCounter++;
    if (!m_debugUpdateTimer.isValid())
        m_debugUpdateTimer.start();

    // How often time is updated, in seconds
    // Longer period increases accuracy
    const int UPDATE_FREQUENCY_MS = 1000;
    if (m_debugUpdateTimer.elapsed() >= UPDATE_FREQUENCY_MS) {
        qint64 ms = 1000000;
        double msElapsed = double(m_debugNsElapsed) / (ms * m_debugCounter);
        m_debugMsElapsed = QString::number(msElapsed, 'f', 3);
        m_debugNsElapsed = 0;
        m_debugCounter = 0;
        m_debugUpdateTimer.start();
    }
    //float fontSize = qMin(QCanvasPainter::ptToPx(14), width*0.04f);
    float fontSize = 14.0f;
    float margin = fontSize * 0.2f;
    float debugHeight = fontSize * 7.0f + margin * 3.0f;
    float debugWidth = qMin(width, 28 * fontSize);
    float debugY = height - debugHeight;
    // Note: Returning these state settings back
    // shouldn't be needed as this debug is painted
    // as the very last thing.
    painter->reset();
    painter->resetClipping();

    // Background
    painter->setFillStyle(QColor::fromRgba(0xB0000000));
    painter->fillRect(0, debugY, debugWidth, debugHeight);

    // Texts
    static const QString debugText1 = QStringLiteral(u"DRAWCALLS:");
    static const QString debugText2 = QStringLiteral(u"FILL");
    static const QString debugText3 = QStringLiteral(u"STROKE");
    static const QString debugText4 = QStringLiteral(u"TEXT");
    static const QString debugText5 = QStringLiteral(u"TOTAL");
    static const QString debugText6 = QStringLiteral(u"TRIANGLES:");
    QFont font;
    font.setPixelSize(fontSize);
    painter->setFont(font);
    painter->setTextAlign(QCanvasPainter::TextAlign::Left);
    painter->setTextBaseline(QCanvasPainter::TextBaseline::Top);
    painter->setFillStyle(0xFFFFFFFF);
    // Draw calls and triangles table
    float textY = debugY;
    float cellWidth = debugWidth / 6.0f;
    painter->fillText(debugText2, margin + 2.0f * cellWidth, textY);
    painter->fillText(debugText3, margin + 3.0f * cellWidth, textY);
    painter->fillText(debugText4, margin + 4.0f * cellWidth, textY);
    painter->fillText(debugText5, margin + 5.0f * cellWidth, textY);
    textY += fontSize + margin;
    painter->fillText(debugText1, margin, textY);
    painter->fillText(QString::number(debugCounters.fillDrawCallCount),
                      margin + 2.0f * cellWidth, textY);
    painter->fillText(QString::number(debugCounters.strokeDrawCallCount),
                      margin + 3.0f * cellWidth, textY);
    painter->fillText(QString::number(debugCounters.textDrawCallCount),
                      margin + 4.0f * cellWidth, textY);
    painter->fillText(QString::number(debugCounters.drawCallCount),
                      margin + 5.0f * cellWidth, textY);
    textY += fontSize + margin;
    painter->fillText(debugText6, margin, textY);
    painter->fillText(QString::number(debugCounters.fillTriangleCount),
                      margin + 2.0f * cellWidth, textY);
    painter->fillText(QString::number(debugCounters.strokeTriangleCount),
                      margin + 3.0f * cellWidth, textY);
    painter->fillText(QString::number(debugCounters.textTriangleCount),
                      margin + 4.0f * cellWidth, textY);
    painter->fillText(QString::number(debugCounters.triangleCount),
                      margin + 5.0f * cellWidth, textY);
    // Textures and timing info
    textY += (fontSize + margin) * 2;
    QString debugText7 = QStringLiteral("IMAGES: %1 (%2 kB); TIME: %3 ms")
                             .arg(debugCounters.imageCount).arg(debugCounters.imageMemoryUsage).arg(m_debugMsElapsed);
    painter->fillText(debugText7, margin, textY);
    textY += fontSize + margin;
    QString debugText8 = QStringLiteral("PATH CACHE: %1 groups, %2 subpaths (%3 kB)")
                             .arg(debugCounters.pathGroupCount).arg(debugCounters.cachedSubpathCount).arg(debugCounters.cachedPathVertexDataSize / 1000);
    painter->fillText(debugText8, margin, textY);
}

QT_END_NAMESPACE
