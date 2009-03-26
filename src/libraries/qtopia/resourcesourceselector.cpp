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

#include "resourcesourceselector_p.h"

#include <QButtonGroup>
#include <QDSServices>
#include <QDSData>
#include <QDSAction>
#include <QLabel>
#include <QLayout>
#include <QPair>
#include <QPushButton>

#include "qtopialog.h"

//===========================================================================

class ResourceSourceSelectorPrivate
{
public:
    typedef QPair<QString, QDSServices*> ResourceType;
    typedef QList<ResourceType> ResourceTypeList;
    typedef QPair<ResourceType*, int> ResourceId;
    typedef QMap<QAbstractButton*, ResourceId> ServiceMap;
    typedef QPair<bool, QButtonGroup*> GroupType;

    QLabel *label;
    QPushButton *removePB, *changePB;
    ResourceTypeList resourceTypes;
    ServiceMap serviceMap;
    QList<GroupType> serviceGroups;
};

ResourceSourceSelector::ResourceSourceSelector( QWidget *parent )
    : QWidget( parent ),
      d( new ResourceSourceSelectorPrivate() )
{
}

ResourceSourceSelector::~ResourceSourceSelector()
{
    // Delete any service groups we allocated
    foreach ( ResourceSourceSelectorPrivate::ResourceType resourceType, d->resourceTypes )
        delete resourceType.second;

    delete d;
}

void ResourceSourceSelector::init( ArrangementType arrangement, ServicesList* servicesList )
{
    QVBoxLayout *sl = new QVBoxLayout( this );
    QHBoxLayout *hl = new QHBoxLayout();

    d->label = new QLabel( this );
    d->label->setAlignment( Qt::AlignCenter );
    ( arrangement == HorizontalArrangement ? static_cast<QBoxLayout*>( hl ) : sl )->addWidget( d->label );
    sl->addLayout(hl);

    QVBoxLayout *vl = new QVBoxLayout();
    hl->addLayout( vl );

    sl->addStretch();

    // Create the 'change' button
    d->changePB = new QPushButton( this );
    vl->addWidget( d->changePB );

    if ( servicesList )
    {
        // For each service group:
        foreach ( ServicesDescriptor desc, *servicesList ) {
            // Find the services matching the descriptor
            QDSServices* services = new QDSServices( desc.requestType, desc.responseType, desc.attributes );

            // Add to the list of services we need to deallocate later
            QString svcType;
            if ( !desc.attributes.isEmpty() )
                svcType = desc.attributes[0];

            ResourceSourceSelectorPrivate::ResourceType resType = qMakePair( svcType, services );
            ResourceSourceSelectorPrivate::ResourceTypeList::iterator it = d->resourceTypes.insert( d->resourceTypes.end(), resType );
            ResourceSourceSelectorPrivate::ResourceType* typeAddress = &(*it);

            if ( !services->isEmpty() )
            {
                QList<QPushButton*> svcButtonList;

                // See which services are available
                foreach ( QDSServiceInfo serviceInfo, *services ) {
                    if ( serviceInfo.isAvailable() ) {
                        /* Icons not currently used..
                        QIcon icon;
                        if ( !serviceInfo.icon().isEmpty() ) {
                            icon = QIcon( ":icon/" + serviceInfo.icon() );
                        }
                        */
                        svcButtonList.append( new QPushButton( /*icon,*/ serviceInfo.description() ) );
                    }
                }

                if ( !svcButtonList.isEmpty() ) {
                    // Create a button for each service found
                    QButtonGroup* servicesGroup = new QButtonGroup( this );
                    connect( servicesGroup, SIGNAL(buttonClicked(QAbstractButton*)),
                             this, SLOT(serviceRequest(QAbstractButton*)) );

                    // Add each button to the layout and the service button group
                    int serviceId = 0;
                    foreach ( QPushButton* button, svcButtonList ) {
                        vl->addWidget( button );

                        // Add a button for this service
                        servicesGroup->addButton( button, serviceId );

                        // Add the mapping to convert button to matching service info
                        d->serviceMap.insert( button, qMakePair( typeAddress, serviceId ) );
                        ++serviceId;
                    }

                    // Add this button group to our set
                    d->serviceGroups.append( qMakePair( desc.requiresContent, servicesGroup ) );
                }
            }
        }
    }

    // Create the 'remove' button
    d->removePB = new QPushButton( this );
    vl->addWidget( d->removePB );

    vl->addStretch();

    // We have nothing selected initially
    haveResource( false );
}

void ResourceSourceSelector::connectSignals( QObject* handler )
{
    connect( changeButton(), SIGNAL(clicked()),
             handler, SLOT(change()) );

    connect( removeButton(), SIGNAL(clicked()),
             handler, SLOT(remove()) );

    connect( this, SIGNAL(serviceRequest(QString,QDSAction&)),
             handler, SLOT(serviceRequest(QString,QDSAction&)));
}

void ResourceSourceSelector::serviceRequest( QAbstractButton* button )
{
    ResourceSourceSelectorPrivate::ServiceMap::const_iterator it = d->serviceMap.find( button );
    if ( it == d->serviceMap.end() )
        return;

    ResourceSourceSelectorPrivate::ResourceType* resType = (*it).first;
    int index = (*it).second;

    QString svcType = resType->first;
    QDSServices* services = resType->second;

    // Find the service represented by this button
    QDSAction action( (*services)[index] );

    // Let the derived class handle the service action
    emit serviceRequest( svcType, action );
}

void ResourceSourceSelector::haveResource( bool f )
{
    d->removePB->setEnabled( f );

    foreach ( ResourceSourceSelectorPrivate::GroupType groupType, d->serviceGroups )
        if ( groupType.first ) // 'needsResource'
            foreach ( QAbstractButton* button, groupType.second->buttons() )
                button->setEnabled( f );
}

QLabel* ResourceSourceSelector::label()
{
    return d->label;
}

QPushButton* ResourceSourceSelector::changeButton()
{
    return d->changePB;
}

QPushButton* ResourceSourceSelector::removeButton()
{
    return d->removePB;
}

