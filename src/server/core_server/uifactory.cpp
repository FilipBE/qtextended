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

#include "uifactory.h"
#include <QDebug>
#include <qtopialog.h> 

struct UIFactoryPrivate 
{
    QMap<QByteArray, UIFactory::WidgetCreateFunction> widgets;

};

Q_GLOBAL_STATIC( UIFactoryPrivate, factoryPrivate);

/*!
  \class UIFactory
    \inpublicgroup QtBaseModule
  \brief The UIFactory class provides a factory for various types of widgets used within the Qt Extended server.
  \ingroup QtopiaServer::GeneralUI

  If a widget registers itself via UIFACTORY_REGISTER_WIDGET() an instance of that widget
  can be obtained via the UIFactory. The caller only refers to the class via its name without
  having to include the actual class definition for that widget. Thus by using this factory 
  dependencies between various widget components within the server are reduced.

  \section1 UIFactory vs. Qt Extended Server Widgets

  The decision whether to use the UIFactory or the Qt Extended Server Widget mechanism depends
  on the use case. In general if UI components require extensive interfaces
  Qt Extended Server Widgets should be used. The following table gives a brief overview of
  the advantages and disadvantages of each concept:

  \table
    \header     \o                      
        \o UIFactory 
        \o Server Widgets
    \row        
        \o Interfaces           
        \o Interface is defined by QDialog/QWidget.
        \o Abstract class interfaces such as QAbstractBrowserScreen are used.
    \row
        \o Flexibility
        \o The caller is limited to functionality provided by QWidget and QDialog (some 
            workarounds are possible and can be found \l{UIFactory#Advanced Usage}{here}).
        \o The caller can access a range of methods exposed via the abstract interface.
    \row
        \o Includes
        \o No includes required and therefore no additional dependencies created.
        \o Direct includes of abstract interface headers required which creates more dependencies.
    \row
        \o Mapping
        \o Maps a class name to an instance of that class.
        \o Maps an abstract interface name to an instance of that interface. More details can be found 
            in the \l {QtopiaServerApplication#Qt Extended Server Widgets}{Server Widget documentation}.
    \row
        \o References
        \o Each call to UIFactory::createWidget() will create a new instance of the widget.
        \o Server widgets support the \l{QtopiaServerApplication#Singleton pattern}{singleton pattern}.
  \endtable

  \section1 How to use UIFactory

  The subsequent example defines and registers the new \c ExampleLabel widget.

  \code
    //in examplelabel.cpp
    class ExampleLabel : public QWidget 
    {
        Q_OBJECT
    public:
        ExampleLabel( QWidget *parent = 0, Qt::WFlags fl = 0) {}
    };

    UIFACTORY_REGISTER_WIDGET( ExampleLabel );
  \endcode

  Note that a widget must use the Q_OBJECT macro in order to be accessable via the UIFactory.

  \code
    //in exampleuser.cpp
    void ExampleUser::showExampleLabel()
    {
        QWidget *label = UIFactory::createWidget( "ExampleLabel", this, 0 );
        if ( label ) {
            label->show();
        }
    }
  \endcode
  
  The caller accesses \c ExampleLabel via the interface provided by QWidget. 
  Widgets such as \c ExampleLabel are usually delivered as part of a server component. If the
  component that provides \c ExampleLabel is not deployed as part of the server widget() 
  returns a NULL pointer. Therefore it is always required to check the returned widget pointer 
  for validity and it is the callers responsibility to handle the case of a non-existing 
  ExampleLabel component.

  \section1 Advanced usage

  In some circumstances it may be required to access some additional methods or properties 
  provided by a widget. A typical example would be a dialog that returns a more sophisticated
  error code than the integer value returned by QDialog::exec(). Nevertheless the caller 
  would like to avoid having to include the dialog declaration. 
  
  The following \c SampleDialog demonstrates such an example. Usually \c SampleDialog::errorCode() 
  would not be accessable via the QDialog interface.

  \code
    //in sampledialog.cpp
    class SampleDialog : public QDialog 
    {
        Q_OBJECT
    public:
        SampleDialog( QWidget *parent = 0, Qt::WFlags fl = 0) {}

        QString notCallableMethod;
        Q_INVOKABLE QString errorCode();

    public slots:
        void setParamter( bool param1, int param2 );
    };
    //class definition for SampleDialog
    ...
    UIFACTORY_REGISTER_WIDGET( SampleDialog );
  \endcode

  The following code demonstrates how other code can access SampleDialog methods
  via Qt's \l{Meta-Object System}. 

  \code
    //in dialoguser.cpp
    void DialogUser::showSampleDialog()
    {
        QDialog *dlg = UIFactory::createDialog( "SampleDialog" );
        if ( dlg ) {
            QMetaObject::invokeMethod( dlg, "setParameter", 
                    Qt::DirectConnection, Q_ARG(bool,true), Q_ARG(int,10) )
            QtopiaApplication::execDialog( dlg );
            
            QSlotInvoker returnCode( dlg, SLOT(errorCode()), 0 );
            QList<QVariant> args;
            QString returnString = returnCode->invoke( args ).toString();
            ...
        } else {
            qWarning("SampleDialog not available");
        }
    }
  \endcode

  The meta system allows the invocation of slots or invokable methods via QMetaObject::invokeMethod(). 
  QSlotInvoker is a convenience class with the same purpose. Its main difference is that the user
  doesn't need to know the exact type of the arguments. In general QMetaObject::invokeMethod()
  should be prefered over QSlotInvoker.
  
  
  The only limitation of using Qt's meta system is that 
  only slots and methods marked with the Q_INVOKABLE macros can be invoked via the meta system.
  In the case of the above \c SampleDialog only \c SampleDialog::errorCode() and \c SampleDialog::setParameter()
  can be invoked but not \c SampleDialog::notCallableMethod().

  \sa QSlotInvoker, QtopiaApplication

  Note that accessing of slots and methods via Qt's \l{Meta-Object System} should only be 
  used in limited cases. This approach is very error prone because programming 
  error may only be discovered at runtime (and not at compile time). As soon as widgets
  are required that expose more sophisticated interfaces \l{Integration guide#server-widgets}{abstract server widgets}
  should be considered.

*/

/*!
  \typedef UIFactory::WidgetCreateFunction
  \internal
*/

/*!
  \macro UIFACTORY_REGISTER_WIDGET( ClassName )
  \relates UIFactory

  Registers the widget \a ClassName so that other parts of the server can utilize the widget.
  The advantage of this macro is the fact that the user of \a ClassName must not include 
  the class declaration for \a ClassName. 
*/

/*!
  Returns a pointer to a new instance of \a widgetClassName. If no widget with 
  that name has been registered this function returns 0. The returned widget is a child of 
  \a parent, with widget flags set to \a fl.

  Each call to this function will create a new instance of \a widgetClassName. It is the callers
  responsibility to manage the life cycle of the returned widget.
  */
QWidget* UIFactory::createWidget( const QByteArray &widgetClassName, 
        QWidget *parent, Qt::WFlags fl)
{
    UIFactoryPrivate* f = factoryPrivate();
    if ( !f ) return 0;

    if ( !f->widgets.contains(widgetClassName) )
        return 0;

    QMap<QByteArray, UIFactory::WidgetCreateFunction>::Iterator iter =
        f->widgets.find(widgetClassName);
    Q_ASSERT(iter != f->widgets.end());

    return (*iter)(parent, fl);
}

/*!
  Returns a pointer to a new instance of \a dialogClassName. If no dialog with 
  that name has been registered this function returns 0. The returned dialog is a child of 
  \a parent, with widget flags set to \a f.

  Each call to this function will create a new instance of \a dialogClassName. It is the callers
  responsibility to manage the life cycle of the returned dialog.
*/
QDialog* UIFactory::createDialog( const QByteArray &dialogClassName, 
        QWidget *parent, Qt::WFlags f)
{
    QWidget *w = UIFactory::createWidget( dialogClassName, parent, f );
    if ( !w ) 
        return 0;

    QDialog* d = qobject_cast<QDialog*>(w);
    if ( !d )
        w->deleteLater();
    return d;
}

/*!
  \internal
  This function registers a new widget representated by \a meta within the server.
  \a function is the function pointer that creates the widget.
  */
void UIFactory::install( const QMetaObject *meta, WidgetCreateFunction function )
{
    QByteArray name( meta->className() );
    if (name.isEmpty())
        return;

    if ( name == "QDialog" || name == "QWidget" ) {
        //metaobject for derived class missing
        qLog(Component) << "Trying to register a widget w/o Q_OBJECT macro";
    }
    qLog(Component) << "Registering UI component" << name;
   
    UIFactoryPrivate* factory = factoryPrivate();
    if ( !factory ) return;

    factory->widgets.insert(name, function);
}

/*!
  Returns true if \a widgetClassName is registered/available as a component that can be
  created via the UIFactory; otherwise false. 

  This function may be used to discover available widgets components at runtime and 
  implement fall-back strategies if a particular widget is not available. Note that the 
  list of widgets/components is created at static construction time. Therefore this function
  does not return reliable data before main() has been entered.
  */
bool UIFactory::isAvailable( const QByteArray &widgetClassName )
{
    UIFactoryPrivate *fp = factoryPrivate();
    if (!fp) return false;

    if ( fp->widgets.contains(widgetClassName) )
        return true;
    return false;
}
