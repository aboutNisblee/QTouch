/**
 * \file parser.cpp
 *
 * \date 06.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "parser.hpp"

#include <QList>
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

namespace
{

QXmlSchema schema(const QString& xsd_path)
{
	QFile xsd(xsd_path);

	if (!xsd.open(QIODevice::ReadOnly))
		throw FileException("Cannot open schema definition file", xsd.fileName());

	QXmlSchema schema;
	schema.load(&xsd, QUrl::fromLocalFile(xsd.fileName()));
	if (!schema.isValid())
		throw XmlException("Invalid schema definition file", xsd.fileName());

	return schema;
}

} /* anonymous namespace */

ValidatorPtr createValidator(const QString& xsd_path) throw (FileException, XmlException)
{
	ValidatorPtr v(new QXmlSchemaValidator(schema(xsd_path)));
	return v;
}

/**
 * Check a single XML file against is schema definition.
 * @param xml_path Path to the XML file.
 * @param validator A schema validator created by e.g. validator().
 * @return True when XML file is valid, else false.
 */
bool validate(const QString& xml_path, const ValidatorPtr& validator) throw (FileException, XmlException)
{
	QFile xml(xml_path);

	if (!xml.open(QIODevice::ReadOnly))
		throw FileException("Cannot open XML file", xml.fileName());

	if (!validator->validate(&xml))
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
 * @return A new Course instance.
 */
CoursePtr parseCourse(const QString& course_path, const ValidatorPtr& validator, ParseResult* result,
                      QString* warningMessage) throw (FileException, XmlException)
{
	QFile xml(course_path);

	if (!xml.open(QIODevice::ReadOnly))
	{
		qDebug() << course_path;
		throw FileException("Cannot open XML file", xml.fileName());
	}

	if (!validator->validate(&xml))
		throw XmlException("Schema validation failed", xml.fileName());

	// NOTE: Validator doesn't reset the file!
	xml.reset();

	// Create a Course
	CoursePtr course = Course::create();

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
		LessonPtr lesson(new Lesson);

		/* Append the lesson before adding any values.
		 * This way the Lesson is able to access its Course and print more
		 * meaningful debug/warning messages. */
		// TODO: This conflicts initialization of the LessonsMap!
		/*course->append(lesson);*/

		// Set title
		text = lessonsElem.firstChildElement("title").text();
		lesson->setTitle(text);

		// Set ID
		text = lessonsElem.firstChildElement("id").text();
		if (!lesson->setId(text))
		{
			if (result)
				*result = InvalidId;
			if (warningMessage)
			{
				*warningMessage += QLatin1String("Invalid Lesson UUID\n");
				*warningMessage += QLatin1String("    Course:") % lesson->getCourse()->getTitle() % "\n";
				*warningMessage += QLatin1String("    Lesson:") % lesson->getTitle() % "\n";
				*warningMessage += QLatin1String("    Generated:") % lesson->getId().toString() % "\n";
			}
		}

		// Add new characters
		text = lessonsElem.firstChildElement("newCharacters").text();
		lesson->setNewChars(text);

		// Copy text
		text = lessonsElem.firstChildElement("text").text();
		lesson->setText(text);

		course->append(lesson);

		lesson->setBuiltin(true);
	}

	return course;
}

}
/* namespace xml */

} /* namespace qtouch */
