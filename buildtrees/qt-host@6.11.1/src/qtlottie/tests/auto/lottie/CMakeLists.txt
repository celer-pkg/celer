add_subdirectory(shape)

if(TARGET Qt::QuickPrivate
   AND TARGET Qt::QuickShapesPrivate
   AND TARGET Qt::QuickVectorImageGeneratorPrivate)
    add_subdirectory(vectorimage)
endif()
