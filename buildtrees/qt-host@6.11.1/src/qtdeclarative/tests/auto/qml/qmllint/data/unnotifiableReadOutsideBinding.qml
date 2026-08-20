import QtQuick
Item {
  DropArea {
    onDropped: (drop) => {
      console.log(drop.hasUrls)
    }
  }

  function f(drop: DragEvent) {
    console.log(drop.hasUrls)
  }
}
