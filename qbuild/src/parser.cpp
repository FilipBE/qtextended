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

#include "parser.h"
#include <QHash>
#include <QFile>
#include "qbuild.h"
#include <QMutex>
#include "qoutput.h"

ParserItem::ParserItem()
: _line(-1), _offset(-1)
{
}
ParserItem::ParserItem(const ParserItem &o)
: _line(o._line), _offset(o._offset)
{
}

ParserItem &ParserItem::operator=(const ParserItem &o)
{
    _line = o._line;
    _offset = o._offset;
    return *this;
}

void ParserItem::setInfo(const ParserItem &item)
{
    ParserItem::operator=(item);
}

void ParserItem::setInfo(const PreprocessorToken &token)
{
    _line = token.line;
    _offset = token.offset;
}

void ParserItem::setInfo(int line, int offset)
{
    _line = line;
    _offset = offset;
}

int ParserItem::lineNumber() const
{
    return _line;
}

int ParserItem::lineOffset() const
{
    return _offset;
}

EvaluatableValue::EvaluatableValue()
: m_type(Characters), m_arguments(0)
{
}

const char *EvaluatableValue::typeToString(Type type)
{
    switch(type) {
        case Characters:
            return "Characters";
        case Variable:
            return "Variable";
        case QtVariable:
            return "QtVariable";
        case Environment:
            return "Environment";
        case Function:
            return "Function";
        case TFunction:
            return "TFunction";
        default:
            return 0;
    }
}

void EvaluatableValue::dump(int depth) const
{
    QByteArray indent(depth * 4, ' ');

    QByteArray typeName = typeToString(type());

    qOutput().nospace() << indent << "EvaluatableValue: " << typeName << "'" << name() << "' " << lineNumber() << ":" << lineOffset();
    if ((Function == type() || TFunction == type()) && m_arguments) {
        for (int ii = 0; ii < m_arguments->count(); ++ii)
            m_arguments->at(ii).dump(depth + 1);
    }
}

EvaluatableValue::~EvaluatableValue()
{
    if (m_arguments)
        delete m_arguments;
}

EvaluatableValue::EvaluatableValue(const EvaluatableValue &other)
: ParserItem(other), m_type(other.m_type), m_name(other.m_name), m_arguments(0)
{
    if (other.m_arguments) {
        m_arguments = new QList<Expression>;
        *m_arguments = *other.m_arguments;
    }
}

EvaluatableValue &EvaluatableValue::operator=(const EvaluatableValue &other)
{
    ParserItem::operator=(other);

    m_type = other.m_type;
    m_name = other.m_name;

    if (other.m_arguments && !m_arguments) {
        m_arguments = new QList<Expression>;
    }
    if (!other.m_arguments && m_arguments) {
        delete m_arguments;
        m_arguments = 0;
    }

    if (other.m_arguments)
        *m_arguments = *other.m_arguments;

    return *this;
}

void EvaluatableValue::setType(Type t)
{
    m_type = t;
}

EvaluatableValue::Type EvaluatableValue::type() const
{
    return m_type;
}

QByteArray EvaluatableValue::name() const
{
    return m_name;
}

void EvaluatableValue::setName(const QByteArray &n)
{
    m_name = n;
}

QList<Expression> EvaluatableValue::arguments() const
{
    if (!m_arguments || m_arguments->isEmpty())
        return QList<Expression>();

    return *m_arguments;
}

void EvaluatableValue::setArguments(const QList<Expression> &arguments)
{
    if (!m_arguments)
        m_arguments = new QList<Expression>(arguments);
    else
        *m_arguments = arguments;
}

Expression::Expression()
{
}

bool Expression::isEmpty() const
{
    return m_values.isEmpty();
}

void Expression::dump(int depth) const
{
    QByteArray indent(depth * 4, ' ');

    qOutput().nospace() << indent << "[";
    for (int ii = 0; ii < m_values.count(); ++ii) {
        qOutput().nospace() << indent << "    (";

        for (int jj = 0; jj < m_values.at(ii).count(); ++jj) {
            m_values.at(ii).at(jj).dump(depth + 2);
        }

        qOutput().nospace() << indent << "    )";
    }
    qOutput().nospace() << indent << "] " << lineNumber() << ":" << lineOffset();
}

Expression::Expression(const Expression &other)
: ParserItem(other), m_values(other.m_values)
{
}

Expression &Expression::operator=(const Expression &other)
{
    ParserItem::operator=(other);

    m_values = other.m_values;
    return *this;
}

void Expression::setEvaluatableValues(const QList<EvaluatableValues> &v)
{
    m_values = v;
}

void Expression::appendEvaluatableValues(const EvaluatableValues &v)
{
    m_values.append(v);
}

QList<EvaluatableValues> Expression::evaluatableValues() const
{
    return m_values;
}

Block::Block(Type t)
: m_type(t)
{
}

Block::~Block()
{
}

Block::Type Block::type() const
{
    return m_type;
}

void Block::dump()
{
    dump(0);
}

Script::Script()
: Block(Block::Script)
{
}

QByteArray Script::script() const
{
    return m_script;
}

void Script::setScript(const QByteArray &s)
{
    m_script = s;
}

void Script::dump(int depth)
{
    QByteArray indent(depth * 4, ' ');

    qOutput().nospace() << indent << "Script:";

    QList<QByteArray> lines = m_script.split('\n');
    foreach(QByteArray line, lines)
        qOutput().nospace() << indent << "    " << line;
}

Assignment::Assignment()
: Block(Block::Assignment), m_op(Assign)
{
}

void Assignment::setName(const QByteArray &n)
{
    m_name = n;
}

QByteArray Assignment::name() const
{
    return m_name;
}

void Assignment::setOp(Op o)
{
    m_op = o;
}

Assignment::Op Assignment::op() const
{
    return m_op;
}

void Assignment::setExpression(const Expression &e)
{
    m_expr = e;
}

Expression Assignment::expression() const
{
    return m_expr;
}

void Assignment::dump(int depth)
{
    QByteArray indent(depth * 4, ' ');

    QByteArray opName;
    switch(m_op) {
        case Assign:
            opName = "Assign";
            break;
        case Add:
            opName = "Add";
            break;
        case Remove:
            opName = "Remove";
            break;
        case SetAdd:
            opName = "SetAdd";
            break;
        case Regexp:
            opName = "Regexp";
            break;
    }

    qOutput().nospace() << indent << "Assignment: '" << m_name << "' '" << opName << "' " << lineNumber() << ":" << lineOffset();
    m_expr.dump(depth + 1);
}

Function::Function()
: Block(Block::Function)
{
}

EvaluatableValue Function::function() const
{
    return m_function;
}

void Function::setFunction(const EvaluatableValue &f)
{
    m_function = f;
}

void Function::dump(int depth)
{
    QByteArray indent(depth * 4, ' ');
    qOutput().nospace() << indent << "Function: " << lineNumber() << ":" << lineOffset();
    m_function.dump(depth + 1);
}

MultiBlock::MultiBlock(Block::Type type)
: Block(type)
{
}

MultiBlock::MultiBlock()
: Block(Block::MultiBlock)
{
}

void MultiBlock::appendBlock(Block *b)
{
    m_blocks.append(b);
}


Blocks MultiBlock::blocks() const
{
    return m_blocks;
}

void MultiBlock::setUsingName(const QByteArray &n)
{
    m_usingName = n;
}

QByteArray MultiBlock::usingName() const
{
    return m_usingName;
}

void MultiBlock::dump(int depth)
{
    QByteArray indent(depth * 4, ' ');
    if (m_usingName.isEmpty()) {
        qOutput().nospace() << indent << "{";
    } else {
        qOutput().nospace() << indent << "Using " << m_usingName << " {";
    }

    for (int ii = 0; ii < m_blocks.count(); ++ii)
        m_blocks.at(ii)->dump(depth + 1);
    qOutput().nospace() << indent << "} " << lineNumber() << ":" << lineOffset();
}

Scope::Scope()
: MultiBlock(Block::Scope), m_type(And), m_elseBlock(0)
{
}

void Scope::setConditionType(ConditionType t)
{
    m_type = t;
}

Scope::ConditionType Scope::conditionType() const
{
    return m_type;
}

void Scope::setConditions(const QList<QPair<EvaluatableValues, bool> > &c)
{
    m_conditions = c;
}

void Scope::appendCondition(const EvaluatableValues &c, bool negate)
{
    m_conditions.append(qMakePair(c, negate));
}

QList<QPair<EvaluatableValues, bool> > Scope::conditions() const
{
    return m_conditions;
}


Block *Scope::elseBlock()
{
    return m_elseBlock;
}

void Scope::setElseBlock(Block *e)
{
    m_elseBlock = e;
}

void Scope::dump(int depth)
{
    QByteArray indent(depth * 4, ' ');
    qOutput().nospace() << indent << "Scope (" << ((m_type==And)?"And":"Or") << "): " << lineNumber() << ":" << lineOffset();
    for (int ii = 0; ii < m_conditions.count(); ++ii) {
        int newDepth = depth + 1;
        if (m_conditions.at(ii).second) {
            qOutput().nospace() << indent << "    NOT";
            newDepth++;
        }
        for (int jj = 0; jj < m_conditions.at(ii).first.count(); ++jj) {
            m_conditions.at(ii).first.at(jj).dump(newDepth);
        }
    }
    if (m_blocks.count()) {
        qOutput().nospace() << indent << "    {";
        for (int ii = 0; ii < m_blocks.count(); ++ii) {
            m_blocks.at(ii)->dump(depth + 2);
        }
        qOutput().nospace() << indent << "    }";
    }
}

QMakeParser::QMakeParser()
: m_parseBlock(0)
{
}

QMakeParser::~QMakeParser()
{
    delete m_parseBlock;
    m_parseBlock = 0;
}

struct ParseCache : public QHash<QString, QMakeParser *>
{
    QMutex _lock;
    void lock() { LOCK(Parser); _lock.lock(); }
    void unlock() { _lock.unlock(); }
};

Q_GLOBAL_STATIC(ParseCache, parseCache)

/*
   Returns the root Block of the project \a fileName, or 0 if the parse of this
   file failed.  Parse results are cached to improve performance.

   Ownership of the returned value remains with the QMakeParser.
 */
Block *QMakeParser::parseFile(const QString &fileName, PreprocessorToken *err, QString *errMsg)
{
    QBuild::beginPerfTiming("QMakeParser::ParseFile");
    ParseCache *cache = parseCache();
    cache->lock();
    ParseCache::ConstIterator iter = cache->find(fileName);
    if (iter == cache->end()) {

        QMakeParser *parser = 0;
        QByteArray data;
        const char *dataStr;
        QList<LexerToken> tokens;
        QList<PreprocessorToken> ptokens;

        QFile file(fileName);

        if (!file.open(QFile::ReadOnly))
            goto error;

        data = file.readAll();

        dataStr = data.constData();
        tokens = tokenize(dataStr, fileName.toLatin1().constData());
        if (tokens.isEmpty())
            goto error;

        ptokens = preprocess(dataStr, tokens);
        if (ptokens.isEmpty())
            goto error;

        parser = new QMakeParser;
        if (!parser->parse(ptokens)) {
            if (err)
                *err = parser->error();
            if (errMsg)
                *errMsg = parser->errorMessage();

            delete parser;
            parser = 0;
            goto error;
        }

error:

        iter = cache->insert(fileName, parser);
    }

    Block *rv = (*iter)?(*iter)->parsedBlock():0;

    cache->unlock();

    QBuild::endPerfTiming();
    return rv;
}

#define SKIP_SPACE_NEWLINE while (SINGLE_SPACE == token() || NEWLINE == token()) currentToken++;
#define SKIP_SPACE while (SINGLE_SPACE == token()) currentToken++;
#define START_PARSE int oldToken = currentToken;
#define FAIL_PARSE { m_error = ptoken(); currentToken = oldToken; return false; }
#define FAIL_PARSE_VAL(val) { m_error = ptoken(); currentToken = oldToken; return val; }
#define FAIL_PARSE_VAL_MSG(val,msg) { m_error = ptoken(); m_errorMessage = (msg); currentToken = oldToken; return val; }
#define RESET_PARSE currentToken = oldToken;
#define RESET_POINT int oldToken = currentToken;

bool QMakeParser::parseExpression(const QList<PreprocessorToken> &tokens)
{
    m_tokens = tokens;
    currentToken = 0;

    if (m_parseBlock) {
        delete m_parseBlock;
        m_parseBlock = 0;
    }

    Block *block = parseSingleExpression();
    if (block)
        m_parseBlock = block;

    return block != 0;
}

bool QMakeParser::parse(const QList<PreprocessorToken> &tokens)
{
    m_tokens = tokens;
    currentToken = 0;

    if (m_parseBlock) {
        delete m_parseBlock;
        m_parseBlock = 0;
    }

    Block *block = parseMultiBlock();
    if (block)
        m_parseBlock = block;

    return block != 0;
}

Block *QMakeParser::parsedBlock() const
{
    return m_parseBlock;
}

/*
   TEST_FUNCTION = <NAME>([<EXPRESSION>[,<EXPRESSION>...]])
   */
Block *QMakeParser::parseTestFunction()
{
    START_PARSE;
    SKIP_SPACE;

    EvaluatableValue func;
    if (!parseTestFunction(func))
        FAIL_PARSE_VAL(0);

    Function *function = new Function();
    function->setFunction(func);
    function->setInfo(func);

    return function;
}

/*
    SCRIPT := <script>...
              ...
              <newline><whitespace></script>
 */
Block *QMakeParser::parseScript()
{
    START_PARSE;
    SKIP_SPACE;

    if (token() != SCRIPT_START)
        FAIL_PARSE;
    PreprocessorToken itoken = ptoken();

    int scriptStart = currentToken;

    adv();

    bool newLine = false;
    bool scriptEndSeen = false;
    while (!scriptEndSeen && !atEnd()) {
        if (token() == NEWLINE) {
            newLine = true;
        } else if (token() == WHITESPACE || token() == SINGLE_SPACE) {
        } else if (token() == SCRIPT_END && newLine) {
            scriptEndSeen = true;
            break;
        } else {
            newLine = false;
        }

        adv();
    }

    if (!scriptEndSeen)
        FAIL_PARSE;

    int scriptEnd = currentToken;
    adv();

    SKIP_SPACE;
    if (token() != NEWLINE && !atEnd())
        FAIL_PARSE;

    if (!atEnd())
        adv();

    const char *start = m_tokens.at(scriptStart).cStr +
                        m_tokens.at(scriptStart).cStrLen;
    const char *end = m_tokens.at(scriptEnd).cStr;
    QByteArray text(start, end - start);

    Script *script = new Script();
    script->setScript(text);
    script->setInfo(itoken);

    return script;
}

/*
    ASSIGNMENT := <NAME> <OP> <EXPRESSION><delim>

    <delim> is NOT consumed
 */
Block *QMakeParser::parseAssignment(Deliminators delim)
{
    START_PARSE;
    SKIP_SPACE;

    PreprocessorToken itoken = ptoken();
    QByteArray name;
    if (!parseName(name))
        FAIL_PARSE_VAL(0);

    SKIP_SPACE;

    Assignment::Op op = Assignment::Assign;
    switch(token()) {
        case EQUAL:
            op = Assignment::Assign;
            break;
        case PLUS_EQ:
            op = Assignment::Add;
            break;
        case MINUS_EQ:
            op = Assignment::Remove;
            break;
        case STAR_EQ:
            op = Assignment::SetAdd;
            break;
        case TILDE_EQ:
            op = Assignment::Regexp;
            break;
        default:
            FAIL_PARSE_VAL(0); // Not assignment
            break;
    }

    adv();
    SKIP_SPACE;

    Expression expr;
    if (!parseExpression(expr, delim))
        FAIL_PARSE_VAL(0);

    Assignment *assignment = new Assignment();
    assignment->setName(name);
    assignment->setOp(op);
    assignment->setExpression(expr);
    assignment->setInfo(itoken);

    return assignment;
}

/*
    USINGBLOCK := <NAME> [ <BLOCK> ]
 */
Block *QMakeParser::parseUsing()
{
    START_PARSE;
    SKIP_SPACE;

    PreprocessorToken itoken = ptoken();
    QByteArray name;
    if (!parseName(name))
        FAIL_PARSE_VAL(0);

    SKIP_SPACE;

    if (token() != LBRACKET)
        FAIL_PARSE_VAL(0);
    adv();

    SKIP_SPACE_NEWLINE;

    MultiBlock *subBlock =
        static_cast<MultiBlock *>(parseMultiBlock(false, true));

    if (!subBlock)
        FAIL_PARSE_VAL(0);

    SKIP_SPACE;
    if (token() != RBRACKET) {
        delete subBlock;
        FAIL_PARSE_VAL(0);
    }
    adv();

    subBlock->setUsingName(name);

    return subBlock;
}

/*
    MULTIBLOCK := <BLOCK>*(<EOF>|<RBRACE if inScope>|<RBRACKET if in using>)
*/
Block *QMakeParser::parseMultiBlock(bool inScope, bool inUsing)
{
    START_PARSE;
    SKIP_SPACE;

    MultiBlock *multiBlock = new MultiBlock();
    multiBlock->setInfo(ptoken());

    while (!atEnd() &&
          (!inScope || token() != RBRACE) &&
          (!inUsing || token() != RBRACKET)) {

        Block *block = parseBlock(inScope);
        if (!block) {
            delete multiBlock;
            FAIL_PARSE_VAL(0);
        }
        multiBlock->appendBlock(block);

    }

    return multiBlock;
}

/*
    SINGLEEXPRESSION := <EXPRESSION><NEWLINE><END>
 */
Block *QMakeParser::parseSingleExpression()
{
    START_PARSE;
    SKIP_SPACE;

    Expression expression;
    if (!parseExpression(expression, NewLine))
        FAIL_PARSE_VAL(0);

    if (token() != NEWLINE)
        FAIL_PARSE_VAL(0);

    adv();

    if (!atEnd())
        FAIL_PARSE_VAL(0);

    Assignment *block = new Assignment;
    block->setOp(Assignment::Assign);
    block->setExpression(expression);
    block->setInfo(expression);

    return block;
}

/*
    SUBSCOPE := { <MULTIBLOCK> }
*/
Block *QMakeParser::parseSubScope()
{
    START_PARSE;
    SKIP_SPACE;
    if (token() != LBRACE)
        FAIL_PARSE_VAL(0);
    PreprocessorToken itoken = ptoken();
    adv();
    SKIP_SPACE_NEWLINE;

    Block *subBlock = parseMultiBlock(true);
    if (!subBlock)
        FAIL_PARSE_VAL(0);
    SKIP_SPACE_NEWLINE;
    if (token() != RBRACE)
        FAIL_PARSE_VAL(0);
    adv();
    subBlock->setInfo(itoken);

    SKIP_SPACE_NEWLINE;

    return subBlock;
}


/*
   ELSEBLOCK = else <SUBSCOPE> |
               else(:|)<BLOCK>
*/
Block *QMakeParser::parseElseBlock()
{
    START_PARSE;
    SKIP_SPACE;

    if (token() != ELSE)
        FAIL_PARSE_VAL(0);
    adv();
    SKIP_SPACE;

    Block *subscope = parseSubScope();
    if (subscope)
        return subscope;

    if (token() != COLON && token() != PIPE)
        FAIL_PARSE_VAL(0);
    adv();
    SKIP_SPACE;

    Block *subblock = parseBlock(false);
    if (subblock)
        return subblock;

    FAIL_PARSE_VAL(0);
}


/*
    SCOPE := [!](<TFUNCTION>|<EVALUATABLE_VALUE not containing OP or LBRACE>)[(:||)<SCOPE>]

    BLOCK := [<SCOPE>(:||)]<NAME> <OP> <EXPRESSION><NEWLINE> |
             [<SCOPE>(:||)]<NAME> "[" <BLOCK> "]" |
             [<SCOPE>(:||)]<TFUNCTION><NEWLINE> |
             [<SCOPE>] <SUBSCOPE>[<NEWLINE>] |
             [<SCOPE>(:||)]<script>...</script> |
             [<SCOPE>] <SUBSCOPE> else(:||)<BLOCK>
 */
Block *QMakeParser::parseBlock(bool inScope)
{
    START_PARSE;
    SKIP_SPACE;

    PreprocessorToken itoken = ptoken();

    QList<QPair<EvaluatableValues, bool> > scopeConditions;
    enum ScopeOperator { Undecided, And, Or, OrFinalAnd };
    ScopeOperator scopeOperator = Undecided;
    // Extract all scoping
    {
        EvaluatableValues scopeCondition;
        bool negate = false;

        while (parseSingleScope(scopeCondition, negate, inScope)) {
            SKIP_SPACE;
            if (token() == COLON || token() == PIPE) {
                ScopeOperator cOp = And;
                if (token() == PIPE) cOp = Or;

                if (scopeOperator == Undecided || scopeOperator == cOp) {
                    scopeOperator = cOp;
                } else if (scopeOperator == Or && cOp == And) {
                    // A colon on the end of a chain is allowed (eg. foo|bar:FOO=baz)
                    scopeOperator = OrFinalAnd;
                } else {
                    FAIL_PARSE_VAL(0);
                }

                // Looks good, append to list
                scopeConditions.append(qMakePair(scopeCondition, negate));
                adv();
                SKIP_SPACE;

            } else if (token() == NEWLINE ||
                      (inScope && token() == RBRACE))
            {
                // The expression must be a TFunction
                if (scopeCondition.count() != 1 ||
                   scopeCondition.first().type() != EvaluatableValue::TFunction)
                    FAIL_PARSE_VAL(0);

                scopeConditions.append(qMakePair(scopeCondition, negate));

                break;

            } else if (token() == LBRACE) {
                // Must be the start of a sub scope
                scopeConditions.append(qMakePair(scopeCondition, negate));
                break;

            } else {
                // Invalid line
                FAIL_PARSE_VAL(0);
                break;
            }

            scopeCondition.clear();
            negate = false;
        }

    }

    Block *block = 0;

    if (token() == LBRACE) {
        // Sub scope
        block = parseSubScope();
        if (!block)
            FAIL_PARSE_VAL(0);
    } else if (token() == SCRIPT_START) {
        // Script
        block = parseScript();
        if (!block)
            FAIL_PARSE_VAL(0);
    } else if (inScope && token() == RBRACE) {
        // Nothing todo
    } else {
        // Must be an assignment or a using block
        block =
            parseAssignment(inScope?((Deliminators)(NewLine | RBrace)):NewLine);
        if (!block) {
            block = parseUsing();
            if (!block && !scopeConditions.isEmpty()) {
                // The scope code above incorrectly parses lines that are formatted as <conditions>:function()
                // To recover from this we take the last scope condition (which is a TFunction) and turn it
                // into a block to be used as the scope's body.
                QPair<EvaluatableValues, bool> p = scopeConditions.takeLast();
                // How would you get more than 1 evaluatable value in a last condition?
                if (p.first.count() != 1)
                    FAIL_PARSE_VAL(0);
                EvaluatableValue ev = p.first.takeFirst();
                // If the last scope isn't a TFunction then we've found a syntax error.
                if (ev.type() != EvaluatableValue::TFunction)
                    FAIL_PARSE_VAL(0);
                Function *func = new Function();
                func->setFunction(ev);
                block = func;
            }
            if (!block)
                FAIL_PARSE_VAL(0);
        }
        SKIP_SPACE;
        if (token() == NEWLINE)
            adv();
        SKIP_SPACE;
    }

    Q_ASSERT(!scopeConditions.isEmpty() || block);

    Scope *scope = 0;
    if (!scopeConditions.isEmpty()) {
        scope = new Scope;
        scope->setInfo(itoken);
        switch(scopeOperator) {
            case Undecided:
            case And:
                scope->setConditionType(Scope::And);
                break;
            case OrFinalAnd:
            case Or:
                scope->setConditionType(Scope::Or);
                break;
        }

        scope->setConditions(scopeConditions);
    }

    if (scope) {
        if (block)
            scope->appendBlock(block);

        Block *elseBlock = parseElseBlock();
        if (elseBlock)
            scope->setElseBlock(elseBlock);

        return scope;
    } else {
        return block;
    }
}

/*
    SCOPE := [!]<TEST_FUNCTION><delim> |
             [!]<EVALUATABLE_VALUES not containing OP or SCRIPT><delim>

    <delim> NOT consumed
*/
bool QMakeParser::parseSingleScope(EvaluatableValues &values, bool &negate, bool inScope)
{
    Deliminators delim = (Deliminators)(NewLine | Colon | Pipe | LBrace | Space);
    if (inScope)
        delim = (Deliminators)(delim | RBrace);

    START_PARSE;
    SKIP_SPACE;

    negate = false;
    if (token() == NOT) {
        negate = true;
        adv();
        SKIP_SPACE;
    }

    {
        RESET_POINT;
        // Attempt to parse a test function
        EvaluatableValue testFunction;
        if (parseTestFunction(testFunction)) {
            SKIP_SPACE;
            if (isDeliminator(delim)) {
                values.append(testFunction);
                return true;
            }
        }

        RESET_PARSE;
    }

    // Attempt to parse an evaluatable value
    int valueCount = values.count();
    // Don't succeed if we successfully parse no values!
    if (parseEvaluatableValues(values, (Deliminators)(delim | Op | ScriptStart)) && valueCount < values.count()) {
        SKIP_SPACE;
        if (isDeliminator(delim)) {
            return true;
        }
    }

    FAIL_PARSE;
}

/*
    TEST_FUNCTION := <NAME>([<EXPRESSION>])
*/
bool QMakeParser::parseTestFunction(EvaluatableValue &value)
{
    START_PARSE;
    SKIP_SPACE;

    PreprocessorToken itoken = ptoken();
    QByteArray name;
    if (!parseName(name))
        FAIL_PARSE;

    if (!parseFunctionArguments(value))
        FAIL_PARSE;

    value.setType(EvaluatableValue::TFunction);
    value.setName(name);
    value.setInfo(itoken);

    return true;
}

/*!
    EVALUATABLE_VALUES := <QUOTED_CHARACTERS>[<EVALUATABLE_VALUES>] |
                          <UNQUOTED_CHARACTERS_NO_SPACE>[<EVALUATABLE_VALUES] |
                          <VAR_EVALUATABLEVALUE>[<EVALUATABLE_VALUES>]
*/
bool QMakeParser::parseEvaluatableValues(EvaluatableValues &values, Deliminators delim)
{
    START_PARSE;
    SKIP_SPACE;

    bool isQuoted = false;

    QByteArray currentText;
    PreprocessorToken currentTextToken;

    while (!atEnd() && (isQuoted || !isDeliminator(delim))) {

        switch(token()) {
            case STRING_START:
                Q_ASSERT(!isQuoted);
                isQuoted = true;
                adv();
                break;
            case STRING_END:
                Q_ASSERT(isQuoted);
                isQuoted = false;
                adv();
                break;
            case DOLLAR_DOLLAR:
                // Attempt variable evaluation
                {
                    EvaluatableValue value;
                    if (parseEvaluatableValueV(value)) {
                        if (!currentText.isEmpty()) {
                            EvaluatableValue textValue;
                            textValue.setType(EvaluatableValue::Characters);
                            textValue.setName(currentText);
                            textValue.setInfo(currentTextToken);
                            values.append(textValue);
                            currentText.clear();
                            currentTextToken = PreprocessorToken();
                        }
                        values.append(value);
                        break;
                    }
                }
                // Fall through

            default:
                if (currentTextToken.line == -1)
                    currentTextToken = ptoken();
                currentText.append(lexem());
                adv();
                break;
        }
    }

    if (!currentText.isEmpty()) {
        EvaluatableValue textValue;
        textValue.setType(EvaluatableValue::Characters);
        textValue.setName(currentText);
        textValue.setInfo(currentTextToken);
        values.append(textValue);
        currentText.clear();
        currentTextToken = PreprocessorToken();
    }

    if (isQuoted)
        FAIL_PARSE;

    return true;
}

/*
    VAR_EVALUATABLEVALUE := $$<NAME> | $${<NAME>} |
                            $$(<NAME>) | $$[<NAME>] |
                            $$<NAME>([<EXPRESSION>]) |
                            $${<NAME>([<EXPRESSION>])}
 */
bool QMakeParser::parseEvaluatableValueV(EvaluatableValue &value)
{
    START_PARSE;
    SKIP_SPACE;

    value.setInfo(ptoken());
    if (token(currentToken) == DOLLAR_DOLLAR) {

        adv();

        if (token() == LPAREN || token() == LBRACKET) {
            // Can't be a function

            Token expected = NOTOKEN;
            EvaluatableValue::Type eType = EvaluatableValue::Characters;
            if (token() == LPAREN) {
                expected = RPAREN;
                eType = EvaluatableValue::Environment;
            } else {
                expected = RBRACKET;
                eType = EvaluatableValue::QtVariable;
            }

            adv();
            SKIP_SPACE;

            QByteArray name;
            if (!parseName(name))
                FAIL_PARSE;

            SKIP_SPACE;
            if (token() != expected)
                FAIL_PARSE;

            adv();

            // Success!
            value.setType(eType);
            value.setName(name);
            return true;

        } else if (token() == LBRACE || token() == NAME) {
            // Could be a function
            bool expectRBrace = false;
            if (token() == LBRACE) {
                expectRBrace = true;
                adv();
                SKIP_SPACE;
            }

            QByteArray name;
            if (!parseName(name))
                FAIL_PARSE;

            value.setType(EvaluatableValue::Variable);
            value.setName(name);

            if (token() == LPAREN) {
                // It's a function!
                value.setType(EvaluatableValue::Function);
                if (!parseFunctionArguments(value))
                    FAIL_PARSE;
            }

            SKIP_SPACE;

            if (expectRBrace) {
                if (token() == RBRACE) {
                    adv();
                    return true;
                } else {
                    FAIL_PARSE;
                }
            } else {
                if (next(-1) == SINGLE_SPACE)
                    currentToken--;

                return true;
            }
        }

    }

    FAIL_PARSE;
}

/*
    FUNCTIONARGUMENTS := (<EXPRESSION>[,<EXPRESSION>...])
*/
bool QMakeParser::parseFunctionArguments(EvaluatableValue &value)
{
    START_PARSE;
    SKIP_SPACE;

    if (token() != LPAREN)
        FAIL_PARSE;
    adv();
    SKIP_SPACE;
    if (token() == RPAREN) {
        // Done!
        adv();
        return true;
    } else {
        bool moreExpressions = true;
        QList<Expression> expressions;
        while (moreExpressions) {
            Expression expression;
            if (!parseExpression(expression, (Deliminators)(NewLine | RParen | Comma)))
                FAIL_PARSE;
            SKIP_SPACE;
            if (token() != COMMA) {
                moreExpressions = false;
                if (!(expressions.isEmpty() && expression.isEmpty()))
                    expressions << expression;
            } else {
                expressions << expression;
                adv();
                SKIP_SPACE;
            }
        }

        value.setArguments(expressions);
        if (token() == RPAREN) {
            adv();
            return true;
        } else {
            FAIL_PARSE;
        }
    }
}

bool QMakeParser::isDeliminator(Deliminators delim) const
{
    return  ((delim & Space) && token() == SINGLE_SPACE) ||
            ((delim & NewLine) && token() == NEWLINE) ||
            ((delim & RParen) && token() == RPAREN) ||
            ((delim & LParen) && token() == LPAREN) ||
            ((delim & Colon) && token() == COLON) ||
            ((delim & RBrace) && token() == RBRACE) ||
            ((delim & LBrace) && token() == LBRACE) ||
            ((delim & Pipe) && token() == PIPE) ||
            ((delim & Comma) && token() == COMMA) ||
            ((delim & ScriptStart) && token() == SCRIPT_START) ||
            ((delim & Op) && token() >= TOKEN_OP_BEGIN && token() <= TOKEN_OP_END);
}

/*
    EXPRESSION := <EVALUATABLE_VALUES>[ <EVALUATABLE_VALUES>](<delim>)

    Note: <delim> is NOT consumed
*/
bool QMakeParser::parseExpression(Expression &expression, Deliminators delim)
{
    START_PARSE;
    SKIP_SPACE;

    expression.setInfo(ptoken());

    // Can't go off end as the preprocess ensures that there is a NEWLINE at
    // the end

    // If RParen is a delim, we need to respect LParen RParen wrappings and
    // *not* use matched RParen's as a delim
    Deliminators extraDelim = None;
    if (delim & RParen)
        extraDelim = LParen;
    int lparens = 0;

    while (!isDeliminator(delim)) {
        EvaluatableValues values;

        while (true) {
            if (!parseEvaluatableValues(values, (Deliminators)(delim | extraDelim | Space)))
                FAIL_PARSE;
            if (!(delim & LParen) && token() == LPAREN) {
                EvaluatableValue value;
                value.setType(EvaluatableValue::Characters);
                value.setName(lexem());
                values.append(value);
                adv();
                lparens++;
                continue;
            } else if ((delim & RParen) && token() == RPAREN && lparens) {
                EvaluatableValue value;
                value.setType(EvaluatableValue::Characters);
                value.setName(lexem());
                values.append(value);
                adv();
                lparens--;
            } else if ((delim & Comma) && token() == COMMA && lparens) {
                // This comma belongs to the parenthesized value - ie. foo(foo(foo,bar))
                EvaluatableValue value;
                value.setType(EvaluatableValue::Characters);
                value.setName(lexem());
                values.append(value);
                adv();
            } else {
                break;
            }
        }

        if (!values.isEmpty())
            expression.appendEvaluatableValues(values);
        SKIP_SPACE;
    }

    return true;
}

/*!
    NAME := <SIMPLE_NAME>[.<NAME>]<Not . or <SIMPLE_NAME>>
 */
bool QMakeParser::parseName(QByteArray &name)
{
    START_PARSE;
    SKIP_SPACE;

    if (token() != NAME)
        FAIL_PARSE;

    bool seenDot = false;
    while (true) {
        name.append(lexem());
        adv();
        switch(token()) {
            case DOT:
                seenDot = true;
                if (next() != NAME && next() != DIGIT)
                    FAIL_PARSE;
                name.append(".");
                adv();
                break;
            case DIGIT:
            case NAME:
                if (!seenDot)
                    FAIL_PARSE;
                break;
            default:
                return true;
        }
    }
}


Token QMakeParser::next(int count) const
{
    return token(currentToken + count);
}

PreprocessorToken QMakeParser::ptoken() const
{
    if (currentToken >= m_tokens.count())
        return PreprocessorToken();
    else
        return m_tokens.at(currentToken);
}

Token QMakeParser::token() const
{
    return token(currentToken);
}

Token QMakeParser::token(int v) const
{
    if (v >= m_tokens.count())
        return NOTOKEN;
    return m_tokens.at(v).token;
}

QByteArray QMakeParser::lexem() const
{
    return lexem(currentToken);
}

QByteArray QMakeParser::lexem(int v) const
{
    if (v >= m_tokens.count())
        return QByteArray();

    const PreprocessorToken &token = m_tokens.at(v);
    return QByteArray(token.cStr, token.cStrLen);
}

void QMakeParser::adv(int count)
{
    currentToken += count;
}

bool QMakeParser::atEnd() const
{
    return !(currentToken < m_tokens.count());
}

bool QMakeParser::isOp() const
{
    return token() >= TOKEN_OP_BEGIN &&
           token() <= TOKEN_OP_END;
}

void QMakeParser::dump()
{
    if (m_parseBlock)
        m_parseBlock->dump();
}

