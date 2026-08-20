README for setVertexCount-spiral project

This manual test project exercises QSGGeometry::setVertexCount().

The slider changes the number of vertices. Pressing the button in the
top right animates the number of vertices and toggles between using
setVertexCount() and allocate() to change the number of vertices.

While animating, the resize mode, vertex count, and frame rate are
logged to the console.

CPU utilization is always lower using setVertextCount()

   top -p `pgrep appVertexCount`

Note: This app does not test indexed drawing.
