set(WRAP_SRC ../api/IRegexHistoryProvider.hpp
    CRegexHistoryProvider.hpp
    ../api/CRegexHistoryTextEdit.hpp)

DMA_qt_wrap_cpp(PROCESSED_MOCS WRAP_SRC)

add_library(DMA_regexHistory STATIC
    CRegexHistoryComponent.cpp
    CRegexHistoryProvider.cpp
    IRegexHistoryProvider.cpp
    CRegexHistoryTextEdit.cpp
    ${PROCESSED_MOCS})

target_link_libraries( DMA_regexHistory PUBLIC
                       ${QT_PREFIX}::Widgets
                       DMA_settings
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
