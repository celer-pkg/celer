import  Qt.test
import QtQml

QtObject {
   MyQmlObject.value2: 42
   property list<QtObject> objects: [
   // try to trigger a reallocation in sharedState->attachedObjectParserStatusCallbacks
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 },
   QtObject { MyQmlObject.value2: 42 }
   ]
}
