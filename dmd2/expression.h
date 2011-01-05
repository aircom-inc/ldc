
// Compiler implementation of the D programming language
// Copyright (c) 1999-2010 by Digital Mars
// All Rights Reserved
// written by Walter Bright
// http://www.digitalmars.com
// License for redistribution is by either the Artistic License
// in artistic.txt, or the GNU General Public License in gnu.txt.
// See the included readme.txt for details.

#ifndef DMD_EXPRESSION_H
#define DMD_EXPRESSION_H

#include "mars.h"
#include "identifier.h"
#include "lexer.h"
#include "arraytypes.h"

struct Type;
struct Scope;
struct TupleDeclaration;
struct VarDeclaration;
struct FuncDeclaration;
struct FuncLiteralDeclaration;
struct Declaration;
struct CtorDeclaration;
struct NewDeclaration;
struct Dsymbol;
struct Import;
struct Module;
struct ScopeDsymbol;
struct InlineCostState;
struct InlineDoState;
struct InlineScanState;
struct Expression;
struct Declaration;
struct AggregateDeclaration;
struct StructDeclaration;
struct TemplateInstance;
struct TemplateDeclaration;
struct ClassDeclaration;
struct HdrGenState;
struct BinExp;
struct InterState;
#if IN_DMD
struct Symbol;          // back end symbol
#endif
struct OverloadSet;
#if IN_LLVM
struct AssignExp;
#endif

enum TOK;

#if IN_DMD
// Back end
struct IRState;
struct dt_t;
#endif

#ifdef IN_GCC
union tree_node; typedef union tree_node elem;
#endif
#if IN_DMD
struct elem;
#endif

#if IN_LLVM
struct IRState;
struct DValue;
namespace llvm {
    class Constant;
    class ConstantInt;
}
#endif

void initPrecedence();

Expression *resolveProperties(Scope *sc, Expression *e);
void accessCheck(Loc loc, Scope *sc, Expression *e, Declaration *d);
Expression *build_overload(Loc loc, Scope *sc, Expression *ethis, Expression *earg, Identifier *id, Objects *targsi = NULL);
Dsymbol *search_function(ScopeDsymbol *ad, Identifier *funcid);
void inferApplyArgTypes(enum TOK op, Parameters *arguments, Expression *aggr, Module *from);
void argExpTypesToCBuffer(OutBuffer *buf, Expressions *arguments, HdrGenState *hgs);
void argsToCBuffer(OutBuffer *buf, Expressions *arguments, HdrGenState *hgs);
void expandTuples(Expressions *exps);
FuncDeclaration *hasThis(Scope *sc);
Expression *fromConstInitializer(int result, Expression *e);
int arrayExpressionCanThrow(Expressions *exps, bool mustNotThrow);

struct IntRange
{   uinteger_t imin;
    uinteger_t imax;
};

struct Expression : Object
{
    Loc loc;                    // file location
    enum TOK op;                // handy to minimize use of dynamic_cast
    Type *type;                 // !=NULL means that semantic() has been run
    unsigned char size;         // # of bytes in Expression so we can copy() it
    unsigned char parens;       // if this is a parenthesized expression

    Expression(Loc loc, enum TOK op, int size);
    Expression *copy();
    virtual Expression *syntaxCopy();
    virtual Expression *semantic(Scope *sc);
    Expression *trySemantic(Scope *sc);

    int dyncast() { return DYNCAST_EXPRESSION; }        // kludge for template.isExpression()

    void print();
    char *toChars();
    virtual void dump(int indent);
    void error(const char *format, ...) IS_PRINTF(2);
    void warning(const char *format, ...) IS_PRINTF(2);
    virtual void rvalue();

    static Expression *combine(Expression *e1, Expression *e2);
    static Expressions *arraySyntaxCopy(Expressions *exps);

    virtual dinteger_t toInteger();
    virtual uinteger_t toUInteger();
    virtual real_t toReal();
    virtual real_t toImaginary();
    virtual complex_t toComplex();
    virtual void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    virtual void toMangleBuffer(OutBuffer *buf);
    virtual int isLvalue();
    virtual Expression *toLvalue(Scope *sc, Expression *e);
    virtual Expression *modifiableLvalue(Scope *sc, Expression *e);
    virtual Expression *implicitCastTo(Scope *sc, Type *t);
    virtual MATCH implicitConvTo(Type *t);
    virtual IntRange getIntRange();
    virtual Expression *castTo(Scope *sc, Type *t);
    virtual void checkEscape();
    virtual void checkEscapeRef();
    virtual Expression *resolveLoc(Loc loc, Scope *sc);
    void checkScalar();
    void checkNoBool();
    Expression *checkIntegral();
    Expression *checkArithmetic();
    void checkDeprecated(Scope *sc, Dsymbol *s);
    void checkPurity(Scope *sc, FuncDeclaration *f);
    void checkSafety(Scope *sc, FuncDeclaration *f);
    virtual Expression *checkToBoolean(Scope *sc);
    Expression *checkToPointer();
    Expression *addressOf(Scope *sc);
    Expression *deref();
    Expression *integralPromotions(Scope *sc);

    Expression *toDelegate(Scope *sc, Type *t);
    virtual void scanForNestedRef(Scope *sc);

    virtual Expression *optimize(int result);
    #define WANTflags   1
    #define WANTvalue   2
    #define WANTinterpret 4

    virtual Expression *interpret(InterState *istate);

    virtual int isConst();
    virtual int isBool(int result);
    virtual int isBit();
    virtual int checkSideEffect(int flag);
    virtual int canThrow(bool mustNotThrow);

    virtual int inlineCost(InlineCostState *ics);
    virtual Expression *doInline(InlineDoState *ids);
    virtual Expression *inlineScan(InlineScanState *iss);
    Expression *inlineCopy(Scope *sc);

    // For operator overloading
    virtual int isCommutative();
    virtual Identifier *opId();
    virtual Identifier *opId_r();

    // For array ops
    virtual void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    virtual Expression *buildArrayLoop(Parameters *fparams);
    int isArrayOperand();

#if IN_DMD
    // Back end
    virtual elem *toElem(IRState *irs);
    virtual dt_t **toDt(dt_t **pdt);
#endif

#if IN_LLVM
    virtual DValue* toElem(IRState* irs);
    virtual llvm::Constant *toConstElem(IRState *irs);
    virtual void cacheLvalue(IRState* irs);

    llvm::Value* cachedLvalue;

    virtual AssignExp* isAssignExp() { return NULL; }
#endif
};

struct IntegerExp : Expression
{
    dinteger_t value;

    IntegerExp(Loc loc, dinteger_t value, Type *type);
    IntegerExp(dinteger_t value);
    int equals(Object *o);
    Expression *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    char *toChars();
    void dump(int indent);
    IntRange getIntRange();
    dinteger_t toInteger();
    real_t toReal();
    real_t toImaginary();
    complex_t toComplex();
    int isConst();
    int isBool(int result);
    MATCH implicitConvTo(Type *t);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void toMangleBuffer(OutBuffer *buf);
    Expression *toLvalue(Scope *sc, Expression *e);
#if IN_DMD
    elem *toElem(IRState *irs);
    dt_t **toDt(dt_t **pdt);
#elif IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
#endif
};

struct ErrorExp : IntegerExp
{
    ErrorExp();

    Expression *implicitCastTo(Scope *sc, Type *t);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    Expression *toLvalue(Scope *sc, Expression *e);
};

struct RealExp : Expression
{
    real_t value;

    RealExp(Loc loc, real_t value, Type *type);
    int equals(Object *o);
    Expression *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    char *toChars();
    dinteger_t toInteger();
    uinteger_t toUInteger();
    real_t toReal();
    real_t toImaginary();
    complex_t toComplex();
    Expression *castTo(Scope *sc, Type *t);
    int isConst();
    int isBool(int result);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void toMangleBuffer(OutBuffer *buf);
#if IN_DMD
    elem *toElem(IRState *irs);
    dt_t **toDt(dt_t **pdt);
#elif IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
#endif
};

struct ComplexExp : Expression
{
    complex_t value;

    ComplexExp(Loc loc, complex_t value, Type *type);
    int equals(Object *o);
    Expression *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    char *toChars();
    dinteger_t toInteger();
    uinteger_t toUInteger();
    real_t toReal();
    real_t toImaginary();
    complex_t toComplex();
    Expression *castTo(Scope *sc, Type *t);
    int isConst();
    int isBool(int result);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void toMangleBuffer(OutBuffer *buf);
#ifdef _DH
    OutBuffer hexp;
#endif
#if IN_DMD
    elem *toElem(IRState *irs);
    dt_t **toDt(dt_t **pdt);
#elif IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
#endif
};

struct IdentifierExp : Expression
{
    Identifier *ident;
    Declaration *var;

    IdentifierExp(Loc loc, Identifier *ident);
    IdentifierExp(Loc loc, Declaration *var);
    Expression *semantic(Scope *sc);
    char *toChars();
    void dump(int indent);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
};

struct DollarExp : IdentifierExp
{
    DollarExp(Loc loc);
};

struct DsymbolExp : Expression
{
    Dsymbol *s;
    int hasOverloads;

    DsymbolExp(Loc loc, Dsymbol *s, int hasOverloads = 0);
    Expression *semantic(Scope *sc);
    char *toChars();
    void dump(int indent);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
};

struct ThisExp : Expression
{
    Declaration *var;

    ThisExp(Loc loc);
    Expression *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    int isBool(int result);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    void scanForNestedRef(Scope *sc);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    //Expression *inlineScan(InlineScanState *iss);

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct SuperExp : ThisExp
{
    SuperExp(Loc loc);
    Expression *semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void scanForNestedRef(Scope *sc);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    //Expression *inlineScan(InlineScanState *iss);
};

struct NullExp : Expression
{
    unsigned char committed;    // !=0 if type is committed

    NullExp(Loc loc, Type *t = NULL);
    Expression *semantic(Scope *sc);
    int isBool(int result);
    int isConst();
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void toMangleBuffer(OutBuffer *buf);
    MATCH implicitConvTo(Type *t);
    Expression *castTo(Scope *sc, Type *t);
    Expression *interpret(InterState *istate);
#if IN_DMD
    elem *toElem(IRState *irs);
    dt_t **toDt(dt_t **pdt);
#elif IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
#endif
};

struct StringExp : Expression
{
    void *string;       // char, wchar, or dchar data
    size_t len;         // number of chars, wchars, or dchars
    unsigned char sz;   // 1: char, 2: wchar, 4: dchar
    unsigned char committed;    // !=0 if type is committed
    unsigned char postfix;      // 'c', 'w', 'd'

    StringExp(Loc loc, char *s);
    StringExp(Loc loc, void *s, size_t len);
    StringExp(Loc loc, void *s, size_t len, unsigned char postfix);
    //Expression *syntaxCopy();
    int equals(Object *o);
    char *toChars();
    Expression *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    size_t length();
    StringExp *toUTF8(Scope *sc);
    Expression *implicitCastTo(Scope *sc, Type *t);
    MATCH implicitConvTo(Type *t);
    Expression *castTo(Scope *sc, Type *t);
    int compare(Object *obj);
    int isBool(int result);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    unsigned charAt(size_t i);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void toMangleBuffer(OutBuffer *buf);
#if IN_DMD
    elem *toElem(IRState *irs);
    dt_t **toDt(dt_t **pdt);
#elif IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
#endif
};

// Tuple

struct TupleExp : Expression
{
    Expressions *exps;

    TupleExp(Loc loc, Expressions *exps);
    TupleExp(Loc loc, TupleDeclaration *tup);
    Expression *syntaxCopy();
    int equals(Object *o);
    Expression *semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void scanForNestedRef(Scope *sc);
    void checkEscape();
    int checkSideEffect(int flag);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    Expression *castTo(Scope *sc, Type *t);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif
    int canThrow(bool mustNotThrow);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    Expression *inlineScan(InlineScanState *iss);

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct ArrayLiteralExp : Expression
{
    Expressions *elements;

    ArrayLiteralExp(Loc loc, Expressions *elements);
    ArrayLiteralExp(Loc loc, Expression *e);

    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    int isBool(int result);
    int checkSideEffect(int flag);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void toMangleBuffer(OutBuffer *buf);
    void scanForNestedRef(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    MATCH implicitConvTo(Type *t);
    Expression *castTo(Scope *sc, Type *t);
    int canThrow(bool mustNotThrow);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    Expression *inlineScan(InlineScanState *iss);

#if IN_DMD
    elem *toElem(IRState *irs);
    dt_t **toDt(dt_t **pdt);
#elif IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
#endif
};

struct AssocArrayLiteralExp : Expression
{
    Expressions *keys;
    Expressions *values;

    AssocArrayLiteralExp(Loc loc, Expressions *keys, Expressions *values);

    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    int isBool(int result);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif
    int checkSideEffect(int flag);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void toMangleBuffer(OutBuffer *buf);
    void scanForNestedRef(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    MATCH implicitConvTo(Type *t);
    Expression *castTo(Scope *sc, Type *t);
    int canThrow(bool mustNotThrow);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    Expression *inlineScan(InlineScanState *iss);

#if IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
#endif
};

struct StructLiteralExp : Expression
{
    StructDeclaration *sd;      // which aggregate this is for
    Expressions *elements;      // parallels sd->fields[] with
                                // NULL entries for fields to skip
    Type *stype;                // final type of result (can be different from sd's type)

#if IN_DMD
    Symbol *sym;                // back end symbol to initialize with literal
#endif
    size_t soffset;             // offset from start of s
    int fillHoles;              // fill alignment 'holes' with zero

    StructLiteralExp(Loc loc, StructDeclaration *sd, Expressions *elements, Type *stype = NULL);

    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    Expression *getField(Type *type, unsigned offset);
    int getFieldIndex(Type *type, unsigned offset);
    int checkSideEffect(int flag);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void toMangleBuffer(OutBuffer *buf);
    void scanForNestedRef(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    int canThrow(bool mustNotThrow);
    MATCH implicitConvTo(Type *t);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    Expression *inlineScan(InlineScanState *iss);

#if IN_DMD
    elem *toElem(IRState *irs);
    dt_t **toDt(dt_t **pdt);
#elif IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
#endif
};

Expression *typeDotIdExp(Loc loc, Type *type, Identifier *ident);
#if IN_DMD
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif

struct TypeExp : Expression
{
    TypeExp(Loc loc, Type *type);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void rvalue();
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    Expression *optimize(int result);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct ScopeExp : Expression
{
    ScopeDsymbol *sds;

    ScopeExp(Loc loc, ScopeDsymbol *sds);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct TemplateExp : Expression
{
    TemplateDeclaration *td;

    TemplateExp(Loc loc, TemplateDeclaration *td);
    void rvalue();
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
};

struct NewExp : Expression
{
    /* thisexp.new(newargs) newtype(arguments)
     */
    Expression *thisexp;        // if !NULL, 'this' for class being allocated
    Expressions *newargs;       // Array of Expression's to call new operator
    Type *newtype;
    Expressions *arguments;     // Array of Expression's

    CtorDeclaration *member;    // constructor function
    NewDeclaration *allocator;  // allocator function
    int onstack;                // allocate on stack

    NewExp(Loc loc, Expression *thisexp, Expressions *newargs,
        Type *newtype, Expressions *arguments);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    Expression *optimize(int result);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif
    int checkSideEffect(int flag);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void scanForNestedRef(Scope *sc);
    int canThrow(bool mustNotThrow);

    //int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    //Expression *inlineScan(InlineScanState *iss);

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct NewAnonClassExp : Expression
{
    /* thisexp.new(newargs) class baseclasses { } (arguments)
     */
    Expression *thisexp;        // if !NULL, 'this' for class being allocated
    Expressions *newargs;       // Array of Expression's to call new operator
    ClassDeclaration *cd;       // class being instantiated
    Expressions *arguments;     // Array of Expression's to call class constructor

    NewAnonClassExp(Loc loc, Expression *thisexp, Expressions *newargs,
        ClassDeclaration *cd, Expressions *arguments);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    int checkSideEffect(int flag);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    int canThrow(bool mustNotThrow);
};

#if DMDV2
struct SymbolExp : Expression
{
    Declaration *var;
    int hasOverloads;

    SymbolExp(Loc loc, enum TOK op, int size, Declaration *var, int hasOverloads);

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};
#endif

// Offset from symbol

struct SymOffExp : SymbolExp
{
    unsigned offset;
    Module* m;	// starting point for overload resolution

    SymOffExp(Loc loc, Declaration *var, unsigned offset, int hasOverloads = 0);
    Expression *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    void checkEscape();
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    int isConst();
    int isBool(int result);
    Expression *doInline(InlineDoState *ids);
    MATCH implicitConvTo(Type *t);
    Expression *castTo(Scope *sc, Type *t);
    void scanForNestedRef(Scope *sc);

#if IN_DMD
    dt_t **toDt(dt_t **pdt);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

// Variable

struct VarExp : SymbolExp
{
    VarExp(Loc loc, Declaration *var, int hasOverloads = 0);
    int equals(Object *o);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void dump(int indent);
    char *toChars();
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void checkEscape();
    void checkEscapeRef();
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
#if IN_DMD
    dt_t **toDt(dt_t **pdt);
#endif
    void scanForNestedRef(Scope *sc);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    //Expression *inlineScan(InlineScanState *iss);

#if IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
    void cacheLvalue(IRState* irs);
#endif
};

#if DMDV2
// Overload Set

struct OverExp : Expression
{
    OverloadSet *vars;

    OverExp(OverloadSet *s);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
};
#endif

// Function/Delegate literal

struct FuncExp : Expression
{
    FuncLiteralDeclaration *fd;

    FuncExp(Loc loc, FuncLiteralDeclaration *fd);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    void scanForNestedRef(Scope *sc);
    char *toChars();
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

    int inlineCost(InlineCostState *ics);
    //Expression *doInline(InlineDoState *ids);
    //Expression *inlineScan(InlineScanState *iss);

#if IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
#endif
};

// Declaration of a symbol

struct DeclarationExp : Expression
{
    Dsymbol *declaration;

    DeclarationExp(Loc loc, Dsymbol *declaration);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    int checkSideEffect(int flag);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif
    void scanForNestedRef(Scope *sc);
    int canThrow(bool mustNotThrow);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    Expression *inlineScan(InlineScanState *iss);

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct TypeidExp : Expression
{
    Object *obj;

    TypeidExp(Loc loc, Object *obj);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
};

#if DMDV2
struct TraitsExp : Expression
{
    Identifier *ident;
    Objects *args;

    TraitsExp(Loc loc, Identifier *ident, Objects *args);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
};
#endif

struct HaltExp : Expression
{
    HaltExp(Loc loc);
    Expression *semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    int checkSideEffect(int flag);

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct IsExp : Expression
{
    /* is(targ id tok tspec)
     * is(targ id == tok2)
     */
    Type *targ;
    Identifier *id;     // can be NULL
    enum TOK tok;       // ':' or '=='
    Type *tspec;        // can be NULL
    enum TOK tok2;      // 'struct', 'union', 'typedef', etc.
    TemplateParameters *parameters;

    IsExp(Loc loc, Type *targ, Identifier *id, enum TOK tok, Type *tspec,
        enum TOK tok2, TemplateParameters *parameters);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
};

/****************************************************************/

struct UnaExp : Expression
{
    Expression *e1;

    UnaExp(Loc loc, enum TOK op, int size, Expression *e1);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    Expression *optimize(int result);
    void dump(int indent);
    void scanForNestedRef(Scope *sc);
    Expression *interpretCommon(InterState *istate, Expression *(*fp)(Type *, Expression *));
    int canThrow(bool mustNotThrow);
    Expression *resolveLoc(Loc loc, Scope *sc);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    Expression *inlineScan(InlineScanState *iss);

    virtual Expression *op_overload(Scope *sc);
};

struct BinExp : Expression
{
    Expression *e1;
    Expression *e2;

    BinExp(Loc loc, enum TOK op, int size, Expression *e1, Expression *e2);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    Expression *semanticp(Scope *sc);
    int checkSideEffect(int flag);
    void checkComplexMulAssign();
    void checkComplexAddAssign();
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    Expression *scaleFactor(Scope *sc);
    Expression *typeCombine(Scope *sc);
    Expression *optimize(int result);
    int isunsigned();
    void incompatibleTypes();
    void dump(int indent);
    void scanForNestedRef(Scope *sc);
    Expression *interpretCommon(InterState *istate, Expression *(*fp)(Type *, Expression *, Expression *));
    Expression *interpretCommon2(InterState *istate, Expression *(*fp)(TOK, Type *, Expression *, Expression *));
    Expression *interpretAssignCommon(InterState *istate, Expression *(*fp)(Type *, Expression *, Expression *), int post = 0);
    int canThrow(bool mustNotThrow);
    Expression *arrayOp(Scope *sc);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    Expression *inlineScan(InlineScanState *iss);

    Expression *op_overload(Scope *sc);
    Expression *compare_overload(Scope *sc, Identifier *id);

#if IN_DMD
    elem *toElemBin(IRState *irs, int op);
#endif
};

struct BinAssignExp : BinExp
{
    BinAssignExp(Loc loc, enum TOK op, int size, Expression *e1, Expression *e2)
        : BinExp(loc, op, size, e1, e2)
    {
    }

    Expression *commonSemanticAssign(Scope *sc);
    Expression *commonSemanticAssignIntegral(Scope *sc);

    Expression *op_overload(Scope *sc);

    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *ex);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
};

/****************************************************************/

struct CompileExp : UnaExp
{
    CompileExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
};

struct FileExp : UnaExp
{
    FileExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
};

struct AssertExp : UnaExp
{
    Expression *msg;

    AssertExp(Loc loc, Expression *e, Expression *msg = NULL);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    int checkSideEffect(int flag);
    int canThrow(bool mustNotThrow);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    Expression *inlineScan(InlineScanState *iss);

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct DotIdExp : UnaExp
{
    Identifier *ident;

    DotIdExp(Loc loc, Expression *e, Identifier *ident);
    Expression *semantic(Scope *sc);
    Expression *semantic(Scope *sc, int flag);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void dump(int i);
};

struct DotTemplateExp : UnaExp
{
    TemplateDeclaration *td;

    DotTemplateExp(Loc loc, Expression *e, TemplateDeclaration *td);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
};

struct DotVarExp : UnaExp
{
    Declaration *var;
    int hasOverloads;

    DotVarExp(Loc loc, Expression *e, Declaration *var, int hasOverloads = 0);
    Expression *semantic(Scope *sc);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void dump(int indent);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
    void cacheLvalue(IRState* irs);
#endif
};

struct DotTemplateInstanceExp : UnaExp
{
    TemplateInstance *ti;

    DotTemplateInstanceExp(Loc loc, Expression *e, Identifier *name, Objects *tiargs);
    Expression *syntaxCopy();
    TemplateDeclaration *getTempdecl(Scope *sc);
    Expression *semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void dump(int indent);
};

struct DelegateExp : UnaExp
{
    FuncDeclaration *func;
    Module* m;	// starting point for overload resolution
    int hasOverloads;

    DelegateExp(Loc loc, Expression *e, FuncDeclaration *func, int hasOverloads = 0);
    Expression *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    MATCH implicitConvTo(Type *t);
    Expression *castTo(Scope *sc, Type *t);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void dump(int indent);

    int inlineCost(InlineCostState *ics);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct DotTypeExp : UnaExp
{
    Dsymbol *sym;               // symbol that represents a type

    DotTypeExp(Loc loc, Expression *e, Dsymbol *sym);
    Expression *semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct CallExp : UnaExp
{
    Expressions *arguments;     // function arguments

    CallExp(Loc loc, Expression *e, Expressions *exps);
    CallExp(Loc loc, Expression *e);
    CallExp(Loc loc, Expression *e, Expression *earg1);
    CallExp(Loc loc, Expression *e, Expression *earg1, Expression *earg2);

    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    int checkSideEffect(int flag);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void dump(int indent);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif
    void scanForNestedRef(Scope *sc);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    int canThrow(bool mustNotThrow);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    Expression *inlineScan(InlineScanState *iss);

#if IN_LLVM
    DValue* toElem(IRState* irs);
#if DMDV2
    void cacheLvalue(IRState* p);
#endif
#endif
};

struct AddrExp : UnaExp
{
    Module* m;	// starting point for overload resolution

    AddrExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    void checkEscape();
#if IN_DMD
    elem *toElem(IRState *irs);
#endif
    MATCH implicitConvTo(Type *t);
    Expression *castTo(Scope *sc, Type *t);
    Expression *optimize(int result);

#if IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
    Expression *interpret(InterState *istate);
#endif
};

struct PtrExp : UnaExp
{
    PtrExp(Loc loc, Expression *e);
    PtrExp(Loc loc, Expression *e, Type *t);
    Expression *semantic(Scope *sc);
    int isLvalue();
    void checkEscapeRef();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);

    // For operator overloading
    Identifier *opId();

#if IN_LLVM
    DValue* toElem(IRState* irs);
    void cacheLvalue(IRState* irs);
#endif
};

struct NegExp : UnaExp
{
    NegExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    Expression *buildArrayLoop(Parameters *fparams);

    // For operator overloading
    Identifier *opId();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct UAddExp : UnaExp
{
    UAddExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);

    // For operator overloading
    Identifier *opId();
};

struct ComExp : UnaExp
{
    ComExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    Expression *buildArrayLoop(Parameters *fparams);

    // For operator overloading
    Identifier *opId();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct NotExp : UnaExp
{
    NotExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    int isBit();
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct BoolExp : UnaExp
{
    BoolExp(Loc loc, Expression *e, Type *type);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    int isBit();
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct DeleteExp : UnaExp
{
    DeleteExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    Expression *checkToBoolean(Scope *sc);
    int checkSideEffect(int flag);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct CastExp : UnaExp
{
    // Possible to cast to one type while painting to another type
    Type *to;                   // type to cast to
    unsigned mod;               // MODxxxxx

    CastExp(Loc loc, Expression *e, Type *t);
    CastExp(Loc loc, Expression *e, unsigned mod);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    MATCH implicitConvTo(Type *t);
    IntRange getIntRange();
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    int checkSideEffect(int flag);
    void checkEscape();
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    Expression *buildArrayLoop(Parameters *fparams);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

    // For operator overloading
    Identifier *opId();
    Expression *op_overload(Scope *sc);

#if IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
    bool disableOptimization;
#endif
};


struct SliceExp : UnaExp
{
    Expression *upr;            // NULL if implicit 0
    Expression *lwr;            // NULL if implicit [length - 1]
    VarDeclaration *lengthVar;

    SliceExp(Loc loc, Expression *e1, Expression *lwr, Expression *upr);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void checkEscape();
    void checkEscapeRef();
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void dump(int indent);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif
    void scanForNestedRef(Scope *sc);
    void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    Expression *buildArrayLoop(Parameters *fparams);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    Expression *inlineScan(InlineScanState *iss);

#if IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
#endif
};

struct ArrayLengthExp : UnaExp
{
    ArrayLengthExp(Loc loc, Expression *e1);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

    static Expression *rewriteOpAssign(BinExp *exp);
    
#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

// e1[a0,a1,a2,a3,...]

struct ArrayExp : UnaExp
{
    Expressions *arguments;             // Array of Expression's

    ArrayExp(Loc loc, Expression *e1, Expressions *arguments);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    void scanForNestedRef(Scope *sc);

    // For operator overloading
    Identifier *opId();
    Expression *op_overload(Scope *sc);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    Expression *inlineScan(InlineScanState *iss);
};

/****************************************************************/

struct DotExp : BinExp
{
    DotExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
};

struct CommaExp : BinExp
{
    CommaExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    void checkEscape();
    void checkEscapeRef();
    IntRange getIntRange();
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
    int isBool(int result);
    int checkSideEffect(int flag);
    MATCH implicitConvTo(Type *t);
    Expression *castTo(Scope *sc, Type *t);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct IndexExp : BinExp
{
    VarDeclaration *lengthVar;
    int modifiable;

    IndexExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    Expression *doInline(InlineDoState *ids);
    void scanForNestedRef(Scope *sc);

#if IN_DMD
    elem *toElem(IRState *irs);
#elif IN_LLVM
    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
    void cacheLvalue(IRState* irs);
#endif
};

/* For both i++ and i--
 */
struct PostExp : BinExp
{
    PostExp(enum TOK op, Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    Identifier *opId();    // For operator overloading
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

/* For both ++i and --i
 */
struct PreExp : UnaExp
{
    PreExp(enum TOK op, Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
};

struct AssignExp : BinExp
{   int ismemset;       // !=0 if setting the contents of an array

    AssignExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *checkToBoolean(Scope *sc);
    Expression *interpret(InterState *istate);
    Identifier *opId();    // For operator overloading
    void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    Expression *buildArrayLoop(Parameters *fparams);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
    virtual AssignExp* isAssignExp() { return this; }
#endif
};

struct ConstructExp : AssignExp
{
    ConstructExp(Loc loc, Expression *e1, Expression *e2);
};

#if IN_DMD
#define ASSIGNEXP_TOELEM    elem *toElem(IRState *irs);
#elif IN_LLVM
#define ASSIGNEXP_TOELEM    DValue* toElem(IRState *irs);
#else
#define ASSIGNEXP_TOELEM
#endif

#define ASSIGNEXP(op)   \
struct op##AssignExp : BinAssignExp                             \
{                                                               \
    op##AssignExp(Loc loc, Expression *e1, Expression *e2);     \
    Expression *semantic(Scope *sc);                            \
    Expression *interpret(InterState *istate);                  \
    X(void buildArrayIdent(OutBuffer *buf, Expressions *arguments);) \
    X(Expression *buildArrayLoop(Parameters *fparams);)         \
                                                                \
    Identifier *opId();    /* For operator overloading */       \
                                                                \
    ASSIGNEXP_TOELEM					\
};

#define X(a) a
ASSIGNEXP(Add)
ASSIGNEXP(Min)
ASSIGNEXP(Mul)
ASSIGNEXP(Div)
ASSIGNEXP(Mod)
ASSIGNEXP(And)
ASSIGNEXP(Or)
ASSIGNEXP(Xor)
#undef X

#define X(a)

ASSIGNEXP(Shl)
ASSIGNEXP(Shr)
ASSIGNEXP(Ushr)
ASSIGNEXP(Cat)

#undef X
#undef ASSIGNEXP
#undef ASSIGNEXP_TOELEM

// Only a reduced subset of operations for now.
struct PowAssignExp : BinAssignExp
{
    PowAssignExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    // For operator overloading
    Identifier *opId();
};

struct AddExp : BinExp
{
    AddExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    Expression *buildArrayLoop(Parameters *fparams);

    // For operator overloading
    int isCommutative();
    Identifier *opId();
    Identifier *opId_r();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct MinExp : BinExp
{
    MinExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    Expression *buildArrayLoop(Parameters *fparams);

    // For operator overloading
    Identifier *opId();
    Identifier *opId_r();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct CatExp : BinExp
{
    CatExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);

    // For operator overloading
    Identifier *opId();
    Identifier *opId_r();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct MulExp : BinExp
{
    MulExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    Expression *buildArrayLoop(Parameters *fparams);

    // For operator overloading
    int isCommutative();
    Identifier *opId();
    Identifier *opId_r();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct DivExp : BinExp
{
    DivExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    Expression *buildArrayLoop(Parameters *fparams);
    IntRange getIntRange();

    // For operator overloading
    Identifier *opId();
    Identifier *opId_r();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct ModExp : BinExp
{
    ModExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    Expression *buildArrayLoop(Parameters *fparams);

    // For operator overloading
    Identifier *opId();
    Identifier *opId_r();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

#if DMDV2
struct PowExp : BinExp
{
    PowExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    // For operator overloading
    Identifier *opId();
    Identifier *opId_r();
};
#endif

struct ShlExp : BinExp
{
    ShlExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    IntRange getIntRange();

    // For operator overloading
    Identifier *opId();
    Identifier *opId_r();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct ShrExp : BinExp
{
    ShrExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    IntRange getIntRange();

    // For operator overloading
    Identifier *opId();
    Identifier *opId_r();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct UshrExp : BinExp
{
    UshrExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    IntRange getIntRange();

    // For operator overloading
    Identifier *opId();
    Identifier *opId_r();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct AndExp : BinExp
{
    AndExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    Expression *buildArrayLoop(Parameters *fparams);
    IntRange getIntRange();

    // For operator overloading
    int isCommutative();
    Identifier *opId();
    Identifier *opId_r();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct OrExp : BinExp
{
    OrExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    Expression *buildArrayLoop(Parameters *fparams);
    MATCH implicitConvTo(Type *t);
    IntRange getIntRange();

    // For operator overloading
    int isCommutative();
    Identifier *opId();
    Identifier *opId_r();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct XorExp : BinExp
{
    XorExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void buildArrayIdent(OutBuffer *buf, Expressions *arguments);
    Expression *buildArrayLoop(Parameters *fparams);
    MATCH implicitConvTo(Type *t);
    IntRange getIntRange();

    // For operator overloading
    int isCommutative();
    Identifier *opId();
    Identifier *opId_r();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct OrOrExp : BinExp
{
    OrOrExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *checkToBoolean(Scope *sc);
    int isBit();
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    int checkSideEffect(int flag);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct AndAndExp : BinExp
{
    AndAndExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *checkToBoolean(Scope *sc);
    int isBit();
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    int checkSideEffect(int flag);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct CmpExp : BinExp
{
    CmpExp(enum TOK op, Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    int isBit();

    // For operator overloading
    int isCommutative();
    Identifier *opId();
    Expression *op_overload(Scope *sc);

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct InExp : BinExp
{
    InExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    int isBit();

    // For operator overloading
    Identifier *opId();
    Identifier *opId_r();

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

struct RemoveExp : BinExp
{
    RemoveExp(Loc loc, Expression *e1, Expression *e2);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

// == and !=

struct EqualExp : BinExp
{
    EqualExp(enum TOK op, Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    int isBit();

    // For operator overloading
    int isCommutative();
    Identifier *opId();
    Expression *op_overload(Scope *sc);

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

// === and !===

struct IdentityExp : BinExp
{
    IdentityExp(enum TOK op, Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    int isBit();
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

/****************************************************************/

struct CondExp : BinExp
{
    Expression *econd;

    CondExp(Loc loc, Expression *econd, Expression *e1, Expression *e2);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    Expression *optimize(int result);
    Expression *interpret(InterState *istate);
    void checkEscape();
    void checkEscapeRef();
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
    Expression *checkToBoolean(Scope *sc);
    int checkSideEffect(int flag);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    MATCH implicitConvTo(Type *t);
    Expression *castTo(Scope *sc, Type *t);
    void scanForNestedRef(Scope *sc);
    int canThrow(bool mustNotThrow);

    int inlineCost(InlineCostState *ics);
    Expression *doInline(InlineDoState *ids);
    Expression *inlineScan(InlineScanState *iss);

#if IN_DMD
    elem *toElem(IRState *irs);
#endif

#if IN_LLVM
    DValue* toElem(IRState* irs);
#endif
};

#if DMDV2
/****************************************************************/

struct DefaultInitExp : Expression
{
    enum TOK subop;             // which of the derived classes this is

    DefaultInitExp(Loc loc, enum TOK subop, int size);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
};

struct FileInitExp : DefaultInitExp
{
    FileInitExp(Loc loc);
    Expression *semantic(Scope *sc);
    Expression *resolveLoc(Loc loc, Scope *sc);
};

struct LineInitExp : DefaultInitExp
{
    LineInitExp(Loc loc);
    Expression *semantic(Scope *sc);
    Expression *resolveLoc(Loc loc, Scope *sc);
};
#endif

/****************************************************************/

#if IN_LLVM

// this stuff is strictly LDC

struct GEPExp : UnaExp
{
    unsigned index;
    Identifier* ident;

    GEPExp(Loc loc, Expression* e, Identifier* id, unsigned idx);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    Expression *toLvalue(Scope *sc, Expression *e);

    DValue* toElem(IRState* irs);
    llvm::Constant *toConstElem(IRState *irs);
};

#endif

/****************************************************************/

/* Special values used by the interpreter
 */
#define EXP_CANT_INTERPRET      ((Expression *)1)
#define EXP_CONTINUE_INTERPRET  ((Expression *)2)
#define EXP_BREAK_INTERPRET     ((Expression *)3)
#define EXP_GOTO_INTERPRET      ((Expression *)4)
#define EXP_VOID_INTERPRET      ((Expression *)5)

Expression *expType(Type *type, Expression *e);

Expression *Neg(Type *type, Expression *e1);
Expression *Com(Type *type, Expression *e1);
Expression *Not(Type *type, Expression *e1);
Expression *Bool(Type *type, Expression *e1);
Expression *Cast(Type *type, Type *to, Expression *e1);
Expression *ArrayLength(Type *type, Expression *e1);
Expression *Ptr(Type *type, Expression *e1);

Expression *Add(Type *type, Expression *e1, Expression *e2);
Expression *Min(Type *type, Expression *e1, Expression *e2);
Expression *Mul(Type *type, Expression *e1, Expression *e2);
Expression *Div(Type *type, Expression *e1, Expression *e2);
Expression *Mod(Type *type, Expression *e1, Expression *e2);
Expression *Shl(Type *type, Expression *e1, Expression *e2);
Expression *Shr(Type *type, Expression *e1, Expression *e2);
Expression *Ushr(Type *type, Expression *e1, Expression *e2);
Expression *And(Type *type, Expression *e1, Expression *e2);
Expression *Or(Type *type, Expression *e1, Expression *e2);
Expression *Xor(Type *type, Expression *e1, Expression *e2);
Expression *Index(Type *type, Expression *e1, Expression *e2);
Expression *Cat(Type *type, Expression *e1, Expression *e2);

Expression *Equal(enum TOK op, Type *type, Expression *e1, Expression *e2);
Expression *Cmp(enum TOK op, Type *type, Expression *e1, Expression *e2);
Expression *Identity(enum TOK op, Type *type, Expression *e1, Expression *e2);

Expression *Slice(Type *type, Expression *e1, Expression *lwr, Expression *upr);

#endif /* DMD_EXPRESSION_H */
