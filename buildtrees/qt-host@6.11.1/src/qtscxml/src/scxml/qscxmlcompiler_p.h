// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:scritical reason:data-parser

#ifndef QSCXMLCOMPILER_P_H
#define QSCXMLCOMPILER_P_H

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

#include "qscxmlcompiler.h"

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qset.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qstring.h>
#include <QtCore/qxmlstream.h>
#include <QtCore/private/qglobal_p.h>

#include <memory>

QT_BEGIN_NAMESPACE

namespace DocumentModel {

struct XmlLocation
{
    int line;
    int column;

    XmlLocation(int theLine, int theColumn): line(theLine), column(theColumn) {}
};

struct If;
struct Send;
struct Invoke;
struct Script;
struct AbstractState;
struct State;
struct Transition;
struct HistoryState;
struct Scxml;
class NodeVisitor;
struct Node {
    XmlLocation xmlLocation;

    Node(const XmlLocation &theLocation): xmlLocation(theLocation) {}
    virtual ~Node();
    virtual void accept(NodeVisitor *visitor) = 0;

    virtual If *asIf() { return nullptr; }
    virtual Send *asSend() { return nullptr; }
    virtual Invoke *asInvoke() { return nullptr; }
    virtual Script *asScript() { return nullptr; }
    virtual State *asState() { return nullptr; }
    virtual Transition *asTransition() { return nullptr; }
    virtual HistoryState *asHistoryState() { return nullptr; }
    virtual Scxml *asScxml() { return nullptr; }
    AbstractState *asAbstractState();

private:
    Q_DISABLE_COPY(Node)
};

struct DataElement: public Node
{
    QString id;
    QString src;
    QString expr;
    QString content;

    DataElement(const XmlLocation &xmlLocation): Node(xmlLocation) {}
    void accept(NodeVisitor *visitor) override;
};

struct Param: public Node
{
    QString name;
    QString expr;
    QString location;

    Param(const XmlLocation &xmlLocation): Node(xmlLocation) {}
    void accept(NodeVisitor *visitor) override;
};

struct DoneData: public Node
{
    QString contents;
    QString expr;
    QList<Param *> params;

    DoneData(const XmlLocation &xmlLocation): Node(xmlLocation) {}
    void accept(NodeVisitor *visitor) override;
};

struct Instruction: public Node
{
    Instruction(const XmlLocation &xmlLocation): Node(xmlLocation) {}
    virtual ~Instruction() {}
};

typedef QList<Instruction *> InstructionSequence;
typedef QList<InstructionSequence *> InstructionSequences;

struct Send: public Instruction
{
    QString event;
    QString eventexpr;
    QString type;
    QString typeexpr;
    QString target;
    QString targetexpr;
    QString id;
    QString idLocation;
    QString delay;
    QString delayexpr;
    QStringList namelist;
    QList<Param *> params;
    QString content;
    QString contentexpr;

    Send(const XmlLocation &xmlLocation): Instruction(xmlLocation) {}
    Send *asSend() override { return this; }
    void accept(NodeVisitor *visitor) override;
};

struct ScxmlDocument;
struct Invoke: public Instruction
{
    QString type;
    QString typeexpr;
    QString src;
    QString srcexpr;
    QString id;
    QString idLocation;
    QStringList namelist;
    bool autoforward;
    QList<Param *> params;
    InstructionSequence finalize;

    QSharedPointer<ScxmlDocument> content;

    Invoke(const XmlLocation &xmlLocation): Instruction(xmlLocation) {}
    Invoke *asInvoke() override { return this; }
    void accept(NodeVisitor *visitor) override;
};

struct Raise: public Instruction
{
    QString event;

    Raise(const XmlLocation &xmlLocation): Instruction(xmlLocation) {}
    void accept(NodeVisitor *visitor) override;
};

struct Log: public Instruction
{
    QString label, expr;

    Log(const XmlLocation &xmlLocation): Instruction(xmlLocation) {}
    void accept(NodeVisitor *visitor) override;
};

struct Script: public Instruction
{
    QString src;
    QString content;

    Script(const XmlLocation &xmlLocation): Instruction(xmlLocation) {}
    Script *asScript() override { return this; }
    void accept(NodeVisitor *visitor) override;
};

struct Assign: public Instruction
{
    QString location;
    QString expr;
    QString content;

    Assign(const XmlLocation &xmlLocation): Instruction(xmlLocation) {}
    void accept(NodeVisitor *visitor) override;
};

struct If: public Instruction
{
    QStringList conditions;
    InstructionSequences blocks;

    If(const XmlLocation &xmlLocation): Instruction(xmlLocation) {}
    If *asIf() override { return this; }
    void accept(NodeVisitor *visitor) override;
};

struct Foreach: public Instruction
{
    QString array;
    QString item;
    QString index;
    InstructionSequence block;

    Foreach(const XmlLocation &xmlLocation): Instruction(xmlLocation) {}
    void accept(NodeVisitor *visitor) override;
};

struct Cancel: public Instruction
{
    QString sendid;
    QString sendidexpr;

    Cancel(const XmlLocation &xmlLocation): Instruction(xmlLocation) {}
    void accept(NodeVisitor *visitor) override;
};

struct StateOrTransition: public Node
{
    StateOrTransition(const XmlLocation &xmlLocation): Node(xmlLocation) {}
};

struct StateContainer
{
    StateContainer()
        : parent(nullptr)
    {}

    StateContainer *parent;

    virtual ~StateContainer() {}
    virtual void add(StateOrTransition *s) = 0;
    virtual AbstractState *asAbstractState() { return nullptr; }
    virtual State *asState() { return nullptr; }
    virtual Scxml *asScxml() { return nullptr; }
};

struct AbstractState: public StateContainer
{
    QString id;

    AbstractState *asAbstractState() override { return this; }
};

struct State: public AbstractState, public StateOrTransition
{
    enum Type { Normal, Parallel, Final };

    QStringList initial;
    QList<DataElement *> dataElements;
    QList<StateOrTransition *> children;
    InstructionSequences onEntry;
    InstructionSequences onExit;
    DoneData *doneData;
    QList<Invoke *> invokes;
    Type type;

    Transition *initialTransition; // when not set, it is filled during verification

    State(const XmlLocation &xmlLocation)
        : StateOrTransition(xmlLocation)
        , doneData(nullptr)
        , type(Normal)
        , initialTransition(nullptr)
    {}

    void add(StateOrTransition *s) override
    {
        Q_ASSERT(s);
        children.append(s);
    }

    State *asState() override { return this; }

    void accept(NodeVisitor *visitor) override;
};

struct Transition: public StateOrTransition
{
    enum Type { Internal, External, Synthetic };
    QStringList events;
    QScopedPointer<QString> condition;
    QStringList targets;
    InstructionSequence instructionsOnTransition;
    Type type;

    QList<AbstractState *> targetStates; // when not set, it is filled during verification

    Transition(const XmlLocation &xmlLocation)
        : StateOrTransition(xmlLocation)
        , type(External)
    {}

    Transition *asTransition() override { return this; }

    void accept(NodeVisitor *visitor) override;
};

struct HistoryState: public AbstractState, public StateOrTransition
{
    enum Type { Deep, Shallow };
    Type type;
    QList<StateOrTransition *> children;

    HistoryState(const XmlLocation &xmlLocation)
        : StateOrTransition(xmlLocation)
        , type(Shallow)
    {}

    void add(StateOrTransition *s) override
    {
        Q_ASSERT(s);
        children.append(s);
    }

    Transition *defaultConfiguration()
    { return children.isEmpty() ? nullptr : children.first()->asTransition(); }

    HistoryState *asHistoryState() override { return this; }
    void accept(NodeVisitor *visitor) override;
};

struct Scxml: public StateContainer, public Node
{
    enum DataModelType {
        NullDataModel,
        JSDataModel,
        CppDataModel
    };
    enum BindingMethod {
        EarlyBinding,
        LateBinding
    };

    QStringList initial;
    QString name;
    DataModelType dataModel;
    QString cppDataModelClassName;
    QString cppDataModelHeaderName;
    BindingMethod binding;
    QList<StateOrTransition *> children;
    QList<DataElement *> dataElements;
    QScopedPointer<Script> script;
    InstructionSequence initialSetup;

    Transition *initialTransition;

    Scxml(const XmlLocation &xmlLocation)
        : Node(xmlLocation)
        , dataModel(NullDataModel)
        , binding(EarlyBinding)
        , initialTransition(nullptr)
    {}

    void add(StateOrTransition *s) override
    {
        Q_ASSERT(s);
        children.append(s);
    }

    Scxml *asScxml() override { return this; }

    void accept(NodeVisitor *visitor) override;
};

struct ScxmlDocument
{
    const QString fileName;
    Scxml *root;
    QList<AbstractState *> allStates;
    QList<Transition *> allTransitions;
    QList<Node *> allNodes;
    QList<InstructionSequence *> allSequences;
    QList<ScxmlDocument *> allSubDocuments; // weak pointers
    bool isVerified;

    ScxmlDocument(const QString &fileName)
        : fileName(fileName)
        , root(nullptr)
        , isVerified(false)
    {}

    ~ScxmlDocument()
    {
        delete root;
        qDeleteAll(allNodes);
        qDeleteAll(allSequences);
    }

    State *newState(StateContainer *parent, State::Type type, const XmlLocation &xmlLocation)
    {
        Q_ASSERT(parent);
        State *s = newNode<State>(xmlLocation);
        s->parent = parent;
        s->type = type;
        allStates.append(s);
        parent->add(s);
        return s;
    }

    HistoryState *newHistoryState(StateContainer *parent, const XmlLocation &xmlLocation)
    {
        Q_ASSERT(parent);
        HistoryState *s = newNode<HistoryState>(xmlLocation);
        s->parent = parent;
        allStates.append(s);
        parent->add(s);
        return s;
    }

    Transition *newTransition(StateContainer *parent, const XmlLocation &xmlLocation)
    {
        Transition *t = newNode<Transition>(xmlLocation);
        allTransitions.append(t);
        if (parent != nullptr) {
            parent->add(t);
        }
        return t;
    }

    template<typename T>
    T *newNode(const XmlLocation &xmlLocation)
    {
        T *node = new T(xmlLocation);
        allNodes.append(node);
        return node;
    }

    InstructionSequence *newSequence(InstructionSequences *container)
    {
        Q_ASSERT(container);
        InstructionSequence *is = new InstructionSequence;
        allSequences.append(is);
        container->append(is);
        return is;
    }
};

class Q_SCXML_EXPORT NodeVisitor
{
public:
    virtual ~NodeVisitor();

    virtual void visit(DataElement *) {}
    virtual void visit(Param *) {}
    virtual bool visit(DoneData *) { return true; }
    virtual void endVisit(DoneData *) {}
    virtual bool visit(Send *) { return true; }
    virtual void endVisit(Send *) {}
    virtual bool visit(Invoke *) { return true; }
    virtual void endVisit(Invoke *) {}
    virtual void visit(Raise *) {}
    virtual void visit(Log *) {}
    virtual void visit(Script *) {}
    virtual void visit(Assign *) {}
    virtual bool visit(If *) { return true; }
    virtual void endVisit(If *) {}
    virtual bool visit(Foreach *) { return true; }
    virtual void endVisit(Foreach *) {}
    virtual void visit(Cancel *) {}
    virtual bool visit(State *) { return true; }
    virtual void endVisit(State *) {}
    virtual bool visit(Transition *) { return true; }
    virtual void endVisit(Transition *) {}
    virtual bool visit(HistoryState *) { return true; }
    virtual void endVisit(HistoryState *) {}
    virtual bool visit(Scxml *) { return true; }
    virtual void endVisit(Scxml *) {}

    void visit(InstructionSequence *sequence)
    {
        Q_ASSERT(sequence);
        for (Instruction *instruction : std::as_const(*sequence)) {
            Q_ASSERT(instruction);
            instruction->accept(this);
        }
    }

    void visit(const QList<DataElement *> &dataElements)
    {
        for (DataElement *dataElement : dataElements) {
            Q_ASSERT(dataElement);
            dataElement->accept(this);
        }
    }

    void visit(const QList<StateOrTransition *> &children)
    {
        for (StateOrTransition *child : children) {
            Q_ASSERT(child);
            child->accept(this);
        }
    }

    void visit(const InstructionSequences &sequences)
    {
        for (InstructionSequence *sequence : sequences) {
            Q_ASSERT(sequence);
            visit(sequence);
        }
    }

    void visit(const QList<Param *> &params)
    {
        for (Param *param : params) {
            Q_ASSERT(param);
            param->accept(this);
        }
    }
};

} // DocumentModel namespace

class Q_SCXML_EXPORT QScxmlCompilerPrivate
{
public:
    static QScxmlCompilerPrivate *get(QScxmlCompiler *compiler);

    QScxmlCompilerPrivate(QXmlStreamReader *reader);

    bool verifyDocument();
    DocumentModel::ScxmlDocument *scxmlDocument() const;

    QString fileName() const;
    void setFileName(const QString &fileName);

    QScxmlCompiler::Loader *loader() const;
    void setLoader(QScxmlCompiler::Loader *loader);

    bool readDocument();
    void parseSubDocument(DocumentModel::Invoke *parentInvoke,
                          QXmlStreamReader *reader,
                          const QString &fileName);
    bool parseSubElement(DocumentModel::Invoke *parentInvoke,
                         QXmlStreamReader *reader,
                         const QString &fileName);
    QByteArray load(const QString &name, bool *ok);

    QList<QScxmlError> errors() const;

    void addError(const QString &msg);
    void addError(const QString &fileName, const DocumentModel::XmlLocation &location, const QString &msg);
    QScxmlStateMachine *instantiateStateMachine() const;
    void instantiateDataModel(QScxmlStateMachine *stateMachine) const;

private:
    DocumentModel::AbstractState *currentParent() const;
    DocumentModel::XmlLocation xmlLocation() const;
    bool maybeId(const QXmlStreamAttributes &attributes, QString *id);
    DocumentModel::If *lastIf();
    bool checkAttributes(const QXmlStreamAttributes &attributes,
                         const QStringList &requiredNames,
                         const QStringList &optionalNames);

    bool preReadElementScxml();
    bool preReadElementState();
    bool preReadElementParallel();
    bool preReadElementInitial();
    bool preReadElementTransition();
    bool preReadElementFinal();
    bool preReadElementHistory();
    bool preReadElementOnEntry();
    bool preReadElementOnExit();
    bool preReadElementRaise();
    bool preReadElementIf();
    bool preReadElementElseIf();
    bool preReadElementElse();
    bool preReadElementForeach();
    bool preReadElementLog();
    bool preReadElementDataModel();
    bool preReadElementData();
    bool preReadElementAssign();
    bool preReadElementDoneData();
    bool preReadElementContent();
    bool preReadElementParam();
    bool preReadElementScript();
    bool preReadElementSend();
    bool preReadElementCancel();
    bool preReadElementInvoke();
    bool preReadElementFinalize();

    bool postReadElementScxml();
    bool postReadElementState();
    bool postReadElementParallel();
    bool postReadElementInitial();
    bool postReadElementTransition();
    bool postReadElementFinal();
    bool postReadElementHistory();
    bool postReadElementOnEntry();
    bool postReadElementOnExit();
    bool postReadElementRaise();
    bool postReadElementIf();
    bool postReadElementElseIf();
    bool postReadElementElse();
    bool postReadElementForeach();
    bool postReadElementLog();
    bool postReadElementDataModel();
    bool postReadElementData();
    bool postReadElementAssign();
    bool postReadElementDoneData();
    bool postReadElementContent();
    bool postReadElementParam();
    bool postReadElementScript();
    bool postReadElementSend();
    bool postReadElementCancel();
    bool postReadElementInvoke();
    bool postReadElementFinalize();

    bool readElement();

    void resetDocument();
    void currentStateUp();
    bool flushInstruction();

private:
    struct ParserState {
        enum Kind {
            Scxml,
            State,
            Parallel,
            Transition,
            Initial,
            Final,
            OnEntry,
            OnExit,
            History,
            Raise,
            If,
            ElseIf,
            Else,
            Foreach,
            Log,
            DataModel,
            Data,
            Assign,
            DoneData,
            Content,
            Param,
            Script,
            Send,
            Cancel,
            Invoke,
            Finalize,
            None
        };
        Kind kind;
        QString chars;
        DocumentModel::Instruction *instruction;
        DocumentModel::InstructionSequence *instructionContainer;

        bool collectChars();

        ParserState(Kind someKind = None);
        ~ParserState() { }

        bool validChild(ParserState::Kind child) const;
        static bool validChild(ParserState::Kind parent, ParserState::Kind child);
        static bool isExecutableContent(ParserState::Kind kind);
        static Kind nameToParserStateKind(QStringView name);
        static QStringList requiredAttributes(Kind kind);
        static QStringList optionalAttributes(Kind kind);
    };

public:
    class DefaultLoader: public QScxmlCompiler::Loader
    {
    public:
        DefaultLoader();
        QByteArray load(const QString &name,
                        const QString &baseDir,
                        QStringList *errors) override final;
    };

private:
    bool checkAttributes(const QXmlStreamAttributes &attributes, QScxmlCompilerPrivate::ParserState::Kind kind);
    ParserState &current();
    ParserState &previous();
    bool hasPrevious() const;

private:
    QString m_fileName;
    QSet<QString> m_allIds;

    std::unique_ptr<DocumentModel::ScxmlDocument> m_doc;
    DocumentModel::StateContainer *m_currentState;
    DefaultLoader m_defaultLoader;
    QScxmlCompiler::Loader *m_loader;

    QXmlStreamReader *m_reader;
    QList<ParserState> m_stack;
    QList<QScxmlError> m_errors;
};

QT_END_NAMESPACE

#endif // QSCXMLCOMPILER_P_H
