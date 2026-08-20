import QtQuick
import QtGraphs

Rectangle {
    width: 800
    height: 600

    GraphsView {
        theme: GraphsTheme {
            colorScheme: GraphsTheme.ColorScheme.Dark
            theme: GraphsTheme.Theme.MixSeries
        }

        anchors.fill: parent
        axisX: BarCategoryAxis { categories: ["2023", "2024", "2025", "2026"] }
        axisY: ValueAxis { max: 8 }

        BarSeries {
            seriesColors: ["#d0d040", "#d04040"]
            borderColors: ["#808020", "#802020"]
            BarSet { id: set1; label: "Axel"; values: [1, 2, 3, 4] }
            BarSet { id: set2; label: "Bob"; values: [4, 3, 2, 1] }
        }

        LineSeries {
            XYPoint { x: 0; y: 6.6 }
            XYPoint { x: 0.6; y: 4.1 }
            XYPoint { x: 1.5; y: 5.3 }
            XYPoint { x: 2.2; y: 7.1 }
            XYPoint { x: 3.3; y: 6.9 }
            XYPoint { x: 3.6; y: 5.0 }
            XYPoint { x: 4.0; y: 5.3 }
        }

        ScatterSeries {
            XYPoint { x: 0; y: 2.6 }
            XYPoint { x: 0.2; y: 3.1 }
            XYPoint { x: 1.3; y: 6.3 }
            XYPoint { x: 2.4; y: 5.1 }
            XYPoint { x: 3.5; y: 6.9 }
            XYPoint { x: 3.6; y: 5.2 }
            XYPoint { x: 4.0; y: 3.3 }
        }

        AreaSeries {
            upperSeries: LineSeries {
                XYPoint { x: 1; y: 5 }
                XYPoint { x: 1.25; y: 6 }
                XYPoint { x: 1.75; y: 5.5 }
                XYPoint { x: 2; y: 6 }
                XYPoint { x: 2.375; y: 5.5 }
                XYPoint { x: 2.875; y: 6 }
                XYPoint { x: 3.375; y: 5 }
            }

            lowerSeries: LineSeries {
                XYPoint { x: 1; y: 4 }
                XYPoint { x: 1.25; y: 4.5 }
                XYPoint { x: 1.75; y: 4 }
                XYPoint { x: 1.875; y: 3.5 }
                XYPoint { x: 2.375; y: 4 }
                XYPoint { x: 3; y: 4.5 }
                XYPoint { x: 3.375; y: 4 }
            }
        }
    }
}
