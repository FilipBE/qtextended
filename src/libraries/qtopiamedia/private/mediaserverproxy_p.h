/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef MEDIASERVERPROXY_P_H
#define MEDIASERVERPROXY_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QString>
#include <QMap>
#include <QUuid>
#include <QUrl>


class QMediaHandle;
class QValueSpaceItem;
class QtopiaIpcAdaptor;

namespace mlp
{

class MediaServerCallback
{
public:

    virtual ~MediaServerCallback();
    virtual void mediaReady() = 0;
    virtual void mediaError(QString const& error) = 0;
};

/*!
    \class mlp::MediaServerProxy
    \internal
*/


class MediaServerProxy : public QObject
{
    Q_OBJECT

    typedef QMap<QUuid, MediaServerCallback*>   CallbackMap;

public:
    ~MediaServerProxy();

    // {{{ Info
    QStringList simpleProviderTags() const;
    QStringList simpleMimeTypesForProvider(QString const& providerTag);
    QStringList simpleUriSchemesForProvider(QString const& providerTag);
    // }}}

    // {{{ QMediaContent
    QMediaHandle prepareContent(MediaServerCallback* callback,
                                QString const& domain,
                                QUrl const& url);
    // }}}

    void destroySession(QMediaHandle const& handle);

    static MediaServerProxy* instance();

private slots:
    void sessionCreated(QUuid const& id);
    void sessionError(QUuid const& id, QString const& error);

    void simpleInfoChanged();

private:
    MediaServerProxy();

    void buildCodecList();
    void buildDeviceList();

    QString                 m_channel;
    QValueSpaceItem*        m_simpleInfo;
    CallbackMap             m_callbackMap;
    QtopiaIpcAdaptor*       m_adaptor;
};

}   // ns mlp

#endif
