melp_add_sources(SRCS
	parser.cpp
)

set(PARSER_TEST_SRCS
	parser_test.cpp
	parser.cpp
	../utils/course.cpp
)

qt5_add_resources(PARSER_TEST_QRCS ${CMAKE_SOURCE_DIR}/resources/resources.qrc)

melp_add_test_executable(parser_test ${PARSER_TEST_SRCS} ${PARSER_TEST_QRCS}
LIBS Qt5::Test Qt5::Xml Qt5::XmlPatterns)
