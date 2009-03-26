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

#ifndef RESOURCESOURCESELECTOR_P_H
#define RESOURCESOURCESELECTOR_P_H

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

#include <QList>
#include <QMap>
#include <QWidget>

class QAbstractButton;
class QDSAction;
class QLabel;
class QPushButton;

class ResourceSourceSelectorPrivate;

// A base class for widgets implementing resource selection
class ResourceSourceSelector : public QWidget
{
    Q_OBJECT

public:
    enum ServiceType {
        ContentRequired,
        NoContentRequired,
    };

    enum ArrangementType {
        HorizontalArrangement,
        VerticalArrangement,
    };

    // Describe a selection of services to operate on our selected resource
    struct ServicesDescriptor
    {
        ServicesDescriptor( const QStringList& attr, const QString& req, const QString& resp, ServiceType content )
            : attributes( attr ),
              requestType( req ),
              responseType ( resp ),
              requiresContent ( content == ContentRequired ) {}

        QStringList attributes;
        QString requestType;
        QString responseType;
        bool requiresContent;
    };

    typedef QList<ServicesDescriptor> ServicesList;
    typedef QMap<QString, QString> Dictionary;

    ResourceSourceSelector( QWidget *parent );
    ~ResourceSourceSelector();

    QLabel* label();
    QPushButton* changeButton();
    QPushButton* removeButton();

    void haveResource( bool f );

    void init( ArrangementType arrangement, ServicesList* services = 0 );

    template<typename HandlerType>
    void connectSignals( HandlerType* handler );

signals:
    void serviceRequest( const QString& type, QDSAction& action );

private slots:
    void serviceRequest( QAbstractButton* );

private:
    void connectSignals( QObject* handler );

    ResourceSourceSelectorPrivate *d;
};

template<typename HandlerType>
void ResourceSourceSelector::connectSignals( HandlerType* handler )
{
    // The object handling our signals must supply the following slots:
    void (HandlerType::*changeSlotTest)() = &HandlerType::change;
    Q_UNUSED(changeSlotTest)

    void (HandlerType::*removeSlotTest)() = &HandlerType::remove;
    Q_UNUSED(removeSlotTest)

    void (HandlerType::*serviceRequestSlotTest)(const QString&, QDSAction&) = &HandlerType::serviceRequest;
    Q_UNUSED(serviceRequestSlotTest)

    connectSignals( static_cast<QObject*>( handler ) );
}

#endif
