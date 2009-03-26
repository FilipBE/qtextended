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

#ifndef PINYINIM_H
#define PINYINIM_H
#include <qwindowsystem_qws.h>
#include <inputmethodinterface.h>
#include <wordpicker.h>
#include <symbolpicker.h>
#include <QDawg>
#include <QStack>
#include <QMultiHash>

class PinyinMatch
{
public:
    PinyinMatch(const QString &path, const QDawg::Node * node);
    ~PinyinMatch();

    QString path() const { return currentpath; }
    const QDawg::Node * node() const { return currentnode; }

private:
    QString currentpath;                //letters used to get to the current node (including letter of current node)
    const QDawg::Node * currentnode;    //position in the dawg
};


class PinyinIM : public QWSInputMethod
{
    Q_OBJECT
public:
    PinyinIM();
    ~PinyinIM();

    void reset();
    using QWSInputMethod::filter; // Don't hide these symbols!
    bool filter(int unicode, int keycode, int modifiers, 
			    bool isPress, bool autoRepeat);

    void setActive(bool);
    bool active() const { return isActive; }
    
    void setMicroFocus( int x, int y );
    virtual void queryResponse( int property, const QVariant & result );

private slots:
    void pinyinSelected(const QString &s);
    void charSelected(const QString &s);
    void symbolSelected(int unicode, int keycode);
    
private:
    bool loadPy2Uni();
    void processDigitKey(int keycode);
    
    bool isActive;
    int microX;
    int microY;
    
    QHash<int,QString> key2let;
    QDawg pinyinDawg;                   //dawg of all possible pinyin syllables
    const QDawg::Node * rootNode;       //root of pinyinDawg
    QMultiHash<QString,QChar> py2uni;
    QStack< QList<PinyinMatch> > matchHistory;
    QPointer<WordPicker> pinyinPicker;
    QPointer<WordPicker> charPicker;
    QPointer<SymbolPicker> symbolPicker;
};

#endif
