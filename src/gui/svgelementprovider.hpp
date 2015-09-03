/**
 * \file svgelementprovider.hpp
 *
 * \date 03.09.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef SVGELEMENTPROVIDER_HPP
#define SVGELEMENTPROVIDER_HPP

#include <QQuickImageProvider>

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
    explicit SvgElementProvider(QQmlImageProviderBase::ImageType type = QQmlImageProviderBase::Image, const QUrl& base = QUrl(QStringLiteral("qrc:///")));
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

#endif // SVGELEMENTPROVIDER_HPP
