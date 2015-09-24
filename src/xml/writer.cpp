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
