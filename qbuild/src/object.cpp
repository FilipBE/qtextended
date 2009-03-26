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

#include "object.h"
#include "options.h"
#include "qoutput.h"
#include "project.h"
#include "functionprovider.h"
#include "qbuild.h"

/*!
  \class QMakeObject
  \brief The QMakeObject class represents a node in the QBuild project tree.

  This class is NOT generally threadsafe.  Once QMakeObject::setReadOnly() has been
  called the object and its internal structure can no longer change and thus can
  be freely accessed concurrently from multiple threads.

  QMakeObject is reenterent.

  This class represents an object in QBuild. At it's simplest, an object can be
  created like this.

  \code
  foo.bar=1
  foo.baz=2
  \endcode

  This causes an object with path \c foo to be created. It has properties \c bar and \c baz.
  Objects can be nested. All QBuild variables are objects so QMakeObject is used to
  represent variables.

*/

/*!
  \internal
*/
QMakeObject::QMakeObject(const QMakeObject *parent, const QString &name)
: m_path(name), m_parent(const_cast<QMakeObject *>(parent)), m_isType(false),
  m_readOnly(false), m_watched_sub( 0 )
{
}

/*!
  \internal
*/
QMakeObject::QMakeObject(const QString &path)
: m_path(path), m_parent(0), m_isType(false), m_readOnly(false), m_watched_sub( 0 )
{
}

/*!
  \internal
*/
QMakeObject::~QMakeObject()
{
    for (int ii = 0; ii < m_subscriptions.count(); ++ii)
        m_subscriptions.at(ii)->removeObject(this);
    QHash<QString, QMakeObject *>::ConstIterator iter;
    for (iter = m_properties.begin();
            iter != m_properties.end();
            ++iter)
        delete *iter;
    m_properties.clear();
    m_orderedProperties.clear();
    qDeleteAll(m_indexObjects);
    m_indexObjects.clear();
    if ( m_watched_sub )
        delete m_watched_sub;
}

/*!
  Returns the parent object.
*/
QMakeObject *QMakeObject::parent() const
{
    return m_parent;
}

/*!
  Walks up the tree and returns the top parent.
*/
QMakeObject *QMakeObject::root() const
{
    QMakeObject *cur = const_cast<QMakeObject *>(this);
    while (cur) cur = cur->m_parent;
    return cur;
}

/*!
  Walks up the tree and returns the name of the top parent.
*/
QString QMakeObject::rootPath() const
{
    if (m_parent)
        return m_parent->rootPath();
    else
        return m_path;
}

/*!
  Returns the fully-qualified name of the object.
*/
QString QMakeObject::absoluteName() const
{
    QString abs;
    const QMakeObject *cur = this;
    while (cur->m_parent) {
        if (!abs.isEmpty()) {
           if (cur->m_parent)
               abs.prepend(".");
        }
        abs.prepend(cur->m_path);
        cur = cur->m_parent;
    }

    if (!cur->m_path.endsWith('/'))
        abs.prepend("/");
    abs.prepend(cur->m_path);
    return abs;
}

/*!
  Returns the name of the object relative to its parent.
*/
QString QMakeObject::name() const
{
    return m_path;
}

/*!
  Set the name of the variable to \a path. This does not reposition the
  object in the heirachy, it will still have the same parent.
*/
void QMakeObject::setName(const QString &path)
{
    if (readOnly()) {
        qOutput() << "Attempt to change read only object" << absoluteName() << "QMakeObject::setName";
        QBuild::shutdown();
    }

    m_path = path;
    m_isType = (path == "TYPE" && !m_parent);
}

/*!
  Returns the number of items the object represents.
*/
int QMakeObject::valueCount() const
{
    return m_value.count();
}

/*!
  Returns the items the object represents.
*/
QStringList QMakeObject::value() const
{
    return m_value;
}

/*!
  Set the items the object represents to \a value.
  The \a ctxt object is for debugging purposes.
*/
void QMakeObject::setValue(const QStringList &value, TraceContext *ctxt)
{
    objectTrace(value, ctxt, "set");

    if (readOnly()) {
        qOutput() << "Attempt to change read only object" << absoluteName() << "QMakeObject::setValue";
        QBuild::shutdown();
    }

    QStringList oldValue = m_value;
    m_value = value;
    qDeleteAll(m_indexObjects);
    m_indexObjects.clear();
    for (int ii = 0; ii < m_value.count(); ++ii)
        m_indexObjects.append(0);

    if (!m_subscriptions.isEmpty()) {
        QStringList added;
        QStringList removed;

        for (int ii = 0; ii < oldValue.count(); ++ii)
            if (!value.contains(oldValue.at(ii)))
                removed.append(oldValue.at(ii));
        for (int ii = 0; ii < value.count(); ++ii)
            if (!oldValue.contains(value.at(ii)))
                added.append(value.at(ii));

        doSubscriptions(added, removed);
    }

    if (m_isType) {
        foreach(const QString &val, oldValue)
            delType(val);
        foreach(const QString &val, value)
            newType(val);
    }
}

/*!
  Append \a value to the items the object represents.
  The \a ctxt object is for debugging purposes.
*/
void QMakeObject::addValue(const QStringList &value, TraceContext *ctxt)
{
    objectTrace(value, ctxt, "add");

    if (readOnly()) {
        qOutput() << "Attempt to change read only object" << absoluteName() << "QMakeObject::addValue";
        QBuild::shutdown();
    }

    QStringList oldValue = m_value;
    m_value << value;
    for (int ii = 0; ii < value.count(); ++ii)
        m_indexObjects.append(0);

    if (!m_subscriptions.isEmpty()) {
        QStringList added;
        for (int ii = 0; ii < value.count(); ++ii)
            if (!oldValue.contains(value.at(ii)))
                added.append(value.at(ii));
        doSubscriptions(added, QStringList());
    }

    if (m_isType) {
        foreach(const QString &val, value)
            newType(val);
    }
}

/*!
  For every item in \a value, remove that item from the items the object represents.
  The \a ctxt object is for debugging purposes.
*/
void QMakeObject::subtractValue(const QStringList &value, TraceContext *ctxt)
{
    objectTrace(value, ctxt, "subtract");

    if (readOnly()) {
        qOutput() << "Attempt to change read only object" << absoluteName() << "QMakeObject::subtractValue";
        QBuild::shutdown();
    }

    QStringList oldValue = m_value;
    QStringList removed;
    for (int ii = 0; ii < m_value.count(); ) {
        bool remove = false;
        for (int jj = 0; !remove && jj < value.count(); ++jj)
            if (value.at(jj) == m_value.at(ii))
                remove = true;

        if (remove) {
            if (m_isType)
                removed << m_value.at(ii);
            delete m_indexObjects.at(ii);
            m_value.removeAt(ii);
            m_indexObjects.removeAt(ii);
        } else {
            ++ii;
        }
    }

    if (!m_subscriptions.isEmpty()) {
        QStringList removed;
        for (int ii = 0; ii < value.count(); ++ii)
            if (oldValue.contains(value.at(ii)))
                removed.append(value.at(ii));
        doSubscriptions(QStringList(), removed);
    }

    if (m_isType) {
        foreach(const QString &val, removed)
            delType(val);
    }
}

/*!
  For every item in \a value, append that item to the items the object represents unless it already exists.
  The \a ctxt object is for debugging purposes.
*/
void QMakeObject::uniteValue(const QStringList &value, TraceContext *ctxt)
{
    objectTrace(value, ctxt, "unite");

    if (readOnly()) {
        qOutput() << "Attempt to change read only object" << absoluteName() << "QMakeObject::uniteValue";
        QBuild::shutdown();
    }

    QStringList added;
    for (int ii = 0; ii < value.count(); ++ii) {
        if (!m_value.contains(value.at(ii))) {
            m_value.append(value.at(ii));
            m_indexObjects.append(0);
            added.append(value.at(ii));
        }
    }

    if (!m_subscriptions.isEmpty())
        doSubscriptions(added, QStringList());

    if (m_isType) {
        foreach(const QString &val, added)
            newType(val);
    }
}

/*!
  \internal
*/
bool QMakeObject::isProperty(int index) const
{
    return index < m_indexObjects.count() && m_indexObjects.at(index);
}

/*!
  Returns true if the object has a property called \a property.
*/
bool QMakeObject::isProperty(const QString &property) const
{
    int index = property.indexOf('.');
    if (index == -1) {
        return m_properties.contains(property);
    } else {
        QString myProperty = property.left(index);
        QString subProperty = property.mid(index + 1);

        return isProperty(myProperty) &&
               this->property(myProperty)->isProperty(subProperty);
    }
}

/*!
  Returns a list of the properties the object has.
*/
QStringList QMakeObject::properties() const
{
    return m_properties.keys();
}

/*!
  \internal
*/
QMakeObject *QMakeObject::property(int index) const
{
    if (index >= m_indexObjects.count() || index < 0)
        return 0;
    if (!m_indexObjects.at(index) && !readOnly())
        m_indexObjects[index] = new QMakeObject(this, "~" + QString::number(index));

    QMakeObject *rv = m_indexObjects.at(index);

    return rv;
}

/*!
  \internal
  Returns a list of objects that match something?!?
*/
QList<QMakeObject *> QMakeObject::find(const QString &variable,
                                       const QString &value,
                                       const QString &result)
{
    QList<QMakeObject *> rv;
    QHash<QString, QMakeObject *>::ConstIterator iter =
        m_properties.find(variable);
    if (iter != m_properties.end() &&
            (value.isEmpty() || (*iter)->value().contains(value))) {
        if (result.isEmpty())
            rv << this;
        else
            rv << this->QMakeObject::property(result);
    }

    for (int ii = 0; ii < m_indexObjects.count(); ++ii)
        if (m_indexObjects.at(ii))
            rv << m_indexObjects.at(ii)->find(variable, value, result);

#if 0
    // doing this causes garbage to come out of the iterator
    for (QList<QMakeObject *>::const_iterator it = m_orderedProperties.begin(); it != m_orderedProperties.end(); ++it) {
        QMakeObject *o = (*it);
        rv << o->find(variable, value, result);
    }
#endif

    // foreach copies the list which prevents garbage from coming out of the iterator
    foreach ( QMakeObject *o, m_orderedProperties )
        rv << o->find(variable, value, result);

    return rv;
}

/*!
  Returns an object representing property \a property.
*/
QMakeObject *QMakeObject::property(const QString &property) const
{
    int index = property.indexOf('.');
    if (index == -1) {
        if (property.startsWith('~')) {
            QString num = property.mid(1);
            int index = num.toInt();
            return QMakeObject::property(index);
        } else {
            QHash<QString, QMakeObject *>::ConstIterator iter =
                m_properties.find(property);
            if (iter == m_properties.end())
                if (!readOnly()) {
                    QMakeObject *o = new QMakeObject(this, property);
                    iter = m_properties.insert(property, o);
                    m_orderedProperties.append(o);
                } else
                    return 0;

            return *iter;
        }
    } else {
        QString myProperty = property.left(index);
        QString subProperty = property.mid(index + 1);

        QMakeObject *prop = this->property(myProperty);
        if (prop)
            return prop->property(subProperty);
        else
            return 0;
    }

}

/*!
  \internal
*/
void QMakeObject::addSubscription(QMakeObjectSubscription *sub)
{
    m_subscriptions.append(sub);
}

/*!
  \internal
*/
void QMakeObject::doSubscriptions(const QStringList &added,
                                  const QStringList &removed)
{
    for (int ii = 0; ii < m_subscriptions.count(); ++ii)
        m_subscriptions.at(ii)->objectChanged(this, added, removed);

    // This handles calling the functions specified by the watch() function.
    if ( m_watched_sub ) {
        // We have to find the project
        Project *proj = 0;
        QMakeObject *obj = this;
        while ( proj == 0 && obj->parent() != 0 ) {
            obj = obj->parent();
            proj = dynamic_cast<Project*>(obj);
        }
        if ( proj ) {
            QStringList rv;
            foreach ( const QString &function, m_watched ) {
                FunctionProvider::evalFunction(proj, rv, function, QStringLists());
            }
        }
    }
}

/*!
  \internal
*/
void QMakeObject::remSubscription(QMakeObjectSubscription *sub)
{
    m_subscriptions.removeAll(sub);
}

/*!
  \internal
*/
void QMakeObject::dump()
{
    qWarning() << "Root =" << value();
    dump(1);
}

QMakeObject::TypeFunctions QMakeObject::m_newTypeFunctions;
QMakeObject::TypeFunctions QMakeObject::m_delTypeFunctions;
/*!
  \internal
*/
void QMakeObject::addNewTypeFunction(const QString &type,
                                     const QString &function)
{
    m_newTypeFunctions[type].append(function);
}

/*!
  \internal
*/
void QMakeObject::addDelTypeFunction(const QString &type,
                                     const QString &function)
{
    m_delTypeFunctions[type].append(function);
}

/*!
  \internal
*/
void QMakeObject::removeNewTypeFunction(const QString &type,
                                        const QString &function)
{
    QStringList functions = m_newTypeFunctions[type];
    for (int ii = 0; ii < functions.count(); ++ii)
        if (functions.at(ii) == function) {
            m_newTypeFunctions[type].removeAt(ii);
            return;
        }
}

/*!
  \internal
*/
void QMakeObject::removeDelTypeFunction(const QString &type,
                                        const QString &function)
{
    QStringList functions = m_delTypeFunctions[type];
    for (int ii = 0; ii < functions.count(); ++ii)
        if (functions.at(ii) == function) {
            m_delTypeFunctions[type].removeAt(ii);
            return;
        }
}

/*!
  \internal
*/
bool QMakeObject::readOnly() const
{
    return m_readOnly;
}

/*!
  \internal
*/
void QMakeObject::setReadOnly()
{
    m_readOnly = true;
    for (QList<QMakeObject *>::Iterator iter = m_indexObjects.begin();
            iter != m_indexObjects.end(); ++iter)
        if (*iter)
            (*iter)->setReadOnly();

    for (QHash<QString, QMakeObject *>::Iterator iter = m_properties.begin();
            iter != m_properties.end(); ++iter)
        (*iter)->setReadOnly();
}

/*!
  \internal
*/
void QMakeObject::clearInternal(TraceContext *ctxt)
{
    clear(ctxt);

    for (int ii = 0; ii < m_subscriptions.count(); ++ii)
        m_subscriptions.at(ii)->removeObject(this);
    m_subscriptions.clear();
    if ( m_watched_sub )
        delete m_watched_sub;
    m_watched_sub = 0;
}

/*!
  \internal
*/
void QMakeObject::clear(TraceContext *ctxt)
{
    objectTrace(QStringList(), ctxt, "clear");

    Q_ASSERT(!readOnly());

    for (QList<QMakeObject *>::Iterator iter = m_indexObjects.begin();
            iter != m_indexObjects.end(); ++iter) {
        if (*iter) {
            if (options.debug & Options::Trace)
                (*iter)->clearInternal(ctxt);
            else
                delete *iter;
        }
    }
    if (!(options.debug & Options::Trace))
        m_indexObjects.clear();

    for (QHash<QString, QMakeObject *>::Iterator iter = m_properties.begin();
            iter != m_properties.end(); ++iter) {

        if (options.debug & Options::Trace)
            (*iter)->clearInternal(ctxt);
        else
            delete *iter;
    }
    if (!(options.debug & Options::Trace)) {
        m_properties.clear();
        m_orderedProperties.clear();
    }

    setValue(QStringList(), ctxt);
}

/*!
  \internal
*/
QList<QMakeObject::TraceInfo> QMakeObject::traceInfo() const
{
    return m_traceInfo;
}

/*!
  \internal
*/
void QMakeObject::dump(int ind)
{
    QByteArray indent(ind * 4, ' ');

    qOutput() << indent << "*" << quote(absoluteName());

    for (int ii = 0; ii < valueCount(); ++ii) {
        if (isProperty(ii)) {
            QMakeObject *prop = property(ii);
            qOutput() << indent << "~" << ii << "="
                      << quote(prop->value());
            prop->dump(ind + 1);
        }
    }

    QStringList props = properties();
    for (int ii = 0; ii < props.count(); ++ii) {
        QMakeObject *prop = property(props.at(ii));
        qOutput() << indent << "+" << props.at(ii) << "="
                  << quote(prop->value());
        prop->dump(ind + 1);
    }

    foreach(TraceInfo trace, m_traceInfo) {
        QString outStr = trace.op;
        outStr += "(";
        for (int ii = 0; ii < trace.value.count(); ++ii) {
            if (ii)
                outStr += ", ";
            outStr += "\"";
            outStr += trace.value.at(ii);
            outStr += "\"";
        }
        outStr += ")@" + trace.fileName;
        if (trace.lineNo != -1)
           outStr += ":" + QString::number(trace.lineNo);

        qOutput() << indent << outStr;
    }
}

/*!
  \internal
*/
void QMakeObject::objectTrace(const QStringList &value,
                              TraceContext *ctxt,
                              const char *op)
{
    if (options.debug & Options::Trace) {
        TraceInfo info;
        if (ctxt) {
            info.fileName = ctxt->fileName;
            info.lineNo = ctxt->lineNumber;
        } else {
            info.fileName = "Unknown";
            info.lineNo = -1;
        }
        info.value = value;
        info.op = op;
        m_traceInfo << info;
    }
}

/*!
  Call \a function when this object changes.
*/
void QMakeObject::watch( const QString &function )
{
    // Create a fake registration to force doSubscriptions to run
    if ( !m_watched_sub ) {
        m_watched_sub = new QMakeObjectSubscription();
        m_watched_sub->subscribe(this, m_path);
    }
    m_watched << function;
}

/*!
  \class QMakeObjectSubscription
  \internal
*/
QMakeObjectSubscription::QMakeObjectSubscription(QObject *parent)
: QObject(parent)
{
}

void QMakeObjectSubscription::subscribe(QMakeObject *object,
                                        const QString &name)
{
    m_subs.append(qMakePair(object, name));
    object->addSubscription(this);
}

void QMakeObjectSubscription::objectChanged(QMakeObject *obj,
                                            const QStringList &added,
                                            const QStringList &removed)
{
    QStringList names;
    for (int ii = 0; ii < m_subs.count(); ++ii) {
        if (m_subs.at(ii).first == obj)
            names.append(m_subs.at(ii).second);
    }

    for (int ii = 0; ii < names.count(); ++ii)
        emit valueChanged(names.at(ii), added, removed);
}

void QMakeObjectSubscription::removeObject(QMakeObject *obj)
{
    for (Subscriptions::Iterator iter = m_subs.begin(); iter != m_subs.end();) {
        if (iter->first == obj)
            iter = m_subs.erase(iter);
        else
            ++iter;
    }
}

