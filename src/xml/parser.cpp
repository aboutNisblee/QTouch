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
 * \file parser.cpp
 *
 * \date 06.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "parser.hpp"

#include <QStringList>
#include <QFile>

#include <QXmlSchema>

#include <QDomDocument>
#include <QDomElement>

#include <QDebug>

namespace qtouch
{
namespace xml
{

std::unique_ptr<QXmlSchemaValidator> createValidator(const QString& xsd_path)
{
	QFile xsd(xsd_path);

	if (!xsd.open(QIODevice::ReadOnly))
		throw FileException("Cannot open schema definition file", xsd.fileName());

	QXmlSchema schema;

	/* Memory leak!
	 * 2912 bytes are definitely lost
	 * despite API usage just like explained in documentation
	 * qthelp://org.qt-project.qtxmlpatterns.542/qtxmlpatterns/qxmlschema.html
	 * Is completely independent from whether the file lives as long as the
	 * schema or closed after load.
	 * Also using "bool QXmlSchema::load(const QUrl & source)" makes no
	 * difference.
	 * TODO: Take a look into the sources and maybe create a bugreport. */
	schema.load(&xsd, QUrl::fromLocalFile(xsd.fileName()));
	if (!schema.isValid())
		throw XmlException("Invalid schema definition file", xsd.fileName());

	std::unique_ptr<QXmlSchemaValidator> v(new QXmlSchemaValidator(schema));
	return v;
}

/**
 * Check a single XML file against is schema definition.
 * @param xml_path Path to the XML file.
 * @param validator A schema validator created by e.g. validator().
 * @throw FileException or XmlException
 * @return True when XML file is valid, else false.
 */
bool validate(const QString& xml_path, const QXmlSchemaValidator& validator)
{
	QFile xml(xml_path);

	if (!xml.open(QIODevice::ReadOnly))
		throw FileException("Cannot open XML file", xml.fileName());

	if (!validator.validate(&xml))
	{
		qWarning() << "Schema validation of \"" << xml.fileName() << "\" failed.";
		return false;
	}

	return true;
}

/**
 * Parse a single course.
 * @param course_path Path to the XML file.
 * @param validator A schema validator created by e.g. validator().
 * @param result The status indicating the success of parsing.
 * @param warningMessage An optional warning message.
 * @throw FileException or XmlException
 * @return A new Course instance.
 */
std::shared_ptr<Course> parseCourse(const QString& course_path, const QXmlSchemaValidator& validator, ParseResult* result,
                      QString* warningMessage)
{
	QFile xml(course_path);

	if (!xml.open(QIODevice::ReadOnly))
	{
		qDebug() << course_path;
		throw FileException("Cannot open XML file", xml.fileName());
	}

	if (!validator.validate(&xml))
		throw XmlException("Schema validation failed", xml.fileName());

	// NOTE: Validator doesn't reset the file!
	xml.reset();

	// Create a Course
	auto course = Course::create();

	QDomDocument dom;
	QString errMsg;
	int errLine = 0;
	int errColumn = 0;

	if (!dom.setContent(&xml, &errMsg, &errLine, &errColumn))
	{
		throw XmlException(QString("Error while parsing \"") % xml.fileName() % "\" at line " % QString::number(
		                       errLine) % " in column " % QString::number(errColumn)
		                   % " : " % errMsg, xml.fileName());
	}

	QDomElement root = dom.documentElement();

	QString text;

	// Reset error variables
	if (result)
		*result = Ok;
	if (warningMessage)
		*warningMessage = QString();

	/* Set the title first, to get a meaningful warning on UUID errors. */

	// Set title
	text = root.firstChildElement("title").text();
	course->setTitle(text);

	// Set ID
	text = root.firstChildElement("id").text();
	if (!course->setId(QUuid(text)))
	{
		if (result)
			*result = InvalidId;
		if (warningMessage)
		{
			*warningMessage += QLatin1String("Invalid Course UUID\n");
			*warningMessage += QLatin1String("    Course:") % course->getTitle() % "\n";
			*warningMessage += QLatin1String("    Generated:") % course->getId().toString() % "\n";
		}
	}

	// Set description
	text = root.firstChildElement("description").text();
	course->setDescription(text);

	// TODO: Add keyboard layout
	//  text = root.firstChildElement("keyboardLayout").text();
	//  qDebug() << "keyboardLayout:" << text;
	//  out->setKeyboardLayout(text);

	// Set builtin flag since only builtin courses come from XML
	course->setBuiltin(true);

	// Add lessons
	for (QDomElement lessonsElem = root.firstChildElement("lessons").firstChildElement(); !lessonsElem.isNull();
	        lessonsElem = lessonsElem.nextSiblingElement())
	{
		Lesson lesson;

		// Set title
		text = lessonsElem.firstChildElement("title").text();
		lesson.setTitle(text);

		// Set ID
		text = lessonsElem.firstChildElement("id").text();
		if (!lesson.setId(text))
		{
			if (result)
				*result = InvalidId;
			if (warningMessage)
			{
				*warningMessage += QLatin1String("Invalid Lesson UUID\n");
				*warningMessage += QLatin1String("    Course:") % course->getTitle() % "\n";
				*warningMessage += QLatin1String("    Lesson:") % lesson.getTitle() % "\n";
				*warningMessage += QLatin1String("    Generated:") % lesson.getId().toString() % "\n";
			}
		}

		// Add new characters
		text = lessonsElem.firstChildElement("newCharacters").text();
		lesson.setNewChars(text);

		// Copy text
		text = lessonsElem.firstChildElement("text").text();
		lesson.setText(text);

		lesson.setBuiltin(true);

		course->push_back(lesson);
	}

	return course;
}

}
/* namespace xml */

} /* namespace qtouch */
