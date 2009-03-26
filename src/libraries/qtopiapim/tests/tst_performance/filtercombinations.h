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

#ifndef TST_PERFORMANCE_H
#define TST_PERFORMANCE_H

#include <QApplication>
#include <QString>
#include <QStringList>

class FilterCombinations {
public:
    FilterCombinations() : testNewFilters(false) {}
    virtual ~FilterCombinations() {}

    enum FieldType {
        NoField,
        ActionTag,
        ActionId,
        FieldId
    };

    QString categoryFilter() const { return m_categoryFilter; }
    QList<int> contextFilter() const { return m_contextFilter; }

    FieldType fieldFilterType() const { return m_filterType; }
    QString fieldFilterText() const { return m_filterText; }

    QString labelFilter() const { return m_labelFilter; }
    QString valueFilter() const { return m_valueFilter; }

    QString sortField() const { return m_sortField; }

    FieldType primaryFieldType() const { return m_primaryFieldType; }
    QString primaryFieldText() const { return m_primaryFieldText; }

    void resetTestCombination();
    bool prepareNextTestCombination();

private:
    bool testNewFilters;

    QString m_categoryFilter;
    QList<int> m_contextFilter;

    FieldType m_filterType;
    QString m_filterText;

    QString m_labelFilter;
    QString m_valueFilter;

    QString m_sortField;

    FieldType m_primaryFieldType;
    QString m_primaryFieldText;

    QString m_actionFilter;
    QString m_actionTagFilter;
    QString m_fieldTagFilter;
};

#endif
