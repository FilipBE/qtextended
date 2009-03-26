/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QPUSHBUTTON_H
#define QPUSHBUTTON_H

#include <QtGui/qabstractbutton.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QPushButtonPrivate;
class QMenu;
class QStyleOptionButton;

class Q_GUI_EXPORT QPushButton : public QAbstractButton
{
    Q_OBJECT

    Q_PROPERTY(bool autoDefault READ autoDefault WRITE setAutoDefault)
    Q_PROPERTY(bool default READ isDefault WRITE setDefault)
    Q_PROPERTY(bool flat READ isFlat WRITE setFlat)

public:
    explicit QPushButton(QWidget *parent=0);
    explicit QPushButton(const QString &text, QWidget *parent=0);
    QPushButton(const QIcon& icon, const QString &text, QWidget *parent=0);
    ~QPushButton();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    bool autoDefault() const;
    void setAutoDefault(bool);
    bool isDefault() const;
    void setDefault(bool);

#ifndef QT_NO_MENU
    void setMenu(QMenu* menu);
    QMenu* menu() const;
#endif

    void setFlat(bool);
    bool isFlat() const;

public Q_SLOTS:
#ifndef QT_NO_MENU
    void showMenu();
#endif

protected:
    bool event(QEvent *e);
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void initStyleOption(QStyleOptionButton *option) const;
    QPushButton(QPushButtonPrivate &dd, QWidget* parent = 0);

public:
#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QPushButton(QWidget *parent, const char* name);
    QT3_SUPPORT_CONSTRUCTOR QPushButton(const QString &text, QWidget *parent, const char* name);
    QT3_SUPPORT_CONSTRUCTOR QPushButton(const QIcon& icon, const QString &text, QWidget *parent, const char* name);
    inline QT3_SUPPORT void openPopup()  { showMenu(); }
    inline QT3_SUPPORT bool isMenuButton() const { return menu() !=  0; }
    inline QT3_SUPPORT void setPopup(QMenu* popup) {setMenu(popup); }
    inline QT3_SUPPORT QMenu* popup() const { return menu(); }
#endif

private:
    Q_DISABLE_COPY(QPushButton)
    Q_DECLARE_PRIVATE(QPushButton)
#ifndef QT_NO_MENU        
    Q_PRIVATE_SLOT(d_func(), void _q_popupPressed())
#endif
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QPUSHBUTTON_H
