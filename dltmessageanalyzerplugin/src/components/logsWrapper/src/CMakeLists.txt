qt5_wrap_cpp(PROCESSED_MOCS
             ../api/IFileWrapper.hpp)

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

target_link_libraries( DMA_logsWrapper PUBLIC qdlt Qt5::Widgets DMA_framework_base DMA_framework_component )
