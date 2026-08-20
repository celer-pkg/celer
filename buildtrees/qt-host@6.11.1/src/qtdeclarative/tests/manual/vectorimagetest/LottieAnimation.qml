import QtQuick
import Qt.labs.lottieqt
import QtQuick.VectorImage

Item {
    width: lottieAnimation.width * (VectorImageManager.scale / 10.0)
    height: lottieAnimation.height * (VectorImageManager.scale / 10.0)
    scale: VectorImageManager.scale / 10.0
    transformOrigin: Item.TopLeft

    LottieAnimation {
        id: lottieAnimation
        textureSize: Qt.size(width * parent.scale, height * parent.scale)
        source: VectorImageManager.currentSource.toString().endsWith("json") ? VectorImageManager.currentSource : ""
        loops: VectorImageManager.looping ? LottieAnimation.Infinite : 1
    }
}
