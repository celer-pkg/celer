pragma Strict

import QtQml
import TestTypes

QtObject {
    property FinalProperty p: FinalProperty {}
    property int f: p.f
}
