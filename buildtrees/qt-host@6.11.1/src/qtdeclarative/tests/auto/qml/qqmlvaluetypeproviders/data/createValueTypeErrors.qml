// only used to ensure that we have a stack frame which can be used for reporting
import QtQml
import Test

QtObject {
   id: root
   required property QtObject runner
   property structured b2
   property rect r

   Component.onCompleted: {
      runner.incompatibleStructuredValue()
      b2 = {i: root.r }
   }
}
