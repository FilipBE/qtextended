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

#ifndef SOFTKEYBAR_H
#define SOFTKEYBAR_H

#include "gfxcanvas.h"

class SoftKeyBar : public QObject, public GfxCanvasItem
{
    Q_OBJECT
public:
    SoftKeyBar(const QSize &, GfxCanvasItem *parent);
    virtual ~SoftKeyBar();

    enum Key { Left, Middle, Right };
    void setLabel(Key, const QString &);
    void press(Key key);
    void release(Key key);

    static void setLabel(GfxCanvasItem *, Key, const QString &);
    static void remove(GfxCanvasItem *);
private slots:
    void focusChanged();

private:
    QSize _s;
    GfxTimeLine tl;
    GfxCanvasImage *left;
    GfxCanvasImage *middle;
    GfxCanvasImage *right;

    QString nleft;
    QString nmiddle;
    QString nright;

    QString lleft;
    QString lmiddle;
    QString lright;


    void change(Key key);
    friend class SKBValue;

    class SKBValue : public GfxValue
    {
    public:
        SKBValue(SoftKeyBar::Key key, SoftKeyBar *p)
            : _key(key), _p(p) {}

    virtual void setValue(qreal v) {
        if(v == 2.)
            _p->change(_key);
    }

    private:
        SoftKeyBar::Key _key;
        SoftKeyBar *_p;
    };

    SKBValue lc, mc, rc;

    static QSet<SoftKeyBar *> keyBars;
    static QHash<GfxCanvasItem *, QStringList> keyMap;
};

#endif
