pragma Strict
import QML

QtObject {
    id: self
    objectName: "me, myself, and I"

    property QtObject delayedShadowable: QtObject {
        objectName: "delayedShadowable"
    }

    property QtObject immediateShadowable: QtObject {
        objectName: "immediateShadowable"
    }

    property QtObject delayedDestroyable: QtObject {
        id: destroyDelayed
    }

    property QtObject immediateDestroyable: QtObject {
        id: destroyNow
    }

    property QtObject scopedImmediateDestroyable: QtObject {
        id: scopedImmediate
        function destroySelf() { destroy() }
    }

    property QtObject scopedDelayedDestroyable: QtObject {
        id: scopedDelayed
        function destroySelf() { destroy(20) }
    }

    property string stringed: toString()
    property string selfStringed: self.toString()
    property string immediateShadowableStringed: immediateShadowable.toString()
    property string delayedShadowableStringed: delayedShadowable.toString();

    function explode() {
        delayedShadowable.destroy(20);
        immediateShadowable.destroy();
        destroyDelayed.destroy(20);
        destroyNow.destroy();
        scopedImmediate.destroySelf();
        scopedDelayed.destroySelf();
    }

    property QtObject overrides: QtObject {
        function toString() : string {
            return "yes";
        }

        function destroy() {
            self.objectName = "no";
        }
    }

    function callOverridden() : string {
        // toString() can be overridden, destroy() not.
        let result = overrides.toString();
        overrides.destroy();
        return result;
    }
}
