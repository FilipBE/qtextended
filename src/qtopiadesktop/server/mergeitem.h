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
#ifndef MERGEITEM_H
#define MERGEITEM_H

#include <QByteArray>
#include <QString>

/*
   Because the structures for pim data types are taken from XML this is a tree like structure.

   There are a number of differences though.  Any list of any kind is assumed to be unordered and
   will be sorted internally in the structure.  Also identifier elements are extracted and stored separately.
*/

class MergeElement;
class QXmlStreamWriter;
class QSyncMerge;
class MergeItem
{
public:
    enum ChangeSource {
        Server,
        Client,
        IdentifierOnly,
        DataOnly,
    };

    MergeItem(QSyncMerge *);
    ~MergeItem();

    /* future intent
       reading server/client as if from storage stub/client device.
       writing server/client as if to send to storage stub/client device.
       writing dataonly suitable for finding identical item clashs
       writing identifieronly includes both server and client ids.

       intent, read dataonly followed by identifier only for whole
       merge item.  Currently identifieronly overwrites data only,
       however architecture will allow memory reduction later
       without changing the API
       */
    QByteArray write(ChangeSource) const;
    bool read(const QByteArray &,ChangeSource);

    void restrictTo(const MergeItem &);

    QString serverIdentifier() const;
    QString clientIdentifier() const;

    QString dump() const;

    // may need to do mappings....
private:
    // attempt for now is to disable copying.  Can implement later
    MergeItem(const MergeItem &) {}
    MergeElement *rootElement;
    QSyncMerge *mMerge; // for id mapping.

    void writeElement(QXmlStreamWriter &, MergeElement *item, ChangeSource) const;
};

#endif
