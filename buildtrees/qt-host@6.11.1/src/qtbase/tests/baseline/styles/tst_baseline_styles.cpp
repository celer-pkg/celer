// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qbaselinetest.h>
#include <qwidgetbaselinetest.h>
#include <QtWidgets>

class tst_QStyles : public QWidgetBaselineTest
{
    Q_OBJECT

public:
    tst_QStyles() = default;

private slots:
    void initTestCase_data();

    void drawPrimitive_data();
    void drawPrimitive();

    void drawControl_data();
    void drawControl();

    void drawComplexControl_data();
    void drawComplexControl();

private:
    void doInit() override;

    template <typename E>
    void addData();

    template <typename Func, typename StyleOption>
    void testStyle(Func&& func, StyleOption *styleOption, const char *name);
};

#define QBASELINE_CHECK_STYLE(name) \
    testStyle([&](auto *style, auto *styleOption, auto *painter, QWidget *widget) { \
        (style->*drawFunction)(styleElement, styleOption, painter, widget); \
    }, styleOption.get(), name);

void tst_QStyles::initTestCase_data()
{
    // FIXME: Check all styles via global data when baseline tests support global tags
}

void tst_QStyles::doInit()
{
    testWindow()->resize(140, 140);
}

void tst_QStyles::drawPrimitive_data()
{
    addData<QStyle::PrimitiveElement>();
}

QStyleOption *styleOptionFor(QStyle::PrimitiveElement element)
{
    switch (element) {
    case QStyle::PE_FrameTabBarBase:
        return new QStyleOptionTabBarBase();
    case QStyle::PE_IndicatorHeaderArrow:
        return new QStyleOptionHeader();
    case QStyle::PE_IndicatorCheckBox:
    case QStyle::PE_PanelButtonCommand:
        return new QStyleOptionFocusRect();
    case QStyle::PE_FrameFocusRect:
        return new QStyleOptionFocusRect();
    case QStyle::PE_Frame:
    case QStyle::PE_FrameMenu:
    case QStyle::PE_PanelMenuBar:
    case QStyle::PE_FrameGroupBox:
    case QStyle::PE_FrameDockWidget:
    case QStyle::PE_PanelLineEdit:
    case QStyle::PE_FrameLineEdit:
    case QStyle::PE_FrameWindow:
    case QStyle::PE_FrameButtonBevel:
    case QStyle::PE_FrameButtonTool:
        return new QStyleOptionFrame();
    case QStyle::PE_FrameTabWidget:
        return new QStyleOptionTabWidgetFrame();
    case QStyle::PE_IndicatorTabTear:
        return new QStyleOptionTab();
    case QStyle::PE_IndicatorColumnViewArrow:
    case QStyle::PE_PanelItemViewRow:
    case QStyle::PE_PanelItemViewItem:
    case QStyle::PE_IndicatorItemViewItemCheck:
        return new QStyleOptionViewItem();
    default:
        return new QStyleOption();
    };
}

void tst_QStyles::drawPrimitive()
{
    QFETCH(QStyle::PrimitiveElement, styleElement);

    auto drawFunction = &QStyle::drawPrimitive;

    std::unique_ptr<QStyleOption> styleOption(styleOptionFor(styleElement));
    styleOption->initFrom(testWindow());
    QBASELINE_CHECK_STYLE("default");

    if (auto *headerOption = qstyleoption_cast<QStyleOptionHeader*>(styleOption.get())) {
        headerOption->sortIndicator = QStyleOptionHeader::SortDown;
        QBASELINE_CHECK_STYLE("down");

        headerOption->sortIndicator = QStyleOptionHeader::SortUp;
        QBASELINE_CHECK_STYLE("up");
    }
}

void tst_QStyles::drawControl_data()
{
    addData<QStyle::ControlElement>();
}

QStyleOption *styleOptionFor(QStyle::ControlElement element)
{
    switch (element) {
    case QStyle::CE_PushButton:
    case QStyle::CE_PushButtonBevel:
    case QStyle::CE_PushButtonLabel:
    case QStyle::CE_RadioButton:
    case QStyle::CE_RadioButtonLabel:
    case QStyle::CE_CheckBox:
    case QStyle::CE_CheckBoxLabel: {
        auto *buttonStyle = new QStyleOptionButton;
        buttonStyle->text = QString("Hello");
        return buttonStyle;
    }
    case QStyle::CE_MenuBarItem:
        return new QStyleOptionMenuItem;
    case QStyle::CE_ProgressBar:
    case QStyle::CE_ProgressBarLabel:
    case QStyle::CE_ProgressBarContents:
        return new QStyleOptionProgressBar;
    case QStyle::CE_Header:
    case QStyle::CE_HeaderLabel:
    case QStyle::CE_HeaderSection:
        return new QStyleOptionHeader;
    case QStyle::CE_ToolButtonLabel:
        return new QStyleOptionToolButton;
    case QStyle::CE_ToolBoxTab:
    case QStyle::CE_ToolBoxTabShape:
    case QStyle::CE_ToolBoxTabLabel:
        return new QStyleOptionToolBox;
    case QStyle::CE_TabBarTab:
    case QStyle::CE_TabBarTabShape:
    case QStyle::CE_TabBarTabLabel:
        return new QStyleOptionTab;
    case QStyle::CE_RubberBand:
        return new QStyleOptionRubberBand;
    case QStyle::CE_DockWidgetTitle:
        return new QStyleOptionDockWidget;
    case QStyle::CE_ComboBoxLabel:
        return new QStyleOptionComboBox;
    case QStyle::CE_ToolBar:
        return new QStyleOptionToolBar;
    case QStyle::CE_ItemViewItem:
        return new QStyleOptionViewItem;
    case QStyle::CE_ShapedFrame:
        return new QStyleOptionFrame;
    default:
        return new QStyleOption;
    };
}

void tst_QStyles::drawControl()
{
    QFETCH(QStyle::ControlElement, styleElement);

    auto drawFunction = &QStyle::drawControl;

    std::unique_ptr<QStyleOption> styleOption(styleOptionFor(styleElement));
    styleOption->state = QStyle::State_Active
        | QStyle::State_Enabled
        | QStyle::State_HasFocus
        | QStyle::State_On;

    QBASELINE_CHECK_STYLE("default");
}

void tst_QStyles::drawComplexControl_data()
{
    addData<QStyle::ComplexControl>();
}

QStyleOptionComplex *styleOptionFor(QStyle::ComplexControl element)
{
    switch (element) {
    case QStyle::CC_Slider:
    case QStyle::CC_ScrollBar:
        return new QStyleOptionSlider();
    case QStyle::CC_ToolButton:
        return new QStyleOptionToolButton();
    case QStyle::CC_SpinBox:
        return new QStyleOptionSpinBox();
    case QStyle::CC_TitleBar:
        return new QStyleOptionTitleBar();
    case QStyle::CC_ComboBox:
        return new QStyleOptionComboBox();
    case QStyle::CC_GroupBox:
        return new QStyleOptionGroupBox();
    default:
        return new QStyleOptionComplex();
    };
}

void tst_QStyles::drawComplexControl()
{
    QFETCH(QStyle::ComplexControl, styleElement);

    auto drawFunction = &QStyle::drawComplexControl;

    std::unique_ptr<QStyleOptionComplex> styleOption(styleOptionFor(styleElement));
    styleOption->initFrom(testWindow());
    styleOption->subControls = QStyle::SC_All;

    QBASELINE_CHECK_STYLE("default");
}

template <typename E>
void tst_QStyles::addData()
{
    auto metaEnum = QMetaEnum::fromType<E>();
    QTest::addColumn<E>("styleElement");

    for (int i = 0; i < metaEnum.keyCount() - 1; ++i) {
        auto value = E(metaEnum.value(i));
        QTest::newRow(metaEnum.key(i)) << value;
    }
}

template <typename Func, typename StyleOption>
void tst_QStyles::testStyle(Func&& drawFunc, StyleOption *styleOption, const char *name)
{
    QStyle *style = qApp->style();
    QWidget *w = testWindow();

    const auto dpr = w->devicePixelRatio();
    const auto size = w->size();

    QImage image(size * dpr, QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(dpr);
    QPainter painter(&image);

    QBrush checkerboardBackground = w->parentWidget()->palette().brush(QPalette::Window);
    painter.fillRect(QRect({}, size), checkerboardBackground);

    styleOption->rect = w->rect() -= QMargins(20, 20, 20, 20);

    drawFunc(style, styleOption, &painter, w);
    painter.end();

    if (!debugRects.isEmpty()) {
        QJsonDocument doc(debugRects);
        image.setText("DebugRects", doc.toJson(QJsonDocument::Compact));
    }

    QBASELINE_CHECK(image, name);
}

QBASELINETEST_MAIN(tst_QStyles)

#include "tst_baseline_styles.moc"
