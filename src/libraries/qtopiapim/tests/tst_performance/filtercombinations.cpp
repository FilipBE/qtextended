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

#include "filtercombinations.h"

#include <QDebug>

/*
    Valid combinations for alternative
        [context: none : single : multiple]
        [category: none : single]
        [action: none : single actionTag : single actionId : single fieldId]
        [labelSearch: none : string]
        [valueMatch: none : string]
        [sort: label : value]
        [primaryValue: talk(a) : message(a) : company(f) ]

    Valid combinations that also work on the original
        [context: none : single : multiple]
        [category: none : single]
        [labelSearch: none : string]
        [sort: label : value]
*/

#if 0
int *contexts[] = {
    { 0 },
    { 4, 5, 0 },
    { 1, 0 },
    { 4, 0 },
    { 5, 0},
    0
};

char * categories[] = {
    "", "Business", "Personal", 0
};

struct FieldFilter {
    FilterCombinations::FieldType type,
    char *text;
};

FieldFilter fieldFilters[] = {
    { FilterCombinations::NoField, 0 },
    { FilterCombinations::ActionTag, "talk" },
    { FilterCombinations::ActionTag, "message" },
    { FilterCombinations::ActionId, "email" },
    { FilterCombinations::FieldId, "home phone" },
    0
};

char *label[] = {
    "", "s", "sa", "sar", "w", "wa", "war", "b", "ba", "bar", 0
};

char *value[] = {
    "", "9244217", "4103495", 0
};

FieldFilter primaryField[] = {
    { FilterCombinations::ActionTag, "talk" },
    { FilterCombinations::ActionTag, "message" },
    0
};

char *sort[] = {
    "", "company", 0
};
#else

struct FieldFilter {
    FilterCombinations::FieldType type;
    char *text;
};

int contextAll[] = { 0 };
int contextPhoneOnly[] = { 1, 0 };
int contextSimOnly[] = { 4, 5, 0 };

int contextsCount = 3;
int *contexts[] = {
    contextAll,
    contextPhoneOnly,
    contextSimOnly
};

int categoriesCount = 2;
char * categories[] = {
    "", "Business"
};

int fieldFiltersCount = 3;
FieldFilter fieldFilters[] = {
    { FilterCombinations::NoField, 0 },
    { FilterCombinations::ActionId, "dial" },
    { FilterCombinations::ActionId, "email" },
};

int labelCount = 2;
char *label[] = {
    "", "s"
};

int valueCount = 2;
char *value[] = {
    "", "9244217"
};

int primaryFieldCount = 1;
FieldFilter primaryField[] = {
    { FilterCombinations::ActionTag, "talk" },
};

int sortCount = 2;
char *sort[] = {
    "", "company"
};
#endif

int combinationIndex = 0;

int totalCount = 0;

void FilterCombinations::resetTestCombination()
{
    combinationIndex = 0;
    totalCount = categoriesCount*contextsCount*fieldFiltersCount*labelCount*valueCount*primaryFieldCount*sortCount;
    qDebug() << "testing a" << totalCount << "filter and search combinations";
}

bool FilterCombinations::prepareNextTestCombination()
{
    if (combinationIndex >= totalCount)
        return false;

    int index = combinationIndex;

    int categoriesIndex = index % categoriesCount;
    index /= categoriesCount;
    int contextsIndex = index % contextsCount;
    index /= contextsCount;
    int fieldFiltersIndex = index % fieldFiltersCount;
    index /= fieldFiltersCount;
    int labelIndex = index % labelCount;
    index /= labelCount;
    int valueIndex = index % valueCount;
    index /= valueCount;
    int primaryFieldIndex = index % primaryFieldCount;
    index /= primaryFieldCount;
    int sortIndex = index % sortCount;

    m_categoryFilter = categories[categoriesIndex];

    int *contextList = contexts[contextsIndex];
    m_contextFilter.clear();
    while(*contextList)
        m_contextFilter.append(*contextList++);


    m_filterType = fieldFilters[fieldFiltersIndex].type;
    m_filterText = fieldFilters[fieldFiltersIndex].type;

    m_primaryFieldType = primaryField[primaryFieldIndex].type;
    m_primaryFieldText = primaryField[primaryFieldIndex].type;

    m_labelFilter = label[labelIndex];
    m_valueFilter = value[valueIndex];

    m_sortField = sort[sortIndex];

    combinationIndex++;
    return true;
}

