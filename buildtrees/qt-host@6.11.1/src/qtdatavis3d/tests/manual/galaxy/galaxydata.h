// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SCATTERDATAMODIFIER_H
#define SCATTERDATAMODIFIER_H

#include "star.h"

#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/qabstract3dseries.h>
#include <QtGui/QFont>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLabel>

using namespace QtDataVisualization;

class GalaxyData : public QObject
{
    Q_OBJECT
public:
    explicit GalaxyData(Q3DScatter *scatter,
                        qreal rad = 13000,
                        qreal radCore = 4000.0,
                        qreal deltaAng = 0.0004,
                        qreal ex1 = 0.9,
                        qreal ex2 = 0.9);
    ~GalaxyData();

    qreal getExcentricity(qreal r) const;
    qreal getAngularOffset(qreal rad) const;

    void radiusGalaxyChanged(int value);
    void radiusCoreChanged(int value);
    void angleOffsetChanged(int value);
    void eccentricityInnerChanged(int value);
    void eccentricityOuterChanged(int value);
    void resetValues();
    void setFilteredEnabled(bool enabled);
    void setStaticEnabled(bool enabled);
    void setStarsVisible(bool enabled);
    void setDustVisible(bool enabled);
    void setH2Visible(bool enabled);
    inline void setSliders(QSlider *rg,
                           QSlider *rc,
                           QSlider *ao,
                           QSlider *ei,
                           QSlider *eo) {
        m_radiusGalaxySlider = rg;
        m_radiusCoreSlider = rc;
        m_angleOffsetSlider = ao;
        m_eccentricityInnerSlider = ei;
        m_eccentricityOuterSlider = eo;
    }
    void setFpsLabel(QLabel *fpsLabel) { m_fpsLabel = fpsLabel; }
    void handleFpsChange(qreal fps);

private:
    void createGalaxy();
    void checkMinMax(const Star &star);
    void createNormalDataView();
    void createFilteredView();
    void createSeries();
    qreal value;

private:
    Q3DScatter *m_graph;
    QScatter3DSeries *m_normalSeries;
    QScatter3DSeries *m_dustSeries;
    QScatter3DSeries *m_H2Series;
    QScatter3DSeries *m_filteredSeries;
    Star *m_pStars;
    Star *m_pDust;
    Star *m_pH2;

    qreal m_elEx1;          // Excentricity of the innermost ellipse
    qreal m_elEx2;          // Excentricity of the outermost ellipse

    qreal m_angleOffset;    // Angular offset per parsec

    qreal m_radCore;        // Radius of the inner core
    qreal m_radGalaxy;      // Radius of the galaxy
    qreal m_radFarField;    // The radius after which all density waves must have circular shape

    QSlider *m_radiusGalaxySlider;
    QSlider *m_radiusCoreSlider;
    QSlider *m_angleOffsetSlider;
    QSlider *m_eccentricityInnerSlider;
    QSlider *m_eccentricityOuterSlider;
    QLabel *m_fpsLabel;

    qreal m_minx;
    qreal m_maxx;
    qreal m_miny;
    qreal m_maxy;
    int m_range;
    bool m_filtered;
};

#endif
