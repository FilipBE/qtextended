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

#ifndef FIRSTUSE_H
#define FIRSTUSE_H

#include <QDialog>


class QPushButton;
class QLabel;
class QWaitWidget;

class FirstUse : public QDialog
{
    Q_OBJECT

public:
    FirstUse(QWidget* parent=0, Qt::WFlags=0);
    ~FirstUse();

    void reloadLanguages();
    int currentDialogIndex() const;

    bool filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat);

public slots:
    void nextDialog();
    void previousDialog();
    void acceptCurrentDialog();
    void done(int r);

private slots:
    void calcMaxWindowRect();
    void switchDialog();
    void dialogAccepted();

protected:
    void paintEvent( QPaintEvent * );
    void mouseReleaseEvent( QMouseEvent *event );
    void keyPressEvent( QKeyEvent *e );
    bool eventFilter(QObject *o, QEvent* e);

private:
    int findNextDialog(bool forwards);
    void updateButtons();
    void showFirstDialog();

private:
    int currDlgIdx;
    QDialog *currDlg;
    QPushButton *back;
    QPushButton *next;
    int controlHeight;
    QWidget *taskBar;
    QLabel *titleBar;
    bool avoidEarlyExit;
    QWaitWidget *waitDialog;
};

#endif
