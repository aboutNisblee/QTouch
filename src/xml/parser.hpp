/**
 * \file parser.hpp
 *
 * \date 06.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <QtContainerFwd>
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

typedef QSharedPointer<QXmlSchemaValidator> ValidatorPtr;

ValidatorPtr createValidator(const QString& xsd_path) throw (FileException, XmlException);

bool validate(const QString& xml_path, const ValidatorPtr& validator) throw (FileException, XmlException);

CoursePtr parseCourse(const QString& course_path, const ValidatorPtr& validator, ParseResult* result, QString* warningMessage) throw (FileException, XmlException);

} /* namespace xml */

} /* namespace qtouch */

#endif /* PARSER_HPP_ */
