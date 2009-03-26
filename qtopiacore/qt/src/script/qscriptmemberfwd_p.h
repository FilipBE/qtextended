/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtScript module of the Qt Toolkit.
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

#ifndef QSCRIPTMEMBERFWD_P_H
#define QSCRIPTMEMBERFWD_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SCRIPT

class QScriptNameIdImpl;

namespace QScript {

    class Member
    {
    public:
        enum PropertyFlag {
            ObjectProperty      = 0x00000100, // Stored in the member table
            NativeProperty      = 0x00000200,

            UninitializedConst  = 0x00000800, // NB: shared with QScriptValue::KeepExistingFlags

            InternalRange       = 0x0000ff00  // Not user-accessible (read as 0, don't change on write)
        };

        inline void resetFlags(uint flags);
        inline void setFlags(uint flags);
        inline void unsetFlags(uint flags);
        inline uint flags() const;
        inline bool testFlags(uint mask) const;

        inline bool isValid() const;

        inline bool isWritable() const;
        inline bool isDeletable() const;

        inline bool dontEnum() const;

        inline bool isObjectProperty() const;
        inline bool isNativeProperty() const;

        inline bool isUninitializedConst() const;

        inline bool isGetter() const;
        inline bool isSetter() const;
        inline bool isGetterOrSetter() const;

        inline int id() const;
        inline QScriptNameIdImpl *nameId() const;

        inline bool operator==(const Member &other) const;
        inline bool operator!=(const Member &other) const;

        inline static Member invalid();
        inline void invalidate();

        inline void native(QScriptNameIdImpl *nameId, int id, uint flags);
        inline void object(QScriptNameIdImpl *nameId, int id, uint flags);

    private:
        QScriptNameIdImpl *m_nameId;
        int m_id;
        uint m_flags;
    };

} // namespace QScript

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT

#endif
