add_library(DMA_settings STATIC
    CSettingsManager.cpp
    ISettingItem.cpp
    CSettingItem.cpp)

################### QT ####################################
target_link_libraries(DMA_settings qdlt Qt5::Widgets )
################### QT ( END ) ############################
