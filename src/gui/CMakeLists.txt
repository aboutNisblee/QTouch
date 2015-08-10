# Just to show qml files in QtCreator
# Files are ignored by CMake when passed to add_executable()
# See: https://bugreports.qt.io/browse/QTCREATORBUG-3166
file(GLOB QMLSRCS qml/*.qml qml/*.js qml/items/*.qml)
melp_add_sources(SRCS ${QMLSRCS})

melp_add_sources(SRCS
	textpage.cpp
)

## QML Tests
# Add resources
qt5_add_resources(QML_TEST_QRCS ${CMAKE_CURRENT_SOURCE_DIR}/qml.qrc)
qt5_add_resources(QML_TEST_QRCS ${CMAKE_SOURCE_DIR}/resources/resources.qrc)
# Specify test source directory
add_definitions(-DQUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
# Add test
melp_add_test_executable(qml_test qml_test.cpp ${QML_TEST_QRCS} LIBS Qt5::QuickTest)

