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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#ifndef QTESTWIDGETS_H
#define QTESTWIDGETS_H

#include <QHash>
#include <QMetaType>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <qtuitestnamespace.h>
#include <qtuitestglobal.h>

// Max timeout before we expect a visible clue on the device that the app is responding.
#define VISIBLE_RESPONSE_TIME 5000

#define USE_ADRESS_FOR_SIGNATURE
class QTestWidget;

class QTUITEST_EXPORT QActiveTestWidgetData : public QObject
{
public:
    QActiveTestWidgetData();
    ~QActiveTestWidgetData();

    void clear();
    void sort();
    void resolveLabels();
    bool scan( QObject* aw );
    QStringList allLabels();

    bool findLabel( const QString &labelText, QTestWidget *&label, QString &error );
    bool findWidget( const QString &labelOrSignature, QTestWidget *&widget, QString &error, int offset );
    bool findTab( const QString &signature, QTestWidget *&tab, QString &error );

    void removeTestWidget( QTestWidget *w );

    QHash< QPointer<QObject>, QTestWidget* > resolved_buddy_pairs;
    QList< QPointer<QTestWidget> > active_scrollbars;
    QList< QPointer<QTestWidget> > active_tabbars;
    QList< QPointer<QTestWidget> > visible_tw_buddies;
    QList< QPointer<QTestWidget> > unresolved_tw_buddies;
    QList< QPointer<QTestWidget> > visible_tw_labels;
    QList< QPointer<QTestWidget> > unresolved_tw_labels;
};

class QTUITEST_EXPORT QActiveTestWidget : public QObject
{
    Q_OBJECT
public:
    static QActiveTestWidget* instance();

    void clear();
    bool rescan( QString &error, int timeout = VISIBLE_RESPONSE_TIME );
    QString toString();
    QStringList allLabels();
    bool findWidget( const QString &labelOrSignature, QTestWidget *&widget, QString &error, int offset = 0 );
    bool getTabBar( QTestWidget *&widget, QString &error );
    bool getSelectedText( QString &selectedText, QString &error );
    QString currentLabelText();
    QString currentFieldText();
    QString currentTab();
    QWidget* focusWidget();

    QString friendlyName(QObject*);

    bool isList();

    static const QString NoActiveWidgetError;

protected:
    bool findLabel( const QString &labelText, QTestWidget *&label, QString &error );
    bool findTab( const QString &signature, QTestWidget *&tab, QString &error );
    bool eventFilter(QObject *obj, QEvent *event);
    bool findWidget_impl( const QString &labelOrSignature, QTestWidget *&widget, QString &error, int offset );

private:
    QActiveTestWidget();
    ~QActiveTestWidget();

    QPointer<QWidget> active_widget;
    QString active_tab;
    QString active_title;
    bool scan_busy;
    int scan_time;

    QActiveTestWidgetData *d;
};

//
// Abstract base class for test widgets.
//
class QTUITEST_EXPORT QTestWidget : public QObject
{
    Q_OBJECT
public:
    QTestWidget( const QString &signature, QObject *w );
    virtual ~QTestWidget();

    QString signature() const;
    QObject* instance() const;
    QString uniqueName() const;

    int x() const;
    int y() const;
    int width() const;
    int height() const;
    QWidget* parentWidget( const QWidget *child ) const;
    QRegion totalVisibleRegion() const;

    virtual QString     isChecked(bool *ok) {
        QtUiTest::CheckWidget *cw = qtuitest_cast<QtUiTest::CheckWidget*>(instance());
        if (cw) {
            if (ok) *ok = true;
            return (cw->checkState() != Qt::Unchecked) ? "YES" : "NO";
        }
        if (ok) *ok = false; return "ERROR: Unsupported class '" + _signature + "'";
    }
    virtual QString     getSelectedText(bool *ok) {
        QtUiTest::TextWidget *tw = qtuitest_cast<QtUiTest::TextWidget*>(instance());
        if (tw) {
            if (ok) *ok = true;
            return tw->selectedText();
        }
        if (ok) *ok = false; return "ERROR: Unsupported class '" + _signature + "'";
    }
    virtual QString     getText(bool *ok) {
        QtUiTest::TextWidget *tw = qtuitest_cast<QtUiTest::TextWidget*>(instance());
        if (tw) {
            if (ok) *ok = true;
            return tw->text();
        }
        if (ok) *ok = false; return "ERROR: Unsupported class '" + _signature + "'";
    }
    virtual QStringList getList(bool *ok) {
        QtUiTest::ListWidget *lw = qtuitest_cast<QtUiTest::ListWidget*>(instance());
        if (lw) {
            if (ok) *ok = true;
            return lw->list();
        }
        if (ok) *ok = false; return QStringList("ERROR: Unsupported class '" + _signature + "'");
    }
    virtual QPoint getCenter(QString const &item, bool *ok) {
        QtUiTest::Widget *w = qtuitest_cast<QtUiTest::Widget*>(instance());
        QtUiTest::ListWidget *lw = qtuitest_cast<QtUiTest::ListWidget*>(instance());
        if (lw && w) {
            if (ok) *ok = true;
            return w->mapToGlobal(lw->visualRect(item).center());
        }
        Q_UNUSED(item); if (ok) *ok = false; return QPoint();
    }
    virtual QList<QPoint> navigateByMouse(QString const &item, QPoint const &pos, int *wait, bool *ok);
    virtual bool isEditableType() = 0;
    virtual bool isCheckboxType() = 0;

    typedef QTestWidget* (*Factory)(const QString &signature, QObject* w);

    static QString const GetListRegExp;

protected slots:
    void onDestroyed();

protected:
    QPointer<QObject> _instance;
    friend class QTestWidgets;
    QObject *_unsafe_instance;

private:
    QString _signature;
};

typedef QTestWidget QTestQWidget;
Q_DECLARE_METATYPE(QTestWidget*)

//
// This is the class that manages the test widgets and handles queries about them.
//
class QTUITEST_EXPORT QTestWidgets
{
public:
    static QWidget* activeWidget();
    static QObjectList allObjects();

    static QString getSelectedText( const QString &signature, bool *ok );
    static QString getText( const QString &signature, bool *ok );
    static QStringList getList( const QString &signature, bool *ok );
    static QString isChecked( const QString &signature, bool *ok );

    static QTestWidget *testWidget( QObject *widget );
    static QTestWidget *testWidget( const QString signature );
    static QObject* _findWidget( const QString &signature );
    static QObject* findObject( const QString &signature );
    static QWidget* locateBuddy( const QTestWidget *testlabel, const QList< QPointer<QTestWidget> > &search_list );

    static QStringList locateObjectByType( const QStringList &typeList );
    static QStringList locateObjectByType( const QStringList &typeList, const QObjectList &searchList );
    static QStringList locateWidgetByText( const QString &widgetText, const QStringList &typeList = QStringList() );
    static QStringList locateWidgetByText( const QString &widgetText, const QStringList &typeList, const QObjectList &searchList, bool checkChildren = true );
    static QString signature( const QObject* object );

    static bool widgetVisible( QWidget *widget );

protected:
    static bool hasVisibleChildren( QObject *w );

private:
    static QString uniqueName( const QObject *item );
    static QObject* findObjectBySignature( const QString &signature, const QObjectList &search_list );
    friend class QActiveTestWidget;
    static QStringList locateWidget( const QString &widgetName, const QStringList &typeList, const QObjectList &searchList );
    static bool objectExists( const QObject *object, const QObjectList &search_list );

    struct FactoryRec
    {
        QString className;
        const QMetaObject *metaObject;
        QTestWidget::Factory factory;
    };

    inline static QList< FactoryRec >& factories()
    { static QList<FactoryRec> l; return l; }

    inline static QString &last_error()
    { static QString s; return s; }

    static bool derivedFrom(FactoryRec class1, FactoryRec class2);

    // functionality to cache widgets and associated testwidgets
    friend class QTestWidget;
    static QTestWidget* createInstance( QObject *w, const QString &signature );
    static void removeWidget( QTestWidget *rec );
    static void clear();
    static uint count();

    friend class QActiveTestWidgetData;
    static QHash<QString, QTestWidget *> testWidgetsBySignature;
    static QHash<QObject*, QTestWidget *> testWidgetsByPtr;
};

#endif
