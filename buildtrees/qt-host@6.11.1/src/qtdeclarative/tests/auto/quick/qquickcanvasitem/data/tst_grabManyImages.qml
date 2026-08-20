import QtQuick 2.0

Item {
    width: 800
    height: 800

    Canvas {
        id: canvas
        renderStrategy: Canvas.Immediate
        anchors.fill: parent
        onPaint: {}
        onAvailableChanged: {
            // must not run out of memory
            for (let i = 0; i < 2000; ++i)
                canvas.getContext("2d").getImageData(0, 0, canvas.width, canvas.height);
        }
    }
}
