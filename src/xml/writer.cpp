/**
 * \file writer.cpp
 *
 * \date 09.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "writer.hpp"

#include <QFile>
#include <QXmlStreamWriter>
#include <QDomDocument>
#include <QDomElement>

#include <QDebug>

namespace qtouch
{
namespace xml
{

/**
 * Write a course file.
 * @param course A Course.
 * @param file_path The target file path.
 * @throw FileException, XmlException
 */
void writeCourse(const Course& course, const QString& file_path)
{
	QFile output(file_path);
	if (!output.open(QIODevice::WriteOnly))
	{
		throw FileException("Cannot open file for writing", output.fileName());
	}

	QXmlStreamWriter writer(&output);
	writer.setAutoFormatting(true);
	writer.setAutoFormattingIndent(1);

	writer.writeStartDocument();

	writer.writeStartElement("course");

	writer.writeStartElement("id");
	writer.writeCharacters(course.getId().toString());
	writer.writeEndElement();

	writer.writeStartElement("title");
	writer.writeCharacters(course.getTitle());
	writer.writeEndElement();

	writer.writeStartElement("description");
	writer.writeCharacters(course.getDescription());
	writer.writeEndElement();

	writer.writeStartElement("keyboardLayout");
	// TODO: Add keyboardLayout
	writer.writeEndElement();

	writer.writeStartElement("lessons");
	for (const auto& it :course)
	{
		writer.writeStartElement("lesson");

		writer.writeStartElement("id");
		writer.writeCharacters(it->getId().toString());
		writer.writeEndElement();

		writer.writeStartElement("title");
		writer.writeCharacters(it->getTitle());
		writer.writeEndElement();

		writer.writeStartElement("newCharacters");
		writer.writeCharacters(it->getNewChars());
		writer.writeEndElement();

		writer.writeStartElement("text");
		writer.writeCharacters(it->getText());
		writer.writeEndElement();

		writer.writeEndElement(); // </lesson>
	}
	writer.writeEndElement(); // </lessons>

	writer.writeEndElement(); // </course>

	writer.writeEndDocument();
}

} /* namespace xml */
} /* namespace qtouch */
