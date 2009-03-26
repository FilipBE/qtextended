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

#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>

#include <phonon/objectdescription.h>
#include <phonon/backendinterface.h>


namespace Phonon
{

namespace qtopiamedia
{

class BackendPrivate;

class Backend : public QObject, public Phonon::BackendInterface
{
    Q_OBJECT
    Q_INTERFACES(Phonon::BackendInterface)

public:
    Backend(QObject* parent = 0, const QVariantList& args = QVariantList());
    ~Backend();

    // Phonon::BackendInterface
    QObject* createObject(BackendInterface::Class c,
                            QObject* parent,
                            const QList<QVariant>& args = QList<QVariant>());
    QList<int> objectDescriptionIndexes(ObjectDescriptionType type) const;
    QHash<QByteArray, QVariant> objectDescriptionProperties(ObjectDescriptionType type, int index) const;
    bool startConnectionChange(QSet<QObject *>);
    bool connectNodes(QObject *, QObject *);
    bool disconnectNodes(QObject *, QObject *);
    bool endConnectionChange(QSet<QObject *>);
    QStringList availableMimeTypes() const;

signals:
    void objectDescriptionChanged(ObjectDescriptionType type);

private:
    BackendPrivate* d;
};

}   // qtopiamedia

}   // Phonon

#endif  // BACKEND_H

