// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QDebug>
#include <QtCore/QMimeData>
#include <QtGui/QDrag>
#include <QtGui/QMouseEvent>
#include <QtGui/QPixmap>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace Qt::Literals;

class DraggableLabel : public QLabel
{
public:
    explicit DraggableLabel(const QString &text, QWidget *parent = nullptr) : QLabel(text, parent)
    {
        setFrameShape(QFrame::Panel);
        setFrameShadow(QFrame::Raised);
        setMinimumHeight(40);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        qDebug() << "DraggableLabel: Mouse Press Event";
        m_dragStartPosition = event->pos();
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        qDebug() << "DraggableLabel: Mouse Move Event";

        if ((event->pos() - m_dragStartPosition).manhattanLength()
            < QApplication::startDragDistance())
            return;

        qDebug() << "DraggableLabel: Starting drag...";

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setText(this->text());
        drag->setMimeData(mimeData);

        QPixmap pixmap = this->grab();
        drag->setPixmap(pixmap);
        drag->setHotSpot(event->pos() - this->rect().topLeft());

        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }

private:
    QPoint m_dragStartPosition;
};

class DragDropWidget : public QWidget
{
public:
    explicit DragDropWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAcceptDrops(true);
        setMouseTracking(true); // Needed for mouseMoveEvent without button press
        setMinimumSize(200, 200);
        setAutoFillBackground(true);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, Qt::lightGray);
        setPalette(pal);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        qDebug() << "DropArea: Mouse Press Event at:" << event->pos() << "with button"
                 << event->button() << event->buttons();
        m_dragStartPosition = event->pos();

        QWidget::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        qDebug() << "DropArea: Mouse Move Event at:" << event->pos();
        if (!(event->buttons() & Qt::LeftButton))
            return;
        if ((event->pos() - m_dragStartPosition).manhattanLength()
            < QApplication::startDragDistance())
            return;

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setText(u"This is a test drag from the drop area."_s);
        drag->setMimeData(mimeData);

        qDebug() << "DropArea: Starting drag...";
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        qDebug() << "DropArea: Mouse Release Event at:" << event->pos() << "with button"
                 << event->button() << event->buttons();
        QWidget::mouseReleaseEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent *event) override
    {
        qDebug() << "DropArea: Mouse Double Click Event at:" << event->pos();
        QWidget::mouseDoubleClickEvent(event);
    }

    void wheelEvent(QWheelEvent *event) override
    {
        qDebug() << "DropArea: Wheel Event with delta:" << event->angleDelta();
        QWidget::wheelEvent(event);
    }

    void dragEnterEvent(QDragEnterEvent *event) override
    {
        qDebug() << "DropArea: Drag Enter Event";
        if (event->mimeData()->hasText())
            event->acceptProposedAction();
    }

    void dragMoveEvent(QDragMoveEvent *event) override
    {
        qDebug() << "DropArea: Drag Move Event at:" << event->position();
        event->accept();
    }

    void dragLeaveEvent(QDragLeaveEvent *event) override
    {
        qDebug() << "DropArea: Drag Leave Event";
        event->accept();
    }

    void dropEvent(QDropEvent *event) override
    {
        qDebug() << "DropArea: Drop Event at:" << event->position() << "with buttons"
                 << event->buttons();
        if (event->mimeData()->hasText()) {
            qDebug() << "DropArea: Dropped text:" << event->mimeData()->text();
            event->acceptProposedAction();
        }
    }

private:
    QPoint m_dragStartPosition;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMainWindow w;

    QWidget *centralWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    DraggableLabel *draggableLabel = new DraggableLabel(u"Drag This Label!"_s);

    QLabel *infoLabel = new QLabel(u"Drag the label above into the grey drop area below. You can "
                                   u"also drag from the grey area itself."_s);
    infoLabel->setWordWrap(true);

    DragDropWidget *dragDropWidget = new DragDropWidget;

    layout->addWidget(draggableLabel); // Add the draggable label
    layout->addWidget(infoLabel);
    layout->addWidget(dragDropWidget);

    w.setCentralWidget(centralWidget);
    w.setWindowTitle(u"Qt Drag and Drop Test"_s);
    w.show();

    return a.exec();
}
