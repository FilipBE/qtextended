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

#ifndef UNISELECT_H
#define UNISELECT_H

#include <qwidget.h>
#include <qstring.h>

class QComboBox;
class CharacterView;
class QScrollArea;

class UniSelect : public QWidget
{
    Q_OBJECT
    friend class CharacterView;
public:
    UniSelect(QWidget *parent, const char *name = 0, Qt::WFlags f = 0);
    ~UniSelect();

    uint character() const;
    QString text() const;

    // uint must be > 0xffff, name must be already translated.
    void addSpecial(uint, const QString &);
    void clearSpecials();
signals:
    void selected(uint);
    void selected(const QString &);

private:
    void ensureVisible(int index);
    QComboBox *mSetSelect;
    CharacterView *mGlyphSelect;
    QScrollArea* mScroller;
};

#endif
