// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import { RegularExpression, Wildcard } from 'objectmaphelper.js';

export var widgetWidget = {"name": "Widget", "type": "Widget", "visible": 1};
export var widgetGroupBox2QGroupBox = {"name": "groupBox_2", "type": "QGroupBox", "visible": 1, "window": widgetWidget};
export var groupBox2RedQRadioButton = {"container": widgetGroupBox2QGroupBox, "name": "red", "type": "QRadioButton", "visible": 1};
export var groupBox2GreenQRadioButton = {"container": widgetGroupBox2QGroupBox, "name": "green", "type": "QRadioButton", "visible": 1};
export var groupBox2YellowQRadioButton = {"container": widgetGroupBox2QGroupBox, "name": "yellow", "type": "QRadioButton", "visible": 1};
export var widgetViewQGraphicsView = {"name": "view", "type": "QGraphicsView", "visible": 1, "window": widgetWidget};
export var widgetGroupBoxQGroupBox = {"name": "groupBox", "type": "QGroupBox", "visible": 1, "window": widgetWidget};
export var groupBoxGroupQPushButton = {"container": widgetGroupBoxQGroupBox, "name": "group", "type": "QPushButton", "visible": 1};
export var viewQGraphicsRectItem = {"acceptDrops": "no", "container": widgetViewQGraphicsView, "enabled": "yes", "fillColor": "#50c850", "focusable": "no", "lineColor": "#000000", "movable": "yes", "selectable": "yes", "type": "QGraphicsRectItem", "visible": "yes"};
export var viewQGraphicsRectItem_2 = {"acceptDrops": "no", "container": widgetViewQGraphicsView, "enabled": "yes", "fillColor": "#c85050", "focusable": "no", "lineColor": "#000000", "movable": "yes", "selectable": "yes", "type": "QGraphicsRectItem", "visible": "yes"};
export var viewQGraphicsRectItem_3 = {"acceptDrops": "no", "container": widgetViewQGraphicsView, "enabled": "yes", "fillColor": "#c8c814", "focusable": "no", "lineColor": "#000000", "movable": "yes", "selectable": "yes", "type": "QGraphicsRectItem", "visible": "yes"};
export var viewQGraphicsItemGroup = {"acceptDrops": "no", "container": widgetViewQGraphicsView, "enabled": "yes", "focusable": "no", "lineColor": "#000000", "movable": "yes", "selectable": "yes", "type": "QGraphicsItemGroup", "visible": "yes"};
export var qGraphicsItemGroup = {"acceptDrops": "no", "container": viewQGraphicsItemGroup, "enabled": "yes", "focusable": "no", "lineColor": "#000000", "movable": "yes", "occurrence": 2, "selectable": "yes", "type": "QGraphicsItemGroup", "visible": "yes"};
export var qGraphicsRectItem = {"acceptDrops": "no", "container": qGraphicsItemGroup, "enabled": "yes", "fillColor": "#c8c814", "focusable": "no", "lineColor": "#000000", "movable": "yes", "selectable": "yes", "type": "QGraphicsRectItem", "visible": "yes"};
export var groupBoxMergeQPushButton = {"container": widgetGroupBoxQGroupBox, "name": "merge", "type": "QPushButton", "visible": 1};
export var groupBoxDismantleQPushButton = {"container": widgetGroupBoxQGroupBox, "name": "dismantle", "type": "QPushButton", "visible": 1};
export var viewQGraphicsRectItem_4 = {"acceptDrops": "no", "container": widgetViewQGraphicsView, "enabled": "yes", "fillColor": "#5050c8", "focusable": "no", "lineColor": "#000000", "movable": "yes", "selectable": "yes", "type": "QGraphicsRectItem", "visible": "yes"};
export var groupBox2UngroupQPushButton = {"container": widgetGroupBox2QGroupBox, "name": "ungroup", "type": "QPushButton", "visible": 1};
export var groupBox2BlueQRadioButton = {"container": widgetGroupBox2QGroupBox, "name": "blue", "type": "QRadioButton", "visible": 1};
export var groupBox2ScaleItemQSlider = {"container": widgetGroupBox2QGroupBox, "name": "scaleItem", "type": "QSlider", "visible": 1};
export var groupBoxRotateQSlider = {"container": widgetGroupBoxQGroupBox, "name": "rotate", "type": "QSlider", "visible": 1};
export var groupBoxScaleQSlider = {"container": widgetGroupBoxQGroupBox, "name": "scale", "type": "QSlider", "visible": 1};
export var qGraphicsRectItem_2 = {"acceptDrops": "no", "container": viewQGraphicsItemGroup, "enabled": "yes", "fillColor": "#50c850", "focusable": "no", "lineColor": "#000000", "movable": "yes", "selectable": "yes", "type": "QGraphicsRectItem", "visible": "yes"};
export var groupBox2RotateItemQSlider = {"container": widgetGroupBox2QGroupBox, "name": "rotateItem", "type": "QSlider", "visible": 1};
