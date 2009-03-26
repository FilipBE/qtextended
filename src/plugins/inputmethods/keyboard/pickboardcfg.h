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
#ifndef PICKBOARDCFG_H
#define PICKBOARDCFG_H

#include <qdawg.h>

#include <QPushButton>
#include <QButtonGroup>
#include <QDialog>
#include <QList>
#include <QFrame>


// Internal stuff...

class PickboardPicks;

class LetterButton : public QPushButton {
    Q_OBJECT
public:
    LetterButton(const QChar& letter, QWidget* parent);
private slots:
    void toggleCase();
private:
    bool skip;
};

class LetterChoice : public QFrame {
    Q_OBJECT
public:
    LetterChoice(QWidget* parent, const QString& set);

    QChar choice() { return ch; }

signals:
    void changed();

private slots:
    void change();

private:
    QChar ch;
};

class PickboardAdd : public QDialog {
    Q_OBJECT
public:
    PickboardAdd(QWidget* owner, const QStringList& setlist);
    ~PickboardAdd();

    QString word() const;
    bool exec();

private slots:
    void checkAllDone();

private:
    QPushButton *yes;
    LetterChoice **lc;
    int nlc;
};

class PickboardConfig : public QObject {
    Q_OBJECT
public:
    PickboardConfig(PickboardPicks* p) : parent(p), nrows(2), resetA(0), helpA(0), pressx(-1) { }
    virtual ~PickboardConfig();
    virtual void pickPoint(const QPoint& p, bool press);
    virtual void draw(QPainter*)=0;
    virtual void fillMenu(QMenu&);
    virtual void doMenu(const QAction *);

    void reset();

protected:
    void updateRows(int from, int to);
    virtual void updateItem(int r, int i);
    virtual void pickInRow(int r, int xpos, bool press)=0;

    void changeMode(int m);
    virtual void generateText(const QString& s);
    void generateKey( int u, int k );

    virtual void pick(bool press, int row, int item)=0;

protected:
    PickboardPicks* parent;
    int nrows;
    QAction *resetA, *helpA;
private:
    int pressrow, pressx;
};

class StringConfig : public PickboardConfig {
    Q_OBJECT
public:
    StringConfig(PickboardPicks* p) : PickboardConfig(p) { }

    void draw(QPainter* p);

protected:
    virtual QString text(int r, int i)=0;
    virtual bool spreadRow(int i)=0;
    virtual QColor rowColor(int) { return Qt::black; }
    virtual void pickInRow(int r, int xpos, bool press);
    virtual void updateItem(int r, int i);
    virtual bool highlight(int,int) const;
};

class CharStringConfig : public StringConfig {
    Q_OBJECT
    QString input;
    QStringList chars;
public:
    CharStringConfig(PickboardPicks* p) : StringConfig(p) { }

    void addChar(const QString& s);
    virtual void doMenu(const QAction *);

protected:
    QString text(int r, int i);
    bool spreadRow(int i);
    void pick(bool press, int row, int item);
};

class DictFilterConfig : public StringConfig {
    Q_OBJECT
    QStringList matches;
    QStringList sets_a;
    QStringList sets;
    QStringList othermodes;
    int lit0;
    int lit1;
    int shift;
    QString capitalize(const QString& s);
    QStringList capitalize(const QStringList& s);

public:
    QStringList input;
    DictFilterConfig(PickboardPicks* p) : StringConfig(p), addA(0)
    {
        shift = 0;
        lit0 = -1;
        lit1 = -1;
    }

    void addSet(const QString& appearance, const QString& set);
    void addMode(const QString& s);

    void fillMenu(QMenu& menu);
    void doMenu(const QAction * i);

    void add(const QString& set);

protected:
    QString text(int r, int i);

    bool spreadRow(int i);

    void pick(bool press, int row, int item);

    bool scanMatch(const QString& set, const QChar& l) const;
    void scan(const QDawg::Node* n, int ipos, const QString& str, int length, bool extend);
    void scanLengths(const QDawg::Node* n, int ipos, int& bitarray);

    bool highlight(int r,int c) const;

    QAction *addA;
};

class CharConfig : public StringConfig {
    Q_OBJECT
    QStringList chars1;
    QStringList chars2;
public:
    CharConfig(PickboardPicks* p) : StringConfig(p) { }
    void addChar(int r, const QString& s);

protected:
    QString text(int r, int i);
    bool spreadRow(int);
    void pick(bool press, int row, int item);
};

class KeycodeConfig : public PickboardConfig {
    Q_OBJECT
    QList<int> keys1;
    QList<int> keys2;
    QList<int> ukeys1;
    QList<int> ukeys2;
    QList<QPixmap> keypm1;
    QList<QPixmap> keypm2;
    enum { xw = 8, xmarg = 8};

public:
    KeycodeConfig(PickboardPicks* p) : PickboardConfig(p) { }
    void addKey(int r, const QPixmap& pm, int code, int uni = 0xffff);
    void addGap(int r, int w);

    void draw(QPainter* p);

protected:
    void pickInRow(int r, int xpos, bool press);
    QList<QPixmap> row(int i);

    void pick(bool press, int row, int item);
};


#endif
