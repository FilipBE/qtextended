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

#ifndef WEBLITEIMG_H
#define WEBLITEIMG_H
#include "webliteclient.h"
#include <Qtopia>

class WebLiteImgPrivate;
class WebLiteImg : public WebLiteClient
{
    Q_OBJECT
    public:
        WebLiteImg(QObject* o = NULL);
        virtual ~WebLiteImg();
        
        void setScale (const QSize &);
        QSize scale () const;
        void setAutoLoad (bool);
        bool autoLoad() const;
        
        QImage image () const;
        QPixmap pixmap () const;
        
        void setLabel (QLabel* lbl);
        QLabel* label ();

    signals:
        void decoded ();            
    private:
        WebLiteImgPrivate* d;
};


#endif
