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

#ifndef GRIDBROWSER_H
#define GRIDBROWSER_H

#include "phonelauncherview.h"
#include "lazycontentstack.h"

#include <QStackedWidget>
#include <QCloseEvent>
#include <QMap>
#include <QSet>
#include <QStack>
#include <QString>
#include <QValueSpaceItem>

#include <qcontent.h>
#include "launcherview.h"
#include <QPointer>
#include "qabstractmessagebox.h"
#include <qcategorymanager.h>
#include "qabstractbrowserscreen.h"
#include "applicationmonitor.h"

class QAction;
class QMenu;
struct TypeView {
    TypeView() : view(0) {};
    LauncherView *view;
    QString name;
    QIcon icon;
};

class QSettings;
class PhoneMainMenu;
class PhoneBrowserStack : public LazyContentStack
{
Q_OBJECT
public:
    PhoneBrowserStack(QWidget *parent = 0);
    virtual void show();
    virtual void hide();
    void insertPhoneMenu(QWidget *m);

    void showType(const QString &);

    QString currentName() const;

    void back();

    QPointer<QAbstractMessageBox> warningBox;
    PhoneMainMenu *phoneLauncher;
    PhoneMainMenu *rotatedPhoneLauncher(bool create = true);
    
    QMap<QString, TypeView> map;
    QStackedWidget *stack;
    
protected:
    void keyPressEvent(QKeyEvent *ke);

    virtual void busy(const QContent &);
    virtual void notBusy();
    virtual void noView(const QString &);

    virtual QObject* createView(const QString &);
    virtual void raiseView(const QString &, bool);
    void closeEvent(QCloseEvent *e);

signals:
    void visibilityChanged();
    void applicationLaunched(const QString &);

private:
    LauncherView *addType(const QString& type,
                          const QString& name, const QIcon &icon);
    LauncherView *createAppView(const QString &);
    LauncherView *createContentSetView();

    QObject *currentViewObject();
    int menuIdx;
    void showMessageBox(const QString& title, const QString& text, QAbstractMessageBox::Icon icon=QAbstractMessageBox::Information);

    QCategoryManager appCategories;
    
    QValueSpaceItem *rotation;
    int currentRotation;
    int defaultRotation;
    bool gridNeedsRotation;
    PhoneMainMenu *delayedRotatedPhoneLauncher;

private slots:
    void rotationChanged();
    
};

class PhoneMainMenu : public PhoneLauncherView
{
Q_OBJECT
public:
    PhoneMainMenu(QSettings &, QWidget * parent = 0);

    void showDefaultSelection();

private slots:
    void expressionChanged();

private:
    void activateItem(const QChar &, int);

    void setMainMenuItemEnabled(const QString &file, const QString &name, const QString &icon, bool enabled);
    void makeLauncherMenu(QSettings &);
    QContent *readLauncherMenuItem(const QString &);

    class ItemExpression;
    struct Item {
        QContent lnk;
        bool exprTrue();
        ItemExpression *expr;
        bool enabled;
    };
    struct Items : public QList<Item>
    {
        Items() : activeItem(-1) {}
        int activeItem;
    };
    QMap<QChar,Items> mainMenuEntries;
    QString menuKeyMap;
    
    int defaultSelection;
};


class GridBrowserScreen : public QAbstractBrowserScreen
{
Q_OBJECT
public:
    GridBrowserScreen(QWidget *parent = 0, Qt::WFlags flags = 0);

    virtual QString currentView() const;
    virtual bool viewAvailable(const QString &) const;
    virtual void resetToView(const QString &);
    virtual void moveToView(const QString &);

protected:
    void closeEvent(QCloseEvent *e);

private:
    PhoneBrowserStack *m_stack;
    QHash<QString, QContent*> m_dynamicallyAddedItems;
};

#endif
