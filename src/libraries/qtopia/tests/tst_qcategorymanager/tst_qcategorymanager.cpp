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


#include <QtopiaApplication>
#include <QCategoryManager>
#include <QSignalSpy>
#include <shared/util.h>
#include <QTest>
#include <shared/qtopiaunittest.h>


//TESTED_CLASS=QCategoryManager
//TESTED_FILES=src/libraries/qtopia/qcategorymanager.cpp

/*
    This class is a unit test for QCategoryManager.
*/
class tst_QCategoryManager : public QObject
{
    Q_OBJECT

public:
    tst_QCategoryManager();
    enum ScopeTestAction { Add, Remove, RemoveAll, Check };

private slots:
    void scope();
    void scope_data();

    void categoriesChanged();

    void staticLabels();

    void bug180072();

    virtual void cleanupTestCase();
    virtual void init();

private:
    void removeCategories();

    QString const _cat;
    QString const _scope;
    int const CATMAX;
    int const SCOPEMAX;
    bool doClean;
    QList<QString> globalSystemCats;
};

QTEST_APP_MAIN( tst_QCategoryManager, QtopiaApplication )
#include "tst_qcategorymanager.moc"

Q_DECLARE_METATYPE( QSet<QString> );
Q_DECLARE_METATYPE( tst_QCategoryManager::ScopeTestAction );


/* Verbose qCompare for QSet<QString> and QIcon */
namespace QTest {
template<>
inline bool qCompare(QSet<QString> const &t1, QSet<QString> const &t2,
                     const char *actual, const char *expected, const char *file, int line)
{
    /* For each set, construct a string of the form:
         { 'elem1', 'elem2', 'elem3' }
       Then compare the two.  This gives a nice user-readable comparison.
     */
    QString l1 = "{ ";
    QString l2 = "{ ";
    QList<QString> list1 = QList<QString>::fromSet(t1);
    QList<QString> list2 = QList<QString>::fromSet(t2);
    qSort(list1);
    qSort(list2);

    foreach( QString str, list1 ) {
        l1.append("'").append(str).append("', ");
    }
    foreach( QString str, list2 ) {
        l2.append("'").append(str).append("', ");
    }
    l1.remove(l1.lastIndexOf(','),1);
    l2.remove(l2.lastIndexOf(','),1);
    l1.append("}");
    l2.append("}");

    return qCompare(l1, l2, actual, expected, file, line);
}

} // namespace QTest

/*?
    Construct QCategoryManager.
*/
tst_QCategoryManager::tst_QCategoryManager() :
 _cat("tst_QCategoryManager"), _scope("tst_QCategoryManager_scope"),
 CATMAX(20), SCOPEMAX(20), doClean(true)
{
}

/*?
    Cleanup after all test functions.
    Calls \l {removeCategories()}.
*/
void tst_QCategoryManager::cleanupTestCase()
{
    removeCategories();
}

/*?
    Initialisation before each test function.
    Calls \l {removeCategories()} if doClean is true,
    and sets doClean to false.
*/
void tst_QCategoryManager::init()
{
    if (doClean) removeCategories();
    doClean = false;
}

void tst_QCategoryManager::bug180072()
{
    QCategoryManager catman("some unique scope");

    QVERIFY( catman.addCategory("some id 1", "some label") );
    QVERIFY( catman.setLabel("some id 1", "some other label" ) );
    QVERIFY( catman.remove("some id 1") );

    QVERIFY( catman.addCategory("some id 2", "some label") );
    QVERIFY( catman.setLabel("some id 2", QString() ) );
    //QEXPECT_FAIL( "", "Bug 180072", Continue );
    QVERIFY( catman.remove("some id 2") );
}

/*?
    Test accessors for the static QCategoryManager labels.
    Simply compares the result of the static QCategoryManager::*Label()
    functions against expected values.
*/
void tst_QCategoryManager::staticLabels()
{
    QCOMPARE(QCategoryManager::allLabel(), tr("All"));
    QCOMPARE(QCategoryManager::unfiledLabel(), tr("Unfiled"));
    QCOMPARE(QCategoryManager::multiLabel(), tr("(Multi) ..."));
}

/*?
    Remove all categories this class might have created.
    Does up to O(SCOPEMAX*CATMAX) SQL queries, so don't make these
    too large!
*/
void tst_QCategoryManager::removeCategories()
{
    globalSystemCats.clear();
    /* For each scope, including global... */
    for (int j = -2; j < SCOPEMAX; ++j) {
        QString myScope;
        /* -2 means global */
        if      (j == -2) {}
        /* -1 means scope w/no appended number */
        else if (j == -1) myScope = _scope;
        /* Anything else means scope<num> */
        else              myScope = QString("%1%2").arg(_scope).arg(j);

        QCategoryManager catman( myScope );
        foreach(QString str, catman.categoryIds()) {
            /* System categories cannot be removed; record all global
             * system categories so we are aware of their existence
             * for subsequent tests. */
            if (catman.isSystem( str )) {
                if (j == -2) {
                    globalSystemCats << str;
                }
            }
            else catman.remove(str);
        }
    }
}

/*?
    Test emission of \l{QCategoryManager}{categoriesChanged()} signal.

    This test ensures that \l{QCategoryManager}{categoriesChanged()} is
    emitted in all the following cases:
        * A category is added through \l{QCategoryManager}{addCategory()}.
        * A category is set to global via \l{QCategoryManager}{setGlobal()}.
        * A category is set to not global via \l{QCategoryManager}{setGlobal()}.
        * A category's label is changed via \l{QCategoryManager}{setLabel()}.
        * A category's icon is changed via \l{QCategoryManager}{setIcon()}.
        * A category is removed via \l{QCategoryManager}{remove()}.
*/
void tst_QCategoryManager::categoriesChanged()
{
    QCategoryManager catman(_scope);
    QSignalSpy spy(&catman,SIGNAL(categoriesChanged()));
    QTest::qWait(100);
    spy.clear();
    QString cat = "tst_QCategoryManager";

    for (int i = 0; i < 50; ++i) {
        QVERIFY( catman.addCategory( cat, "some label" ) );
        QTRY_COMPARE( spy.count(), 1 ); spy.takeLast();

        QVERIFY( catman.contains(cat) );

        QVERIFY( catman.setGlobal( cat, true ) );
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
        QVERIFY( catman.isGlobal( cat ) );

        QVERIFY( catman.setGlobal( cat, false ) );
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
        QVERIFY( !catman.isGlobal( cat ) );

        QVERIFY( catman.setLabel( cat, "a different label" ) );
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
        QCOMPARE( catman.label( cat ), QString("a different label") );

        QVERIFY( catman.setLabel( cat, "another different label" ) );
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
        QCOMPARE( catman.label( cat ), QString("another different label") );

        /*
        QVERIFY( catman.setLabel( cat, QString() ) );
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
        QCOMPARE( catman.label( cat ), QString() );
        */

        QVERIFY( catman.setIcon( cat, "some icon" ) );
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
        QCOMPARE( catman.iconFile( cat ), QString("some icon") );

        QVERIFY( catman.setIcon( cat, "another icon" ) );
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
        QCOMPARE( catman.iconFile( cat ), QString("another icon") );

        QVERIFY( catman.setIcon( cat, QString() ) );
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
        QCOMPARE( catman.iconFile( cat ), QString() );

        QVERIFY( catman.remove( cat ) );
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
    }
    doClean = true;
}

/*?
    Test that QCategoryManagers in separate scopes interact correctly.
    This is a highly state-based test.  It does the following:
        * Constructs a QCategoryManager for a specified scope (can be global).
        * Depending on a specified `Action', does one of:
            - Add a specified category with a unique label and icon, and checks
              success against a specified value.
            - Removes a specified cateogry and checks success against a specified
              value.
            - Removes all categories from all scopes.
        * Checks every category available through the current QCategoryManager
          matches a set of expected categories.
        * Checks the icon and label for every category matches expected values.
*/
void tst_QCategoryManager::scope()
{
    QFETCH(QString, scope);
    QFETCH(QString, cat);
    QFETCH(ScopeTestAction, action);
    QFETCH(QSet<QString>, expectedCats);
    QFETCH(QSet<QString>, allExpectedCats);
    QFETCH(bool, success);

    QCategoryManager catman(scope);
    if (Add == action) {
        QCOMPARE(catman.addCategory(cat, QString(cat).append("_label"), QString(cat).append("_icon")), success);
    }
    else if (Remove == action) {
        QCOMPARE(catman.remove(cat), success);
    }
    else if (RemoveAll == action) {
        removeCategories();
    }

    QCOMPARE(catman.categoryIds().toSet(), expectedCats);


    /* Go through all expected categories and check the icons and labels
     * are as expected. */
    QList<QString> labels;
    foreach (QString str, expectedCats) {
        /* For global system cats, we didn't create them, so we don't know
         * what their icon or label should be. */
        if (!globalSystemCats.contains(str)) {
            QString iconstr = QString(str).append("_icon");
            QString labelstr = QString(str).append("_label");

            QCOMPARE( catman.iconFile(str), iconstr );
            QCOMPARE( catman.label(str), labelstr );
            QVERIFY(catman.containsLabel(labelstr));
            QCOMPARE( catman.idForLabel(labelstr), str );

            labels << labelstr;
        }
        else {
            QCOMPARE( catman.idForLabel(catman.label(str)), str );
            labels << catman.label(str);
        }
    }

    /* Check the labels() function. */
    QCOMPARE(catman.labels( expectedCats.toList() ).toSet(), labels.toSet()); 
    /* Should ignore nonexistent categories. */
    QCOMPARE(catman.labels( expectedCats.toList() << "nonexistent1" << "nonexistent2").toSet(), labels.toSet()); 

    foreach (QString str, allExpectedCats) {
        QVERIFY( catman.exists(str) );
    }

}

/*?
    Data for scope() test function.
    The data in this test strongly depends on state, hence
    the order matters.
*/
void tst_QCategoryManager::scope_data()
{
    removeCategories();

    QTest::addColumn<QString>("scope");
    QTest::addColumn<QString>("cat");
    QTest::addColumn<ScopeTestAction>("action");
    QTest::addColumn< QSet<QString> >("expectedCats");    // global and specific scope categories
    QTest::addColumn< QSet<QString> >("allExpectedCats"); // categories across _all_ scopes
    QTest::addColumn<bool>("success");
    
    /* expCats* - set of expected categories for each scope */
    QSet<QString> expCats = globalSystemCats.toSet();
    QSet<QString> expCatsA;
    QSet<QString> expCatsB;

    /* sc* - strings for each scope */
    QString scA = QString("%1%2").arg(_scope).arg(0);
    QString scB = QString("%1%2").arg(_scope).arg(1);
    QString sc;

    int checks = 0;
    int adds = 0;
    int addsA = 0;
    int addsB = 0;
    int removes = 0;
    int removesA = 0;
    int removesB = 0;

    /* Check the contents of each scope against expected values */
#define CHECK_ALL() { \
    QTest::newRow(QString("check global %1").arg(checks).toLatin1()) \
        << QString() << QString() << Check << expCats << (expCats|expCatsA|expCatsB) << true;    \
    QTest::newRow(QString("check %1 %2").arg(scA).arg(checks).toLatin1())   \
        << scA << QString() << Check << (expCats|expCatsA) << (expCats|expCatsA|expCatsB) << true; \
    QTest::newRow(QString("check %1 %2").arg(scB).arg(checks).toLatin1())   \
        << scB << QString() << Check << (expCats|expCatsB) << (expCats|expCatsA|expCatsB) << true; \
    ++checks;                                                     \
}

    /* Add category 'B' to scope 'A' */
#define ADD(A, B) { \
    expCats##A << B; \
    QTest::newRow(QString("add %1 to %2").arg(B).arg((!sc##A.isEmpty()) ? sc##A : "global").toLatin1()) \
       << sc##A \
       << B \
       << Add \
       << (expCats|expCats##A) \
       << (expCats|expCatsA|expCatsB) \
       << true; \
    ++adds##A; \
}

    /* Remove category 'B' from scope 'A' */
#define REMOVE(A, B) { \
    expCats##A -= B; \
    QTest::newRow(QString("remove %1 from %2").arg(B).arg((!sc##A.isEmpty()) ? sc##A : "global").toLatin1()) \
       << sc##A \
       << B \
       << Remove \
       << (expCats|expCats##A) \
       << (expCats|expCatsA|expCatsB) \
       << true; \
    ++removes##A; \
}

    CHECK_ALL();

    ADD(, "nice_cat");
    CHECK_ALL();

    ADD(, "nice_cat2");
    CHECK_ALL();

    ADD(A, "nice_cat_A_only");
    CHECK_ALL();

    ADD(B, "nice_cat_B_only");
    CHECK_ALL();

    QTest::newRow("remove nonexistent from B")
        << scB
        << "nonexistent"
        << Remove
        << (expCats|expCatsB)
        << (expCats|expCatsA|expCatsB)
        << false;
    CHECK_ALL();

    QTest::newRow("remove nonexistent from global")
        << QString()
        << "nonexistent"
        << Remove
        << expCats
        << (expCats|expCatsA|expCatsB)
        << false;
    CHECK_ALL();

    expCats -= "nice_cat";
    QTest::newRow("remove global from B")
        << scB
        << "nice_cat"
        << Remove
        << (expCats|expCatsB)
        << (expCats|expCatsA|expCatsB)
        << true;
    CHECK_ALL();

    ADD(B, "nice_cat_B_only_2");
    CHECK_ALL();

    ADD(A, "yet another great category");
    CHECK_ALL();

    REMOVE(A, "nice_cat_A_only");
    CHECK_ALL();

    QTest::newRow("add existing to B")
        << scB
        << "nice_cat_B_only_2"
        << Add
        << (expCats|expCatsB)
        << (expCats|expCatsA|expCatsB)
        << false;
    CHECK_ALL();

    QTest::newRow("remove B from global")
        << sc
        << "nice_cat_B_only_2"
        << Remove
        << expCats
        << (expCats|expCatsA|expCatsB)
        << false;
    CHECK_ALL();

    REMOVE(, "nice_cat2");
    CHECK_ALL();

    ADD(, "nice_cat2");
    CHECK_ALL();

    ADD(A, "nice_catA2");
    CHECK_ALL();

    ADD(B, "nice_catB2");
    CHECK_ALL();

    expCats = globalSystemCats.toSet();
    expCatsA.clear();
    expCatsB.clear();
    QTest::newRow("remove all")
        << sc
        << QString()
        << RemoveAll
        << expCats
        << (expCats|expCatsA|expCatsB)
        << true;
    CHECK_ALL();

#undef CHECK_ALL
#undef ADD
#undef REMOVE
}

