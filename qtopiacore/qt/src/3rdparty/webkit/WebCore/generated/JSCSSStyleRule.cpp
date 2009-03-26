/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#include "JSCSSStyleRule.h"

#include <wtf/GetPtr.h>

#include "CSSMutableStyleDeclaration.h"
#include "CSSStyleDeclaration.h"
#include "CSSStyleRule.h"
#include "JSCSSStyleDeclaration.h"
#include "PlatformString.h"

using namespace KJS;

namespace WebCore {

/* Hash table */

static const HashEntry JSCSSStyleRuleTableEntries[] =
{
    { "selectorText", JSCSSStyleRule::SelectorTextAttrNum, DontDelete, 0, &JSCSSStyleRuleTableEntries[3] },
    { 0, 0, 0, 0, 0 },
    { "style", JSCSSStyleRule::StyleAttrNum, DontDelete|ReadOnly, 0, 0 },
    { "constructor", JSCSSStyleRule::ConstructorAttrNum, DontDelete|DontEnum|ReadOnly, 0, 0 }
};

static const HashTable JSCSSStyleRuleTable = 
{
    2, 4, JSCSSStyleRuleTableEntries, 3
};

/* Hash table for constructor */

static const HashEntry JSCSSStyleRuleConstructorTableEntries[] =
{
    { 0, 0, 0, 0, 0 }
};

static const HashTable JSCSSStyleRuleConstructorTable = 
{
    2, 1, JSCSSStyleRuleConstructorTableEntries, 1
};

class JSCSSStyleRuleConstructor : public DOMObject {
public:
    JSCSSStyleRuleConstructor(ExecState* exec)
    {
        setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
        putDirect(exec->propertyNames().prototype, JSCSSStyleRulePrototype::self(exec), None);
    }
    virtual bool getOwnPropertySlot(ExecState*, const Identifier&, PropertySlot&);
    JSValue* getValueProperty(ExecState*, int token) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;

    virtual bool implementsHasInstance() const { return true; }
};

const ClassInfo JSCSSStyleRuleConstructor::info = { "CSSStyleRuleConstructor", 0, &JSCSSStyleRuleConstructorTable, 0 };

bool JSCSSStyleRuleConstructor::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSCSSStyleRuleConstructor, DOMObject>(exec, &JSCSSStyleRuleConstructorTable, this, propertyName, slot);
}

JSValue* JSCSSStyleRuleConstructor::getValueProperty(ExecState*, int token) const
{
    // The token is the numeric value of its associated constant
    return jsNumber(token);
}

/* Hash table for prototype */

static const HashEntry JSCSSStyleRulePrototypeTableEntries[] =
{
    { 0, 0, 0, 0, 0 }
};

static const HashTable JSCSSStyleRulePrototypeTable = 
{
    2, 1, JSCSSStyleRulePrototypeTableEntries, 1
};

const ClassInfo JSCSSStyleRulePrototype::info = { "CSSStyleRulePrototype", 0, &JSCSSStyleRulePrototypeTable, 0 };

JSObject* JSCSSStyleRulePrototype::self(ExecState* exec)
{
    return KJS::cacheGlobalObject<JSCSSStyleRulePrototype>(exec, "[[JSCSSStyleRule.prototype]]");
}

const ClassInfo JSCSSStyleRule::info = { "CSSStyleRule", &JSCSSRule::info, &JSCSSStyleRuleTable, 0 };

JSCSSStyleRule::JSCSSStyleRule(ExecState* exec, CSSStyleRule* impl)
    : JSCSSRule(exec, impl)
{
    setPrototype(JSCSSStyleRulePrototype::self(exec));
}

bool JSCSSStyleRule::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSCSSStyleRule, JSCSSRule>(exec, &JSCSSStyleRuleTable, this, propertyName, slot);
}

JSValue* JSCSSStyleRule::getValueProperty(ExecState* exec, int token) const
{
    switch (token) {
    case SelectorTextAttrNum: {
        CSSStyleRule* imp = static_cast<CSSStyleRule*>(impl());

        return jsStringOrNull(imp->selectorText());
    }
    case StyleAttrNum: {
        CSSStyleRule* imp = static_cast<CSSStyleRule*>(impl());

        return toJS(exec, WTF::getPtr(imp->style()));
    }
    case ConstructorAttrNum:
        return getConstructor(exec);
    }
    return 0;
}

void JSCSSStyleRule::put(ExecState* exec, const Identifier& propertyName, JSValue* value, int attr)
{
    lookupPut<JSCSSStyleRule, JSCSSRule>(exec, propertyName, value, attr, &JSCSSStyleRuleTable, this);
}

void JSCSSStyleRule::putValueProperty(ExecState* exec, int token, JSValue* value, int /*attr*/)
{
    switch (token) {
    case SelectorTextAttrNum: {
        CSSStyleRule* imp = static_cast<CSSStyleRule*>(impl());

        ExceptionCode ec = 0;
        imp->setSelectorText(valueToStringWithNullCheck(exec, value), ec);
        setDOMException(exec, ec);
        break;
    }
    }
}

JSValue* JSCSSStyleRule::getConstructor(ExecState* exec)
{
    return KJS::cacheGlobalObject<JSCSSStyleRuleConstructor>(exec, "[[CSSStyleRule.constructor]]");
}

}