add_library(corelib_snippets_eventfilters OBJECT)

set_target_properties(corelib_snippets_eventfilters PROPERTIES COMPILE_OPTIONS "-w")

target_link_libraries(corelib_snippets_eventfilters PRIVATE
    Qt::Core
)

if(QT_FEATURE_widgets)
    target_link_libraries(corelib_snippets_eventfilters PRIVATE
        Qt::Widgets
    )
    target_sources(corelib_snippets_eventfilters PRIVATE
        main.cpp
    )
endif()

if(QT_FEATURE_gui)
    target_link_libraries(corelib_snippets_eventfilters PRIVATE
        Qt::Gui
    )
    target_sources(corelib_snippets_eventfilters PRIVATE
        filterobject.cpp
    )
endif()
