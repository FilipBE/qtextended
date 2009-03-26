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

#include "servercontactmodel.h"
#include <QSettings>


/*!
    \class ServerContactModel
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
    \ingroup QtopiaServer
    \brief The ServerContactModel class provides access to a singleton instance of a QContactModel.

    The model extends the QContactModel to read the Trolltech/Contacts settings on construction to include the same sources of contact data as configured in the Qt Extended Contacts application.

    This class is part of the Qt Extended server and cannot be used by other applications.
*/

/*!
  Constructs a new ServerContactModel.
*/
ServerContactModel::ServerContactModel()
    : QContactModel()
{
    readSettings();
}

/*!
  Reads settings information for the contact model.  This includes
  the set of contact sources as configured in the Contacts application.
*/
void ServerContactModel::readSettings()
{
    QSettings config( "Trolltech", "Contacts" );

    // load SIM/No SIM settings.
    config.beginGroup( "default" );
    if (config.contains("SelectedSources/size")) {
        int count = config.beginReadArray("SelectedSources");
        QSet<QPimSource> set;
        for(int i = 0; i < count; ++i) {
            config.setArrayIndex(i);
            QPimSource s;
            s.context = QUuid(config.value("context").toString());
            s.identity = config.value("identity").toString();
            set.insert(s);
        }
        config.endArray();
        setVisibleSources(set);
    }
}

/*!
  Destroys the ServerContactModel.
  */
ServerContactModel::~ServerContactModel()
{
}

ServerContactModel *ServerContactModel::mInstance = 0;

/*!
  Returns the instance of a ServerContactModel.  If an instance
  has not yet been constructed a new ServerContactModel will be
  constructed.
*/
ServerContactModel *ServerContactModel::instance()
{
    if (!mInstance)
        mInstance = new ServerContactModel();

    return mInstance;
}
