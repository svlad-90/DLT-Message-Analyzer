set(WRAP_SRC ../api/ISettingsManager.hpp)

DMA_qt_wrap_cpp(PROCESSED_MOCS WRAP_SRC)

add_library(DMA_settings STATIC
    CSettingsManager.cpp
    ISettingItem.cpp
    CSettingItem.cpp
    ISettingsManager.cpp
    CSettingsComponent.cpp
    CSettingsManagerClient.cpp
    ${PROCESSED_MOCS})

target_link_libraries( DMA_settings PUBLIC
                       qdlt
                       ${QT_PREFIX}::Widgets
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
