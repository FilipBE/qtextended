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

#ifndef MEDIANODE_H
#define MEDIANODE_H


class QMediaContent;


namespace Phonon
{

namespace qtopiamedia
{

class MediaNode
{
public:
    virtual ~MediaNode() {}
    virtual bool connectNode(MediaNode* node) = 0;
    virtual bool disconnectNode(MediaNode* node) = 0;
    virtual void setContent(QMediaContent* content) = 0;
};

}

}

Q_DECLARE_INTERFACE(Phonon::qtopiamedia::MediaNode, "org.phonon.qtopiamedia.MediaNode")

#endif  // MEDIANODE_H
