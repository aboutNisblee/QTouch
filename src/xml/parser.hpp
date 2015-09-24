/* Copyright (C) 2015  Moritz Nisblé <moritz.nisble@gmx.de>
 *
 * This file is part of QTouch.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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
