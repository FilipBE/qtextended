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

#include <QTest>

#define private public
#include <qtuitestwidgets_p.h>
#undef private

#include <qtuitestnamespace.h>

#include <QLabel>
#include <QCheckBox>
#include <QPluginManager>

//TESTED_COMPONENT=QA: Testing Framework (18707)

using namespace QtUiTest;

class tst_QtUiTestWidgets : public QObject
{
    Q_OBJECT

private slots:
    void cast_zero();
    void cast_inherit_single();
    void cast_self();
    void cast();
    void cast_unregistered();

    void cleanup();
};

QTEST_MAIN(tst_QtUiTestWidgets)

class WidgetObject : public QObject, public Widget
{
    Q_OBJECT
    Q_INTERFACES(QtUiTest::Widget)

public:
    WidgetObject(QObject *_wrappee)
        : QObject(), wrappee(_wrappee), geometryCalls(0)
    { ++constructed; }

    virtual ~WidgetObject()
    { ++destructed; }

    virtual const QRect& geometry() const
    { ++geometryCalls; static QRect nullRect; return nullRect; }

    virtual QRect rect() const
    { return QRect(); }

    virtual bool isVisible() const
    { return false; }

    virtual QRegion visibleRegion() const
    { return QRegion(); }

    virtual QRegion childrenVisibleRegion() const
    { return QRegion(); }

    virtual const QObjectList &children() const
    { static QObjectList c; return c; }

    virtual QObject *parent() const
    { return 0; }

    virtual QString windowTitle() const
    { return QString(); }

    virtual QPoint mapToGlobal(QPoint const&) const
    { return QPoint(); }

    virtual QPoint mapFromGlobal(QPoint const&) const
    { return QPoint(); }

    virtual bool setFocus()
    { return false; }

    virtual bool setEditFocus(bool)
    { return false; }

    virtual void focusOutEvent()
    {}

    virtual bool hasFocus() const
    { return false; }

    virtual bool hasEditFocus() const
    { return false; }

    virtual bool ensureVisibleRegion(QRegion const&)
    { return false; }

    QObject *wrappee;
    mutable int geometryCalls;

    static int constructed;
    static int destructed;
};
int WidgetObject::constructed = 0;
int WidgetObject::destructed  = 0;

class TextWidgetObject : public WidgetObject, public TextWidget
{
    Q_OBJECT
    Q_INTERFACES(QtUiTest::TextWidget)

public:
    TextWidgetObject(QObject *_wrappee)
        : WidgetObject(_wrappee)
    { ++constructed; }

    virtual ~TextWidgetObject()
    { ++destructed; }

    virtual QString text() const
    {
        QLabel const *label = qobject_cast<QLabel*>(wrappee);
        return label ? label->text() : QString();
    }

    virtual QString selectedText() const
    { return text(); }

    static int constructed;
    static int destructed;

    static QObject* constructor(QObject* wrappee)
    { return new TextWidgetObject(wrappee); }
};
int TextWidgetObject::constructed = 0;
int TextWidgetObject::destructed  = 0;


class TestableCheckBox : public QCheckBox,
    public Widget, public CheckWidget, public TextWidget
{
    Q_OBJECT
    Q_INTERFACES(
            QtUiTest::Widget
            QtUiTest::CheckWidget
            QtUiTest::TextWidget)

public:
    TestableCheckBox(QWidget *parent = 0)
        : QCheckBox(parent)
    { ++constructed; }

    virtual ~TestableCheckBox()
    { ++destructed; }

    virtual const QRect& geometry() const
    { return QCheckBox::geometry(); }

    virtual QRect rect() const
    { return QCheckBox::rect(); }

    virtual bool isVisible() const
    { return QCheckBox::isVisible(); }

    virtual QRegion visibleRegion() const
    { return QCheckBox::visibleRegion(); }

    virtual QRegion childrenVisibleRegion() const
    { return QRegion(); }

    virtual const QObjectList &children() const
    { return QCheckBox::children(); }

    virtual QObject *parent() const
    { return QCheckBox::parent(); }

    virtual QString windowTitle() const
    { return QCheckBox::windowTitle(); }

    virtual QString text() const
    { return QCheckBox::text(); }

    virtual QString selectedText() const
    { return QCheckBox::text(); }

    virtual bool isTristate() const
    { return QCheckBox::isTristate(); }

    virtual Qt::CheckState checkState() const
    { return QCheckBox::checkState(); }

    virtual QPoint mapToGlobal(QPoint const &p) const
    { return QCheckBox::mapToGlobal(p); }

    virtual QPoint mapFromGlobal(QPoint const &p) const
    { return QCheckBox::mapFromGlobal(p); }

    virtual bool setFocus()
    { QCheckBox::setFocus(); return true; }

    virtual bool setEditFocus(bool)
    { return false; }

    virtual void focusOutEvent()
    {}

    virtual bool ensureVisibleRegion(QRegion const&)
    { return false; }

    virtual bool hasFocus() const
    { return QCheckBox::hasFocus(); }

    virtual bool hasEditFocus() const
    { return hasFocus(); }

    static QObject* constructor(QObject* wrappee)
    { return qobject_cast<TestableCheckBox*>(wrappee); }

    static int constructed;
    static int destructed;
};
int TestableCheckBox::constructed = 0;
int TestableCheckBox::destructed  = 0;


class TestWidgetFactory : public QObject, public WidgetFactory
{
    Q_OBJECT
    Q_INTERFACES(QtUiTest::WidgetFactory)

    public:
        QObject* create(QObject *wrappee)
        {
            QObject *ret = 0;
            if (!ret && wrappee->inherits("QLabel"))
                ret = new TextWidgetObject(wrappee);
            if (!ret && wrappee->inherits("QWidget"))
                ret = new WidgetObject(wrappee);
            return ret;
        }

        QStringList keys() const
        { return QStringList() << "QLabel" << "QWidget"; }
};
Q_EXPORT_PLUGIN2(testwidgetfactory, TestWidgetFactory)

Q_IMPORT_PLUGIN(testwidgetfactory)


/*
    \req QTOPIA-78

    \groups
*/
void tst_QtUiTestWidgets::cast_unregistered()
{
    // Test that casting without calling registerClass fails.
    QObject obj;
    QVERIFY(!qtuitest_cast<Widget*>(&obj));
    // Cast again to ensure any caching code path gets executed.
    QVERIFY(!qtuitest_cast<Widget*>(&obj));
}

/*
    \req QTOPIA-78

    \groups
*/
void tst_QtUiTestWidgets::cast()
{
    QtUiTestWidgets::instance()->clear();

    QCOMPARE(WidgetObject::constructed, 0);
    QCOMPARE(WidgetObject::destructed, 0);

    Widget *wrapper = 0;
    WidgetObject *wo = 0;

    {
        QWidget widget;

        QtUiTestWidgets::instance()->refreshPlugins();

        QCOMPARE(WidgetObject::constructed, 0);
        QCOMPARE(WidgetObject::destructed, 0);

        wrapper = qtuitest_cast<Widget*>(&widget);
        wo = static_cast<WidgetObject*>(wrapper);
        QVERIFY(wrapper);
        QVERIFY(wo);
        QCOMPARE(wo->wrappee, &widget);
        QCOMPARE(wo->geometryCalls, 0);

        QCOMPARE(WidgetObject::constructed, 1);
        QCOMPARE(WidgetObject::destructed, 0);

        // Cast something that's already a wrapper
        QCOMPARE(qtuitest_cast<Widget*>(wo), wrapper);

        QCOMPARE(WidgetObject::constructed, 1);
        QCOMPARE(WidgetObject::destructed, 0);

        // Verify that WidgetObject's functions are really being called
        wrapper->geometry();
        QCOMPARE(wo->geometryCalls, 1);

        // Cast again and verify that it returns the object we already have
        QCOMPARE( qtuitest_cast<Widget*>(&widget), wrapper );
        QCOMPARE(WidgetObject::constructed, 1);
        QCOMPARE(WidgetObject::destructed, 0);

        // Delete wrapped object and verify that testwidget gets deleted
    }
    QCOMPARE(WidgetObject::constructed, 1);
    QCOMPARE(WidgetObject::destructed, 1);

    // Construct and test again
    {
        QWidget widget;
        wrapper = qtuitest_cast<Widget*>(&widget);
        wo = static_cast<WidgetObject*>(wrapper);
        QVERIFY(wrapper);
        QVERIFY(wo);
        QCOMPARE(wo->wrappee, &widget);
        QCOMPARE(wo->geometryCalls, 0);

        QCOMPARE(WidgetObject::constructed, 2);
        QCOMPARE(WidgetObject::destructed, 1);

        // Delete wrapper without deleting object
        delete wrapper;
        QCOMPARE(WidgetObject::constructed, 2);
        QCOMPARE(WidgetObject::destructed, 2);

        // Verify new wrapper can be obtained after deleting old wrapper
        wrapper = qtuitest_cast<Widget*>(&widget);
        wo = static_cast<WidgetObject*>(wrapper);
        QVERIFY(wrapper);
        QVERIFY(wo);
        QCOMPARE(wo->wrappee, &widget);
        QCOMPARE(wo->geometryCalls, 0);

        QCOMPARE(WidgetObject::constructed, 3);
        QCOMPARE(WidgetObject::destructed, 2);

        QCOMPARE( qtuitest_cast<Widget*>(&widget), wrapper );
        QCOMPARE(WidgetObject::constructed, 3);
        QCOMPARE(WidgetObject::destructed, 2);
    }
    QCOMPARE(WidgetObject::constructed, 3);
    QCOMPARE(WidgetObject::destructed, 3);
}

/*
    \req QTOPIA-78

    \groups
    Test case where an interface is implemented by a class which inherits a
    class which implements one other interface.
*/
void tst_QtUiTestWidgets::cast_inherit_single()
{
    QCOMPARE(WidgetObject::constructed, 0);
    QCOMPARE(WidgetObject::destructed, 0);
    QCOMPARE(TextWidgetObject::constructed, 0);
    QCOMPARE(TextWidgetObject::destructed, 0);

    QtUiTestWidgets::instance()->refreshPlugins();

    Widget *w;
    TextWidget *tw;

    {
        QWidget widget;
        QLabel label;

        w  = qtuitest_cast<Widget*>(&widget);
        tw = qtuitest_cast<TextWidget*>(&label);

        QVERIFY(w);
        QVERIFY(tw);
        QCOMPARE(TextWidgetObject::constructed, 1);
        QCOMPARE(TextWidgetObject::destructed, 0);

        /* One or more widget objects may have been constructed while finding
         * the interfaces for QLabel. However there should be exactly 2
         * still existing. */
        QCOMPARE(WidgetObject::constructed - WidgetObject::destructed, 2);

        QCOMPARE(tw->text(), QString());
        label.setText("Hi there");
        QCOMPARE(tw->text(), QString("Hi there"));
    }

    QCOMPARE(TextWidgetObject::constructed, 1);
    QCOMPARE(TextWidgetObject::destructed, 1);
    QCOMPARE(WidgetObject::constructed - WidgetObject::destructed, 0);
}

/*
    \req QTOPIA-78

    \groups
    Test casting of a widget class which implements its own interfaces.
*/
void tst_QtUiTestWidgets::cast_self()
{
    QtUiTestWidgets::instance()->clear();

    QCOMPARE( TestableCheckBox::constructed, 0 );
    QCOMPARE( TestableCheckBox::destructed, 0 );

    Widget *w;
    TextWidget *tw;
    CheckWidget *cw;

    {
        TestableCheckBox tcb;
        QCheckBox *cb = &tcb;

        QCOMPARE( TestableCheckBox::constructed, 1 );
        QCOMPARE( TestableCheckBox::destructed, 0 );

        /* Doesn't need to be registered before cast. */
        QVERIFY( w  = qtuitest_cast<Widget*>(&tcb) );
        QVERIFY( tw = qtuitest_cast<TextWidget*>(&tcb) );
        QVERIFY( cw = qtuitest_cast<CheckWidget*>(&tcb) );

        QCOMPARE( w, &tcb );
        QCOMPARE( tw, &tcb );
        QCOMPARE( cw, &tcb );

        /* However, should still work when other classes are registered...*/
        QtUiTestWidgets::instance()->refreshPlugins();
        QCOMPARE( qtuitest_cast<Widget*>(&tcb), w );
        QCOMPARE( qtuitest_cast<TextWidget*>(&tcb), tw );
        QCOMPARE( qtuitest_cast<CheckWidget*>(&tcb), cw );
        QCOMPARE( WidgetObject::constructed, 0 );
        QCOMPARE( WidgetObject::destructed, 0 );

        QCOMPARE( TestableCheckBox::constructed, 1 );
        QCOMPARE( TestableCheckBox::destructed, 0 );

        /* Use it a bit and make sure it works as expected. */
        QCOMPARE( w->geometry(), tcb.geometry() );

        tcb.setText("Walk the Dog");
        QCOMPARE( tw->text(), tcb.text() );

        tcb.setTristate(true);
        cb->setCheckState(Qt::PartiallyChecked);
        QCOMPARE( cw->isTristate(), tcb.isTristate() );
        QCOMPARE( cw->checkState(), tcb.checkState() );
    }

    /* Ensure we didn't double-delete */
    QCOMPARE( TestableCheckBox::constructed, 1 );
    QCOMPARE( TestableCheckBox::destructed, 1 );

    /* Ensure we can make another one with no problems */
    {
        TestableCheckBox tcb;

        QCOMPARE( TestableCheckBox::constructed, 2 );
        QCOMPARE( TestableCheckBox::destructed, 1 );

        QVERIFY( w  = qtuitest_cast<Widget*>(&tcb) );
        QVERIFY( tw = qtuitest_cast<TextWidget*>(&tcb) );
        QVERIFY( cw = qtuitest_cast<CheckWidget*>(&tcb) );

        QCOMPARE( w, &tcb );
        QCOMPARE( tw, &tcb );
        QCOMPARE( cw, &tcb );
    }
    QCOMPARE( TestableCheckBox::constructed, 2 );
    QCOMPARE( TestableCheckBox::destructed, 2 );

    /* Ensure casting a null pointer has no ill effects */
    w = qtuitest_cast<Widget*>( static_cast<TestableCheckBox*>(0) );
    QVERIFY( !w );

    QCOMPARE( TestableCheckBox::constructed, 2 );
    QCOMPARE( TestableCheckBox::destructed, 2 );

    QCOMPARE( WidgetObject::constructed, 0 );
    QCOMPARE( WidgetObject::destructed, 0 );
}

/*
    \req QTOPIA-78

    \groups
*/
void tst_QtUiTestWidgets::cast_zero()
{
    QtUiTestWidgets::instance()->clear();

    QVERIFY( !qtuitest_cast<Widget*>(static_cast<QObject*>(0)) );

    QtUiTestWidgets::instance()->refreshPlugins();

    QVERIFY( !qtuitest_cast<Widget*>(static_cast<QObject*>(0)) );
}

void tst_QtUiTestWidgets::cleanup()
{
    QtUiTestWidgets::instance()->clear();
    QtUiTestWidgets::instance()->refreshPlugins();
    WidgetObject::constructed = 0;
    WidgetObject::destructed = 0;
    TextWidgetObject::constructed = 0;
    TextWidgetObject::destructed = 0;
    TestableCheckBox::constructed = 0;
    TestableCheckBox::destructed = 0;
}

#include "tst_qtuitestwidgets.moc"
