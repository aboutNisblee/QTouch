melp_add_sources(SRCS
	db.cpp
)

set(DB_TEST_SRCS
	db_test.cpp
	db.cpp
	../entities/course.cpp
	../xml/parser.cpp
)

qt5_add_resources(DB_TEST_QRCS ${CMAKE_SOURCE_DIR}/resources/resources.qrc)

melp_add_test_executable(db_test ${DB_TEST_SRCS} ${DB_TEST_QRCS}
LIBS Qt5::Test Qt5::Sql Qt5::Xml Qt5::XmlPatterns)
