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

/*!
    \class QPrivateImplementationBase
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \brief The QPrivateImplementationBase provides a base class for implicitly
        shared implementation classes.
    \ingroup messaginglibrary
    \internal
    \sa QSharedData
*/

/*!
    \class QPrivateImplementationPointer
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \brief The QPrivateImplementationPointer is a smart pointer which manages
        implicit sharing of classes derived from QPrivateImplementationBase.
    \ingroup messaginglibrary
    \internal
    \sa QSharedDataPointer
*/

/*!
    \class QPrivatelyImplemented
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \brief The QPrivatelyImplemented class template allows a class to delegate
        its implementation to an incomplete template parameter class.
    \ingroup messaginglibrary
    \internal
*/

/*
    A simple demonstration of these classes...

    \code
    #include "qprivateimplementationdef.h"

    #include <QString>
    #include "qtopialog.h"

    // Class A provides the property 'name'
    class A_Impl;
    class A : public QPrivatelyImplemented<A_Impl>
    {
    public:
        // Typedef to name our implementation class
        typedef A_Impl ImplementationType;

        // Constructor for client use
        A(const QString& name = QString());

        QString name() const;
        void setName(const QString& name);

    protected:
        // Constructor for subclasses' use
        template<typename Subclass>
        A(Subclass* p, const QString& name = QString());
    };


    // Class B provides the interface of A, plus the property 'address'
    class B_Impl;
    class B : public A
    {
    public:
        // Typedef to name our implementation class
        typedef B_Impl ImplementationType;

        // Constructor for client use
        B(const QString& name = QString(), const QString& address = QString());

        QString address() const;
        void setAddress(const QString& address);
    };


    // Class A_Impl implements A's interface
    class A_Impl : public QPrivateImplementationBase
    {
        QString _name;

    public:
        A_Impl();

        QString name() const;
        void setName(const QString& name);

    protected:
        template<typename Subclass>
        A_Impl(Subclass* p);
    };

    // Create an A_Impl directly
    A_Impl::A_Impl()
        : QPrivateImplementationBase(this)
    {
    }

    // Create an A_Impl object with the space allocated by a subtype class
    template<typename Subclass>
    A_Impl::A_Impl(Subclass* p)
        : QPrivateImplementationBase(p)
    {
    }

    QString A_Impl::name() const
    {
        return _name;
    }

    void A_Impl::setName(const QString& name)
    {
        _name = name;
    }

    // Instantiate the implementation class for A
    template class QPrivatelyImplemented<A_Impl>;


    // Class B_Impl implements B's interface
    class B_Impl : public A_Impl
    {
        QString _address;

    public:
        B_Impl();

        QString address() const;
        void setAddress(const QString& address);
    };

    // Create a B_Impl directly
    B_Impl::B_Impl()
        : A_Impl(this)
    {
    }

    QString B_Impl::address() const
    {
        return _address;
    }

    void B_Impl::setAddress(const QString& address)
    {
        _address = address;
    }

    // Instantiate the implementation class for B
    template class QPrivatelyImplemented<B_Impl>;


    // Implement class A by delegation, now that the definition of A_Impl is visible
    A::A(const QString& name)
        : QPrivatelyImplemented<A_Impl>(new A_Impl)
    {
        impl(this)->setName(name);
    }

    template<typename Subclass>
    A::A(Subclass* p, const QString& name)
        : QPrivatelyImplemented<A_Impl>(p)
    {
        setName(name);
    }

    QString A::name() const
    {
        return impl(this)->name();
    }

    void A::setName(const QString& name)
    {
        impl(this)->setName(name);
    }


    // Implement class B by delegation, now that the definition of B_Impl is visible
    B::B(const QString& name, const QString& address)
        : A(new B_Impl)
    {
        setName(name);
        setAddress(address);
    }

    QString B::address() const
    {
        return impl(this)->address();
    }

    void B::setAddress(const QString& address)
    {
        impl(this)->setAddress(address);
    }


    QDebug& operator<< (QDebug& dbg, const A& a) { return dbg << a.name(); }
    QDebug& operator<< (QDebug& dbg, const B& b) { return dbg << b.name() + ':' + b.address(); }

    static QDebug& out()
    {
        // Provide your own...
    }

    static int doTest()
    {
        A a1(QString("Homer J. Simpson"));
        A a2(a1);
        out() << "doTest - a1:" << a1 << "a2:" << a2;

        a1.setName(QString("C. Mongomery Burns"));
        out() << "doTest - a1:" << a1 << "a2:" << a2;

        a2 = a1;
        out() << "doTest - a1:" << a1 << "a2:" << a2;

        B b1(QString("Homer J. Simpson"), QString("Evergreen Tce."));
        B b2(b1);
        out() << "doTest - b1:" << b1 << "b2:" << b2;

        b1.setName(QString("C. Montgomery Burns"));
        b1.setAddress(QString("Mammon St."));
        out() << "doTest - b1:" << b1 << "b2:" << b2;

        b2 = b1;
        out() << "doTest - b1:" << b1 << "b2:" << b2;

        b1.setName(QString("Bart Simpson"));
        a1 = b1;
        out() << "doTest - a1:" << a1 << "b1:" << b1;

        a1.setName(QString("Marge Simpson"));
        out() << "doTest - a1:" << a1 << "b1:" << b1;

        static_cast<A&>(b1) = a1;
        out() << "doTest - a1:" << a1 << "b1:" << b1;

        b1.setAddress(QString("Evergeen Tce."));
        out() << "doTest - a1:" << a1 << "b1:" << b1;

        return 0;
    }

    static const int testComplete = doTest();
    \endcode
*/

