melp_add_sources(SRCS
	profile.cpp
	course.cpp
	typo.cpp
)

melp_add_test_executable(course_test course_test.cpp course.cpp LIBS Qt5::Test)

melp_add_test_executable(typo_test typo_test.cpp typo.cpp LIBS Qt5::Test)
