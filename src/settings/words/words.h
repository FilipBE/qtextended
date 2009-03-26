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
#ifndef WORDS_H
#define WORDS_H

#include <QDialog>
#include <QItemDelegate>

class QAction;
class QLineEdit;
class QListWidget;
class InputMatcher;
class QLabel;

class WordListDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    WordListDelegate(QObject *parent);
    ~WordListDelegate() {};

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
};


class Words : public QDialog
{
    Q_OBJECT

public:
    Words( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~Words();

    QSize sizeHint() const;

protected:
    void keyPressEvent(QKeyEvent*);
    void resizeEvent(QResizeEvent *resizeEvent);
    bool eventFilter(QObject* watched, QEvent* e);

private slots:
    void updateActions();
    void lookup();
    void lookup(const QString& in);
    void modeChanged();
    void addWord();
    void addWord(const QString& word);
    void deleteWord();
    void preferWord();
    void showAddedDict();
    void showPreferredDict();
    void showDeletedDict();

private:
    void showDict(const char* name);
    void search();
    void search(const QString&);
    QLineEdit *line;
    QListWidget *box;
    QAction *a_add, *a_del, *a_prefer, *a_pkim, *a_word;
    QAction *a_alllocal, *a_allpref, *a_alldel;
    InputMatcher *matcher;
    QLabel *tooltip;
    bool dummy;
};

#endif
