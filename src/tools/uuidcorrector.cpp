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
 * \file uuidcorrector.cpp
 *
 * \date 05.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QIODevice>

#include <QDebug>

#include "xml/parser.hpp"
#include "xml/writer.hpp"

//Apart from determining if the variant bits are set correctly, and that the Time value used in a time-based UUID is in the
//future (and therefore not yet assignable), there is no mechanism for determining if a UUID is valid in any real sense, as
//all possible values can otherwise occur.

/* Pickup arguments:
 * /
 * / /
 * ../resources/courses/course.xsd ../resources/courses
 */

namespace
{

enum CommandLineParseResult
{
	CommandLineOk, CommandLineError, CommandLineVersionRequested, CommandLineHelpRequested
};

struct Config
{
	bool simulation;
	bool inplace;
	bool verbose;
	QString xsdFile;
	QString xmlDir;
};

// *INDENT-OFF*
struct CourseFile
{
	CourseFile(std::shared_ptr<qtouch::Course> c, const QString& p) : course(c), filePath(p) {}
	std::shared_ptr<qtouch::Course> course;
	QString filePath;
};
// *INDENT-ON*

QTextStream& qStdOut()
{
	static QTextStream std(stdout);
	return std;
}

CommandLineParseResult parseCommandLine(QCommandLineParser& parser, Config* config, QString* errorMessage)
{
	const QCommandLineOption simulateOption(QStringList() << "s" << "simulate", "Simulate");
	parser.addOption(simulateOption);

	const QCommandLineOption inPlaceOption(QStringList() << "i" << "inplace", "Make changes in place");
	parser.addOption(inPlaceOption);

	const QCommandLineOption verboseOption(QStringList() << "V" << "verbose", "Be verbose");
	parser.addOption(verboseOption);

	parser.addPositionalArgument("schema", "XML Schema Definition file");
	parser.addPositionalArgument("sources", "Source directory");

	const QCommandLineOption helpOption = parser.addHelpOption();
	const QCommandLineOption versionOption = parser.addVersionOption();

	if (!parser.parse(QCoreApplication::arguments()))
	{
		*errorMessage = parser.errorText();
		return CommandLineError;
	}

	if (parser.isSet(versionOption))
		return CommandLineVersionRequested;

	if (parser.isSet(helpOption))
		return CommandLineHelpRequested;

	config->simulation = parser.isSet(simulateOption);
	config->inplace = parser.isSet(inPlaceOption);
	config->verbose = parser.isSet(verboseOption);

	const QStringList args = parser.positionalArguments();

	if (args.size() != 2)
	{
		*errorMessage = "Invalid number of arguments.";
		return CommandLineError;
	}

	config->xsdFile = args.at(0);
	if (!QFile(config->xsdFile).exists())
	{
		*errorMessage = QStringLiteral("Unable to find XML Schema Definition File at: ") % config->xsdFile;
		return CommandLineError;
	}

	config->xmlDir = args.at(1);
	if (!QDir(config->xmlDir).exists())
	{
		*errorMessage = QStringLiteral("Invalid source directory: ") % config->xmlDir;
		return CommandLineError;
	}

	return CommandLineOk;
}

} /* anonymous namespace */

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral("uuidcorrector"));
	QCoreApplication::setApplicationVersion(QStringLiteral("v1.0") % " (Qt " % QT_VERSION_STR % ")");

	// Parse options
	QCommandLineParser parser;
	parser.setApplicationDescription(
	    QStringLiteral("This tool is able to correct invalid UUIDs in the course files downloaded from the ktouch repository."));

	Config c;
	QString errorMsg;

	switch (parseCommandLine(parser, &c, &errorMsg))
	{
	case CommandLineOk:
		break;
	case CommandLineError:
		fputs(qPrintable(errorMsg), stderr);
		fputs("\n\n", stderr);
		fputs(qPrintable(parser.helpText()), stderr);
		return 1;
	case CommandLineVersionRequested:
		printf("%s %s\n", qPrintable(QCoreApplication::applicationName()),
		       qPrintable(QCoreApplication::applicationVersion()));
		return 0;
	case CommandLineHelpRequested:
		parser.showHelp();
		Q_UNREACHABLE();
	}

	QDir coursepath(c.xmlDir, "*.xml", QDir::Name | QDir::IgnoreCase, QDir::Files);

	QStringList coursefiles;
	for (const auto& s : coursepath.entryList())
	{
		coursefiles.append(coursepath.filePath(s));
	}

	if (c.verbose)
	{
		qStdOut() << "Course files found in " << c.xmlDir << " :" << "\n";
		for (const auto& s : coursefiles)
		{
			qStdOut() << "    " << s << "\n";
		}
	}

	// Create a validator
	std::unique_ptr<QXmlSchemaValidator> validator;
	try
	{
		validator = qtouch::xml::createValidator(c.xsdFile);
	}
	catch (qtouch::Exception& e)
	{
		qCritical() << e.message();
		return 1;
	}

	std::vector<std::shared_ptr<CourseFile>> candidates;

	for (QStringList::const_iterator it = coursefiles.begin(); it != coursefiles.end(); ++it)
	{
		qtouch::xml::ParseResult error;
		QString message;

		try
		{
			auto course = qtouch::xml::parseCourse(*it, *validator, &error, &message);

			if (qtouch::xml::InvalidId == error)
			{
				if (c.verbose)
				{
					qStdOut() << "Invalid ID in " << *it << "\n";
					qStdOut() << message;
				}

				auto c = std::make_shared<CourseFile>(course, *it);
				candidates.push_back(c);
			}
		}
		catch (qtouch::Exception& e)
		{
			qCritical() << e.message();
			return 1;
		}
	}

	for (const auto& candidate : candidates)
	{
		if (c.simulation)
		{
			qStdOut() << "Simulation: Would write correct course " << candidate->course->getTitle() << " to " << candidate->filePath
			          << "\n";
			continue;
		}

		// Copy the old file to .bak if inplace not set
		if (!c.inplace)
		{
			// If there is already an old backup file, delete it
			if (QFile(QString(candidate->filePath).append(".bak")).exists())
			{
				qStdOut() << "Deleting old backup file at \"" << QString(candidate->filePath).append(".bak") << "\"\n";
				if (!QFile(QString(candidate->filePath).append(".bak")).remove())
				{
					qCritical() << "Unable to delete backup file ...";
					return 1;
				}
			}

			if (!QFile(candidate->filePath).rename(QString(candidate->filePath).append(".bak")))
			{
				qCritical() << "Unable to rename original file at" << candidate->filePath << "to" << QString(
				                candidate->filePath).append(".bak");
				return 1;
			}
		}

		// Write the corrected data out
		try
		{
			qtouch::xml::writeCourse(*candidate->course, candidate->filePath);
		}
		catch (qtouch::Exception& e)
		{
			qCritical() << e.message();
			return 1;
		}

		qStdOut() << "Corrected course data have been written to " << QString(candidate->filePath) << "\n";
	}

	return 0;
}

