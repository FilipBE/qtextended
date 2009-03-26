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

#include "qtscript_qtcore.h"

#include <QtScript>
#include <QtCore>

template <typename T>
static QScriptValue _q_ScriptValueFromEnum(QScriptEngine *engine, T const &in)
{
    return QScriptValue(engine, static_cast<int>(in));
}

template <typename T>
static void _q_ScriptValueToEnum(const QScriptValue &v, T &out)
{
    if (v.isVariant() && qVariantCanConvert<T>(v.toVariant()))
        out = qvariant_cast<T>(v.toVariant());
    else
        out = T(v.toInt32());
}

template <typename T>
static int _q_ScriptRegisterEnumMetaType(QScriptEngine *engine)
{
    return qScriptRegisterMetaType<T>(engine, _q_ScriptValueFromEnum<T>, _q_ScriptValueToEnum<T>);
}

template <typename T>
static QScriptValue _q_ScriptValueFromQObject(QScriptEngine *engine, T* const &in)
{
    return engine->newQObject(in);
}
template <typename T>
static void _q_ScriptValueToQObject(const QScriptValue &v, T* &out)
{    out = qobject_cast<T*>(v.toQObject());
}
template <typename T>
static int _q_ScriptRegisterQObjectMetaType(QScriptEngine *engine, const QScriptValue &prototype)
{
    return qScriptRegisterMetaType<T*>(engine, _q_ScriptValueFromQObject<T>, _q_ScriptValueToQObject<T>, prototype);
}

template <typename T>
static QScriptValue _q_ScriptValueFromValue(QScriptEngine *engine, T const &in)
{
    return engine->newVariant(QVariant::fromValue(in));
}
template <typename T>
static void _q_ScriptValueToValue(const QScriptValue &v, T &out)
{
    out = qvariant_cast<T>(v.toVariant());
}
template <>
QScriptValue _q_ScriptValueFromValue(QScriptEngine *engine, QVariant const &in)
{
    return engine->newVariant(in);
}
template <>
void _q_ScriptValueToValue(const QScriptValue &v, QVariant &out)
{
    out = v.toVariant();
}
template <typename T>
static int _q_ScriptRegisterValueMetaType(QScriptEngine *engine, const QScriptValue &prototype)
{
    return qScriptRegisterMetaType<T>(engine, _q_ScriptValueFromValue<T>, _q_ScriptValueToValue<T>, prototype);
}

Q_DECLARE_METATYPE(Qt::CursorShape)
Q_DECLARE_METATYPE(Qt::Corner)
Q_DECLARE_METATYPE(Qt::Axis)
Q_DECLARE_METATYPE(Qt::Orientation)
Q_DECLARE_METATYPE(Qt::LayoutDirection)
Q_DECLARE_METATYPE(Qt::BGMode)
Q_DECLARE_METATYPE(Qt::AspectRatioMode)
Q_DECLARE_METATYPE(Qt::TextElideMode)
Q_DECLARE_METATYPE(Qt::WindowType)
Q_DECLARE_METATYPE(Qt::ItemDataRole)
Q_DECLARE_METATYPE(Qt::SortOrder)
Q_DECLARE_METATYPE(Qt::MatchFlag)
Q_DECLARE_METATYPE(Qt::PenJoinStyle)
Q_DECLARE_METATYPE(Qt::CaseSensitivity)
Q_DECLARE_METATYPE(Qt::BrushStyle)
Q_DECLARE_METATYPE(Qt::ClipOperation)
Q_DECLARE_METATYPE(Qt::FocusReason)
Q_DECLARE_METATYPE(Qt::ToolBarArea)
Q_DECLARE_METATYPE(Qt::KeyboardModifier)
Q_DECLARE_METATYPE(Qt::DayOfWeek)
Q_DECLARE_METATYPE(Qt::EventPriority)
Q_DECLARE_METATYPE(Qt::DateFormat)
Q_DECLARE_METATYPE(Qt::MaskMode)
Q_DECLARE_METATYPE(Qt::UIEffect)
Q_DECLARE_METATYPE(Qt::ContextMenuPolicy)
Q_DECLARE_METATYPE(Qt::AnchorAttribute)
Q_DECLARE_METATYPE(Qt::ScrollBarPolicy)
Q_DECLARE_METATYPE(Qt::ToolButtonStyle)
Q_DECLARE_METATYPE(Qt::TextFlag)
Q_DECLARE_METATYPE(Qt::ItemSelectionMode)
Q_DECLARE_METATYPE(Qt::Key)
Q_DECLARE_METATYPE(Qt::ToolBarAreaSizes)
Q_DECLARE_METATYPE(Qt::ArrowType)
Q_DECLARE_METATYPE(Qt::FocusPolicy)
Q_DECLARE_METATYPE(Qt::InputMethodQuery)
Q_DECLARE_METATYPE(Qt::DropAction)
Q_DECLARE_METATYPE(Qt::FillRule)
Q_DECLARE_METATYPE(Qt::GlobalColor)
Q_DECLARE_METATYPE(Qt::ConnectionType)
Q_DECLARE_METATYPE(Qt::PenCapStyle)
Q_DECLARE_METATYPE(Qt::TransformationMode)
Q_DECLARE_METATYPE(Qt::DockWidgetAreaSizes)
Q_DECLARE_METATYPE(Qt::ApplicationAttribute)
Q_DECLARE_METATYPE(Qt::ShortcutContext)
Q_DECLARE_METATYPE(Qt::TextInteractionFlag)
Q_DECLARE_METATYPE(Qt::CheckState)
Q_DECLARE_METATYPE(Qt::DockWidgetArea)
Q_DECLARE_METATYPE(Qt::TimeSpec)
Q_DECLARE_METATYPE(Qt::ImageConversionFlag)
Q_DECLARE_METATYPE(Qt::WindowModality)
Q_DECLARE_METATYPE(Qt::Modifier)
Q_DECLARE_METATYPE(Qt::AlignmentFlag)
Q_DECLARE_METATYPE(Qt::WidgetAttribute)
Q_DECLARE_METATYPE(Qt::TextFormat)
Q_DECLARE_METATYPE(Qt::MouseButton)
Q_DECLARE_METATYPE(Qt::WindowState)
Q_DECLARE_METATYPE(Qt::PenStyle)
Q_DECLARE_METATYPE(Qt::ItemFlag)
Q_DECLARE_METATYPE(Qt::ImageConversionFlags)
Q_DECLARE_METATYPE(Qt::KeyboardModifiers)
Q_DECLARE_METATYPE(Qt::DropActions)
Q_DECLARE_METATYPE(Qt::Orientations)
Q_DECLARE_METATYPE(Qt::MouseButtons)
Q_DECLARE_METATYPE(Qt::WindowStates)
Q_DECLARE_METATYPE(Qt::ToolBarAreas)
Q_DECLARE_METATYPE(Qt::Alignment)
Q_DECLARE_METATYPE(Qt::TextInteractionFlags)
Q_DECLARE_METATYPE(Qt::WindowFlags)
Q_DECLARE_METATYPE(Qt::DockWidgetAreas)
Q_DECLARE_METATYPE(Qt::ItemFlags)
Q_DECLARE_METATYPE(Qt::MatchFlags)

// QSystemLocale

Q_DECLARE_METATYPE(QSystemLocale)
Q_DECLARE_METATYPE(QSystemLocale*)
Q_DECLARE_METATYPE(QSystemLocale::QueryType)

class Constructor_QSystemLocale:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QSystemLocale()); }
};

class Prototype_QSystemLocale:
public QObject, public QSystemLocale, protected QScriptable
{
    Q_OBJECT
private:
inline QSystemLocale *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QSystemLocale*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QSystemLocale.prototype.%0: this object is not a QSystemLocale")
            .arg(method));
}
public:
Prototype_QSystemLocale(QObject *parent = 0)
    : QSystemLocale() { setParent(parent); }
public Q_SLOTS:
QVariant query(QSystemLocale::QueryType type, QVariant in) const
{ if (QSystemLocale *_q_q = this->self()) return _q_q->query(type, in);
  throwTypeError(QLatin1String("query")); return QVariant(); }
QLocale fallbackLocale() const
{ if (QSystemLocale *_q_q = this->self()) return _q_q->fallbackLocale();
  throwTypeError(QLatin1String("fallbackLocale")); return QLocale(); }
QString toString() const
{ return QLatin1String("QSystemLocale"); }
};

static QScriptValue create_QSystemLocale_class(QScriptEngine *engine)
{
    Prototype_QSystemLocale *pq = new Prototype_QSystemLocale;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QSystemLocale *cq = new Constructor_QSystemLocale;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QSystemLocale>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QSystemLocale>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QSystemLocale*>(), pv);
    _q_ScriptRegisterEnumMetaType<QSystemLocale::QueryType>(engine);
    cv.setProperty("LanguageId", QScriptValue(engine, static_cast<int>(QSystemLocale::LanguageId)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CountryId", QScriptValue(engine, static_cast<int>(QSystemLocale::CountryId)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DecimalPoint", QScriptValue(engine, static_cast<int>(QSystemLocale::DecimalPoint)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GroupSeparator", QScriptValue(engine, static_cast<int>(QSystemLocale::GroupSeparator)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ZeroDigit", QScriptValue(engine, static_cast<int>(QSystemLocale::ZeroDigit)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NegativeSign", QScriptValue(engine, static_cast<int>(QSystemLocale::NegativeSign)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DateFormatLong", QScriptValue(engine, static_cast<int>(QSystemLocale::DateFormatLong)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DateFormatShort", QScriptValue(engine, static_cast<int>(QSystemLocale::DateFormatShort)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TimeFormatLong", QScriptValue(engine, static_cast<int>(QSystemLocale::TimeFormatLong)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TimeFormatShort", QScriptValue(engine, static_cast<int>(QSystemLocale::TimeFormatShort)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DayNameLong", QScriptValue(engine, static_cast<int>(QSystemLocale::DayNameLong)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DayNameShort", QScriptValue(engine, static_cast<int>(QSystemLocale::DayNameShort)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MonthNameLong", QScriptValue(engine, static_cast<int>(QSystemLocale::MonthNameLong)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MonthNameShort", QScriptValue(engine, static_cast<int>(QSystemLocale::MonthNameShort)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DateToStringLong", QScriptValue(engine, static_cast<int>(QSystemLocale::DateToStringLong)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DateToStringShort", QScriptValue(engine, static_cast<int>(QSystemLocale::DateToStringShort)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TimeToStringLong", QScriptValue(engine, static_cast<int>(QSystemLocale::TimeToStringLong)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TimeToStringShort", QScriptValue(engine, static_cast<int>(QSystemLocale::TimeToStringShort)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QWaitCondition

Q_DECLARE_METATYPE(QWaitCondition*)

class Constructor_QWaitCondition:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(new QWaitCondition()); }
};

class Prototype_QWaitCondition:
public QObject, public QWaitCondition, protected QScriptable
{
    Q_OBJECT
private:
inline QWaitCondition *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QWaitCondition*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QWaitCondition.prototype.%0: this object is not a QWaitCondition")
            .arg(method));
}
public:
Prototype_QWaitCondition(QObject *parent = 0)
    : QWaitCondition() { setParent(parent); }
public Q_SLOTS:
void wakeAll()
{ if (QWaitCondition *_q_q = this->self()) _q_q->wakeAll();
  else throwTypeError(QLatin1String("wakeAll")); }
bool wait(QMutex* mutex, unsigned long time = ULONG_MAX)
{ if (QWaitCondition *_q_q = this->self()) return _q_q->wait(mutex, time);
  throwTypeError(QLatin1String("wait")); return bool(); }
void wakeOne()
{ if (QWaitCondition *_q_q = this->self()) _q_q->wakeOne();
  else throwTypeError(QLatin1String("wakeOne")); }
QString toString() const
{ return QLatin1String("QWaitCondition"); }
private:
Q_DISABLE_COPY(Prototype_QWaitCondition)
};

static QScriptValue create_QWaitCondition_class(QScriptEngine *engine)
{
    Prototype_QWaitCondition *pq = new Prototype_QWaitCondition;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QWaitCondition *cq = new Constructor_QWaitCondition;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QWaitCondition*>(), pv);
    return cv;
}

// QUuid

Q_DECLARE_METATYPE(QUuid)
Q_DECLARE_METATYPE(QUuid*)
Q_DECLARE_METATYPE(QUuid::Variant)
Q_DECLARE_METATYPE(QUuid::Version)

class Constructor_QUuid:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(char const* arg1)
{ return this->engine()->toScriptValue(QUuid(arg1)); }
QScriptValue qscript_call(QString const& arg1)
{ return this->engine()->toScriptValue(QUuid(arg1)); }
QScriptValue qscript_call(uint l, ushort w1, ushort w2, uchar b1, uchar b2, uchar b3, uchar b4, uchar b5, uchar b6, uchar b7, uchar b8)
{ return this->engine()->toScriptValue(QUuid(l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QUuid()); }
QUuid createUuid()
{ return QUuid::createUuid(); }
};

class Prototype_QUuid:
public QObject, public QUuid, protected QScriptable
{
    Q_OBJECT
private:
inline QUuid *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QUuid*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QUuid.prototype.%0: this object is not a QUuid")
            .arg(method));
}
public:
Prototype_QUuid(QObject *parent = 0)
    : QUuid() { setParent(parent); }
public Q_SLOTS:
QUuid::Version version() const
{ if (QUuid *_q_q = this->self()) return _q_q->version();
  throwTypeError(QLatin1String("version")); return QUuid::Version(); }
bool lessThan(QUuid const& other) const
{ if (QUuid *_q_q = this->self()) return _q_q->operator<(other);
  throwTypeError(QLatin1String("lessThan")); return bool(); }
QString toString() const
{ if (QUuid *_q_q = this->self()) return _q_q->toString();
  throwTypeError(QLatin1String("toString")); return QString(); }
bool equals(QUuid const& orig) const
{ if (QUuid *_q_q = this->self()) return _q_q->operator==(orig);
  throwTypeError(QLatin1String("equals")); return bool(); }
bool isNull() const
{ if (QUuid *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
QUuid::Variant variant() const
{ if (QUuid *_q_q = this->self()) return _q_q->variant();
  throwTypeError(QLatin1String("variant")); return QUuid::Variant(); }
};

static QScriptValue create_QUuid_class(QScriptEngine *engine)
{
    Prototype_QUuid *pq = new Prototype_QUuid;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QUuid *cq = new Constructor_QUuid;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QUuid>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QUuid>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QUuid*>(), pv);
    _q_ScriptRegisterEnumMetaType<QUuid::Variant>(engine);
    cv.setProperty("VarUnknown", QScriptValue(engine, static_cast<int>(QUuid::VarUnknown)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NCS", QScriptValue(engine, static_cast<int>(QUuid::NCS)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DCE", QScriptValue(engine, static_cast<int>(QUuid::DCE)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Microsoft", QScriptValue(engine, static_cast<int>(QUuid::Microsoft)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Reserved", QScriptValue(engine, static_cast<int>(QUuid::Reserved)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QUuid::Version>(engine);
    cv.setProperty("VerUnknown", QScriptValue(engine, static_cast<int>(QUuid::VerUnknown)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Time", QScriptValue(engine, static_cast<int>(QUuid::Time)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("EmbeddedPOSIX", QScriptValue(engine, static_cast<int>(QUuid::EmbeddedPOSIX)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Name", QScriptValue(engine, static_cast<int>(QUuid::Name)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Random", QScriptValue(engine, static_cast<int>(QUuid::Random)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QMetaClassInfo

Q_DECLARE_METATYPE(QMetaClassInfo)
Q_DECLARE_METATYPE(QMetaClassInfo*)

class Constructor_QMetaClassInfo:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QMetaClassInfo()); }
};

class Prototype_QMetaClassInfo:
public QObject, public QMetaClassInfo, protected QScriptable
{
    Q_OBJECT
private:
inline QMetaClassInfo *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QMetaClassInfo*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QMetaClassInfo.prototype.%0: this object is not a QMetaClassInfo")
            .arg(method));
}
public:
Prototype_QMetaClassInfo(QObject *parent = 0)
    : QMetaClassInfo() { setParent(parent); }
public Q_SLOTS:
char const* value() const
{ if (QMetaClassInfo *_q_q = this->self()) return _q_q->value();
  throwTypeError(QLatin1String("value")); return 0; }
char const* name() const
{ if (QMetaClassInfo *_q_q = this->self()) return _q_q->name();
  throwTypeError(QLatin1String("name")); return 0; }
QString toString() const
{ return QLatin1String("QMetaClassInfo"); }
};

static QScriptValue create_QMetaClassInfo_class(QScriptEngine *engine)
{
    Prototype_QMetaClassInfo *pq = new Prototype_QMetaClassInfo;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QMetaClassInfo *cq = new Constructor_QMetaClassInfo;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QMetaClassInfo>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QMetaClassInfo>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QMetaClassInfo*>(), pv);
    return cv;
}

// QFileInfo

Q_DECLARE_METATYPE(QFileInfo)
Q_DECLARE_METATYPE(QFileInfo*)

class Constructor_QFileInfo:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QFileInfo const& fileinfo)
{ return this->engine()->toScriptValue(QFileInfo(fileinfo)); }
QScriptValue qscript_call(QDir const& dir, QString const& file)
{ return this->engine()->toScriptValue(QFileInfo(dir, file)); }
QScriptValue qscript_call(QFile const& file)
{ return this->engine()->toScriptValue(QFileInfo(file)); }
QScriptValue qscript_call(QString const& file)
{ return this->engine()->toScriptValue(QFileInfo(file)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QFileInfo()); }
};

class Prototype_QFileInfo:
public QObject, public QFileInfo, protected QScriptable
{
    Q_OBJECT
private:
inline QFileInfo *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QFileInfo*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QFileInfo.prototype.%0: this object is not a QFileInfo")
            .arg(method));
}
public:
Prototype_QFileInfo(QObject *parent = 0)
    : QFileInfo() { setParent(parent); }
public Q_SLOTS:
bool equals(QFileInfo const& fileinfo) const
{ if (QFileInfo *_q_q = this->self()) return _q_q->operator==(fileinfo);
  throwTypeError(QLatin1String("equals")); return bool(); }
bool equals(QFileInfo const& fileinfo)
{ if (QFileInfo *_q_q = this->self()) return _q_q->operator==(fileinfo);
  throwTypeError(QLatin1String("equals")); return bool(); }
bool exists() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->exists();
  throwTypeError(QLatin1String("exists")); return bool(); }
QFile::Permissions permissions() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->permissions();
  throwTypeError(QLatin1String("permissions")); return QFile::Permissions(); }
uint ownerId() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->ownerId();
  throwTypeError(QLatin1String("ownerId")); return uint(); }
QString readLink() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->readLink();
  throwTypeError(QLatin1String("readLink")); return QString(); }
bool permission(QFile::Permissions permissions) const
{ if (QFileInfo *_q_q = this->self()) return _q_q->permission(permissions);
  throwTypeError(QLatin1String("permission")); return bool(); }
bool isWritable() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->isWritable();
  throwTypeError(QLatin1String("isWritable")); return bool(); }
bool isRelative() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->isRelative();
  throwTypeError(QLatin1String("isRelative")); return bool(); }
bool isSymLink() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->isSymLink();
  throwTypeError(QLatin1String("isSymLink")); return bool(); }
bool isRoot() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->isRoot();
  throwTypeError(QLatin1String("isRoot")); return bool(); }
qint64 size() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->size();
  throwTypeError(QLatin1String("size")); return qint64(); }
void detach()
{ if (QFileInfo *_q_q = this->self()) _q_q->detach();
  else throwTypeError(QLatin1String("detach")); }
QString filePath() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->filePath();
  throwTypeError(QLatin1String("filePath")); return QString(); }
QString completeSuffix() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->completeSuffix();
  throwTypeError(QLatin1String("completeSuffix")); return QString(); }
QString suffix() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->suffix();
  throwTypeError(QLatin1String("suffix")); return QString(); }
void setFile(QDir const& dir, QString const& file)
{ if (QFileInfo *_q_q = this->self()) _q_q->setFile(dir, file);
  else throwTypeError(QLatin1String("setFile")); }
void setFile(QFile const& file)
{ if (QFileInfo *_q_q = this->self()) _q_q->setFile(file);
  else throwTypeError(QLatin1String("setFile")); }
void setFile(QString const& file)
{ if (QFileInfo *_q_q = this->self()) _q_q->setFile(file);
  else throwTypeError(QLatin1String("setFile")); }
QString group() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->group();
  throwTypeError(QLatin1String("group")); return QString(); }
bool isExecutable() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->isExecutable();
  throwTypeError(QLatin1String("isExecutable")); return bool(); }
bool makeAbsolute()
{ if (QFileInfo *_q_q = this->self()) return _q_q->makeAbsolute();
  throwTypeError(QLatin1String("makeAbsolute")); return bool(); }
bool isReadable() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->isReadable();
  throwTypeError(QLatin1String("isReadable")); return bool(); }
void refresh()
{ if (QFileInfo *_q_q = this->self()) _q_q->refresh();
  else throwTypeError(QLatin1String("refresh")); }
QString completeBaseName() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->completeBaseName();
  throwTypeError(QLatin1String("completeBaseName")); return QString(); }
bool isHidden() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->isHidden();
  throwTypeError(QLatin1String("isHidden")); return bool(); }
bool isAbsolute() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->isAbsolute();
  throwTypeError(QLatin1String("isAbsolute")); return bool(); }
QString symLinkTarget() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->symLinkTarget();
  throwTypeError(QLatin1String("symLinkTarget")); return QString(); }
QString canonicalPath() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->canonicalPath();
  throwTypeError(QLatin1String("canonicalPath")); return QString(); }
QString baseName() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->baseName();
  throwTypeError(QLatin1String("baseName")); return QString(); }
QString owner() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->owner();
  throwTypeError(QLatin1String("owner")); return QString(); }
void setCaching(bool on)
{ if (QFileInfo *_q_q = this->self()) _q_q->setCaching(on);
  else throwTypeError(QLatin1String("setCaching")); }
QString fileName() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->fileName();
  throwTypeError(QLatin1String("fileName")); return QString(); }
bool isDir() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->isDir();
  throwTypeError(QLatin1String("isDir")); return bool(); }
QString absoluteFilePath() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->absoluteFilePath();
  throwTypeError(QLatin1String("absoluteFilePath")); return QString(); }
bool isFile() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->isFile();
  throwTypeError(QLatin1String("isFile")); return bool(); }
QString absolutePath() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->absolutePath();
  throwTypeError(QLatin1String("absolutePath")); return QString(); }
QDateTime created() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->created();
  throwTypeError(QLatin1String("created")); return QDateTime(); }
QDateTime lastRead() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->lastRead();
  throwTypeError(QLatin1String("lastRead")); return QDateTime(); }
QDir dir() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->dir();
  throwTypeError(QLatin1String("dir")); return QDir(); }
QDateTime lastModified() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->lastModified();
  throwTypeError(QLatin1String("lastModified")); return QDateTime(); }
QString bundleName() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->bundleName();
  throwTypeError(QLatin1String("bundleName")); return QString(); }
QDir absoluteDir() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->absoluteDir();
  throwTypeError(QLatin1String("absoluteDir")); return QDir(); }
QString canonicalFilePath() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->canonicalFilePath();
  throwTypeError(QLatin1String("canonicalFilePath")); return QString(); }
QString path() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->path();
  throwTypeError(QLatin1String("path")); return QString(); }
bool isBundle() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->isBundle();
  throwTypeError(QLatin1String("isBundle")); return bool(); }
bool caching() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->caching();
  throwTypeError(QLatin1String("caching")); return bool(); }
uint groupId() const
{ if (QFileInfo *_q_q = this->self()) return _q_q->groupId();
  throwTypeError(QLatin1String("groupId")); return uint(); }
QString toString() const
{ return QLatin1String("QFileInfo"); }
};

static QScriptValue create_QFileInfo_class(QScriptEngine *engine)
{
    Prototype_QFileInfo *pq = new Prototype_QFileInfo;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QFileInfo *cq = new Constructor_QFileInfo;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QFileInfo>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QFileInfo>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QFileInfo*>(), pv);
    return cv;
}

// QLineF

Q_DECLARE_METATYPE(QLineF)
Q_DECLARE_METATYPE(QLineF*)
Q_DECLARE_METATYPE(QLineF::IntersectType)

class Constructor_QLineF:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QLine const& line)
{ return this->engine()->toScriptValue(QLineF(line)); }
QScriptValue qscript_call(qreal x1, qreal y1, qreal x2, qreal y2)
{ return this->engine()->toScriptValue(QLineF(x1, y1, x2, y2)); }
QScriptValue qscript_call(QPointF const& pt1, QPointF const& pt2)
{ return this->engine()->toScriptValue(QLineF(pt1, pt2)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QLineF()); }
};

class Prototype_QLineF:
public QObject, public QLineF, protected QScriptable
{
    Q_OBJECT
private:
inline QLineF *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QLineF*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QLineF.prototype.%0: this object is not a QLineF")
            .arg(method));
}
public:
Prototype_QLineF(QObject *parent = 0)
    : QLineF() { setParent(parent); }
public Q_SLOTS:
QLineF unitVector() const
{ if (QLineF *_q_q = this->self()) return _q_q->unitVector();
  throwTypeError(QLatin1String("unitVector")); return QLineF(); }
QLineF normalVector() const
{ if (QLineF *_q_q = this->self()) return _q_q->normalVector();
  throwTypeError(QLatin1String("normalVector")); return QLineF(); }
qreal x1() const
{ if (QLineF *_q_q = this->self()) return _q_q->x1();
  throwTypeError(QLatin1String("x1")); return qreal(); }
qreal x2() const
{ if (QLineF *_q_q = this->self()) return _q_q->x2();
  throwTypeError(QLatin1String("x2")); return qreal(); }
void translate(qreal dx, qreal dy)
{ if (QLineF *_q_q = this->self()) _q_q->translate(dx, dy);
  else throwTypeError(QLatin1String("translate")); }
void translate(QPointF const& p)
{ if (QLineF *_q_q = this->self()) _q_q->translate(p);
  else throwTypeError(QLatin1String("translate")); }
bool equals(QLineF const& d) const
{ if (QLineF *_q_q = this->self()) return _q_q->operator==(d);
  throwTypeError(QLatin1String("equals")); return bool(); }
qreal dx() const
{ if (QLineF *_q_q = this->self()) return _q_q->dx();
  throwTypeError(QLatin1String("dx")); return qreal(); }
qreal dy() const
{ if (QLineF *_q_q = this->self()) return _q_q->dy();
  throwTypeError(QLatin1String("dy")); return qreal(); }
bool isNull() const
{ if (QLineF *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
qreal y1() const
{ if (QLineF *_q_q = this->self()) return _q_q->y1();
  throwTypeError(QLatin1String("y1")); return qreal(); }
QLine toLine() const
{ if (QLineF *_q_q = this->self()) return _q_q->toLine();
  throwTypeError(QLatin1String("toLine")); return QLine(); }
qreal y2() const
{ if (QLineF *_q_q = this->self()) return _q_q->y2();
  throwTypeError(QLatin1String("y2")); return qreal(); }
QLineF::IntersectType intersect(QLineF const& l, QPointF* intersectionPoint) const
{ if (QLineF *_q_q = this->self()) return _q_q->intersect(l, intersectionPoint);
  throwTypeError(QLatin1String("intersect")); return QLineF::IntersectType(); }
QPointF p1() const
{ if (QLineF *_q_q = this->self()) return _q_q->p1();
  throwTypeError(QLatin1String("p1")); return QPointF(); }
QPointF p2() const
{ if (QLineF *_q_q = this->self()) return _q_q->p2();
  throwTypeError(QLatin1String("p2")); return QPointF(); }
qreal length() const
{ if (QLineF *_q_q = this->self()) return _q_q->length();
  throwTypeError(QLatin1String("length")); return qreal(); }
QPointF pointAt(qreal t) const
{ if (QLineF *_q_q = this->self()) return _q_q->pointAt(t);
  throwTypeError(QLatin1String("pointAt")); return QPointF(); }
void setLength(qreal len)
{ if (QLineF *_q_q = this->self()) _q_q->setLength(len);
  else throwTypeError(QLatin1String("setLength")); }
qreal angle(QLineF const& l) const
{ if (QLineF *_q_q = this->self()) return _q_q->angle(l);
  throwTypeError(QLatin1String("angle")); return qreal(); }
QString toString() const
{ return QLatin1String("QLineF"); }
};

static QScriptValue create_QLineF_class(QScriptEngine *engine)
{
    Prototype_QLineF *pq = new Prototype_QLineF;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QLineF *cq = new Constructor_QLineF;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QLineF>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QLineF>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QLineF*>(), pv);
    _q_ScriptRegisterEnumMetaType<QLineF::IntersectType>(engine);
    cv.setProperty("NoIntersection", QScriptValue(engine, static_cast<int>(QLineF::NoIntersection)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("BoundedIntersection", QScriptValue(engine, static_cast<int>(QLineF::BoundedIntersection)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("UnboundedIntersection", QScriptValue(engine, static_cast<int>(QLineF::UnboundedIntersection)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QMetaProperty

Q_DECLARE_METATYPE(QMetaProperty)
Q_DECLARE_METATYPE(QMetaProperty*)

class Constructor_QMetaProperty:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QMetaProperty()); }
};

class Prototype_QMetaProperty:
public QObject, public QMetaProperty, protected QScriptable
{
    Q_OBJECT
private:
inline QMetaProperty *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QMetaProperty*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QMetaProperty.prototype.%0: this object is not a QMetaProperty")
            .arg(method));
}
public:
Prototype_QMetaProperty(QObject *parent = 0)
    : QMetaProperty() { setParent(parent); }
public Q_SLOTS:
bool isValid() const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
bool isWritable() const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->isWritable();
  throwTypeError(QLatin1String("isWritable")); return bool(); }
bool reset(QObject* obj) const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->reset(obj);
  throwTypeError(QLatin1String("reset")); return bool(); }
int userType() const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->userType();
  throwTypeError(QLatin1String("userType")); return int(); }
bool isEditable(QObject const* obj = 0) const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->isEditable(obj);
  throwTypeError(QLatin1String("isEditable")); return bool(); }
char const* name() const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->name();
  throwTypeError(QLatin1String("name")); return 0; }
bool isStored(QObject const* obj = 0) const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->isStored(obj);
  throwTypeError(QLatin1String("isStored")); return bool(); }
bool isEnumType() const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->isEnumType();
  throwTypeError(QLatin1String("isEnumType")); return bool(); }
bool write(QObject* obj, QVariant const& value) const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->write(obj, value);
  throwTypeError(QLatin1String("write")); return bool(); }
bool hasStdCppSet() const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->hasStdCppSet();
  throwTypeError(QLatin1String("hasStdCppSet")); return bool(); }
bool isResettable() const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->isResettable();
  throwTypeError(QLatin1String("isResettable")); return bool(); }
QVariant read(QObject const* obj) const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->read(obj);
  throwTypeError(QLatin1String("read")); return QVariant(); }
char const* typeName() const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->typeName();
  throwTypeError(QLatin1String("typeName")); return 0; }
bool isReadable() const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->isReadable();
  throwTypeError(QLatin1String("isReadable")); return bool(); }
bool isUser(QObject const* obj = 0) const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->isUser(obj);
  throwTypeError(QLatin1String("isUser")); return bool(); }
bool isScriptable(QObject const* obj = 0) const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->isScriptable(obj);
  throwTypeError(QLatin1String("isScriptable")); return bool(); }
bool isFlagType() const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->isFlagType();
  throwTypeError(QLatin1String("isFlagType")); return bool(); }
QMetaEnum enumerator() const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->enumerator();
  throwTypeError(QLatin1String("enumerator")); return QMetaEnum(); }
QVariant::Type type() const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->type();
  throwTypeError(QLatin1String("type")); return QVariant::Type(); }
bool isDesignable(QObject const* obj = 0) const
{ if (QMetaProperty *_q_q = this->self()) return _q_q->isDesignable(obj);
  throwTypeError(QLatin1String("isDesignable")); return bool(); }
QString toString() const
{ return QLatin1String("QMetaProperty"); }
};

static QScriptValue create_QMetaProperty_class(QScriptEngine *engine)
{
    Prototype_QMetaProperty *pq = new Prototype_QMetaProperty;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QMetaProperty *cq = new Constructor_QMetaProperty;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QMetaProperty>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QMetaProperty>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QMetaProperty*>(), pv);
    return cv;
}

// QMetaEnum

Q_DECLARE_METATYPE(QMetaEnum)
Q_DECLARE_METATYPE(QMetaEnum*)

class Constructor_QMetaEnum:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QMetaEnum()); }
};

class Prototype_QMetaEnum:
public QObject, public QMetaEnum, protected QScriptable
{
    Q_OBJECT
private:
inline QMetaEnum *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QMetaEnum*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QMetaEnum.prototype.%0: this object is not a QMetaEnum")
            .arg(method));
}
public:
Prototype_QMetaEnum(QObject *parent = 0)
    : QMetaEnum() { setParent(parent); }
public Q_SLOTS:
bool isValid() const
{ if (QMetaEnum *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
bool isFlag() const
{ if (QMetaEnum *_q_q = this->self()) return _q_q->isFlag();
  throwTypeError(QLatin1String("isFlag")); return bool(); }
int keyCount() const
{ if (QMetaEnum *_q_q = this->self()) return _q_q->keyCount();
  throwTypeError(QLatin1String("keyCount")); return int(); }
char const* key(int index) const
{ if (QMetaEnum *_q_q = this->self()) return _q_q->key(index);
  throwTypeError(QLatin1String("key")); return 0; }
QByteArray valueToKeys(int value) const
{ if (QMetaEnum *_q_q = this->self()) return _q_q->valueToKeys(value);
  throwTypeError(QLatin1String("valueToKeys")); return QByteArray(); }
char const* valueToKey(int value) const
{ if (QMetaEnum *_q_q = this->self()) return _q_q->valueToKey(value);
  throwTypeError(QLatin1String("valueToKey")); return 0; }
int keyToValue(char const* key) const
{ if (QMetaEnum *_q_q = this->self()) return _q_q->keyToValue(key);
  throwTypeError(QLatin1String("keyToValue")); return int(); }
int value(int index) const
{ if (QMetaEnum *_q_q = this->self()) return _q_q->value(index);
  throwTypeError(QLatin1String("value")); return int(); }
char const* name() const
{ if (QMetaEnum *_q_q = this->self()) return _q_q->name();
  throwTypeError(QLatin1String("name")); return 0; }
char const* scope() const
{ if (QMetaEnum *_q_q = this->self()) return _q_q->scope();
  throwTypeError(QLatin1String("scope")); return 0; }
int keysToValue(char const* keys) const
{ if (QMetaEnum *_q_q = this->self()) return _q_q->keysToValue(keys);
  throwTypeError(QLatin1String("keysToValue")); return int(); }
QString toString() const
{ return QLatin1String("QMetaEnum"); }
};

static QScriptValue create_QMetaEnum_class(QScriptEngine *engine)
{
    Prototype_QMetaEnum *pq = new Prototype_QMetaEnum;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QMetaEnum *cq = new Constructor_QMetaEnum;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QMetaEnum>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QMetaEnum>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QMetaEnum*>(), pv);
    return cv;
}

// QStringMatcher

Q_DECLARE_METATYPE(QStringMatcher)
Q_DECLARE_METATYPE(QStringMatcher*)

class Constructor_QStringMatcher:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QStringMatcher const& other)
{ return this->engine()->toScriptValue(QStringMatcher(other)); }
QScriptValue qscript_call(QString const& pattern, Qt::CaseSensitivity cs = Qt::CaseSensitive)
{ return this->engine()->toScriptValue(QStringMatcher(pattern, cs)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QStringMatcher()); }
};

class Prototype_QStringMatcher:
public QObject, public QStringMatcher, protected QScriptable
{
    Q_OBJECT
private:
inline QStringMatcher *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QStringMatcher*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QStringMatcher.prototype.%0: this object is not a QStringMatcher")
            .arg(method));
}
public:
Prototype_QStringMatcher(QObject *parent = 0)
    : QStringMatcher() { setParent(parent); }
public Q_SLOTS:
void setPattern(QString const& pattern)
{ if (QStringMatcher *_q_q = this->self()) _q_q->setPattern(pattern);
  else throwTypeError(QLatin1String("setPattern")); }
int indexIn(QString const& str, int from = 0) const
{ if (QStringMatcher *_q_q = this->self()) return _q_q->indexIn(str, from);
  throwTypeError(QLatin1String("indexIn")); return int(); }
QString pattern() const
{ if (QStringMatcher *_q_q = this->self()) return _q_q->pattern();
  throwTypeError(QLatin1String("pattern")); return QString(); }
Qt::CaseSensitivity caseSensitivity() const
{ if (QStringMatcher *_q_q = this->self()) return _q_q->caseSensitivity();
  throwTypeError(QLatin1String("caseSensitivity")); return Qt::CaseSensitivity(); }
void setCaseSensitivity(Qt::CaseSensitivity cs)
{ if (QStringMatcher *_q_q = this->self()) _q_q->setCaseSensitivity(cs);
  else throwTypeError(QLatin1String("setCaseSensitivity")); }
QString toString() const
{ return QLatin1String("QStringMatcher"); }
};

static QScriptValue create_QStringMatcher_class(QScriptEngine *engine)
{
    Prototype_QStringMatcher *pq = new Prototype_QStringMatcher;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QStringMatcher *cq = new Constructor_QStringMatcher;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QStringMatcher>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QStringMatcher>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QStringMatcher*>(), pv);
    return cv;
}

// QFactoryInterface

Q_DECLARE_METATYPE(QFactoryInterface*)

class Constructor_QFactoryInterface:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
};

class Prototype_QFactoryInterface:
public QObject, public QFactoryInterface, protected QScriptable
{
    Q_OBJECT
private:
inline QFactoryInterface *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QFactoryInterface*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QFactoryInterface.prototype.%0: this object is not a QFactoryInterface")
            .arg(method));
}
public:
Prototype_QFactoryInterface(QObject *parent = 0)
    : QFactoryInterface() { setParent(parent); }
public Q_SLOTS:
QStringList keys() const
{ if (QFactoryInterface *_q_q = this->self()) return _q_q->keys();
  throwTypeError(QLatin1String("keys")); return QStringList(); }
QString toString() const
{ return QLatin1String("QFactoryInterface"); }
public:
private:
Q_DISABLE_COPY(Prototype_QFactoryInterface)
};

static QScriptValue create_QFactoryInterface_class(QScriptEngine *engine)
{
    Prototype_QFactoryInterface *pq = new Prototype_QFactoryInterface;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QFactoryInterface *cq = new Constructor_QFactoryInterface;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QFactoryInterface*>(), pv);
    return cv;
}

// QStringRef

Q_DECLARE_METATYPE(QStringRef)
Q_DECLARE_METATYPE(QStringRef*)

class Constructor_QStringRef:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QStringRef const& other)
{ return this->engine()->toScriptValue(QStringRef(other)); }
QScriptValue qscript_call(QString const* string)
{ return this->engine()->toScriptValue(QStringRef(string)); }
QScriptValue qscript_call(QString const* string, int position, int size)
{ return this->engine()->toScriptValue(QStringRef(string, position, size)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QStringRef()); }
};

class Prototype_QStringRef:
public QObject, public QStringRef, protected QScriptable
{
    Q_OBJECT
private:
inline QStringRef *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QStringRef*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QStringRef.prototype.%0: this object is not a QStringRef")
            .arg(method));
}
public:
Prototype_QStringRef(QObject *parent = 0)
    : QStringRef() { setParent(parent); }
public Q_SLOTS:
QChar const at(int i) const
{ if (QStringRef *_q_q = this->self()) return _q_q->at(i);
  throwTypeError(QLatin1String("at")); return QChar(); }
QString toString() const
{ if (QStringRef *_q_q = this->self()) return _q_q->toString();
  throwTypeError(QLatin1String("toString")); return QString(); }
QChar const* data() const
{ if (QStringRef *_q_q = this->self()) return _q_q->data();
  throwTypeError(QLatin1String("data")); return 0; }
int count() const
{ if (QStringRef *_q_q = this->self()) return _q_q->count();
  throwTypeError(QLatin1String("count")); return int(); }
QStringRef appendTo(QString* string) const
{ if (QStringRef *_q_q = this->self()) return _q_q->appendTo(string);
  throwTypeError(QLatin1String("appendTo")); return QStringRef(); }
int position() const
{ if (QStringRef *_q_q = this->self()) return _q_q->position();
  throwTypeError(QLatin1String("position")); return int(); }
bool isNull() const
{ if (QStringRef *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
QString const* string() const
{ if (QStringRef *_q_q = this->self()) return _q_q->string();
  throwTypeError(QLatin1String("string")); return 0; }
int size() const
{ if (QStringRef *_q_q = this->self()) return _q_q->size();
  throwTypeError(QLatin1String("size")); return int(); }
QChar const* constData() const
{ if (QStringRef *_q_q = this->self()) return _q_q->constData();
  throwTypeError(QLatin1String("constData")); return 0; }
void clear()
{ if (QStringRef *_q_q = this->self()) _q_q->clear();
  else throwTypeError(QLatin1String("clear")); }
int length() const
{ if (QStringRef *_q_q = this->self()) return _q_q->length();
  throwTypeError(QLatin1String("length")); return int(); }
bool isEmpty() const
{ if (QStringRef *_q_q = this->self()) return _q_q->isEmpty();
  throwTypeError(QLatin1String("isEmpty")); return bool(); }
QChar const* unicode() const
{ if (QStringRef *_q_q = this->self()) return _q_q->unicode();
  throwTypeError(QLatin1String("unicode")); return 0; }
};

static QScriptValue create_QStringRef_class(QScriptEngine *engine)
{
    Prototype_QStringRef *pq = new Prototype_QStringRef;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QStringRef *cq = new Constructor_QStringRef;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QStringRef>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QStringRef>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QStringRef*>(), pv);
    return cv;
}

// QResource

Q_DECLARE_METATYPE(QResource)
Q_DECLARE_METATYPE(QResource*)

class Constructor_QResource:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QString const& file = QString(), QLocale const& locale = QLocale())
{ return this->engine()->toScriptValue(QResource(file, locale)); }
bool unregisterResource(uchar const* rccData, QString const& resourceRoot = QString())
{ return QResource::unregisterResource(rccData, resourceRoot); }
bool unregisterResource(QString const& rccFilename, QString const& resourceRoot = QString())
{ return QResource::unregisterResource(rccFilename, resourceRoot); }
bool registerResource(uchar const* rccData, QString const& resourceRoot = QString())
{ return QResource::registerResource(rccData, resourceRoot); }
bool registerResource(QString const& rccFilename, QString const& resourceRoot = QString())
{ return QResource::registerResource(rccFilename, resourceRoot); }
void addSearchPath(QString const& path)
{ QResource::addSearchPath(path); }
QStringList searchPaths()
{ return QResource::searchPaths(); }
};

class Prototype_QResource:
public QObject, public QResource, protected QScriptable
{
    Q_OBJECT
private:
inline QResource *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QResource*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QResource.prototype.%0: this object is not a QResource")
            .arg(method));
}
public:
Prototype_QResource(QObject *parent = 0)
    : QResource() { setParent(parent); }
public Q_SLOTS:
QString fileName() const
{ if (QResource *_q_q = this->self()) return _q_q->fileName();
  throwTypeError(QLatin1String("fileName")); return QString(); }
bool isValid() const
{ if (QResource *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
bool isCompressed() const
{ if (QResource *_q_q = this->self()) return _q_q->isCompressed();
  throwTypeError(QLatin1String("isCompressed")); return bool(); }
void setFileName(QString const& file)
{ if (QResource *_q_q = this->self()) _q_q->setFileName(file);
  else throwTypeError(QLatin1String("setFileName")); }
uchar const* data() const
{ if (QResource *_q_q = this->self()) return _q_q->data();
  throwTypeError(QLatin1String("data")); return 0; }
void setLocale(QLocale const& locale)
{ if (QResource *_q_q = this->self()) _q_q->setLocale(locale);
  else throwTypeError(QLatin1String("setLocale")); }
QLocale locale() const
{ if (QResource *_q_q = this->self()) return _q_q->locale();
  throwTypeError(QLatin1String("locale")); return QLocale(); }
QString absoluteFilePath() const
{ if (QResource *_q_q = this->self()) return _q_q->absoluteFilePath();
  throwTypeError(QLatin1String("absoluteFilePath")); return QString(); }
qint64 size() const
{ if (QResource *_q_q = this->self()) return _q_q->size();
  throwTypeError(QLatin1String("size")); return qint64(); }
QString toString() const
{ return QLatin1String("QResource"); }
};

static QScriptValue create_QResource_class(QScriptEngine *engine)
{
    Prototype_QResource *pq = new Prototype_QResource;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QResource *cq = new Constructor_QResource;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QResource>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QResource>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QResource*>(), pv);
    return cv;
}

// QMutex

Q_DECLARE_METATYPE(QMutex*)
Q_DECLARE_METATYPE(QMutex::RecursionMode)

class Constructor_QMutex:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QMutex::RecursionMode mode = QMutex::NonRecursive)
{ return this->engine()->toScriptValue(new QMutex(mode)); }
};

class Prototype_QMutex:
public QObject, public QMutex, protected QScriptable
{
    Q_OBJECT
private:
inline QMutex *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QMutex*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QMutex.prototype.%0: this object is not a QMutex")
            .arg(method));
}
public:
Prototype_QMutex(QObject *parent = 0)
    : QMutex() { setParent(parent); }
public Q_SLOTS:
void unlock()
{ if (QMutex *_q_q = this->self()) _q_q->unlock();
  else throwTypeError(QLatin1String("unlock")); }
bool tryLock(int timeout)
{ if (QMutex *_q_q = this->self()) return _q_q->tryLock(timeout);
  throwTypeError(QLatin1String("tryLock")); return bool(); }
bool tryLock()
{ if (QMutex *_q_q = this->self()) return _q_q->tryLock();
  throwTypeError(QLatin1String("tryLock")); return bool(); }
void lock()
{ if (QMutex *_q_q = this->self()) _q_q->lock();
  else throwTypeError(QLatin1String("lock")); }
QString toString() const
{ return QLatin1String("QMutex"); }
private:
Q_DISABLE_COPY(Prototype_QMutex)
};

static QScriptValue create_QMutex_class(QScriptEngine *engine)
{
    Prototype_QMutex *pq = new Prototype_QMutex;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QMutex *cq = new Constructor_QMutex;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QMutex*>(), pv);
    _q_ScriptRegisterEnumMetaType<QMutex::RecursionMode>(engine);
    cv.setProperty("NonRecursive", QScriptValue(engine, static_cast<int>(QMutex::NonRecursive)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Recursive", QScriptValue(engine, static_cast<int>(QMutex::Recursive)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QUrl

Q_DECLARE_METATYPE(QUrl)
Q_DECLARE_METATYPE(QUrl*)
Q_DECLARE_METATYPE(QUrl::ParsingMode)
Q_DECLARE_METATYPE(QUrl::FormattingOption)
Q_DECLARE_METATYPE(QUrl::FormattingOptions)

class Constructor_QUrl:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QUrl const& copy)
{ return this->engine()->toScriptValue(QUrl(copy)); }
QScriptValue qscript_call(QString const& url, QUrl::ParsingMode mode)
{ return this->engine()->toScriptValue(QUrl(url, mode)); }
QScriptValue qscript_call(QString const& url)
{ return this->engine()->toScriptValue(QUrl(url)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QUrl()); }
QUrl fromLocalFile(QString const& localfile)
{ return QUrl::fromLocalFile(localfile); }
void setIdnWhitelist(QStringList const& arg1)
{ QUrl::setIdnWhitelist(arg1); }
QString fromPunycode(QByteArray const& arg1)
{ return QUrl::fromPunycode(arg1); }
QString fromAce(QByteArray const& arg1)
{ return QUrl::fromAce(arg1); }
QByteArray toPunycode(QString const& arg1)
{ return QUrl::toPunycode(arg1); }
QByteArray toAce(QString const& arg1)
{ return QUrl::toAce(arg1); }
QStringList idnWhitelist()
{ return QUrl::idnWhitelist(); }
QByteArray toPercentEncoding(QString const& arg1, QByteArray const& exclude = QByteArray(), QByteArray const& include = QByteArray())
{ return QUrl::toPercentEncoding(arg1, exclude, include); }
QUrl fromEncoded(QByteArray const& url, QUrl::ParsingMode mode)
{ return QUrl::fromEncoded(url, mode); }
QUrl fromEncoded(QByteArray const& url)
{ return QUrl::fromEncoded(url); }
QString fromPercentEncoding(QByteArray const& arg1)
{ return QUrl::fromPercentEncoding(arg1); }
};

class Prototype_QUrl:
public QObject, public QUrl, protected QScriptable
{
    Q_OBJECT
private:
inline QUrl *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QUrl*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QUrl.prototype.%0: this object is not a QUrl")
            .arg(method));
}
public:
Prototype_QUrl(QObject *parent = 0)
    : QUrl() { setParent(parent); }
public Q_SLOTS:
QString scheme() const
{ if (QUrl *_q_q = this->self()) return _q_q->scheme();
  throwTypeError(QLatin1String("scheme")); return QString(); }
QString errorString() const
{ if (QUrl *_q_q = this->self()) return _q_q->errorString();
  throwTypeError(QLatin1String("errorString")); return QString(); }
QByteArray encodedQuery() const
{ if (QUrl *_q_q = this->self()) return _q_q->encodedQuery();
  throwTypeError(QLatin1String("encodedQuery")); return QByteArray(); }
void clear()
{ if (QUrl *_q_q = this->self()) _q_q->clear();
  else throwTypeError(QLatin1String("clear")); }
void setAuthority(QString const& authority)
{ if (QUrl *_q_q = this->self()) _q_q->setAuthority(authority);
  else throwTypeError(QLatin1String("setAuthority")); }
void setPath(QString const& path)
{ if (QUrl *_q_q = this->self()) _q_q->setPath(path);
  else throwTypeError(QLatin1String("setPath")); }
QStringList allQueryItemValues(QString const& key) const
{ if (QUrl *_q_q = this->self()) return _q_q->allQueryItemValues(key);
  throwTypeError(QLatin1String("allQueryItemValues")); return QStringList(); }
void setUrl(QString const& url, QUrl::ParsingMode mode)
{ if (QUrl *_q_q = this->self()) _q_q->setUrl(url, mode);
  else throwTypeError(QLatin1String("setUrl")); }
void setUrl(QString const& url)
{ if (QUrl *_q_q = this->self()) _q_q->setUrl(url);
  else throwTypeError(QLatin1String("setUrl")); }
QByteArray toEncoded(QUrl::FormattingOptions options = QUrl::None) const
{ if (QUrl *_q_q = this->self()) return _q_q->toEncoded(options);
  throwTypeError(QLatin1String("toEncoded")); return QByteArray(); }
QString queryItemValue(QString const& key) const
{ if (QUrl *_q_q = this->self()) return _q_q->queryItemValue(key);
  throwTypeError(QLatin1String("queryItemValue")); return QString(); }
void removeAllQueryItems(QString const& key)
{ if (QUrl *_q_q = this->self()) _q_q->removeAllQueryItems(key);
  else throwTypeError(QLatin1String("removeAllQueryItems")); }
void setScheme(QString const& scheme)
{ if (QUrl *_q_q = this->self()) _q_q->setScheme(scheme);
  else throwTypeError(QLatin1String("setScheme")); }
char queryPairDelimiter() const
{ if (QUrl *_q_q = this->self()) return _q_q->queryPairDelimiter();
  throwTypeError(QLatin1String("queryPairDelimiter")); return char(); }
void setHost(QString const& host)
{ if (QUrl *_q_q = this->self()) _q_q->setHost(host);
  else throwTypeError(QLatin1String("setHost")); }
void addQueryItem(QString const& key, QString const& value)
{ if (QUrl *_q_q = this->self()) _q_q->addQueryItem(key, value);
  else throwTypeError(QLatin1String("addQueryItem")); }
void setEncodedUrl(QByteArray const& url, QUrl::ParsingMode mode)
{ if (QUrl *_q_q = this->self()) _q_q->setEncodedUrl(url, mode);
  else throwTypeError(QLatin1String("setEncodedUrl")); }
void setEncodedUrl(QByteArray const& url)
{ if (QUrl *_q_q = this->self()) _q_q->setEncodedUrl(url);
  else throwTypeError(QLatin1String("setEncodedUrl")); }
void setUserInfo(QString const& userInfo)
{ if (QUrl *_q_q = this->self()) _q_q->setUserInfo(userInfo);
  else throwTypeError(QLatin1String("setUserInfo")); }
QString password() const
{ if (QUrl *_q_q = this->self()) return _q_q->password();
  throwTypeError(QLatin1String("password")); return QString(); }
bool isParentOf(QUrl const& url) const
{ if (QUrl *_q_q = this->self()) return _q_q->isParentOf(url);
  throwTypeError(QLatin1String("isParentOf")); return bool(); }
QString path() const
{ if (QUrl *_q_q = this->self()) return _q_q->path();
  throwTypeError(QLatin1String("path")); return QString(); }
void setPassword(QString const& password)
{ if (QUrl *_q_q = this->self()) _q_q->setPassword(password);
  else throwTypeError(QLatin1String("setPassword")); }
void setEncodedQuery(QByteArray const& query)
{ if (QUrl *_q_q = this->self()) _q_q->setEncodedQuery(query);
  else throwTypeError(QLatin1String("setEncodedQuery")); }
bool isEmpty() const
{ if (QUrl *_q_q = this->self()) return _q_q->isEmpty();
  throwTypeError(QLatin1String("isEmpty")); return bool(); }
bool hasQueryItem(QString const& key) const
{ if (QUrl *_q_q = this->self()) return _q_q->hasQueryItem(key);
  throwTypeError(QLatin1String("hasQueryItem")); return bool(); }
QString authority() const
{ if (QUrl *_q_q = this->self()) return _q_q->authority();
  throwTypeError(QLatin1String("authority")); return QString(); }
QList<QPair<QString,QString> > queryItems() const
{ if (QUrl *_q_q = this->self()) return _q_q->queryItems();
  throwTypeError(QLatin1String("queryItems")); return QList<QPair<QString,QString> >(); }
void setFragment(QString const& fragment)
{ if (QUrl *_q_q = this->self()) _q_q->setFragment(fragment);
  else throwTypeError(QLatin1String("setFragment")); }
bool equals(QUrl const& url) const
{ if (QUrl *_q_q = this->self()) return _q_q->operator==(url);
  throwTypeError(QLatin1String("equals")); return bool(); }
QString host() const
{ if (QUrl *_q_q = this->self()) return _q_q->host();
  throwTypeError(QLatin1String("host")); return QString(); }
int port(int defaultPort) const
{ if (QUrl *_q_q = this->self()) return _q_q->port(defaultPort);
  throwTypeError(QLatin1String("port")); return int(); }
int port() const
{ if (QUrl *_q_q = this->self()) return _q_q->port();
  throwTypeError(QLatin1String("port")); return int(); }
bool lessThan(QUrl const& url) const
{ if (QUrl *_q_q = this->self()) return _q_q->operator<(url);
  throwTypeError(QLatin1String("lessThan")); return bool(); }
char queryValueDelimiter() const
{ if (QUrl *_q_q = this->self()) return _q_q->queryValueDelimiter();
  throwTypeError(QLatin1String("queryValueDelimiter")); return char(); }
bool isValid() const
{ if (QUrl *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
void setQueryItems(QList<QPair<QString,QString> > const& query)
{ if (QUrl *_q_q = this->self()) _q_q->setQueryItems(query);
  else throwTypeError(QLatin1String("setQueryItems")); }
QString fragment() const
{ if (QUrl *_q_q = this->self()) return _q_q->fragment();
  throwTypeError(QLatin1String("fragment")); return QString(); }
QString toString(QUrl::FormattingOptions options = QUrl::None) const
{ if (QUrl *_q_q = this->self()) return _q_q->toString(options);
  throwTypeError(QLatin1String("toString")); return QString(); }
QUrl::DataPtr* data_ptr()
{ if (QUrl *_q_q = this->self()) return &_q_q->data_ptr();
  throwTypeError(QLatin1String("data_ptr")); return 0; }
bool hasQuery() const
{ if (QUrl *_q_q = this->self()) return _q_q->hasQuery();
  throwTypeError(QLatin1String("hasQuery")); return bool(); }
void setQueryDelimiters(char valueDelimiter, char pairDelimiter)
{ if (QUrl *_q_q = this->self()) _q_q->setQueryDelimiters(valueDelimiter, pairDelimiter);
  else throwTypeError(QLatin1String("setQueryDelimiters")); }
bool isRelative() const
{ if (QUrl *_q_q = this->self()) return _q_q->isRelative();
  throwTypeError(QLatin1String("isRelative")); return bool(); }
QString userName() const
{ if (QUrl *_q_q = this->self()) return _q_q->userName();
  throwTypeError(QLatin1String("userName")); return QString(); }
QString toLocalFile() const
{ if (QUrl *_q_q = this->self()) return _q_q->toLocalFile();
  throwTypeError(QLatin1String("toLocalFile")); return QString(); }
QUrl resolved(QUrl const& relative) const
{ if (QUrl *_q_q = this->self()) return _q_q->resolved(relative);
  throwTypeError(QLatin1String("resolved")); return QUrl(); }
bool isDetached() const
{ if (QUrl *_q_q = this->self()) return _q_q->isDetached();
  throwTypeError(QLatin1String("isDetached")); return bool(); }
void setPort(int port)
{ if (QUrl *_q_q = this->self()) _q_q->setPort(port);
  else throwTypeError(QLatin1String("setPort")); }
void detach()
{ if (QUrl *_q_q = this->self()) _q_q->detach();
  else throwTypeError(QLatin1String("detach")); }
void removeQueryItem(QString const& key)
{ if (QUrl *_q_q = this->self()) _q_q->removeQueryItem(key);
  else throwTypeError(QLatin1String("removeQueryItem")); }
QString userInfo() const
{ if (QUrl *_q_q = this->self()) return _q_q->userInfo();
  throwTypeError(QLatin1String("userInfo")); return QString(); }
bool hasFragment() const
{ if (QUrl *_q_q = this->self()) return _q_q->hasFragment();
  throwTypeError(QLatin1String("hasFragment")); return bool(); }
void setUserName(QString const& userName)
{ if (QUrl *_q_q = this->self()) _q_q->setUserName(userName);
  else throwTypeError(QLatin1String("setUserName")); }
};

static QScriptValue create_QUrl_class(QScriptEngine *engine)
{
    Prototype_QUrl *pq = new Prototype_QUrl;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QUrl *cq = new Constructor_QUrl;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QUrl>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QUrl>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QUrl*>(), pv);
    _q_ScriptRegisterEnumMetaType<QUrl::ParsingMode>(engine);
    cv.setProperty("TolerantMode", QScriptValue(engine, static_cast<int>(QUrl::TolerantMode)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("StrictMode", QScriptValue(engine, static_cast<int>(QUrl::StrictMode)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QUrl::FormattingOption>(engine);
    cv.setProperty("None", QScriptValue(engine, static_cast<int>(QUrl::None)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RemoveScheme", QScriptValue(engine, static_cast<int>(QUrl::RemoveScheme)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RemovePassword", QScriptValue(engine, static_cast<int>(QUrl::RemovePassword)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RemoveUserInfo", QScriptValue(engine, static_cast<int>(QUrl::RemoveUserInfo)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RemovePort", QScriptValue(engine, static_cast<int>(QUrl::RemovePort)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RemoveAuthority", QScriptValue(engine, static_cast<int>(QUrl::RemoveAuthority)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RemovePath", QScriptValue(engine, static_cast<int>(QUrl::RemovePath)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RemoveQuery", QScriptValue(engine, static_cast<int>(QUrl::RemoveQuery)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RemoveFragment", QScriptValue(engine, static_cast<int>(QUrl::RemoveFragment)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("StripTrailingSlash", QScriptValue(engine, static_cast<int>(QUrl::StripTrailingSlash)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QUrl::FormattingOptions>(engine);
    return cv;
}

// QPointF

Q_DECLARE_METATYPE(QPointF)
Q_DECLARE_METATYPE(QPointF*)

class Constructor_QPointF:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(qreal xpos, qreal ypos)
{ return this->engine()->toScriptValue(QPointF(xpos, ypos)); }
QScriptValue qscript_call(QPoint const& p)
{ return this->engine()->toScriptValue(QPointF(p)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QPointF()); }
};

class Prototype_QPointF:
public QObject, public QPointF, protected QScriptable
{
    Q_OBJECT
private:
inline QPointF *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QPointF*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QPointF.prototype.%0: this object is not a QPointF")
            .arg(method));
}
public:
Prototype_QPointF(QObject *parent = 0)
    : QPointF() { setParent(parent); }
public Q_SLOTS:
void setX(qreal x)
{ if (QPointF *_q_q = this->self()) _q_q->setX(x);
  else throwTypeError(QLatin1String("setX")); }
void setY(qreal y)
{ if (QPointF *_q_q = this->self()) _q_q->setY(y);
  else throwTypeError(QLatin1String("setY")); }
qreal x() const
{ if (QPointF *_q_q = this->self()) return _q_q->x();
  throwTypeError(QLatin1String("x")); return qreal(); }
qreal y() const
{ if (QPointF *_q_q = this->self()) return _q_q->y();
  throwTypeError(QLatin1String("y")); return qreal(); }
QPoint toPoint() const
{ if (QPointF *_q_q = this->self()) return _q_q->toPoint();
  throwTypeError(QLatin1String("toPoint")); return QPoint(); }
qreal* rx()
{ if (QPointF *_q_q = this->self()) return &_q_q->rx();
  throwTypeError(QLatin1String("rx")); return 0; }
qreal* ry()
{ if (QPointF *_q_q = this->self()) return &_q_q->ry();
  throwTypeError(QLatin1String("ry")); return 0; }
bool isNull() const
{ if (QPointF *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
QString toString() const
{ return QLatin1String("QPointF"); }
};

static QScriptValue create_QPointF_class(QScriptEngine *engine)
{
    Prototype_QPointF *pq = new Prototype_QPointF;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QPointF *cq = new Constructor_QPointF;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QPointF>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QPointF>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QPointF*>(), pv);
    return cv;
}

// QObject

Q_DECLARE_METATYPE(QObject*)

class Constructor_QObject:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* parent = 0)
{ return this->engine()->newQObject(new QObject(parent), QScriptEngine::AutoOwnership); }
bool disconnect(QObject const* sender, char const* signal, QObject const* receiver, char const* member)
{ return QObject::disconnect(sender, signal, receiver, member); }
bool connect(QObject const* sender, char const* signal, QObject const* receiver, char const* member, Qt::ConnectionType arg5 = Qt::AutoConnection)
{ return QObject::connect(sender, signal, receiver, member, arg5); }
uint registerUserData()
{ return QObject::registerUserData(); }
};

class Prototype_QObject:
public QObject, protected QScriptable
{
    Q_OBJECT
private:
inline QObject *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QObject*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QObject.prototype.%0: this object is not a QObject")
            .arg(method));
}
public:
Prototype_QObject(QObject *parent = 0)
    : QObject() { setParent(parent); }
public Q_SLOTS:
void installEventFilter(QObject* arg1)
{ if (QObject *_q_q = this->self()) _q_q->installEventFilter(arg1);
  else throwTypeError(QLatin1String("installEventFilter")); }
QObjectUserData* userData(uint id) const
{ if (QObject *_q_q = this->self()) return _q_q->userData(id);
  throwTypeError(QLatin1String("userData")); return 0; }
bool disconnect(QObject const* receiver, char const* member = 0)
{ if (QObject *_q_q = this->self()) return _q_q->disconnect(receiver, member);
  throwTypeError(QLatin1String("disconnect")); return bool(); }
bool disconnect(char const* signal = 0, QObject const* receiver = 0, char const* member = 0)
{ if (QObject *_q_q = this->self()) return _q_q->disconnect(signal, receiver, member);
  throwTypeError(QLatin1String("disconnect")); return bool(); }
void dumpObjectInfo()
{ if (QObject *_q_q = this->self()) _q_q->dumpObjectInfo();
  else throwTypeError(QLatin1String("dumpObjectInfo")); }
void moveToThread(QThread* thread)
{ if (QObject *_q_q = this->self()) _q_q->moveToThread(thread);
  else throwTypeError(QLatin1String("moveToThread")); }
void removeEventFilter(QObject* arg1)
{ if (QObject *_q_q = this->self()) _q_q->removeEventFilter(arg1);
  else throwTypeError(QLatin1String("removeEventFilter")); }
bool isWidgetType() const
{ if (QObject *_q_q = this->self()) return _q_q->isWidgetType();
  throwTypeError(QLatin1String("isWidgetType")); return bool(); }
bool setProperty(char const* name, QVariant const& value)
{ if (QObject *_q_q = this->self()) return _q_q->setProperty(name, value);
  throwTypeError(QLatin1String("setProperty")); return bool(); }
void deleteLater()
{ if (QObject *_q_q = this->self()) _q_q->deleteLater();
  else throwTypeError(QLatin1String("deleteLater")); }
QObject* parent() const
{ if (QObject *_q_q = this->self()) return _q_q->parent();
  throwTypeError(QLatin1String("parent")); return 0; }
bool connect(QObject const* sender, char const* signal, char const* member, Qt::ConnectionType type = Qt::AutoConnection) const
{ if (QObject *_q_q = this->self()) return _q_q->connect(sender, signal, member, type);
  throwTypeError(QLatin1String("connect")); return bool(); }
void dumpObjectTree()
{ if (QObject *_q_q = this->self()) _q_q->dumpObjectTree();
  else throwTypeError(QLatin1String("dumpObjectTree")); }
QObjectList  children() const
{ if (QObject *_q_q = this->self()) return _q_q->children();
  throwTypeError(QLatin1String("children")); return QObjectList(); }
void killTimer(int id)
{ if (QObject *_q_q = this->self()) _q_q->killTimer(id);
  else throwTypeError(QLatin1String("killTimer")); }
void setObjectName(QString const& name)
{ if (QObject *_q_q = this->self()) _q_q->setObjectName(name);
  else throwTypeError(QLatin1String("setObjectName")); }
QThread* thread() const
{ if (QObject *_q_q = this->self()) return _q_q->thread();
  throwTypeError(QLatin1String("thread")); return 0; }
bool inherits(char const* classname) const
{ if (QObject *_q_q = this->self()) return _q_q->inherits(classname);
  throwTypeError(QLatin1String("inherits")); return bool(); }
bool signalsBlocked() const
{ if (QObject *_q_q = this->self()) return _q_q->signalsBlocked();
  throwTypeError(QLatin1String("signalsBlocked")); return bool(); }
void setParent(QObject* arg1)
{ if (QObject *_q_q = this->self()) _q_q->setParent(arg1);
  else throwTypeError(QLatin1String("setParent")); }
int startTimer(int interval)
{ if (QObject *_q_q = this->self()) return _q_q->startTimer(interval);
  throwTypeError(QLatin1String("startTimer")); return int(); }
void setUserData(uint id, QObjectUserData* data)
{ if (QObject *_q_q = this->self()) _q_q->setUserData(id, data);
  else throwTypeError(QLatin1String("setUserData")); }
bool blockSignals(bool b)
{ if (QObject *_q_q = this->self()) return _q_q->blockSignals(b);
  throwTypeError(QLatin1String("blockSignals")); return bool(); }
QVariant property(char const* name) const
{ if (QObject *_q_q = this->self()) return _q_q->property(name);
  throwTypeError(QLatin1String("property")); return QVariant(); }
bool event(QEvent* arg1)
{ if (QObject *_q_q = this->self()) return _q_q->event(arg1);
  throwTypeError(QLatin1String("event")); return bool(); }
bool eventFilter(QObject* arg1, QEvent* arg2)
{ if (QObject *_q_q = this->self()) return _q_q->eventFilter(arg1, arg2);
  throwTypeError(QLatin1String("eventFilter")); return bool(); }
QList<QByteArray> dynamicPropertyNames() const
{ if (QObject *_q_q = this->self()) return _q_q->dynamicPropertyNames();
  throwTypeError(QLatin1String("dynamicPropertyNames")); return QList<QByteArray>(); }
QString objectName() const
{ if (QObject *_q_q = this->self()) return _q_q->objectName();
  throwTypeError(QLatin1String("objectName")); return QString(); }
QString toString() const
{ return QLatin1String("QObject"); }
private:
Q_DISABLE_COPY(Prototype_QObject)
};

static QScriptValue create_QObject_class(QScriptEngine *engine)
{
    Prototype_QObject *pq = new Prototype_QObject;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Constructor_QObject *cq = new Constructor_QObject;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QObject*>(), pv);
    return cv;
}

// QPoint

Q_DECLARE_METATYPE(QPoint)
Q_DECLARE_METATYPE(QPoint*)

class Constructor_QPoint:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(int xpos, int ypos)
{ return this->engine()->toScriptValue(QPoint(xpos, ypos)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QPoint()); }
};

class Prototype_QPoint:
public QObject, public QPoint, protected QScriptable
{
    Q_OBJECT
private:
inline QPoint *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QPoint*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QPoint.prototype.%0: this object is not a QPoint")
            .arg(method));
}
public:
Prototype_QPoint(QObject *parent = 0)
    : QPoint() { setParent(parent); }
public Q_SLOTS:
void setX(int x)
{ if (QPoint *_q_q = this->self()) _q_q->setX(x);
  else throwTypeError(QLatin1String("setX")); }
void setY(int y)
{ if (QPoint *_q_q = this->self()) _q_q->setY(y);
  else throwTypeError(QLatin1String("setY")); }
int x() const
{ if (QPoint *_q_q = this->self()) return _q_q->x();
  throwTypeError(QLatin1String("x")); return int(); }
int y() const
{ if (QPoint *_q_q = this->self()) return _q_q->y();
  throwTypeError(QLatin1String("y")); return int(); }
int* rx()
{ if (QPoint *_q_q = this->self()) return &_q_q->rx();
  throwTypeError(QLatin1String("rx")); return 0; }
int* ry()
{ if (QPoint *_q_q = this->self()) return &_q_q->ry();
  throwTypeError(QLatin1String("ry")); return 0; }
bool isNull() const
{ if (QPoint *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
int manhattanLength() const
{ if (QPoint *_q_q = this->self()) return _q_q->manhattanLength();
  throwTypeError(QLatin1String("manhattanLength")); return int(); }
QPoint* subtract(QPoint const& p)
{ if (QPoint *_q_q = this->self()) return &_q_q->operator-=(p);
  throwTypeError(QLatin1String("subtract")); return 0; }
QString toString() const
{ return QLatin1String("QPoint"); }
};

static QScriptValue create_QPoint_class(QScriptEngine *engine)
{
    Prototype_QPoint *pq = new Prototype_QPoint;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QPoint *cq = new Constructor_QPoint;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QPoint>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QPoint>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QPoint*>(), pv);
    return cv;
}

// QCryptographicHash

Q_DECLARE_METATYPE(QCryptographicHash*)
Q_DECLARE_METATYPE(QCryptographicHash::Algorithm)

class Constructor_QCryptographicHash:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QCryptographicHash::Algorithm method)
{ return this->engine()->toScriptValue(new QCryptographicHash(method)); }
QByteArray hash(QByteArray const& data, QCryptographicHash::Algorithm method)
{ return QCryptographicHash::hash(data, method); }
};

class Prototype_QCryptographicHash:
public QObject, public QCryptographicHash, protected QScriptable
{
    Q_OBJECT
private:
inline QCryptographicHash *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QCryptographicHash*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QCryptographicHash.prototype.%0: this object is not a QCryptographicHash")
            .arg(method));
}
public:
Prototype_QCryptographicHash(QObject *parent = 0)
    : QCryptographicHash(QCryptographicHash::Md5) { setParent(parent); }
public Q_SLOTS:
void reset()
{ if (QCryptographicHash *_q_q = this->self()) _q_q->reset();
  else throwTypeError(QLatin1String("reset")); }
QByteArray result() const
{ if (QCryptographicHash *_q_q = this->self()) return _q_q->result();
  throwTypeError(QLatin1String("result")); return QByteArray(); }
void addData(QByteArray const& data)
{ if (QCryptographicHash *_q_q = this->self()) _q_q->addData(data);
  else throwTypeError(QLatin1String("addData")); }
void addData(char const* data, int length)
{ if (QCryptographicHash *_q_q = this->self()) _q_q->addData(data, length);
  else throwTypeError(QLatin1String("addData")); }
QString toString() const
{ return QLatin1String("QCryptographicHash"); }
private:
Q_DISABLE_COPY(Prototype_QCryptographicHash)
};

static QScriptValue create_QCryptographicHash_class(QScriptEngine *engine)
{
    Prototype_QCryptographicHash *pq = new Prototype_QCryptographicHash;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QCryptographicHash *cq = new Constructor_QCryptographicHash;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QCryptographicHash*>(), pv);
    _q_ScriptRegisterEnumMetaType<QCryptographicHash::Algorithm>(engine);
    cv.setProperty("Md4", QScriptValue(engine, static_cast<int>(QCryptographicHash::Md4)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Md5", QScriptValue(engine, static_cast<int>(QCryptographicHash::Md5)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Sha1", QScriptValue(engine, static_cast<int>(QCryptographicHash::Sha1)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QBasicTimer

Q_DECLARE_METATYPE(QBasicTimer)
Q_DECLARE_METATYPE(QBasicTimer*)

class Constructor_QBasicTimer:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QBasicTimer()); }
};

class Prototype_QBasicTimer:
public QObject, public QBasicTimer, protected QScriptable
{
    Q_OBJECT
private:
inline QBasicTimer *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QBasicTimer*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QBasicTimer.prototype.%0: this object is not a QBasicTimer")
            .arg(method));
}
public:
Prototype_QBasicTimer(QObject *parent = 0)
    : QBasicTimer() { setParent(parent); }
public Q_SLOTS:
void stop()
{ if (QBasicTimer *_q_q = this->self()) _q_q->stop();
  else throwTypeError(QLatin1String("stop")); }
bool isActive() const
{ if (QBasicTimer *_q_q = this->self()) return _q_q->isActive();
  throwTypeError(QLatin1String("isActive")); return bool(); }
int timerId() const
{ if (QBasicTimer *_q_q = this->self()) return _q_q->timerId();
  throwTypeError(QLatin1String("timerId")); return int(); }
void start(int msec, QObject* obj)
{ if (QBasicTimer *_q_q = this->self()) _q_q->start(msec, obj);
  else throwTypeError(QLatin1String("start")); }
QString toString() const
{ return QLatin1String("QBasicTimer"); }
};

static QScriptValue create_QBasicTimer_class(QScriptEngine *engine)
{
    Prototype_QBasicTimer *pq = new Prototype_QBasicTimer;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QBasicTimer *cq = new Constructor_QBasicTimer;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QBasicTimer>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QBasicTimer>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QBasicTimer*>(), pv);
    return cv;
}

// QAbstractFileEngine

Q_DECLARE_METATYPE(QAbstractFileEngine*)
Q_DECLARE_METATYPE(QAbstractFileEngine::FileName)
Q_DECLARE_METATYPE(QAbstractFileEngine::FileTime)
Q_DECLARE_METATYPE(QAbstractFileEngine::FileOwner)
Q_DECLARE_METATYPE(QAbstractFileEngine::Extension)
Q_DECLARE_METATYPE(QAbstractFileEngine::FileFlag)
Q_DECLARE_METATYPE(QAbstractFileEngine::FileFlags)

class Constructor_QAbstractFileEngine:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QAbstractFileEngine* create(QString const& fileName)
{ return QAbstractFileEngine::create(fileName); }
};

class Prototype_QAbstractFileEngine:
public QObject, public QAbstractFileEngine, protected QScriptable
{
    Q_OBJECT
private:
inline QAbstractFileEngine *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QAbstractFileEngine*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QAbstractFileEngine.prototype.%0: this object is not a QAbstractFileEngine")
            .arg(method));
}
public:
Prototype_QAbstractFileEngine(QObject *parent = 0)
    : QAbstractFileEngine() { setParent(parent); }
public Q_SLOTS:
int handle() const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->handle();
  throwTypeError(QLatin1String("handle")); return int(); }
bool seek(qint64 pos)
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->seek(pos);
  throwTypeError(QLatin1String("seek")); return bool(); }
qint64 readLine(char* data, qint64 maxlen)
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->readLine(data, maxlen);
  throwTypeError(QLatin1String("readLine")); return qint64(); }
uint ownerId(QAbstractFileEngine::FileOwner arg1) const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->ownerId(arg1);
  throwTypeError(QLatin1String("ownerId")); return uint(); }
qint64 pos() const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->pos();
  throwTypeError(QLatin1String("pos")); return qint64(); }
QFile::FileError error() const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->error();
  throwTypeError(QLatin1String("error")); return QFile::FileError(); }
bool rename(QString const& newName)
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->rename(newName);
  throwTypeError(QLatin1String("rename")); return bool(); }
bool extension(QAbstractFileEngine::Extension extension, QAbstractFileEngine::ExtensionOption const* option = 0, QAbstractFileEngine::ExtensionReturn* output = 0)
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->extension(extension, option, output);
  throwTypeError(QLatin1String("extension")); return bool(); }
qint64 size() const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->size();
  throwTypeError(QLatin1String("size")); return qint64(); }
void setFileName(QString const& file)
{ if (QAbstractFileEngine *_q_q = this->self()) _q_q->setFileName(file);
  else throwTypeError(QLatin1String("setFileName")); }
bool close()
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->close();
  throwTypeError(QLatin1String("close")); return bool(); }
bool isRelativePath() const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->isRelativePath();
  throwTypeError(QLatin1String("isRelativePath")); return bool(); }
QStringList entryList(QDir::Filters filters, QStringList const& filterNames) const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->entryList(filters, filterNames);
  throwTypeError(QLatin1String("entryList")); return QStringList(); }
bool setSize(qint64 size)
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->setSize(size);
  throwTypeError(QLatin1String("setSize")); return bool(); }
bool atEnd() const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->atEnd();
  throwTypeError(QLatin1String("atEnd")); return bool(); }
QString errorString() const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->errorString();
  throwTypeError(QLatin1String("errorString")); return QString(); }
qint64 write(char const* data, qint64 len)
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->write(data, len);
  throwTypeError(QLatin1String("write")); return qint64(); }
bool mkdir(QString const& dirName, bool createParentDirectories) const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->mkdir(dirName, createParentDirectories);
  throwTypeError(QLatin1String("mkdir")); return bool(); }
QAbstractFileEngine::Iterator* beginEntryList(QDir::Filters filters, QStringList const& filterNames)
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->beginEntryList(filters, filterNames);
  throwTypeError(QLatin1String("beginEntryList")); return 0; }
bool rmdir(QString const& dirName, bool recurseParentDirectories) const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->rmdir(dirName, recurseParentDirectories);
  throwTypeError(QLatin1String("rmdir")); return bool(); }
bool link(QString const& newName)
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->link(newName);
  throwTypeError(QLatin1String("link")); return bool(); }
QAbstractFileEngine::Iterator* endEntryList()
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->endEntryList();
  throwTypeError(QLatin1String("endEntryList")); return 0; }
bool open(QIODevice::OpenMode openMode)
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->open(openMode);
  throwTypeError(QLatin1String("open")); return bool(); }
bool supportsExtension(QAbstractFileEngine::Extension extension) const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->supportsExtension(extension);
  throwTypeError(QLatin1String("supportsExtension")); return bool(); }
bool setPermissions(uint perms)
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->setPermissions(perms);
  throwTypeError(QLatin1String("setPermissions")); return bool(); }
bool isSequential() const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->isSequential();
  throwTypeError(QLatin1String("isSequential")); return bool(); }
QString owner(QAbstractFileEngine::FileOwner arg1) const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->owner(arg1);
  throwTypeError(QLatin1String("owner")); return QString(); }
QString fileName(QAbstractFileEngine::FileName file = QAbstractFileEngine::DefaultName) const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->fileName(file);
  throwTypeError(QLatin1String("fileName")); return QString(); }
bool caseSensitive() const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->caseSensitive();
  throwTypeError(QLatin1String("caseSensitive")); return bool(); }
bool remove()
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->remove();
  throwTypeError(QLatin1String("remove")); return bool(); }
bool flush()
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->flush();
  throwTypeError(QLatin1String("flush")); return bool(); }
qint64 read(char* data, qint64 maxlen)
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->read(data, maxlen);
  throwTypeError(QLatin1String("read")); return qint64(); }
QAbstractFileEngine::FileFlags fileFlags(QAbstractFileEngine::FileFlags type = QAbstractFileEngine::FileInfoAll) const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->fileFlags(type);
  throwTypeError(QLatin1String("fileFlags")); return QAbstractFileEngine::FileFlags(); }
QDateTime fileTime(QAbstractFileEngine::FileTime time) const
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->fileTime(time);
  throwTypeError(QLatin1String("fileTime")); return QDateTime(); }
bool copy(QString const& newName)
{ if (QAbstractFileEngine *_q_q = this->self()) return _q_q->copy(newName);
  throwTypeError(QLatin1String("copy")); return bool(); }
QString toString() const
{ return QLatin1String("QAbstractFileEngine"); }
private:
Q_DISABLE_COPY(Prototype_QAbstractFileEngine)
};

static QScriptValue create_QAbstractFileEngine_class(QScriptEngine *engine)
{
    Prototype_QAbstractFileEngine *pq = new Prototype_QAbstractFileEngine;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QAbstractFileEngine *cq = new Constructor_QAbstractFileEngine;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QAbstractFileEngine*>(), pv);
    _q_ScriptRegisterEnumMetaType<QAbstractFileEngine::FileName>(engine);
    cv.setProperty("DefaultName", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::DefaultName)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("BaseName", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::BaseName)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("PathName", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::PathName)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AbsoluteName", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::AbsoluteName)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AbsolutePathName", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::AbsolutePathName)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LinkName", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::LinkName)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CanonicalName", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::CanonicalName)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CanonicalPathName", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::CanonicalPathName)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("BundleName", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::BundleName)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QAbstractFileEngine::FileTime>(engine);
    cv.setProperty("CreationTime", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::CreationTime)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ModificationTime", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::ModificationTime)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AccessTime", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::AccessTime)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QAbstractFileEngine::FileOwner>(engine);
    cv.setProperty("OwnerUser", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::OwnerUser)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("OwnerGroup", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::OwnerGroup)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QAbstractFileEngine::Extension>(engine);
    cv.setProperty("AtEndExtension", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::AtEndExtension)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FastReadLineExtension", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::FastReadLineExtension)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QAbstractFileEngine::FileFlag>(engine);
    cv.setProperty("ReadOwnerPerm", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::ReadOwnerPerm)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WriteOwnerPerm", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::WriteOwnerPerm)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ExeOwnerPerm", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::ExeOwnerPerm)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadUserPerm", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::ReadUserPerm)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WriteUserPerm", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::WriteUserPerm)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ExeUserPerm", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::ExeUserPerm)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadGroupPerm", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::ReadGroupPerm)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WriteGroupPerm", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::WriteGroupPerm)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ExeGroupPerm", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::ExeGroupPerm)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadOtherPerm", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::ReadOtherPerm)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WriteOtherPerm", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::WriteOtherPerm)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ExeOtherPerm", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::ExeOtherPerm)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LinkType", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::LinkType)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FileType", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::FileType)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirectoryType", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::DirectoryType)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("BundleType", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::BundleType)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("HiddenFlag", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::HiddenFlag)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LocalDiskFlag", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::LocalDiskFlag)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ExistsFlag", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::ExistsFlag)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RootFlag", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::RootFlag)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Refresh", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::Refresh)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("PermsMask", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::PermsMask)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TypesMask", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::TypesMask)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FlagsMask", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::FlagsMask)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FileInfoAll", QScriptValue(engine, static_cast<int>(QAbstractFileEngine::FileInfoAll)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QAbstractFileEngine::FileFlags>(engine);
    return cv;
}

// QDateTime

Q_DECLARE_METATYPE(QDateTime)
Q_DECLARE_METATYPE(QDateTime*)

class Constructor_QDateTime:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QDateTime const& other)
{ return this->engine()->toScriptValue(QDateTime(other)); }
QScriptValue qscript_call(QDate const& arg1, QTime const& arg2, Qt::TimeSpec spec = Qt::LocalTime)
{ return this->engine()->toScriptValue(QDateTime(arg1, arg2, spec)); }
QScriptValue qscript_call(QDate const& arg1)
{ return this->engine()->toScriptValue(QDateTime(arg1)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QDateTime()); }
QDateTime fromString(QString const& s, QString const& format)
{ return QDateTime::fromString(s, format); }
QDateTime fromString(QString const& s, Qt::DateFormat f = Qt::TextDate)
{ return QDateTime::fromString(s, f); }
QDateTime currentDateTime()
{ return QDateTime::currentDateTime(); }
QDateTime fromTime_t(uint secsSince1Jan1970UTC)
{ return QDateTime::fromTime_t(secsSince1Jan1970UTC); }
};

class Prototype_QDateTime:
public QObject, public QDateTime, protected QScriptable
{
    Q_OBJECT
private:
inline QDateTime *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QDateTime*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QDateTime.prototype.%0: this object is not a QDateTime")
            .arg(method));
}
public:
Prototype_QDateTime(QObject *parent = 0)
    : QDateTime() { setParent(parent); }
public Q_SLOTS:
bool equals(QDateTime const& other) const
{ if (QDateTime *_q_q = this->self()) return _q_q->operator==(other);
  throwTypeError(QLatin1String("equals")); return bool(); }
bool lessThan(QDateTime const& other) const
{ if (QDateTime *_q_q = this->self()) return _q_q->operator<(other);
  throwTypeError(QLatin1String("lessThan")); return bool(); }
QDateTime toUTC() const
{ if (QDateTime *_q_q = this->self()) return _q_q->toUTC();
  throwTypeError(QLatin1String("toUTC")); return QDateTime(); }
int secsTo(QDateTime const& arg1) const
{ if (QDateTime *_q_q = this->self()) return _q_q->secsTo(arg1);
  throwTypeError(QLatin1String("secsTo")); return int(); }
QDateTime addDays(int days) const
{ if (QDateTime *_q_q = this->self()) return _q_q->addDays(days);
  throwTypeError(QLatin1String("addDays")); return QDateTime(); }
QDate date() const
{ if (QDateTime *_q_q = this->self()) return _q_q->date();
  throwTypeError(QLatin1String("date")); return QDate(); }
QDateTime addYears(int years) const
{ if (QDateTime *_q_q = this->self()) return _q_q->addYears(years);
  throwTypeError(QLatin1String("addYears")); return QDateTime(); }
QDateTime addMonths(int months) const
{ if (QDateTime *_q_q = this->self()) return _q_q->addMonths(months);
  throwTypeError(QLatin1String("addMonths")); return QDateTime(); }
QTime time() const
{ if (QDateTime *_q_q = this->self()) return _q_q->time();
  throwTypeError(QLatin1String("time")); return QTime(); }
QDateTime addSecs(int secs) const
{ if (QDateTime *_q_q = this->self()) return _q_q->addSecs(secs);
  throwTypeError(QLatin1String("addSecs")); return QDateTime(); }
QString toString(QString const& format) const
{ if (QDateTime *_q_q = this->self()) return _q_q->toString(format);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(Qt::DateFormat f = Qt::TextDate) const
{ if (QDateTime *_q_q = this->self()) return _q_q->toString(f);
  throwTypeError(QLatin1String("toString")); return QString(); }
Qt::TimeSpec timeSpec() const
{ if (QDateTime *_q_q = this->self()) return _q_q->timeSpec();
  throwTypeError(QLatin1String("timeSpec")); return Qt::TimeSpec(); }
void setTime(QTime const& time)
{ if (QDateTime *_q_q = this->self()) _q_q->setTime(time);
  else throwTypeError(QLatin1String("setTime")); }
void setTimeSpec(Qt::TimeSpec spec)
{ if (QDateTime *_q_q = this->self()) _q_q->setTimeSpec(spec);
  else throwTypeError(QLatin1String("setTimeSpec")); }
int daysTo(QDateTime const& arg1) const
{ if (QDateTime *_q_q = this->self()) return _q_q->daysTo(arg1);
  throwTypeError(QLatin1String("daysTo")); return int(); }
void setDate(QDate const& date)
{ if (QDateTime *_q_q = this->self()) _q_q->setDate(date);
  else throwTypeError(QLatin1String("setDate")); }
void setTime_t(uint secsSince1Jan1970UTC)
{ if (QDateTime *_q_q = this->self()) _q_q->setTime_t(secsSince1Jan1970UTC);
  else throwTypeError(QLatin1String("setTime_t")); }
bool isNull() const
{ if (QDateTime *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
QDateTime addMSecs(qint64 msecs) const
{ if (QDateTime *_q_q = this->self()) return _q_q->addMSecs(msecs);
  throwTypeError(QLatin1String("addMSecs")); return QDateTime(); }
uint toTime_t() const
{ if (QDateTime *_q_q = this->self()) return _q_q->toTime_t();
  throwTypeError(QLatin1String("toTime_t")); return uint(); }
QDateTime toLocalTime() const
{ if (QDateTime *_q_q = this->self()) return _q_q->toLocalTime();
  throwTypeError(QLatin1String("toLocalTime")); return QDateTime(); }
bool isValid() const
{ if (QDateTime *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
QDateTime toTimeSpec(Qt::TimeSpec spec) const
{ if (QDateTime *_q_q = this->self()) return _q_q->toTimeSpec(spec);
  throwTypeError(QLatin1String("toTimeSpec")); return QDateTime(); }
};

static QScriptValue create_QDateTime_class(QScriptEngine *engine)
{
    Prototype_QDateTime *pq = new Prototype_QDateTime;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QDateTime *cq = new Constructor_QDateTime;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QDateTime>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QDateTime>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QDateTime*>(), pv);
    return cv;
}

// QTime

Q_DECLARE_METATYPE(QTime)
Q_DECLARE_METATYPE(QTime*)

class Constructor_QTime:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(int h, int m, int s = 0, int ms = 0)
{ return this->engine()->toScriptValue(QTime(h, m, s, ms)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QTime()); }
bool isValid(int h, int m, int s, int ms = 0)
{ return QTime::isValid(h, m, s, ms); }
QTime fromString(QString const& s, QString const& format)
{ return QTime::fromString(s, format); }
QTime fromString(QString const& s, Qt::DateFormat f = Qt::TextDate)
{ return QTime::fromString(s, f); }
QTime currentTime()
{ return QTime::currentTime(); }
};

class Prototype_QTime:
public QObject, public QTime, protected QScriptable
{
    Q_OBJECT
private:
inline QTime *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QTime*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QTime.prototype.%0: this object is not a QTime")
            .arg(method));
}
public:
Prototype_QTime(QObject *parent = 0)
    : QTime() { setParent(parent); }
public Q_SLOTS:
bool isValid() const
{ if (QTime *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
void start()
{ if (QTime *_q_q = this->self()) _q_q->start();
  else throwTypeError(QLatin1String("start")); }
QString toString(QString const& format) const
{ if (QTime *_q_q = this->self()) return _q_q->toString(format);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(Qt::DateFormat f = Qt::TextDate) const
{ if (QTime *_q_q = this->self()) return _q_q->toString(f);
  throwTypeError(QLatin1String("toString")); return QString(); }
QTime addSecs(int secs) const
{ if (QTime *_q_q = this->self()) return _q_q->addSecs(secs);
  throwTypeError(QLatin1String("addSecs")); return QTime(); }
bool lessThan(QTime const& other) const
{ if (QTime *_q_q = this->self()) return _q_q->operator<(other);
  throwTypeError(QLatin1String("lessThan")); return bool(); }
int msecsTo(QTime const& arg1) const
{ if (QTime *_q_q = this->self()) return _q_q->msecsTo(arg1);
  throwTypeError(QLatin1String("msecsTo")); return int(); }
int elapsed() const
{ if (QTime *_q_q = this->self()) return _q_q->elapsed();
  throwTypeError(QLatin1String("elapsed")); return int(); }
bool setHMS(int h, int m, int s, int ms = 0)
{ if (QTime *_q_q = this->self()) return _q_q->setHMS(h, m, s, ms);
  throwTypeError(QLatin1String("setHMS")); return bool(); }
int minute() const
{ if (QTime *_q_q = this->self()) return _q_q->minute();
  throwTypeError(QLatin1String("minute")); return int(); }
bool equals(QTime const& other) const
{ if (QTime *_q_q = this->self()) return _q_q->operator==(other);
  throwTypeError(QLatin1String("equals")); return bool(); }
bool isNull() const
{ if (QTime *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
int msec() const
{ if (QTime *_q_q = this->self()) return _q_q->msec();
  throwTypeError(QLatin1String("msec")); return int(); }
int secsTo(QTime const& arg1) const
{ if (QTime *_q_q = this->self()) return _q_q->secsTo(arg1);
  throwTypeError(QLatin1String("secsTo")); return int(); }
int hour() const
{ if (QTime *_q_q = this->self()) return _q_q->hour();
  throwTypeError(QLatin1String("hour")); return int(); }
QTime addMSecs(int ms) const
{ if (QTime *_q_q = this->self()) return _q_q->addMSecs(ms);
  throwTypeError(QLatin1String("addMSecs")); return QTime(); }
int restart()
{ if (QTime *_q_q = this->self()) return _q_q->restart();
  throwTypeError(QLatin1String("restart")); return int(); }
int second() const
{ if (QTime *_q_q = this->self()) return _q_q->second();
  throwTypeError(QLatin1String("second")); return int(); }
};

static QScriptValue create_QTime_class(QScriptEngine *engine)
{
    Prototype_QTime *pq = new Prototype_QTime;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QTime *cq = new Constructor_QTime;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QTime>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QTime>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QTime*>(), pv);
    return cv;
}

// QRectF

Q_DECLARE_METATYPE(QRectF)
Q_DECLARE_METATYPE(QRectF*)

class Constructor_QRectF:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QRect const& rect)
{ return this->engine()->toScriptValue(QRectF(rect)); }
QScriptValue qscript_call(qreal left, qreal top, qreal width, qreal height)
{ return this->engine()->toScriptValue(QRectF(left, top, width, height)); }
QScriptValue qscript_call(QPointF const& topleft, QPointF const& bottomRight)
{ return this->engine()->toScriptValue(QRectF(topleft, bottomRight)); }
QScriptValue qscript_call(QPointF const& topleft, QSizeF const& size)
{ return this->engine()->toScriptValue(QRectF(topleft, size)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QRectF()); }
};

class Prototype_QRectF:
public QObject, public QRectF, protected QScriptable
{
    Q_OBJECT
private:
inline QRectF *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QRectF*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QRectF.prototype.%0: this object is not a QRectF")
            .arg(method));
}
public:
Prototype_QRectF(QObject *parent = 0)
    : QRectF() { setParent(parent); }
public Q_SLOTS:
QPointF bottomRight() const
{ if (QRectF *_q_q = this->self()) return _q_q->bottomRight();
  throwTypeError(QLatin1String("bottomRight")); return QPointF(); }
void setTopLeft(QPointF const& p)
{ if (QRectF *_q_q = this->self()) _q_q->setTopLeft(p);
  else throwTypeError(QLatin1String("setTopLeft")); }
void moveCenter(QPointF const& p)
{ if (QRectF *_q_q = this->self()) _q_q->moveCenter(p);
  else throwTypeError(QLatin1String("moveCenter")); }
QPointF topLeft() const
{ if (QRectF *_q_q = this->self()) return _q_q->topLeft();
  throwTypeError(QLatin1String("topLeft")); return QPointF(); }
qreal top() const
{ if (QRectF *_q_q = this->self()) return _q_q->top();
  throwTypeError(QLatin1String("top")); return qreal(); }
qreal width() const
{ if (QRectF *_q_q = this->self()) return _q_q->width();
  throwTypeError(QLatin1String("width")); return qreal(); }
QRectF intersect(QRectF const& r) const
{ if (QRectF *_q_q = this->self()) return _q_q->intersect(r);
  throwTypeError(QLatin1String("intersect")); return QRectF(); }
void moveBottom(qreal pos)
{ if (QRectF *_q_q = this->self()) _q_q->moveBottom(pos);
  else throwTypeError(QLatin1String("moveBottom")); }
QRectF translated(QPointF const& p) const
{ if (QRectF *_q_q = this->self()) return _q_q->translated(p);
  throwTypeError(QLatin1String("translated")); return QRectF(); }
QRectF translated(qreal dx, qreal dy) const
{ if (QRectF *_q_q = this->self()) return _q_q->translated(dx, dy);
  throwTypeError(QLatin1String("translated")); return QRectF(); }
void setTopRight(QPointF const& p)
{ if (QRectF *_q_q = this->self()) _q_q->setTopRight(p);
  else throwTypeError(QLatin1String("setTopRight")); }
void getRect(qreal* x, qreal* y, qreal* w, qreal* h) const
{ if (QRectF *_q_q = this->self()) _q_q->getRect(x, y, w, h);
  else throwTypeError(QLatin1String("getRect")); }
QRect toAlignedRect() const
{ if (QRectF *_q_q = this->self()) return _q_q->toAlignedRect();
  throwTypeError(QLatin1String("toAlignedRect")); return QRect(); }
void setWidth(qreal w)
{ if (QRectF *_q_q = this->self()) _q_q->setWidth(w);
  else throwTypeError(QLatin1String("setWidth")); }
void moveTop(qreal pos)
{ if (QRectF *_q_q = this->self()) _q_q->moveTop(pos);
  else throwTypeError(QLatin1String("moveTop")); }
qreal right() const
{ if (QRectF *_q_q = this->self()) return _q_q->right();
  throwTypeError(QLatin1String("right")); return qreal(); }
QRectF normalized() const
{ if (QRectF *_q_q = this->self()) return _q_q->normalized();
  throwTypeError(QLatin1String("normalized")); return QRectF(); }
QPointF topRight() const
{ if (QRectF *_q_q = this->self()) return _q_q->topRight();
  throwTypeError(QLatin1String("topRight")); return QPointF(); }
QPointF center() const
{ if (QRectF *_q_q = this->self()) return _q_q->center();
  throwTypeError(QLatin1String("center")); return QPointF(); }
void setBottom(qreal pos)
{ if (QRectF *_q_q = this->self()) _q_q->setBottom(pos);
  else throwTypeError(QLatin1String("setBottom")); }
bool intersects(QRectF const& r) const
{ if (QRectF *_q_q = this->self()) return _q_q->intersects(r);
  throwTypeError(QLatin1String("intersects")); return bool(); }
QPointF bottomLeft() const
{ if (QRectF *_q_q = this->self()) return _q_q->bottomLeft();
  throwTypeError(QLatin1String("bottomLeft")); return QPointF(); }
qreal bottom() const
{ if (QRectF *_q_q = this->self()) return _q_q->bottom();
  throwTypeError(QLatin1String("bottom")); return qreal(); }
bool isEmpty() const
{ if (QRectF *_q_q = this->self()) return _q_q->isEmpty();
  throwTypeError(QLatin1String("isEmpty")); return bool(); }
QRectF united(QRectF const& other) const
{ if (QRectF *_q_q = this->self()) return _q_q->united(other);
  throwTypeError(QLatin1String("united")); return QRectF(); }
void setRight(qreal pos)
{ if (QRectF *_q_q = this->self()) _q_q->setRight(pos);
  else throwTypeError(QLatin1String("setRight")); }
void setRect(qreal x, qreal y, qreal w, qreal h)
{ if (QRectF *_q_q = this->self()) _q_q->setRect(x, y, w, h);
  else throwTypeError(QLatin1String("setRect")); }
void moveLeft(qreal pos)
{ if (QRectF *_q_q = this->self()) _q_q->moveLeft(pos);
  else throwTypeError(QLatin1String("moveLeft")); }
qreal left() const
{ if (QRectF *_q_q = this->self()) return _q_q->left();
  throwTypeError(QLatin1String("left")); return qreal(); }
void moveBottomLeft(QPointF const& p)
{ if (QRectF *_q_q = this->self()) _q_q->moveBottomLeft(p);
  else throwTypeError(QLatin1String("moveBottomLeft")); }
QSizeF size() const
{ if (QRectF *_q_q = this->self()) return _q_q->size();
  throwTypeError(QLatin1String("size")); return QSizeF(); }
void translate(QPointF const& p)
{ if (QRectF *_q_q = this->self()) _q_q->translate(p);
  else throwTypeError(QLatin1String("translate")); }
void translate(qreal dx, qreal dy)
{ if (QRectF *_q_q = this->self()) _q_q->translate(dx, dy);
  else throwTypeError(QLatin1String("translate")); }
void moveBottomRight(QPointF const& p)
{ if (QRectF *_q_q = this->self()) _q_q->moveBottomRight(p);
  else throwTypeError(QLatin1String("moveBottomRight")); }
QRectF adjusted(qreal x1, qreal y1, qreal x2, qreal y2) const
{ if (QRectF *_q_q = this->self()) return _q_q->adjusted(x1, y1, x2, y2);
  throwTypeError(QLatin1String("adjusted")); return QRectF(); }
void getCoords(qreal* x1, qreal* y1, qreal* x2, qreal* y2) const
{ if (QRectF *_q_q = this->self()) _q_q->getCoords(x1, y1, x2, y2);
  else throwTypeError(QLatin1String("getCoords")); }
bool contains(QRectF const& r) const
{ if (QRectF *_q_q = this->self()) return _q_q->contains(r);
  throwTypeError(QLatin1String("contains")); return bool(); }
bool contains(qreal x, qreal y) const
{ if (QRectF *_q_q = this->self()) return _q_q->contains(x, y);
  throwTypeError(QLatin1String("contains")); return bool(); }
bool contains(QPointF const& p) const
{ if (QRectF *_q_q = this->self()) return _q_q->contains(p);
  throwTypeError(QLatin1String("contains")); return bool(); }
void setHeight(qreal h)
{ if (QRectF *_q_q = this->self()) _q_q->setHeight(h);
  else throwTypeError(QLatin1String("setHeight")); }
void moveTopLeft(QPointF const& p)
{ if (QRectF *_q_q = this->self()) _q_q->moveTopLeft(p);
  else throwTypeError(QLatin1String("moveTopLeft")); }
QRectF unite(QRectF const& r) const
{ if (QRectF *_q_q = this->self()) return _q_q->unite(r);
  throwTypeError(QLatin1String("unite")); return QRectF(); }
bool isValid() const
{ if (QRectF *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
void setBottomRight(QPointF const& p)
{ if (QRectF *_q_q = this->self()) _q_q->setBottomRight(p);
  else throwTypeError(QLatin1String("setBottomRight")); }
void moveTopRight(QPointF const& p)
{ if (QRectF *_q_q = this->self()) _q_q->moveTopRight(p);
  else throwTypeError(QLatin1String("moveTopRight")); }
void setCoords(qreal x1, qreal y1, qreal x2, qreal y2)
{ if (QRectF *_q_q = this->self()) _q_q->setCoords(x1, y1, x2, y2);
  else throwTypeError(QLatin1String("setCoords")); }
void setSize(QSizeF const& s)
{ if (QRectF *_q_q = this->self()) _q_q->setSize(s);
  else throwTypeError(QLatin1String("setSize")); }
void moveTo(QPointF const& p)
{ if (QRectF *_q_q = this->self()) _q_q->moveTo(p);
  else throwTypeError(QLatin1String("moveTo")); }
void moveTo(qreal x, qreal t)
{ if (QRectF *_q_q = this->self()) _q_q->moveTo(x, t);
  else throwTypeError(QLatin1String("moveTo")); }
bool isNull() const
{ if (QRectF *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
QRect toRect() const
{ if (QRectF *_q_q = this->self()) return _q_q->toRect();
  throwTypeError(QLatin1String("toRect")); return QRect(); }
void moveRight(qreal pos)
{ if (QRectF *_q_q = this->self()) _q_q->moveRight(pos);
  else throwTypeError(QLatin1String("moveRight")); }
void setLeft(qreal pos)
{ if (QRectF *_q_q = this->self()) _q_q->setLeft(pos);
  else throwTypeError(QLatin1String("setLeft")); }
void adjust(qreal x1, qreal y1, qreal x2, qreal y2)
{ if (QRectF *_q_q = this->self()) _q_q->adjust(x1, y1, x2, y2);
  else throwTypeError(QLatin1String("adjust")); }
void setBottomLeft(QPointF const& p)
{ if (QRectF *_q_q = this->self()) _q_q->setBottomLeft(p);
  else throwTypeError(QLatin1String("setBottomLeft")); }
qreal x() const
{ if (QRectF *_q_q = this->self()) return _q_q->x();
  throwTypeError(QLatin1String("x")); return qreal(); }
qreal y() const
{ if (QRectF *_q_q = this->self()) return _q_q->y();
  throwTypeError(QLatin1String("y")); return qreal(); }
void setX(qreal pos)
{ if (QRectF *_q_q = this->self()) _q_q->setX(pos);
  else throwTypeError(QLatin1String("setX")); }
QRectF intersected(QRectF const& other) const
{ if (QRectF *_q_q = this->self()) return _q_q->intersected(other);
  throwTypeError(QLatin1String("intersected")); return QRectF(); }
void setY(qreal pos)
{ if (QRectF *_q_q = this->self()) _q_q->setY(pos);
  else throwTypeError(QLatin1String("setY")); }
void setTop(qreal pos)
{ if (QRectF *_q_q = this->self()) _q_q->setTop(pos);
  else throwTypeError(QLatin1String("setTop")); }
qreal height() const
{ if (QRectF *_q_q = this->self()) return _q_q->height();
  throwTypeError(QLatin1String("height")); return qreal(); }
QString toString() const
{ return QLatin1String("QRectF"); }
};

static QScriptValue create_QRectF_class(QScriptEngine *engine)
{
    Prototype_QRectF *pq = new Prototype_QRectF;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QRectF *cq = new Constructor_QRectF;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QRectF>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QRectF>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QRectF*>(), pv);
    return cv;
}

// QLocale

Q_DECLARE_METATYPE(QLocale)
Q_DECLARE_METATYPE(QLocale*)
Q_DECLARE_METATYPE(QLocale::NumberOption)
Q_DECLARE_METATYPE(QLocale::FormatType)
Q_DECLARE_METATYPE(QLocale::Country)
Q_DECLARE_METATYPE(QLocale::Language)
Q_DECLARE_METATYPE(QLocale::NumberOptions)

class Constructor_QLocale:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QLocale const& other)
{ return this->engine()->toScriptValue(QLocale(other)); }
QScriptValue qscript_call(QLocale::Language language, QLocale::Country country = QLocale::AnyCountry)
{ return this->engine()->toScriptValue(QLocale(language, country)); }
QScriptValue qscript_call(QString const& name)
{ return this->engine()->toScriptValue(QLocale(name)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QLocale()); }
QLocale system()
{ return QLocale::system(); }
void setDefault(QLocale const& locale)
{ QLocale::setDefault(locale); }
QString languageToString(QLocale::Language language)
{ return QLocale::languageToString(language); }
QLocale c()
{ return QLocale::c(); }
QList<QLocale::Country> countriesForLanguage(QLocale::Language lang)
{ return QLocale::countriesForLanguage(lang); }
QString countryToString(QLocale::Country country)
{ return QLocale::countryToString(country); }
};

class Prototype_QLocale:
public QObject, public QLocale, protected QScriptable
{
    Q_OBJECT
private:
inline QLocale *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QLocale*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QLocale.prototype.%0: this object is not a QLocale")
            .arg(method));
}
public:
Prototype_QLocale(QObject *parent = 0)
    : QLocale() { setParent(parent); }
public Q_SLOTS:
QString monthName(int arg1, QLocale::FormatType format = QLocale::LongFormat) const
{ if (QLocale *_q_q = this->self()) return _q_q->monthName(arg1, format);
  throwTypeError(QLatin1String("monthName")); return QString(); }
bool equals(QLocale const& other) const
{ if (QLocale *_q_q = this->self()) return _q_q->operator==(other);
  throwTypeError(QLatin1String("equals")); return bool(); }
QString dayName(int arg1, QLocale::FormatType format = QLocale::LongFormat) const
{ if (QLocale *_q_q = this->self()) return _q_q->dayName(arg1, format);
  throwTypeError(QLatin1String("dayName")); return QString(); }
ushort toUShort(QString const& s, bool* ok = 0, int base = 0) const
{ if (QLocale *_q_q = this->self()) return _q_q->toUShort(s, ok, base);
  throwTypeError(QLatin1String("toUShort")); return ushort(); }
int toInt(QString const& s, bool* ok = 0, int base = 0) const
{ if (QLocale *_q_q = this->self()) return _q_q->toInt(s, ok, base);
  throwTypeError(QLatin1String("toInt")); return int(); }
QString timeFormat(QLocale::FormatType format = QLocale::LongFormat) const
{ if (QLocale *_q_q = this->self()) return _q_q->timeFormat(format);
  throwTypeError(QLatin1String("timeFormat")); return QString(); }
qlonglong toULongLong(QString const& s, bool* ok = 0, int base = 0) const
{ if (QLocale *_q_q = this->self()) return _q_q->toULongLong(s, ok, base);
  throwTypeError(QLatin1String("toULongLong")); return qlonglong(); }
double toDouble(QString const& s, bool* ok = 0) const
{ if (QLocale *_q_q = this->self()) return _q_q->toDouble(s, ok);
  throwTypeError(QLatin1String("toDouble")); return double(); }
QLocale::Country country() const
{ if (QLocale *_q_q = this->self()) return _q_q->country();
  throwTypeError(QLatin1String("country")); return QLocale::Country(); }
QChar exponential() const
{ if (QLocale *_q_q = this->self()) return _q_q->exponential();
  throwTypeError(QLatin1String("exponential")); return QChar(); }
QChar decimalPoint() const
{ if (QLocale *_q_q = this->self()) return _q_q->decimalPoint();
  throwTypeError(QLatin1String("decimalPoint")); return QChar(); }
QLocale::Language language() const
{ if (QLocale *_q_q = this->self()) return _q_q->language();
  throwTypeError(QLatin1String("language")); return QLocale::Language(); }
float toFloat(QString const& s, bool* ok = 0) const
{ if (QLocale *_q_q = this->self()) return _q_q->toFloat(s, ok);
  throwTypeError(QLatin1String("toFloat")); return float(); }
QChar negativeSign() const
{ if (QLocale *_q_q = this->self()) return _q_q->negativeSign();
  throwTypeError(QLatin1String("negativeSign")); return QChar(); }
QString toString(QTime const& time, QLocale::FormatType format = QLocale::LongFormat) const
{ if (QLocale *_q_q = this->self()) return _q_q->toString(time, format);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(QTime const& time, QString const& formatStr) const
{ if (QLocale *_q_q = this->self()) return _q_q->toString(time, formatStr);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(QDate const& date, QLocale::FormatType format = QLocale::LongFormat) const
{ if (QLocale *_q_q = this->self()) return _q_q->toString(date, format);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(QDate const& date, QString const& formatStr) const
{ if (QLocale *_q_q = this->self()) return _q_q->toString(date, formatStr);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(float i, char f = 'g', int prec = 6) const
{ if (QLocale *_q_q = this->self()) return _q_q->toString(i, f, prec);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(double i, char f = 'g', int prec = 6) const
{ if (QLocale *_q_q = this->self()) return _q_q->toString(i, f, prec);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(uint i) const
{ if (QLocale *_q_q = this->self()) return _q_q->toString(i);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(int i) const
{ if (QLocale *_q_q = this->self()) return _q_q->toString(i);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(ushort i) const
{ if (QLocale *_q_q = this->self()) return _q_q->toString(i);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(short i) const
{ if (QLocale *_q_q = this->self()) return _q_q->toString(i);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(qulonglong i) const
{ if (QLocale *_q_q = this->self()) return _q_q->toString(i);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(qlonglong i) const
{ if (QLocale *_q_q = this->self()) return _q_q->toString(i);
  throwTypeError(QLatin1String("toString")); return QString(); }
QChar groupSeparator() const
{ if (QLocale *_q_q = this->self()) return _q_q->groupSeparator();
  throwTypeError(QLatin1String("groupSeparator")); return QChar(); }
qlonglong toLongLong(QString const& s, bool* ok = 0, int base = 0) const
{ if (QLocale *_q_q = this->self()) return _q_q->toLongLong(s, ok, base);
  throwTypeError(QLatin1String("toLongLong")); return qlonglong(); }
void setNumberOptions(QLocale::NumberOptions options)
{ if (QLocale *_q_q = this->self()) _q_q->setNumberOptions(options);
  else throwTypeError(QLatin1String("setNumberOptions")); }
QString dateFormat(QLocale::FormatType format = QLocale::LongFormat) const
{ if (QLocale *_q_q = this->self()) return _q_q->dateFormat(format);
  throwTypeError(QLatin1String("dateFormat")); return QString(); }
QLocale::NumberOptions numberOptions() const
{ if (QLocale *_q_q = this->self()) return _q_q->numberOptions();
  throwTypeError(QLatin1String("numberOptions")); return QLocale::NumberOptions(); }
QChar percent() const
{ if (QLocale *_q_q = this->self()) return _q_q->percent();
  throwTypeError(QLatin1String("percent")); return QChar(); }
short toShort(QString const& s, bool* ok = 0, int base = 0) const
{ if (QLocale *_q_q = this->self()) return _q_q->toShort(s, ok, base);
  throwTypeError(QLatin1String("toShort")); return short(); }
uint toUInt(QString const& s, bool* ok = 0, int base = 0) const
{ if (QLocale *_q_q = this->self()) return _q_q->toUInt(s, ok, base);
  throwTypeError(QLatin1String("toUInt")); return uint(); }
QString name() const
{ if (QLocale *_q_q = this->self()) return _q_q->name();
  throwTypeError(QLatin1String("name")); return QString(); }
QChar zeroDigit() const
{ if (QLocale *_q_q = this->self()) return _q_q->zeroDigit();
  throwTypeError(QLatin1String("zeroDigit")); return QChar(); }
};

static QScriptValue create_QLocale_class(QScriptEngine *engine)
{
    Prototype_QLocale *pq = new Prototype_QLocale;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QLocale *cq = new Constructor_QLocale;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QLocale>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QLocale>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QLocale*>(), pv);
    _q_ScriptRegisterEnumMetaType<QLocale::NumberOption>(engine);
    cv.setProperty("OmitGroupSeparator", QScriptValue(engine, static_cast<int>(QLocale::OmitGroupSeparator)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RejectGroupSeparator", QScriptValue(engine, static_cast<int>(QLocale::RejectGroupSeparator)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QLocale::FormatType>(engine);
    cv.setProperty("LongFormat", QScriptValue(engine, static_cast<int>(QLocale::LongFormat)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ShortFormat", QScriptValue(engine, static_cast<int>(QLocale::ShortFormat)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QLocale::Country>(engine);
    cv.setProperty("AnyCountry", QScriptValue(engine, static_cast<int>(QLocale::AnyCountry)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Afghanistan", QScriptValue(engine, static_cast<int>(QLocale::Afghanistan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Albania", QScriptValue(engine, static_cast<int>(QLocale::Albania)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Algeria", QScriptValue(engine, static_cast<int>(QLocale::Algeria)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AmericanSamoa", QScriptValue(engine, static_cast<int>(QLocale::AmericanSamoa)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Andorra", QScriptValue(engine, static_cast<int>(QLocale::Andorra)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Angola", QScriptValue(engine, static_cast<int>(QLocale::Angola)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Anguilla", QScriptValue(engine, static_cast<int>(QLocale::Anguilla)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Antarctica", QScriptValue(engine, static_cast<int>(QLocale::Antarctica)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AntiguaAndBarbuda", QScriptValue(engine, static_cast<int>(QLocale::AntiguaAndBarbuda)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Argentina", QScriptValue(engine, static_cast<int>(QLocale::Argentina)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Armenia", QScriptValue(engine, static_cast<int>(QLocale::Armenia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Aruba", QScriptValue(engine, static_cast<int>(QLocale::Aruba)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Australia", QScriptValue(engine, static_cast<int>(QLocale::Australia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Austria", QScriptValue(engine, static_cast<int>(QLocale::Austria)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Azerbaijan", QScriptValue(engine, static_cast<int>(QLocale::Azerbaijan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bahamas", QScriptValue(engine, static_cast<int>(QLocale::Bahamas)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bahrain", QScriptValue(engine, static_cast<int>(QLocale::Bahrain)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bangladesh", QScriptValue(engine, static_cast<int>(QLocale::Bangladesh)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Barbados", QScriptValue(engine, static_cast<int>(QLocale::Barbados)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Belarus", QScriptValue(engine, static_cast<int>(QLocale::Belarus)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Belgium", QScriptValue(engine, static_cast<int>(QLocale::Belgium)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Belize", QScriptValue(engine, static_cast<int>(QLocale::Belize)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Benin", QScriptValue(engine, static_cast<int>(QLocale::Benin)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bermuda", QScriptValue(engine, static_cast<int>(QLocale::Bermuda)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bhutan", QScriptValue(engine, static_cast<int>(QLocale::Bhutan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bolivia", QScriptValue(engine, static_cast<int>(QLocale::Bolivia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("BosniaAndHerzegowina", QScriptValue(engine, static_cast<int>(QLocale::BosniaAndHerzegowina)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Botswana", QScriptValue(engine, static_cast<int>(QLocale::Botswana)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("BouvetIsland", QScriptValue(engine, static_cast<int>(QLocale::BouvetIsland)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Brazil", QScriptValue(engine, static_cast<int>(QLocale::Brazil)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("BritishIndianOceanTerritory", QScriptValue(engine, static_cast<int>(QLocale::BritishIndianOceanTerritory)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("BruneiDarussalam", QScriptValue(engine, static_cast<int>(QLocale::BruneiDarussalam)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bulgaria", QScriptValue(engine, static_cast<int>(QLocale::Bulgaria)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("BurkinaFaso", QScriptValue(engine, static_cast<int>(QLocale::BurkinaFaso)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Burundi", QScriptValue(engine, static_cast<int>(QLocale::Burundi)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Cambodia", QScriptValue(engine, static_cast<int>(QLocale::Cambodia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Cameroon", QScriptValue(engine, static_cast<int>(QLocale::Cameroon)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Canada", QScriptValue(engine, static_cast<int>(QLocale::Canada)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CapeVerde", QScriptValue(engine, static_cast<int>(QLocale::CapeVerde)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CaymanIslands", QScriptValue(engine, static_cast<int>(QLocale::CaymanIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CentralAfricanRepublic", QScriptValue(engine, static_cast<int>(QLocale::CentralAfricanRepublic)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Chad", QScriptValue(engine, static_cast<int>(QLocale::Chad)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Chile", QScriptValue(engine, static_cast<int>(QLocale::Chile)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("China", QScriptValue(engine, static_cast<int>(QLocale::China)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ChristmasIsland", QScriptValue(engine, static_cast<int>(QLocale::ChristmasIsland)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CocosIslands", QScriptValue(engine, static_cast<int>(QLocale::CocosIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Colombia", QScriptValue(engine, static_cast<int>(QLocale::Colombia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Comoros", QScriptValue(engine, static_cast<int>(QLocale::Comoros)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DemocraticRepublicOfCongo", QScriptValue(engine, static_cast<int>(QLocale::DemocraticRepublicOfCongo)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("PeoplesRepublicOfCongo", QScriptValue(engine, static_cast<int>(QLocale::PeoplesRepublicOfCongo)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CookIslands", QScriptValue(engine, static_cast<int>(QLocale::CookIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CostaRica", QScriptValue(engine, static_cast<int>(QLocale::CostaRica)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("IvoryCoast", QScriptValue(engine, static_cast<int>(QLocale::IvoryCoast)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Croatia", QScriptValue(engine, static_cast<int>(QLocale::Croatia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Cuba", QScriptValue(engine, static_cast<int>(QLocale::Cuba)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Cyprus", QScriptValue(engine, static_cast<int>(QLocale::Cyprus)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CzechRepublic", QScriptValue(engine, static_cast<int>(QLocale::CzechRepublic)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Denmark", QScriptValue(engine, static_cast<int>(QLocale::Denmark)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Djibouti", QScriptValue(engine, static_cast<int>(QLocale::Djibouti)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Dominica", QScriptValue(engine, static_cast<int>(QLocale::Dominica)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DominicanRepublic", QScriptValue(engine, static_cast<int>(QLocale::DominicanRepublic)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("EastTimor", QScriptValue(engine, static_cast<int>(QLocale::EastTimor)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Ecuador", QScriptValue(engine, static_cast<int>(QLocale::Ecuador)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Egypt", QScriptValue(engine, static_cast<int>(QLocale::Egypt)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ElSalvador", QScriptValue(engine, static_cast<int>(QLocale::ElSalvador)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("EquatorialGuinea", QScriptValue(engine, static_cast<int>(QLocale::EquatorialGuinea)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Eritrea", QScriptValue(engine, static_cast<int>(QLocale::Eritrea)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Estonia", QScriptValue(engine, static_cast<int>(QLocale::Estonia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Ethiopia", QScriptValue(engine, static_cast<int>(QLocale::Ethiopia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FalklandIslands", QScriptValue(engine, static_cast<int>(QLocale::FalklandIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FaroeIslands", QScriptValue(engine, static_cast<int>(QLocale::FaroeIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FijiCountry", QScriptValue(engine, static_cast<int>(QLocale::FijiCountry)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Finland", QScriptValue(engine, static_cast<int>(QLocale::Finland)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("France", QScriptValue(engine, static_cast<int>(QLocale::France)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MetropolitanFrance", QScriptValue(engine, static_cast<int>(QLocale::MetropolitanFrance)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FrenchGuiana", QScriptValue(engine, static_cast<int>(QLocale::FrenchGuiana)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FrenchPolynesia", QScriptValue(engine, static_cast<int>(QLocale::FrenchPolynesia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FrenchSouthernTerritories", QScriptValue(engine, static_cast<int>(QLocale::FrenchSouthernTerritories)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Gabon", QScriptValue(engine, static_cast<int>(QLocale::Gabon)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Gambia", QScriptValue(engine, static_cast<int>(QLocale::Gambia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Georgia", QScriptValue(engine, static_cast<int>(QLocale::Georgia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Germany", QScriptValue(engine, static_cast<int>(QLocale::Germany)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Ghana", QScriptValue(engine, static_cast<int>(QLocale::Ghana)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Gibraltar", QScriptValue(engine, static_cast<int>(QLocale::Gibraltar)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Greece", QScriptValue(engine, static_cast<int>(QLocale::Greece)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Greenland", QScriptValue(engine, static_cast<int>(QLocale::Greenland)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Grenada", QScriptValue(engine, static_cast<int>(QLocale::Grenada)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Guadeloupe", QScriptValue(engine, static_cast<int>(QLocale::Guadeloupe)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Guam", QScriptValue(engine, static_cast<int>(QLocale::Guam)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Guatemala", QScriptValue(engine, static_cast<int>(QLocale::Guatemala)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Guinea", QScriptValue(engine, static_cast<int>(QLocale::Guinea)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GuineaBissau", QScriptValue(engine, static_cast<int>(QLocale::GuineaBissau)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Guyana", QScriptValue(engine, static_cast<int>(QLocale::Guyana)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Haiti", QScriptValue(engine, static_cast<int>(QLocale::Haiti)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("HeardAndMcDonaldIslands", QScriptValue(engine, static_cast<int>(QLocale::HeardAndMcDonaldIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Honduras", QScriptValue(engine, static_cast<int>(QLocale::Honduras)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("HongKong", QScriptValue(engine, static_cast<int>(QLocale::HongKong)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Hungary", QScriptValue(engine, static_cast<int>(QLocale::Hungary)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Iceland", QScriptValue(engine, static_cast<int>(QLocale::Iceland)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("India", QScriptValue(engine, static_cast<int>(QLocale::India)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Indonesia", QScriptValue(engine, static_cast<int>(QLocale::Indonesia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Iran", QScriptValue(engine, static_cast<int>(QLocale::Iran)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Iraq", QScriptValue(engine, static_cast<int>(QLocale::Iraq)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Ireland", QScriptValue(engine, static_cast<int>(QLocale::Ireland)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Israel", QScriptValue(engine, static_cast<int>(QLocale::Israel)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Italy", QScriptValue(engine, static_cast<int>(QLocale::Italy)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Jamaica", QScriptValue(engine, static_cast<int>(QLocale::Jamaica)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Japan", QScriptValue(engine, static_cast<int>(QLocale::Japan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Jordan", QScriptValue(engine, static_cast<int>(QLocale::Jordan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Kazakhstan", QScriptValue(engine, static_cast<int>(QLocale::Kazakhstan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Kenya", QScriptValue(engine, static_cast<int>(QLocale::Kenya)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Kiribati", QScriptValue(engine, static_cast<int>(QLocale::Kiribati)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DemocraticRepublicOfKorea", QScriptValue(engine, static_cast<int>(QLocale::DemocraticRepublicOfKorea)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RepublicOfKorea", QScriptValue(engine, static_cast<int>(QLocale::RepublicOfKorea)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Kuwait", QScriptValue(engine, static_cast<int>(QLocale::Kuwait)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Kyrgyzstan", QScriptValue(engine, static_cast<int>(QLocale::Kyrgyzstan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Lao", QScriptValue(engine, static_cast<int>(QLocale::Lao)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Latvia", QScriptValue(engine, static_cast<int>(QLocale::Latvia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Lebanon", QScriptValue(engine, static_cast<int>(QLocale::Lebanon)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Lesotho", QScriptValue(engine, static_cast<int>(QLocale::Lesotho)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Liberia", QScriptValue(engine, static_cast<int>(QLocale::Liberia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LibyanArabJamahiriya", QScriptValue(engine, static_cast<int>(QLocale::LibyanArabJamahiriya)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Liechtenstein", QScriptValue(engine, static_cast<int>(QLocale::Liechtenstein)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Lithuania", QScriptValue(engine, static_cast<int>(QLocale::Lithuania)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Luxembourg", QScriptValue(engine, static_cast<int>(QLocale::Luxembourg)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Macau", QScriptValue(engine, static_cast<int>(QLocale::Macau)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Macedonia", QScriptValue(engine, static_cast<int>(QLocale::Macedonia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Madagascar", QScriptValue(engine, static_cast<int>(QLocale::Madagascar)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Malawi", QScriptValue(engine, static_cast<int>(QLocale::Malawi)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Malaysia", QScriptValue(engine, static_cast<int>(QLocale::Malaysia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Maldives", QScriptValue(engine, static_cast<int>(QLocale::Maldives)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Mali", QScriptValue(engine, static_cast<int>(QLocale::Mali)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Malta", QScriptValue(engine, static_cast<int>(QLocale::Malta)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MarshallIslands", QScriptValue(engine, static_cast<int>(QLocale::MarshallIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Martinique", QScriptValue(engine, static_cast<int>(QLocale::Martinique)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Mauritania", QScriptValue(engine, static_cast<int>(QLocale::Mauritania)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Mauritius", QScriptValue(engine, static_cast<int>(QLocale::Mauritius)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Mayotte", QScriptValue(engine, static_cast<int>(QLocale::Mayotte)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Mexico", QScriptValue(engine, static_cast<int>(QLocale::Mexico)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Micronesia", QScriptValue(engine, static_cast<int>(QLocale::Micronesia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Moldova", QScriptValue(engine, static_cast<int>(QLocale::Moldova)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Monaco", QScriptValue(engine, static_cast<int>(QLocale::Monaco)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Mongolia", QScriptValue(engine, static_cast<int>(QLocale::Mongolia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Montserrat", QScriptValue(engine, static_cast<int>(QLocale::Montserrat)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Morocco", QScriptValue(engine, static_cast<int>(QLocale::Morocco)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Mozambique", QScriptValue(engine, static_cast<int>(QLocale::Mozambique)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Myanmar", QScriptValue(engine, static_cast<int>(QLocale::Myanmar)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Namibia", QScriptValue(engine, static_cast<int>(QLocale::Namibia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NauruCountry", QScriptValue(engine, static_cast<int>(QLocale::NauruCountry)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Nepal", QScriptValue(engine, static_cast<int>(QLocale::Nepal)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Netherlands", QScriptValue(engine, static_cast<int>(QLocale::Netherlands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NetherlandsAntilles", QScriptValue(engine, static_cast<int>(QLocale::NetherlandsAntilles)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NewCaledonia", QScriptValue(engine, static_cast<int>(QLocale::NewCaledonia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NewZealand", QScriptValue(engine, static_cast<int>(QLocale::NewZealand)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Nicaragua", QScriptValue(engine, static_cast<int>(QLocale::Nicaragua)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Niger", QScriptValue(engine, static_cast<int>(QLocale::Niger)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Nigeria", QScriptValue(engine, static_cast<int>(QLocale::Nigeria)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Niue", QScriptValue(engine, static_cast<int>(QLocale::Niue)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NorfolkIsland", QScriptValue(engine, static_cast<int>(QLocale::NorfolkIsland)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NorthernMarianaIslands", QScriptValue(engine, static_cast<int>(QLocale::NorthernMarianaIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Norway", QScriptValue(engine, static_cast<int>(QLocale::Norway)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Oman", QScriptValue(engine, static_cast<int>(QLocale::Oman)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Pakistan", QScriptValue(engine, static_cast<int>(QLocale::Pakistan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Palau", QScriptValue(engine, static_cast<int>(QLocale::Palau)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("PalestinianTerritory", QScriptValue(engine, static_cast<int>(QLocale::PalestinianTerritory)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Panama", QScriptValue(engine, static_cast<int>(QLocale::Panama)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("PapuaNewGuinea", QScriptValue(engine, static_cast<int>(QLocale::PapuaNewGuinea)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Paraguay", QScriptValue(engine, static_cast<int>(QLocale::Paraguay)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Peru", QScriptValue(engine, static_cast<int>(QLocale::Peru)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Philippines", QScriptValue(engine, static_cast<int>(QLocale::Philippines)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Pitcairn", QScriptValue(engine, static_cast<int>(QLocale::Pitcairn)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Poland", QScriptValue(engine, static_cast<int>(QLocale::Poland)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Portugal", QScriptValue(engine, static_cast<int>(QLocale::Portugal)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("PuertoRico", QScriptValue(engine, static_cast<int>(QLocale::PuertoRico)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Qatar", QScriptValue(engine, static_cast<int>(QLocale::Qatar)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Reunion", QScriptValue(engine, static_cast<int>(QLocale::Reunion)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Romania", QScriptValue(engine, static_cast<int>(QLocale::Romania)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RussianFederation", QScriptValue(engine, static_cast<int>(QLocale::RussianFederation)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Rwanda", QScriptValue(engine, static_cast<int>(QLocale::Rwanda)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SaintKittsAndNevis", QScriptValue(engine, static_cast<int>(QLocale::SaintKittsAndNevis)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("StLucia", QScriptValue(engine, static_cast<int>(QLocale::StLucia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("StVincentAndTheGrenadines", QScriptValue(engine, static_cast<int>(QLocale::StVincentAndTheGrenadines)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Samoa", QScriptValue(engine, static_cast<int>(QLocale::Samoa)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SanMarino", QScriptValue(engine, static_cast<int>(QLocale::SanMarino)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SaoTomeAndPrincipe", QScriptValue(engine, static_cast<int>(QLocale::SaoTomeAndPrincipe)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SaudiArabia", QScriptValue(engine, static_cast<int>(QLocale::SaudiArabia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Senegal", QScriptValue(engine, static_cast<int>(QLocale::Senegal)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Seychelles", QScriptValue(engine, static_cast<int>(QLocale::Seychelles)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SierraLeone", QScriptValue(engine, static_cast<int>(QLocale::SierraLeone)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Singapore", QScriptValue(engine, static_cast<int>(QLocale::Singapore)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Slovakia", QScriptValue(engine, static_cast<int>(QLocale::Slovakia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Slovenia", QScriptValue(engine, static_cast<int>(QLocale::Slovenia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SolomonIslands", QScriptValue(engine, static_cast<int>(QLocale::SolomonIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Somalia", QScriptValue(engine, static_cast<int>(QLocale::Somalia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SouthAfrica", QScriptValue(engine, static_cast<int>(QLocale::SouthAfrica)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SouthGeorgiaAndTheSouthSandwichIslands", QScriptValue(engine, static_cast<int>(QLocale::SouthGeorgiaAndTheSouthSandwichIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Spain", QScriptValue(engine, static_cast<int>(QLocale::Spain)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SriLanka", QScriptValue(engine, static_cast<int>(QLocale::SriLanka)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("StHelena", QScriptValue(engine, static_cast<int>(QLocale::StHelena)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("StPierreAndMiquelon", QScriptValue(engine, static_cast<int>(QLocale::StPierreAndMiquelon)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Sudan", QScriptValue(engine, static_cast<int>(QLocale::Sudan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Suriname", QScriptValue(engine, static_cast<int>(QLocale::Suriname)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SvalbardAndJanMayenIslands", QScriptValue(engine, static_cast<int>(QLocale::SvalbardAndJanMayenIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Swaziland", QScriptValue(engine, static_cast<int>(QLocale::Swaziland)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Sweden", QScriptValue(engine, static_cast<int>(QLocale::Sweden)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Switzerland", QScriptValue(engine, static_cast<int>(QLocale::Switzerland)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SyrianArabRepublic", QScriptValue(engine, static_cast<int>(QLocale::SyrianArabRepublic)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Taiwan", QScriptValue(engine, static_cast<int>(QLocale::Taiwan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tajikistan", QScriptValue(engine, static_cast<int>(QLocale::Tajikistan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tanzania", QScriptValue(engine, static_cast<int>(QLocale::Tanzania)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Thailand", QScriptValue(engine, static_cast<int>(QLocale::Thailand)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Togo", QScriptValue(engine, static_cast<int>(QLocale::Togo)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tokelau", QScriptValue(engine, static_cast<int>(QLocale::Tokelau)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TongaCountry", QScriptValue(engine, static_cast<int>(QLocale::TongaCountry)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TrinidadAndTobago", QScriptValue(engine, static_cast<int>(QLocale::TrinidadAndTobago)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tunisia", QScriptValue(engine, static_cast<int>(QLocale::Tunisia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Turkey", QScriptValue(engine, static_cast<int>(QLocale::Turkey)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Turkmenistan", QScriptValue(engine, static_cast<int>(QLocale::Turkmenistan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TurksAndCaicosIslands", QScriptValue(engine, static_cast<int>(QLocale::TurksAndCaicosIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tuvalu", QScriptValue(engine, static_cast<int>(QLocale::Tuvalu)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Uganda", QScriptValue(engine, static_cast<int>(QLocale::Uganda)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Ukraine", QScriptValue(engine, static_cast<int>(QLocale::Ukraine)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("UnitedArabEmirates", QScriptValue(engine, static_cast<int>(QLocale::UnitedArabEmirates)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("UnitedKingdom", QScriptValue(engine, static_cast<int>(QLocale::UnitedKingdom)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("UnitedStates", QScriptValue(engine, static_cast<int>(QLocale::UnitedStates)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("UnitedStatesMinorOutlyingIslands", QScriptValue(engine, static_cast<int>(QLocale::UnitedStatesMinorOutlyingIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Uruguay", QScriptValue(engine, static_cast<int>(QLocale::Uruguay)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Uzbekistan", QScriptValue(engine, static_cast<int>(QLocale::Uzbekistan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Vanuatu", QScriptValue(engine, static_cast<int>(QLocale::Vanuatu)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("VaticanCityState", QScriptValue(engine, static_cast<int>(QLocale::VaticanCityState)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Venezuela", QScriptValue(engine, static_cast<int>(QLocale::Venezuela)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("VietNam", QScriptValue(engine, static_cast<int>(QLocale::VietNam)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("BritishVirginIslands", QScriptValue(engine, static_cast<int>(QLocale::BritishVirginIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("USVirginIslands", QScriptValue(engine, static_cast<int>(QLocale::USVirginIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WallisAndFutunaIslands", QScriptValue(engine, static_cast<int>(QLocale::WallisAndFutunaIslands)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WesternSahara", QScriptValue(engine, static_cast<int>(QLocale::WesternSahara)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Yemen", QScriptValue(engine, static_cast<int>(QLocale::Yemen)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Yugoslavia", QScriptValue(engine, static_cast<int>(QLocale::Yugoslavia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Zambia", QScriptValue(engine, static_cast<int>(QLocale::Zambia)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Zimbabwe", QScriptValue(engine, static_cast<int>(QLocale::Zimbabwe)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SerbiaAndMontenegro", QScriptValue(engine, static_cast<int>(QLocale::SerbiaAndMontenegro)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LastCountry", QScriptValue(engine, static_cast<int>(QLocale::LastCountry)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QLocale::Language>(engine);
    cv.setProperty("C", QScriptValue(engine, static_cast<int>(QLocale::C)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Abkhazian", QScriptValue(engine, static_cast<int>(QLocale::Abkhazian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Afan", QScriptValue(engine, static_cast<int>(QLocale::Afan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Afar", QScriptValue(engine, static_cast<int>(QLocale::Afar)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Afrikaans", QScriptValue(engine, static_cast<int>(QLocale::Afrikaans)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Albanian", QScriptValue(engine, static_cast<int>(QLocale::Albanian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Amharic", QScriptValue(engine, static_cast<int>(QLocale::Amharic)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Arabic", QScriptValue(engine, static_cast<int>(QLocale::Arabic)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Armenian", QScriptValue(engine, static_cast<int>(QLocale::Armenian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Assamese", QScriptValue(engine, static_cast<int>(QLocale::Assamese)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Aymara", QScriptValue(engine, static_cast<int>(QLocale::Aymara)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Azerbaijani", QScriptValue(engine, static_cast<int>(QLocale::Azerbaijani)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bashkir", QScriptValue(engine, static_cast<int>(QLocale::Bashkir)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Basque", QScriptValue(engine, static_cast<int>(QLocale::Basque)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bengali", QScriptValue(engine, static_cast<int>(QLocale::Bengali)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bhutani", QScriptValue(engine, static_cast<int>(QLocale::Bhutani)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bihari", QScriptValue(engine, static_cast<int>(QLocale::Bihari)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bislama", QScriptValue(engine, static_cast<int>(QLocale::Bislama)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Breton", QScriptValue(engine, static_cast<int>(QLocale::Breton)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bulgarian", QScriptValue(engine, static_cast<int>(QLocale::Bulgarian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Burmese", QScriptValue(engine, static_cast<int>(QLocale::Burmese)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Byelorussian", QScriptValue(engine, static_cast<int>(QLocale::Byelorussian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Cambodian", QScriptValue(engine, static_cast<int>(QLocale::Cambodian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Catalan", QScriptValue(engine, static_cast<int>(QLocale::Catalan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Chinese", QScriptValue(engine, static_cast<int>(QLocale::Chinese)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Corsican", QScriptValue(engine, static_cast<int>(QLocale::Corsican)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Croatian", QScriptValue(engine, static_cast<int>(QLocale::Croatian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Czech", QScriptValue(engine, static_cast<int>(QLocale::Czech)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Danish", QScriptValue(engine, static_cast<int>(QLocale::Danish)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Dutch", QScriptValue(engine, static_cast<int>(QLocale::Dutch)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("English", QScriptValue(engine, static_cast<int>(QLocale::English)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Esperanto", QScriptValue(engine, static_cast<int>(QLocale::Esperanto)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Estonian", QScriptValue(engine, static_cast<int>(QLocale::Estonian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Faroese", QScriptValue(engine, static_cast<int>(QLocale::Faroese)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FijiLanguage", QScriptValue(engine, static_cast<int>(QLocale::FijiLanguage)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Finnish", QScriptValue(engine, static_cast<int>(QLocale::Finnish)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("French", QScriptValue(engine, static_cast<int>(QLocale::French)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Frisian", QScriptValue(engine, static_cast<int>(QLocale::Frisian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Gaelic", QScriptValue(engine, static_cast<int>(QLocale::Gaelic)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Galician", QScriptValue(engine, static_cast<int>(QLocale::Galician)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Georgian", QScriptValue(engine, static_cast<int>(QLocale::Georgian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("German", QScriptValue(engine, static_cast<int>(QLocale::German)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Greek", QScriptValue(engine, static_cast<int>(QLocale::Greek)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Greenlandic", QScriptValue(engine, static_cast<int>(QLocale::Greenlandic)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Guarani", QScriptValue(engine, static_cast<int>(QLocale::Guarani)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Gujarati", QScriptValue(engine, static_cast<int>(QLocale::Gujarati)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Hausa", QScriptValue(engine, static_cast<int>(QLocale::Hausa)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Hebrew", QScriptValue(engine, static_cast<int>(QLocale::Hebrew)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Hindi", QScriptValue(engine, static_cast<int>(QLocale::Hindi)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Hungarian", QScriptValue(engine, static_cast<int>(QLocale::Hungarian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Icelandic", QScriptValue(engine, static_cast<int>(QLocale::Icelandic)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Indonesian", QScriptValue(engine, static_cast<int>(QLocale::Indonesian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Interlingua", QScriptValue(engine, static_cast<int>(QLocale::Interlingua)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Interlingue", QScriptValue(engine, static_cast<int>(QLocale::Interlingue)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Inuktitut", QScriptValue(engine, static_cast<int>(QLocale::Inuktitut)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Inupiak", QScriptValue(engine, static_cast<int>(QLocale::Inupiak)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Irish", QScriptValue(engine, static_cast<int>(QLocale::Irish)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Italian", QScriptValue(engine, static_cast<int>(QLocale::Italian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Japanese", QScriptValue(engine, static_cast<int>(QLocale::Japanese)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Javanese", QScriptValue(engine, static_cast<int>(QLocale::Javanese)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Kannada", QScriptValue(engine, static_cast<int>(QLocale::Kannada)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Kashmiri", QScriptValue(engine, static_cast<int>(QLocale::Kashmiri)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Kazakh", QScriptValue(engine, static_cast<int>(QLocale::Kazakh)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Kinyarwanda", QScriptValue(engine, static_cast<int>(QLocale::Kinyarwanda)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Kirghiz", QScriptValue(engine, static_cast<int>(QLocale::Kirghiz)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Korean", QScriptValue(engine, static_cast<int>(QLocale::Korean)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Kurdish", QScriptValue(engine, static_cast<int>(QLocale::Kurdish)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Kurundi", QScriptValue(engine, static_cast<int>(QLocale::Kurundi)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Laothian", QScriptValue(engine, static_cast<int>(QLocale::Laothian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Latin", QScriptValue(engine, static_cast<int>(QLocale::Latin)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Latvian", QScriptValue(engine, static_cast<int>(QLocale::Latvian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Lingala", QScriptValue(engine, static_cast<int>(QLocale::Lingala)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Lithuanian", QScriptValue(engine, static_cast<int>(QLocale::Lithuanian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Macedonian", QScriptValue(engine, static_cast<int>(QLocale::Macedonian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Malagasy", QScriptValue(engine, static_cast<int>(QLocale::Malagasy)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Malay", QScriptValue(engine, static_cast<int>(QLocale::Malay)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Malayalam", QScriptValue(engine, static_cast<int>(QLocale::Malayalam)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Maltese", QScriptValue(engine, static_cast<int>(QLocale::Maltese)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Maori", QScriptValue(engine, static_cast<int>(QLocale::Maori)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Marathi", QScriptValue(engine, static_cast<int>(QLocale::Marathi)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Moldavian", QScriptValue(engine, static_cast<int>(QLocale::Moldavian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Mongolian", QScriptValue(engine, static_cast<int>(QLocale::Mongolian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NauruLanguage", QScriptValue(engine, static_cast<int>(QLocale::NauruLanguage)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Nepali", QScriptValue(engine, static_cast<int>(QLocale::Nepali)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Norwegian", QScriptValue(engine, static_cast<int>(QLocale::Norwegian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NorwegianBokmal", QScriptValue(engine, static_cast<int>(QLocale::NorwegianBokmal)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Occitan", QScriptValue(engine, static_cast<int>(QLocale::Occitan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Oriya", QScriptValue(engine, static_cast<int>(QLocale::Oriya)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Pashto", QScriptValue(engine, static_cast<int>(QLocale::Pashto)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Persian", QScriptValue(engine, static_cast<int>(QLocale::Persian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Polish", QScriptValue(engine, static_cast<int>(QLocale::Polish)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Portuguese", QScriptValue(engine, static_cast<int>(QLocale::Portuguese)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Punjabi", QScriptValue(engine, static_cast<int>(QLocale::Punjabi)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Quechua", QScriptValue(engine, static_cast<int>(QLocale::Quechua)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RhaetoRomance", QScriptValue(engine, static_cast<int>(QLocale::RhaetoRomance)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Romanian", QScriptValue(engine, static_cast<int>(QLocale::Romanian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Russian", QScriptValue(engine, static_cast<int>(QLocale::Russian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Samoan", QScriptValue(engine, static_cast<int>(QLocale::Samoan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Sangho", QScriptValue(engine, static_cast<int>(QLocale::Sangho)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Sanskrit", QScriptValue(engine, static_cast<int>(QLocale::Sanskrit)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Serbian", QScriptValue(engine, static_cast<int>(QLocale::Serbian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SerboCroatian", QScriptValue(engine, static_cast<int>(QLocale::SerboCroatian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Sesotho", QScriptValue(engine, static_cast<int>(QLocale::Sesotho)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Setswana", QScriptValue(engine, static_cast<int>(QLocale::Setswana)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Shona", QScriptValue(engine, static_cast<int>(QLocale::Shona)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Sindhi", QScriptValue(engine, static_cast<int>(QLocale::Sindhi)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Singhalese", QScriptValue(engine, static_cast<int>(QLocale::Singhalese)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Siswati", QScriptValue(engine, static_cast<int>(QLocale::Siswati)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Slovak", QScriptValue(engine, static_cast<int>(QLocale::Slovak)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Slovenian", QScriptValue(engine, static_cast<int>(QLocale::Slovenian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Somali", QScriptValue(engine, static_cast<int>(QLocale::Somali)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Spanish", QScriptValue(engine, static_cast<int>(QLocale::Spanish)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Sundanese", QScriptValue(engine, static_cast<int>(QLocale::Sundanese)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Swahili", QScriptValue(engine, static_cast<int>(QLocale::Swahili)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Swedish", QScriptValue(engine, static_cast<int>(QLocale::Swedish)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tagalog", QScriptValue(engine, static_cast<int>(QLocale::Tagalog)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tajik", QScriptValue(engine, static_cast<int>(QLocale::Tajik)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tamil", QScriptValue(engine, static_cast<int>(QLocale::Tamil)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tatar", QScriptValue(engine, static_cast<int>(QLocale::Tatar)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Telugu", QScriptValue(engine, static_cast<int>(QLocale::Telugu)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Thai", QScriptValue(engine, static_cast<int>(QLocale::Thai)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tibetan", QScriptValue(engine, static_cast<int>(QLocale::Tibetan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tigrinya", QScriptValue(engine, static_cast<int>(QLocale::Tigrinya)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TongaLanguage", QScriptValue(engine, static_cast<int>(QLocale::TongaLanguage)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tsonga", QScriptValue(engine, static_cast<int>(QLocale::Tsonga)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Turkish", QScriptValue(engine, static_cast<int>(QLocale::Turkish)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Turkmen", QScriptValue(engine, static_cast<int>(QLocale::Turkmen)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Twi", QScriptValue(engine, static_cast<int>(QLocale::Twi)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Uigur", QScriptValue(engine, static_cast<int>(QLocale::Uigur)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Ukrainian", QScriptValue(engine, static_cast<int>(QLocale::Ukrainian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Urdu", QScriptValue(engine, static_cast<int>(QLocale::Urdu)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Uzbek", QScriptValue(engine, static_cast<int>(QLocale::Uzbek)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Vietnamese", QScriptValue(engine, static_cast<int>(QLocale::Vietnamese)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Volapuk", QScriptValue(engine, static_cast<int>(QLocale::Volapuk)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Welsh", QScriptValue(engine, static_cast<int>(QLocale::Welsh)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Wolof", QScriptValue(engine, static_cast<int>(QLocale::Wolof)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Xhosa", QScriptValue(engine, static_cast<int>(QLocale::Xhosa)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Yiddish", QScriptValue(engine, static_cast<int>(QLocale::Yiddish)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Yoruba", QScriptValue(engine, static_cast<int>(QLocale::Yoruba)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Zhuang", QScriptValue(engine, static_cast<int>(QLocale::Zhuang)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Zulu", QScriptValue(engine, static_cast<int>(QLocale::Zulu)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NorwegianNynorsk", QScriptValue(engine, static_cast<int>(QLocale::NorwegianNynorsk)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Nynorsk", QScriptValue(engine, static_cast<int>(QLocale::Nynorsk)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Bosnian", QScriptValue(engine, static_cast<int>(QLocale::Bosnian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Divehi", QScriptValue(engine, static_cast<int>(QLocale::Divehi)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Manx", QScriptValue(engine, static_cast<int>(QLocale::Manx)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Cornish", QScriptValue(engine, static_cast<int>(QLocale::Cornish)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Akan", QScriptValue(engine, static_cast<int>(QLocale::Akan)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Konkani", QScriptValue(engine, static_cast<int>(QLocale::Konkani)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Ga", QScriptValue(engine, static_cast<int>(QLocale::Ga)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Igbo", QScriptValue(engine, static_cast<int>(QLocale::Igbo)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Kamba", QScriptValue(engine, static_cast<int>(QLocale::Kamba)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Syriac", QScriptValue(engine, static_cast<int>(QLocale::Syriac)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Blin", QScriptValue(engine, static_cast<int>(QLocale::Blin)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Geez", QScriptValue(engine, static_cast<int>(QLocale::Geez)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Koro", QScriptValue(engine, static_cast<int>(QLocale::Koro)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Sidamo", QScriptValue(engine, static_cast<int>(QLocale::Sidamo)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Atsam", QScriptValue(engine, static_cast<int>(QLocale::Atsam)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tigre", QScriptValue(engine, static_cast<int>(QLocale::Tigre)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Jju", QScriptValue(engine, static_cast<int>(QLocale::Jju)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Friulian", QScriptValue(engine, static_cast<int>(QLocale::Friulian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Venda", QScriptValue(engine, static_cast<int>(QLocale::Venda)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Ewe", QScriptValue(engine, static_cast<int>(QLocale::Ewe)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Walamo", QScriptValue(engine, static_cast<int>(QLocale::Walamo)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Hawaiian", QScriptValue(engine, static_cast<int>(QLocale::Hawaiian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Tyap", QScriptValue(engine, static_cast<int>(QLocale::Tyap)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Chewa", QScriptValue(engine, static_cast<int>(QLocale::Chewa)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LastLanguage", QScriptValue(engine, static_cast<int>(QLocale::LastLanguage)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QLocale::NumberOptions>(engine);
    return cv;
}

// QRect

Q_DECLARE_METATYPE(QRect)
Q_DECLARE_METATYPE(QRect*)

class Constructor_QRect:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(int left, int top, int width, int height)
{ return this->engine()->toScriptValue(QRect(left, top, width, height)); }
QScriptValue qscript_call(QPoint const& topleft, QSize const& size)
{ return this->engine()->toScriptValue(QRect(topleft, size)); }
QScriptValue qscript_call(QPoint const& topleft, QPoint const& bottomright)
{ return this->engine()->toScriptValue(QRect(topleft, bottomright)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QRect()); }
};

class Prototype_QRect:
public QObject, public QRect, protected QScriptable
{
    Q_OBJECT
private:
inline QRect *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QRect*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QRect.prototype.%0: this object is not a QRect")
            .arg(method));
}
public:
Prototype_QRect(QObject *parent = 0)
    : QRect() { setParent(parent); }
public Q_SLOTS:
QPoint bottomRight() const
{ if (QRect *_q_q = this->self()) return _q_q->bottomRight();
  throwTypeError(QLatin1String("bottomRight")); return QPoint(); }
void setTopLeft(QPoint const& p)
{ if (QRect *_q_q = this->self()) _q_q->setTopLeft(p);
  else throwTypeError(QLatin1String("setTopLeft")); }
void moveCenter(QPoint const& p)
{ if (QRect *_q_q = this->self()) _q_q->moveCenter(p);
  else throwTypeError(QLatin1String("moveCenter")); }
QPoint topLeft() const
{ if (QRect *_q_q = this->self()) return _q_q->topLeft();
  throwTypeError(QLatin1String("topLeft")); return QPoint(); }
int top() const
{ if (QRect *_q_q = this->self()) return _q_q->top();
  throwTypeError(QLatin1String("top")); return int(); }
int width() const
{ if (QRect *_q_q = this->self()) return _q_q->width();
  throwTypeError(QLatin1String("width")); return int(); }
QRect intersect(QRect const& r) const
{ if (QRect *_q_q = this->self()) return _q_q->intersect(r);
  throwTypeError(QLatin1String("intersect")); return QRect(); }
void moveBottom(int pos)
{ if (QRect *_q_q = this->self()) _q_q->moveBottom(pos);
  else throwTypeError(QLatin1String("moveBottom")); }
QRect translated(QPoint const& p) const
{ if (QRect *_q_q = this->self()) return _q_q->translated(p);
  throwTypeError(QLatin1String("translated")); return QRect(); }
QRect translated(int dx, int dy) const
{ if (QRect *_q_q = this->self()) return _q_q->translated(dx, dy);
  throwTypeError(QLatin1String("translated")); return QRect(); }
void setTopRight(QPoint const& p)
{ if (QRect *_q_q = this->self()) _q_q->setTopRight(p);
  else throwTypeError(QLatin1String("setTopRight")); }
void getRect(int* x, int* y, int* w, int* h) const
{ if (QRect *_q_q = this->self()) _q_q->getRect(x, y, w, h);
  else throwTypeError(QLatin1String("getRect")); }
void setWidth(int w)
{ if (QRect *_q_q = this->self()) _q_q->setWidth(w);
  else throwTypeError(QLatin1String("setWidth")); }
void moveTop(int pos)
{ if (QRect *_q_q = this->self()) _q_q->moveTop(pos);
  else throwTypeError(QLatin1String("moveTop")); }
int right() const
{ if (QRect *_q_q = this->self()) return _q_q->right();
  throwTypeError(QLatin1String("right")); return int(); }
QRect normalized() const
{ if (QRect *_q_q = this->self()) return _q_q->normalized();
  throwTypeError(QLatin1String("normalized")); return QRect(); }
QPoint topRight() const
{ if (QRect *_q_q = this->self()) return _q_q->topRight();
  throwTypeError(QLatin1String("topRight")); return QPoint(); }
QPoint center() const
{ if (QRect *_q_q = this->self()) return _q_q->center();
  throwTypeError(QLatin1String("center")); return QPoint(); }
void setBottom(int pos)
{ if (QRect *_q_q = this->self()) _q_q->setBottom(pos);
  else throwTypeError(QLatin1String("setBottom")); }
bool intersects(QRect const& r) const
{ if (QRect *_q_q = this->self()) return _q_q->intersects(r);
  throwTypeError(QLatin1String("intersects")); return bool(); }
QPoint bottomLeft() const
{ if (QRect *_q_q = this->self()) return _q_q->bottomLeft();
  throwTypeError(QLatin1String("bottomLeft")); return QPoint(); }
int bottom() const
{ if (QRect *_q_q = this->self()) return _q_q->bottom();
  throwTypeError(QLatin1String("bottom")); return int(); }
bool isEmpty() const
{ if (QRect *_q_q = this->self()) return _q_q->isEmpty();
  throwTypeError(QLatin1String("isEmpty")); return bool(); }
QRect united(QRect const& other) const
{ if (QRect *_q_q = this->self()) return _q_q->united(other);
  throwTypeError(QLatin1String("united")); return QRect(); }
void setRight(int pos)
{ if (QRect *_q_q = this->self()) _q_q->setRight(pos);
  else throwTypeError(QLatin1String("setRight")); }
void setRect(int x, int y, int w, int h)
{ if (QRect *_q_q = this->self()) _q_q->setRect(x, y, w, h);
  else throwTypeError(QLatin1String("setRect")); }
void moveLeft(int pos)
{ if (QRect *_q_q = this->self()) _q_q->moveLeft(pos);
  else throwTypeError(QLatin1String("moveLeft")); }
int left() const
{ if (QRect *_q_q = this->self()) return _q_q->left();
  throwTypeError(QLatin1String("left")); return int(); }
void moveBottomLeft(QPoint const& p)
{ if (QRect *_q_q = this->self()) _q_q->moveBottomLeft(p);
  else throwTypeError(QLatin1String("moveBottomLeft")); }
QSize size() const
{ if (QRect *_q_q = this->self()) return _q_q->size();
  throwTypeError(QLatin1String("size")); return QSize(); }
void translate(QPoint const& p)
{ if (QRect *_q_q = this->self()) _q_q->translate(p);
  else throwTypeError(QLatin1String("translate")); }
void translate(int dx, int dy)
{ if (QRect *_q_q = this->self()) _q_q->translate(dx, dy);
  else throwTypeError(QLatin1String("translate")); }
void moveBottomRight(QPoint const& p)
{ if (QRect *_q_q = this->self()) _q_q->moveBottomRight(p);
  else throwTypeError(QLatin1String("moveBottomRight")); }
QRect adjusted(int x1, int y1, int x2, int y2) const
{ if (QRect *_q_q = this->self()) return _q_q->adjusted(x1, y1, x2, y2);
  throwTypeError(QLatin1String("adjusted")); return QRect(); }
void getCoords(int* x1, int* y1, int* x2, int* y2) const
{ if (QRect *_q_q = this->self()) _q_q->getCoords(x1, y1, x2, y2);
  else throwTypeError(QLatin1String("getCoords")); }
bool contains(QRect const& r, bool proper = false) const
{ if (QRect *_q_q = this->self()) return _q_q->contains(r, proper);
  throwTypeError(QLatin1String("contains")); return bool(); }
bool contains(int x, int y, bool proper) const
{ if (QRect *_q_q = this->self()) return _q_q->contains(x, y, proper);
  throwTypeError(QLatin1String("contains")); return bool(); }
bool contains(int x, int y) const
{ if (QRect *_q_q = this->self()) return _q_q->contains(x, y);
  throwTypeError(QLatin1String("contains")); return bool(); }
bool contains(QPoint const& p, bool proper = false) const
{ if (QRect *_q_q = this->self()) return _q_q->contains(p, proper);
  throwTypeError(QLatin1String("contains")); return bool(); }
void setHeight(int h)
{ if (QRect *_q_q = this->self()) _q_q->setHeight(h);
  else throwTypeError(QLatin1String("setHeight")); }
void moveTopLeft(QPoint const& p)
{ if (QRect *_q_q = this->self()) _q_q->moveTopLeft(p);
  else throwTypeError(QLatin1String("moveTopLeft")); }
QRect unite(QRect const& r) const
{ if (QRect *_q_q = this->self()) return _q_q->unite(r);
  throwTypeError(QLatin1String("unite")); return QRect(); }
bool isValid() const
{ if (QRect *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
void setBottomRight(QPoint const& p)
{ if (QRect *_q_q = this->self()) _q_q->setBottomRight(p);
  else throwTypeError(QLatin1String("setBottomRight")); }
void moveTopRight(QPoint const& p)
{ if (QRect *_q_q = this->self()) _q_q->moveTopRight(p);
  else throwTypeError(QLatin1String("moveTopRight")); }
void setCoords(int x1, int y1, int x2, int y2)
{ if (QRect *_q_q = this->self()) _q_q->setCoords(x1, y1, x2, y2);
  else throwTypeError(QLatin1String("setCoords")); }
void setSize(QSize const& s)
{ if (QRect *_q_q = this->self()) _q_q->setSize(s);
  else throwTypeError(QLatin1String("setSize")); }
void moveTo(QPoint const& p)
{ if (QRect *_q_q = this->self()) _q_q->moveTo(p);
  else throwTypeError(QLatin1String("moveTo")); }
void moveTo(int x, int t)
{ if (QRect *_q_q = this->self()) _q_q->moveTo(x, t);
  else throwTypeError(QLatin1String("moveTo")); }
bool isNull() const
{ if (QRect *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
void moveRight(int pos)
{ if (QRect *_q_q = this->self()) _q_q->moveRight(pos);
  else throwTypeError(QLatin1String("moveRight")); }
void setLeft(int pos)
{ if (QRect *_q_q = this->self()) _q_q->setLeft(pos);
  else throwTypeError(QLatin1String("setLeft")); }
void adjust(int x1, int y1, int x2, int y2)
{ if (QRect *_q_q = this->self()) _q_q->adjust(x1, y1, x2, y2);
  else throwTypeError(QLatin1String("adjust")); }
void setBottomLeft(QPoint const& p)
{ if (QRect *_q_q = this->self()) _q_q->setBottomLeft(p);
  else throwTypeError(QLatin1String("setBottomLeft")); }
int x() const
{ if (QRect *_q_q = this->self()) return _q_q->x();
  throwTypeError(QLatin1String("x")); return int(); }
int y() const
{ if (QRect *_q_q = this->self()) return _q_q->y();
  throwTypeError(QLatin1String("y")); return int(); }
void setX(int x)
{ if (QRect *_q_q = this->self()) _q_q->setX(x);
  else throwTypeError(QLatin1String("setX")); }
QRect intersected(QRect const& other) const
{ if (QRect *_q_q = this->self()) return _q_q->intersected(other);
  throwTypeError(QLatin1String("intersected")); return QRect(); }
void setY(int y)
{ if (QRect *_q_q = this->self()) _q_q->setY(y);
  else throwTypeError(QLatin1String("setY")); }
void setTop(int pos)
{ if (QRect *_q_q = this->self()) _q_q->setTop(pos);
  else throwTypeError(QLatin1String("setTop")); }
int height() const
{ if (QRect *_q_q = this->self()) return _q_q->height();
  throwTypeError(QLatin1String("height")); return int(); }
QString toString() const
{ return QLatin1String("QRect"); }
};

static QScriptValue create_QRect_class(QScriptEngine *engine)
{
    Prototype_QRect *pq = new Prototype_QRect;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QRect *cq = new Constructor_QRect;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QRect>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QRect>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QRect*>(), pv);
    return cv;
}

// QPersistentModelIndex

Q_DECLARE_METATYPE(QPersistentModelIndex)
Q_DECLARE_METATYPE(QPersistentModelIndex*)

class Constructor_QPersistentModelIndex:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QPersistentModelIndex const& other)
{ return this->engine()->toScriptValue(QPersistentModelIndex(other)); }
QScriptValue qscript_call(QModelIndex const& index)
{ return this->engine()->toScriptValue(QPersistentModelIndex(index)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QPersistentModelIndex()); }
};

class Prototype_QPersistentModelIndex:
public QObject, public QPersistentModelIndex, protected QScriptable
{
    Q_OBJECT
private:
inline QPersistentModelIndex *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QPersistentModelIndex*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QPersistentModelIndex.prototype.%0: this object is not a QPersistentModelIndex")
            .arg(method));
}
public:
Prototype_QPersistentModelIndex(QObject *parent = 0)
    : QPersistentModelIndex() { setParent(parent); }
public Q_SLOTS:
QAbstractItemModel const* model() const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->model();
  throwTypeError(QLatin1String("model")); return 0; }
QModelIndex child(int row, int column) const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->child(row, column);
  throwTypeError(QLatin1String("child")); return QModelIndex(); }
bool isValid() const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
QModelIndex parent() const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->parent();
  throwTypeError(QLatin1String("parent")); return QModelIndex(); }
QVariant data(int role = Qt::DisplayRole) const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->data(role);
  throwTypeError(QLatin1String("data")); return QVariant(); }
bool lessThan(QPersistentModelIndex const& other) const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->operator<(other);
  throwTypeError(QLatin1String("lessThan")); return bool(); }
qint64 internalId() const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->internalId();
  throwTypeError(QLatin1String("internalId")); return qint64(); }
bool equals(QModelIndex const& other) const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->operator==(other);
  throwTypeError(QLatin1String("equals")); return bool(); }
bool equals(QPersistentModelIndex const& other) const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->operator==(other);
  throwTypeError(QLatin1String("equals")); return bool(); }
Qt::ItemFlags flags() const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->flags();
  throwTypeError(QLatin1String("flags")); return Qt::ItemFlags(); }
void* internalPointer() const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->internalPointer();
  throwTypeError(QLatin1String("internalPointer")); return 0; }
QModelIndex sibling(int row, int column) const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->sibling(row, column);
  throwTypeError(QLatin1String("sibling")); return QModelIndex(); }
int column() const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->column();
  throwTypeError(QLatin1String("column")); return int(); }
int row() const
{ if (QPersistentModelIndex *_q_q = this->self()) return _q_q->row();
  throwTypeError(QLatin1String("row")); return int(); }
QString toString() const
{ return QLatin1String("QPersistentModelIndex"); }
};

static QScriptValue create_QPersistentModelIndex_class(QScriptEngine *engine)
{
    Prototype_QPersistentModelIndex *pq = new Prototype_QPersistentModelIndex;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QPersistentModelIndex *cq = new Constructor_QPersistentModelIndex;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QPersistentModelIndex>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QPersistentModelIndex>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QPersistentModelIndex*>(), pv);
    return cv;
}

// QSemaphore

Q_DECLARE_METATYPE(QSemaphore*)

class Constructor_QSemaphore:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(int n = 0)
{ return this->engine()->toScriptValue(new QSemaphore(n)); }
};

class Prototype_QSemaphore:
public QObject, public QSemaphore, protected QScriptable
{
    Q_OBJECT
private:
inline QSemaphore *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QSemaphore*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QSemaphore.prototype.%0: this object is not a QSemaphore")
            .arg(method));
}
public:
Prototype_QSemaphore(QObject *parent = 0)
    : QSemaphore() { setParent(parent); }
public Q_SLOTS:
int available() const
{ if (QSemaphore *_q_q = this->self()) return _q_q->available();
  throwTypeError(QLatin1String("available")); return int(); }
void release(int n = 1)
{ if (QSemaphore *_q_q = this->self()) _q_q->release(n);
  else throwTypeError(QLatin1String("release")); }
void acquire(int n = 1)
{ if (QSemaphore *_q_q = this->self()) _q_q->acquire(n);
  else throwTypeError(QLatin1String("acquire")); }
bool tryAcquire(int n, int timeout)
{ if (QSemaphore *_q_q = this->self()) return _q_q->tryAcquire(n, timeout);
  throwTypeError(QLatin1String("tryAcquire")); return bool(); }
bool tryAcquire(int n = 1)
{ if (QSemaphore *_q_q = this->self()) return _q_q->tryAcquire(n);
  throwTypeError(QLatin1String("tryAcquire")); return bool(); }
QString toString() const
{ return QLatin1String("QSemaphore"); }
private:
Q_DISABLE_COPY(Prototype_QSemaphore)
};

static QScriptValue create_QSemaphore_class(QScriptEngine *engine)
{
    Prototype_QSemaphore *pq = new Prototype_QSemaphore;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QSemaphore *cq = new Constructor_QSemaphore;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QSemaphore*>(), pv);
    return cv;
}

// QMetaMethod

Q_DECLARE_METATYPE(QMetaMethod)
Q_DECLARE_METATYPE(QMetaMethod*)
Q_DECLARE_METATYPE(QMetaMethod::Access)
Q_DECLARE_METATYPE(QMetaMethod::Attributes)
Q_DECLARE_METATYPE(QMetaMethod::MethodType)

class Constructor_QMetaMethod:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QMetaMethod()); }
};

class Prototype_QMetaMethod:
public QObject, public QMetaMethod, protected QScriptable
{
    Q_OBJECT
private:
inline QMetaMethod *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QMetaMethod*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QMetaMethod.prototype.%0: this object is not a QMetaMethod")
            .arg(method));
}
public:
Prototype_QMetaMethod(QObject *parent = 0)
    : QMetaMethod() { setParent(parent); }
public Q_SLOTS:
QList<QByteArray> parameterNames() const
{ if (QMetaMethod *_q_q = this->self()) return _q_q->parameterNames();
  throwTypeError(QLatin1String("parameterNames")); return QList<QByteArray>(); }
char const* tag() const
{ if (QMetaMethod *_q_q = this->self()) return _q_q->tag();
  throwTypeError(QLatin1String("tag")); return 0; }
QMetaMethod::Access access() const
{ if (QMetaMethod *_q_q = this->self()) return _q_q->access();
  throwTypeError(QLatin1String("access")); return QMetaMethod::Access(); }
char const* typeName() const
{ if (QMetaMethod *_q_q = this->self()) return _q_q->typeName();
  throwTypeError(QLatin1String("typeName")); return 0; }
int attributes() const
{ if (QMetaMethod *_q_q = this->self()) return _q_q->attributes();
  throwTypeError(QLatin1String("attributes")); return int(); }
char const* signature() const
{ if (QMetaMethod *_q_q = this->self()) return _q_q->signature();
  throwTypeError(QLatin1String("signature")); return 0; }
QMetaMethod::MethodType methodType() const
{ if (QMetaMethod *_q_q = this->self()) return _q_q->methodType();
  throwTypeError(QLatin1String("methodType")); return QMetaMethod::MethodType(); }
QList<QByteArray> parameterTypes() const
{ if (QMetaMethod *_q_q = this->self()) return _q_q->parameterTypes();
  throwTypeError(QLatin1String("parameterTypes")); return QList<QByteArray>(); }
QString toString() const
{ return QLatin1String("QMetaMethod"); }
};

static QScriptValue create_QMetaMethod_class(QScriptEngine *engine)
{
    Prototype_QMetaMethod *pq = new Prototype_QMetaMethod;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QMetaMethod *cq = new Constructor_QMetaMethod;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QMetaMethod>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QMetaMethod>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QMetaMethod*>(), pv);
    _q_ScriptRegisterEnumMetaType<QMetaMethod::Access>(engine);
    cv.setProperty("Private", QScriptValue(engine, static_cast<int>(QMetaMethod::Private)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Protected", QScriptValue(engine, static_cast<int>(QMetaMethod::Protected)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Public", QScriptValue(engine, static_cast<int>(QMetaMethod::Public)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QMetaMethod::Attributes>(engine);
    cv.setProperty("Compatibility", QScriptValue(engine, static_cast<int>(QMetaMethod::Compatibility)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Cloned", QScriptValue(engine, static_cast<int>(QMetaMethod::Cloned)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Scriptable", QScriptValue(engine, static_cast<int>(QMetaMethod::Scriptable)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QMetaMethod::MethodType>(engine);
    cv.setProperty("Method", QScriptValue(engine, static_cast<int>(QMetaMethod::Method)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Signal", QScriptValue(engine, static_cast<int>(QMetaMethod::Signal)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Slot", QScriptValue(engine, static_cast<int>(QMetaMethod::Slot)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QTextEncoder

Q_DECLARE_METATYPE(QTextEncoder*)

class Constructor_QTextEncoder:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QTextCodec const* codec)
{ return this->engine()->toScriptValue(new QTextEncoder(codec)); }
};

class Prototype_QTextEncoder:
public QObject, public QTextEncoder, protected QScriptable
{
    Q_OBJECT
private:
inline QTextEncoder *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QTextEncoder*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QTextEncoder.prototype.%0: this object is not a QTextEncoder")
            .arg(method));
}
public:
Prototype_QTextEncoder(QObject *parent = 0)
    : QTextEncoder(0) { setParent(parent); }
public Q_SLOTS:
QByteArray fromUnicode(QChar const* uc, int len)
{ if (QTextEncoder *_q_q = this->self()) return _q_q->fromUnicode(uc, len);
  throwTypeError(QLatin1String("fromUnicode")); return QByteArray(); }
QByteArray fromUnicode(QString const& str)
{ if (QTextEncoder *_q_q = this->self()) return _q_q->fromUnicode(str);
  throwTypeError(QLatin1String("fromUnicode")); return QByteArray(); }
QString toString() const
{ return QLatin1String("QTextEncoder"); }
private:
Q_DISABLE_COPY(Prototype_QTextEncoder)
};

static QScriptValue create_QTextEncoder_class(QScriptEngine *engine)
{
    Prototype_QTextEncoder *pq = new Prototype_QTextEncoder;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QTextEncoder *cq = new Constructor_QTextEncoder;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QTextEncoder*>(), pv);
    return cv;
}

// QSizeF

Q_DECLARE_METATYPE(QSizeF)
Q_DECLARE_METATYPE(QSizeF*)

class Constructor_QSizeF:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(qreal w, qreal h)
{ return this->engine()->toScriptValue(QSizeF(w, h)); }
QScriptValue qscript_call(QSize const& sz)
{ return this->engine()->toScriptValue(QSizeF(sz)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QSizeF()); }
};

class Prototype_QSizeF:
public QObject, public QSizeF, protected QScriptable
{
    Q_OBJECT
private:
inline QSizeF *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QSizeF*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QSizeF.prototype.%0: this object is not a QSizeF")
            .arg(method));
}
public:
Prototype_QSizeF(QObject *parent = 0)
    : QSizeF() { setParent(parent); }
public Q_SLOTS:
bool isValid() const
{ if (QSizeF *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
void scale(QSizeF const& s, Qt::AspectRatioMode mode)
{ if (QSizeF *_q_q = this->self()) _q_q->scale(s, mode);
  else throwTypeError(QLatin1String("scale")); }
void scale(qreal w, qreal h, Qt::AspectRatioMode mode)
{ if (QSizeF *_q_q = this->self()) _q_q->scale(w, h, mode);
  else throwTypeError(QLatin1String("scale")); }
qreal* rwidth()
{ if (QSizeF *_q_q = this->self()) return &_q_q->rwidth();
  throwTypeError(QLatin1String("rwidth")); return 0; }
QSizeF expandedTo(QSizeF const& arg1) const
{ if (QSizeF *_q_q = this->self()) return _q_q->expandedTo(arg1);
  throwTypeError(QLatin1String("expandedTo")); return QSizeF(); }
void setWidth(qreal w)
{ if (QSizeF *_q_q = this->self()) _q_q->setWidth(w);
  else throwTypeError(QLatin1String("setWidth")); }
void setHeight(qreal h)
{ if (QSizeF *_q_q = this->self()) _q_q->setHeight(h);
  else throwTypeError(QLatin1String("setHeight")); }
qreal* rheight()
{ if (QSizeF *_q_q = this->self()) return &_q_q->rheight();
  throwTypeError(QLatin1String("rheight")); return 0; }
bool isNull() const
{ if (QSizeF *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
QSizeF boundedTo(QSizeF const& arg1) const
{ if (QSizeF *_q_q = this->self()) return _q_q->boundedTo(arg1);
  throwTypeError(QLatin1String("boundedTo")); return QSizeF(); }
qreal width() const
{ if (QSizeF *_q_q = this->self()) return _q_q->width();
  throwTypeError(QLatin1String("width")); return qreal(); }
void transpose()
{ if (QSizeF *_q_q = this->self()) _q_q->transpose();
  else throwTypeError(QLatin1String("transpose")); }
QSize toSize() const
{ if (QSizeF *_q_q = this->self()) return _q_q->toSize();
  throwTypeError(QLatin1String("toSize")); return QSize(); }
bool isEmpty() const
{ if (QSizeF *_q_q = this->self()) return _q_q->isEmpty();
  throwTypeError(QLatin1String("isEmpty")); return bool(); }
qreal height() const
{ if (QSizeF *_q_q = this->self()) return _q_q->height();
  throwTypeError(QLatin1String("height")); return qreal(); }
QString toString() const
{ return QLatin1String("QSizeF"); }
};

static QScriptValue create_QSizeF_class(QScriptEngine *engine)
{
    Prototype_QSizeF *pq = new Prototype_QSizeF;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QSizeF *cq = new Constructor_QSizeF;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QSizeF>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QSizeF>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QSizeF*>(), pv);
    return cv;
}

// QAbstractFileEngineIterator

Q_DECLARE_METATYPE(QAbstractFileEngineIterator*)

class Constructor_QAbstractFileEngineIterator:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QDir::Filters /* filters */, QStringList const& /* nameFilters */)
{ return this->context()->throwError(QString::fromLatin1("QAbstractFileEngineIterator cannot be instantiated")); }
};

class Prototype_QAbstractFileEngineIterator:
public QObject, public QAbstractFileEngineIterator, protected QScriptable
{
    Q_OBJECT
private:
inline QAbstractFileEngineIterator *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QAbstractFileEngineIterator*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QAbstractFileEngineIterator.prototype.%0: this object is not a QAbstractFileEngineIterator")
            .arg(method));
}
public:
Prototype_QAbstractFileEngineIterator(QObject *parent = 0)
    : QAbstractFileEngineIterator(QDir::AllEntries, QStringList()) { setParent(parent); }
public Q_SLOTS:
QFileInfo currentFileInfo() const
{ if (QAbstractFileEngineIterator *_q_q = this->self()) return _q_q->currentFileInfo();
  throwTypeError(QLatin1String("currentFileInfo")); return QFileInfo(); }
QString currentFilePath() const
{ if (QAbstractFileEngineIterator *_q_q = this->self()) return _q_q->currentFilePath();
  throwTypeError(QLatin1String("currentFilePath")); return QString(); }
QString currentFileName() const
{ if (QAbstractFileEngineIterator *_q_q = this->self()) return _q_q->currentFileName();
  throwTypeError(QLatin1String("currentFileName")); return QString(); }
QString next()
{ if (QAbstractFileEngineIterator *_q_q = this->self()) return _q_q->next();
  throwTypeError(QLatin1String("next")); return QString(); }
bool hasNext() const
{ if (QAbstractFileEngineIterator *_q_q = this->self()) return _q_q->hasNext();
  throwTypeError(QLatin1String("hasNext")); return bool(); }
QString path() const
{ if (QAbstractFileEngineIterator *_q_q = this->self()) return _q_q->path();
  throwTypeError(QLatin1String("path")); return QString(); }
QStringList nameFilters() const
{ if (QAbstractFileEngineIterator *_q_q = this->self()) return _q_q->nameFilters();
  throwTypeError(QLatin1String("nameFilters")); return QStringList(); }
QDir::Filters filters() const
{ if (QAbstractFileEngineIterator *_q_q = this->self()) return _q_q->filters();
  throwTypeError(QLatin1String("filters")); return QDir::Filters(); }
QString toString() const
{ return QLatin1String("QAbstractFileEngineIterator"); }
public:
private:
Q_DISABLE_COPY(Prototype_QAbstractFileEngineIterator)
};

static QScriptValue create_QAbstractFileEngineIterator_class(QScriptEngine *engine)
{
    Prototype_QAbstractFileEngineIterator *pq = new Prototype_QAbstractFileEngineIterator;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QAbstractFileEngineIterator *cq = new Constructor_QAbstractFileEngineIterator;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QAbstractFileEngineIterator*>(), pv);
    return cv;
}

// QByteArray

Q_DECLARE_METATYPE(QByteArray)
Q_DECLARE_METATYPE(QByteArray*)

class Constructor_QByteArray:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QByteArray const& arg1)
{ return this->engine()->toScriptValue(QByteArray(arg1)); }
QScriptValue qscript_call(int size, char c)
{ return this->engine()->toScriptValue(QByteArray(size, c)); }
QScriptValue qscript_call(char const* arg1, int size)
{ return this->engine()->toScriptValue(QByteArray(arg1, size)); }
QScriptValue qscript_call(char const* arg1)
{ return this->engine()->toScriptValue(QByteArray(arg1)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QByteArray()); }
QByteArray fromHex(QByteArray const& hexEncoded)
{ return QByteArray::fromHex(hexEncoded); }
QByteArray fromRawData(char const* arg1, int size)
{ return QByteArray::fromRawData(arg1, size); }
QByteArray fromBase64(QByteArray const& base64)
{ return QByteArray::fromBase64(base64); }
QByteArray number(double arg1, char f = 'g', int prec = 6)
{ return QByteArray::number(arg1, f, prec); }
QByteArray number(qulonglong arg1, int base = 10)
{ return QByteArray::number(arg1, base); }
QByteArray number(qlonglong arg1, int base = 10)
{ return QByteArray::number(arg1, base); }
QByteArray number(uint arg1, int base = 10)
{ return QByteArray::number(arg1, base); }
QByteArray number(int arg1, int base = 10)
{ return QByteArray::number(arg1, base); }
};

class Prototype_QByteArray:
public QObject, public QByteArray, protected QScriptable
{
    Q_OBJECT
private:
inline QByteArray *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QByteArray*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QByteArray.prototype.%0: this object is not a QByteArray")
            .arg(method));
}
public:
Prototype_QByteArray(QObject *parent = 0)
    : QByteArray() { setParent(parent); }
public Q_SLOTS:
QByteArray* remove(int index, int len)
{ if (QByteArray *_q_q = this->self()) return &_q_q->remove(index, len);
  throwTypeError(QLatin1String("remove")); return 0; }
QByteArray toHex() const
{ if (QByteArray *_q_q = this->self()) return _q_q->toHex();
  throwTypeError(QLatin1String("toHex")); return QByteArray(); }
qlonglong toLongLong(bool* ok = 0, int base = 10) const
{ if (QByteArray *_q_q = this->self()) return _q_q->toLongLong(ok, base);
  throwTypeError(QLatin1String("toLongLong")); return qlonglong(); }
QList<QByteArray> split(char sep) const
{ if (QByteArray *_q_q = this->self()) return _q_q->split(sep);
  throwTypeError(QLatin1String("split")); return QList<QByteArray>(); }
QByteArray* replace(QString const& before, QByteArray const& after)
{ if (QByteArray *_q_q = this->self()) return &_q_q->replace(before, after);
  throwTypeError(QLatin1String("replace")); return 0; }
QByteArray* replace(char c, QString const& after)
{ if (QByteArray *_q_q = this->self()) return &_q_q->replace(c, after);
  throwTypeError(QLatin1String("replace")); return 0; }
QByteArray* replace(QString const& before, char const* after)
{ if (QByteArray *_q_q = this->self()) return &_q_q->replace(before, after);
  throwTypeError(QLatin1String("replace")); return 0; }
QByteArray* replace(char before, char after)
{ if (QByteArray *_q_q = this->self()) return &_q_q->replace(before, after);
  throwTypeError(QLatin1String("replace")); return 0; }
QByteArray* replace(char const* before, QByteArray const& after)
{ if (QByteArray *_q_q = this->self()) return &_q_q->replace(before, after);
  throwTypeError(QLatin1String("replace")); return 0; }
QByteArray* replace(QByteArray const& before, char const* after)
{ if (QByteArray *_q_q = this->self()) return &_q_q->replace(before, after);
  throwTypeError(QLatin1String("replace")); return 0; }
QByteArray* replace(QByteArray const& before, QByteArray const& after)
{ if (QByteArray *_q_q = this->self()) return &_q_q->replace(before, after);
  throwTypeError(QLatin1String("replace")); return 0; }
QByteArray* replace(char const* before, char const* after)
{ if (QByteArray *_q_q = this->self()) return &_q_q->replace(before, after);
  throwTypeError(QLatin1String("replace")); return 0; }
QByteArray* replace(char before, QByteArray const& after)
{ if (QByteArray *_q_q = this->self()) return &_q_q->replace(before, after);
  throwTypeError(QLatin1String("replace")); return 0; }
QByteArray* replace(char before, char const* after)
{ if (QByteArray *_q_q = this->self()) return &_q_q->replace(before, after);
  throwTypeError(QLatin1String("replace")); return 0; }
QByteArray* replace(int index, int len, QByteArray const& s)
{ if (QByteArray *_q_q = this->self()) return &_q_q->replace(index, len, s);
  throwTypeError(QLatin1String("replace")); return 0; }
QByteArray* replace(int index, int len, char const* s)
{ if (QByteArray *_q_q = this->self()) return &_q_q->replace(index, len, s);
  throwTypeError(QLatin1String("replace")); return 0; }
void reserve(int size)
{ if (QByteArray *_q_q = this->self()) _q_q->reserve(size);
  else throwTypeError(QLatin1String("reserve")); }
QByteArray::DataPtr* data_ptr()
{ if (QByteArray *_q_q = this->self()) return &_q_q->data_ptr();
  throwTypeError(QLatin1String("data_ptr")); return 0; }
QByteArray rightJustified(int width, char fill = ' ', bool truncate = false) const
{ if (QByteArray *_q_q = this->self()) return _q_q->rightJustified(width, fill, truncate);
  throwTypeError(QLatin1String("rightJustified")); return QByteArray(); }
int indexOf(QString const& s, int from = 0) const
{ if (QByteArray *_q_q = this->self()) return _q_q->indexOf(s, from);
  throwTypeError(QLatin1String("indexOf")); return int(); }
int indexOf(QByteArray const& a, int from = 0) const
{ if (QByteArray *_q_q = this->self()) return _q_q->indexOf(a, from);
  throwTypeError(QLatin1String("indexOf")); return int(); }
int indexOf(char const* c, int from = 0) const
{ if (QByteArray *_q_q = this->self()) return _q_q->indexOf(c, from);
  throwTypeError(QLatin1String("indexOf")); return int(); }
int indexOf(char c, int from = 0) const
{ if (QByteArray *_q_q = this->self()) return _q_q->indexOf(c, from);
  throwTypeError(QLatin1String("indexOf")); return int(); }
qulonglong toULongLong(bool* ok = 0, int base = 10) const
{ if (QByteArray *_q_q = this->self()) return _q_q->toULongLong(ok, base);
  throwTypeError(QLatin1String("toULongLong")); return qulonglong(); }
double toDouble(bool* ok = 0) const
{ if (QByteArray *_q_q = this->self()) return _q_q->toDouble(ok);
  throwTypeError(QLatin1String("toDouble")); return double(); }
void push_front(QByteArray const& a)
{ if (QByteArray *_q_q = this->self()) _q_q->push_front(a);
  else throwTypeError(QLatin1String("push_front")); }
void push_front(char const* c)
{ if (QByteArray *_q_q = this->self()) _q_q->push_front(c);
  else throwTypeError(QLatin1String("push_front")); }
void push_front(char c)
{ if (QByteArray *_q_q = this->self()) _q_q->push_front(c);
  else throwTypeError(QLatin1String("push_front")); }
int capacity() const
{ if (QByteArray *_q_q = this->self()) return _q_q->capacity();
  throwTypeError(QLatin1String("capacity")); return int(); }
int lastIndexOf(QString const& s, int from = -1) const
{ if (QByteArray *_q_q = this->self()) return _q_q->lastIndexOf(s, from);
  throwTypeError(QLatin1String("lastIndexOf")); return int(); }
int lastIndexOf(QByteArray const& a, int from = -1) const
{ if (QByteArray *_q_q = this->self()) return _q_q->lastIndexOf(a, from);
  throwTypeError(QLatin1String("lastIndexOf")); return int(); }
int lastIndexOf(char const* c, int from = -1) const
{ if (QByteArray *_q_q = this->self()) return _q_q->lastIndexOf(c, from);
  throwTypeError(QLatin1String("lastIndexOf")); return int(); }
int lastIndexOf(char c, int from = -1) const
{ if (QByteArray *_q_q = this->self()) return _q_q->lastIndexOf(c, from);
  throwTypeError(QLatin1String("lastIndexOf")); return int(); }
int count() const
{ if (QByteArray *_q_q = this->self()) return _q_q->count();
  throwTypeError(QLatin1String("count")); return int(); }
int count(QByteArray const& a) const
{ if (QByteArray *_q_q = this->self()) return _q_q->count(a);
  throwTypeError(QLatin1String("count")); return int(); }
int count(char const* a) const
{ if (QByteArray *_q_q = this->self()) return _q_q->count(a);
  throwTypeError(QLatin1String("count")); return int(); }
int count(char c) const
{ if (QByteArray *_q_q = this->self()) return _q_q->count(c);
  throwTypeError(QLatin1String("count")); return int(); }
QByteArray* append(QString const& s)
{ if (QByteArray *_q_q = this->self()) return &_q_q->append(s);
  throwTypeError(QLatin1String("append")); return 0; }
QByteArray* append(QByteArray const& a)
{ if (QByteArray *_q_q = this->self()) return &_q_q->append(a);
  throwTypeError(QLatin1String("append")); return 0; }
QByteArray* append(char const* s)
{ if (QByteArray *_q_q = this->self()) return &_q_q->append(s);
  throwTypeError(QLatin1String("append")); return 0; }
QByteArray* append(char c)
{ if (QByteArray *_q_q = this->self()) return &_q_q->append(c);
  throwTypeError(QLatin1String("append")); return 0; }
bool equals(QString const& s2) const
{ if (QByteArray *_q_q = this->self()) return _q_q->operator==(s2);
  throwTypeError(QLatin1String("equals")); return bool(); }
uint toUInt(bool* ok = 0, int base = 10) const
{ if (QByteArray *_q_q = this->self()) return _q_q->toUInt(ok, base);
  throwTypeError(QLatin1String("toUInt")); return uint(); }
float toFloat(bool* ok = 0) const
{ if (QByteArray *_q_q = this->self()) return _q_q->toFloat(ok);
  throwTypeError(QLatin1String("toFloat")); return float(); }
QByteArray* fill(char c, int size = -1)
{ if (QByteArray *_q_q = this->self()) return &_q_q->fill(c, size);
  throwTypeError(QLatin1String("fill")); return 0; }
int size() const
{ if (QByteArray *_q_q = this->self()) return _q_q->size();
  throwTypeError(QLatin1String("size")); return int(); }
QByteArray right(int len) const
{ if (QByteArray *_q_q = this->self()) return _q_q->right(len);
  throwTypeError(QLatin1String("right")); return QByteArray(); }
char const at(int i) const
{ if (QByteArray *_q_q = this->self()) return _q_q->at(i);
  throwTypeError(QLatin1String("at")); return char(); }
int length() const
{ if (QByteArray *_q_q = this->self()) return _q_q->length();
  throwTypeError(QLatin1String("length")); return int(); }
QByteArray mid(int index, int len = -1) const
{ if (QByteArray *_q_q = this->self()) return _q_q->mid(index, len);
  throwTypeError(QLatin1String("mid")); return QByteArray(); }
char const* constData() const
{ if (QByteArray *_q_q = this->self()) return _q_q->constData();
  throwTypeError(QLatin1String("constData")); return 0; }
bool isEmpty() const
{ if (QByteArray *_q_q = this->self()) return _q_q->isEmpty();
  throwTypeError(QLatin1String("isEmpty")); return bool(); }
int toInt(bool* ok = 0, int base = 10) const
{ if (QByteArray *_q_q = this->self()) return _q_q->toInt(ok, base);
  throwTypeError(QLatin1String("toInt")); return int(); }
void push_back(QByteArray const& a)
{ if (QByteArray *_q_q = this->self()) _q_q->push_back(a);
  else throwTypeError(QLatin1String("push_back")); }
void push_back(char const* c)
{ if (QByteArray *_q_q = this->self()) _q_q->push_back(c);
  else throwTypeError(QLatin1String("push_back")); }
void push_back(char c)
{ if (QByteArray *_q_q = this->self()) _q_q->push_back(c);
  else throwTypeError(QLatin1String("push_back")); }
QByteArray trimmed() const
{ if (QByteArray *_q_q = this->self()) return _q_q->trimmed();
  throwTypeError(QLatin1String("trimmed")); return QByteArray(); }
void detach()
{ if (QByteArray *_q_q = this->self()) _q_q->detach();
  else throwTypeError(QLatin1String("detach")); }
ushort toUShort(bool* ok = 0, int base = 10) const
{ if (QByteArray *_q_q = this->self()) return _q_q->toUShort(ok, base);
  throwTypeError(QLatin1String("toUShort")); return ushort(); }
bool startsWith(char const* c) const
{ if (QByteArray *_q_q = this->self()) return _q_q->startsWith(c);
  throwTypeError(QLatin1String("startsWith")); return bool(); }
bool startsWith(char c) const
{ if (QByteArray *_q_q = this->self()) return _q_q->startsWith(c);
  throwTypeError(QLatin1String("startsWith")); return bool(); }
bool startsWith(QByteArray const& a) const
{ if (QByteArray *_q_q = this->self()) return _q_q->startsWith(a);
  throwTypeError(QLatin1String("startsWith")); return bool(); }
void squeeze()
{ if (QByteArray *_q_q = this->self()) _q_q->squeeze();
  else throwTypeError(QLatin1String("squeeze")); }
QByteArray::const_iterator begin() const
{ if (QByteArray *_q_q = this->self()) return _q_q->begin();
  throwTypeError(QLatin1String("begin")); return QByteArray::const_iterator(); }
QByteArray::iterator begin()
{ if (QByteArray *_q_q = this->self()) return _q_q->begin();
  throwTypeError(QLatin1String("begin")); return QByteArray::iterator(); }
ulong toULong(bool* ok = 0, int base = 10) const
{ if (QByteArray *_q_q = this->self()) return _q_q->toULong(ok, base);
  throwTypeError(QLatin1String("toULong")); return ulong(); }
QByteArray left(int len) const
{ if (QByteArray *_q_q = this->self()) return _q_q->left(len);
  throwTypeError(QLatin1String("left")); return QByteArray(); }
QByteArray* prepend(QByteArray const& a)
{ if (QByteArray *_q_q = this->self()) return &_q_q->prepend(a);
  throwTypeError(QLatin1String("prepend")); return 0; }
QByteArray* prepend(char const* s)
{ if (QByteArray *_q_q = this->self()) return &_q_q->prepend(s);
  throwTypeError(QLatin1String("prepend")); return 0; }
QByteArray* prepend(char c)
{ if (QByteArray *_q_q = this->self()) return &_q_q->prepend(c);
  throwTypeError(QLatin1String("prepend")); return 0; }
short toShort(bool* ok = 0, int base = 10) const
{ if (QByteArray *_q_q = this->self()) return _q_q->toShort(ok, base);
  throwTypeError(QLatin1String("toShort")); return short(); }
QByteArray* setNum(double arg1, char f = 'g', int prec = 6)
{ if (QByteArray *_q_q = this->self()) return &_q_q->setNum(arg1, f, prec);
  throwTypeError(QLatin1String("setNum")); return 0; }
QByteArray* setNum(float arg1, char f = 'g', int prec = 6)
{ if (QByteArray *_q_q = this->self()) return &_q_q->setNum(arg1, f, prec);
  throwTypeError(QLatin1String("setNum")); return 0; }
QByteArray* setNum(qulonglong arg1, int base = 10)
{ if (QByteArray *_q_q = this->self()) return &_q_q->setNum(arg1, base);
  throwTypeError(QLatin1String("setNum")); return 0; }
QByteArray* setNum(qlonglong arg1, int base = 10)
{ if (QByteArray *_q_q = this->self()) return &_q_q->setNum(arg1, base);
  throwTypeError(QLatin1String("setNum")); return 0; }
QByteArray* setNum(uint arg1, int base = 10)
{ if (QByteArray *_q_q = this->self()) return &_q_q->setNum(arg1, base);
  throwTypeError(QLatin1String("setNum")); return 0; }
QByteArray* setNum(int arg1, int base = 10)
{ if (QByteArray *_q_q = this->self()) return &_q_q->setNum(arg1, base);
  throwTypeError(QLatin1String("setNum")); return 0; }
QByteArray* setNum(ushort arg1, int base = 10)
{ if (QByteArray *_q_q = this->self()) return &_q_q->setNum(arg1, base);
  throwTypeError(QLatin1String("setNum")); return 0; }
QByteArray* setNum(short arg1, int base = 10)
{ if (QByteArray *_q_q = this->self()) return &_q_q->setNum(arg1, base);
  throwTypeError(QLatin1String("setNum")); return 0; }
bool lessThan(QString const& s2) const
{ if (QByteArray *_q_q = this->self()) return _q_q->operator<(s2);
  throwTypeError(QLatin1String("lessThan")); return bool(); }
long toLong(bool* ok = 0, int base = 10) const
{ if (QByteArray *_q_q = this->self()) return _q_q->toLong(ok, base);
  throwTypeError(QLatin1String("toLong")); return long(); }
QByteArray simplified() const
{ if (QByteArray *_q_q = this->self()) return _q_q->simplified();
  throwTypeError(QLatin1String("simplified")); return QByteArray(); }
void clear()
{ if (QByteArray *_q_q = this->self()) _q_q->clear();
  else throwTypeError(QLatin1String("clear")); }
void chop(int n)
{ if (QByteArray *_q_q = this->self()) _q_q->chop(n);
  else throwTypeError(QLatin1String("chop")); }
QByteArray leftJustified(int width, char fill = ' ', bool truncate = false) const
{ if (QByteArray *_q_q = this->self()) return _q_q->leftJustified(width, fill, truncate);
  throwTypeError(QLatin1String("leftJustified")); return QByteArray(); }
QByteArray::const_iterator constEnd() const
{ if (QByteArray *_q_q = this->self()) return _q_q->constEnd();
  throwTypeError(QLatin1String("constEnd")); return QByteArray::const_iterator(); }
bool isNull() const
{ if (QByteArray *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
QByteArray::const_iterator end() const
{ if (QByteArray *_q_q = this->self()) return _q_q->end();
  throwTypeError(QLatin1String("end")); return QByteArray::const_iterator(); }
QByteArray::iterator end()
{ if (QByteArray *_q_q = this->self()) return _q_q->end();
  throwTypeError(QLatin1String("end")); return QByteArray::iterator(); }
QByteArray toBase64() const
{ if (QByteArray *_q_q = this->self()) return _q_q->toBase64();
  throwTypeError(QLatin1String("toBase64")); return QByteArray(); }
void truncate(int pos)
{ if (QByteArray *_q_q = this->self()) _q_q->truncate(pos);
  else throwTypeError(QLatin1String("truncate")); }
QByteArray::const_iterator constBegin() const
{ if (QByteArray *_q_q = this->self()) return _q_q->constBegin();
  throwTypeError(QLatin1String("constBegin")); return QByteArray::const_iterator(); }
void resize(int size)
{ if (QByteArray *_q_q = this->self()) _q_q->resize(size);
  else throwTypeError(QLatin1String("resize")); }
QByteArray toLower() const
{ if (QByteArray *_q_q = this->self()) return _q_q->toLower();
  throwTypeError(QLatin1String("toLower")); return QByteArray(); }
QByteArray toUpper() const
{ if (QByteArray *_q_q = this->self()) return _q_q->toUpper();
  throwTypeError(QLatin1String("toUpper")); return QByteArray(); }
bool endsWith(char const* c) const
{ if (QByteArray *_q_q = this->self()) return _q_q->endsWith(c);
  throwTypeError(QLatin1String("endsWith")); return bool(); }
bool endsWith(char c) const
{ if (QByteArray *_q_q = this->self()) return _q_q->endsWith(c);
  throwTypeError(QLatin1String("endsWith")); return bool(); }
bool endsWith(QByteArray const& a) const
{ if (QByteArray *_q_q = this->self()) return _q_q->endsWith(a);
  throwTypeError(QLatin1String("endsWith")); return bool(); }
QBool contains(QByteArray const& a) const
{ if (QByteArray *_q_q = this->self()) return _q_q->contains(a);
  throwTypeError(QLatin1String("contains")); return QBool(false); }
QBool contains(char const* a) const
{ if (QByteArray *_q_q = this->self()) return _q_q->contains(a);
  throwTypeError(QLatin1String("contains")); return QBool(false); }
QBool contains(char c) const
{ if (QByteArray *_q_q = this->self()) return _q_q->contains(c);
  throwTypeError(QLatin1String("contains")); return QBool(false); }
QByteArray* insert(int i, QString const& s)
{ if (QByteArray *_q_q = this->self()) return &_q_q->insert(i, s);
  throwTypeError(QLatin1String("insert")); return 0; }
QByteArray* insert(int i, QByteArray const& a)
{ if (QByteArray *_q_q = this->self()) return &_q_q->insert(i, a);
  throwTypeError(QLatin1String("insert")); return 0; }
QByteArray* insert(int i, char const* s)
{ if (QByteArray *_q_q = this->self()) return &_q_q->insert(i, s);
  throwTypeError(QLatin1String("insert")); return 0; }
QByteArray* insert(int i, char c)
{ if (QByteArray *_q_q = this->self()) return &_q_q->insert(i, c);
  throwTypeError(QLatin1String("insert")); return 0; }
char const* data() const
{ if (QByteArray *_q_q = this->self()) return _q_q->data();
  throwTypeError(QLatin1String("data")); return 0; }
char* data()
{ if (QByteArray *_q_q = this->self()) return _q_q->data();
  throwTypeError(QLatin1String("data")); return 0; }
bool isDetached() const
{ if (QByteArray *_q_q = this->self()) return _q_q->isDetached();
  throwTypeError(QLatin1String("isDetached")); return bool(); }
QString toString() const
{ return QLatin1String("QByteArray"); }
};

static QScriptValue create_QByteArray_class(QScriptEngine *engine)
{
    Prototype_QByteArray *pq = new Prototype_QByteArray;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QByteArray *cq = new Constructor_QByteArray;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QByteArray>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QByteArray>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QByteArray*>(), pv);
    return cv;
}

// QLine

Q_DECLARE_METATYPE(QLine)
Q_DECLARE_METATYPE(QLine*)

class Constructor_QLine:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(int x1, int y1, int x2, int y2)
{ return this->engine()->toScriptValue(QLine(x1, y1, x2, y2)); }
QScriptValue qscript_call(QPoint const& pt1, QPoint const& pt2)
{ return this->engine()->toScriptValue(QLine(pt1, pt2)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QLine()); }
};

class Prototype_QLine:
public QObject, public QLine, protected QScriptable
{
    Q_OBJECT
private:
inline QLine *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QLine*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QLine.prototype.%0: this object is not a QLine")
            .arg(method));
}
public:
Prototype_QLine(QObject *parent = 0)
    : QLine() { setParent(parent); }
public Q_SLOTS:
int dx() const
{ if (QLine *_q_q = this->self()) return _q_q->dx();
  throwTypeError(QLatin1String("dx")); return int(); }
int dy() const
{ if (QLine *_q_q = this->self()) return _q_q->dy();
  throwTypeError(QLatin1String("dy")); return int(); }
QPoint p1() const
{ if (QLine *_q_q = this->self()) return _q_q->p1();
  throwTypeError(QLatin1String("p1")); return QPoint(); }
QPoint p2() const
{ if (QLine *_q_q = this->self()) return _q_q->p2();
  throwTypeError(QLatin1String("p2")); return QPoint(); }
void translate(int dx, int dy)
{ if (QLine *_q_q = this->self()) _q_q->translate(dx, dy);
  else throwTypeError(QLatin1String("translate")); }
void translate(QPoint const& p)
{ if (QLine *_q_q = this->self()) _q_q->translate(p);
  else throwTypeError(QLatin1String("translate")); }
bool equals(QLine const& d) const
{ if (QLine *_q_q = this->self()) return _q_q->operator==(d);
  throwTypeError(QLatin1String("equals")); return bool(); }
int y1() const
{ if (QLine *_q_q = this->self()) return _q_q->y1();
  throwTypeError(QLatin1String("y1")); return int(); }
int x1() const
{ if (QLine *_q_q = this->self()) return _q_q->x1();
  throwTypeError(QLatin1String("x1")); return int(); }
int y2() const
{ if (QLine *_q_q = this->self()) return _q_q->y2();
  throwTypeError(QLatin1String("y2")); return int(); }
bool isNull() const
{ if (QLine *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
int x2() const
{ if (QLine *_q_q = this->self()) return _q_q->x2();
  throwTypeError(QLatin1String("x2")); return int(); }
QString toString() const
{ return QLatin1String("QLine"); }
};

static QScriptValue create_QLine_class(QScriptEngine *engine)
{
    Prototype_QLine *pq = new Prototype_QLine;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QLine *cq = new Constructor_QLine;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QLine>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QLine>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QLine*>(), pv);
    return cv;
}

// QDir

Q_DECLARE_METATYPE(QDir*)
Q_DECLARE_METATYPE(QDir::Filter)
Q_DECLARE_METATYPE(QDir::SortFlag)
Q_DECLARE_METATYPE(QDir::Filters)
Q_DECLARE_METATYPE(QDir::SortFlags)

class Constructor_QDir:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QString const& path, QString const& nameFilter, QDir::SortFlags sort = QDir::SortFlags(QDir::Name | QDir::IgnoreCase), QDir::Filters filter = QDir::AllEntries)
{ return this->engine()->toScriptValue(new QDir(path, nameFilter, sort, filter)); }
QScriptValue qscript_call(QString const& path = QString())
{ return this->engine()->toScriptValue(new QDir(path)); }
QScriptValue qscript_call(QDir const& arg1)
{ return this->engine()->toScriptValue(new QDir(arg1)); }
QString toNativeSeparators(QString const& pathName)
{ return QDir::toNativeSeparators(pathName); }
QString cleanPath(QString const& path)
{ return QDir::cleanPath(path); }
void addSearchPath(QString const& prefix, QString const& path)
{ QDir::addSearchPath(prefix, path); }
bool isRelativePath(QString const& path)
{ return QDir::isRelativePath(path); }
QStringList nameFiltersFromString(QString const& nameFilter)
{ return QDir::nameFiltersFromString(nameFilter); }
QString tempPath()
{ return QDir::tempPath(); }
bool setCurrent(QString const& path)
{ return QDir::setCurrent(path); }
QString rootPath()
{ return QDir::rootPath(); }
void addResourceSearchPath(QString const& path)
{ QDir::addResourceSearchPath(path); }
bool match(QString const& filter, QString const& fileName)
{ return QDir::match(filter, fileName); }
bool match(QStringList const& filters, QString const& fileName)
{ return QDir::match(filters, fileName); }
QDir current()
{ return QDir::current(); }
QString fromNativeSeparators(QString const& pathName)
{ return QDir::fromNativeSeparators(pathName); }
QString currentPath()
{ return QDir::currentPath(); }
bool isAbsolutePath(QString const& path)
{ return QDir::isAbsolutePath(path); }
QDir temp()
{ return QDir::temp(); }
void setSearchPaths(QString const& prefix, QStringList const& searchPaths)
{ QDir::setSearchPaths(prefix, searchPaths); }
QStringList searchPaths(QString const& prefix)
{ return QDir::searchPaths(prefix); }
QChar separator()
{ return QDir::separator(); }
QDir home()
{ return QDir::home(); }
QDir root()
{ return QDir::root(); }
QFileInfoList drives()
{ return QDir::drives(); }
QString homePath()
{ return QDir::homePath(); }
};

class Prototype_QDir:
public QObject, public QDir, protected QScriptable
{
    Q_OBJECT
private:
inline QDir *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QDir*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QDir.prototype.%0: this object is not a QDir")
            .arg(method));
}
public:
Prototype_QDir(QObject *parent = 0)
    : QDir() { setParent(parent); }
public Q_SLOTS:
bool rename(QString const& oldName, QString const& newName)
{ if (QDir *_q_q = this->self()) return _q_q->rename(oldName, newName);
  throwTypeError(QLatin1String("rename")); return bool(); }
QFileInfoList entryInfoList(QStringList const& nameFilters, QDir::Filters filters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort) const
{ if (QDir *_q_q = this->self()) return _q_q->entryInfoList(nameFilters, filters, sort);
  throwTypeError(QLatin1String("entryInfoList")); return QFileInfoList(); }
QFileInfoList entryInfoList(QDir::Filters filters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort) const
{ if (QDir *_q_q = this->self()) return _q_q->entryInfoList(filters, sort);
  throwTypeError(QLatin1String("entryInfoList")); return QFileInfoList(); }
QDir::Filters filter() const
{ if (QDir *_q_q = this->self()) return _q_q->filter();
  throwTypeError(QLatin1String("filter")); return QDir::Filters(); }
bool cdUp()
{ if (QDir *_q_q = this->self()) return _q_q->cdUp();
  throwTypeError(QLatin1String("cdUp")); return bool(); }
bool mkpath(QString const& dirPath) const
{ if (QDir *_q_q = this->self()) return _q_q->mkpath(dirPath);
  throwTypeError(QLatin1String("mkpath")); return bool(); }
QString canonicalPath() const
{ if (QDir *_q_q = this->self()) return _q_q->canonicalPath();
  throwTypeError(QLatin1String("canonicalPath")); return QString(); }
QString relativeFilePath(QString const& fileName) const
{ if (QDir *_q_q = this->self()) return _q_q->relativeFilePath(fileName);
  throwTypeError(QLatin1String("relativeFilePath")); return QString(); }
void setPath(QString const& path)
{ if (QDir *_q_q = this->self()) _q_q->setPath(path);
  else throwTypeError(QLatin1String("setPath")); }
bool remove(QString const& fileName)
{ if (QDir *_q_q = this->self()) return _q_q->remove(fileName);
  throwTypeError(QLatin1String("remove")); return bool(); }
void refresh() const
{ if (QDir *_q_q = this->self()) _q_q->refresh();
  else throwTypeError(QLatin1String("refresh")); }
bool isReadable() const
{ if (QDir *_q_q = this->self()) return _q_q->isReadable();
  throwTypeError(QLatin1String("isReadable")); return bool(); }
void setFilter(QDir::Filters filter)
{ if (QDir *_q_q = this->self()) _q_q->setFilter(filter);
  else throwTypeError(QLatin1String("setFilter")); }
QStringList nameFilters() const
{ if (QDir *_q_q = this->self()) return _q_q->nameFilters();
  throwTypeError(QLatin1String("nameFilters")); return QStringList(); }
void setSorting(QDir::SortFlags sort)
{ if (QDir *_q_q = this->self()) _q_q->setSorting(sort);
  else throwTypeError(QLatin1String("setSorting")); }
QString absolutePath() const
{ if (QDir *_q_q = this->self()) return _q_q->absolutePath();
  throwTypeError(QLatin1String("absolutePath")); return QString(); }
QString dirName() const
{ if (QDir *_q_q = this->self()) return _q_q->dirName();
  throwTypeError(QLatin1String("dirName")); return QString(); }
void setNameFilters(QStringList const& nameFilters)
{ if (QDir *_q_q = this->self()) _q_q->setNameFilters(nameFilters);
  else throwTypeError(QLatin1String("setNameFilters")); }
QString path() const
{ if (QDir *_q_q = this->self()) return _q_q->path();
  throwTypeError(QLatin1String("path")); return QString(); }
bool isRoot() const
{ if (QDir *_q_q = this->self()) return _q_q->isRoot();
  throwTypeError(QLatin1String("isRoot")); return bool(); }
bool rmdir(QString const& dirName) const
{ if (QDir *_q_q = this->self()) return _q_q->rmdir(dirName);
  throwTypeError(QLatin1String("rmdir")); return bool(); }
QDir::SortFlags sorting() const
{ if (QDir *_q_q = this->self()) return _q_q->sorting();
  throwTypeError(QLatin1String("sorting")); return QDir::SortFlags(); }
bool equals(QDir const& dir) const
{ if (QDir *_q_q = this->self()) return _q_q->operator==(dir);
  throwTypeError(QLatin1String("equals")); return bool(); }
bool rmpath(QString const& dirPath) const
{ if (QDir *_q_q = this->self()) return _q_q->rmpath(dirPath);
  throwTypeError(QLatin1String("rmpath")); return bool(); }
bool mkdir(QString const& dirName) const
{ if (QDir *_q_q = this->self()) return _q_q->mkdir(dirName);
  throwTypeError(QLatin1String("mkdir")); return bool(); }
bool isAbsolute() const
{ if (QDir *_q_q = this->self()) return _q_q->isAbsolute();
  throwTypeError(QLatin1String("isAbsolute")); return bool(); }
QStringList entryList(QStringList const& nameFilters, QDir::Filters filters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort) const
{ if (QDir *_q_q = this->self()) return _q_q->entryList(nameFilters, filters, sort);
  throwTypeError(QLatin1String("entryList")); return QStringList(); }
QStringList entryList(QDir::Filters filters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort) const
{ if (QDir *_q_q = this->self()) return _q_q->entryList(filters, sort);
  throwTypeError(QLatin1String("entryList")); return QStringList(); }
bool makeAbsolute()
{ if (QDir *_q_q = this->self()) return _q_q->makeAbsolute();
  throwTypeError(QLatin1String("makeAbsolute")); return bool(); }
bool isRelative() const
{ if (QDir *_q_q = this->self()) return _q_q->isRelative();
  throwTypeError(QLatin1String("isRelative")); return bool(); }
uint count() const
{ if (QDir *_q_q = this->self()) return _q_q->count();
  throwTypeError(QLatin1String("count")); return uint(); }
bool cd(QString const& dirName)
{ if (QDir *_q_q = this->self()) return _q_q->cd(dirName);
  throwTypeError(QLatin1String("cd")); return bool(); }
bool exists(QString const& name) const
{ if (QDir *_q_q = this->self()) return _q_q->exists(name);
  throwTypeError(QLatin1String("exists")); return bool(); }
bool exists() const
{ if (QDir *_q_q = this->self()) return _q_q->exists();
  throwTypeError(QLatin1String("exists")); return bool(); }
QString absoluteFilePath(QString const& fileName) const
{ if (QDir *_q_q = this->self()) return _q_q->absoluteFilePath(fileName);
  throwTypeError(QLatin1String("absoluteFilePath")); return QString(); }
QString filePath(QString const& fileName) const
{ if (QDir *_q_q = this->self()) return _q_q->filePath(fileName);
  throwTypeError(QLatin1String("filePath")); return QString(); }
QString toString() const
{ return QLatin1String("QDir"); }
private:
Q_DISABLE_COPY(Prototype_QDir)
};

static QScriptValue create_QDir_class(QScriptEngine *engine)
{
    Prototype_QDir *pq = new Prototype_QDir;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QDir *cq = new Constructor_QDir;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QDir*>(), pv);
    _q_ScriptRegisterEnumMetaType<QDir::Filter>(engine);
    cv.setProperty("Dirs", QScriptValue(engine, static_cast<int>(QDir::Dirs)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Files", QScriptValue(engine, static_cast<int>(QDir::Files)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Drives", QScriptValue(engine, static_cast<int>(QDir::Drives)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NoSymLinks", QScriptValue(engine, static_cast<int>(QDir::NoSymLinks)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AllEntries", QScriptValue(engine, static_cast<int>(QDir::AllEntries)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TypeMask", QScriptValue(engine, static_cast<int>(QDir::TypeMask)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Readable", QScriptValue(engine, static_cast<int>(QDir::Readable)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Writable", QScriptValue(engine, static_cast<int>(QDir::Writable)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Executable", QScriptValue(engine, static_cast<int>(QDir::Executable)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("PermissionMask", QScriptValue(engine, static_cast<int>(QDir::PermissionMask)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Modified", QScriptValue(engine, static_cast<int>(QDir::Modified)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Hidden", QScriptValue(engine, static_cast<int>(QDir::Hidden)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("System", QScriptValue(engine, static_cast<int>(QDir::System)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AccessMask", QScriptValue(engine, static_cast<int>(QDir::AccessMask)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AllDirs", QScriptValue(engine, static_cast<int>(QDir::AllDirs)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CaseSensitive", QScriptValue(engine, static_cast<int>(QDir::CaseSensitive)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NoDotAndDotDot", QScriptValue(engine, static_cast<int>(QDir::NoDotAndDotDot)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NoFilter", QScriptValue(engine, static_cast<int>(QDir::NoFilter)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QDir::SortFlag>(engine);
    cv.setProperty("Name", QScriptValue(engine, static_cast<int>(QDir::Name)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Time", QScriptValue(engine, static_cast<int>(QDir::Time)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Size", QScriptValue(engine, static_cast<int>(QDir::Size)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Unsorted", QScriptValue(engine, static_cast<int>(QDir::Unsorted)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SortByMask", QScriptValue(engine, static_cast<int>(QDir::SortByMask)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirsFirst", QScriptValue(engine, static_cast<int>(QDir::DirsFirst)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Reversed", QScriptValue(engine, static_cast<int>(QDir::Reversed)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("IgnoreCase", QScriptValue(engine, static_cast<int>(QDir::IgnoreCase)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirsLast", QScriptValue(engine, static_cast<int>(QDir::DirsLast)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LocaleAware", QScriptValue(engine, static_cast<int>(QDir::LocaleAware)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Type", QScriptValue(engine, static_cast<int>(QDir::Type)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NoSort", QScriptValue(engine, static_cast<int>(QDir::NoSort)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QDir::Filters>(engine);
    _q_ScriptRegisterEnumMetaType<QDir::SortFlags>(engine);
    return cv;
}

// QBitArray

Q_DECLARE_METATYPE(QBitArray)
Q_DECLARE_METATYPE(QBitArray*)

class Constructor_QBitArray:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QBitArray const& other)
{ return this->engine()->toScriptValue(QBitArray(other)); }
QScriptValue qscript_call(int size, bool val = false)
{ return this->engine()->toScriptValue(QBitArray(size, val)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QBitArray()); }
};

class Prototype_QBitArray:
public QObject, public QBitArray, protected QScriptable
{
    Q_OBJECT
private:
inline QBitArray *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QBitArray*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QBitArray.prototype.%0: this object is not a QBitArray")
            .arg(method));
}
public:
Prototype_QBitArray(QObject *parent = 0)
    : QBitArray() { setParent(parent); }
public Q_SLOTS:
void detach()
{ if (QBitArray *_q_q = this->self()) _q_q->detach();
  else throwTypeError(QLatin1String("detach")); }
void resize(int size)
{ if (QBitArray *_q_q = this->self()) _q_q->resize(size);
  else throwTypeError(QLatin1String("resize")); }
void clearBit(int i)
{ if (QBitArray *_q_q = this->self()) _q_q->clearBit(i);
  else throwTypeError(QLatin1String("clearBit")); }
bool at(int i) const
{ if (QBitArray *_q_q = this->self()) return _q_q->at(i);
  throwTypeError(QLatin1String("at")); return bool(); }
bool isDetached() const
{ if (QBitArray *_q_q = this->self()) return _q_q->isDetached();
  throwTypeError(QLatin1String("isDetached")); return bool(); }
void truncate(int pos)
{ if (QBitArray *_q_q = this->self()) _q_q->truncate(pos);
  else throwTypeError(QLatin1String("truncate")); }
int count(bool on) const
{ if (QBitArray *_q_q = this->self()) return _q_q->count(on);
  throwTypeError(QLatin1String("count")); return int(); }
int count() const
{ if (QBitArray *_q_q = this->self()) return _q_q->count();
  throwTypeError(QLatin1String("count")); return int(); }
bool equals(QBitArray const& a) const
{ if (QBitArray *_q_q = this->self()) return _q_q->operator==(a);
  throwTypeError(QLatin1String("equals")); return bool(); }
bool isNull() const
{ if (QBitArray *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
QBitArray::DataPtr* data_ptr()
{ if (QBitArray *_q_q = this->self()) return &_q_q->data_ptr();
  throwTypeError(QLatin1String("data_ptr")); return 0; }
int size() const
{ if (QBitArray *_q_q = this->self()) return _q_q->size();
  throwTypeError(QLatin1String("size")); return int(); }
void fill(bool val, int first, int last)
{ if (QBitArray *_q_q = this->self()) _q_q->fill(val, first, last);
  else throwTypeError(QLatin1String("fill")); }
bool fill(bool val, int size = -1)
{ if (QBitArray *_q_q = this->self()) return _q_q->fill(val, size);
  throwTypeError(QLatin1String("fill")); return bool(); }
void clear()
{ if (QBitArray *_q_q = this->self()) _q_q->clear();
  else throwTypeError(QLatin1String("clear")); }
bool isEmpty() const
{ if (QBitArray *_q_q = this->self()) return _q_q->isEmpty();
  throwTypeError(QLatin1String("isEmpty")); return bool(); }
void setBit(int i, bool val)
{ if (QBitArray *_q_q = this->self()) _q_q->setBit(i, val);
  else throwTypeError(QLatin1String("setBit")); }
void setBit(int i)
{ if (QBitArray *_q_q = this->self()) _q_q->setBit(i);
  else throwTypeError(QLatin1String("setBit")); }
bool testBit(int i) const
{ if (QBitArray *_q_q = this->self()) return _q_q->testBit(i);
  throwTypeError(QLatin1String("testBit")); return bool(); }
bool toggleBit(int i)
{ if (QBitArray *_q_q = this->self()) return _q_q->toggleBit(i);
  throwTypeError(QLatin1String("toggleBit")); return bool(); }
QString toString() const
{ return QLatin1String("QBitArray"); }
};

static QScriptValue create_QBitArray_class(QScriptEngine *engine)
{
    Prototype_QBitArray *pq = new Prototype_QBitArray;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QBitArray *cq = new Constructor_QBitArray;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QBitArray>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QBitArray>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QBitArray*>(), pv);
    return cv;
}

// QSysInfo

Q_DECLARE_METATYPE(QSysInfo*)
Q_DECLARE_METATYPE(QSysInfo::Endian)
Q_DECLARE_METATYPE(QSysInfo::Sizes)

class Constructor_QSysInfo:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
};

class Prototype_QSysInfo:
public QObject, public QSysInfo, protected QScriptable
{
    Q_OBJECT
private:
inline QSysInfo *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QSysInfo*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QSysInfo.prototype.%0: this object is not a QSysInfo")
            .arg(method));
}
public:
Prototype_QSysInfo(QObject *parent = 0)
    : QSysInfo() { setParent(parent); }
public Q_SLOTS:
QString toString() const
{ return QLatin1String("QSysInfo"); }
private:
Q_DISABLE_COPY(Prototype_QSysInfo)
};

static QScriptValue create_QSysInfo_class(QScriptEngine *engine)
{
    Prototype_QSysInfo *pq = new Prototype_QSysInfo;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QSysInfo *cq = new Constructor_QSysInfo;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QSysInfo*>(), pv);
    _q_ScriptRegisterEnumMetaType<QSysInfo::Endian>(engine);
    cv.setProperty("BigEndian", QScriptValue(engine, static_cast<int>(QSysInfo::BigEndian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LittleEndian", QScriptValue(engine, static_cast<int>(QSysInfo::LittleEndian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ByteOrder", QScriptValue(engine, static_cast<int>(QSysInfo::ByteOrder)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QSysInfo::Sizes>(engine);
    cv.setProperty("WordSize", QScriptValue(engine, static_cast<int>(QSysInfo::WordSize)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QEvent

Q_DECLARE_METATYPE(QEvent*)
Q_DECLARE_METATYPE(QEvent::Type)

class Constructor_QEvent:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QEvent::Type type)
{ return this->engine()->toScriptValue(new QEvent(type)); }
};

class Prototype_QEvent:
public QObject, public QEvent, protected QScriptable
{
    Q_OBJECT
private:
inline QEvent *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QEvent*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QEvent.prototype.%0: this object is not a QEvent")
            .arg(method));
}
public:
Prototype_QEvent(QObject *parent = 0)
    : QEvent(QEvent::None) { setParent(parent); }
public Q_SLOTS:
bool spontaneous() const
{ if (QEvent *_q_q = this->self()) return _q_q->spontaneous();
  throwTypeError(QLatin1String("spontaneous")); return bool(); }
void ignore()
{ if (QEvent *_q_q = this->self()) _q_q->ignore();
  else throwTypeError(QLatin1String("ignore")); }
void accept()
{ if (QEvent *_q_q = this->self()) _q_q->accept();
  else throwTypeError(QLatin1String("accept")); }
QEvent::Type type() const
{ if (QEvent *_q_q = this->self()) return _q_q->type();
  throwTypeError(QLatin1String("type")); return QEvent::Type(); }
void setAccepted(bool accepted)
{ if (QEvent *_q_q = this->self()) _q_q->setAccepted(accepted);
  else throwTypeError(QLatin1String("setAccepted")); }
bool isAccepted() const
{ if (QEvent *_q_q = this->self()) return _q_q->isAccepted();
  throwTypeError(QLatin1String("isAccepted")); return bool(); }
QString toString() const
{ return QLatin1String("QEvent"); }
private:
Q_DISABLE_COPY(Prototype_QEvent)
};

static QScriptValue create_QEvent_class(QScriptEngine *engine)
{
    Prototype_QEvent *pq = new Prototype_QEvent;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QEvent *cq = new Constructor_QEvent;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QEvent*>(), pv);
    _q_ScriptRegisterEnumMetaType<QEvent::Type>(engine);
    cv.setProperty("None", QScriptValue(engine, static_cast<int>(QEvent::None)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Timer", QScriptValue(engine, static_cast<int>(QEvent::Timer)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MouseButtonPress", QScriptValue(engine, static_cast<int>(QEvent::MouseButtonPress)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MouseButtonRelease", QScriptValue(engine, static_cast<int>(QEvent::MouseButtonRelease)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MouseButtonDblClick", QScriptValue(engine, static_cast<int>(QEvent::MouseButtonDblClick)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MouseMove", QScriptValue(engine, static_cast<int>(QEvent::MouseMove)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("KeyPress", QScriptValue(engine, static_cast<int>(QEvent::KeyPress)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("KeyRelease", QScriptValue(engine, static_cast<int>(QEvent::KeyRelease)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FocusIn", QScriptValue(engine, static_cast<int>(QEvent::FocusIn)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FocusOut", QScriptValue(engine, static_cast<int>(QEvent::FocusOut)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Enter", QScriptValue(engine, static_cast<int>(QEvent::Enter)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Leave", QScriptValue(engine, static_cast<int>(QEvent::Leave)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Paint", QScriptValue(engine, static_cast<int>(QEvent::Paint)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Move", QScriptValue(engine, static_cast<int>(QEvent::Move)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Resize", QScriptValue(engine, static_cast<int>(QEvent::Resize)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Create", QScriptValue(engine, static_cast<int>(QEvent::Create)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Destroy", QScriptValue(engine, static_cast<int>(QEvent::Destroy)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Show", QScriptValue(engine, static_cast<int>(QEvent::Show)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Hide", QScriptValue(engine, static_cast<int>(QEvent::Hide)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Close", QScriptValue(engine, static_cast<int>(QEvent::Close)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Quit", QScriptValue(engine, static_cast<int>(QEvent::Quit)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ParentChange", QScriptValue(engine, static_cast<int>(QEvent::ParentChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ParentAboutToChange", QScriptValue(engine, static_cast<int>(QEvent::ParentAboutToChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ThreadChange", QScriptValue(engine, static_cast<int>(QEvent::ThreadChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WindowActivate", QScriptValue(engine, static_cast<int>(QEvent::WindowActivate)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WindowDeactivate", QScriptValue(engine, static_cast<int>(QEvent::WindowDeactivate)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ShowToParent", QScriptValue(engine, static_cast<int>(QEvent::ShowToParent)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("HideToParent", QScriptValue(engine, static_cast<int>(QEvent::HideToParent)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Wheel", QScriptValue(engine, static_cast<int>(QEvent::Wheel)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WindowTitleChange", QScriptValue(engine, static_cast<int>(QEvent::WindowTitleChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WindowIconChange", QScriptValue(engine, static_cast<int>(QEvent::WindowIconChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ApplicationWindowIconChange", QScriptValue(engine, static_cast<int>(QEvent::ApplicationWindowIconChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ApplicationFontChange", QScriptValue(engine, static_cast<int>(QEvent::ApplicationFontChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ApplicationLayoutDirectionChange", QScriptValue(engine, static_cast<int>(QEvent::ApplicationLayoutDirectionChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ApplicationPaletteChange", QScriptValue(engine, static_cast<int>(QEvent::ApplicationPaletteChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("PaletteChange", QScriptValue(engine, static_cast<int>(QEvent::PaletteChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Clipboard", QScriptValue(engine, static_cast<int>(QEvent::Clipboard)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Speech", QScriptValue(engine, static_cast<int>(QEvent::Speech)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MetaCall", QScriptValue(engine, static_cast<int>(QEvent::MetaCall)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SockAct", QScriptValue(engine, static_cast<int>(QEvent::SockAct)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WinEventAct", QScriptValue(engine, static_cast<int>(QEvent::WinEventAct)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DeferredDelete", QScriptValue(engine, static_cast<int>(QEvent::DeferredDelete)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DragEnter", QScriptValue(engine, static_cast<int>(QEvent::DragEnter)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DragMove", QScriptValue(engine, static_cast<int>(QEvent::DragMove)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DragLeave", QScriptValue(engine, static_cast<int>(QEvent::DragLeave)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Drop", QScriptValue(engine, static_cast<int>(QEvent::Drop)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DragResponse", QScriptValue(engine, static_cast<int>(QEvent::DragResponse)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ChildAdded", QScriptValue(engine, static_cast<int>(QEvent::ChildAdded)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ChildPolished", QScriptValue(engine, static_cast<int>(QEvent::ChildPolished)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ChildRemoved", QScriptValue(engine, static_cast<int>(QEvent::ChildRemoved)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ShowWindowRequest", QScriptValue(engine, static_cast<int>(QEvent::ShowWindowRequest)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("PolishRequest", QScriptValue(engine, static_cast<int>(QEvent::PolishRequest)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Polish", QScriptValue(engine, static_cast<int>(QEvent::Polish)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LayoutRequest", QScriptValue(engine, static_cast<int>(QEvent::LayoutRequest)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("UpdateRequest", QScriptValue(engine, static_cast<int>(QEvent::UpdateRequest)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("UpdateLater", QScriptValue(engine, static_cast<int>(QEvent::UpdateLater)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("EmbeddingControl", QScriptValue(engine, static_cast<int>(QEvent::EmbeddingControl)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ActivateControl", QScriptValue(engine, static_cast<int>(QEvent::ActivateControl)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DeactivateControl", QScriptValue(engine, static_cast<int>(QEvent::DeactivateControl)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ContextMenu", QScriptValue(engine, static_cast<int>(QEvent::ContextMenu)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("InputMethod", QScriptValue(engine, static_cast<int>(QEvent::InputMethod)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AccessibilityPrepare", QScriptValue(engine, static_cast<int>(QEvent::AccessibilityPrepare)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TabletMove", QScriptValue(engine, static_cast<int>(QEvent::TabletMove)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LocaleChange", QScriptValue(engine, static_cast<int>(QEvent::LocaleChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LanguageChange", QScriptValue(engine, static_cast<int>(QEvent::LanguageChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LayoutDirectionChange", QScriptValue(engine, static_cast<int>(QEvent::LayoutDirectionChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Style", QScriptValue(engine, static_cast<int>(QEvent::Style)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TabletPress", QScriptValue(engine, static_cast<int>(QEvent::TabletPress)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TabletRelease", QScriptValue(engine, static_cast<int>(QEvent::TabletRelease)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("OkRequest", QScriptValue(engine, static_cast<int>(QEvent::OkRequest)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("HelpRequest", QScriptValue(engine, static_cast<int>(QEvent::HelpRequest)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("IconDrag", QScriptValue(engine, static_cast<int>(QEvent::IconDrag)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FontChange", QScriptValue(engine, static_cast<int>(QEvent::FontChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("EnabledChange", QScriptValue(engine, static_cast<int>(QEvent::EnabledChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ActivationChange", QScriptValue(engine, static_cast<int>(QEvent::ActivationChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("StyleChange", QScriptValue(engine, static_cast<int>(QEvent::StyleChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("IconTextChange", QScriptValue(engine, static_cast<int>(QEvent::IconTextChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ModifiedChange", QScriptValue(engine, static_cast<int>(QEvent::ModifiedChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MouseTrackingChange", QScriptValue(engine, static_cast<int>(QEvent::MouseTrackingChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WindowBlocked", QScriptValue(engine, static_cast<int>(QEvent::WindowBlocked)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WindowUnblocked", QScriptValue(engine, static_cast<int>(QEvent::WindowUnblocked)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WindowStateChange", QScriptValue(engine, static_cast<int>(QEvent::WindowStateChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ToolTip", QScriptValue(engine, static_cast<int>(QEvent::ToolTip)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WhatsThis", QScriptValue(engine, static_cast<int>(QEvent::WhatsThis)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("StatusTip", QScriptValue(engine, static_cast<int>(QEvent::StatusTip)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ActionChanged", QScriptValue(engine, static_cast<int>(QEvent::ActionChanged)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ActionAdded", QScriptValue(engine, static_cast<int>(QEvent::ActionAdded)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ActionRemoved", QScriptValue(engine, static_cast<int>(QEvent::ActionRemoved)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FileOpen", QScriptValue(engine, static_cast<int>(QEvent::FileOpen)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Shortcut", QScriptValue(engine, static_cast<int>(QEvent::Shortcut)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ShortcutOverride", QScriptValue(engine, static_cast<int>(QEvent::ShortcutOverride)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WhatsThisClicked", QScriptValue(engine, static_cast<int>(QEvent::WhatsThisClicked)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ToolBarChange", QScriptValue(engine, static_cast<int>(QEvent::ToolBarChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ApplicationActivate", QScriptValue(engine, static_cast<int>(QEvent::ApplicationActivate)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ApplicationActivated", QScriptValue(engine, static_cast<int>(QEvent::ApplicationActivated)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ApplicationDeactivate", QScriptValue(engine, static_cast<int>(QEvent::ApplicationDeactivate)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ApplicationDeactivated", QScriptValue(engine, static_cast<int>(QEvent::ApplicationDeactivated)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("QueryWhatsThis", QScriptValue(engine, static_cast<int>(QEvent::QueryWhatsThis)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("EnterWhatsThisMode", QScriptValue(engine, static_cast<int>(QEvent::EnterWhatsThisMode)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LeaveWhatsThisMode", QScriptValue(engine, static_cast<int>(QEvent::LeaveWhatsThisMode)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ZOrderChange", QScriptValue(engine, static_cast<int>(QEvent::ZOrderChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("HoverEnter", QScriptValue(engine, static_cast<int>(QEvent::HoverEnter)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("HoverLeave", QScriptValue(engine, static_cast<int>(QEvent::HoverLeave)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("HoverMove", QScriptValue(engine, static_cast<int>(QEvent::HoverMove)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AccessibilityHelp", QScriptValue(engine, static_cast<int>(QEvent::AccessibilityHelp)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AccessibilityDescription", QScriptValue(engine, static_cast<int>(QEvent::AccessibilityDescription)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AcceptDropsChange", QScriptValue(engine, static_cast<int>(QEvent::AcceptDropsChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MenubarUpdated", QScriptValue(engine, static_cast<int>(QEvent::MenubarUpdated)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ZeroTimerEvent", QScriptValue(engine, static_cast<int>(QEvent::ZeroTimerEvent)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneMouseMove", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneMouseMove)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneMousePress", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneMousePress)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneMouseRelease", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneMouseRelease)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneMouseDoubleClick", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneMouseDoubleClick)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneContextMenu", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneContextMenu)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneHoverEnter", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneHoverEnter)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneHoverMove", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneHoverMove)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneHoverLeave", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneHoverLeave)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneHelp", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneHelp)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneDragEnter", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneDragEnter)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneDragMove", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneDragMove)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneDragLeave", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneDragLeave)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneDrop", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneDrop)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("GraphicsSceneWheel", QScriptValue(engine, static_cast<int>(QEvent::GraphicsSceneWheel)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("KeyboardLayoutChange", QScriptValue(engine, static_cast<int>(QEvent::KeyboardLayoutChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DynamicPropertyChange", QScriptValue(engine, static_cast<int>(QEvent::DynamicPropertyChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TabletEnterProximity", QScriptValue(engine, static_cast<int>(QEvent::TabletEnterProximity)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TabletLeaveProximity", QScriptValue(engine, static_cast<int>(QEvent::TabletLeaveProximity)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NonClientAreaMouseMove", QScriptValue(engine, static_cast<int>(QEvent::NonClientAreaMouseMove)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NonClientAreaMouseButtonPress", QScriptValue(engine, static_cast<int>(QEvent::NonClientAreaMouseButtonPress)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NonClientAreaMouseButtonRelease", QScriptValue(engine, static_cast<int>(QEvent::NonClientAreaMouseButtonRelease)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NonClientAreaMouseButtonDblClick", QScriptValue(engine, static_cast<int>(QEvent::NonClientAreaMouseButtonDblClick)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MacSizeChange", QScriptValue(engine, static_cast<int>(QEvent::MacSizeChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ContentsRectChange", QScriptValue(engine, static_cast<int>(QEvent::ContentsRectChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MacGLWindowChange", QScriptValue(engine, static_cast<int>(QEvent::MacGLWindowChange)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("User", QScriptValue(engine, static_cast<int>(QEvent::User)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MaxUser", QScriptValue(engine, static_cast<int>(QEvent::MaxUser)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QTextDecoder

Q_DECLARE_METATYPE(QTextDecoder*)

class Constructor_QTextDecoder:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QTextCodec const* codec)
{ return this->engine()->toScriptValue(new QTextDecoder(codec)); }
};

class Prototype_QTextDecoder:
public QObject, public QTextDecoder, protected QScriptable
{
    Q_OBJECT
private:
inline QTextDecoder *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QTextDecoder*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QTextDecoder.prototype.%0: this object is not a QTextDecoder")
            .arg(method));
}
public:
Prototype_QTextDecoder(QObject *parent = 0)
    : QTextDecoder(0) { setParent(parent); }
public Q_SLOTS:
void toUnicode(QString* target, char const* chars, int len)
{ if (QTextDecoder *_q_q = this->self()) _q_q->toUnicode(target, chars, len);
  else throwTypeError(QLatin1String("toUnicode")); }
QString toUnicode(QByteArray const& ba)
{ if (QTextDecoder *_q_q = this->self()) return _q_q->toUnicode(ba);
  throwTypeError(QLatin1String("toUnicode")); return QString(); }
QString toUnicode(char const* chars, int len)
{ if (QTextDecoder *_q_q = this->self()) return _q_q->toUnicode(chars, len);
  throwTypeError(QLatin1String("toUnicode")); return QString(); }
bool hasFailure() const
{ if (QTextDecoder *_q_q = this->self()) return _q_q->hasFailure();
  throwTypeError(QLatin1String("hasFailure")); return bool(); }
QString toString() const
{ return QLatin1String("QTextDecoder"); }
private:
Q_DISABLE_COPY(Prototype_QTextDecoder)
};

static QScriptValue create_QTextDecoder_class(QScriptEngine *engine)
{
    Prototype_QTextDecoder *pq = new Prototype_QTextDecoder;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QTextDecoder *cq = new Constructor_QTextDecoder;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QTextDecoder*>(), pv);
    return cv;
}

// QLibraryInfo

Q_DECLARE_METATYPE(QLibraryInfo*)
Q_DECLARE_METATYPE(QLibraryInfo::LibraryLocation)

class Constructor_QLibraryInfo:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QString licensedProducts()
{ return QLibraryInfo::licensedProducts(); }
QString location(QLibraryInfo::LibraryLocation arg1)
{ return QLibraryInfo::location(arg1); }
QString licensee()
{ return QLibraryInfo::licensee(); }
QString buildKey()
{ return QLibraryInfo::buildKey(); }
};

static QScriptValue create_QLibraryInfo_class(QScriptEngine *engine)
{
    Constructor_QLibraryInfo *cq = new Constructor_QLibraryInfo;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    _q_ScriptRegisterEnumMetaType<QLibraryInfo::LibraryLocation>(engine);
    cv.setProperty("PrefixPath", QScriptValue(engine, static_cast<int>(QLibraryInfo::PrefixPath)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DocumentationPath", QScriptValue(engine, static_cast<int>(QLibraryInfo::DocumentationPath)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("HeadersPath", QScriptValue(engine, static_cast<int>(QLibraryInfo::HeadersPath)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LibrariesPath", QScriptValue(engine, static_cast<int>(QLibraryInfo::LibrariesPath)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("BinariesPath", QScriptValue(engine, static_cast<int>(QLibraryInfo::BinariesPath)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("PluginsPath", QScriptValue(engine, static_cast<int>(QLibraryInfo::PluginsPath)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DataPath", QScriptValue(engine, static_cast<int>(QLibraryInfo::DataPath)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TranslationsPath", QScriptValue(engine, static_cast<int>(QLibraryInfo::TranslationsPath)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SettingsPath", QScriptValue(engine, static_cast<int>(QLibraryInfo::SettingsPath)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DemosPath", QScriptValue(engine, static_cast<int>(QLibraryInfo::DemosPath)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ExamplesPath", QScriptValue(engine, static_cast<int>(QLibraryInfo::ExamplesPath)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QTextCodec

Q_DECLARE_METATYPE(QTextCodec*)
Q_DECLARE_METATYPE(QTextCodec::ConversionFlag)
Q_DECLARE_METATYPE(QTextCodec::ConversionFlags)

class Constructor_QTextCodec:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QTextCodec* codecForName(char const* name)
{ return QTextCodec::codecForName(name); }
QTextCodec* codecForName(QByteArray const& name)
{ return QTextCodec::codecForName(name); }
void setCodecForCStrings(QTextCodec* c)
{ QTextCodec::setCodecForCStrings(c); }
QTextCodec* codecForTr()
{ return QTextCodec::codecForTr(); }
QTextCodec* codecForMib(int mib)
{ return QTextCodec::codecForMib(mib); }
QTextCodec* codecForLocale()
{ return QTextCodec::codecForLocale(); }
QList<QByteArray> availableCodecs()
{ return QTextCodec::availableCodecs(); }
QTextCodec* codecForCStrings()
{ return QTextCodec::codecForCStrings(); }
void setCodecForTr(QTextCodec* c)
{ QTextCodec::setCodecForTr(c); }
QList<int> availableMibs()
{ return QTextCodec::availableMibs(); }
void setCodecForLocale(QTextCodec* c)
{ QTextCodec::setCodecForLocale(c); }
QTextCodec* codecForHtml(QByteArray const& ba)
{ return QTextCodec::codecForHtml(ba); }
};

class Prototype_QTextCodec:
public QObject, protected QScriptable
{
    Q_OBJECT
private:
inline QTextCodec *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QTextCodec*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QTextCodec.prototype.%0: this object is not a QTextCodec")
            .arg(method));
}
public:
Prototype_QTextCodec(QObject *parent = 0)
 { setParent(parent); }
public Q_SLOTS:
void setCodecForCStrings(QTextCodec* c)
{ if (QTextCodec *_q_q = this->self()) _q_q->setCodecForCStrings(c);
  else throwTypeError(QLatin1String("setCodecForCStrings")); }
bool canEncode(QString const& arg1) const
{ if (QTextCodec *_q_q = this->self()) return _q_q->canEncode(arg1);
  throwTypeError(QLatin1String("canEncode")); return bool(); }
bool canEncode(QChar arg1) const
{ if (QTextCodec *_q_q = this->self()) return _q_q->canEncode(arg1);
  throwTypeError(QLatin1String("canEncode")); return bool(); }
QByteArray fromUnicode(QChar const* in, int length, QTextCodec::ConverterState* state = 0) const
{ if (QTextCodec *_q_q = this->self()) return _q_q->fromUnicode(in, length, state);
  throwTypeError(QLatin1String("fromUnicode")); return QByteArray(); }
QByteArray fromUnicode(QString const& uc) const
{ if (QTextCodec *_q_q = this->self()) return _q_q->fromUnicode(uc);
  throwTypeError(QLatin1String("fromUnicode")); return QByteArray(); }
QTextCodec* codecForTr()
{ if (QTextCodec *_q_q = this->self()) return _q_q->codecForTr();
  throwTypeError(QLatin1String("codecForTr")); return 0; }
QTextEncoder* makeEncoder() const
{ if (QTextCodec *_q_q = this->self()) return _q_q->makeEncoder();
  throwTypeError(QLatin1String("makeEncoder")); return 0; }
QTextDecoder* makeDecoder() const
{ if (QTextCodec *_q_q = this->self()) return _q_q->makeDecoder();
  throwTypeError(QLatin1String("makeDecoder")); return 0; }
int mibEnum() const
{ if (QTextCodec *_q_q = this->self()) return _q_q->mibEnum();
  throwTypeError(QLatin1String("mibEnum")); return int(); }
QByteArray name() const
{ if (QTextCodec *_q_q = this->self()) return _q_q->name();
  throwTypeError(QLatin1String("name")); return QByteArray(); }
QTextCodec* codecForCStrings()
{ if (QTextCodec *_q_q = this->self()) return _q_q->codecForCStrings();
  throwTypeError(QLatin1String("codecForCStrings")); return 0; }
void setCodecForTr(QTextCodec* c)
{ if (QTextCodec *_q_q = this->self()) _q_q->setCodecForTr(c);
  else throwTypeError(QLatin1String("setCodecForTr")); }
QString toUnicode(char const* in, int length, QTextCodec::ConverterState* state = 0) const
{ if (QTextCodec *_q_q = this->self()) return _q_q->toUnicode(in, length, state);
  throwTypeError(QLatin1String("toUnicode")); return QString(); }
QString toUnicode(char const* chars) const
{ if (QTextCodec *_q_q = this->self()) return _q_q->toUnicode(chars);
  throwTypeError(QLatin1String("toUnicode")); return QString(); }
QString toUnicode(QByteArray const& arg1) const
{ if (QTextCodec *_q_q = this->self()) return _q_q->toUnicode(arg1);
  throwTypeError(QLatin1String("toUnicode")); return QString(); }
QList<QByteArray> aliases() const
{ if (QTextCodec *_q_q = this->self()) return _q_q->aliases();
  throwTypeError(QLatin1String("aliases")); return QList<QByteArray>(); }
QString toString() const
{ return QLatin1String("QTextCodec"); }
public:
QByteArray convertFromUnicode(QChar const* /* in */, int /* length */, QTextCodec::ConverterState* /* state */) const { return QByteArray(); }
QString convertToUnicode(char const* /* in */, int /* length */, QTextCodec::ConverterState* /* state */) const { return QString(); }
private:
Q_DISABLE_COPY(Prototype_QTextCodec)
};

static QScriptValue create_QTextCodec_class(QScriptEngine *engine)
{
    Prototype_QTextCodec *pq = new Prototype_QTextCodec;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QTextCodec *cq = new Constructor_QTextCodec;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QTextCodec*>(), pv);
    _q_ScriptRegisterEnumMetaType<QTextCodec::ConversionFlag>(engine);
    cv.setProperty("DefaultConversion", QScriptValue(engine, static_cast<int>(QTextCodec::DefaultConversion)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ConvertInvalidToNull", QScriptValue(engine, static_cast<int>(QTextCodec::ConvertInvalidToNull)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("IgnoreHeader", QScriptValue(engine, static_cast<int>(QTextCodec::IgnoreHeader)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QTextCodec::ConversionFlags>(engine);
    return cv;
}

// QTextStream

Q_DECLARE_METATYPE(QTextStream*)
Q_DECLARE_METATYPE(QTextStream::RealNumberNotation)
Q_DECLARE_METATYPE(QTextStream::NumberFlag)
Q_DECLARE_METATYPE(QTextStream::FieldAlignment)
Q_DECLARE_METATYPE(QTextStream::Status)
Q_DECLARE_METATYPE(QTextStream::NumberFlags)

class Constructor_QTextStream:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QByteArray const& array, QIODevice::OpenMode openMode = QIODevice::ReadOnly)
{ return this->engine()->toScriptValue(new QTextStream(array, openMode)); }
QScriptValue qscript_call(QByteArray* array, QIODevice::OpenMode openMode = QIODevice::ReadWrite)
{ return this->engine()->toScriptValue(new QTextStream(array, openMode)); }
QScriptValue qscript_call(QString* string, QIODevice::OpenMode openMode = QIODevice::ReadWrite)
{ return this->engine()->toScriptValue(new QTextStream(string, openMode)); }
QScriptValue qscript_call(FILE* fileHandle, QIODevice::OpenMode openMode = QIODevice::ReadWrite)
{ return this->engine()->toScriptValue(new QTextStream(fileHandle, openMode)); }
QScriptValue qscript_call(QIODevice* device)
{ return this->engine()->toScriptValue(new QTextStream(device)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(new QTextStream()); }
};

class Prototype_QTextStream:
public QObject, public QTextStream, protected QScriptable
{
    Q_OBJECT
private:
inline QTextStream *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QTextStream*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QTextStream.prototype.%0: this object is not a QTextStream")
            .arg(method));
}
public:
Prototype_QTextStream(QObject *parent = 0)
    : QTextStream() { setParent(parent); }
public Q_SLOTS:
int fieldWidth() const
{ if (QTextStream *_q_q = this->self()) return _q_q->fieldWidth();
  throwTypeError(QLatin1String("fieldWidth")); return int(); }
QIODevice* device() const
{ if (QTextStream *_q_q = this->self()) return _q_q->device();
  throwTypeError(QLatin1String("device")); return 0; }
QString readLine(qint64 maxlen = 0)
{ if (QTextStream *_q_q = this->self()) return _q_q->readLine(maxlen);
  throwTypeError(QLatin1String("readLine")); return QString(); }
int integerBase() const
{ if (QTextStream *_q_q = this->self()) return _q_q->integerBase();
  throwTypeError(QLatin1String("integerBase")); return int(); }
void setGenerateByteOrderMark(bool generate)
{ if (QTextStream *_q_q = this->self()) _q_q->setGenerateByteOrderMark(generate);
  else throwTypeError(QLatin1String("setGenerateByteOrderMark")); }
void setIntegerBase(int base)
{ if (QTextStream *_q_q = this->self()) _q_q->setIntegerBase(base);
  else throwTypeError(QLatin1String("setIntegerBase")); }
int realNumberPrecision() const
{ if (QTextStream *_q_q = this->self()) return _q_q->realNumberPrecision();
  throwTypeError(QLatin1String("realNumberPrecision")); return int(); }
QTextStream::FieldAlignment fieldAlignment() const
{ if (QTextStream *_q_q = this->self()) return _q_q->fieldAlignment();
  throwTypeError(QLatin1String("fieldAlignment")); return QTextStream::FieldAlignment(); }
void skipWhiteSpace()
{ if (QTextStream *_q_q = this->self()) _q_q->skipWhiteSpace();
  else throwTypeError(QLatin1String("skipWhiteSpace")); }
bool autoDetectUnicode() const
{ if (QTextStream *_q_q = this->self()) return _q_q->autoDetectUnicode();
  throwTypeError(QLatin1String("autoDetectUnicode")); return bool(); }
bool seek(qint64 pos)
{ if (QTextStream *_q_q = this->self()) return _q_q->seek(pos);
  throwTypeError(QLatin1String("seek")); return bool(); }
void reset()
{ if (QTextStream *_q_q = this->self()) _q_q->reset();
  else throwTypeError(QLatin1String("reset")); }
void setNumberFlags(QTextStream::NumberFlags flags)
{ if (QTextStream *_q_q = this->self()) _q_q->setNumberFlags(flags);
  else throwTypeError(QLatin1String("setNumberFlags")); }
QTextStream::Status status() const
{ if (QTextStream *_q_q = this->self()) return _q_q->status();
  throwTypeError(QLatin1String("status")); return QTextStream::Status(); }
qint64 pos() const
{ if (QTextStream *_q_q = this->self()) return _q_q->pos();
  throwTypeError(QLatin1String("pos")); return qint64(); }
QTextStream::RealNumberNotation realNumberNotation() const
{ if (QTextStream *_q_q = this->self()) return _q_q->realNumberNotation();
  throwTypeError(QLatin1String("realNumberNotation")); return QTextStream::RealNumberNotation(); }
void setCodec(char const* codecName)
{ if (QTextStream *_q_q = this->self()) _q_q->setCodec(codecName);
  else throwTypeError(QLatin1String("setCodec")); }
void setCodec(QTextCodec* codec)
{ if (QTextStream *_q_q = this->self()) _q_q->setCodec(codec);
  else throwTypeError(QLatin1String("setCodec")); }
void flush()
{ if (QTextStream *_q_q = this->self()) _q_q->flush();
  else throwTypeError(QLatin1String("flush")); }
QTextCodec* codec() const
{ if (QTextStream *_q_q = this->self()) return _q_q->codec();
  throwTypeError(QLatin1String("codec")); return 0; }
QString* string() const
{ if (QTextStream *_q_q = this->self()) return _q_q->string();
  throwTypeError(QLatin1String("string")); return 0; }
void setPadChar(QChar ch)
{ if (QTextStream *_q_q = this->self()) _q_q->setPadChar(ch);
  else throwTypeError(QLatin1String("setPadChar")); }
QString readAll()
{ if (QTextStream *_q_q = this->self()) return _q_q->readAll();
  throwTypeError(QLatin1String("readAll")); return QString(); }
QTextStream::NumberFlags numberFlags() const
{ if (QTextStream *_q_q = this->self()) return _q_q->numberFlags();
  throwTypeError(QLatin1String("numberFlags")); return QTextStream::NumberFlags(); }
QString read(qint64 maxlen)
{ if (QTextStream *_q_q = this->self()) return _q_q->read(maxlen);
  throwTypeError(QLatin1String("read")); return QString(); }
void setDevice(QIODevice* device)
{ if (QTextStream *_q_q = this->self()) _q_q->setDevice(device);
  else throwTypeError(QLatin1String("setDevice")); }
void setFieldWidth(int width)
{ if (QTextStream *_q_q = this->self()) _q_q->setFieldWidth(width);
  else throwTypeError(QLatin1String("setFieldWidth")); }
void resetStatus()
{ if (QTextStream *_q_q = this->self()) _q_q->resetStatus();
  else throwTypeError(QLatin1String("resetStatus")); }
QChar padChar() const
{ if (QTextStream *_q_q = this->self()) return _q_q->padChar();
  throwTypeError(QLatin1String("padChar")); return QChar(); }
bool atEnd() const
{ if (QTextStream *_q_q = this->self()) return _q_q->atEnd();
  throwTypeError(QLatin1String("atEnd")); return bool(); }
void setRealNumberNotation(QTextStream::RealNumberNotation notation)
{ if (QTextStream *_q_q = this->self()) _q_q->setRealNumberNotation(notation);
  else throwTypeError(QLatin1String("setRealNumberNotation")); }
void setFieldAlignment(QTextStream::FieldAlignment alignment)
{ if (QTextStream *_q_q = this->self()) _q_q->setFieldAlignment(alignment);
  else throwTypeError(QLatin1String("setFieldAlignment")); }
void setRealNumberPrecision(int precision)
{ if (QTextStream *_q_q = this->self()) _q_q->setRealNumberPrecision(precision);
  else throwTypeError(QLatin1String("setRealNumberPrecision")); }
bool generateByteOrderMark() const
{ if (QTextStream *_q_q = this->self()) return _q_q->generateByteOrderMark();
  throwTypeError(QLatin1String("generateByteOrderMark")); return bool(); }
void setString(QString* string, QIODevice::OpenMode openMode = QIODevice::ReadWrite)
{ if (QTextStream *_q_q = this->self()) _q_q->setString(string, openMode);
  else throwTypeError(QLatin1String("setString")); }
void setStatus(QTextStream::Status status)
{ if (QTextStream *_q_q = this->self()) _q_q->setStatus(status);
  else throwTypeError(QLatin1String("setStatus")); }
void setAutoDetectUnicode(bool enabled)
{ if (QTextStream *_q_q = this->self()) _q_q->setAutoDetectUnicode(enabled);
  else throwTypeError(QLatin1String("setAutoDetectUnicode")); }
QString toString() const
{ return QLatin1String("QTextStream"); }
private:
Q_DISABLE_COPY(Prototype_QTextStream)
};

static QScriptValue create_QTextStream_class(QScriptEngine *engine)
{
    Prototype_QTextStream *pq = new Prototype_QTextStream;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QTextStream *cq = new Constructor_QTextStream;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QTextStream*>(), pv);
    _q_ScriptRegisterEnumMetaType<QTextStream::RealNumberNotation>(engine);
    cv.setProperty("SmartNotation", QScriptValue(engine, static_cast<int>(QTextStream::SmartNotation)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FixedNotation", QScriptValue(engine, static_cast<int>(QTextStream::FixedNotation)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ScientificNotation", QScriptValue(engine, static_cast<int>(QTextStream::ScientificNotation)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QTextStream::NumberFlag>(engine);
    cv.setProperty("ShowBase", QScriptValue(engine, static_cast<int>(QTextStream::ShowBase)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ForcePoint", QScriptValue(engine, static_cast<int>(QTextStream::ForcePoint)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ForceSign", QScriptValue(engine, static_cast<int>(QTextStream::ForceSign)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("UppercaseBase", QScriptValue(engine, static_cast<int>(QTextStream::UppercaseBase)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("UppercaseDigits", QScriptValue(engine, static_cast<int>(QTextStream::UppercaseDigits)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QTextStream::FieldAlignment>(engine);
    cv.setProperty("AlignLeft", QScriptValue(engine, static_cast<int>(QTextStream::AlignLeft)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AlignRight", QScriptValue(engine, static_cast<int>(QTextStream::AlignRight)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AlignCenter", QScriptValue(engine, static_cast<int>(QTextStream::AlignCenter)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AlignAccountingStyle", QScriptValue(engine, static_cast<int>(QTextStream::AlignAccountingStyle)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QTextStream::Status>(engine);
    cv.setProperty("Ok", QScriptValue(engine, static_cast<int>(QTextStream::Ok)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadPastEnd", QScriptValue(engine, static_cast<int>(QTextStream::ReadPastEnd)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadCorruptData", QScriptValue(engine, static_cast<int>(QTextStream::ReadCorruptData)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QTextStream::NumberFlags>(engine);
    return cv;
}

// QAbstractFileEngineHandler

Q_DECLARE_METATYPE(QAbstractFileEngineHandler*)

class Constructor_QAbstractFileEngineHandler:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call()
{ return this->context()->throwError(QString::fromLatin1("QAbstractFileEngineHandler cannot be instantiated")); }
};

class Prototype_QAbstractFileEngineHandler:
public QObject, protected QScriptable
{
    Q_OBJECT
private:
inline QAbstractFileEngineHandler *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QAbstractFileEngineHandler*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QAbstractFileEngineHandler.prototype.%0: this object is not a QAbstractFileEngineHandler")
            .arg(method));
}
public:
Prototype_QAbstractFileEngineHandler(QObject *parent = 0)
 { setParent(parent); }
public Q_SLOTS:
QAbstractFileEngine* create(QString const& fileName) const
{ if (QAbstractFileEngineHandler *_q_q = this->self()) return _q_q->create(fileName);
  throwTypeError(QLatin1String("create")); return 0; }
QString toString() const
{ return QLatin1String("QAbstractFileEngineHandler"); }
public:
private:
Q_DISABLE_COPY(Prototype_QAbstractFileEngineHandler)
};

static QScriptValue create_QAbstractFileEngineHandler_class(QScriptEngine *engine)
{
    Prototype_QAbstractFileEngineHandler *pq = new Prototype_QAbstractFileEngineHandler;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QAbstractFileEngineHandler *cq = new Constructor_QAbstractFileEngineHandler;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QAbstractFileEngineHandler*>(), pv);
    return cv;
}

// QReadWriteLock

Q_DECLARE_METATYPE(QReadWriteLock*)

class Constructor_QReadWriteLock:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(new QReadWriteLock()); }
};

class Prototype_QReadWriteLock:
public QObject, public QReadWriteLock, protected QScriptable
{
    Q_OBJECT
private:
inline QReadWriteLock *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QReadWriteLock*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QReadWriteLock.prototype.%0: this object is not a QReadWriteLock")
            .arg(method));
}
public:
Prototype_QReadWriteLock(QObject *parent = 0)
    : QReadWriteLock() { setParent(parent); }
public Q_SLOTS:
void unlock()
{ if (QReadWriteLock *_q_q = this->self()) _q_q->unlock();
  else throwTypeError(QLatin1String("unlock")); }
void lockForWrite()
{ if (QReadWriteLock *_q_q = this->self()) _q_q->lockForWrite();
  else throwTypeError(QLatin1String("lockForWrite")); }
void lockForRead()
{ if (QReadWriteLock *_q_q = this->self()) _q_q->lockForRead();
  else throwTypeError(QLatin1String("lockForRead")); }
bool tryLockForRead(int timeout)
{ if (QReadWriteLock *_q_q = this->self()) return _q_q->tryLockForRead(timeout);
  throwTypeError(QLatin1String("tryLockForRead")); return bool(); }
bool tryLockForRead()
{ if (QReadWriteLock *_q_q = this->self()) return _q_q->tryLockForRead();
  throwTypeError(QLatin1String("tryLockForRead")); return bool(); }
bool tryLockForWrite(int timeout)
{ if (QReadWriteLock *_q_q = this->self()) return _q_q->tryLockForWrite(timeout);
  throwTypeError(QLatin1String("tryLockForWrite")); return bool(); }
bool tryLockForWrite()
{ if (QReadWriteLock *_q_q = this->self()) return _q_q->tryLockForWrite();
  throwTypeError(QLatin1String("tryLockForWrite")); return bool(); }
QString toString() const
{ return QLatin1String("QReadWriteLock"); }
private:
Q_DISABLE_COPY(Prototype_QReadWriteLock)
};

static QScriptValue create_QReadWriteLock_class(QScriptEngine *engine)
{
    Prototype_QReadWriteLock *pq = new Prototype_QReadWriteLock;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QReadWriteLock *cq = new Constructor_QReadWriteLock;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QReadWriteLock*>(), pv);
    return cv;
}

// QModelIndex

Q_DECLARE_METATYPE(QModelIndex)
Q_DECLARE_METATYPE(QModelIndex*)

class Constructor_QModelIndex:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QModelIndex const& other)
{ return this->engine()->toScriptValue(QModelIndex(other)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QModelIndex()); }
};

class Prototype_QModelIndex:
public QObject, public QModelIndex, protected QScriptable
{
    Q_OBJECT
private:
inline QModelIndex *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QModelIndex*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QModelIndex.prototype.%0: this object is not a QModelIndex")
            .arg(method));
}
public:
Prototype_QModelIndex(QObject *parent = 0)
    : QModelIndex() { setParent(parent); }
public Q_SLOTS:
QAbstractItemModel const* model() const
{ if (QModelIndex *_q_q = this->self()) return _q_q->model();
  throwTypeError(QLatin1String("model")); return 0; }
bool isValid() const
{ if (QModelIndex *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
QModelIndex child(int row, int column) const
{ if (QModelIndex *_q_q = this->self()) return _q_q->child(row, column);
  throwTypeError(QLatin1String("child")); return QModelIndex(); }
QModelIndex parent() const
{ if (QModelIndex *_q_q = this->self()) return _q_q->parent();
  throwTypeError(QLatin1String("parent")); return QModelIndex(); }
QVariant data(int role = Qt::DisplayRole) const
{ if (QModelIndex *_q_q = this->self()) return _q_q->data(role);
  throwTypeError(QLatin1String("data")); return QVariant(); }
bool lessThan(QModelIndex const& other) const
{ if (QModelIndex *_q_q = this->self()) return _q_q->operator<(other);
  throwTypeError(QLatin1String("lessThan")); return bool(); }
qint64 internalId() const
{ if (QModelIndex *_q_q = this->self()) return _q_q->internalId();
  throwTypeError(QLatin1String("internalId")); return qint64(); }
bool equals(QModelIndex const& other) const
{ if (QModelIndex *_q_q = this->self()) return _q_q->operator==(other);
  throwTypeError(QLatin1String("equals")); return bool(); }
Qt::ItemFlags flags() const
{ if (QModelIndex *_q_q = this->self()) return _q_q->flags();
  throwTypeError(QLatin1String("flags")); return Qt::ItemFlags(); }
void* internalPointer() const
{ if (QModelIndex *_q_q = this->self()) return _q_q->internalPointer();
  throwTypeError(QLatin1String("internalPointer")); return 0; }
QModelIndex sibling(int row, int column) const
{ if (QModelIndex *_q_q = this->self()) return _q_q->sibling(row, column);
  throwTypeError(QLatin1String("sibling")); return QModelIndex(); }
int column() const
{ if (QModelIndex *_q_q = this->self()) return _q_q->column();
  throwTypeError(QLatin1String("column")); return int(); }
int row() const
{ if (QModelIndex *_q_q = this->self()) return _q_q->row();
  throwTypeError(QLatin1String("row")); return int(); }
QString toString() const
{ return QLatin1String("QModelIndex"); }
};

static QScriptValue create_QModelIndex_class(QScriptEngine *engine)
{
    Prototype_QModelIndex *pq = new Prototype_QModelIndex;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QModelIndex *cq = new Constructor_QModelIndex;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QModelIndex>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QModelIndex>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QModelIndex*>(), pv);
    return cv;
}

// QRegExp

Q_DECLARE_METATYPE(QRegExp)
Q_DECLARE_METATYPE(QRegExp*)
Q_DECLARE_METATYPE(QRegExp::PatternSyntax)
Q_DECLARE_METATYPE(QRegExp::CaretMode)

class Constructor_QRegExp:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QRegExp const& rx)
{ return this->engine()->toScriptValue(QRegExp(rx)); }
QScriptValue qscript_call(QString const& pattern, Qt::CaseSensitivity cs = Qt::CaseSensitive, QRegExp::PatternSyntax syntax = QRegExp::RegExp)
{ return this->engine()->toScriptValue(QRegExp(pattern, cs, syntax)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QRegExp()); }
QString escape(QString const& str)
{ return QRegExp::escape(str); }
};

class Prototype_QRegExp:
public QObject, public QRegExp, protected QScriptable
{
    Q_OBJECT
private:
inline QRegExp *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QRegExp*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QRegExp.prototype.%0: this object is not a QRegExp")
            .arg(method));
}
public:
Prototype_QRegExp(QObject *parent = 0)
    : QRegExp() { setParent(parent); }
public Q_SLOTS:
void setPatternSyntax(QRegExp::PatternSyntax syntax)
{ if (QRegExp *_q_q = this->self()) _q_q->setPatternSyntax(syntax);
  else throwTypeError(QLatin1String("setPatternSyntax")); }
int pos(int nth = 0)
{ if (QRegExp *_q_q = this->self()) return _q_q->pos(nth);
  throwTypeError(QLatin1String("pos")); return int(); }
bool isValid() const
{ if (QRegExp *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
QStringList capturedTexts()
{ if (QRegExp *_q_q = this->self()) return _q_q->capturedTexts();
  throwTypeError(QLatin1String("capturedTexts")); return QStringList(); }
int numCaptures() const
{ if (QRegExp *_q_q = this->self()) return _q_q->numCaptures();
  throwTypeError(QLatin1String("numCaptures")); return int(); }
int indexIn(QString const& str, int offset = 0, QRegExp::CaretMode caretMode = QRegExp::CaretAtZero) const
{ if (QRegExp *_q_q = this->self()) return _q_q->indexIn(str, offset, caretMode);
  throwTypeError(QLatin1String("indexIn")); return int(); }
void setPattern(QString const& pattern)
{ if (QRegExp *_q_q = this->self()) _q_q->setPattern(pattern);
  else throwTypeError(QLatin1String("setPattern")); }
Qt::CaseSensitivity caseSensitivity() const
{ if (QRegExp *_q_q = this->self()) return _q_q->caseSensitivity();
  throwTypeError(QLatin1String("caseSensitivity")); return Qt::CaseSensitivity(); }
void setCaseSensitivity(Qt::CaseSensitivity cs)
{ if (QRegExp *_q_q = this->self()) _q_q->setCaseSensitivity(cs);
  else throwTypeError(QLatin1String("setCaseSensitivity")); }
QString pattern() const
{ if (QRegExp *_q_q = this->self()) return _q_q->pattern();
  throwTypeError(QLatin1String("pattern")); return QString(); }
bool equals(QRegExp const& rx) const
{ if (QRegExp *_q_q = this->self()) return _q_q->operator==(rx);
  throwTypeError(QLatin1String("equals")); return bool(); }
int matchedLength() const
{ if (QRegExp *_q_q = this->self()) return _q_q->matchedLength();
  throwTypeError(QLatin1String("matchedLength")); return int(); }
bool exactMatch(QString const& str) const
{ if (QRegExp *_q_q = this->self()) return _q_q->exactMatch(str);
  throwTypeError(QLatin1String("exactMatch")); return bool(); }
bool isMinimal() const
{ if (QRegExp *_q_q = this->self()) return _q_q->isMinimal();
  throwTypeError(QLatin1String("isMinimal")); return bool(); }
bool isEmpty() const
{ if (QRegExp *_q_q = this->self()) return _q_q->isEmpty();
  throwTypeError(QLatin1String("isEmpty")); return bool(); }
QString errorString()
{ if (QRegExp *_q_q = this->self()) return _q_q->errorString();
  throwTypeError(QLatin1String("errorString")); return QString(); }
QRegExp::PatternSyntax patternSyntax() const
{ if (QRegExp *_q_q = this->self()) return _q_q->patternSyntax();
  throwTypeError(QLatin1String("patternSyntax")); return QRegExp::PatternSyntax(); }
QString cap(int nth = 0)
{ if (QRegExp *_q_q = this->self()) return _q_q->cap(nth);
  throwTypeError(QLatin1String("cap")); return QString(); }
void setMinimal(bool minimal)
{ if (QRegExp *_q_q = this->self()) _q_q->setMinimal(minimal);
  else throwTypeError(QLatin1String("setMinimal")); }
int lastIndexIn(QString const& str, int offset = -1, QRegExp::CaretMode caretMode = QRegExp::CaretAtZero) const
{ if (QRegExp *_q_q = this->self()) return _q_q->lastIndexIn(str, offset, caretMode);
  throwTypeError(QLatin1String("lastIndexIn")); return int(); }
QString toString() const
{ return QLatin1String("QRegExp"); }
};

static QScriptValue create_QRegExp_class(QScriptEngine *engine)
{
    Prototype_QRegExp *pq = new Prototype_QRegExp;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QRegExp *cq = new Constructor_QRegExp;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QRegExp>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QRegExp>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QRegExp*>(), pv);
    _q_ScriptRegisterEnumMetaType<QRegExp::PatternSyntax>(engine);
    cv.setProperty("RegExp", QScriptValue(engine, static_cast<int>(QRegExp::RegExp)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Wildcard", QScriptValue(engine, static_cast<int>(QRegExp::Wildcard)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FixedString", QScriptValue(engine, static_cast<int>(QRegExp::FixedString)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RegExp2", QScriptValue(engine, static_cast<int>(QRegExp::RegExp2)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QRegExp::CaretMode>(engine);
    cv.setProperty("CaretAtZero", QScriptValue(engine, static_cast<int>(QRegExp::CaretAtZero)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CaretAtOffset", QScriptValue(engine, static_cast<int>(QRegExp::CaretAtOffset)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CaretWontMatch", QScriptValue(engine, static_cast<int>(QRegExp::CaretWontMatch)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QDate

Q_DECLARE_METATYPE(QDate)
Q_DECLARE_METATYPE(QDate*)

class Constructor_QDate:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(int y, int m, int d)
{ return this->engine()->toScriptValue(QDate(y, m, d)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QDate()); }
QDate fromString(QString const& s, QString const& format)
{ return QDate::fromString(s, format); }
QDate fromString(QString const& s, Qt::DateFormat f = Qt::TextDate)
{ return QDate::fromString(s, f); }
QString shortMonthName(int month)
{ return QDate::shortMonthName(month); }
void julianToGregorian(uint jd, int& y, int& m, int& d)
{ QDate::julianToGregorian(jd, y, m, d); }
QDate fromJulianDay(int jd)
{ return QDate::fromJulianDay(jd); }
QDate currentDate()
{ return QDate::currentDate(); }
QString longDayName(int weekday)
{ return QDate::longDayName(weekday); }
uint gregorianToJulian(int y, int m, int d)
{ return QDate::gregorianToJulian(y, m, d); }
QString shortDayName(int weekday)
{ return QDate::shortDayName(weekday); }
QString longMonthName(int month)
{ return QDate::longMonthName(month); }
bool isLeapYear(int year)
{ return QDate::isLeapYear(year); }
bool isValid(int y, int m, int d)
{ return QDate::isValid(y, m, d); }
};

class Prototype_QDate:
public QObject, public QDate, protected QScriptable
{
    Q_OBJECT
private:
inline QDate *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QDate*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QDate.prototype.%0: this object is not a QDate")
            .arg(method));
}
public:
Prototype_QDate(QObject *parent = 0)
    : QDate() { setParent(parent); }
public Q_SLOTS:
bool equals(QDate const& other) const
{ if (QDate *_q_q = this->self()) return _q_q->operator==(other);
  throwTypeError(QLatin1String("equals")); return bool(); }
bool lessThan(QDate const& other) const
{ if (QDate *_q_q = this->self()) return _q_q->operator<(other);
  throwTypeError(QLatin1String("lessThan")); return bool(); }
int year() const
{ if (QDate *_q_q = this->self()) return _q_q->year();
  throwTypeError(QLatin1String("year")); return int(); }
int day() const
{ if (QDate *_q_q = this->self()) return _q_q->day();
  throwTypeError(QLatin1String("day")); return int(); }
QDate addDays(int days) const
{ if (QDate *_q_q = this->self()) return _q_q->addDays(days);
  throwTypeError(QLatin1String("addDays")); return QDate(); }
int dayOfYear() const
{ if (QDate *_q_q = this->self()) return _q_q->dayOfYear();
  throwTypeError(QLatin1String("dayOfYear")); return int(); }
QDate addYears(int years) const
{ if (QDate *_q_q = this->self()) return _q_q->addYears(years);
  throwTypeError(QLatin1String("addYears")); return QDate(); }
QDate addMonths(int months) const
{ if (QDate *_q_q = this->self()) return _q_q->addMonths(months);
  throwTypeError(QLatin1String("addMonths")); return QDate(); }
int weekNumber(int* yearNum = 0) const
{ if (QDate *_q_q = this->self()) return _q_q->weekNumber(yearNum);
  throwTypeError(QLatin1String("weekNumber")); return int(); }
int daysInYear() const
{ if (QDate *_q_q = this->self()) return _q_q->daysInYear();
  throwTypeError(QLatin1String("daysInYear")); return int(); }
QString toString(QString const& format) const
{ if (QDate *_q_q = this->self()) return _q_q->toString(format);
  throwTypeError(QLatin1String("toString")); return QString(); }
QString toString(Qt::DateFormat f = Qt::TextDate) const
{ if (QDate *_q_q = this->self()) return _q_q->toString(f);
  throwTypeError(QLatin1String("toString")); return QString(); }
int toJulianDay() const
{ if (QDate *_q_q = this->self()) return _q_q->toJulianDay();
  throwTypeError(QLatin1String("toJulianDay")); return int(); }
int daysInMonth() const
{ if (QDate *_q_q = this->self()) return _q_q->daysInMonth();
  throwTypeError(QLatin1String("daysInMonth")); return int(); }
int daysTo(QDate const& arg1) const
{ if (QDate *_q_q = this->self()) return _q_q->daysTo(arg1);
  throwTypeError(QLatin1String("daysTo")); return int(); }
bool setDate(int year, int month, int date)
{ if (QDate *_q_q = this->self()) return _q_q->setDate(year, month, date);
  throwTypeError(QLatin1String("setDate")); return bool(); }
int month() const
{ if (QDate *_q_q = this->self()) return _q_q->month();
  throwTypeError(QLatin1String("month")); return int(); }
bool setYMD(int y, int m, int d)
{ if (QDate *_q_q = this->self()) return _q_q->setYMD(y, m, d);
  throwTypeError(QLatin1String("setYMD")); return bool(); }
int dayOfWeek() const
{ if (QDate *_q_q = this->self()) return _q_q->dayOfWeek();
  throwTypeError(QLatin1String("dayOfWeek")); return int(); }
bool isNull() const
{ if (QDate *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
bool isValid() const
{ if (QDate *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
};

static QScriptValue create_QDate_class(QScriptEngine *engine)
{
    Prototype_QDate *pq = new Prototype_QDate;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QDate *cq = new Constructor_QDate;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QDate>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QDate>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QDate*>(), pv);
    return cv;
}

// QDataStream

Q_DECLARE_METATYPE(QDataStream*)
Q_DECLARE_METATYPE(QDataStream::ByteOrder)
Q_DECLARE_METATYPE(QDataStream::Version)
Q_DECLARE_METATYPE(QDataStream::Status)

class Constructor_QDataStream:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QByteArray const& arg1)
{ return this->engine()->toScriptValue(new QDataStream(arg1)); }
QScriptValue qscript_call(QByteArray* arg1, QIODevice::OpenMode flags)
{ return this->engine()->toScriptValue(new QDataStream(arg1, flags)); }
QScriptValue qscript_call(QIODevice* arg1)
{ return this->engine()->toScriptValue(new QDataStream(arg1)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(new QDataStream()); }
};

class Prototype_QDataStream:
public QObject, public QDataStream, protected QScriptable
{
    Q_OBJECT
private:
inline QDataStream *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QDataStream*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QDataStream.prototype.%0: this object is not a QDataStream")
            .arg(method));
}
public:
Prototype_QDataStream(QObject *parent = 0)
    : QDataStream() { setParent(parent); }
public Q_SLOTS:
QIODevice* device() const
{ if (QDataStream *_q_q = this->self()) return _q_q->device();
  throwTypeError(QLatin1String("device")); return 0; }
int skipRawData(int len)
{ if (QDataStream *_q_q = this->self()) return _q_q->skipRawData(len);
  throwTypeError(QLatin1String("skipRawData")); return int(); }
QDataStream* writeBytes(char const* arg1, uint len)
{ if (QDataStream *_q_q = this->self()) return &_q_q->writeBytes(arg1, len);
  throwTypeError(QLatin1String("writeBytes")); return 0; }
void resetStatus()
{ if (QDataStream *_q_q = this->self()) _q_q->resetStatus();
  else throwTypeError(QLatin1String("resetStatus")); }
int readRawData(char* arg1, int len)
{ if (QDataStream *_q_q = this->self()) return _q_q->readRawData(arg1, len);
  throwTypeError(QLatin1String("readRawData")); return int(); }
int writeRawData(char const* arg1, int len)
{ if (QDataStream *_q_q = this->self()) return _q_q->writeRawData(arg1, len);
  throwTypeError(QLatin1String("writeRawData")); return int(); }
void unsetDevice()
{ if (QDataStream *_q_q = this->self()) _q_q->unsetDevice();
  else throwTypeError(QLatin1String("unsetDevice")); }
bool atEnd() const
{ if (QDataStream *_q_q = this->self()) return _q_q->atEnd();
  throwTypeError(QLatin1String("atEnd")); return bool(); }
QDataStream* readBytes(char*& arg1, uint& len)
{ if (QDataStream *_q_q = this->self()) return &_q_q->readBytes(arg1, len);
  throwTypeError(QLatin1String("readBytes")); return 0; }
void setVersion(int arg1)
{ if (QDataStream *_q_q = this->self()) _q_q->setVersion(arg1);
  else throwTypeError(QLatin1String("setVersion")); }
int version() const
{ if (QDataStream *_q_q = this->self()) return _q_q->version();
  throwTypeError(QLatin1String("version")); return int(); }
void setStatus(QDataStream::Status status)
{ if (QDataStream *_q_q = this->self()) _q_q->setStatus(status);
  else throwTypeError(QLatin1String("setStatus")); }
QDataStream::Status status() const
{ if (QDataStream *_q_q = this->self()) return _q_q->status();
  throwTypeError(QLatin1String("status")); return QDataStream::Status(); }
void setDevice(QIODevice* arg1)
{ if (QDataStream *_q_q = this->self()) _q_q->setDevice(arg1);
  else throwTypeError(QLatin1String("setDevice")); }
void setByteOrder(QDataStream::ByteOrder arg1)
{ if (QDataStream *_q_q = this->self()) _q_q->setByteOrder(arg1);
  else throwTypeError(QLatin1String("setByteOrder")); }
QDataStream::ByteOrder byteOrder() const
{ if (QDataStream *_q_q = this->self()) return _q_q->byteOrder();
  throwTypeError(QLatin1String("byteOrder")); return QDataStream::ByteOrder(); }
QString toString() const
{ return QLatin1String("QDataStream"); }
private:
Q_DISABLE_COPY(Prototype_QDataStream)
};

static QScriptValue create_QDataStream_class(QScriptEngine *engine)
{
    Prototype_QDataStream *pq = new Prototype_QDataStream;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QDataStream *cq = new Constructor_QDataStream;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QDataStream*>(), pv);
    _q_ScriptRegisterEnumMetaType<QDataStream::ByteOrder>(engine);
    cv.setProperty("BigEndian", QScriptValue(engine, static_cast<int>(QDataStream::BigEndian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LittleEndian", QScriptValue(engine, static_cast<int>(QDataStream::LittleEndian)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QDataStream::Version>(engine);
    cv.setProperty("Qt_1_0", QScriptValue(engine, static_cast<int>(QDataStream::Qt_1_0)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Qt_2_0", QScriptValue(engine, static_cast<int>(QDataStream::Qt_2_0)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Qt_2_1", QScriptValue(engine, static_cast<int>(QDataStream::Qt_2_1)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Qt_3_0", QScriptValue(engine, static_cast<int>(QDataStream::Qt_3_0)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Qt_3_1", QScriptValue(engine, static_cast<int>(QDataStream::Qt_3_1)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Qt_3_3", QScriptValue(engine, static_cast<int>(QDataStream::Qt_3_3)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Qt_4_0", QScriptValue(engine, static_cast<int>(QDataStream::Qt_4_0)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Qt_4_1", QScriptValue(engine, static_cast<int>(QDataStream::Qt_4_1)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Qt_4_2", QScriptValue(engine, static_cast<int>(QDataStream::Qt_4_2)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Qt_4_3", QScriptValue(engine, static_cast<int>(QDataStream::Qt_4_3)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QDataStream::Status>(engine);
    cv.setProperty("Ok", QScriptValue(engine, static_cast<int>(QDataStream::Ok)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadPastEnd", QScriptValue(engine, static_cast<int>(QDataStream::ReadPastEnd)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadCorruptData", QScriptValue(engine, static_cast<int>(QDataStream::ReadCorruptData)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QDirIterator

Q_DECLARE_METATYPE(QDirIterator*)
Q_DECLARE_METATYPE(QDirIterator::IteratorFlag)
Q_DECLARE_METATYPE(QDirIterator::IteratorFlags)

class Constructor_QDirIterator:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QString const& path, QStringList const& nameFilters, QDir::Filters filters = QDir::NoFilter, QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{ return this->engine()->toScriptValue(new QDirIterator(path, nameFilters, filters, flags)); }
QScriptValue qscript_call(QString const& path, QDir::Filters filter, QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{ return this->engine()->toScriptValue(new QDirIterator(path, filter, flags)); }
QScriptValue qscript_call(QString const& path, QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{ return this->engine()->toScriptValue(new QDirIterator(path, flags)); }
QScriptValue qscript_call(QDir const& dir, QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{ return this->engine()->toScriptValue(new QDirIterator(dir, flags)); }
};

class Prototype_QDirIterator:
public QObject, public QDirIterator, protected QScriptable
{
    Q_OBJECT
private:
inline QDirIterator *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QDirIterator*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QDirIterator.prototype.%0: this object is not a QDirIterator")
            .arg(method));
}
public:
Prototype_QDirIterator(QObject *parent = 0)
    : QDirIterator(QDir()) { setParent(parent); }
public Q_SLOTS:
QString next()
{ if (QDirIterator *_q_q = this->self()) return _q_q->next();
  throwTypeError(QLatin1String("next")); return QString(); }
bool hasNext() const
{ if (QDirIterator *_q_q = this->self()) return _q_q->hasNext();
  throwTypeError(QLatin1String("hasNext")); return bool(); }
QString path() const
{ if (QDirIterator *_q_q = this->self()) return _q_q->path();
  throwTypeError(QLatin1String("path")); return QString(); }
QFileInfo fileInfo() const
{ if (QDirIterator *_q_q = this->self()) return _q_q->fileInfo();
  throwTypeError(QLatin1String("fileInfo")); return QFileInfo(); }
QString fileName() const
{ if (QDirIterator *_q_q = this->self()) return _q_q->fileName();
  throwTypeError(QLatin1String("fileName")); return QString(); }
QString filePath() const
{ if (QDirIterator *_q_q = this->self()) return _q_q->filePath();
  throwTypeError(QLatin1String("filePath")); return QString(); }
QString toString() const
{ return QLatin1String("QDirIterator"); }
private:
Q_DISABLE_COPY(Prototype_QDirIterator)
};

static QScriptValue create_QDirIterator_class(QScriptEngine *engine)
{
    Prototype_QDirIterator *pq = new Prototype_QDirIterator;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QDirIterator *cq = new Constructor_QDirIterator;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QDirIterator*>(), pv);
    _q_ScriptRegisterEnumMetaType<QDirIterator::IteratorFlag>(engine);
    cv.setProperty("NoIteratorFlags", QScriptValue(engine, static_cast<int>(QDirIterator::NoIteratorFlags)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FollowSymlinks", QScriptValue(engine, static_cast<int>(QDirIterator::FollowSymlinks)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Subdirectories", QScriptValue(engine, static_cast<int>(QDirIterator::Subdirectories)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QDirIterator::IteratorFlags>(engine);
    return cv;
}

// QChar

Q_DECLARE_METATYPE(QChar)
Q_DECLARE_METATYPE(QChar*)
Q_DECLARE_METATYPE(QChar::Joining)
Q_DECLARE_METATYPE(QChar::SpecialCharacter)
Q_DECLARE_METATYPE(QChar::UnicodeVersion)
Q_DECLARE_METATYPE(QChar::Direction)
Q_DECLARE_METATYPE(QChar::Category)
Q_DECLARE_METATYPE(QChar::CombiningClass)
Q_DECLARE_METATYPE(QChar::Decomposition)

class Constructor_QChar:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QChar::SpecialCharacter sc)
{ return this->engine()->toScriptValue(QChar(sc)); }
QScriptValue qscript_call(int rc)
{ return this->engine()->toScriptValue(QChar(rc)); }
QScriptValue qscript_call(uint rc)
{ return this->engine()->toScriptValue(QChar(rc)); }
QScriptValue qscript_call(short rc)
{ return this->engine()->toScriptValue(QChar(rc)); }
QScriptValue qscript_call(ushort rc)
{ return this->engine()->toScriptValue(QChar(rc)); }
QScriptValue qscript_call(uchar c, uchar r)
{ return this->engine()->toScriptValue(QChar(c, r)); }
QScriptValue qscript_call(QLatin1Char ch)
{ return this->engine()->toScriptValue(QChar(ch)); }
QScriptValue qscript_call(uchar c)
{ return this->engine()->toScriptValue(QChar(c)); }
QScriptValue qscript_call(char c)
{ return this->engine()->toScriptValue(QChar(c)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QChar()); }
QChar::Direction direction(ushort ucs2)
{ return QChar::direction(ucs2); }
QChar::Direction direction(uint ucs4)
{ return QChar::direction(ucs4); }
QChar fromLatin1(char c)
{ return QChar::fromLatin1(c); }
QChar::UnicodeVersion unicodeVersion(ushort ucs2)
{ return QChar::unicodeVersion(ucs2); }
QChar::UnicodeVersion unicodeVersion(uint ucs4)
{ return QChar::unicodeVersion(ucs4); }
QString decomposition(uint ucs4)
{ return QChar::decomposition(ucs4); }
QChar fromAscii(char c)
{ return QChar::fromAscii(c); }
QChar::Joining joining(ushort ucs2)
{ return QChar::joining(ucs2); }
QChar::Joining joining(uint ucs4)
{ return QChar::joining(ucs4); }
uint surrogateToUcs4(QChar high, QChar low)
{ return QChar::surrogateToUcs4(high, low); }
uint surrogateToUcs4(ushort high, ushort low)
{ return QChar::surrogateToUcs4(high, low); }
ushort lowSurrogate(uint ucs4)
{ return QChar::lowSurrogate(ucs4); }
ushort mirroredChar(ushort ucs2)
{ return QChar::mirroredChar(ucs2); }
uint mirroredChar(uint ucs4)
{ return QChar::mirroredChar(ucs4); }
unsigned char combiningClass(ushort ucs2)
{ return QChar::combiningClass(ucs2); }
unsigned char combiningClass(uint ucs4)
{ return QChar::combiningClass(ucs4); }
QChar::Category category(ushort ucs2)
{ return QChar::category(ucs2); }
QChar::Category category(uint ucs4)
{ return QChar::category(ucs4); }
int digitValue(ushort ucs2)
{ return QChar::digitValue(ucs2); }
int digitValue(uint ucs4)
{ return QChar::digitValue(ucs4); }
ushort toLower(ushort ucs2)
{ return QChar::toLower(ucs2); }
uint toLower(uint ucs4)
{ return QChar::toLower(ucs4); }
ushort toUpper(ushort ucs2)
{ return QChar::toUpper(ucs2); }
uint toUpper(uint ucs4)
{ return QChar::toUpper(ucs4); }
ushort toTitleCase(ushort ucs2)
{ return QChar::toTitleCase(ucs2); }
uint toTitleCase(uint ucs4)
{ return QChar::toTitleCase(ucs4); }
QChar::Decomposition decompositionTag(uint ucs4)
{ return QChar::decompositionTag(ucs4); }
ushort highSurrogate(uint ucs4)
{ return QChar::highSurrogate(ucs4); }
ushort toCaseFolded(ushort ucs2)
{ return QChar::toCaseFolded(ucs2); }
uint toCaseFolded(uint ucs4)
{ return QChar::toCaseFolded(ucs4); }
};

class Prototype_QChar:
public QObject, public QChar, protected QScriptable
{
    Q_OBJECT
private:
inline QChar *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QChar*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QChar.prototype.%0: this object is not a QChar")
            .arg(method));
}
public:
Prototype_QChar(QObject *parent = 0)
    : QChar() { setParent(parent); }
public Q_SLOTS:
QChar::Direction direction() const
{ if (QChar *_q_q = this->self()) return _q_q->direction();
  throwTypeError(QLatin1String("direction")); return QChar::Direction(); }
QChar fromLatin1(char c)
{ if (QChar *_q_q = this->self()) return _q_q->fromLatin1(c);
  throwTypeError(QLatin1String("fromLatin1")); return QChar(); }
bool isNumber() const
{ if (QChar *_q_q = this->self()) return _q_q->isNumber();
  throwTypeError(QLatin1String("isNumber")); return bool(); }
bool isSpace() const
{ if (QChar *_q_q = this->self()) return _q_q->isSpace();
  throwTypeError(QLatin1String("isSpace")); return bool(); }
void setRow(uchar row)
{ if (QChar *_q_q = this->self()) _q_q->setRow(row);
  else throwTypeError(QLatin1String("setRow")); }
bool isLetter() const
{ if (QChar *_q_q = this->self()) return _q_q->isLetter();
  throwTypeError(QLatin1String("isLetter")); return bool(); }
bool isMark() const
{ if (QChar *_q_q = this->self()) return _q_q->isMark();
  throwTypeError(QLatin1String("isMark")); return bool(); }
uchar row() const
{ if (QChar *_q_q = this->self()) return _q_q->row();
  throwTypeError(QLatin1String("row")); return uchar(); }
bool isLower() const
{ if (QChar *_q_q = this->self()) return _q_q->isLower();
  throwTypeError(QLatin1String("isLower")); return bool(); }
QChar::UnicodeVersion unicodeVersion() const
{ if (QChar *_q_q = this->self()) return _q_q->unicodeVersion();
  throwTypeError(QLatin1String("unicodeVersion")); return QChar::UnicodeVersion(); }
QString decomposition() const
{ if (QChar *_q_q = this->self()) return _q_q->decomposition();
  throwTypeError(QLatin1String("decomposition")); return QString(); }
bool isUpper() const
{ if (QChar *_q_q = this->self()) return _q_q->isUpper();
  throwTypeError(QLatin1String("isUpper")); return bool(); }
QChar::Joining joining() const
{ if (QChar *_q_q = this->self()) return _q_q->joining();
  throwTypeError(QLatin1String("joining")); return QChar::Joining(); }
bool isSymbol() const
{ if (QChar *_q_q = this->self()) return _q_q->isSymbol();
  throwTypeError(QLatin1String("isSymbol")); return bool(); }
QChar mirroredChar() const
{ if (QChar *_q_q = this->self()) return _q_q->mirroredChar();
  throwTypeError(QLatin1String("mirroredChar")); return QChar(); }
unsigned char combiningClass() const
{ if (QChar *_q_q = this->self()) return _q_q->combiningClass();
  throwTypeError(QLatin1String("combiningClass")); return 0; }
char const toAscii() const
{ if (QChar *_q_q = this->self()) return _q_q->toAscii();
  throwTypeError(QLatin1String("toAscii")); return char(); }
QChar::Category category() const
{ if (QChar *_q_q = this->self()) return _q_q->category();
  throwTypeError(QLatin1String("category")); return QChar::Category(); }
bool isTitleCase() const
{ if (QChar *_q_q = this->self()) return _q_q->isTitleCase();
  throwTypeError(QLatin1String("isTitleCase")); return bool(); }
int digitValue() const
{ if (QChar *_q_q = this->self()) return _q_q->digitValue();
  throwTypeError(QLatin1String("digitValue")); return int(); }
bool isHighSurrogate() const
{ if (QChar *_q_q = this->self()) return _q_q->isHighSurrogate();
  throwTypeError(QLatin1String("isHighSurrogate")); return bool(); }
void setCell(uchar cell)
{ if (QChar *_q_q = this->self()) _q_q->setCell(cell);
  else throwTypeError(QLatin1String("setCell")); }
QChar toLower() const
{ if (QChar *_q_q = this->self()) return _q_q->toLower();
  throwTypeError(QLatin1String("toLower")); return QChar(); }
bool hasMirrored() const
{ if (QChar *_q_q = this->self()) return _q_q->hasMirrored();
  throwTypeError(QLatin1String("hasMirrored")); return bool(); }
QChar toUpper() const
{ if (QChar *_q_q = this->self()) return _q_q->toUpper();
  throwTypeError(QLatin1String("toUpper")); return QChar(); }
bool isDigit() const
{ if (QChar *_q_q = this->self()) return _q_q->isDigit();
  throwTypeError(QLatin1String("isDigit")); return bool(); }
bool isPunct() const
{ if (QChar *_q_q = this->self()) return _q_q->isPunct();
  throwTypeError(QLatin1String("isPunct")); return bool(); }
QChar toTitleCase() const
{ if (QChar *_q_q = this->self()) return _q_q->toTitleCase();
  throwTypeError(QLatin1String("toTitleCase")); return QChar(); }
bool isLowSurrogate() const
{ if (QChar *_q_q = this->self()) return _q_q->isLowSurrogate();
  throwTypeError(QLatin1String("isLowSurrogate")); return bool(); }
QChar::Decomposition decompositionTag() const
{ if (QChar *_q_q = this->self()) return _q_q->decompositionTag();
  throwTypeError(QLatin1String("decompositionTag")); return QChar::Decomposition(); }
QChar toCaseFolded() const
{ if (QChar *_q_q = this->self()) return _q_q->toCaseFolded();
  throwTypeError(QLatin1String("toCaseFolded")); return QChar(); }
bool isNull() const
{ if (QChar *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
char const toLatin1() const
{ if (QChar *_q_q = this->self()) return _q_q->toLatin1();
  throwTypeError(QLatin1String("toLatin1")); return char(); }
uchar cell() const
{ if (QChar *_q_q = this->self()) return _q_q->cell();
  throwTypeError(QLatin1String("cell")); return uchar(); }
bool isLetterOrNumber() const
{ if (QChar *_q_q = this->self()) return _q_q->isLetterOrNumber();
  throwTypeError(QLatin1String("isLetterOrNumber")); return bool(); }
bool isPrint() const
{ if (QChar *_q_q = this->self()) return _q_q->isPrint();
  throwTypeError(QLatin1String("isPrint")); return bool(); }
ushort const unicode() const
{ if (QChar *_q_q = this->self()) return _q_q->unicode();
  throwTypeError(QLatin1String("unicode")); return ushort(); }
QString toString() const
{ return QLatin1String("QChar"); }
};

static QScriptValue create_QChar_class(QScriptEngine *engine)
{
    Prototype_QChar *pq = new Prototype_QChar;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QChar *cq = new Constructor_QChar;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QChar>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QChar>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QChar*>(), pv);
    _q_ScriptRegisterEnumMetaType<QChar::Joining>(engine);
    cv.setProperty("OtherJoining", QScriptValue(engine, static_cast<int>(QChar::OtherJoining)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Dual", QScriptValue(engine, static_cast<int>(QChar::Dual)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Right", QScriptValue(engine, static_cast<int>(QChar::Right)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Center", QScriptValue(engine, static_cast<int>(QChar::Center)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QChar::SpecialCharacter>(engine);
    cv.setProperty("Null", QScriptValue(engine, static_cast<int>(QChar::Null)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Nbsp", QScriptValue(engine, static_cast<int>(QChar::Nbsp)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReplacementCharacter", QScriptValue(engine, static_cast<int>(QChar::ReplacementCharacter)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ObjectReplacementCharacter", QScriptValue(engine, static_cast<int>(QChar::ObjectReplacementCharacter)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ByteOrderMark", QScriptValue(engine, static_cast<int>(QChar::ByteOrderMark)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ByteOrderSwapped", QScriptValue(engine, static_cast<int>(QChar::ByteOrderSwapped)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ParagraphSeparator", QScriptValue(engine, static_cast<int>(QChar::ParagraphSeparator)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LineSeparator", QScriptValue(engine, static_cast<int>(QChar::LineSeparator)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QChar::UnicodeVersion>(engine);
    cv.setProperty("Unicode_Unassigned", QScriptValue(engine, static_cast<int>(QChar::Unicode_Unassigned)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Unicode_1_1", QScriptValue(engine, static_cast<int>(QChar::Unicode_1_1)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Unicode_2_0", QScriptValue(engine, static_cast<int>(QChar::Unicode_2_0)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Unicode_2_1_2", QScriptValue(engine, static_cast<int>(QChar::Unicode_2_1_2)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Unicode_3_0", QScriptValue(engine, static_cast<int>(QChar::Unicode_3_0)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Unicode_3_1", QScriptValue(engine, static_cast<int>(QChar::Unicode_3_1)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Unicode_3_2", QScriptValue(engine, static_cast<int>(QChar::Unicode_3_2)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Unicode_4_0", QScriptValue(engine, static_cast<int>(QChar::Unicode_4_0)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Unicode_4_1", QScriptValue(engine, static_cast<int>(QChar::Unicode_4_1)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Unicode_5_0", QScriptValue(engine, static_cast<int>(QChar::Unicode_5_0)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QChar::Direction>(engine);
    cv.setProperty("DirL", QScriptValue(engine, static_cast<int>(QChar::DirL)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirR", QScriptValue(engine, static_cast<int>(QChar::DirR)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirEN", QScriptValue(engine, static_cast<int>(QChar::DirEN)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirES", QScriptValue(engine, static_cast<int>(QChar::DirES)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirET", QScriptValue(engine, static_cast<int>(QChar::DirET)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirAN", QScriptValue(engine, static_cast<int>(QChar::DirAN)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirCS", QScriptValue(engine, static_cast<int>(QChar::DirCS)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirB", QScriptValue(engine, static_cast<int>(QChar::DirB)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirS", QScriptValue(engine, static_cast<int>(QChar::DirS)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirWS", QScriptValue(engine, static_cast<int>(QChar::DirWS)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirON", QScriptValue(engine, static_cast<int>(QChar::DirON)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirLRE", QScriptValue(engine, static_cast<int>(QChar::DirLRE)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirLRO", QScriptValue(engine, static_cast<int>(QChar::DirLRO)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirAL", QScriptValue(engine, static_cast<int>(QChar::DirAL)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirRLE", QScriptValue(engine, static_cast<int>(QChar::DirRLE)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirRLO", QScriptValue(engine, static_cast<int>(QChar::DirRLO)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirPDF", QScriptValue(engine, static_cast<int>(QChar::DirPDF)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirNSM", QScriptValue(engine, static_cast<int>(QChar::DirNSM)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DirBN", QScriptValue(engine, static_cast<int>(QChar::DirBN)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QChar::Category>(engine);
    cv.setProperty("NoCategory", QScriptValue(engine, static_cast<int>(QChar::NoCategory)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Mark_NonSpacing", QScriptValue(engine, static_cast<int>(QChar::Mark_NonSpacing)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Mark_SpacingCombining", QScriptValue(engine, static_cast<int>(QChar::Mark_SpacingCombining)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Mark_Enclosing", QScriptValue(engine, static_cast<int>(QChar::Mark_Enclosing)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Number_DecimalDigit", QScriptValue(engine, static_cast<int>(QChar::Number_DecimalDigit)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Number_Letter", QScriptValue(engine, static_cast<int>(QChar::Number_Letter)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Number_Other", QScriptValue(engine, static_cast<int>(QChar::Number_Other)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Separator_Space", QScriptValue(engine, static_cast<int>(QChar::Separator_Space)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Separator_Line", QScriptValue(engine, static_cast<int>(QChar::Separator_Line)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Separator_Paragraph", QScriptValue(engine, static_cast<int>(QChar::Separator_Paragraph)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Other_Control", QScriptValue(engine, static_cast<int>(QChar::Other_Control)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Other_Format", QScriptValue(engine, static_cast<int>(QChar::Other_Format)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Other_Surrogate", QScriptValue(engine, static_cast<int>(QChar::Other_Surrogate)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Other_PrivateUse", QScriptValue(engine, static_cast<int>(QChar::Other_PrivateUse)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Other_NotAssigned", QScriptValue(engine, static_cast<int>(QChar::Other_NotAssigned)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Letter_Uppercase", QScriptValue(engine, static_cast<int>(QChar::Letter_Uppercase)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Letter_Lowercase", QScriptValue(engine, static_cast<int>(QChar::Letter_Lowercase)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Letter_Titlecase", QScriptValue(engine, static_cast<int>(QChar::Letter_Titlecase)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Letter_Modifier", QScriptValue(engine, static_cast<int>(QChar::Letter_Modifier)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Letter_Other", QScriptValue(engine, static_cast<int>(QChar::Letter_Other)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Punctuation_Connector", QScriptValue(engine, static_cast<int>(QChar::Punctuation_Connector)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Punctuation_Dash", QScriptValue(engine, static_cast<int>(QChar::Punctuation_Dash)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Punctuation_Open", QScriptValue(engine, static_cast<int>(QChar::Punctuation_Open)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Punctuation_Close", QScriptValue(engine, static_cast<int>(QChar::Punctuation_Close)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Punctuation_InitialQuote", QScriptValue(engine, static_cast<int>(QChar::Punctuation_InitialQuote)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Punctuation_FinalQuote", QScriptValue(engine, static_cast<int>(QChar::Punctuation_FinalQuote)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Punctuation_Other", QScriptValue(engine, static_cast<int>(QChar::Punctuation_Other)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Symbol_Math", QScriptValue(engine, static_cast<int>(QChar::Symbol_Math)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Symbol_Currency", QScriptValue(engine, static_cast<int>(QChar::Symbol_Currency)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Symbol_Modifier", QScriptValue(engine, static_cast<int>(QChar::Symbol_Modifier)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Symbol_Other", QScriptValue(engine, static_cast<int>(QChar::Symbol_Other)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Punctuation_Dask", QScriptValue(engine, static_cast<int>(QChar::Punctuation_Dask)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QChar::CombiningClass>(engine);
    cv.setProperty("Combining_BelowLeftAttached", QScriptValue(engine, static_cast<int>(QChar::Combining_BelowLeftAttached)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_BelowAttached", QScriptValue(engine, static_cast<int>(QChar::Combining_BelowAttached)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_BelowRightAttached", QScriptValue(engine, static_cast<int>(QChar::Combining_BelowRightAttached)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_LeftAttached", QScriptValue(engine, static_cast<int>(QChar::Combining_LeftAttached)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_RightAttached", QScriptValue(engine, static_cast<int>(QChar::Combining_RightAttached)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_AboveLeftAttached", QScriptValue(engine, static_cast<int>(QChar::Combining_AboveLeftAttached)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_AboveAttached", QScriptValue(engine, static_cast<int>(QChar::Combining_AboveAttached)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_AboveRightAttached", QScriptValue(engine, static_cast<int>(QChar::Combining_AboveRightAttached)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_BelowLeft", QScriptValue(engine, static_cast<int>(QChar::Combining_BelowLeft)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_Below", QScriptValue(engine, static_cast<int>(QChar::Combining_Below)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_BelowRight", QScriptValue(engine, static_cast<int>(QChar::Combining_BelowRight)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_Left", QScriptValue(engine, static_cast<int>(QChar::Combining_Left)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_Right", QScriptValue(engine, static_cast<int>(QChar::Combining_Right)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_AboveLeft", QScriptValue(engine, static_cast<int>(QChar::Combining_AboveLeft)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_Above", QScriptValue(engine, static_cast<int>(QChar::Combining_Above)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_AboveRight", QScriptValue(engine, static_cast<int>(QChar::Combining_AboveRight)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_DoubleBelow", QScriptValue(engine, static_cast<int>(QChar::Combining_DoubleBelow)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_DoubleAbove", QScriptValue(engine, static_cast<int>(QChar::Combining_DoubleAbove)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Combining_IotaSubscript", QScriptValue(engine, static_cast<int>(QChar::Combining_IotaSubscript)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QChar::Decomposition>(engine);
    cv.setProperty("NoDecomposition", QScriptValue(engine, static_cast<int>(QChar::NoDecomposition)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Canonical", QScriptValue(engine, static_cast<int>(QChar::Canonical)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Font", QScriptValue(engine, static_cast<int>(QChar::Font)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NoBreak", QScriptValue(engine, static_cast<int>(QChar::NoBreak)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Initial", QScriptValue(engine, static_cast<int>(QChar::Initial)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Medial", QScriptValue(engine, static_cast<int>(QChar::Medial)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Final", QScriptValue(engine, static_cast<int>(QChar::Final)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Isolated", QScriptValue(engine, static_cast<int>(QChar::Isolated)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Circle", QScriptValue(engine, static_cast<int>(QChar::Circle)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Super", QScriptValue(engine, static_cast<int>(QChar::Super)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Sub", QScriptValue(engine, static_cast<int>(QChar::Sub)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Vertical", QScriptValue(engine, static_cast<int>(QChar::Vertical)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Wide", QScriptValue(engine, static_cast<int>(QChar::Wide)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Narrow", QScriptValue(engine, static_cast<int>(QChar::Narrow)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Small", QScriptValue(engine, static_cast<int>(QChar::Small)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Square", QScriptValue(engine, static_cast<int>(QChar::Square)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Compat", QScriptValue(engine, static_cast<int>(QChar::Compat)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Fraction", QScriptValue(engine, static_cast<int>(QChar::Fraction)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QByteArrayMatcher

Q_DECLARE_METATYPE(QByteArrayMatcher)
Q_DECLARE_METATYPE(QByteArrayMatcher*)

class Constructor_QByteArrayMatcher:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QByteArrayMatcher const& other)
{ return this->engine()->toScriptValue(QByteArrayMatcher(other)); }
QScriptValue qscript_call(QByteArray const& pattern)
{ return this->engine()->toScriptValue(QByteArrayMatcher(pattern)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QByteArrayMatcher()); }
};

class Prototype_QByteArrayMatcher:
public QObject, public QByteArrayMatcher, protected QScriptable
{
    Q_OBJECT
private:
inline QByteArrayMatcher *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QByteArrayMatcher*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QByteArrayMatcher.prototype.%0: this object is not a QByteArrayMatcher")
            .arg(method));
}
public:
Prototype_QByteArrayMatcher(QObject *parent = 0)
    : QByteArrayMatcher() { setParent(parent); }
public Q_SLOTS:
void setPattern(QByteArray const& pattern)
{ if (QByteArrayMatcher *_q_q = this->self()) _q_q->setPattern(pattern);
  else throwTypeError(QLatin1String("setPattern")); }
int indexIn(QByteArray const& ba, int from = 0) const
{ if (QByteArrayMatcher *_q_q = this->self()) return _q_q->indexIn(ba, from);
  throwTypeError(QLatin1String("indexIn")); return int(); }
QByteArray pattern() const
{ if (QByteArrayMatcher *_q_q = this->self()) return _q_q->pattern();
  throwTypeError(QLatin1String("pattern")); return QByteArray(); }
QString toString() const
{ return QLatin1String("QByteArrayMatcher"); }
};

static QScriptValue create_QByteArrayMatcher_class(QScriptEngine *engine)
{
    Prototype_QByteArrayMatcher *pq = new Prototype_QByteArrayMatcher;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QByteArrayMatcher *cq = new Constructor_QByteArrayMatcher;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QByteArrayMatcher>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QByteArrayMatcher>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QByteArrayMatcher*>(), pv);
    return cv;
}

// QSize

Q_DECLARE_METATYPE(QSize)
Q_DECLARE_METATYPE(QSize*)

class Constructor_QSize:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(int w, int h)
{ return this->engine()->toScriptValue(QSize(w, h)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(QSize()); }
};

class Prototype_QSize:
public QObject, public QSize, protected QScriptable
{
    Q_OBJECT
private:
inline QSize *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QSize*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QSize.prototype.%0: this object is not a QSize")
            .arg(method));
}
public:
Prototype_QSize(QObject *parent = 0)
    : QSize() { setParent(parent); }
public Q_SLOTS:
bool isValid() const
{ if (QSize *_q_q = this->self()) return _q_q->isValid();
  throwTypeError(QLatin1String("isValid")); return bool(); }
void scale(QSize const& s, Qt::AspectRatioMode mode)
{ if (QSize *_q_q = this->self()) _q_q->scale(s, mode);
  else throwTypeError(QLatin1String("scale")); }
void scale(int w, int h, Qt::AspectRatioMode mode)
{ if (QSize *_q_q = this->self()) _q_q->scale(w, h, mode);
  else throwTypeError(QLatin1String("scale")); }
int* rwidth()
{ if (QSize *_q_q = this->self()) return &_q_q->rwidth();
  throwTypeError(QLatin1String("rwidth")); return 0; }
QSize expandedTo(QSize const& arg1) const
{ if (QSize *_q_q = this->self()) return _q_q->expandedTo(arg1);
  throwTypeError(QLatin1String("expandedTo")); return QSize(); }
void setWidth(int w)
{ if (QSize *_q_q = this->self()) _q_q->setWidth(w);
  else throwTypeError(QLatin1String("setWidth")); }
void setHeight(int h)
{ if (QSize *_q_q = this->self()) _q_q->setHeight(h);
  else throwTypeError(QLatin1String("setHeight")); }
int* rheight()
{ if (QSize *_q_q = this->self()) return &_q_q->rheight();
  throwTypeError(QLatin1String("rheight")); return 0; }
bool isNull() const
{ if (QSize *_q_q = this->self()) return _q_q->isNull();
  throwTypeError(QLatin1String("isNull")); return bool(); }
QSize boundedTo(QSize const& arg1) const
{ if (QSize *_q_q = this->self()) return _q_q->boundedTo(arg1);
  throwTypeError(QLatin1String("boundedTo")); return QSize(); }
int width() const
{ if (QSize *_q_q = this->self()) return _q_q->width();
  throwTypeError(QLatin1String("width")); return int(); }
void transpose()
{ if (QSize *_q_q = this->self()) _q_q->transpose();
  else throwTypeError(QLatin1String("transpose")); }
bool isEmpty() const
{ if (QSize *_q_q = this->self()) return _q_q->isEmpty();
  throwTypeError(QLatin1String("isEmpty")); return bool(); }
int height() const
{ if (QSize *_q_q = this->self()) return _q_q->height();
  throwTypeError(QLatin1String("height")); return int(); }
QString toString() const
{ return QLatin1String("QSize"); }
};

static QScriptValue create_QSize_class(QScriptEngine *engine)
{
    Prototype_QSize *pq = new Prototype_QSize;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    pv.setPrototype(engine->globalObject().property("Object").property("prototype"));
    Constructor_QSize *cq = new Constructor_QSize;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterValueMetaType<QSize>(engine, pv);
    engine->setDefaultPrototype(qMetaTypeId<QSize>(), pv);
    engine->setDefaultPrototype(qMetaTypeId<QSize*>(), pv);
    return cv;
}

// QTimer

Q_DECLARE_METATYPE(QTimer*)

class Constructor_QTimer:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* parent = 0)
{ return this->engine()->newQObject(new QTimer(parent), QScriptEngine::AutoOwnership); }
void singleShot(int msec, QObject* receiver, char const* member)
{ QTimer::singleShot(msec, receiver, member); }
};

class Prototype_QTimer:
public QTimer, protected QScriptable
{
    Q_OBJECT
private:
inline QTimer *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QTimer*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QTimer.prototype.%0: this object is not a QTimer")
            .arg(method));
}
public:
Prototype_QTimer(QObject *parent = 0)
    : QTimer() { setParent(parent); }
public Q_SLOTS:
void start()
{ if (QTimer *_q_q = this->self()) _q_q->start();
  else throwTypeError(QLatin1String("start")); }
void start(int msec)
{ if (QTimer *_q_q = this->self()) _q_q->start(msec);
  else throwTypeError(QLatin1String("start")); }
int timerId() const
{ if (QTimer *_q_q = this->self()) return _q_q->timerId();
  throwTypeError(QLatin1String("timerId")); return int(); }
void stop()
{ if (QTimer *_q_q = this->self()) _q_q->stop();
  else throwTypeError(QLatin1String("stop")); }
void setSingleShot(bool singleShot)
{ if (QTimer *_q_q = this->self()) _q_q->setSingleShot(singleShot);
  else throwTypeError(QLatin1String("setSingleShot")); }
bool isActive() const
{ if (QTimer *_q_q = this->self()) return _q_q->isActive();
  throwTypeError(QLatin1String("isActive")); return bool(); }
bool isSingleShot() const
{ if (QTimer *_q_q = this->self()) return _q_q->isSingleShot();
  throwTypeError(QLatin1String("isSingleShot")); return bool(); }
void setInterval(int msec)
{ if (QTimer *_q_q = this->self()) _q_q->setInterval(msec);
  else throwTypeError(QLatin1String("setInterval")); }
int interval() const
{ if (QTimer *_q_q = this->self()) return _q_q->interval();
  throwTypeError(QLatin1String("interval")); return int(); }
QString toString() const
{ return QLatin1String("QTimer"); }
private:
Q_DISABLE_COPY(Prototype_QTimer)
};

static QScriptValue create_QTimer_class(QScriptEngine *engine)
{
    Prototype_QTimer *pq = new Prototype_QTimer;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QTimer *cq = new Constructor_QTimer;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QTimer>(engine, pv);
    return cv;
}

// QObjectCleanupHandler

Q_DECLARE_METATYPE(QObjectCleanupHandler*)

class Constructor_QObjectCleanupHandler:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call()
{ return this->engine()->newQObject(new QObjectCleanupHandler(), QScriptEngine::AutoOwnership); }
};

class Prototype_QObjectCleanupHandler:
public QObjectCleanupHandler, protected QScriptable
{
    Q_OBJECT
private:
inline QObjectCleanupHandler *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QObjectCleanupHandler*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QObjectCleanupHandler.prototype.%0: this object is not a QObjectCleanupHandler")
            .arg(method));
}
public:
Prototype_QObjectCleanupHandler(QObject *parent = 0)
    : QObjectCleanupHandler() { setParent(parent); }
public Q_SLOTS:
void remove(QObject* object)
{ if (QObjectCleanupHandler *_q_q = this->self()) _q_q->remove(object);
  else throwTypeError(QLatin1String("remove")); }
void clear()
{ if (QObjectCleanupHandler *_q_q = this->self()) _q_q->clear();
  else throwTypeError(QLatin1String("clear")); }
QObject* add(QObject* object)
{ if (QObjectCleanupHandler *_q_q = this->self()) return _q_q->add(object);
  throwTypeError(QLatin1String("add")); return 0; }
bool isEmpty() const
{ if (QObjectCleanupHandler *_q_q = this->self()) return _q_q->isEmpty();
  throwTypeError(QLatin1String("isEmpty")); return bool(); }
QString toString() const
{ return QLatin1String("QObjectCleanupHandler"); }
private:
Q_DISABLE_COPY(Prototype_QObjectCleanupHandler)
};

static QScriptValue create_QObjectCleanupHandler_class(QScriptEngine *engine)
{
    Prototype_QObjectCleanupHandler *pq = new Prototype_QObjectCleanupHandler;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QObjectCleanupHandler *cq = new Constructor_QObjectCleanupHandler;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QObjectCleanupHandler>(engine, pv);
    return cv;
}

// QSettings

Q_DECLARE_METATYPE(QSettings*)
Q_DECLARE_METATYPE(QSettings::Format)
Q_DECLARE_METATYPE(QSettings::Status)
Q_DECLARE_METATYPE(QSettings::Scope)

class Constructor_QSettings:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* parent = 0)
{ return this->engine()->newQObject(new QSettings(parent), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call(QString const& fileName, QSettings::Format format, QObject* parent = 0)
{ return this->engine()->newQObject(new QSettings(fileName, format, parent), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call(QSettings::Format format, QSettings::Scope scope, QString const& organization, QString const& application = QString(), QObject* parent = 0)
{ return this->engine()->newQObject(new QSettings(format, scope, organization, application, parent), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call(QSettings::Scope scope, QString const& organization, QString const& application = QString(), QObject* parent = 0)
{ return this->engine()->newQObject(new QSettings(scope, organization, application, parent), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call(QString const& organization, QString const& application = QString(), QObject* parent = 0)
{ return this->engine()->newQObject(new QSettings(organization, application, parent), QScriptEngine::AutoOwnership); }
void setUserIniPath(QString const& dir)
{ QSettings::setUserIniPath(dir); }
QSettings::Format registerFormat(QString const& extension, QSettings::ReadFunc readFunc, QSettings::WriteFunc writeFunc, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive)
{ return QSettings::registerFormat(extension, readFunc, writeFunc, caseSensitivity); }
void setSystemIniPath(QString const& dir)
{ QSettings::setSystemIniPath(dir); }
void setPath(QSettings::Format format, QSettings::Scope scope, QString const& path)
{ QSettings::setPath(format, scope, path); }
};

class Prototype_QSettings:
public QSettings, protected QScriptable
{
    Q_OBJECT
private:
inline QSettings *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QSettings*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QSettings.prototype.%0: this object is not a QSettings")
            .arg(method));
}
public:
Prototype_QSettings(QObject *parent = 0)
    : QSettings() { setParent(parent); }
public Q_SLOTS:
QStringList allKeys() const
{ if (QSettings *_q_q = this->self()) return _q_q->allKeys();
  throwTypeError(QLatin1String("allKeys")); return QStringList(); }
QString fileName() const
{ if (QSettings *_q_q = this->self()) return _q_q->fileName();
  throwTypeError(QLatin1String("fileName")); return QString(); }
void beginGroup(QString const& prefix)
{ if (QSettings *_q_q = this->self()) _q_q->beginGroup(prefix);
  else throwTypeError(QLatin1String("beginGroup")); }
void beginWriteArray(QString const& prefix, int size = -1)
{ if (QSettings *_q_q = this->self()) _q_q->beginWriteArray(prefix, size);
  else throwTypeError(QLatin1String("beginWriteArray")); }
bool fallbacksEnabled() const
{ if (QSettings *_q_q = this->self()) return _q_q->fallbacksEnabled();
  throwTypeError(QLatin1String("fallbacksEnabled")); return bool(); }
void setFallbacksEnabled(bool b)
{ if (QSettings *_q_q = this->self()) _q_q->setFallbacksEnabled(b);
  else throwTypeError(QLatin1String("setFallbacksEnabled")); }
bool isWritable() const
{ if (QSettings *_q_q = this->self()) return _q_q->isWritable();
  throwTypeError(QLatin1String("isWritable")); return bool(); }
QSettings::Status status() const
{ if (QSettings *_q_q = this->self()) return _q_q->status();
  throwTypeError(QLatin1String("status")); return QSettings::Status(); }
int beginReadArray(QString const& prefix)
{ if (QSettings *_q_q = this->self()) return _q_q->beginReadArray(prefix);
  throwTypeError(QLatin1String("beginReadArray")); return int(); }
void sync()
{ if (QSettings *_q_q = this->self()) _q_q->sync();
  else throwTypeError(QLatin1String("sync")); }
QStringList childGroups() const
{ if (QSettings *_q_q = this->self()) return _q_q->childGroups();
  throwTypeError(QLatin1String("childGroups")); return QStringList(); }
void endArray()
{ if (QSettings *_q_q = this->self()) _q_q->endArray();
  else throwTypeError(QLatin1String("endArray")); }
QStringList childKeys() const
{ if (QSettings *_q_q = this->self()) return _q_q->childKeys();
  throwTypeError(QLatin1String("childKeys")); return QStringList(); }
QVariant value(QString const& key, QVariant const& defaultValue = QVariant()) const
{ if (QSettings *_q_q = this->self()) return _q_q->value(key, defaultValue);
  throwTypeError(QLatin1String("value")); return QVariant(); }
bool contains(QString const& key) const
{ if (QSettings *_q_q = this->self()) return _q_q->contains(key);
  throwTypeError(QLatin1String("contains")); return bool(); }
void remove(QString const& key)
{ if (QSettings *_q_q = this->self()) _q_q->remove(key);
  else throwTypeError(QLatin1String("remove")); }
void setArrayIndex(int i)
{ if (QSettings *_q_q = this->self()) _q_q->setArrayIndex(i);
  else throwTypeError(QLatin1String("setArrayIndex")); }
void endGroup()
{ if (QSettings *_q_q = this->self()) _q_q->endGroup();
  else throwTypeError(QLatin1String("endGroup")); }
QString group() const
{ if (QSettings *_q_q = this->self()) return _q_q->group();
  throwTypeError(QLatin1String("group")); return QString(); }
void clear()
{ if (QSettings *_q_q = this->self()) _q_q->clear();
  else throwTypeError(QLatin1String("clear")); }
void setValue(QString const& key, QVariant const& value)
{ if (QSettings *_q_q = this->self()) _q_q->setValue(key, value);
  else throwTypeError(QLatin1String("setValue")); }
QString toString() const
{ return QLatin1String("QSettings"); }
private:
Q_DISABLE_COPY(Prototype_QSettings)
};

static QScriptValue create_QSettings_class(QScriptEngine *engine)
{
    Prototype_QSettings *pq = new Prototype_QSettings;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QSettings *cq = new Constructor_QSettings;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QSettings>(engine, pv);
    _q_ScriptRegisterEnumMetaType<QSettings::Format>(engine);
    cv.setProperty("NativeFormat", QScriptValue(engine, static_cast<int>(QSettings::NativeFormat)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("IniFormat", QScriptValue(engine, static_cast<int>(QSettings::IniFormat)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("InvalidFormat", QScriptValue(engine, static_cast<int>(QSettings::InvalidFormat)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat1", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat1)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat2", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat2)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat3", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat3)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat4", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat4)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat5", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat5)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat6", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat6)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat7", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat7)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat8", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat8)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat9", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat9)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat10", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat10)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat11", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat11)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat12", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat12)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat13", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat13)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat14", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat14)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat15", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat15)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CustomFormat16", QScriptValue(engine, static_cast<int>(QSettings::CustomFormat16)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QSettings::Status>(engine);
    cv.setProperty("NoError", QScriptValue(engine, static_cast<int>(QSettings::NoError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AccessError", QScriptValue(engine, static_cast<int>(QSettings::AccessError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FormatError", QScriptValue(engine, static_cast<int>(QSettings::FormatError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QSettings::Scope>(engine);
    cv.setProperty("UserScope", QScriptValue(engine, static_cast<int>(QSettings::UserScope)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SystemScope", QScriptValue(engine, static_cast<int>(QSettings::SystemScope)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QTranslator

Q_DECLARE_METATYPE(QTranslator*)

class Constructor_QTranslator:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* parent = 0)
{ return this->engine()->newQObject(new QTranslator(parent), QScriptEngine::AutoOwnership); }
};

class Prototype_QTranslator:
public QTranslator, protected QScriptable
{
    Q_OBJECT
private:
inline QTranslator *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QTranslator*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QTranslator.prototype.%0: this object is not a QTranslator")
            .arg(method));
}
public:
Prototype_QTranslator(QObject *parent = 0)
    : QTranslator() { setParent(parent); }
public Q_SLOTS:
bool load(uchar const* data, int len)
{ if (QTranslator *_q_q = this->self()) return _q_q->load(data, len);
  throwTypeError(QLatin1String("load")); return bool(); }
bool load(QString const& filename, QString const& directory = QString(), QString const& search_delimiters = QString(), QString const& suffix = QString())
{ if (QTranslator *_q_q = this->self()) return _q_q->load(filename, directory, search_delimiters, suffix);
  throwTypeError(QLatin1String("load")); return bool(); }
QString translate(char const* context, char const* sourceText, char const* comment, int n) const
{ if (QTranslator *_q_q = this->self()) return _q_q->translate(context, sourceText, comment, n);
  throwTypeError(QLatin1String("translate")); return QString(); }
QString translate(char const* context, char const* sourceText, char const* comment = 0) const
{ if (QTranslator *_q_q = this->self()) return _q_q->translate(context, sourceText, comment);
  throwTypeError(QLatin1String("translate")); return QString(); }
bool isEmpty() const
{ if (QTranslator *_q_q = this->self()) return _q_q->isEmpty();
  throwTypeError(QLatin1String("isEmpty")); return bool(); }
QString toString() const
{ return QLatin1String("QTranslator"); }
private:
Q_DISABLE_COPY(Prototype_QTranslator)
};

static QScriptValue create_QTranslator_class(QScriptEngine *engine)
{
    Prototype_QTranslator *pq = new Prototype_QTranslator;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QTranslator *cq = new Constructor_QTranslator;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QTranslator>(engine, pv);
    return cv;
}

// QAbstractEventDispatcher

Q_DECLARE_METATYPE(QAbstractEventDispatcher*)

class Constructor_QAbstractEventDispatcher:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* /* parent */ = 0)
{ return this->context()->throwError(QString::fromLatin1("QAbstractEventDispatcher cannot be instantiated")); }
QAbstractEventDispatcher* instance(QThread* thread = 0)
{ return QAbstractEventDispatcher::instance(thread); }
};

class Prototype_QAbstractEventDispatcher:
public QObject, protected QScriptable
{
    Q_OBJECT
private:
inline QAbstractEventDispatcher *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QAbstractEventDispatcher*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QAbstractEventDispatcher.prototype.%0: this object is not a QAbstractEventDispatcher")
            .arg(method));
}
public:
Prototype_QAbstractEventDispatcher(QObject *parent = 0)
 { setParent(parent); }
public Q_SLOTS:
void registerTimer(int timerId, int interval, QObject* object)
{ if (QAbstractEventDispatcher *_q_q = this->self()) _q_q->registerTimer(timerId, interval, object);
  else throwTypeError(QLatin1String("registerTimer")); }
int registerTimer(int interval, QObject* object)
{ if (QAbstractEventDispatcher *_q_q = this->self()) return _q_q->registerTimer(interval, object);
  throwTypeError(QLatin1String("registerTimer")); return int(); }
bool unregisterTimer(int timerId)
{ if (QAbstractEventDispatcher *_q_q = this->self()) return _q_q->unregisterTimer(timerId);
  throwTypeError(QLatin1String("unregisterTimer")); return bool(); }
void closingDown()
{ if (QAbstractEventDispatcher *_q_q = this->self()) _q_q->closingDown();
  else throwTypeError(QLatin1String("closingDown")); }
void startingUp()
{ if (QAbstractEventDispatcher *_q_q = this->self()) _q_q->startingUp();
  else throwTypeError(QLatin1String("startingUp")); }
void interrupt()
{ if (QAbstractEventDispatcher *_q_q = this->self()) _q_q->interrupt();
  else throwTypeError(QLatin1String("interrupt")); }
bool filterEvent(void* message)
{ if (QAbstractEventDispatcher *_q_q = this->self()) return _q_q->filterEvent(message);
  throwTypeError(QLatin1String("filterEvent")); return bool(); }
bool processEvents(QEventLoop::ProcessEventsFlags flags)
{ if (QAbstractEventDispatcher *_q_q = this->self()) return _q_q->processEvents(flags);
  throwTypeError(QLatin1String("processEvents")); return bool(); }
void wakeUp()
{ if (QAbstractEventDispatcher *_q_q = this->self()) _q_q->wakeUp();
  else throwTypeError(QLatin1String("wakeUp")); }
QAbstractEventDispatcher::EventFilter setEventFilter(QAbstractEventDispatcher::EventFilter filter)
{ if (QAbstractEventDispatcher *_q_q = this->self()) return _q_q->setEventFilter(filter);
  throwTypeError(QLatin1String("setEventFilter")); return QAbstractEventDispatcher::EventFilter(); }
void unregisterSocketNotifier(QSocketNotifier* notifier)
{ if (QAbstractEventDispatcher *_q_q = this->self()) _q_q->unregisterSocketNotifier(notifier);
  else throwTypeError(QLatin1String("unregisterSocketNotifier")); }
void flush()
{ if (QAbstractEventDispatcher *_q_q = this->self()) _q_q->flush();
  else throwTypeError(QLatin1String("flush")); }
void registerSocketNotifier(QSocketNotifier* notifier)
{ if (QAbstractEventDispatcher *_q_q = this->self()) _q_q->registerSocketNotifier(notifier);
  else throwTypeError(QLatin1String("registerSocketNotifier")); }
bool unregisterTimers(QObject* object)
{ if (QAbstractEventDispatcher *_q_q = this->self()) return _q_q->unregisterTimers(object);
  throwTypeError(QLatin1String("unregisterTimers")); return bool(); }
QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject* object) const
{ if (QAbstractEventDispatcher *_q_q = this->self()) return _q_q->registeredTimers(object);
  throwTypeError(QLatin1String("registeredTimers")); return QList<QAbstractEventDispatcher::TimerInfo>(); }
bool hasPendingEvents()
{ if (QAbstractEventDispatcher *_q_q = this->self()) return _q_q->hasPendingEvents();
  throwTypeError(QLatin1String("hasPendingEvents")); return bool(); }
QString toString() const
{ return QLatin1String("QAbstractEventDispatcher"); }
public:
private:
Q_DISABLE_COPY(Prototype_QAbstractEventDispatcher)
};

static QScriptValue create_QAbstractEventDispatcher_class(QScriptEngine *engine)
{
    Prototype_QAbstractEventDispatcher *pq = new Prototype_QAbstractEventDispatcher;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QAbstractEventDispatcher *cq = new Constructor_QAbstractEventDispatcher;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QAbstractEventDispatcher>(engine, pv);
    return cv;
}

// QTimeLine

Q_DECLARE_METATYPE(QTimeLine*)
Q_DECLARE_METATYPE(QTimeLine::Direction)
Q_DECLARE_METATYPE(QTimeLine::CurveShape)
Q_DECLARE_METATYPE(QTimeLine::State)

class Constructor_QTimeLine:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(int duration = 1000, QObject* parent = 0)
{ return this->engine()->newQObject(new QTimeLine(duration, parent), QScriptEngine::AutoOwnership); }
};

class Prototype_QTimeLine:
public QTimeLine, protected QScriptable
{
    Q_OBJECT
private:
inline QTimeLine *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QTimeLine*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QTimeLine.prototype.%0: this object is not a QTimeLine")
            .arg(method));
}
public:
Prototype_QTimeLine(QObject *parent = 0)
    : QTimeLine() { setParent(parent); }
public Q_SLOTS:
void setCurrentTime(int msec)
{ if (QTimeLine *_q_q = this->self()) _q_q->setCurrentTime(msec);
  else throwTypeError(QLatin1String("setCurrentTime")); }
void toggleDirection()
{ if (QTimeLine *_q_q = this->self()) _q_q->toggleDirection();
  else throwTypeError(QLatin1String("toggleDirection")); }
int loopCount() const
{ if (QTimeLine *_q_q = this->self()) return _q_q->loopCount();
  throwTypeError(QLatin1String("loopCount")); return int(); }
void setDirection(QTimeLine::Direction direction)
{ if (QTimeLine *_q_q = this->self()) _q_q->setDirection(direction);
  else throwTypeError(QLatin1String("setDirection")); }
void setDuration(int duration)
{ if (QTimeLine *_q_q = this->self()) _q_q->setDuration(duration);
  else throwTypeError(QLatin1String("setDuration")); }
int duration() const
{ if (QTimeLine *_q_q = this->self()) return _q_q->duration();
  throwTypeError(QLatin1String("duration")); return int(); }
void setLoopCount(int count)
{ if (QTimeLine *_q_q = this->self()) _q_q->setLoopCount(count);
  else throwTypeError(QLatin1String("setLoopCount")); }
QTimeLine::CurveShape curveShape() const
{ if (QTimeLine *_q_q = this->self()) return _q_q->curveShape();
  throwTypeError(QLatin1String("curveShape")); return QTimeLine::CurveShape(); }
void start()
{ if (QTimeLine *_q_q = this->self()) _q_q->start();
  else throwTypeError(QLatin1String("start")); }
int endFrame() const
{ if (QTimeLine *_q_q = this->self()) return _q_q->endFrame();
  throwTypeError(QLatin1String("endFrame")); return int(); }
void setFrameRange(int startFrame, int endFrame)
{ if (QTimeLine *_q_q = this->self()) _q_q->setFrameRange(startFrame, endFrame);
  else throwTypeError(QLatin1String("setFrameRange")); }
void resume()
{ if (QTimeLine *_q_q = this->self()) _q_q->resume();
  else throwTypeError(QLatin1String("resume")); }
qreal valueForTime(int msec) const
{ if (QTimeLine *_q_q = this->self()) return _q_q->valueForTime(msec);
  throwTypeError(QLatin1String("valueForTime")); return qreal(); }
void setCurveShape(QTimeLine::CurveShape shape)
{ if (QTimeLine *_q_q = this->self()) _q_q->setCurveShape(shape);
  else throwTypeError(QLatin1String("setCurveShape")); }
qreal currentValue() const
{ if (QTimeLine *_q_q = this->self()) return _q_q->currentValue();
  throwTypeError(QLatin1String("currentValue")); return qreal(); }
QTimeLine::State state() const
{ if (QTimeLine *_q_q = this->self()) return _q_q->state();
  throwTypeError(QLatin1String("state")); return QTimeLine::State(); }
int frameForTime(int msec) const
{ if (QTimeLine *_q_q = this->self()) return _q_q->frameForTime(msec);
  throwTypeError(QLatin1String("frameForTime")); return int(); }
void setStartFrame(int frame)
{ if (QTimeLine *_q_q = this->self()) _q_q->setStartFrame(frame);
  else throwTypeError(QLatin1String("setStartFrame")); }
void setPaused(bool paused)
{ if (QTimeLine *_q_q = this->self()) _q_q->setPaused(paused);
  else throwTypeError(QLatin1String("setPaused")); }
int updateInterval() const
{ if (QTimeLine *_q_q = this->self()) return _q_q->updateInterval();
  throwTypeError(QLatin1String("updateInterval")); return int(); }
void stop()
{ if (QTimeLine *_q_q = this->self()) _q_q->stop();
  else throwTypeError(QLatin1String("stop")); }
void setEndFrame(int frame)
{ if (QTimeLine *_q_q = this->self()) _q_q->setEndFrame(frame);
  else throwTypeError(QLatin1String("setEndFrame")); }
void setUpdateInterval(int interval)
{ if (QTimeLine *_q_q = this->self()) _q_q->setUpdateInterval(interval);
  else throwTypeError(QLatin1String("setUpdateInterval")); }
int currentFrame() const
{ if (QTimeLine *_q_q = this->self()) return _q_q->currentFrame();
  throwTypeError(QLatin1String("currentFrame")); return int(); }
int currentTime() const
{ if (QTimeLine *_q_q = this->self()) return _q_q->currentTime();
  throwTypeError(QLatin1String("currentTime")); return int(); }
int startFrame() const
{ if (QTimeLine *_q_q = this->self()) return _q_q->startFrame();
  throwTypeError(QLatin1String("startFrame")); return int(); }
QTimeLine::Direction direction() const
{ if (QTimeLine *_q_q = this->self()) return _q_q->direction();
  throwTypeError(QLatin1String("direction")); return QTimeLine::Direction(); }
QString toString() const
{ return QLatin1String("QTimeLine"); }
private:
Q_DISABLE_COPY(Prototype_QTimeLine)
};

static QScriptValue create_QTimeLine_class(QScriptEngine *engine)
{
    Prototype_QTimeLine *pq = new Prototype_QTimeLine;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QTimeLine *cq = new Constructor_QTimeLine;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QTimeLine>(engine, pv);
    _q_ScriptRegisterEnumMetaType<QTimeLine::Direction>(engine);
    cv.setProperty("Forward", QScriptValue(engine, static_cast<int>(QTimeLine::Forward)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Backward", QScriptValue(engine, static_cast<int>(QTimeLine::Backward)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QTimeLine::CurveShape>(engine);
    cv.setProperty("EaseInCurve", QScriptValue(engine, static_cast<int>(QTimeLine::EaseInCurve)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("EaseOutCurve", QScriptValue(engine, static_cast<int>(QTimeLine::EaseOutCurve)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("EaseInOutCurve", QScriptValue(engine, static_cast<int>(QTimeLine::EaseInOutCurve)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LinearCurve", QScriptValue(engine, static_cast<int>(QTimeLine::LinearCurve)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("SineCurve", QScriptValue(engine, static_cast<int>(QTimeLine::SineCurve)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QTimeLine::State>(engine);
    cv.setProperty("NotRunning", QScriptValue(engine, static_cast<int>(QTimeLine::NotRunning)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Paused", QScriptValue(engine, static_cast<int>(QTimeLine::Paused)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Running", QScriptValue(engine, static_cast<int>(QTimeLine::Running)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QSocketNotifier

Q_DECLARE_METATYPE(QSocketNotifier*)
Q_DECLARE_METATYPE(QSocketNotifier::Type)

class Constructor_QSocketNotifier:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(int socket, QSocketNotifier::Type arg2, QObject* parent = 0)
{ return this->engine()->newQObject(new QSocketNotifier(socket, arg2, parent), QScriptEngine::AutoOwnership); }
};

class Prototype_QSocketNotifier:
public QObject, protected QScriptable
{
    Q_OBJECT
private:
inline QSocketNotifier *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QSocketNotifier*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QSocketNotifier.prototype.%0: this object is not a QSocketNotifier")
            .arg(method));
}
public:
Prototype_QSocketNotifier(QObject *parent = 0)
 { setParent(parent); }
public Q_SLOTS:
int socket() const
{ if (QSocketNotifier *_q_q = this->self()) return _q_q->socket();
  throwTypeError(QLatin1String("socket")); return int(); }
bool isEnabled() const
{ if (QSocketNotifier *_q_q = this->self()) return _q_q->isEnabled();
  throwTypeError(QLatin1String("isEnabled")); return bool(); }
void setEnabled(bool arg1)
{ if (QSocketNotifier *_q_q = this->self()) _q_q->setEnabled(arg1);
  else throwTypeError(QLatin1String("setEnabled")); }
QSocketNotifier::Type type() const
{ if (QSocketNotifier *_q_q = this->self()) return _q_q->type();
  throwTypeError(QLatin1String("type")); return QSocketNotifier::Type(); }
QString toString() const
{ return QLatin1String("QSocketNotifier"); }
private:
Q_DISABLE_COPY(Prototype_QSocketNotifier)
};

static QScriptValue create_QSocketNotifier_class(QScriptEngine *engine)
{
    Prototype_QSocketNotifier *pq = new Prototype_QSocketNotifier;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QSocketNotifier *cq = new Constructor_QSocketNotifier;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QSocketNotifier>(engine, pv);
    _q_ScriptRegisterEnumMetaType<QSocketNotifier::Type>(engine);
    cv.setProperty("Read", QScriptValue(engine, static_cast<int>(QSocketNotifier::Read)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Write", QScriptValue(engine, static_cast<int>(QSocketNotifier::Write)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Exception", QScriptValue(engine, static_cast<int>(QSocketNotifier::Exception)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QTextCodecPlugin

Q_DECLARE_METATYPE(QTextCodecPlugin*)

class Constructor_QTextCodecPlugin:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* /* parent */ = 0)
{ return this->context()->throwError(QString::fromLatin1("QTextCodecPlugin cannot be instantiated")); }
};

class Prototype_QTextCodecPlugin:
public QTextCodecPlugin, protected QScriptable
{
    Q_OBJECT
private:
inline QTextCodecPlugin *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QTextCodecPlugin*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QTextCodecPlugin.prototype.%0: this object is not a QTextCodecPlugin")
            .arg(method));
}
public:
Prototype_QTextCodecPlugin(QObject *parent = 0)
    : QTextCodecPlugin() { setParent(parent); }
public Q_SLOTS:
QList<QByteArray> names() const
{ if (QTextCodecPlugin *_q_q = this->self()) return _q_q->names();
  throwTypeError(QLatin1String("names")); return QList<QByteArray>(); }
QList<int> mibEnums() const
{ if (QTextCodecPlugin *_q_q = this->self()) return _q_q->mibEnums();
  throwTypeError(QLatin1String("mibEnums")); return QList<int>(); }
QList<QByteArray> aliases() const
{ if (QTextCodecPlugin *_q_q = this->self()) return _q_q->aliases();
  throwTypeError(QLatin1String("aliases")); return QList<QByteArray>(); }
QTextCodec* createForName(QByteArray const& name)
{ if (QTextCodecPlugin *_q_q = this->self()) return _q_q->createForName(name);
  throwTypeError(QLatin1String("createForName")); return 0; }
QTextCodec* createForMib(int mib)
{ if (QTextCodecPlugin *_q_q = this->self()) return _q_q->createForMib(mib);
  throwTypeError(QLatin1String("createForMib")); return 0; }
QString toString() const
{ return QLatin1String("QTextCodecPlugin"); }
public:
private:
Q_DISABLE_COPY(Prototype_QTextCodecPlugin)
};

static QScriptValue create_QTextCodecPlugin_class(QScriptEngine *engine)
{
    Prototype_QTextCodecPlugin *pq = new Prototype_QTextCodecPlugin;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QTextCodecPlugin *cq = new Constructor_QTextCodecPlugin;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QTextCodecPlugin>(engine, pv);
    return cv;
}

// QLibrary

Q_DECLARE_METATYPE(QLibrary*)
Q_DECLARE_METATYPE(QLibrary::LoadHint)
Q_DECLARE_METATYPE(QLibrary::LoadHints)

class Constructor_QLibrary:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QString const& fileName, int verNum, QObject* parent = 0)
{ return this->engine()->newQObject(new QLibrary(fileName, verNum, parent), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call(QString const& fileName, QObject* parent = 0)
{ return this->engine()->newQObject(new QLibrary(fileName, parent), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call(QObject* parent = 0)
{ return this->engine()->newQObject(new QLibrary(parent), QScriptEngine::AutoOwnership); }
void* resolve(QString const& fileName, int verNum, char const* symbol)
{ return QLibrary::resolve(fileName, verNum, symbol); }
void* resolve(QString const& fileName, char const* symbol)
{ return QLibrary::resolve(fileName, symbol); }
bool isLibrary(QString const& fileName)
{ return QLibrary::isLibrary(fileName); }
};

class Prototype_QLibrary:
public QLibrary, protected QScriptable
{
    Q_OBJECT
private:
inline QLibrary *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QLibrary*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QLibrary.prototype.%0: this object is not a QLibrary")
            .arg(method));
}
public:
Prototype_QLibrary(QObject *parent = 0)
    : QLibrary() { setParent(parent); }
public Q_SLOTS:
QString fileName() const
{ if (QLibrary *_q_q = this->self()) return _q_q->fileName();
  throwTypeError(QLatin1String("fileName")); return QString(); }
QLibrary::LoadHints loadHints() const
{ if (QLibrary *_q_q = this->self()) return _q_q->loadHints();
  throwTypeError(QLatin1String("loadHints")); return QLibrary::LoadHints(); }
void setFileName(QString const& fileName)
{ if (QLibrary *_q_q = this->self()) _q_q->setFileName(fileName);
  else throwTypeError(QLatin1String("setFileName")); }
void setFileNameAndVersion(QString const& fileName, int verNum)
{ if (QLibrary *_q_q = this->self()) _q_q->setFileNameAndVersion(fileName, verNum);
  else throwTypeError(QLatin1String("setFileNameAndVersion")); }
void* resolve(char const* symbol)
{ if (QLibrary *_q_q = this->self()) return _q_q->resolve(symbol);
  throwTypeError(QLatin1String("resolve")); return 0; }
bool isLoaded() const
{ if (QLibrary *_q_q = this->self()) return _q_q->isLoaded();
  throwTypeError(QLatin1String("isLoaded")); return bool(); }
bool load()
{ if (QLibrary *_q_q = this->self()) return _q_q->load();
  throwTypeError(QLatin1String("load")); return bool(); }
bool unload()
{ if (QLibrary *_q_q = this->self()) return _q_q->unload();
  throwTypeError(QLatin1String("unload")); return bool(); }
void setLoadHints(QLibrary::LoadHints hints)
{ if (QLibrary *_q_q = this->self()) _q_q->setLoadHints(hints);
  else throwTypeError(QLatin1String("setLoadHints")); }
QString errorString() const
{ if (QLibrary *_q_q = this->self()) return _q_q->errorString();
  throwTypeError(QLatin1String("errorString")); return QString(); }
QString toString() const
{ return QLatin1String("QLibrary"); }
private:
Q_DISABLE_COPY(Prototype_QLibrary)
};

static QScriptValue create_QLibrary_class(QScriptEngine *engine)
{
    Prototype_QLibrary *pq = new Prototype_QLibrary;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QLibrary *cq = new Constructor_QLibrary;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QLibrary>(engine, pv);
    _q_ScriptRegisterEnumMetaType<QLibrary::LoadHint>(engine);
    cv.setProperty("ResolveAllSymbolsHint", QScriptValue(engine, static_cast<int>(QLibrary::ResolveAllSymbolsHint)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ExportExternalSymbolsHint", QScriptValue(engine, static_cast<int>(QLibrary::ExportExternalSymbolsHint)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LoadArchiveMemberHint", QScriptValue(engine, static_cast<int>(QLibrary::LoadArchiveMemberHint)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QLibrary::LoadHints>(engine);
    return cv;
}

// QThread

Q_DECLARE_METATYPE(QThread*)
Q_DECLARE_METATYPE(QThread::Priority)

class Constructor_QThread:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* /* parent */ = 0)
{ return this->context()->throwError(QString::fromLatin1("QThread cannot be instantiated")); }
QThread* currentThread()
{ return QThread::currentThread(); }
int idealThreadCount()
{ return QThread::idealThreadCount(); }
Qt::HANDLE currentThreadId()
{ return QThread::currentThreadId(); }
};

class Prototype_QThread:
public QObject, protected QScriptable
{
    Q_OBJECT
private:
inline QThread *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QThread*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QThread.prototype.%0: this object is not a QThread")
            .arg(method));
}
public:
Prototype_QThread(QObject *parent = 0)
 { setParent(parent); }
public Q_SLOTS:
void start(QThread::Priority arg1 = QThread::InheritPriority)
{ if (QThread *_q_q = this->self()) _q_q->start(arg1);
  else throwTypeError(QLatin1String("start")); }
bool wait(unsigned long time = ULONG_MAX)
{ if (QThread *_q_q = this->self()) return _q_q->wait(time);
  throwTypeError(QLatin1String("wait")); return bool(); }
void setPriority(QThread::Priority priority)
{ if (QThread *_q_q = this->self()) _q_q->setPriority(priority);
  else throwTypeError(QLatin1String("setPriority")); }
void quit()
{ if (QThread *_q_q = this->self()) _q_q->quit();
  else throwTypeError(QLatin1String("quit")); }
QThread::Priority priority() const
{ if (QThread *_q_q = this->self()) return _q_q->priority();
  throwTypeError(QLatin1String("priority")); return QThread::Priority(); }
void setStackSize(uint stackSize)
{ if (QThread *_q_q = this->self()) _q_q->setStackSize(stackSize);
  else throwTypeError(QLatin1String("setStackSize")); }
bool isFinished() const
{ if (QThread *_q_q = this->self()) return _q_q->isFinished();
  throwTypeError(QLatin1String("isFinished")); return bool(); }
uint stackSize() const
{ if (QThread *_q_q = this->self()) return _q_q->stackSize();
  throwTypeError(QLatin1String("stackSize")); return uint(); }
void terminate()
{ if (QThread *_q_q = this->self()) _q_q->terminate();
  else throwTypeError(QLatin1String("terminate")); }
void exit(int retcode = 0)
{ if (QThread *_q_q = this->self()) _q_q->exit(retcode);
  else throwTypeError(QLatin1String("exit")); }
bool isRunning() const
{ if (QThread *_q_q = this->self()) return _q_q->isRunning();
  throwTypeError(QLatin1String("isRunning")); return bool(); }
QString toString() const
{ return QLatin1String("QThread"); }
public:
void run() {  }
private:
Q_DISABLE_COPY(Prototype_QThread)
};

static QScriptValue create_QThread_class(QScriptEngine *engine)
{
    Prototype_QThread *pq = new Prototype_QThread;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QThread *cq = new Constructor_QThread;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QThread>(engine, pv);
    _q_ScriptRegisterEnumMetaType<QThread::Priority>(engine);
    cv.setProperty("IdlePriority", QScriptValue(engine, static_cast<int>(QThread::IdlePriority)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LowestPriority", QScriptValue(engine, static_cast<int>(QThread::LowestPriority)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("LowPriority", QScriptValue(engine, static_cast<int>(QThread::LowPriority)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("NormalPriority", QScriptValue(engine, static_cast<int>(QThread::NormalPriority)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("HighPriority", QScriptValue(engine, static_cast<int>(QThread::HighPriority)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("HighestPriority", QScriptValue(engine, static_cast<int>(QThread::HighestPriority)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TimeCriticalPriority", QScriptValue(engine, static_cast<int>(QThread::TimeCriticalPriority)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("InheritPriority", QScriptValue(engine, static_cast<int>(QThread::InheritPriority)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QAbstractItemModel

Q_DECLARE_METATYPE(QAbstractItemModel*)

class Constructor_QAbstractItemModel:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* /* parent */ = 0)
{ return this->context()->throwError(QString::fromLatin1("QAbstractItemModel cannot be instantiated")); }
};

class Prototype_QAbstractItemModel:
public QAbstractItemModel, protected QScriptable
{
    Q_OBJECT
private:
inline QAbstractItemModel *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QAbstractItemModel*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QAbstractItemModel.prototype.%0: this object is not a QAbstractItemModel")
            .arg(method));
}
public:
Prototype_QAbstractItemModel(QObject *parent = 0)
    : QAbstractItemModel() { setParent(parent); }
public Q_SLOTS:
bool insertRows(int row, int count, QModelIndex const& parent = QModelIndex())
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->insertRows(row, count, parent);
  throwTypeError(QLatin1String("insertRows")); return bool(); }
QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->headerData(section, orientation, role);
  throwTypeError(QLatin1String("headerData")); return QVariant(); }
QMap<int,QVariant> itemData(QModelIndex const& index) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->itemData(index);
  throwTypeError(QLatin1String("itemData")); return QMap<int,QVariant>(); }
QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->data(index, role);
  throwTypeError(QLatin1String("data")); return QVariant(); }
QModelIndex parent(QModelIndex const& child) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->parent(child);
  throwTypeError(QLatin1String("parent")); return QModelIndex(); }
QSize span(QModelIndex const& index) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->span(index);
  throwTypeError(QLatin1String("span")); return QSize(); }
bool removeColumn(int column, QModelIndex const& parent = QModelIndex())
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->removeColumn(column, parent);
  throwTypeError(QLatin1String("removeColumn")); return bool(); }
QMimeData* mimeData(QModelIndexList const& indexes) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->mimeData(indexes);
  throwTypeError(QLatin1String("mimeData")); return 0; }
bool dropMimeData(QMimeData const* data, Qt::DropAction action, int row, int column, QModelIndex const& parent)
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->dropMimeData(data, action, row, column, parent);
  throwTypeError(QLatin1String("dropMimeData")); return bool(); }
bool insertRow(int row, QModelIndex const& parent = QModelIndex())
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->insertRow(row, parent);
  throwTypeError(QLatin1String("insertRow")); return bool(); }
int rowCount(QModelIndex const& parent = QModelIndex()) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->rowCount(parent);
  throwTypeError(QLatin1String("rowCount")); return int(); }
bool setItemData(QModelIndex const& index, QMap<int,QVariant> const& roles)
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->setItemData(index, roles);
  throwTypeError(QLatin1String("setItemData")); return bool(); }
bool removeColumns(int column, int count, QModelIndex const& parent = QModelIndex())
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->removeColumns(column, count, parent);
  throwTypeError(QLatin1String("removeColumns")); return bool(); }
QModelIndexList match(QModelIndex const& start, int role, QVariant const& value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap)) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->match(start, role, value, hits, flags);
  throwTypeError(QLatin1String("match")); return QModelIndexList(); }
void sort(int column, Qt::SortOrder order = Qt::AscendingOrder)
{ if (QAbstractItemModel *_q_q = this->self()) _q_q->sort(column, order);
  else throwTypeError(QLatin1String("sort")); }
bool setHeaderData(int section, Qt::Orientation orientation, QVariant const& value, int role = Qt::EditRole)
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->setHeaderData(section, orientation, value, role);
  throwTypeError(QLatin1String("setHeaderData")); return bool(); }
bool removeRows(int row, int count, QModelIndex const& parent = QModelIndex())
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->removeRows(row, count, parent);
  throwTypeError(QLatin1String("removeRows")); return bool(); }
Qt::DropActions supportedDropActions() const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->supportedDropActions();
  throwTypeError(QLatin1String("supportedDropActions")); return Qt::DropActions(); }
void fetchMore(QModelIndex const& parent)
{ if (QAbstractItemModel *_q_q = this->self()) _q_q->fetchMore(parent);
  else throwTypeError(QLatin1String("fetchMore")); }
bool insertColumns(int column, int count, QModelIndex const& parent = QModelIndex())
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->insertColumns(column, count, parent);
  throwTypeError(QLatin1String("insertColumns")); return bool(); }
QModelIndex sibling(int row, int column, QModelIndex const& idx) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->sibling(row, column, idx);
  throwTypeError(QLatin1String("sibling")); return QModelIndex(); }
bool submit()
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->submit();
  throwTypeError(QLatin1String("submit")); return bool(); }
void revert()
{ if (QAbstractItemModel *_q_q = this->self()) _q_q->revert();
  else throwTypeError(QLatin1String("revert")); }
QModelIndex buddy(QModelIndex const& index) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->buddy(index);
  throwTypeError(QLatin1String("buddy")); return QModelIndex(); }
bool insertColumn(int column, QModelIndex const& parent = QModelIndex())
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->insertColumn(column, parent);
  throwTypeError(QLatin1String("insertColumn")); return bool(); }
bool canFetchMore(QModelIndex const& parent) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->canFetchMore(parent);
  throwTypeError(QLatin1String("canFetchMore")); return bool(); }
bool hasIndex(int row, int column, QModelIndex const& parent = QModelIndex()) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->hasIndex(row, column, parent);
  throwTypeError(QLatin1String("hasIndex")); return bool(); }
int columnCount(QModelIndex const& parent = QModelIndex()) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->columnCount(parent);
  throwTypeError(QLatin1String("columnCount")); return int(); }
bool removeRow(int row, QModelIndex const& parent = QModelIndex())
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->removeRow(row, parent);
  throwTypeError(QLatin1String("removeRow")); return bool(); }
void setSupportedDragActions(Qt::DropActions arg1)
{ if (QAbstractItemModel *_q_q = this->self()) _q_q->setSupportedDragActions(arg1);
  else throwTypeError(QLatin1String("setSupportedDragActions")); }
bool hasChildren(QModelIndex const& parent = QModelIndex()) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->hasChildren(parent);
  throwTypeError(QLatin1String("hasChildren")); return bool(); }
Qt::DropActions supportedDragActions() const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->supportedDragActions();
  throwTypeError(QLatin1String("supportedDragActions")); return Qt::DropActions(); }
Qt::ItemFlags flags(QModelIndex const& index) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->flags(index);
  throwTypeError(QLatin1String("flags")); return Qt::ItemFlags(); }
QStringList mimeTypes() const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->mimeTypes();
  throwTypeError(QLatin1String("mimeTypes")); return QStringList(); }
QModelIndex index(int row, int column, QModelIndex const& parent = QModelIndex()) const
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->index(row, column, parent);
  throwTypeError(QLatin1String("index")); return QModelIndex(); }
bool setData(QModelIndex const& index, QVariant const& value, int role = Qt::EditRole)
{ if (QAbstractItemModel *_q_q = this->self()) return _q_q->setData(index, value, role);
  throwTypeError(QLatin1String("setData")); return bool(); }
QString toString() const
{ return QLatin1String("QAbstractItemModel"); }
public:
private:
Q_DISABLE_COPY(Prototype_QAbstractItemModel)
};

static QScriptValue create_QAbstractItemModel_class(QScriptEngine *engine)
{
    Prototype_QAbstractItemModel *pq = new Prototype_QAbstractItemModel;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QAbstractItemModel *cq = new Constructor_QAbstractItemModel;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QAbstractItemModel>(engine, pv);
    return cv;
}

// QFileSystemWatcher

Q_DECLARE_METATYPE(QFileSystemWatcher*)

class Constructor_QFileSystemWatcher:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QStringList const& paths, QObject* parent = 0)
{ return this->engine()->newQObject(new QFileSystemWatcher(paths, parent), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call(QObject* parent = 0)
{ return this->engine()->newQObject(new QFileSystemWatcher(parent), QScriptEngine::AutoOwnership); }
};

class Prototype_QFileSystemWatcher:
public QFileSystemWatcher, protected QScriptable
{
    Q_OBJECT
private:
inline QFileSystemWatcher *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QFileSystemWatcher*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QFileSystemWatcher.prototype.%0: this object is not a QFileSystemWatcher")
            .arg(method));
}
public:
Prototype_QFileSystemWatcher(QObject *parent = 0)
    : QFileSystemWatcher() { setParent(parent); }
public Q_SLOTS:
QStringList files() const
{ if (QFileSystemWatcher *_q_q = this->self()) return _q_q->files();
  throwTypeError(QLatin1String("files")); return QStringList(); }
QStringList directories() const
{ if (QFileSystemWatcher *_q_q = this->self()) return _q_q->directories();
  throwTypeError(QLatin1String("directories")); return QStringList(); }
void addPaths(QStringList const& files)
{ if (QFileSystemWatcher *_q_q = this->self()) _q_q->addPaths(files);
  else throwTypeError(QLatin1String("addPaths")); }
void addPath(QString const& file)
{ if (QFileSystemWatcher *_q_q = this->self()) _q_q->addPath(file);
  else throwTypeError(QLatin1String("addPath")); }
void removePath(QString const& file)
{ if (QFileSystemWatcher *_q_q = this->self()) _q_q->removePath(file);
  else throwTypeError(QLatin1String("removePath")); }
void removePaths(QStringList const& files)
{ if (QFileSystemWatcher *_q_q = this->self()) _q_q->removePaths(files);
  else throwTypeError(QLatin1String("removePaths")); }
QString toString() const
{ return QLatin1String("QFileSystemWatcher"); }
private:
Q_DISABLE_COPY(Prototype_QFileSystemWatcher)
};

static QScriptValue create_QFileSystemWatcher_class(QScriptEngine *engine)
{
    Prototype_QFileSystemWatcher *pq = new Prototype_QFileSystemWatcher;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QFileSystemWatcher *cq = new Constructor_QFileSystemWatcher;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QFileSystemWatcher>(engine, pv);
    return cv;
}

// QEventLoop

Q_DECLARE_METATYPE(QEventLoop*)
Q_DECLARE_METATYPE(QEventLoop::ProcessEventsFlag)
Q_DECLARE_METATYPE(QEventLoop::ProcessEventsFlags)

class Constructor_QEventLoop:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* parent = 0)
{ return this->engine()->newQObject(new QEventLoop(parent), QScriptEngine::AutoOwnership); }
};

class Prototype_QEventLoop:
public QEventLoop, protected QScriptable
{
    Q_OBJECT
private:
inline QEventLoop *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QEventLoop*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QEventLoop.prototype.%0: this object is not a QEventLoop")
            .arg(method));
}
public:
Prototype_QEventLoop(QObject *parent = 0)
    : QEventLoop() { setParent(parent); }
public Q_SLOTS:
int exec(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents)
{ if (QEventLoop *_q_q = this->self()) return _q_q->exec(flags);
  throwTypeError(QLatin1String("exec")); return int(); }
void processEvents(QEventLoop::ProcessEventsFlags flags, int maximumTime)
{ if (QEventLoop *_q_q = this->self()) _q_q->processEvents(flags, maximumTime);
  else throwTypeError(QLatin1String("processEvents")); }
bool processEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents)
{ if (QEventLoop *_q_q = this->self()) return _q_q->processEvents(flags);
  throwTypeError(QLatin1String("processEvents")); return bool(); }
bool isRunning() const
{ if (QEventLoop *_q_q = this->self()) return _q_q->isRunning();
  throwTypeError(QLatin1String("isRunning")); return bool(); }
void exit(int returnCode = 0)
{ if (QEventLoop *_q_q = this->self()) _q_q->exit(returnCode);
  else throwTypeError(QLatin1String("exit")); }
void quit()
{ if (QEventLoop *_q_q = this->self()) _q_q->quit();
  else throwTypeError(QLatin1String("quit")); }
void wakeUp()
{ if (QEventLoop *_q_q = this->self()) _q_q->wakeUp();
  else throwTypeError(QLatin1String("wakeUp")); }
QString toString() const
{ return QLatin1String("QEventLoop"); }
private:
Q_DISABLE_COPY(Prototype_QEventLoop)
};

static QScriptValue create_QEventLoop_class(QScriptEngine *engine)
{
    Prototype_QEventLoop *pq = new Prototype_QEventLoop;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QEventLoop *cq = new Constructor_QEventLoop;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QEventLoop>(engine, pv);
    _q_ScriptRegisterEnumMetaType<QEventLoop::ProcessEventsFlag>(engine);
    cv.setProperty("AllEvents", QScriptValue(engine, static_cast<int>(QEventLoop::AllEvents)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ExcludeUserInputEvents", QScriptValue(engine, static_cast<int>(QEventLoop::ExcludeUserInputEvents)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ExcludeSocketNotifiers", QScriptValue(engine, static_cast<int>(QEventLoop::ExcludeSocketNotifiers)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WaitForMoreEvents", QScriptValue(engine, static_cast<int>(QEventLoop::WaitForMoreEvents)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("X11ExcludeTimers", QScriptValue(engine, static_cast<int>(QEventLoop::X11ExcludeTimers)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DeferredDeletion", QScriptValue(engine, static_cast<int>(QEventLoop::DeferredDeletion)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QEventLoop::ProcessEventsFlags>(engine);
    return cv;
}

// QSignalMapper

Q_DECLARE_METATYPE(QSignalMapper*)

class Constructor_QSignalMapper:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* parent = 0)
{ return this->engine()->newQObject(new QSignalMapper(parent), QScriptEngine::AutoOwnership); }
};

class Prototype_QSignalMapper:
public QSignalMapper, protected QScriptable
{
    Q_OBJECT
private:
inline QSignalMapper *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QSignalMapper*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QSignalMapper.prototype.%0: this object is not a QSignalMapper")
            .arg(method));
}
public:
Prototype_QSignalMapper(QObject *parent = 0)
    : QSignalMapper() { setParent(parent); }
public Q_SLOTS:
QObject* mapping(QObject* object) const
{ if (QSignalMapper *_q_q = this->self()) return _q_q->mapping(object);
  throwTypeError(QLatin1String("mapping")); return 0; }
QObject* mapping(QWidget* widget) const
{ if (QSignalMapper *_q_q = this->self()) return _q_q->mapping(widget);
  throwTypeError(QLatin1String("mapping")); return 0; }
QObject* mapping(QString const& text) const
{ if (QSignalMapper *_q_q = this->self()) return _q_q->mapping(text);
  throwTypeError(QLatin1String("mapping")); return 0; }
QObject* mapping(int id) const
{ if (QSignalMapper *_q_q = this->self()) return _q_q->mapping(id);
  throwTypeError(QLatin1String("mapping")); return 0; }
void map(QObject* sender)
{ if (QSignalMapper *_q_q = this->self()) _q_q->map(sender);
  else throwTypeError(QLatin1String("map")); }
void map()
{ if (QSignalMapper *_q_q = this->self()) _q_q->map();
  else throwTypeError(QLatin1String("map")); }
void removeMappings(QObject* sender)
{ if (QSignalMapper *_q_q = this->self()) _q_q->removeMappings(sender);
  else throwTypeError(QLatin1String("removeMappings")); }
void setMapping(QObject* sender, QObject* object)
{ if (QSignalMapper *_q_q = this->self()) _q_q->setMapping(sender, object);
  else throwTypeError(QLatin1String("setMapping")); }
void setMapping(QObject* sender, QWidget* widget)
{ if (QSignalMapper *_q_q = this->self()) _q_q->setMapping(sender, widget);
  else throwTypeError(QLatin1String("setMapping")); }
void setMapping(QObject* sender, QString const& text)
{ if (QSignalMapper *_q_q = this->self()) _q_q->setMapping(sender, text);
  else throwTypeError(QLatin1String("setMapping")); }
void setMapping(QObject* sender, int id)
{ if (QSignalMapper *_q_q = this->self()) _q_q->setMapping(sender, id);
  else throwTypeError(QLatin1String("setMapping")); }
QString toString() const
{ return QLatin1String("QSignalMapper"); }
private:
Q_DISABLE_COPY(Prototype_QSignalMapper)
};

static QScriptValue create_QSignalMapper_class(QScriptEngine *engine)
{
    Prototype_QSignalMapper *pq = new Prototype_QSignalMapper;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QSignalMapper *cq = new Constructor_QSignalMapper;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QSignalMapper>(engine, pv);
    return cv;
}

// QIODevice

Q_DECLARE_METATYPE(QIODevice*)
Q_DECLARE_METATYPE(QIODevice::OpenModeFlag)
Q_DECLARE_METATYPE(QIODevice::OpenMode)

class Constructor_QIODevice:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* /* parent */)
{ return this->context()->throwError(QString::fromLatin1("QIODevice cannot be instantiated")); }
QScriptValue qscript_call()
{ return this->context()->throwError(QString::fromLatin1("QIODevice cannot be instantiated")); }
};

class Prototype_QIODevice:
public QIODevice, protected QScriptable
{
    Q_OBJECT
private:
inline QIODevice *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QIODevice*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QIODevice.prototype.%0: this object is not a QIODevice")
            .arg(method));
}
public:
Prototype_QIODevice(QObject *parent = 0)
    : QIODevice() { setParent(parent); }
public Q_SLOTS:
bool canReadLine() const
{ if (QIODevice *_q_q = this->self()) return _q_q->canReadLine();
  throwTypeError(QLatin1String("canReadLine")); return bool(); }
bool seek(qint64 pos)
{ if (QIODevice *_q_q = this->self()) return _q_q->seek(pos);
  throwTypeError(QLatin1String("seek")); return bool(); }
QByteArray readLine(qint64 maxlen = 0)
{ if (QIODevice *_q_q = this->self()) return _q_q->readLine(maxlen);
  throwTypeError(QLatin1String("readLine")); return QByteArray(); }
qint64 readLine(char* data, qint64 maxlen)
{ if (QIODevice *_q_q = this->self()) return _q_q->readLine(data, maxlen);
  throwTypeError(QLatin1String("readLine")); return qint64(); }
qint64 pos() const
{ if (QIODevice *_q_q = this->self()) return _q_q->pos();
  throwTypeError(QLatin1String("pos")); return qint64(); }
qint64 bytesAvailable() const
{ if (QIODevice *_q_q = this->self()) return _q_q->bytesAvailable();
  throwTypeError(QLatin1String("bytesAvailable")); return qint64(); }
bool isOpen() const
{ if (QIODevice *_q_q = this->self()) return _q_q->isOpen();
  throwTypeError(QLatin1String("isOpen")); return bool(); }
bool getChar(char* c)
{ if (QIODevice *_q_q = this->self()) return _q_q->getChar(c);
  throwTypeError(QLatin1String("getChar")); return bool(); }
bool isWritable() const
{ if (QIODevice *_q_q = this->self()) return _q_q->isWritable();
  throwTypeError(QLatin1String("isWritable")); return bool(); }
qint64 size() const
{ if (QIODevice *_q_q = this->self()) return _q_q->size();
  throwTypeError(QLatin1String("size")); return qint64(); }
bool waitForReadyRead(int msecs)
{ if (QIODevice *_q_q = this->self()) return _q_q->waitForReadyRead(msecs);
  throwTypeError(QLatin1String("waitForReadyRead")); return bool(); }
void ungetChar(char c)
{ if (QIODevice *_q_q = this->self()) _q_q->ungetChar(c);
  else throwTypeError(QLatin1String("ungetChar")); }
void close()
{ if (QIODevice *_q_q = this->self()) _q_q->close();
  else throwTypeError(QLatin1String("close")); }
QIODevice::OpenMode openMode() const
{ if (QIODevice *_q_q = this->self()) return _q_q->openMode();
  throwTypeError(QLatin1String("openMode")); return QIODevice::OpenMode(); }
bool atEnd() const
{ if (QIODevice *_q_q = this->self()) return _q_q->atEnd();
  throwTypeError(QLatin1String("atEnd")); return bool(); }
QString errorString() const
{ if (QIODevice *_q_q = this->self()) return _q_q->errorString();
  throwTypeError(QLatin1String("errorString")); return QString(); }
bool putChar(char c)
{ if (QIODevice *_q_q = this->self()) return _q_q->putChar(c);
  throwTypeError(QLatin1String("putChar")); return bool(); }
qint64 write(QByteArray const& data)
{ if (QIODevice *_q_q = this->self()) return _q_q->write(data);
  throwTypeError(QLatin1String("write")); return qint64(); }
qint64 write(char const* data, qint64 len)
{ if (QIODevice *_q_q = this->self()) return _q_q->write(data, len);
  throwTypeError(QLatin1String("write")); return qint64(); }
bool isReadable() const
{ if (QIODevice *_q_q = this->self()) return _q_q->isReadable();
  throwTypeError(QLatin1String("isReadable")); return bool(); }
bool open(QIODevice::OpenMode mode)
{ if (QIODevice *_q_q = this->self()) return _q_q->open(mode);
  throwTypeError(QLatin1String("open")); return bool(); }
bool isTextModeEnabled() const
{ if (QIODevice *_q_q = this->self()) return _q_q->isTextModeEnabled();
  throwTypeError(QLatin1String("isTextModeEnabled")); return bool(); }
void setTextModeEnabled(bool enabled)
{ if (QIODevice *_q_q = this->self()) _q_q->setTextModeEnabled(enabled);
  else throwTypeError(QLatin1String("setTextModeEnabled")); }
QByteArray peek(qint64 maxlen)
{ if (QIODevice *_q_q = this->self()) return _q_q->peek(maxlen);
  throwTypeError(QLatin1String("peek")); return QByteArray(); }
qint64 peek(char* data, qint64 maxlen)
{ if (QIODevice *_q_q = this->self()) return _q_q->peek(data, maxlen);
  throwTypeError(QLatin1String("peek")); return qint64(); }
bool isSequential() const
{ if (QIODevice *_q_q = this->self()) return _q_q->isSequential();
  throwTypeError(QLatin1String("isSequential")); return bool(); }
QByteArray readAll()
{ if (QIODevice *_q_q = this->self()) return _q_q->readAll();
  throwTypeError(QLatin1String("readAll")); return QByteArray(); }
bool waitForBytesWritten(int msecs)
{ if (QIODevice *_q_q = this->self()) return _q_q->waitForBytesWritten(msecs);
  throwTypeError(QLatin1String("waitForBytesWritten")); return bool(); }
bool reset()
{ if (QIODevice *_q_q = this->self()) return _q_q->reset();
  throwTypeError(QLatin1String("reset")); return bool(); }
QByteArray read(qint64 maxlen)
{ if (QIODevice *_q_q = this->self()) return _q_q->read(maxlen);
  throwTypeError(QLatin1String("read")); return QByteArray(); }
qint64 read(char* data, qint64 maxlen)
{ if (QIODevice *_q_q = this->self()) return _q_q->read(data, maxlen);
  throwTypeError(QLatin1String("read")); return qint64(); }
qint64 bytesToWrite() const
{ if (QIODevice *_q_q = this->self()) return _q_q->bytesToWrite();
  throwTypeError(QLatin1String("bytesToWrite")); return qint64(); }
QString toString() const
{ return QLatin1String("QIODevice"); }
public:
qint64 readData(char* /* data */, qint64 /* maxlen */) { return qint64(); }
qint64 writeData(char const* /* data */, qint64 /* len */) { return qint64(); }
private:
Q_DISABLE_COPY(Prototype_QIODevice)
};

static QScriptValue create_QIODevice_class(QScriptEngine *engine)
{
    Prototype_QIODevice *pq = new Prototype_QIODevice;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QIODevice *cq = new Constructor_QIODevice;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QIODevice>(engine, pv);
    _q_ScriptRegisterEnumMetaType<QIODevice::OpenModeFlag>(engine);
    cv.setProperty("NotOpen", QScriptValue(engine, static_cast<int>(QIODevice::NotOpen)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadOnly", QScriptValue(engine, static_cast<int>(QIODevice::ReadOnly)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WriteOnly", QScriptValue(engine, static_cast<int>(QIODevice::WriteOnly)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadWrite", QScriptValue(engine, static_cast<int>(QIODevice::ReadWrite)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Append", QScriptValue(engine, static_cast<int>(QIODevice::Append)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Truncate", QScriptValue(engine, static_cast<int>(QIODevice::Truncate)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Text", QScriptValue(engine, static_cast<int>(QIODevice::Text)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Unbuffered", QScriptValue(engine, static_cast<int>(QIODevice::Unbuffered)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QIODevice::OpenMode>(engine);
    return cv;
}

// QCoreApplication

Q_DECLARE_METATYPE(QCoreApplication*)
Q_DECLARE_METATYPE(QCoreApplication::Encoding)

class Constructor_QCoreApplication:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(int& argc, char** argv)
{ return this->engine()->newQObject(new QCoreApplication(argc, argv), QScriptEngine::AutoOwnership); }
void setOrganizationDomain(QString const& orgDomain)
{ QCoreApplication::setOrganizationDomain(orgDomain); }
void quit()
{ QCoreApplication::quit(); }
void processEvents(QEventLoop::ProcessEventsFlags flags, int maxtime)
{ QCoreApplication::processEvents(flags, maxtime); }
void processEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents)
{ QCoreApplication::processEvents(flags); }
void removeLibraryPath(QString const& arg1)
{ QCoreApplication::removeLibraryPath(arg1); }
QString organizationName()
{ return QCoreApplication::organizationName(); }
void setApplicationName(QString const& application)
{ QCoreApplication::setApplicationName(application); }
void removePostedEvents(QObject* receiver, int eventType)
{ QCoreApplication::removePostedEvents(receiver, eventType); }
void removePostedEvents(QObject* receiver)
{ QCoreApplication::removePostedEvents(receiver); }
QStringList arguments()
{ return QCoreApplication::arguments(); }
QStringList libraryPaths()
{ return QCoreApplication::libraryPaths(); }
void removeTranslator(QTranslator* messageFile)
{ QCoreApplication::removeTranslator(messageFile); }
QString organizationDomain()
{ return QCoreApplication::organizationDomain(); }
void sendPostedEvents()
{ QCoreApplication::sendPostedEvents(); }
void sendPostedEvents(QObject* receiver, int event_type)
{ QCoreApplication::sendPostedEvents(receiver, event_type); }
bool testAttribute(Qt::ApplicationAttribute attribute)
{ return QCoreApplication::testAttribute(attribute); }
void installTranslator(QTranslator* messageFile)
{ QCoreApplication::installTranslator(messageFile); }
bool sendEvent(QObject* receiver, QEvent* event)
{ return QCoreApplication::sendEvent(receiver, event); }
void postEvent(QObject* receiver, QEvent* event, int priority)
{ QCoreApplication::postEvent(receiver, event, priority); }
void postEvent(QObject* receiver, QEvent* event)
{ QCoreApplication::postEvent(receiver, event); }
void addLibraryPath(QString const& arg1)
{ QCoreApplication::addLibraryPath(arg1); }
void setLibraryPaths(QStringList const& arg1)
{ QCoreApplication::setLibraryPaths(arg1); }
QString applicationName()
{ return QCoreApplication::applicationName(); }
bool hasPendingEvents()
{ return QCoreApplication::hasPendingEvents(); }
bool startingUp()
{ return QCoreApplication::startingUp(); }
QString translate(char const* context, char const* key, char const* comment, QCoreApplication::Encoding encoding, int n)
{ return QCoreApplication::translate(context, key, comment, encoding, n); }
QString translate(char const* context, char const* key, char const* comment = 0, QCoreApplication::Encoding encoding = QCoreApplication::CodecForTr)
{ return QCoreApplication::translate(context, key, comment, encoding); }
QString applicationFilePath()
{ return QCoreApplication::applicationFilePath(); }
int exec()
{ return QCoreApplication::exec(); }
void setAttribute(Qt::ApplicationAttribute attribute, bool on = true)
{ QCoreApplication::setAttribute(attribute, on); }
void setOrganizationName(QString const& orgName)
{ QCoreApplication::setOrganizationName(orgName); }
void flush()
{ QCoreApplication::flush(); }
QString applicationDirPath()
{ return QCoreApplication::applicationDirPath(); }
bool closingDown()
{ return QCoreApplication::closingDown(); }
void exit(int retcode = 0)
{ QCoreApplication::exit(retcode); }
QCoreApplication* instance()
{ return QCoreApplication::instance(); }
};

class Prototype_QCoreApplication:
public QObject, protected QScriptable
{
    Q_OBJECT
private:
inline QCoreApplication *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QCoreApplication*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QCoreApplication.prototype.%0: this object is not a QCoreApplication")
            .arg(method));
}
public:
Prototype_QCoreApplication(QObject *parent = 0)
 { setParent(parent); }
public Q_SLOTS:
bool notify(QObject* arg1, QEvent* arg2)
{ if (QCoreApplication *_q_q = this->self()) return _q_q->notify(arg1, arg2);
  throwTypeError(QLatin1String("notify")); return bool(); }
void sendPostedEvents()
{ if (QCoreApplication *_q_q = this->self()) _q_q->sendPostedEvents();
  else throwTypeError(QLatin1String("sendPostedEvents")); }
bool sendEvent(QObject* receiver, QEvent* event)
{ if (QCoreApplication *_q_q = this->self()) return _q_q->sendEvent(receiver, event);
  throwTypeError(QLatin1String("sendEvent")); return bool(); }
QCoreApplication::EventFilter setEventFilter(QCoreApplication::EventFilter filter)
{ if (QCoreApplication *_q_q = this->self()) return _q_q->setEventFilter(filter);
  throwTypeError(QLatin1String("setEventFilter")); return QCoreApplication::EventFilter(); }
bool filterEvent(void* message, long* result)
{ if (QCoreApplication *_q_q = this->self()) return _q_q->filterEvent(message, result);
  throwTypeError(QLatin1String("filterEvent")); return bool(); }
QString toString() const
{ return QLatin1String("QCoreApplication"); }
private:
Q_DISABLE_COPY(Prototype_QCoreApplication)
};

static QScriptValue create_QCoreApplication_class(QScriptEngine *engine)
{
    Prototype_QCoreApplication *pq = new Prototype_QCoreApplication;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QCoreApplication *cq = new Constructor_QCoreApplication;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QCoreApplication>(engine, pv);
    _q_ScriptRegisterEnumMetaType<QCoreApplication::Encoding>(engine);
    cv.setProperty("CodecForTr", QScriptValue(engine, static_cast<int>(QCoreApplication::CodecForTr)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("UnicodeUTF8", QScriptValue(engine, static_cast<int>(QCoreApplication::UnicodeUTF8)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("DefaultCodec", QScriptValue(engine, static_cast<int>(QCoreApplication::DefaultCodec)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QPluginLoader

Q_DECLARE_METATYPE(QPluginLoader*)

class Constructor_QPluginLoader:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QString const& fileName, QObject* parent = 0)
{ return this->engine()->newQObject(new QPluginLoader(fileName, parent), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call(QObject* parent = 0)
{ return this->engine()->newQObject(new QPluginLoader(parent), QScriptEngine::AutoOwnership); }
QObjectList staticInstances()
{ return QPluginLoader::staticInstances(); }
};

class Prototype_QPluginLoader:
public QPluginLoader, protected QScriptable
{
    Q_OBJECT
private:
inline QPluginLoader *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QPluginLoader*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QPluginLoader.prototype.%0: this object is not a QPluginLoader")
            .arg(method));
}
public:
Prototype_QPluginLoader(QObject *parent = 0)
    : QPluginLoader() { setParent(parent); }
public Q_SLOTS:
bool isLoaded() const
{ if (QPluginLoader *_q_q = this->self()) return _q_q->isLoaded();
  throwTypeError(QLatin1String("isLoaded")); return bool(); }
QObject* instance()
{ if (QPluginLoader *_q_q = this->self()) return _q_q->instance();
  throwTypeError(QLatin1String("instance")); return 0; }
QString errorString() const
{ if (QPluginLoader *_q_q = this->self()) return _q_q->errorString();
  throwTypeError(QLatin1String("errorString")); return QString(); }
bool load()
{ if (QPluginLoader *_q_q = this->self()) return _q_q->load();
  throwTypeError(QLatin1String("load")); return bool(); }
void setFileName(QString const& fileName)
{ if (QPluginLoader *_q_q = this->self()) _q_q->setFileName(fileName);
  else throwTypeError(QLatin1String("setFileName")); }
QString fileName() const
{ if (QPluginLoader *_q_q = this->self()) return _q_q->fileName();
  throwTypeError(QLatin1String("fileName")); return QString(); }
bool unload()
{ if (QPluginLoader *_q_q = this->self()) return _q_q->unload();
  throwTypeError(QLatin1String("unload")); return bool(); }
QString toString() const
{ return QLatin1String("QPluginLoader"); }
private:
Q_DISABLE_COPY(Prototype_QPluginLoader)
};

static QScriptValue create_QPluginLoader_class(QScriptEngine *engine)
{
    Prototype_QPluginLoader *pq = new Prototype_QPluginLoader;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QPluginLoader *cq = new Constructor_QPluginLoader;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QPluginLoader>(engine, pv);
    return cv;
}

// QMimeData

Q_DECLARE_METATYPE(QMimeData*)

class Constructor_QMimeData:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call()
{ return this->engine()->newQObject(new QMimeData(), QScriptEngine::AutoOwnership); }
};

class Prototype_QMimeData:
public QMimeData, protected QScriptable
{
    Q_OBJECT
private:
inline QMimeData *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QMimeData*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QMimeData.prototype.%0: this object is not a QMimeData")
            .arg(method));
}
public:
Prototype_QMimeData(QObject *parent = 0)
    : QMimeData() { setParent(parent); }
public Q_SLOTS:
QVariant imageData() const
{ if (QMimeData *_q_q = this->self()) return _q_q->imageData();
  throwTypeError(QLatin1String("imageData")); return QVariant(); }
bool hasHtml() const
{ if (QMimeData *_q_q = this->self()) return _q_q->hasHtml();
  throwTypeError(QLatin1String("hasHtml")); return bool(); }
bool hasUrls() const
{ if (QMimeData *_q_q = this->self()) return _q_q->hasUrls();
  throwTypeError(QLatin1String("hasUrls")); return bool(); }
bool hasImage() const
{ if (QMimeData *_q_q = this->self()) return _q_q->hasImage();
  throwTypeError(QLatin1String("hasImage")); return bool(); }
QVariant colorData() const
{ if (QMimeData *_q_q = this->self()) return _q_q->colorData();
  throwTypeError(QLatin1String("colorData")); return QVariant(); }
QByteArray data(QString const& mimetype) const
{ if (QMimeData *_q_q = this->self()) return _q_q->data(mimetype);
  throwTypeError(QLatin1String("data")); return QByteArray(); }
bool hasFormat(QString const& mimetype) const
{ if (QMimeData *_q_q = this->self()) return _q_q->hasFormat(mimetype);
  throwTypeError(QLatin1String("hasFormat")); return bool(); }
void setData(QString const& mimetype, QByteArray const& data)
{ if (QMimeData *_q_q = this->self()) _q_q->setData(mimetype, data);
  else throwTypeError(QLatin1String("setData")); }
bool hasColor() const
{ if (QMimeData *_q_q = this->self()) return _q_q->hasColor();
  throwTypeError(QLatin1String("hasColor")); return bool(); }
bool hasText() const
{ if (QMimeData *_q_q = this->self()) return _q_q->hasText();
  throwTypeError(QLatin1String("hasText")); return bool(); }
QString text() const
{ if (QMimeData *_q_q = this->self()) return _q_q->text();
  throwTypeError(QLatin1String("text")); return QString(); }
QStringList formats() const
{ if (QMimeData *_q_q = this->self()) return _q_q->formats();
  throwTypeError(QLatin1String("formats")); return QStringList(); }
void setHtml(QString const& html)
{ if (QMimeData *_q_q = this->self()) _q_q->setHtml(html);
  else throwTypeError(QLatin1String("setHtml")); }
void setColorData(QVariant const& color)
{ if (QMimeData *_q_q = this->self()) _q_q->setColorData(color);
  else throwTypeError(QLatin1String("setColorData")); }
void setUrls(QList<QUrl> const& urls)
{ if (QMimeData *_q_q = this->self()) _q_q->setUrls(urls);
  else throwTypeError(QLatin1String("setUrls")); }
QString html() const
{ if (QMimeData *_q_q = this->self()) return _q_q->html();
  throwTypeError(QLatin1String("html")); return QString(); }
QList<QUrl> urls() const
{ if (QMimeData *_q_q = this->self()) return _q_q->urls();
  throwTypeError(QLatin1String("urls")); return QList<QUrl>(); }
void clear()
{ if (QMimeData *_q_q = this->self()) _q_q->clear();
  else throwTypeError(QLatin1String("clear")); }
void setImageData(QVariant const& image)
{ if (QMimeData *_q_q = this->self()) _q_q->setImageData(image);
  else throwTypeError(QLatin1String("setImageData")); }
void setText(QString const& text)
{ if (QMimeData *_q_q = this->self()) _q_q->setText(text);
  else throwTypeError(QLatin1String("setText")); }
QString toString() const
{ return QLatin1String("QMimeData"); }
private:
Q_DISABLE_COPY(Prototype_QMimeData)
};

static QScriptValue create_QMimeData_class(QScriptEngine *engine)
{
    Prototype_QMimeData *pq = new Prototype_QMimeData;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QObject*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    Constructor_QMimeData *cq = new Constructor_QMimeData;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QMimeData>(engine, pv);
    return cv;
}

// QFSFileEngine

Q_DECLARE_METATYPE(QFSFileEngine*)

class Constructor_QFSFileEngine:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QString const& file)
{ return this->engine()->toScriptValue(new QFSFileEngine(file)); }
QScriptValue qscript_call()
{ return this->engine()->toScriptValue(new QFSFileEngine()); }
bool setCurrentPath(QString const& path)
{ return QFSFileEngine::setCurrentPath(path); }
QString rootPath()
{ return QFSFileEngine::rootPath(); }
QFileInfoList drives()
{ return QFSFileEngine::drives(); }
QString tempPath()
{ return QFSFileEngine::tempPath(); }
QString homePath()
{ return QFSFileEngine::homePath(); }
QString currentPath(QString const& path = QString())
{ return QFSFileEngine::currentPath(path); }
};

class Prototype_QFSFileEngine:
public QObject, public QFSFileEngine, protected QScriptable
{
    Q_OBJECT
private:
inline QFSFileEngine *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QFSFileEngine*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QFSFileEngine.prototype.%0: this object is not a QFSFileEngine")
            .arg(method));
}
public:
Prototype_QFSFileEngine(QObject *parent = 0)
    : QFSFileEngine() { setParent(parent); }
public Q_SLOTS:
int handle() const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->handle();
  throwTypeError(QLatin1String("handle")); return int(); }
bool seek(qint64 arg1)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->seek(arg1);
  throwTypeError(QLatin1String("seek")); return bool(); }
qint64 readLine(char* data, qint64 maxlen)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->readLine(data, maxlen);
  throwTypeError(QLatin1String("readLine")); return qint64(); }
uint ownerId(QAbstractFileEngine::FileOwner arg1) const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->ownerId(arg1);
  throwTypeError(QLatin1String("ownerId")); return uint(); }
qint64 pos() const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->pos();
  throwTypeError(QLatin1String("pos")); return qint64(); }
bool rename(QString const& newName)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->rename(newName);
  throwTypeError(QLatin1String("rename")); return bool(); }
bool extension(QAbstractFileEngine::Extension extension, QAbstractFileEngine::ExtensionOption const* option = 0, QAbstractFileEngine::ExtensionReturn* output = 0)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->extension(extension, option, output);
  throwTypeError(QLatin1String("extension")); return bool(); }
qint64 size() const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->size();
  throwTypeError(QLatin1String("size")); return qint64(); }
void setFileName(QString const& file)
{ if (QFSFileEngine *_q_q = this->self()) _q_q->setFileName(file);
  else throwTypeError(QLatin1String("setFileName")); }
bool close()
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->close();
  throwTypeError(QLatin1String("close")); return bool(); }
bool isRelativePath() const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->isRelativePath();
  throwTypeError(QLatin1String("isRelativePath")); return bool(); }
QStringList entryList(QDir::Filters filters, QStringList const& filterNames) const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->entryList(filters, filterNames);
  throwTypeError(QLatin1String("entryList")); return QStringList(); }
bool setSize(qint64 size)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->setSize(size);
  throwTypeError(QLatin1String("setSize")); return bool(); }
qint64 write(char const* data, qint64 len)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->write(data, len);
  throwTypeError(QLatin1String("write")); return qint64(); }
bool mkdir(QString const& dirName, bool createParentDirectories) const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->mkdir(dirName, createParentDirectories);
  throwTypeError(QLatin1String("mkdir")); return bool(); }
QAbstractFileEngine::Iterator* beginEntryList(QDir::Filters filters, QStringList const& filterNames)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->beginEntryList(filters, filterNames);
  throwTypeError(QLatin1String("beginEntryList")); return 0; }
bool rmdir(QString const& dirName, bool recurseParentDirectories) const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->rmdir(dirName, recurseParentDirectories);
  throwTypeError(QLatin1String("rmdir")); return bool(); }
bool link(QString const& newName)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->link(newName);
  throwTypeError(QLatin1String("link")); return bool(); }
QAbstractFileEngine::Iterator* endEntryList()
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->endEntryList();
  throwTypeError(QLatin1String("endEntryList")); return 0; }
bool open(QIODevice::OpenMode flags, int fd)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->open(flags, fd);
  throwTypeError(QLatin1String("open")); return bool(); }
bool open(QIODevice::OpenMode flags, FILE* fh)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->open(flags, fh);
  throwTypeError(QLatin1String("open")); return bool(); }
bool open(QIODevice::OpenMode openMode)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->open(openMode);
  throwTypeError(QLatin1String("open")); return bool(); }
bool supportsExtension(QAbstractFileEngine::Extension extension) const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->supportsExtension(extension);
  throwTypeError(QLatin1String("supportsExtension")); return bool(); }
bool setPermissions(uint perms)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->setPermissions(perms);
  throwTypeError(QLatin1String("setPermissions")); return bool(); }
bool isSequential() const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->isSequential();
  throwTypeError(QLatin1String("isSequential")); return bool(); }
QString owner(QAbstractFileEngine::FileOwner arg1) const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->owner(arg1);
  throwTypeError(QLatin1String("owner")); return QString(); }
QString fileName(QAbstractFileEngine::FileName file) const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->fileName(file);
  throwTypeError(QLatin1String("fileName")); return QString(); }
bool caseSensitive() const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->caseSensitive();
  throwTypeError(QLatin1String("caseSensitive")); return bool(); }
bool remove()
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->remove();
  throwTypeError(QLatin1String("remove")); return bool(); }
bool flush()
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->flush();
  throwTypeError(QLatin1String("flush")); return bool(); }
qint64 read(char* data, qint64 maxlen)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->read(data, maxlen);
  throwTypeError(QLatin1String("read")); return qint64(); }
QAbstractFileEngine::FileFlags fileFlags(QAbstractFileEngine::FileFlags type) const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->fileFlags(type);
  throwTypeError(QLatin1String("fileFlags")); return QAbstractFileEngine::FileFlags(); }
QDateTime fileTime(QAbstractFileEngine::FileTime time) const
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->fileTime(time);
  throwTypeError(QLatin1String("fileTime")); return QDateTime(); }
bool copy(QString const& newName)
{ if (QFSFileEngine *_q_q = this->self()) return _q_q->copy(newName);
  throwTypeError(QLatin1String("copy")); return bool(); }
QString toString() const
{ return QLatin1String("QFSFileEngine"); }
private:
Q_DISABLE_COPY(Prototype_QFSFileEngine)
};

static QScriptValue create_QFSFileEngine_class(QScriptEngine *engine)
{
    Prototype_QFSFileEngine *pq = new Prototype_QFSFileEngine;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QAbstractFileEngine*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QAbstractFileEngine*>()));
    Constructor_QFSFileEngine *cq = new Constructor_QFSFileEngine;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QFSFileEngine*>(), pv);
    return cv;
}

// QDynamicPropertyChangeEvent

Q_DECLARE_METATYPE(QDynamicPropertyChangeEvent*)

class Constructor_QDynamicPropertyChangeEvent:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QByteArray const& name)
{ return this->engine()->toScriptValue(new QDynamicPropertyChangeEvent(name)); }
};

class Prototype_QDynamicPropertyChangeEvent:
public QObject, public QDynamicPropertyChangeEvent, protected QScriptable
{
    Q_OBJECT
private:
inline QDynamicPropertyChangeEvent *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QDynamicPropertyChangeEvent*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QDynamicPropertyChangeEvent.prototype.%0: this object is not a QDynamicPropertyChangeEvent")
            .arg(method));
}
public:
Prototype_QDynamicPropertyChangeEvent(QObject *parent = 0)
    : QDynamicPropertyChangeEvent(QByteArray()) { setParent(parent); }
public Q_SLOTS:
QByteArray propertyName() const
{ if (QDynamicPropertyChangeEvent *_q_q = this->self()) return _q_q->propertyName();
  throwTypeError(QLatin1String("propertyName")); return QByteArray(); }
QString toString() const
{ return QLatin1String("QDynamicPropertyChangeEvent"); }
private:
Q_DISABLE_COPY(Prototype_QDynamicPropertyChangeEvent)
};

static QScriptValue create_QDynamicPropertyChangeEvent_class(QScriptEngine *engine)
{
    Prototype_QDynamicPropertyChangeEvent *pq = new Prototype_QDynamicPropertyChangeEvent;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QEvent*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QEvent*>()));
    Constructor_QDynamicPropertyChangeEvent *cq = new Constructor_QDynamicPropertyChangeEvent;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QDynamicPropertyChangeEvent*>(), pv);
    return cv;
}

// QTimerEvent

Q_DECLARE_METATYPE(QTimerEvent*)

class Constructor_QTimerEvent:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(int timerId)
{ return this->engine()->toScriptValue(new QTimerEvent(timerId)); }
};

class Prototype_QTimerEvent:
public QObject, public QTimerEvent, protected QScriptable
{
    Q_OBJECT
private:
inline QTimerEvent *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QTimerEvent*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QTimerEvent.prototype.%0: this object is not a QTimerEvent")
            .arg(method));
}
public:
Prototype_QTimerEvent(QObject *parent = 0)
    : QTimerEvent(-1) { setParent(parent); }
public Q_SLOTS:
int timerId() const
{ if (QTimerEvent *_q_q = this->self()) return _q_q->timerId();
  throwTypeError(QLatin1String("timerId")); return int(); }
QString toString() const
{ return QLatin1String("QTimerEvent"); }
private:
Q_DISABLE_COPY(Prototype_QTimerEvent)
};

static QScriptValue create_QTimerEvent_class(QScriptEngine *engine)
{
    Prototype_QTimerEvent *pq = new Prototype_QTimerEvent;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QEvent*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QEvent*>()));
    Constructor_QTimerEvent *cq = new Constructor_QTimerEvent;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QTimerEvent*>(), pv);
    return cv;
}

// QChildEvent

Q_DECLARE_METATYPE(QChildEvent*)

class Constructor_QChildEvent:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QEvent::Type type, QObject* child)
{ return this->engine()->toScriptValue(new QChildEvent(type, child)); }
};

class Prototype_QChildEvent:
public QObject, public QChildEvent, protected QScriptable
{
    Q_OBJECT
private:
inline QChildEvent *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QChildEvent*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QChildEvent.prototype.%0: this object is not a QChildEvent")
            .arg(method));
}
public:
Prototype_QChildEvent(QObject *parent = 0)
    : QChildEvent(QEvent::ChildAdded, 0) { setParent(parent); }
public Q_SLOTS:
bool removed() const
{ if (QChildEvent *_q_q = this->self()) return _q_q->removed();
  throwTypeError(QLatin1String("removed")); return bool(); }
QObject* child() const
{ if (QChildEvent *_q_q = this->self()) return _q_q->child();
  throwTypeError(QLatin1String("child")); return 0; }
bool polished() const
{ if (QChildEvent *_q_q = this->self()) return _q_q->polished();
  throwTypeError(QLatin1String("polished")); return bool(); }
bool added() const
{ if (QChildEvent *_q_q = this->self()) return _q_q->added();
  throwTypeError(QLatin1String("added")); return bool(); }
QString toString() const
{ return QLatin1String("QChildEvent"); }
private:
Q_DISABLE_COPY(Prototype_QChildEvent)
};

static QScriptValue create_QChildEvent_class(QScriptEngine *engine)
{
    Prototype_QChildEvent *pq = new Prototype_QChildEvent;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership,
        QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QEvent*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QEvent*>()));
    Constructor_QChildEvent *cq = new Constructor_QChildEvent;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    engine->setDefaultPrototype(qMetaTypeId<QChildEvent*>(), pv);
    return cv;
}

// QAbstractTableModel

Q_DECLARE_METATYPE(QAbstractTableModel*)

class Constructor_QAbstractTableModel:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* /* parent */ = 0)
{ return this->context()->throwError(QString::fromLatin1("QAbstractTableModel cannot be instantiated")); }
};

class Prototype_QAbstractTableModel:
public QAbstractTableModel, protected QScriptable
{
    Q_OBJECT
private:
inline QAbstractTableModel *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QAbstractTableModel*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QAbstractTableModel.prototype.%0: this object is not a QAbstractTableModel")
            .arg(method));
}
public:
Prototype_QAbstractTableModel(QObject *parent = 0)
    : QAbstractTableModel() { setParent(parent); }
public Q_SLOTS:
QModelIndex index(int row, int column, QModelIndex const& parent = QModelIndex()) const
{ if (QAbstractTableModel *_q_q = this->self()) return _q_q->index(row, column, parent);
  throwTypeError(QLatin1String("index")); return QModelIndex(); }
bool dropMimeData(QMimeData const* data, Qt::DropAction action, int row, int column, QModelIndex const& parent)
{ if (QAbstractTableModel *_q_q = this->self()) return _q_q->dropMimeData(data, action, row, column, parent);
  throwTypeError(QLatin1String("dropMimeData")); return bool(); }
QString toString() const
{ return QLatin1String("QAbstractTableModel"); }
public:
QVariant data(QModelIndex const& /* index */, int /* role */ = Qt::DisplayRole) const { return QVariant(); }
int rowCount(QModelIndex const& /* parent */ = QModelIndex()) const { return int(); }
int columnCount(QModelIndex const& /* parent */ = QModelIndex()) const { return int(); }
private:
Q_DISABLE_COPY(Prototype_QAbstractTableModel)
};

static QScriptValue create_QAbstractTableModel_class(QScriptEngine *engine)
{
    Prototype_QAbstractTableModel *pq = new Prototype_QAbstractTableModel;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QAbstractItemModel*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QAbstractItemModel*>()));
    Constructor_QAbstractTableModel *cq = new Constructor_QAbstractTableModel;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QAbstractTableModel>(engine, pv);
    return cv;
}

// QAbstractListModel

Q_DECLARE_METATYPE(QAbstractListModel*)

class Constructor_QAbstractListModel:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* /* parent */ = 0)
{ return this->context()->throwError(QString::fromLatin1("QAbstractListModel cannot be instantiated")); }
};

class Prototype_QAbstractListModel:
public QAbstractListModel, protected QScriptable
{
    Q_OBJECT
private:
inline QAbstractListModel *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QAbstractListModel*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QAbstractListModel.prototype.%0: this object is not a QAbstractListModel")
            .arg(method));
}
public:
Prototype_QAbstractListModel(QObject *parent = 0)
    : QAbstractListModel() { setParent(parent); }
public Q_SLOTS:
QModelIndex index(int row, int column = 0, QModelIndex const& parent = QModelIndex()) const
{ if (QAbstractListModel *_q_q = this->self()) return _q_q->index(row, column, parent);
  throwTypeError(QLatin1String("index")); return QModelIndex(); }
bool dropMimeData(QMimeData const* data, Qt::DropAction action, int row, int column, QModelIndex const& parent)
{ if (QAbstractListModel *_q_q = this->self()) return _q_q->dropMimeData(data, action, row, column, parent);
  throwTypeError(QLatin1String("dropMimeData")); return bool(); }
QString toString() const
{ return QLatin1String("QAbstractListModel"); }
public:
QVariant data(QModelIndex const& /* index */, int /* role */ = Qt::DisplayRole) const { return QVariant(); }
int rowCount(QModelIndex const& /* parent */ = QModelIndex()) const { return int(); }
private:
Q_DISABLE_COPY(Prototype_QAbstractListModel)
};

static QScriptValue create_QAbstractListModel_class(QScriptEngine *engine)
{
    Prototype_QAbstractListModel *pq = new Prototype_QAbstractListModel;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QAbstractItemModel*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QAbstractItemModel*>()));
    Constructor_QAbstractListModel *cq = new Constructor_QAbstractListModel;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QAbstractListModel>(engine, pv);
    return cv;
}

// QProcess

Q_DECLARE_METATYPE(QProcess*)
Q_DECLARE_METATYPE(QProcess::ProcessError)
Q_DECLARE_METATYPE(QProcess::ProcessChannelMode)
Q_DECLARE_METATYPE(QProcess::ProcessChannel)
Q_DECLARE_METATYPE(QProcess::ProcessState)
Q_DECLARE_METATYPE(QProcess::ExitStatus)

class Constructor_QProcess:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QObject* parent = 0)
{ return this->engine()->newQObject(new QProcess(parent), QScriptEngine::AutoOwnership); }
int execute(QString const& program)
{ return QProcess::execute(program); }
int execute(QString const& program, QStringList const& arguments)
{ return QProcess::execute(program, arguments); }
QStringList systemEnvironment()
{ return QProcess::systemEnvironment(); }
bool startDetached(QString const& program)
{ return QProcess::startDetached(program); }
bool startDetached(QString const& program, QStringList const& arguments)
{ return QProcess::startDetached(program, arguments); }
bool startDetached(QString const& program, QStringList const& arguments, QString const& workingDirectory, qint64* pid = 0)
{ return QProcess::startDetached(program, arguments, workingDirectory, pid); }
};

class Prototype_QProcess:
public QProcess, protected QScriptable
{
    Q_OBJECT
private:
inline QProcess *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QProcess*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QProcess.prototype.%0: this object is not a QProcess")
            .arg(method));
}
public:
Prototype_QProcess(QObject *parent = 0)
    : QProcess() { setParent(parent); }
~Prototype_QProcess()
{
    setProcessState(QProcess::NotRunning);
}
public Q_SLOTS:
bool canReadLine() const
{ if (QProcess *_q_q = this->self()) return _q_q->canReadLine();
  throwTypeError(QLatin1String("canReadLine")); return bool(); }
void closeReadChannel(QProcess::ProcessChannel channel)
{ if (QProcess *_q_q = this->self()) _q_q->closeReadChannel(channel);
  else throwTypeError(QLatin1String("closeReadChannel")); }
void setReadChannel(QProcess::ProcessChannel channel)
{ if (QProcess *_q_q = this->self()) _q_q->setReadChannel(channel);
  else throwTypeError(QLatin1String("setReadChannel")); }
QStringList environment() const
{ if (QProcess *_q_q = this->self()) return _q_q->environment();
  throwTypeError(QLatin1String("environment")); return QStringList(); }
void setReadChannelMode(QProcess::ProcessChannelMode mode)
{ if (QProcess *_q_q = this->self()) _q_q->setReadChannelMode(mode);
  else throwTypeError(QLatin1String("setReadChannelMode")); }
bool waitForStarted(int msecs = 30000)
{ if (QProcess *_q_q = this->self()) return _q_q->waitForStarted(msecs);
  throwTypeError(QLatin1String("waitForStarted")); return bool(); }
void terminate()
{ if (QProcess *_q_q = this->self()) _q_q->terminate();
  else throwTypeError(QLatin1String("terminate")); }
bool waitForFinished(int msecs = 30000)
{ if (QProcess *_q_q = this->self()) return _q_q->waitForFinished(msecs);
  throwTypeError(QLatin1String("waitForFinished")); return bool(); }
QProcess::ProcessError error() const
{ if (QProcess *_q_q = this->self()) return _q_q->error();
  throwTypeError(QLatin1String("error")); return QProcess::ProcessError(); }
qint64 bytesAvailable() const
{ if (QProcess *_q_q = this->self()) return _q_q->bytesAvailable();
  throwTypeError(QLatin1String("bytesAvailable")); return qint64(); }
void setWorkingDirectory(QString const& dir)
{ if (QProcess *_q_q = this->self()) _q_q->setWorkingDirectory(dir);
  else throwTypeError(QLatin1String("setWorkingDirectory")); }
void closeWriteChannel()
{ if (QProcess *_q_q = this->self()) _q_q->closeWriteChannel();
  else throwTypeError(QLatin1String("closeWriteChannel")); }
QString workingDirectory() const
{ if (QProcess *_q_q = this->self()) return _q_q->workingDirectory();
  throwTypeError(QLatin1String("workingDirectory")); return QString(); }
bool waitForReadyRead(int msecs = 30000)
{ if (QProcess *_q_q = this->self()) return _q_q->waitForReadyRead(msecs);
  throwTypeError(QLatin1String("waitForReadyRead")); return bool(); }
void start(QString const& program, QIODevice::OpenMode mode = QIODevice::ReadWrite)
{ if (QProcess *_q_q = this->self()) _q_q->start(program, mode);
  else throwTypeError(QLatin1String("start")); }
void start(QString const& program, QStringList const& arguments, QIODevice::OpenMode mode = QIODevice::ReadWrite)
{ if (QProcess *_q_q = this->self()) _q_q->start(program, arguments, mode);
  else throwTypeError(QLatin1String("start")); }
void setStandardInputFile(QString const& fileName)
{ if (QProcess *_q_q = this->self()) _q_q->setStandardInputFile(fileName);
  else throwTypeError(QLatin1String("setStandardInputFile")); }
void close()
{ if (QProcess *_q_q = this->self()) _q_q->close();
  else throwTypeError(QLatin1String("close")); }
QProcess::ProcessChannelMode processChannelMode() const
{ if (QProcess *_q_q = this->self()) return _q_q->processChannelMode();
  throwTypeError(QLatin1String("processChannelMode")); return QProcess::ProcessChannelMode(); }
void setStandardOutputFile(QString const& fileName, QIODevice::OpenMode mode = QIODevice::Truncate)
{ if (QProcess *_q_q = this->self()) _q_q->setStandardOutputFile(fileName, mode);
  else throwTypeError(QLatin1String("setStandardOutputFile")); }
bool atEnd() const
{ if (QProcess *_q_q = this->self()) return _q_q->atEnd();
  throwTypeError(QLatin1String("atEnd")); return bool(); }
QByteArray readAllStandardError()
{ if (QProcess *_q_q = this->self()) return _q_q->readAllStandardError();
  throwTypeError(QLatin1String("readAllStandardError")); return QByteArray(); }
Q_PID pid() const
{ if (QProcess *_q_q = this->self()) return _q_q->pid();
  throwTypeError(QLatin1String("pid")); return Q_PID(); }
QProcess::ProcessState state() const
{ if (QProcess *_q_q = this->self()) return _q_q->state();
  throwTypeError(QLatin1String("state")); return QProcess::ProcessState(); }
bool isSequential() const
{ if (QProcess *_q_q = this->self()) return _q_q->isSequential();
  throwTypeError(QLatin1String("isSequential")); return bool(); }
bool waitForBytesWritten(int msecs = 30000)
{ if (QProcess *_q_q = this->self()) return _q_q->waitForBytesWritten(msecs);
  throwTypeError(QLatin1String("waitForBytesWritten")); return bool(); }
QByteArray readAllStandardOutput()
{ if (QProcess *_q_q = this->self()) return _q_q->readAllStandardOutput();
  throwTypeError(QLatin1String("readAllStandardOutput")); return QByteArray(); }
void setEnvironment(QStringList const& environment)
{ if (QProcess *_q_q = this->self()) _q_q->setEnvironment(environment);
  else throwTypeError(QLatin1String("setEnvironment")); }
int exitCode() const
{ if (QProcess *_q_q = this->self()) return _q_q->exitCode();
  throwTypeError(QLatin1String("exitCode")); return int(); }
QProcess::ProcessChannel readChannel() const
{ if (QProcess *_q_q = this->self()) return _q_q->readChannel();
  throwTypeError(QLatin1String("readChannel")); return QProcess::ProcessChannel(); }
void setProcessChannelMode(QProcess::ProcessChannelMode mode)
{ if (QProcess *_q_q = this->self()) _q_q->setProcessChannelMode(mode);
  else throwTypeError(QLatin1String("setProcessChannelMode")); }
void setStandardOutputProcess(QProcess* destination)
{ if (QProcess *_q_q = this->self()) _q_q->setStandardOutputProcess(destination);
  else throwTypeError(QLatin1String("setStandardOutputProcess")); }
QProcess::ProcessChannelMode readChannelMode() const
{ if (QProcess *_q_q = this->self()) return _q_q->readChannelMode();
  throwTypeError(QLatin1String("readChannelMode")); return QProcess::ProcessChannelMode(); }
qint64 bytesToWrite() const
{ if (QProcess *_q_q = this->self()) return _q_q->bytesToWrite();
  throwTypeError(QLatin1String("bytesToWrite")); return qint64(); }
QProcess::ExitStatus exitStatus() const
{ if (QProcess *_q_q = this->self()) return _q_q->exitStatus();
  throwTypeError(QLatin1String("exitStatus")); return QProcess::ExitStatus(); }
void kill()
{ if (QProcess *_q_q = this->self()) _q_q->kill();
  else throwTypeError(QLatin1String("kill")); }
void setStandardErrorFile(QString const& fileName, QIODevice::OpenMode mode = QIODevice::Truncate)
{ if (QProcess *_q_q = this->self()) _q_q->setStandardErrorFile(fileName, mode);
  else throwTypeError(QLatin1String("setStandardErrorFile")); }
QString toString() const
{ return QLatin1String("QProcess"); }
private:
Q_DISABLE_COPY(Prototype_QProcess)
};

static QScriptValue create_QProcess_class(QScriptEngine *engine)
{
    Prototype_QProcess *pq = new Prototype_QProcess;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QIODevice*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QIODevice*>()));
    Constructor_QProcess *cq = new Constructor_QProcess;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QProcess>(engine, pv);
    _q_ScriptRegisterEnumMetaType<QProcess::ProcessError>(engine);
    cv.setProperty("FailedToStart", QScriptValue(engine, static_cast<int>(QProcess::FailedToStart)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Crashed", QScriptValue(engine, static_cast<int>(QProcess::Crashed)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Timedout", QScriptValue(engine, static_cast<int>(QProcess::Timedout)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadError", QScriptValue(engine, static_cast<int>(QProcess::ReadError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WriteError", QScriptValue(engine, static_cast<int>(QProcess::WriteError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("UnknownError", QScriptValue(engine, static_cast<int>(QProcess::UnknownError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QProcess::ProcessChannelMode>(engine);
    cv.setProperty("SeparateChannels", QScriptValue(engine, static_cast<int>(QProcess::SeparateChannels)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("MergedChannels", QScriptValue(engine, static_cast<int>(QProcess::MergedChannels)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ForwardedChannels", QScriptValue(engine, static_cast<int>(QProcess::ForwardedChannels)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QProcess::ProcessChannel>(engine);
    cv.setProperty("StandardOutput", QScriptValue(engine, static_cast<int>(QProcess::StandardOutput)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("StandardError", QScriptValue(engine, static_cast<int>(QProcess::StandardError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QProcess::ProcessState>(engine);
    cv.setProperty("NotRunning", QScriptValue(engine, static_cast<int>(QProcess::NotRunning)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Starting", QScriptValue(engine, static_cast<int>(QProcess::Starting)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("Running", QScriptValue(engine, static_cast<int>(QProcess::Running)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QProcess::ExitStatus>(engine);
    cv.setProperty("NormalExit", QScriptValue(engine, static_cast<int>(QProcess::NormalExit)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CrashExit", QScriptValue(engine, static_cast<int>(QProcess::CrashExit)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    return cv;
}

// QFile

Q_DECLARE_METATYPE(QFile*)
Q_DECLARE_METATYPE(QFile::FileError)
Q_DECLARE_METATYPE(QFile::Permission)
Q_DECLARE_METATYPE(QFile::Permissions)

class Constructor_QFile:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QString const& name, QObject* parent)
{ return this->engine()->newQObject(new QFile(name, parent), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call(QObject* parent)
{ return this->engine()->newQObject(new QFile(parent), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call(QString const& name)
{ return this->engine()->newQObject(new QFile(name), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call()
{ return this->engine()->newQObject(new QFile(), QScriptEngine::AutoOwnership); }
bool exists(QString const& fileName)
{ return QFile::exists(fileName); }
QFile::Permissions permissions(QString const& filename)
{ return QFile::permissions(filename); }
QString readLink(QString const& fileName)
{ return QFile::readLink(fileName); }
bool rename(QString const& oldName, QString const& newName)
{ return QFile::rename(oldName, newName); }
QByteArray encodeName(QString const& fileName)
{ return QFile::encodeName(fileName); }
QString decodeName(char const* localFileName)
{ return QFile::decodeName(localFileName); }
QString decodeName(QByteArray const& localFileName)
{ return QFile::decodeName(localFileName); }
void setEncodingFunction(QFile::EncoderFn arg1)
{ QFile::setEncodingFunction(arg1); }
bool link(QString const& oldname, QString const& newName)
{ return QFile::link(oldname, newName); }
bool resize(QString const& filename, qint64 sz)
{ return QFile::resize(filename, sz); }
QString symLinkTarget(QString const& fileName)
{ return QFile::symLinkTarget(fileName); }
bool setPermissions(QString const& filename, QFile::Permissions permissionSpec)
{ return QFile::setPermissions(filename, permissionSpec); }
bool remove(QString const& fileName)
{ return QFile::remove(fileName); }
void setDecodingFunction(QFile::DecoderFn arg1)
{ QFile::setDecodingFunction(arg1); }
bool copy(QString const& fileName, QString const& newName)
{ return QFile::copy(fileName, newName); }
};

class Prototype_QFile:
public QFile, protected QScriptable
{
    Q_OBJECT
private:
inline QFile *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QFile*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QFile.prototype.%0: this object is not a QFile")
            .arg(method));
}
public:
Prototype_QFile(QObject *parent = 0)
    : QFile() { setParent(parent); }
public Q_SLOTS:
int handle() const
{ if (QFile *_q_q = this->self()) return _q_q->handle();
  throwTypeError(QLatin1String("handle")); return int(); }
bool exists() const
{ if (QFile *_q_q = this->self()) return _q_q->exists();
  throwTypeError(QLatin1String("exists")); return bool(); }
QAbstractFileEngine* fileEngine() const
{ if (QFile *_q_q = this->self()) return _q_q->fileEngine();
  throwTypeError(QLatin1String("fileEngine")); return 0; }
bool seek(qint64 offset)
{ if (QFile *_q_q = this->self()) return _q_q->seek(offset);
  throwTypeError(QLatin1String("seek")); return bool(); }
QFile::Permissions permissions() const
{ if (QFile *_q_q = this->self()) return _q_q->permissions();
  throwTypeError(QLatin1String("permissions")); return QFile::Permissions(); }
qint64 pos() const
{ if (QFile *_q_q = this->self()) return _q_q->pos();
  throwTypeError(QLatin1String("pos")); return qint64(); }
QFile::FileError error() const
{ if (QFile *_q_q = this->self()) return _q_q->error();
  throwTypeError(QLatin1String("error")); return QFile::FileError(); }
QString readLink() const
{ if (QFile *_q_q = this->self()) return _q_q->readLink();
  throwTypeError(QLatin1String("readLink")); return QString(); }
bool rename(QString const& newName)
{ if (QFile *_q_q = this->self()) return _q_q->rename(newName);
  throwTypeError(QLatin1String("rename")); return bool(); }
qint64 size() const
{ if (QFile *_q_q = this->self()) return _q_q->size();
  throwTypeError(QLatin1String("size")); return qint64(); }
void setFileName(QString const& name)
{ if (QFile *_q_q = this->self()) _q_q->setFileName(name);
  else throwTypeError(QLatin1String("setFileName")); }
void close()
{ if (QFile *_q_q = this->self()) _q_q->close();
  else throwTypeError(QLatin1String("close")); }
bool atEnd() const
{ if (QFile *_q_q = this->self()) return _q_q->atEnd();
  throwTypeError(QLatin1String("atEnd")); return bool(); }
bool link(QString const& newName)
{ if (QFile *_q_q = this->self()) return _q_q->link(newName);
  throwTypeError(QLatin1String("link")); return bool(); }
bool resize(qint64 sz)
{ if (QFile *_q_q = this->self()) return _q_q->resize(sz);
  throwTypeError(QLatin1String("resize")); return bool(); }
bool open(int fd, QIODevice::OpenMode flags)
{ if (QFile *_q_q = this->self()) return _q_q->open(fd, flags);
  throwTypeError(QLatin1String("open")); return bool(); }
bool open(FILE* f, QIODevice::OpenMode flags)
{ if (QFile *_q_q = this->self()) return _q_q->open(f, flags);
  throwTypeError(QLatin1String("open")); return bool(); }
bool open(QIODevice::OpenMode flags)
{ if (QFile *_q_q = this->self()) return _q_q->open(flags);
  throwTypeError(QLatin1String("open")); return bool(); }
QString symLinkTarget() const
{ if (QFile *_q_q = this->self()) return _q_q->symLinkTarget();
  throwTypeError(QLatin1String("symLinkTarget")); return QString(); }
bool setPermissions(QFile::Permissions permissionSpec)
{ if (QFile *_q_q = this->self()) return _q_q->setPermissions(permissionSpec);
  throwTypeError(QLatin1String("setPermissions")); return bool(); }
bool isSequential() const
{ if (QFile *_q_q = this->self()) return _q_q->isSequential();
  throwTypeError(QLatin1String("isSequential")); return bool(); }
QString fileName() const
{ if (QFile *_q_q = this->self()) return _q_q->fileName();
  throwTypeError(QLatin1String("fileName")); return QString(); }
bool remove()
{ if (QFile *_q_q = this->self()) return _q_q->remove();
  throwTypeError(QLatin1String("remove")); return bool(); }
bool flush()
{ if (QFile *_q_q = this->self()) return _q_q->flush();
  throwTypeError(QLatin1String("flush")); return bool(); }
void unsetError()
{ if (QFile *_q_q = this->self()) _q_q->unsetError();
  else throwTypeError(QLatin1String("unsetError")); }
bool copy(QString const& newName)
{ if (QFile *_q_q = this->self()) return _q_q->copy(newName);
  throwTypeError(QLatin1String("copy")); return bool(); }
QString toString() const
{ return QLatin1String("QFile"); }
private:
Q_DISABLE_COPY(Prototype_QFile)
};

static QScriptValue create_QFile_class(QScriptEngine *engine)
{
    Prototype_QFile *pq = new Prototype_QFile;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QIODevice*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QIODevice*>()));
    Constructor_QFile *cq = new Constructor_QFile;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QFile>(engine, pv);
    _q_ScriptRegisterEnumMetaType<QFile::FileError>(engine);
    cv.setProperty("NoError", QScriptValue(engine, static_cast<int>(QFile::NoError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadError", QScriptValue(engine, static_cast<int>(QFile::ReadError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WriteError", QScriptValue(engine, static_cast<int>(QFile::WriteError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("FatalError", QScriptValue(engine, static_cast<int>(QFile::FatalError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ResourceError", QScriptValue(engine, static_cast<int>(QFile::ResourceError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("OpenError", QScriptValue(engine, static_cast<int>(QFile::OpenError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("AbortError", QScriptValue(engine, static_cast<int>(QFile::AbortError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("TimeOutError", QScriptValue(engine, static_cast<int>(QFile::TimeOutError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("UnspecifiedError", QScriptValue(engine, static_cast<int>(QFile::UnspecifiedError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RemoveError", QScriptValue(engine, static_cast<int>(QFile::RemoveError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("RenameError", QScriptValue(engine, static_cast<int>(QFile::RenameError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("PositionError", QScriptValue(engine, static_cast<int>(QFile::PositionError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ResizeError", QScriptValue(engine, static_cast<int>(QFile::ResizeError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("PermissionsError", QScriptValue(engine, static_cast<int>(QFile::PermissionsError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("CopyError", QScriptValue(engine, static_cast<int>(QFile::CopyError)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QFile::Permission>(engine);
    cv.setProperty("ReadOwner", QScriptValue(engine, static_cast<int>(QFile::ReadOwner)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WriteOwner", QScriptValue(engine, static_cast<int>(QFile::WriteOwner)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ExeOwner", QScriptValue(engine, static_cast<int>(QFile::ExeOwner)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadUser", QScriptValue(engine, static_cast<int>(QFile::ReadUser)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WriteUser", QScriptValue(engine, static_cast<int>(QFile::WriteUser)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ExeUser", QScriptValue(engine, static_cast<int>(QFile::ExeUser)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadGroup", QScriptValue(engine, static_cast<int>(QFile::ReadGroup)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WriteGroup", QScriptValue(engine, static_cast<int>(QFile::WriteGroup)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ExeGroup", QScriptValue(engine, static_cast<int>(QFile::ExeGroup)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ReadOther", QScriptValue(engine, static_cast<int>(QFile::ReadOther)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("WriteOther", QScriptValue(engine, static_cast<int>(QFile::WriteOther)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    cv.setProperty("ExeOther", QScriptValue(engine, static_cast<int>(QFile::ExeOther)),
        QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    _q_ScriptRegisterEnumMetaType<QFile::Permissions>(engine);
    return cv;
}

// QBuffer

Q_DECLARE_METATYPE(QBuffer*)

class Constructor_QBuffer:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QByteArray* buf, QObject* parent = 0)
{ return this->engine()->newQObject(new QBuffer(buf, parent), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call(QObject* parent = 0)
{ return this->engine()->newQObject(new QBuffer(parent), QScriptEngine::AutoOwnership); }
};

class Prototype_QBuffer:
public QBuffer, protected QScriptable
{
    Q_OBJECT
private:
inline QBuffer *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QBuffer*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QBuffer.prototype.%0: this object is not a QBuffer")
            .arg(method));
}
public:
Prototype_QBuffer(QObject *parent = 0)
    : QBuffer() { setParent(parent); }
public Q_SLOTS:
qint64 pos() const
{ if (QBuffer *_q_q = this->self()) return _q_q->pos();
  throwTypeError(QLatin1String("pos")); return qint64(); }
QByteArray  buffer() const
{ if (QBuffer *_q_q = this->self()) return _q_q->buffer();
  throwTypeError(QLatin1String("buffer")); return QByteArray(); }
bool seek(qint64 off)
{ if (QBuffer *_q_q = this->self()) return _q_q->seek(off);
  throwTypeError(QLatin1String("seek")); return bool(); }
QByteArray  data() const
{ if (QBuffer *_q_q = this->self()) return _q_q->data();
  throwTypeError(QLatin1String("data")); return QByteArray(); }
void setData(char const* data, int len)
{ if (QBuffer *_q_q = this->self()) _q_q->setData(data, len);
  else throwTypeError(QLatin1String("setData")); }
void setData(QByteArray const& data)
{ if (QBuffer *_q_q = this->self()) _q_q->setData(data);
  else throwTypeError(QLatin1String("setData")); }
void close()
{ if (QBuffer *_q_q = this->self()) _q_q->close();
  else throwTypeError(QLatin1String("close")); }
void setBuffer(QByteArray* a)
{ if (QBuffer *_q_q = this->self()) _q_q->setBuffer(a);
  else throwTypeError(QLatin1String("setBuffer")); }
bool open(QIODevice::OpenMode openMode)
{ if (QBuffer *_q_q = this->self()) return _q_q->open(openMode);
  throwTypeError(QLatin1String("open")); return bool(); }
bool canReadLine() const
{ if (QBuffer *_q_q = this->self()) return _q_q->canReadLine();
  throwTypeError(QLatin1String("canReadLine")); return bool(); }
qint64 size() const
{ if (QBuffer *_q_q = this->self()) return _q_q->size();
  throwTypeError(QLatin1String("size")); return qint64(); }
bool atEnd() const
{ if (QBuffer *_q_q = this->self()) return _q_q->atEnd();
  throwTypeError(QLatin1String("atEnd")); return bool(); }
QString toString() const
{ return QLatin1String("QBuffer"); }
private:
Q_DISABLE_COPY(Prototype_QBuffer)
};

static QScriptValue create_QBuffer_class(QScriptEngine *engine)
{
    Prototype_QBuffer *pq = new Prototype_QBuffer;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QIODevice*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QIODevice*>()));
    Constructor_QBuffer *cq = new Constructor_QBuffer;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QBuffer>(engine, pv);
    return cv;
}

// QTemporaryFile

Q_DECLARE_METATYPE(QTemporaryFile*)

class Constructor_QTemporaryFile:
public QObject, protected QScriptable
{
    Q_OBJECT
public Q_SLOTS:
QScriptValue qscript_call(QString const& templateName, QObject* parent)
{ return this->engine()->newQObject(new QTemporaryFile(templateName, parent), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call(QObject* parent)
{ return this->engine()->newQObject(new QTemporaryFile(parent), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call(QString const& templateName)
{ return this->engine()->newQObject(new QTemporaryFile(templateName), QScriptEngine::AutoOwnership); }
QScriptValue qscript_call()
{ return this->engine()->newQObject(new QTemporaryFile(), QScriptEngine::AutoOwnership); }
QTemporaryFile* createLocalFile(QFile& file)
{ return QTemporaryFile::createLocalFile(file); }
QTemporaryFile* createLocalFile(QString const& fileName)
{ return QTemporaryFile::createLocalFile(fileName); }
};

class Prototype_QTemporaryFile:
public QTemporaryFile, protected QScriptable
{
    Q_OBJECT
private:
inline QTemporaryFile *self() const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return 0;
    return qscriptvalue_cast<QTemporaryFile*>(ctx->thisObject());
}
void throwTypeError(const QString &method) const
{
    QScriptContext *ctx = QScriptable::context();
    if (!ctx)
        return;
    ctx->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QTemporaryFile.prototype.%0: this object is not a QTemporaryFile")
            .arg(method));
}
public:
Prototype_QTemporaryFile(QObject *parent = 0)
    : QTemporaryFile() { setParent(parent); }
public Q_SLOTS:
QString fileName() const
{ if (QTemporaryFile *_q_q = this->self()) return _q_q->fileName();
  throwTypeError(QLatin1String("fileName")); return QString(); }
QAbstractFileEngine* fileEngine() const
{ if (QTemporaryFile *_q_q = this->self()) return _q_q->fileEngine();
  throwTypeError(QLatin1String("fileEngine")); return 0; }
bool autoRemove() const
{ if (QTemporaryFile *_q_q = this->self()) return _q_q->autoRemove();
  throwTypeError(QLatin1String("autoRemove")); return bool(); }
void setAutoRemove(bool b)
{ if (QTemporaryFile *_q_q = this->self()) _q_q->setAutoRemove(b);
  else throwTypeError(QLatin1String("setAutoRemove")); }
void setFileTemplate(QString const& name)
{ if (QTemporaryFile *_q_q = this->self()) _q_q->setFileTemplate(name);
  else throwTypeError(QLatin1String("setFileTemplate")); }
QString fileTemplate() const
{ if (QTemporaryFile *_q_q = this->self()) return _q_q->fileTemplate();
  throwTypeError(QLatin1String("fileTemplate")); return QString(); }
QString toString() const
{ return QLatin1String("QTemporaryFile"); }
private:
Q_DISABLE_COPY(Prototype_QTemporaryFile)
};

static QScriptValue create_QTemporaryFile_class(QScriptEngine *engine)
{
    Prototype_QTemporaryFile *pq = new Prototype_QTemporaryFile;
    QScriptValue pv = engine->newQObject(pq, QScriptEngine::AutoOwnership);
    Q_ASSERT(engine->defaultPrototype(qMetaTypeId<QFile*>()).isValid());
    pv.setPrototype(engine->defaultPrototype(qMetaTypeId<QFile*>()));
    Constructor_QTemporaryFile *cq = new Constructor_QTemporaryFile;
    QScriptValue cv = engine->newQObject(cq, QScriptEngine::AutoOwnership);
    cv.setPrototype(engine->globalObject().property("Function").property("prototype"));
    cv.setProperty("prototype", pv);
    pv.setProperty("constructor", cv);
    _q_ScriptRegisterQObjectMetaType<QTemporaryFile>(engine, pv);
    return cv;
}

// initialize everything
void qscript_initialize_Qt_classes(QScriptEngine *engine)
{
    QScriptValue globalObject = engine->globalObject();
    globalObject.setProperty(QLatin1String("QSystemLocale"), create_QSystemLocale_class(engine));
    globalObject.setProperty(QLatin1String("QWaitCondition"), create_QWaitCondition_class(engine));
    globalObject.setProperty(QLatin1String("QUuid"), create_QUuid_class(engine));
    globalObject.setProperty(QLatin1String("QMetaClassInfo"), create_QMetaClassInfo_class(engine));
    globalObject.setProperty(QLatin1String("QFileInfo"), create_QFileInfo_class(engine));
    globalObject.setProperty(QLatin1String("QLineF"), create_QLineF_class(engine));
    globalObject.setProperty(QLatin1String("QMetaProperty"), create_QMetaProperty_class(engine));
    globalObject.setProperty(QLatin1String("QMetaEnum"), create_QMetaEnum_class(engine));
    globalObject.setProperty(QLatin1String("QStringMatcher"), create_QStringMatcher_class(engine));
    globalObject.setProperty(QLatin1String("QFactoryInterface"), create_QFactoryInterface_class(engine));
    globalObject.setProperty(QLatin1String("QStringRef"), create_QStringRef_class(engine));
    globalObject.setProperty(QLatin1String("QResource"), create_QResource_class(engine));
    globalObject.setProperty(QLatin1String("QMutex"), create_QMutex_class(engine));
    globalObject.setProperty(QLatin1String("QUrl"), create_QUrl_class(engine));
    globalObject.setProperty(QLatin1String("QPointF"), create_QPointF_class(engine));
    globalObject.setProperty(QLatin1String("QObject"), create_QObject_class(engine));
    globalObject.setProperty(QLatin1String("QPoint"), create_QPoint_class(engine));
    globalObject.setProperty(QLatin1String("QCryptographicHash"), create_QCryptographicHash_class(engine));
    globalObject.setProperty(QLatin1String("QBasicTimer"), create_QBasicTimer_class(engine));
    globalObject.setProperty(QLatin1String("QAbstractFileEngine"), create_QAbstractFileEngine_class(engine));
    globalObject.setProperty(QLatin1String("QDateTime"), create_QDateTime_class(engine));
    globalObject.setProperty(QLatin1String("QTime"), create_QTime_class(engine));
    globalObject.setProperty(QLatin1String("QRectF"), create_QRectF_class(engine));
    globalObject.setProperty(QLatin1String("QLocale"), create_QLocale_class(engine));
    globalObject.setProperty(QLatin1String("QRect"), create_QRect_class(engine));
    globalObject.setProperty(QLatin1String("QPersistentModelIndex"), create_QPersistentModelIndex_class(engine));
    globalObject.setProperty(QLatin1String("QSemaphore"), create_QSemaphore_class(engine));
    globalObject.setProperty(QLatin1String("QMetaMethod"), create_QMetaMethod_class(engine));
    globalObject.setProperty(QLatin1String("QTextEncoder"), create_QTextEncoder_class(engine));
    globalObject.setProperty(QLatin1String("QSizeF"), create_QSizeF_class(engine));
    globalObject.setProperty(QLatin1String("QAbstractFileEngineIterator"), create_QAbstractFileEngineIterator_class(engine));
    globalObject.setProperty(QLatin1String("QByteArray"), create_QByteArray_class(engine));
    globalObject.setProperty(QLatin1String("QLine"), create_QLine_class(engine));
    globalObject.setProperty(QLatin1String("QDir"), create_QDir_class(engine));
    globalObject.setProperty(QLatin1String("QBitArray"), create_QBitArray_class(engine));
    globalObject.setProperty(QLatin1String("QSysInfo"), create_QSysInfo_class(engine));
    globalObject.setProperty(QLatin1String("QEvent"), create_QEvent_class(engine));
    globalObject.setProperty(QLatin1String("QTextDecoder"), create_QTextDecoder_class(engine));
    globalObject.setProperty(QLatin1String("QLibraryInfo"), create_QLibraryInfo_class(engine));
    globalObject.setProperty(QLatin1String("QTextCodec"), create_QTextCodec_class(engine));
    globalObject.setProperty(QLatin1String("QTextStream"), create_QTextStream_class(engine));
    //globalObject.setProperty(QLatin1String("QVariant"), create_QVariant_class(engine));
    globalObject.setProperty(QLatin1String("QAbstractFileEngineHandler"), create_QAbstractFileEngineHandler_class(engine));
    globalObject.setProperty(QLatin1String("QReadWriteLock"), create_QReadWriteLock_class(engine));
    globalObject.setProperty(QLatin1String("QModelIndex"), create_QModelIndex_class(engine));
    globalObject.setProperty(QLatin1String("QRegExp"), create_QRegExp_class(engine));
    globalObject.setProperty(QLatin1String("QDate"), create_QDate_class(engine));
    globalObject.setProperty(QLatin1String("QDataStream"), create_QDataStream_class(engine));
    globalObject.setProperty(QLatin1String("QDirIterator"), create_QDirIterator_class(engine));
    globalObject.setProperty(QLatin1String("QChar"), create_QChar_class(engine));
    globalObject.setProperty(QLatin1String("QByteArrayMatcher"), create_QByteArrayMatcher_class(engine));
    globalObject.setProperty(QLatin1String("QSize"), create_QSize_class(engine));
    globalObject.setProperty(QLatin1String("QTimer"), create_QTimer_class(engine));
    globalObject.setProperty(QLatin1String("QObjectCleanupHandler"), create_QObjectCleanupHandler_class(engine));
    globalObject.setProperty(QLatin1String("QSettings"), create_QSettings_class(engine));
    globalObject.setProperty(QLatin1String("QTranslator"), create_QTranslator_class(engine));
    globalObject.setProperty(QLatin1String("QAbstractEventDispatcher"), create_QAbstractEventDispatcher_class(engine));
    globalObject.setProperty(QLatin1String("QTimeLine"), create_QTimeLine_class(engine));
    globalObject.setProperty(QLatin1String("QSocketNotifier"), create_QSocketNotifier_class(engine));
    globalObject.setProperty(QLatin1String("QTextCodecPlugin"), create_QTextCodecPlugin_class(engine));
    globalObject.setProperty(QLatin1String("QLibrary"), create_QLibrary_class(engine));
    globalObject.setProperty(QLatin1String("QThread"), create_QThread_class(engine));
    globalObject.setProperty(QLatin1String("QAbstractItemModel"), create_QAbstractItemModel_class(engine));
    globalObject.setProperty(QLatin1String("QFileSystemWatcher"), create_QFileSystemWatcher_class(engine));
    globalObject.setProperty(QLatin1String("QEventLoop"), create_QEventLoop_class(engine));
    globalObject.setProperty(QLatin1String("QSignalMapper"), create_QSignalMapper_class(engine));
    globalObject.setProperty(QLatin1String("QIODevice"), create_QIODevice_class(engine));
    globalObject.setProperty(QLatin1String("QCoreApplication"), create_QCoreApplication_class(engine));
    globalObject.setProperty(QLatin1String("QPluginLoader"), create_QPluginLoader_class(engine));
    globalObject.setProperty(QLatin1String("QMimeData"), create_QMimeData_class(engine));
    globalObject.setProperty(QLatin1String("QFSFileEngine"), create_QFSFileEngine_class(engine));
    globalObject.setProperty(QLatin1String("QDynamicPropertyChangeEvent"), create_QDynamicPropertyChangeEvent_class(engine));
    globalObject.setProperty(QLatin1String("QTimerEvent"), create_QTimerEvent_class(engine));
    globalObject.setProperty(QLatin1String("QChildEvent"), create_QChildEvent_class(engine));
    globalObject.setProperty(QLatin1String("QAbstractTableModel"), create_QAbstractTableModel_class(engine));
    globalObject.setProperty(QLatin1String("QAbstractListModel"), create_QAbstractListModel_class(engine));
    globalObject.setProperty(QLatin1String("QProcess"), create_QProcess_class(engine));
    globalObject.setProperty(QLatin1String("QFile"), create_QFile_class(engine));
    globalObject.setProperty(QLatin1String("QBuffer"), create_QBuffer_class(engine));
    globalObject.setProperty(QLatin1String("QTemporaryFile"), create_QTemporaryFile_class(engine));
    _q_ScriptRegisterEnumMetaType<Qt::CursorShape>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::Corner>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::Axis>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::Orientation>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::LayoutDirection>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::BGMode>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::AspectRatioMode>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::TextElideMode>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::WindowType>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ItemDataRole>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::SortOrder>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::MatchFlag>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::PenJoinStyle>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::CaseSensitivity>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::BrushStyle>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ClipOperation>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::FocusReason>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ToolBarArea>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::KeyboardModifier>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::DayOfWeek>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::EventPriority>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::DateFormat>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::MaskMode>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::UIEffect>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ContextMenuPolicy>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::AnchorAttribute>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ScrollBarPolicy>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ToolButtonStyle>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::TextFlag>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ItemSelectionMode>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::Key>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ToolBarAreaSizes>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ArrowType>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::FocusPolicy>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::InputMethodQuery>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::DropAction>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::FillRule>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::GlobalColor>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ConnectionType>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::PenCapStyle>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::TransformationMode>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::DockWidgetAreaSizes>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ApplicationAttribute>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ShortcutContext>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::TextInteractionFlag>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::CheckState>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::DockWidgetArea>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::TimeSpec>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ImageConversionFlag>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::WindowModality>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::Modifier>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::AlignmentFlag>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::WidgetAttribute>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::TextFormat>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::MouseButton>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::WindowState>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::PenStyle>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ItemFlag>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ImageConversionFlags>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::KeyboardModifiers>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::DropActions>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::Orientations>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::MouseButtons>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::WindowStates>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ToolBarAreas>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::Alignment>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::TextInteractionFlags>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::WindowFlags>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::DockWidgetAreas>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::ItemFlags>(engine);
    _q_ScriptRegisterEnumMetaType<Qt::MatchFlags>(engine);

}

#include "qtscript_qtcore.moc"

