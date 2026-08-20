README for setIndexCount-spiral project

This manual test project exercises the QSGGeometry::setIndexCount()
functionality for the purpose of developing and debugging.

It shows a spiral line strip which constains 16000 vertices. A slider
control changes the number of visible vertices by reducing the index
count.

When the handle is in the middle of the slider, all vertices are
visible. Depending on which way the handle is dragged, the vertices
will disappear either from the middle of the spiral or from the outer
edge. The beginning and end of the spiral is colored white so that
is is easy to see when all allocated vertices are visible.
