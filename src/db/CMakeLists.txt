melp_add_sources(SRCS
	dbv1.cpp
	dbhelper.cpp
)

set(DBV1_TEST_SRCS
	dbv1.cpp
	dbv1_test.cpp
	../entities/course.cpp
	../xml/parser.cpp
)

qt5_add_resources(DB_TEST_QRCS ${CMAKE_SOURCE_DIR}/resources/resources.qrc)

melp_add_test_executable(dbv1_test ${DBV1_TEST_SRCS} ${DB_TEST_QRCS}
LIBS Qt5::Test Qt5::Sql Qt5::Xml Qt5::XmlPatterns)

set(DBHELPER_TEST_SRCS
	dbv1.cpp
	dbhelper.cpp
	dbhelper_test.cpp
	../entities/course.cpp
	../xml/parser.cpp
)

melp_add_test_executable(dbhelper_test ${DBHELPER_TEST_SRCS} ${DB_TEST_QRCS}
LIBS Qt5::Test Qt5::Sql Qt5::Xml Qt5::XmlPatterns)
