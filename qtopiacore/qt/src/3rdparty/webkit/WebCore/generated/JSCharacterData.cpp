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

#include "JSCharacterData.h"

#include <wtf/GetPtr.h>

#include "CharacterData.h"
#include "ExceptionCode.h"
#include "PlatformString.h"

using namespace KJS;

namespace WebCore {

/* Hash table */

static const HashEntry JSCharacterDataTableEntries[] =
{
    { "data", JSCharacterData::DataAttrNum, DontDelete, 0, &JSCharacterDataTableEntries[3] },
    { "length", JSCharacterData::LengthAttrNum, DontDelete|ReadOnly, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { "constructor", JSCharacterData::ConstructorAttrNum, DontDelete|DontEnum|ReadOnly, 0, 0 }
};

static const HashTable JSCharacterDataTable = 
{
    2, 4, JSCharacterDataTableEntries, 3
};

/* Hash table for constructor */

static const HashEntry JSCharacterDataConstructorTableEntries[] =
{
    { 0, 0, 0, 0, 0 }
};

static const HashTable JSCharacterDataConstructorTable = 
{
    2, 1, JSCharacterDataConstructorTableEntries, 1
};

class JSCharacterDataConstructor : public DOMObject {
public:
    JSCharacterDataConstructor(ExecState* exec)
    {
        setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
        putDirect(exec->propertyNames().prototype, JSCharacterDataPrototype::self(exec), None);
    }
    virtual bool getOwnPropertySlot(ExecState*, const Identifier&, PropertySlot&);
    JSValue* getValueProperty(ExecState*, int token) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;

    virtual bool implementsHasInstance() const { return true; }
};

const ClassInfo JSCharacterDataConstructor::info = { "CharacterDataConstructor", 0, &JSCharacterDataConstructorTable, 0 };

bool JSCharacterDataConstructor::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSCharacterDataConstructor, DOMObject>(exec, &JSCharacterDataConstructorTable, this, propertyName, slot);
}

JSValue* JSCharacterDataConstructor::getValueProperty(ExecState*, int token) const
{
    // The token is the numeric value of its associated constant
    return jsNumber(token);
}

/* Hash table for prototype */

static const HashEntry JSCharacterDataPrototypeTableEntries[] =
{
    { "appendData", JSCharacterData::AppendDataFuncNum, DontDelete|Function, 1, &JSCharacterDataPrototypeTableEntries[5] },
    { "insertData", JSCharacterData::InsertDataFuncNum, DontDelete|Function, 2, 0 },
    { 0, 0, 0, 0, 0 },
    { "substringData", JSCharacterData::SubstringDataFuncNum, DontDelete|Function, 2, 0 },
    { "deleteData", JSCharacterData::DeleteDataFuncNum, DontDelete|Function, 2, 0 },
    { "replaceData", JSCharacterData::ReplaceDataFuncNum, DontDelete|Function, 3, 0 }
};

static const HashTable JSCharacterDataPrototypeTable = 
{
    2, 6, JSCharacterDataPrototypeTableEntries, 5
};

const ClassInfo JSCharacterDataPrototype::info = { "CharacterDataPrototype", 0, &JSCharacterDataPrototypeTable, 0 };

JSObject* JSCharacterDataPrototype::self(ExecState* exec)
{
    return KJS::cacheGlobalObject<JSCharacterDataPrototype>(exec, "[[JSCharacterData.prototype]]");
}

bool JSCharacterDataPrototype::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticFunctionSlot<JSCharacterDataPrototypeFunction, JSObject>(exec, &JSCharacterDataPrototypeTable, this, propertyName, slot);
}

const ClassInfo JSCharacterData::info = { "CharacterData", &JSEventTargetNode::info, &JSCharacterDataTable, 0 };

JSCharacterData::JSCharacterData(ExecState* exec, CharacterData* impl)
    : JSEventTargetNode(exec, impl)
{
    setPrototype(JSCharacterDataPrototype::self(exec));
}

bool JSCharacterData::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSCharacterData, JSEventTargetNode>(exec, &JSCharacterDataTable, this, propertyName, slot);
}

JSValue* JSCharacterData::getValueProperty(ExecState* exec, int token) const
{
    switch (token) {
    case DataAttrNum: {
        CharacterData* imp = static_cast<CharacterData*>(impl());

        return jsString(imp->data());
    }
    case LengthAttrNum: {
        CharacterData* imp = static_cast<CharacterData*>(impl());

        return jsNumber(imp->length());
    }
    case ConstructorAttrNum:
        return getConstructor(exec);
    }
    return 0;
}

void JSCharacterData::put(ExecState* exec, const Identifier& propertyName, JSValue* value, int attr)
{
    lookupPut<JSCharacterData, JSEventTargetNode>(exec, propertyName, value, attr, &JSCharacterDataTable, this);
}

void JSCharacterData::putValueProperty(ExecState* exec, int token, JSValue* value, int /*attr*/)
{
    switch (token) {
    case DataAttrNum: {
        CharacterData* imp = static_cast<CharacterData*>(impl());

        ExceptionCode ec = 0;
        imp->setData(valueToStringWithNullCheck(exec, value), ec);
        setDOMException(exec, ec);
        break;
    }
    }
}

JSValue* JSCharacterData::getConstructor(ExecState* exec)
{
    return KJS::cacheGlobalObject<JSCharacterDataConstructor>(exec, "[[CharacterData.constructor]]");
}
JSValue* JSCharacterDataPrototypeFunction::callAsFunction(ExecState* exec, JSObject* thisObj, const List& args)
{
    if (!thisObj->inherits(&JSCharacterData::info))
      return throwError(exec, TypeError);

    CharacterData* imp = static_cast<CharacterData*>(static_cast<JSCharacterData*>(thisObj)->impl());

    switch (id) {
    case JSCharacterData::SubstringDataFuncNum: {
        ExceptionCode ec = 0;
        bool offsetOk;
        int offset = args[0]->toInt32(exec, offsetOk);
        if (!offsetOk) {
            setDOMException(exec, TYPE_MISMATCH_ERR);
            return jsUndefined();
        }
        if (offset < 0) {
            setDOMException(exec, INDEX_SIZE_ERR);
            return jsUndefined();
        }
        bool lengthOk;
        int length = args[1]->toInt32(exec, lengthOk);
        if (!lengthOk) {
            setDOMException(exec, TYPE_MISMATCH_ERR);
            return jsUndefined();
        }
        if (length < 0) {
            setDOMException(exec, INDEX_SIZE_ERR);
            return jsUndefined();
        }


        KJS::JSValue* result = jsStringOrNull(imp->substringData(offset, length, ec));
        setDOMException(exec, ec);
        return result;
    }
    case JSCharacterData::AppendDataFuncNum: {
        ExceptionCode ec = 0;
        String data = args[0]->toString(exec);

        imp->appendData(data, ec);
        setDOMException(exec, ec);
        return jsUndefined();
    }
    case JSCharacterData::InsertDataFuncNum: {
        ExceptionCode ec = 0;
        bool offsetOk;
        int offset = args[0]->toInt32(exec, offsetOk);
        if (!offsetOk) {
            setDOMException(exec, TYPE_MISMATCH_ERR);
            return jsUndefined();
        }
        if (offset < 0) {
            setDOMException(exec, INDEX_SIZE_ERR);
            return jsUndefined();
        }
        String data = args[1]->toString(exec);

        imp->insertData(offset, data, ec);
        setDOMException(exec, ec);
        return jsUndefined();
    }
    case JSCharacterData::DeleteDataFuncNum: {
        ExceptionCode ec = 0;
        bool offsetOk;
        int offset = args[0]->toInt32(exec, offsetOk);
        if (!offsetOk) {
            setDOMException(exec, TYPE_MISMATCH_ERR);
            return jsUndefined();
        }
        if (offset < 0) {
            setDOMException(exec, INDEX_SIZE_ERR);
            return jsUndefined();
        }
        bool lengthOk;
        int length = args[1]->toInt32(exec, lengthOk);
        if (!lengthOk) {
            setDOMException(exec, TYPE_MISMATCH_ERR);
            return jsUndefined();
        }
        if (length < 0) {
            setDOMException(exec, INDEX_SIZE_ERR);
            return jsUndefined();
        }

        imp->deleteData(offset, length, ec);
        setDOMException(exec, ec);
        return jsUndefined();
    }
    case JSCharacterData::ReplaceDataFuncNum: {
        ExceptionCode ec = 0;
        bool offsetOk;
        int offset = args[0]->toInt32(exec, offsetOk);
        if (!offsetOk) {
            setDOMException(exec, TYPE_MISMATCH_ERR);
            return jsUndefined();
        }
        if (offset < 0) {
            setDOMException(exec, INDEX_SIZE_ERR);
            return jsUndefined();
        }
        bool lengthOk;
        int length = args[1]->toInt32(exec, lengthOk);
        if (!lengthOk) {
            setDOMException(exec, TYPE_MISMATCH_ERR);
            return jsUndefined();
        }
        if (length < 0) {
            setDOMException(exec, INDEX_SIZE_ERR);
            return jsUndefined();
        }
        String data = args[2]->toString(exec);

        imp->replaceData(offset, length, data, ec);
        setDOMException(exec, ec);
        return jsUndefined();
    }
    }
    return 0;
}

}