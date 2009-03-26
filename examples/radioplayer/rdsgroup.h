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

#ifndef RDSGROUP_H
#define RDSGROUP_H

#include <QByteArray>

class RdsGroup
{
public:
    RdsGroup();
    RdsGroup( const RdsGroup& other );
    ~RdsGroup();

    enum Status
    {
        Ok,
        Invalid,
        WaitingForMore
    };

    RdsGroup& operator=( const RdsGroup& other );

    QByteArray data() const { return _data; }
    Status status() const { return _status; }

    void addBlock( const char *data );
    void clear();

    int groupType() const;
    bool isTypeAGroup() const;
    bool isTypeBGroup() const;

    int lsb( int block ) const;
    int msb( int block ) const;
    int word( int block ) const;

private:
    QByteArray _data;
    Status _status;
    int _prevBlock;
};

#endif
