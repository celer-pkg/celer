// The extra import shuffles the type IDs around, so that we
// get a different ID for the attached properties. If the attached
// properties aren't properly cleared, this will crash.

import QtQml.StateMachine 1.0
import QtQuick 2.2
Item { KeyNavigation.up: null }
