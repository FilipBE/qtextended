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

#ifndef CHARSETEDIT_H
#define CHARSETEDIT_H

#include <qlist.h>
#include <qdialog.h>
#include <profile.h>

#include "ui_charseteditbase.h"

class UniSelect;

class QIMPenInputCharDlg : public QDialog
{
    Q_OBJECT
public:
    QIMPenInputCharDlg( QWidget *parent = 0, const char *name = 0,
            bool modal = false, bool fs = false, Qt::WFlags f = 0 );

    unsigned int unicode() const { return uni; }

protected slots:
    void setCharacter( uint sp );

private:
    void addSpecial(bool isFS );
    uint uni;
    UniSelect *u;
};

class CharSetEdit : public QFrame, private Ui::CharSetEditBase
{
    Q_OBJECT
public:
    CharSetEdit( QWidget *parent=0, const char *name=0 );
    CharSetEdit( QIMPenCharSet *c, QWidget *parent=0,
                const char *name=0 );

    void setCharSet( QIMPenCharSet *c );

    QIMPenCharSet *charSet() const;

    void checkStoreMatch();
    void clearMatch();

    void setIsFS(bool b) {
        mIsFS=b;
    }
protected:
    void fillCharList();
    void enableButtons();

    QIMPenChar *findPrev();
    QIMPenChar *findNext();

    void setCurrentChar( QIMPenChar * );

protected slots:
    void prevMatch();
    void nextMatch();

    void selectCode( int );
    void selectItemCode( QListWidgetItem *item, QListWidgetItem *previous = 0);

    void addChar();
    void removeChar();

    void addMatch();
    void removeMatch();
    void resetMatches();

    void newStroke( QIMPenStroke * );

protected:

    void appendMatch(QListWidgetItem *item = 0);
    void init();
    void updateLabel();
    void firstMatch();
    void lastMatch();

    // current code, not nearly detailed enough eh?
    QIMPenChar *currentChar;
    QIMPenChar *inputChar;
    QIMPenCharSet *currentSet;
    int lastCs;
    int lastCh;
    bool addFlag;
    uint matchCount;
    uint matchIndex;
    bool mIsFS;
};

class CharSetDlg : public QDialog
{
    Q_OBJECT
public:
    CharSetDlg( QWidget *parent = 0, const char *name = 0,
            bool modal = false, Qt::WFlags f = 0 );
    ~CharSetDlg();

    void setCharSet( QIMPenCharSet *c );

    QIMPenCharSet *charSet() const;

    void setIsFS(bool b) {
        edit->setIsFS(b);
    }
protected:
    void accept();
    void reject();

private:
    CharSetEdit *edit;
};

#endif
