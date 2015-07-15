set(uuidcorrector_SRCS
	uuidcorrector.cpp
	../entities/course.cpp
	../xml/parser.cpp
	../xml/writer.cpp
)
melp_print_list(uuidcorrector_SRCS "uuidcorrector source files" SEPERATOR HALFINDENT)

qt5_add_resources(uuidcorrector_QRCS
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
)
melp_print_list(uuidcorrector_QRCS "uuidcorrector resource files" SEPERATOR HALFINDENT)

add_executable(uuidcorrector ${uuidcorrector_SRCS} ${uuidcorrector_QRCS})
target_link_libraries(uuidcorrector Qt5::Xml Qt5::XmlPatterns)
