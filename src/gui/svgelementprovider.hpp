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
 * \file svgelementprovider.hpp
 *
 * \date 03.09.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef SVGELEMENTPROVIDER_HPP
#define SVGELEMENTPROVIDER_HPP

#include <QQuickImageProvider>

namespace qtouch
{

/**
 * \brief QQuickImageProvider that is able to extract elements from Scalable Vector Graphics (SVG).
 * After adding an instance of this class to the QML engine, is is possible to load SVG elements into
 * Images. See Qt reference for an usage example.\n
 * The element is specified by a URL fragment identifier (#elementId). If the given URL lacks a fragment identifier,
 * the whole image is loaded.
 */
class SvgElementProvider : public QQuickImageProvider
{
public:
	explicit SvgElementProvider(QQmlImageProviderBase::ImageType type = QQmlImageProviderBase::Image,
	                            const QUrl& base = QUrl(QStringLiteral("qrc:///")));
	virtual ~SvgElementProvider() {}

	virtual QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);
	virtual QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize);

	/**
	 * \brief Set the base URL for all images loaded by this provider.
	 * \param base The base URL (should end with an "/")
	 */
	inline void setBaseUrl(const QUrl& base) { mBaseUrl = base; }

private:
	QUrl mBaseUrl;
};

} /* namespace qtouch */


#endif // SVGELEMENTPROVIDER_HPP
