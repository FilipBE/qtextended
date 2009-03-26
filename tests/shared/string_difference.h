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

#ifndef STRING_DIFFERENCE_H
#define STRING_DIFFERENCE_H

// Include this file to add diagnostic string comparison, printed to the
// autotest log in the case of string comparison failure.
// Output is only visible when test is invoked with '-v', or from 'make test'
// when ARGS includes '-v'.

#ifdef QTOPIA_TEST

#include <qtopialog.h>
#include <qtopiaglobal.h>
#include <shared/qtopiaunittest.h>

#include <QByteArray>
#include <QLatin1String>
#include <QString>

namespace QTest {

    // Report the difference between two strings, which can be very hard to see
    template<typename StringType>
    void stringDifference(StringType& actual, StringType& expected)
    {
        QString output;

        if (actual != expected) {
            int maxCompare = qMin(actual.length(), expected.length());

            // Find where they differ:
            if (maxCompare > 0) {
                typename StringType::const_iterator begin = actual.constData();
                typename StringType::const_iterator end = begin + maxCompare;

                typename StringType::const_iterator li = begin;
                typename StringType::const_iterator ri = expected.constData();
                while ((li != end) && (*li++ == *ri++));

                if (li == end) {
                    QString descriptor = (maxCompare == actual.length() ? "Expected" : "Actual");
                    const StringType& trailing = (maxCompare == actual.length() ? expected : actual);
                    output.append(QString("   %1 has %2 trailing characters after %3:\n      \"%4\"")
                                         .arg(descriptor)
                                         .arg(trailing.length() - maxCompare)
                                         .arg(maxCompare)
                                         .arg(QString(trailing.mid(maxCompare))));
                } else {
                    int same = (li - begin - 1);

                    if (same) {
                        output.append(QString("   Actual matches Expected for %1 characters:\n      \"%2\"")
                                             .arg(same)
                                             .arg(QString(actual.left(same))));
                    } else {
                        output.append(QString("   Actual has no match with Expected"));
                    }

                    output.append(QString("\n   Actual continues for %1 characters:\n      \"%2\"")
                                         .arg(actual.length() - same)
                                         .arg(QString(actual.mid(same))));
                    output.append(QString("\n   Expected continues for %1 characters:\n      \"%2\"")
                                         .arg(expected.length() - same)
                                         .arg(QString(expected.mid(same))));
                }
            } else {
                QString descriptor = (actual.length() ? "Expected" : "Actual");
                output.append(QString("   %1 is empty").arg(descriptor));
            }
        }

        if (!output.isNull())
            qLog(Autotest).nospace() << "String difference:\n" << qPrintable(output);
    }

    // Specialize for char*
    template<>
    void stringDifference(const char*& actual, const char*& expected)
    {
        stringDifference<const QByteArray>(QByteArray(actual), QByteArray(expected));
    }

    // Override qCompare to report the differences for QByteArray
    template<>
    inline bool qCompare(const QByteArray& t1, const QByteArray& t2, const char* actual, const char* expected, const char* file, int line)
    {
        bool result((t1 == t2)
                     ? compare_helper(true, "COMPARE()", file, line)
                     : compare_helper(false, "Compared values are not the same", toString(t1), toString(t2), actual, expected, file, line));

        if (!result)
            stringDifference(t1, t2);

        return result;
    }

    // Override qCompare to report the differences for QLatin1String
    template<>
    inline bool qCompare(const QLatin1String& t1, const QLatin1String& t2, const char* actual, const char* expected, const char* file, int line)
    {
        return qCompare(t1.latin1(), t2.latin1(), actual, expected, file, line);
    }

    // Overload qCompare because char* is already specialized...
    inline bool qCompare(const char* t1, const char* t2, const char* actual, const char* expected, const char* file, int line)
    {
        bool result(qstrcmp(t1, t2) == 0
                     ? compare_helper(true, "COMPARE()", file, line)
                     : compare_helper(false, "Compared values are not the same", toString(t1), toString(t2), actual, expected, file, line));

        if (!result)
            stringDifference(t1, t2);

        return result;
    }

    // Overload qCompare because QString is already specialized...
    inline bool qCompare(const QString& t1, const QString& t2, const char* actual, const char* expected, const char* file, int line)
    {
        bool result((t1 == t2)
                     ? compare_helper(true, "COMPARE()", file, line)
                     : compare_helper(false, "Compared values are not the same", toString(t1), toString(t2), actual, expected, file, line));

        if (!result)
            stringDifference(t1, t2);

        return result;
    }

    // We also need to specialize qTest for QString
    template <>
    inline bool qTest(const QString& actual, const char *elementName, const char *actualStr, const char *expected, const char *file, int line)
    {
        return qCompare(actual, *static_cast<const QString*>(QTest::qElementData(elementName, qMetaTypeId<QString>())), actualStr, expected, file, line);
    }

}

#endif

#endif
