/**
 * \file parser.hpp
 *
 * \date 06.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <memory>
#include <QXmlSchemaValidator>

#include "entities/course.hpp"
#include "utils/exceptions.hpp"

class QString;

namespace qtouch
{

/**
 * Namespace that offers some free functions used to manipulate
 * course and keyboard layout XML files from ktouch.
 */
namespace xml
{

enum ParseResult { Ok, InvalidId };

std::unique_ptr<QXmlSchemaValidator> createValidator(const QString& xsd_path);

bool validate(const QString& xml_path, const QXmlSchemaValidator& validator);

std::shared_ptr<Course> parseCourse(const QString& course_path, const QXmlSchemaValidator& validator, ParseResult* result, QString* warningMessage);

} /* namespace xml */

} /* namespace qtouch */

#endif /* PARSER_HPP_ */
