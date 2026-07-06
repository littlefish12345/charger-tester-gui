set(SOURCES
    main.cpp
    app/MainWindow.h
    app/MainWindow.cpp
    app/pages/MonitoringPage.h
    app/pages/MonitoringPage.cpp
    app/pages/ProtocolAnalysisPage.h
    app/pages/ProtocolAnalysisPage.cpp
    app/pages/LoadControlPage.h
    app/pages/LoadControlPage.cpp
    app/pages/SettingsPage.h
    app/pages/SettingsPage.cpp
    io/SerialPortConfig.h
    io/SerialPortManager.h
    io/SerialPortManager.cpp
    io/SerialWorker.h
    io/SerialWorker.cpp
    protocol/ChargerTypes.h
    protocol/ProtocolParser.h
    protocol/ProtocolParser.cpp
    protocol/CommandBuilder.h
    protocol/CommandBuilder.cpp
    widgets/LcdPanelWidget.h
    widgets/LcdPanelWidget.cpp
    widgets/ProtocolStatusBar.h
    widgets/ProtocolStatusBar.cpp
    widgets/MonitoringChart.h
    widgets/MonitoringChart.cpp
    utils/ThemeUtils.h
    utils/ThemeUtils.cpp
)

set(ELAWIDGETTOOLS_ROOT ${CMAKE_SOURCE_DIR}/third_party/ElaWidgetTools)
set(ELAWIDGETTOOLS_INCLUDE_DIR ${ELAWIDGETTOOLS_ROOT}/ElaWidgetTools/include)
set(ELAWIDGETTOOLS_LIB_DIR ${ELAWIDGETTOOLS_ROOT}/ElaWidgetTools/lib)

add_library(ElaWidgetTools SHARED IMPORTED)
set_target_properties(ElaWidgetTools PROPERTIES
    IMPORTED_LOCATION ${ELAWIDGETTOOLS_LIB_DIR}/libElaWidgetTools.dylib
    INTERFACE_INCLUDE_DIRECTORIES ${ELAWIDGETTOOLS_INCLUDE_DIR}
)
target_link_directories(ElaWidgetTools INTERFACE ${ELAWIDGETTOOLS_LIB_DIR})
target_link_libraries(ElaWidgetTools INTERFACE Qt6::Widgets)

add_executable(charger_tester ${SOURCES})
target_include_directories(charger_tester PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
if(APPLE)
    target_link_options(charger_tester PRIVATE
        "-F${CMAKE_SOURCE_DIR}/third_party/stub_frameworks"
    )
endif()
target_link_libraries(charger_tester PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort
    ElaWidgetTools
)
