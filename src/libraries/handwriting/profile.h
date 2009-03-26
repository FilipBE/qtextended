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

#ifndef PROFILE_H
#define PROFILE_H

#include "char.h"

#include <qtopiaglobal.h>

class QTOPIAHW_EXPORT QIMPenProfile
{
public:
    explicit QIMPenProfile( const QString &fn );
    ~QIMPenProfile();

    const QString name() const { return pname; }
    const QString identifier() const;
    const QString description() const { return pdesc; }

    bool canSelectStyle() const { return tstyle; }
    bool canIgnoreStroke() const { return istyle; }

    enum Style { ToggleCases, BothCases };

    Style style() const { return pstyle; }
    void setStyle( Style s );

    int multiStrokeTimeout() const { return msTimeout; }
    void setMultiStrokeTimeout( int t );

    int ignoreStrokeTimeout() const { return isTimeout; }
    void setIgnoreStrokeTimeout( int t );

    // shouldn't use, overly restricts usage of set
    // returns first char set of this type.
    QIMPenCharSet *uppercase();
    QIMPenCharSet *lowercase();
    QIMPenCharSet *numeric();
    QIMPenCharSet *punctuation();
    QIMPenCharSet *symbol();
    QIMPenCharSet *shortcut();
    QIMPenCharSet *find( QIMPenCharSet::Type t );

    // this is more generic, and translateable.
    QIMPenCharSet *charSet( const QString & ); // internal (not translated)
    QString title( const QString & ); // translated
    QString description( const QString & ); // translated

    QStringList charSets(); // internal (not translated)

    void save() const;
private:
    void load();
    QString userConfig() const;
    void loadData();
    void saveData() const;

private:
    struct ProfileSet {
        ProfileSet() : set(0) {}
        ProfileSet(QIMPenCharSet *s) : set(s) {}
        ~ProfileSet() { delete set; }

        QString id;
        QIMPenCharSet *set;
    };

    typedef QList<ProfileSet *> ProfileSetList;
    typedef QList<ProfileSet *>::iterator ProfileSetListIterator;
    typedef QList<ProfileSet *>::const_iterator ProfileSetListConstIterator;
    ProfileSetList sets;

    QString filename;
    QString pname;
    QString pdesc;
    Style pstyle;
    bool tstyle;
    bool istyle;
    int msTimeout;
    int isTimeout;
};

#endif
