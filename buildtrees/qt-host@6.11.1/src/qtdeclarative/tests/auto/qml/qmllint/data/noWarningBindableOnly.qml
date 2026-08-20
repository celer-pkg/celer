import PropertyChangeHandlers 1.0
import QtQml

QtObject {
  required property TypeWithProperties withProperties
  property var v: withProperties.a
}
