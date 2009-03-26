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

#ifndef PARSER_H
#define PARSER_H

#include <QList>
#include <QPair>
#include <QObject>
#include "tokens.h"
#include "preprocessor.h"

class Expression;
class EvaluatableValue;
typedef QList<EvaluatableValue> EvaluatableValues;

class ParserItem
{
public:
    ParserItem();
    ParserItem(const ParserItem &);
    ParserItem &operator=(const ParserItem &);

    void setInfo(const ParserItem &item);
    void setInfo(const PreprocessorToken &token);
    void setInfo(int line, int offset);
    int lineNumber() const;
    int lineOffset() const;

private:
    int _line;
    int _offset;
};

class EvaluatableValue : public ParserItem
{
public:
    EvaluatableValue();
    EvaluatableValue(const EvaluatableValue &);
    EvaluatableValue &operator=(const EvaluatableValue &);
    ~EvaluatableValue();

    enum Type { Characters, Variable, QtVariable, Environment, Function,
                TFunction };
    void setType(Type);
    Type type() const;

    QByteArray name() const;
    void setName(const QByteArray &);
    QList<Expression> arguments() const; // Only valid for Function type
    void setArguments(const QList<Expression> &);

    static const char *typeToString(Type);
    void dump(int) const;
private:
    Type m_type;
    QByteArray m_name;
    QList<Expression> *m_arguments;
};


class Expression : public ParserItem
{
public:
    Expression();
    Expression(const Expression &);
    Expression &operator=(const Expression &);

    void setEvaluatableValues(const QList<EvaluatableValues> &);
    void appendEvaluatableValues(const EvaluatableValues &);
    QList<EvaluatableValues> evaluatableValues() const;

    bool isEmpty() const;

    void dump(int) const;
private:
    QList<EvaluatableValues> m_values;
};

class Block : public ParserItem
{
public:
    enum Type { MultiBlock, Scope, Assignment, Function, Script };
    Block(Type);
    virtual ~Block();

    Type type() const;

    void dump();
    virtual void dump(int) = 0;

private:
    Type m_type;
};
typedef QList<Block *> Blocks;

class Script : public Block
{
public:
    Script();

    QByteArray script() const;
    void setScript(const QByteArray &);

    virtual void dump(int);
private:
    QByteArray m_script;
};

class Assignment : public Block
{
public:
    Assignment();

    enum Op { Assign, Add, Remove, SetAdd, Regexp };

    void setName(const QByteArray &);
    QByteArray name() const;
    void setOp(Op);
    Op op() const;
    void setExpression(const Expression &);
    Expression expression() const;

    virtual void dump(int);

private:
    QByteArray m_name;
    Op m_op;
    Expression m_expr;
};

class Function : public Block
{
public:
    Function();

    EvaluatableValue function() const;
    void setFunction(const EvaluatableValue &);

    virtual void dump(int);

private:
    EvaluatableValue m_function;
};

class MultiBlock : public Block
{
public:
    MultiBlock();

    void appendBlock(Block *);
    Blocks blocks() const;

    void setUsingName(const QByteArray &);
    QByteArray usingName() const;

    virtual void dump(int);

protected:
    MultiBlock(Block::Type);
    Blocks m_blocks;
    QByteArray m_usingName;
};

class Scope : public MultiBlock
{
public:
    Scope();

    enum ConditionType { And, Or };

    void setConditionType(ConditionType);
    ConditionType conditionType() const;
    void setConditions(const QList<QPair<EvaluatableValues, bool> > &);
    void appendCondition(const EvaluatableValues &, bool negate);
    QList<QPair<EvaluatableValues, bool> > conditions() const;

    Block *elseBlock();
    void setElseBlock(Block *);

protected:
    virtual void dump(int);

private:
    ConditionType m_type;
    QList<QPair<EvaluatableValues, bool> > m_conditions;

    Block *m_elseBlock;
};

class QMakeParser
{
public:
    QMakeParser();
    ~QMakeParser();

    static Block *parseFile(const QString &, PreprocessorToken * = 0, QString * = 0);

    bool parseExpression(const QList<PreprocessorToken> &);
    bool parse(const QList<PreprocessorToken> &);
    void dump();
    Block *parsedBlock() const;
    PreprocessorToken error() const {
        return m_error;
    }
    QString errorMessage() const {
        return m_errorMessage;
    }

private:
    enum Deliminators { None = 0x0000,
                        Space = 0x0001,
                        NewLine = 0x0002,
                        RParen = 0x0004,
                        Colon = 0x0010,
                        RBrace = 0x0020,
                        LBrace = 0x0040,
                        Op = 0x0080,
                        LParen = 0x0100,
                        Pipe = 0x0200,
                        Comma = 0x0400,
                        ScriptStart = 0x0800 };
    bool isDeliminator(Deliminators) const;

    Block *parseMultiBlock(bool inScope = false, bool inUsing = false);
    Block *parseBlock(bool inScope);
    Block *parseTestFunction();
    Block *parseAssignment(Deliminators delim = NewLine);
    Block *parseUsing();
    Block *parseSubScope();
    Block *parseSingleExpression();
    Block *parseElseBlock();
    Block *parseScript();

    bool parseSingleScope(EvaluatableValues &, bool &, bool);
    bool parseTestFunction(EvaluatableValue &);
    bool parseExpression(Expression &,
                         Deliminators = (Deliminators)(NewLine));
    bool parseEvaluatableValues(EvaluatableValues &,
                                Deliminators = (Deliminators)(Space | NewLine));
    bool parseEvaluatableValueV(EvaluatableValue &);

    bool parseName(QByteArray &);
    bool parseFunctionArguments(EvaluatableValue &);

    PreprocessorToken ptoken() const;
    Token token() const;
    Token next(int count = 1) const;
    Token token(int) const;
    QByteArray lexem() const;
    QByteArray lexem(int) const;
    void adv(int count = 1);
    bool atEnd() const;
    bool isOp() const;

    QList<PreprocessorToken> m_tokens;
    int currentToken;

    Block *m_parseBlock;
    PreprocessorToken m_error;
    QString m_errorMessage;
};

#endif
