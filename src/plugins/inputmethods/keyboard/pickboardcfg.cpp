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

#include "pickboardcfg.h"
#include "pickboardpicks.h"

#include <qtopianamespace.h>

#include <QPainter>
#include <QList>
#include <QBitmap>
#include <QLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QMenu>
#include <QPushButton>
#include <QMessageBox>
#include <QScrollArea>
#ifdef Q_WS_QWS
#include <QWSServer>
#endif

const int intermatchmargin=5;


PickboardConfig::~PickboardConfig() { }

void PickboardConfig::updateRows(int from, int to)
{
    if ( from != to ) { // (all)
        parent->update();
    } else {
        QFontMetrics fm = parent->fontMetrics();
        parent->update(QRect(0,1+fm.descent() + from * fm.lineSpacing(), parent->width(),
            fm.lineSpacing()));
    }
}

void PickboardConfig::updateItem(int r, int)
{
    updateRows(r,r);
}

void PickboardConfig::changeMode(int m)
{
    parent->setMode(m);
}
void PickboardConfig::generateText(const QString& s)
{
#if defined(Q_WS_QWS) || defined(Q_WS_QWS)
    for (int i=0; i<(int)s.length(); i++) {
        uint code = 0;
        if ( s[i].unicode() >= 'a' && s[i].unicode() <= 'z' ) {
            code = s[i].unicode() - 'a' + Qt::Key_A;
        }
        qwsServer->sendKeyEvent(s[i].unicode(), code, 0, true, false);
        qwsServer->sendKeyEvent(s[i].unicode(), code, 0, false, false);
    }
#endif
}
void PickboardConfig::generateKey( int uni, int k )
{
    qwsServer->sendKeyEvent(uni, k, 0, true, false);
    qwsServer->sendKeyEvent(uni, k, 0, false, false);
}

void PickboardConfig::pickPoint(const QPoint& p, bool press)
{
    if ( press ) {
        int ls=parent->height()/nrows;
        int y=0;
        pressx = -1;
        for (int r=0; r<nrows; r++) {
            if ( p.y() >= y && p.y() < y+ls ) {
                pressrow = r;
                pressx = p.x();
                pickInRow( pressrow, pressx, true );
                return;
            }
            y += ls;
        }
    } else if ( pressx >= 0 ) {
        pickInRow( pressrow, pressx, false );
        pressx = -1;
    }
}

void PickboardConfig::fillMenu(QMenu& menu)
{
    resetA = menu.addAction(tr("Reset"));
    helpA = menu.addAction(tr("Help"));
    menu.insertSeparator(helpA);
}

void PickboardConfig::reset()
{
    if ( parent->currentMode() ) {
        changeMode(0);
        updateRows(0,1);
    }
}

void PickboardConfig::doMenu(const QAction * i)
{
    if (i == resetA) {
        reset();
    //} else if (i == helpA) {
        //???
        //;
    }
}

void StringConfig::draw(QPainter* p)
{
    QFontMetrics fm = p->fontMetrics();

    for (int r=0; r<nrows; r++) {
        p->translate(0,fm.lineSpacing());
        p->setPen(rowColor(r));

        int tw=0;
        QString s;
        int i=0;
        for (; !(s=text(r,i)).isNull(); ++i) {
            int w = fm.width(s);
            tw += w;
        }
        bool spread = spreadRow(r);// && parent->width() > tw;
        int xw = spread ? (parent->width()-tw)/(i-1) : 3;
        int x = spread ? (parent->width()-tw-xw*(i-1))/2 : 2;

        i=0;
        for (; !(s=text(r,i)).isNull(); ++i) {
            int w = fm.width(s)+xw;
            if ( highlight(r,i) ) {
                p->fillRect(x-xw/2,1+fm.descent()-fm.lineSpacing(),w,fm.lineSpacing(),Qt::black);
                p->setPen(Qt::white);
            }else{
                p->setPen(Qt::black);
            }
            p->drawText(x,-fm.descent()-1,s);
            x += w;
        }
    }
}

void StringConfig::pickInRow(int r, int xpos, bool press)
{
    QFontMetrics fm = parent->fontMetrics();

    int tw=0;
    QString s;
    int i=0;
    for (; !(s=text(r,i)).isNull(); ++i) {
        int w = fm.width(s);
        tw += w;
    }
    bool spread = spreadRow(r) && parent->width() > tw;
    int xw = spread ? (parent->width()-tw)/(i-1) : 3;
    int x = spread ? (parent->width()-tw-xw*(i-1))/2 : 2;

    i=0;
    for (; !(s=text(r,i)).isNull(); ++i) {
        int x2 = x + fm.width(s)+xw;
        if ( xpos >= x && xpos < x2 ) {
            pick(press, r, i);
            return;
        }
        x = x2;
    }
}

void StringConfig::updateItem(int r, int item)
{
    QFontMetrics fm = parent->fontMetrics();

    int y = r * fm.lineSpacing();

    int tw=0;
    QString s;
    int i=0;
    for (; !(s=text(r,i)).isNull(); ++i) {
        int w = fm.width(s);
        tw += w;
    }
    bool spread = spreadRow(r) && parent->width() > tw;
    int xw = spread ? (parent->width()-tw)/(i-1) : 3;
    int x = spread ? (parent->width()-tw-xw*(i-1))/2 : 2;

    i=0;
    for (; !(s=text(r,i)).isNull(); ++i) {
        int w = fm.width(s)+xw;
        if ( i == item ) {
            parent->update(QRect(x-xw/2,y+1+fm.descent(),w,fm.lineSpacing()));
            return;
        }
        x += w;
    }
}

bool StringConfig::highlight(int,int) const
{
    return false;
}

LetterButton::LetterButton(const QChar& letter, QWidget* parent) :
        QPushButton(QString(letter),parent)
{
    setCheckable(true);
    setAutoDefault(false);
    connect(this,SIGNAL(clicked()),this,SLOT(toggleCase()));
    skip=true;
}

void LetterButton::toggleCase()
{
    if ( skip ) {
        // Don't toggle case the first time
        skip=false;
        return;
    }

    QChar ch = text()[0];
    QChar nch = ch.toLower();
    if ( ch == nch )
        nch = ch.toUpper();
    setText(QString(nch));
}

LetterChoice::LetterChoice(QWidget* parent, const QString& set) :
    QFrame(parent)
{
    QHBoxLayout *l = new QHBoxLayout(this);
    setFrameShape(QFrame::NoFrame);
    QButtonGroup *gp = new QButtonGroup(this);
    gp->setExclusive(true);
    for (int i=0; i<(int)set.length(); i++) {
        LetterButton* b = new LetterButton(set[i],this);
        l->addWidget(b,1,Qt::AlignCenter);
        gp->addButton(b);
        connect(b,SIGNAL(clicked()),this,SLOT(change()));
    }
}

void LetterChoice::change()
{
    LetterButton* b = (LetterButton*)sender();
    ch = b->text()[0];
    emit changed();
}


PickboardAdd::PickboardAdd(QWidget* owner, const QStringList& setlist) :
    QDialog( owner )
{
    setModal(true);
    QVBoxLayout* l = new QVBoxLayout(this);

    QScrollArea *sv = new QScrollArea(this);
    l->addWidget(sv);
    setMaximumHeight(200); // ### QDialog shouldn't allow us to be bigger than the screen
    QWidget *letters = new QWidget(sv);
    QVBoxLayout *letters_layout = new QVBoxLayout(letters);
    letters_layout->setSpacing(0);
    lc = new LetterChoice*[setlist.count()];
    nlc = (int)setlist.count();
    for (int i=0; i<nlc; i++) {
        lc[i] = new LetterChoice(letters,setlist[i]);
        letters_layout->addWidget(lc[i]);
        connect(lc[i],SIGNAL(changed()),this,SLOT(checkAllDone()));
    }
    sv->setWidget(letters);
    QHBoxLayout* hb = new QHBoxLayout(this);
    hb->setSpacing(0);
    l->addLayout(hb);
    yes = new QPushButton(tr("OK"),this);
    hb->addWidget(yes);
    yes->setEnabled(false);
    QPushButton *no = new QPushButton(tr("Cancel"),this);
    hb->addWidget(no);
    connect(yes, SIGNAL(clicked()), this, SLOT(accept()));
    connect(no, SIGNAL(clicked()), this, SLOT(reject()));
}

PickboardAdd::~PickboardAdd()
{
    delete [] lc;
}

QString PickboardAdd::word() const
{
    QString str;
    for (int i=0; i<nlc; i++) {
        str += lc[i]->choice();
    }
    return str;
}

bool PickboardAdd::exec()
{
    QPoint pos = parentWidget()->mapToGlobal(QPoint(0,0));
    pos.ry() -= height();
    if ( QDialog::exec() ) {
        Qtopia::addWords(QStringList(word()));
        return true;
    } else {
        return false;
    }
}

void PickboardAdd::checkAllDone()
{
    if ( !yes->isEnabled() ) {
        for (int i=0; i<nlc; i++) {
            if ( lc[i]->choice().isNull() )
                return;
        }
        yes->setEnabled(true);
    }
}


void DictFilterConfig::doMenu(const QAction * i)
{
    if (i == addA) {
        if ( input.count() == 0 ) {
            QMessageBox::information(0, tr("Adding Words"),
                tr("<qt>To add words, pick the letters, then "
                "open the Add dialog. In that dialog, tap "
                "the correct letters from the list "
                "(tap twice for capitals).</qt>"));
        } else {
            PickboardAdd add(parent,capitalize(input));
            if ( add.exec() )
                generateText(add.word());
            input.clear();
            matches.clear();
            updateRows(0,0);
        }
    } else if (i == resetA) {
        if ( !input.isEmpty() ) {
            input.clear();
            matches.clear();
            StringConfig::doMenu(i);
            updateRows(0,1);
        } else {
            reset();
        }
    } else {
        StringConfig::doMenu(i);
    }
    shift = 0;
    lit0 = -1;
}

QString DictFilterConfig::text(int r, int i)
{
    QStringList l = r ? sets_a : input.isEmpty() ? othermodes : matches;
    return i < (int)l.count() ?
        (input.isEmpty() ? l[i] : capitalize(l[i]))
        : QString();
}

bool DictFilterConfig::spreadRow(int r)
{
    return r ? true : input.isEmpty() ? true : false;
}

QStringList DictFilterConfig::capitalize(const QStringList& l)
{
    switch ( shift ) {
      case 1: {
        QStringList r;
        QStringList::ConstIterator it = l.begin();
        r.append((*it).toUpper());
        for (++it; it != l.end(); ++it)
            r.append(*it);
        return r;
      } case 2: {
        QStringList r;
        for (QStringList::ConstIterator it = l.begin(); it != l.end(); ++it)
            r.append((*it).toUpper());
        return r;
      }
    }
    return l;
}

QString DictFilterConfig::capitalize(const QString& s)
{
    switch ( shift ) {
        case 1: {
            QString u = s;
            u[0] = u[0].toUpper();
            return u;
            break;
        } case 2:
            return s.toUpper();
            break;
    }
    return s;
}

void DictFilterConfig::pick(bool press, int row, int item)
{
    if ( row == 0 ) {
        if ( press ) {
            if ( input.isEmpty() ) {
                lit0 = item;
                if ( othermodes[item] == PickboardPicks::tr("Space") ) {
                    updateItem(row,item);
                    qwsServer->sendKeyEvent( ' ', Qt::Key_Space, 0, true, false );
                    qwsServer->sendKeyEvent( ' ', Qt::Key_Space, 0, false, false );
                } else if ( othermodes[item] == PickboardPicks::tr("Back") ) {
                    updateItem(row,item);
                    qwsServer->sendKeyEvent( 8, Qt::Key_Backspace, 0, true, false );
                    qwsServer->sendKeyEvent( 8, Qt::Key_Backspace, 0, false, false );
                } else if ( othermodes[item] == PickboardPicks::tr("Enter") ) {
                    updateItem(row,item);
                    qwsServer->sendKeyEvent( 8, Qt::Key_Backspace, 0, true, false );
                    qwsServer->sendKeyEvent( 8, Qt::Key_Backspace, 0, false, false );
                } else if ( othermodes[item] == PickboardPicks::tr("Shift") ) {
                    updateItem(row,item);
                    shift = (shift+1)%3;
                }
            }
        } else {
            if ( !input.isEmpty() ) {
                input.clear();
                if ( item>=0 ) {
                    generateText(capitalize(matches[item]));
                }
                shift = 0;
                matches.clear();
                updateRows(0,0);
            } else if ( item < 3 ) {
                lit0 = -1;
                changeMode(item+1); // I'm mode 0! ####
                updateRows(0,1);
            }
            if ( lit0 >= 0 ) {
                if ( !shift || othermodes[lit0] != PickboardPicks::tr("Shift") ) {
                    updateItem(0,lit0);
                    lit0 = -1;
                }
            }
        }
    } else {
        lit0 = -1;
        if ( press && item >= 0 ) {
            lit1 = item;
            add(sets[item]);
            updateItem(1,item);
            updateRows(0,0);
        } else {
            updateItem(1,lit1);
            lit1 = -1;
        }
    }
}

bool DictFilterConfig::scanMatch(const QString& set, const QChar& l) const
{
    return set == "?" || set == "*" || set.contains(l);
}

//static int visit=0;
//static int lvisit=0;

void DictFilterConfig::scan(const QDawg::Node* n, int ipos, const QString& str, int length, bool extend)
{
    if ( n ) {
        do {
//visit++;
            bool pastend = ipos >= (int)input.count();
            if ( pastend && extend || !pastend && scanMatch(input[ipos],n->letter().toLower()) ) {
                if ( length>1 ) {
                    if ( !pastend && input[ipos] == "*" ) {
                        scan(n->jump(),ipos+1,str+n->letter(),length-1,false);
                        scan(n->jump(),ipos,str+n->letter(),length,false);
                    } else {
                        scan(n->jump(),ipos+1,str+n->letter(),length-1,extend);
                    }
                } else {
                    if ( n->isWord() ) {
                        matches.append(str+n->letter());
                    }
                }
            }
            n = n->next();
        } while (n);
    }
}

void DictFilterConfig::scanLengths(const QDawg::Node* n, int ipos, int& length_bitarray)
{
    if ( n ) {
        do {
//lvisit++;
            bool pastend = ipos >= (int)input.count();
            if ( pastend || scanMatch(input[ipos],n->letter().toLower()) ) {
                scanLengths(n->jump(),ipos+1,length_bitarray);
                if ( n->isWord() )
                    length_bitarray |= (1<<(ipos+1));
            }
            n = n->next();
        } while (n);
    }
}

void DictFilterConfig::add(const QString& set)
{
    QFontMetrics fm = parent->fontMetrics();
    input.append(set.toLower());
    matches.clear();
//visit=0;
//lvisit=0;
    int length_bitarray = 0;
    if ( input.count() > 4 ) {
        scanLengths(Qtopia::addedDawg().root(),0,length_bitarray);
        scanLengths(Qtopia::fixedDawg().root(),0,length_bitarray);
    } else {
        length_bitarray = 0xffffffff;
    }
    for (int len=input.count(); len<22 /* 32 */; ++len) {
        if ( length_bitarray & (1<<len) ) {
            scan(Qtopia::addedDawg().root(),0,"",len,true);
            scan(Qtopia::fixedDawg().root(),0,"",len,true);
            int x = 2;
            for (QStringList::Iterator it=matches.begin(); it!=matches.end(); ++it) {
                x += fm.width(*it)+intermatchmargin;
                if ( x >= parent->width() ) {
                    return; // RETURN - No point finding more
                }
            }
        }
        if ( len == 1 && input.count() == 1 ) {
            // Allow all single-characters to show as "matches"
            for ( int i=0; i<(int)set.length(); i++ ) {
                QChar ch = set[i].toLower();
                matches.append(QString(ch));
            }
        }
    }
}

bool DictFilterConfig::highlight(int r,int c) const
{
    return r == 0 ? c == lit0 : c == lit1;
}


void DictFilterConfig::addSet(const QString& appearance, const QString& set)
{
    sets_a.append(appearance);
    sets.append(set);
}

void DictFilterConfig::addMode(const QString& s)
{
    othermodes.append(s);
}

void DictFilterConfig::fillMenu(QMenu& menu)
{
    addA = menu.addAction(tr("Add..."));
    StringConfig::fillMenu(menu);
}

QList<QPixmap> KeycodeConfig::row(int i)
{
    return i ? keypm2 : keypm1;
}

void KeycodeConfig::pickInRow(int r, int xpos, bool press)
{
    QList<QPixmap> pl = row(r);
    QList<QPixmap>::Iterator it;
    int item=0;
    int x=xmarg;
    for (it=pl.begin(); it!=pl.end(); ++it) {
        int x2 = x + (*it).width();
        if ( (*it).height() > 1 )
            x2 += xw;
        if ( xpos >= x && xpos < x2 ) {
            pick(press, r, item);
            return;
        }
        x = x2;
        item++;
    }
}

void KeycodeConfig::pick(bool press, int row, int item)
{
    if ( !press ) {
        if ( item >= 0 ) {
            int k = row == 0 ? keys1[item] : keys2[item];
            int u = row == 0 ? ukeys1[item] : ukeys2[item];
            // more than key, uni as well
            if ( k )
                generateKey(u, k);
        }
        changeMode(0);
        updateRows(0,1);
    }
}

void KeycodeConfig::draw(QPainter* p)
{
    int y=3;
    QList<QPixmap>::Iterator it;
    for (int r=0; r<nrows; r++) {
        QList<QPixmap> pl = row(r);
        int x = xmarg;
        for (it=pl.begin(); it!=pl.end(); ++it) {
            if ( (*it).height() == 1 ) {
                // just a gap
                x += (*it).width();
            } else {
                p->drawPixmap(x,y,*it);
                x += (*it).width()+xw;
            }
        }
        y += parent->height()/nrows;
    }
}


void KeycodeConfig::addKey(int r, const QPixmap& pm, int code, int uni)
{
    if ( r == 0 ) {
        keypm1.append(pm);
        keys1.append(code);
        ukeys1.append(uni);
    } else {
        keypm2.append(pm);
        keys2.append(code);
        ukeys2.append(uni);
    }
}
void KeycodeConfig::addGap(int r, int w)
{
    QBitmap pm(w,1); // ick.
    addKey(r,pm,0,0);
}

QString CharConfig::text(int r, int i)
{
    QStringList l = r ? chars2 : chars1;
    return i < (int)l.count() ? l[i] : QString();
}
bool CharConfig::spreadRow(int)
{
    return true;
}

void CharConfig::pick(bool press, int row, int item)
{
    if ( !press ) {
        if ( item >= 0 ) {
            generateText(row == 0 ? chars1[item] : chars2[item]);
        }
        changeMode(0);
        updateRows(0,1);
    }
}

void CharConfig::addChar(int r, const QString& s)
{
    if ( r ) chars2.append(s); else chars1.append(s);
}

QString CharStringConfig::text(int r, int i)
{
    QStringList l = r ? chars : QStringList(input);
    return i < (int)l.count() ? l[i] : QString();
}

bool CharStringConfig::spreadRow(int i)
{
    return i ? true : false;
}

void CharStringConfig::pick(bool press, int row, int item)
{
    if ( row == 0 ) {
        if ( !press ) {
            if ( item>=0 ) {
                generateText(input);
            }
            input = "";
            changeMode(0);
            updateRows(0,1);
        }
    } else {
        if ( press && item >= 0 ) {
            input.append(chars[item]);
            updateRows(0,0);
        }
    }
}

void CharStringConfig::addChar(const QString& s)
{
    chars.append(s);
}

void CharStringConfig::doMenu(const QAction * i)
{
    if ( i == resetA ) {
        input = "";
        updateRows(0,0);
    }

    StringConfig::doMenu(i);
}

