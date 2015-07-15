/**
 * \file writer.hpp
 *
 * \date 09.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef WRITER_HPP_
#define WRITER_HPP_

#include <QtContainerFwd>

#include "entities/course.hpp"
#include "utils/exceptions.hpp"

namespace qtouch
{
namespace xml
{

void writeCourse(const CoursePtr& course, const QString& file_path) throw (FileException, XmlException);

} /* namespace xml */
} /* namespace qtouch */

#endif /* WRITER_HPP_ */
