:: Copyright (C) 2020 The Qt Company Ltd.
:: SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

:: For HLSL we invoke fxc.exe (-c argument) and store the resulting intermediate format
:: instead of HLSL source, so this needs to be run on Windows from a developer command prompt.

qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o opacitymask.frag.qsb opacitymask.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o opacitymask_invert.frag.qsb opacitymask_invert.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o fastblur.frag.qsb fastblur.frag
qsb -b --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o fastblur_internal.vert.qsb fastblur_internal.vert
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o fastblur_internal.frag.qsb fastblur_internal.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o fastinnershadow.frag.qsb fastinnershadow.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o fastinnershadow_level0.frag.qsb fastinnershadow_level0.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o fastglow.frag.qsb fastglow.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o colorize.frag.qsb colorize.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o brightnesscontrast.frag.qsb brightnesscontrast.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o coloroverlay.frag.qsb coloroverlay.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o desaturate.frag.qsb desaturate.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o displace.frag.qsb displace.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o gammaadjust.frag.qsb gammaadjust.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o gaussianinnershadow.frag.qsb gaussianinnershadow.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o gaussianinnershadow_shadow.frag.qsb gaussianinnershadow_shadow.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o huesaturation.frag.qsb huesaturation.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o rectangularglow.frag.qsb rectangularglow.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o thresholdmask.frag.qsb thresholdmask.frag
qsb -b --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o lineargradient.vert.qsb lineargradient.vert
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o lineargradient_nomask.frag.qsb lineargradient_nomask.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o lineargradient_mask.frag.qsb lineargradient_mask.frag
qsb -b --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o radialgradient.vert.qsb radialgradient.vert
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o radialgradient_nomask.frag.qsb radialgradient_nomask.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o radialgradient_mask.frag.qsb radialgradient_mask.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o recursiveblur.frag.qsb recursiveblur.frag
qsb -b --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o recursiveblur.vert.qsb recursiveblur.vert
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o conicalgradient_nomask.frag.qsb conicalgradient_nomask.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o conicalgradient_mask.frag.qsb conicalgradient_mask.frag
qsb --glsl "150,120,100 es" --hlsl 50 -c --msl 12 -o leveladjust.frag.qsb leveladjust.frag
