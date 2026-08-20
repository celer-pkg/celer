// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/QEvent>
#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QRandomGenerator>
#include <QtCore/QSequentialAnimationGroup>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtStateMachine/QAbstractTransition>
#include <QtStateMachine/QState>
#include <QtStateMachine/QStateMachine>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsWidget>

//![15]
class StateSwitchEvent: public QEvent
{
public:
    explicit StateSwitchEvent(int rand) : QEvent(StateSwitchType), m_rand(rand) { }

    static constexpr QEvent::Type StateSwitchType = QEvent::Type(QEvent::User + 256);

    int rand() const { return m_rand; }

private:
    int m_rand;
};
//![15]

//![16]
class QGraphicsRectWidget : public QGraphicsWidget
{
public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override
    {
        painter->fillRect(rect(), Qt::blue);
    }
};
//![16]

class StateSwitchTransition: public QAbstractTransition
{
public:
    explicit StateSwitchTransition(int rand) : QAbstractTransition(), m_rand(rand) { }

protected:
//![14]
    bool eventTest(QEvent *event) override
    {
        return (event->type() == QEvent::Type(StateSwitchEvent::StateSwitchType))
            && (static_cast<StateSwitchEvent *>(event)->rand() == m_rand);
    }
//![14]

    void onTransition(QEvent *) override {}

private:
    int m_rand;
};

//![10]
class StateSwitcher : public QState
{
    Q_OBJECT
public:
    explicit StateSwitcher(QStateMachine *machine) : QState(machine) { }
//![10]

//![11]
    void onEntry(QEvent *) override
    {
        int n;
        while ((n = QRandomGenerator::global()->bounded(m_stateCount) + 1) == m_lastIndex)
        { }
        m_lastIndex = n;
        machine()->postEvent(new StateSwitchEvent(n));
    }
    void onExit(QEvent *) override {}
//![11]

//![12]
    void addState(QState *state, QAbstractAnimation *animation) {
        auto trans = new StateSwitchTransition(++m_stateCount);
        trans->setTargetState(state);
        addTransition(trans);
        trans->addAnimation(animation);
    }
//![12]

private:
    int m_stateCount = 0;
    int m_lastIndex = 0;
};

//![13]
static QState *createGeometryState(QObject *w1, const QRect &rect1, QObject *w2, const QRect &rect2,
                                   QObject *w3, const QRect &rect3, QObject *w4, const QRect &rect4,
                                   QState *parent)
{
    auto result = new QState(parent);
    result->assignProperty(w1, "geometry", rect1);
    result->assignProperty(w2, "geometry", rect2);
    result->assignProperty(w3, "geometry", rect3);
    result->assignProperty(w4, "geometry", rect4);

    return result;
}
//![13]


class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr)
        : QGraphicsView(scene, parent)
    {
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        fitInView(scene()->sceneRect());
        QGraphicsView::resizeEvent(event);
    }
};


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

//![1]
    auto button1 = new QGraphicsRectWidget;
    auto button2 = new QGraphicsRectWidget;
    auto button3 = new QGraphicsRectWidget;
    auto button4 = new QGraphicsRectWidget;
    button2->setZValue(1);
    button3->setZValue(2);
    button4->setZValue(3);
    QGraphicsScene scene(0, 0, 300, 300);
    scene.setBackgroundBrush(Qt::black);
    scene.addItem(button1);
    scene.addItem(button2);
    scene.addItem(button3);
    scene.addItem(button4);
//![1]
    GraphicsView window(&scene);
    window.setFrameStyle(0);
    window.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    window.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    window.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//![2]
    QStateMachine machine;

    auto group = new QState;
    group->setObjectName("group");
    QTimer timer;
    timer.setInterval(1250);
    timer.setSingleShot(true);
    QObject::connect(group, &QState::entered, &timer, QOverload<>::of(&QTimer::start));
//![2]

//![3]
    auto state1 = createGeometryState(button1, QRect(100, 0, 50, 50),
                                      button2, QRect(150, 0, 50, 50),
                                      button3, QRect(200, 0, 50, 50),
                                      button4, QRect(250, 0, 50, 50),
                                      group);
//![3]
    auto state2 = createGeometryState(button1, QRect(250, 100, 50, 50),
                                      button2, QRect(250, 150, 50, 50),
                                      button3, QRect(250, 200, 50, 50),
                                      button4, QRect(250, 250, 50, 50),
                                      group);
    auto state3 = createGeometryState(button1, QRect(150, 250, 50, 50),
                                      button2, QRect(100, 250, 50, 50),
                                      button3, QRect(50, 250, 50, 50),
                                      button4, QRect(0, 250, 50, 50),
                                      group);
    auto state4 = createGeometryState(button1, QRect(0, 150, 50, 50),
                                      button2, QRect(0, 100, 50, 50),
                                      button3, QRect(0, 50, 50, 50),
                                      button4, QRect(0, 0, 50, 50),
                                      group);
    auto state5 = createGeometryState(button1, QRect(100, 100, 50, 50),
                                      button2, QRect(150, 100, 50, 50),
                                      button3, QRect(100, 150, 50, 50),
                                      button4, QRect(150, 150, 50, 50),
                                      group);
    auto state6 = createGeometryState(button1, QRect(50, 50, 50, 50),
                                      button2, QRect(200, 50, 50, 50),
                                      button3, QRect(50, 200, 50, 50),
                                      button4, QRect(200, 200, 50, 50),
                                      group);
//![4]
    auto state7 = createGeometryState(button1, QRect(0, 0, 50, 50),
                                      button2, QRect(250, 0, 50, 50),
                                      button3, QRect(0, 250, 50, 50),
                                      button4, QRect(250, 250, 50, 50),
                                      group);
    group->setInitialState(state1);
//![4]

//![5]
    QParallelAnimationGroup animationGroup;

    auto anim = new QPropertyAnimation(button4, "geometry");
    anim->setDuration(1000);
    anim->setEasingCurve(QEasingCurve::OutElastic);
    animationGroup.addAnimation(anim);
//![5]

//![6]
    auto subGroup = new QSequentialAnimationGroup(&animationGroup);
    subGroup->addPause(100);
    anim = new QPropertyAnimation(button3, "geometry");
    anim->setDuration(1000);
    anim->setEasingCurve(QEasingCurve::OutElastic);
    subGroup->addAnimation(anim);
//![6]

    subGroup = new QSequentialAnimationGroup(&animationGroup);
    subGroup->addPause(150);
    anim = new QPropertyAnimation(button2, "geometry");
    anim->setDuration(1000);
    anim->setEasingCurve(QEasingCurve::OutElastic);
    subGroup->addAnimation(anim);

    subGroup = new QSequentialAnimationGroup(&animationGroup);
    subGroup->addPause(200);
    anim = new QPropertyAnimation(button1, "geometry");
    anim->setDuration(1000);
    anim->setEasingCurve(QEasingCurve::OutElastic);
    subGroup->addAnimation(anim);

//![7]
    auto stateSwitcher = new StateSwitcher(&machine);
    stateSwitcher->setObjectName("stateSwitcher");
    group->addTransition(&timer, &QTimer::timeout, stateSwitcher);
    stateSwitcher->addState(state1, &animationGroup);
    stateSwitcher->addState(state2, &animationGroup);
//![7]
    stateSwitcher->addState(state3, &animationGroup);
    stateSwitcher->addState(state4, &animationGroup);
    stateSwitcher->addState(state5, &animationGroup);
    stateSwitcher->addState(state6, &animationGroup);
//![8]
    stateSwitcher->addState(state7, &animationGroup);
//![8]

//![9]
    machine.addState(group);
    machine.setInitialState(group);
    machine.start();
//![9]

    window.resize(300, 300);
    window.show();

    return app.exec();
}

#include "main.moc"
