import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    spacing: 10
    height: 300

    // TO-DO: Add a test case for autoSuggest property
    // SearchField {
    //     autoSuggest: true
    // }

    SearchField {
        searchIndicator.indicator: null
    }

    SearchField {
        clearIndicator.indicator: null
        text: "Search"
    }

    SearchField {
        text: "Type to search"
    }

    SortFilterProxyModel {
        id: modelFilter
        model: ListModel {
            ListElement { color: "blue" }
            ListElement { color: "green" }
            ListElement { color: "red" }
            ListElement { color: "yellow" }
            ListElement { color: "orange" }
            ListElement { color: "purple" }
        }
        sorters: [
            RoleSorter {
                roleName: "color"
            }
        ]
        filters: [
            FunctionFilter {
                component CustomData: QtObject { property string color }
                property var regExp: new RegExp(searchField.text, "i")
                onRegExpChanged: invalidate()
                function filter(data: CustomData): bool {
                   return regExp.test(data.color);
                }
            }
        ]
    }

    SearchField {
        id: searchField
        suggestionModel: modelFilter
    }
}
