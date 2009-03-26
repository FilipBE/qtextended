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

#ifndef CONTEXTLABEL_H
#define CONTEXTLABEL_H

#include <qtopiaglobal.h>
#include <themedview.h>
#include "qtopiainputevents.h"
#include "qsoftmenubarprovider.h"
#include "qabstractcontextlabel.h"

class QTimer;
class QSettings;



class BaseContextLabel : public QAbstractContextLabel, public QtopiaKeyboardFilter
{
    Q_OBJECT
public:
    BaseContextLabel( QWidget *parent=0, Qt::WFlags f=0 );
    ~BaseContextLabel();
    virtual QSize reservedSize() const = 0;

    class BaseButton
    {
    public:
        BaseButton(int key ) : m_key(key)
        {
            m_changed = false;
        };

        int  key() { return m_key; };
        bool changed() { return m_changed; };
        void setChanged(bool c) { m_changed = c; };

    private:
        int m_key;
        bool m_changed;
    };

signals:
    void buttonsChanged();
protected:
    virtual bool filter(int unicode, int keycode, int modifiers, bool press,
                        bool autoRepeat);
    QSoftMenuBarProvider *softMenuProvider() const;
protected slots:
    void keyChanged(const QSoftMenuBarProvider::MenuButton &);
    void buttonPressed(int);
    void buttonReleased(int);
protected:
    QList<BaseButton*> baseButtons() const;


private:
    QList<BaseButton*> baseBtns;
    QSoftMenuBarProvider *menuProvider;
};

#endif

