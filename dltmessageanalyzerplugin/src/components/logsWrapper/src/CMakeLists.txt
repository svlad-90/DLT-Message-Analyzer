set(WRAP_SRC ../api/IFileWrapper.hpp)

DMA_qt_wrap_cpp(PROCESSED_MOCS WRAP_SRC)

add_library(DMA_logsWrapper STATIC
    CDLTFileWrapper.cpp
    CDLTMsgWrapper.cpp
    CDLTMsgDecoder.cpp
    CLogsWrapperComponent.cpp
    IFileWrapper.cpp
    IMsgDecoder.cpp
    IMsgWrapper.cpp
    IDLTLogsWrapperCreator.cpp
    ${PROCESSED_MOCS})

target_link_libraries( DMA_logsWrapper PUBLIC
                       qdlt
                       ${QT_PREFIX}::Widgets
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
