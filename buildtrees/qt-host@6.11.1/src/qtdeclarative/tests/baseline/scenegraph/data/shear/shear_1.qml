import QtQuick

Rectangle {
    width: 480
    height: 240
    property int standardWidth: 60
    property int standardHeight: 60
    property int standardSpacing: 60
    property bool smoothing: true
    Grid{
        id: grid_0000
        anchors.top: parent.baseline
        anchors.left: parent.left
        anchors.margins: 30
        columns: 4
        spacing: standardSpacing
        Rectangle{ color: "red"; width: standardWidth; height: standardHeight; transform: Shear { origin.x: standardWidth/2; origin.y: standardHeight/2; xAngle: 45 } smooth: smoothing}
        Rectangle{ color: "orange"; width: standardWidth; height: standardHeight; transform: Shear { origin.x: standardWidth/2; origin.y: standardHeight/2; yAngle: 45 } smooth: smoothing }
        Rectangle{ color: "yellow"; width: standardWidth; height: standardHeight; transform: Shear { origin.x: standardWidth/2; origin.y: standardHeight/2; xFactor: 1; }  smooth: smoothing }
        Rectangle{ color: "blue"; width: standardWidth; height: standardHeight; transform: Shear { origin.x: standardWidth/2; origin.y: standardHeight/2; yFactor: 1; }  smooth: smoothing }
        Rectangle{ color: "green"; width: standardWidth; height: standardHeight; transform: Shear { origin.x: standardWidth/2; origin.y: standardHeight/2; yFactor: 0.5; yAngle: 25 }  smooth: smoothing }
        Rectangle{ color: "indigo"; width: standardWidth; height: standardHeight; transform: Shear { origin.x: standardWidth/2; origin.y: standardHeight/2; xFactor: 0.5; yAngle: 25 }  smooth: smoothing }
        Rectangle{ color: "violet"; width: standardWidth; height: standardHeight; transform: Shear { origin.x: standardWidth/2; origin.y: standardHeight/2; xFactor: 0.5; yFactor: 0.5 }  smooth: smoothing }
        Rectangle{ color: "light green"; width: standardWidth; height: standardHeight; transform: Shear { origin.x: standardWidth/2; origin.y: standardHeight/2; xAngle: 25; yAngle: 25 }  smooth: smoothing }
    }
}
