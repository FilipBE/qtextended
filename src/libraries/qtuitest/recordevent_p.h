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

#ifndef RECORDEVENT_P_H
#define RECORDEVENT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QString>
#include <QVariant>

struct RecordEvent {
    enum Type {
        GotFocus,
        Entered,
        Selected,
        Activated,
        CheckStateChanged,
        TitleChanged,
        MessageBoxShown
    };
    Type     type;
    QString  widget;
    QString  focusWidget;
    QVariant data;
};

#define Q_DECLARE_METATYPE_STREAM(TYPE) \
template <> \
struct QMetaTypeId< TYPE > \
{ \
        enum { Defined = 1 }; \
        static int qt_metatype_id() \
        { \
            static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0);  \
            if (!metatype_id) {                                     \
                metatype_id = qRegisterMetaType< TYPE >(#TYPE);     \
                qRegisterMetaTypeStreamOperators< TYPE >(#TYPE);    \
            }                                                       \
            return metatype_id;                                     \
        } \
};

Q_DECLARE_METATYPE_STREAM(RecordEvent);
Q_DECLARE_METATYPE_STREAM(QList<RecordEvent>);

inline bool operator==(RecordEvent const& a, RecordEvent const& b)
{ return a.type == b.type && a.widget == b.widget && a.focusWidget == b.focusWidget && a.data == b.data; }

inline bool operator!=(RecordEvent const& a, RecordEvent const& b)
{ return !(a == b); }

inline QDataStream &operator<<(QDataStream &out, const RecordEvent &re)
{ return (out << static_cast<int>(re.type) << re.widget << re.focusWidget << re.data); }

inline QDataStream &operator>>(QDataStream &in, RecordEvent &re)
{
    int reType;
    QDataStream &ret = (in >> reType >> re.widget >> re.focusWidget >> re.data);
    re.type = static_cast<RecordEvent::Type>(reType);
    return ret;
}

inline QDataStream &operator<<(QDataStream &out, const QList<RecordEvent> &l)
{
    out << l.count();
    foreach (RecordEvent re, l) {
        out << re;
    }
    return out;
}
inline QDataStream &operator>>(QDataStream &in, QList<RecordEvent> &l)
{
    int count = 0;
    in >> count;
    RecordEvent re;
    for (int i = 0; i < count; ++i) {
        in >> re;
        l << re;
    }
    return in;
}

#endif

