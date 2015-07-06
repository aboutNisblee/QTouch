# Just to show qml files in QtCreator
# Files are ignored by CMake when passed to add_executable()
# See: https://bugreports.qt.io/browse/QTCREATORBUG-3166
file(GLOB QMLSRCS qml/*.qml qml/*.js qml/items/*.qml)
melp_add_sources(SRCS ${QMLSRCS})
