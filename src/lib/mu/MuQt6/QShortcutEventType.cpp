//*****************************************************************************
// Copyright (c) 2024 Autodesk, Inc.
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//
//*****************************************************************************

// IMPORTANT: This file (not the template) is auto-generated by qt2mu.py script.
//            The prefered way to do a fix is to handrolled it or modify the
//            qt2mu.py script. If it is not possible, manual editing is ok but
//            it could be lost in future generations.

#include <MuQt6/qtUtils.h>
#include <MuQt6/QShortcutEventType.h>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include <QtSvg/QtSvg>
#include <QSvgWidget>
#include <QtNetwork/QtNetwork>
#include <MuQt6/QWidgetType.h>
#include <MuQt6/QActionType.h>
#include <MuQt6/QIconType.h>
#include <Mu/BaseFunctions.h>
#include <Mu/Alias.h>
#include <Mu/SymbolicConstant.h>
#include <Mu/Thread.h>
#include <Mu/ClassInstance.h>
#include <Mu/Function.h>
#include <Mu/MemberFunction.h>
#include <Mu/MemberVariable.h>
#include <Mu/Node.h>
#include <Mu/Exception.h>
#include <Mu/ParameterVariable.h>
#include <Mu/ReferenceType.h>
#include <Mu/Value.h>
#include <MuLang/MuLangContext.h>
#include <MuLang/StringType.h>
#include <MuQt6/QKeySequenceType.h>

namespace Mu
{
    using namespace std;

    QShortcutEventType::QShortcutEventType(Context* c, const char* name,
                                           Class* super)
        : Class(c, name, super)
    {
    }

    QShortcutEventType::~QShortcutEventType() {}

    //----------------------------------------------------------------------
    //  PRE-COMPILED FUNCTIONS

    bool
    qt_QShortcutEvent_isAmbiguous_bool_QShortcutEvent(Mu::Thread& NODE_THREAD,
                                                      Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QShortcutEvent* arg0 = getqpointer<QShortcutEventType>(param_this);
        return arg0->isAmbiguous();
    }

    Pointer
    qt_QShortcutEvent_key_QKeySequence_QShortcutEvent(Mu::Thread& NODE_THREAD,
                                                      Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QShortcutEvent* arg0 = getqpointer<QShortcutEventType>(param_this);
        return makeqtype<QKeySequenceType>(c, arg0->key(), "qt.QKeySequence");
    }

    static NODE_IMPLEMENTATION(_n_isAmbiguous0, bool)
    {
        NODE_RETURN(qt_QShortcutEvent_isAmbiguous_bool_QShortcutEvent(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_key0, Pointer)
    {
        NODE_RETURN(qt_QShortcutEvent_key_QKeySequence_QShortcutEvent(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    void QShortcutEventType::load()
    {
        USING_MU_FUNCTION_SYMBOLS;
        MuLangContext* c = static_cast<MuLangContext*>(context());
        Module* global = globalModule();

        const string typeName = name();
        const string refTypeName = typeName + "&";
        const string fullTypeName = fullyQualifiedName();
        const string fullRefTypeName = fullTypeName + "&";
        const char* tn = typeName.c_str();
        const char* ftn = fullTypeName.c_str();
        const char* rtn = refTypeName.c_str();
        const char* frtn = fullRefTypeName.c_str();

        scope()->addSymbols(new ReferenceType(c, rtn, this),

                            new Function(c, tn, BaseFunctions::dereference,
                                         Cast, Return, ftn, Args, frtn, End),

                            EndArguments);

        addSymbols(new Function(c, "__allocate", BaseFunctions::classAllocate,
                                None, Return, ftn, End),

                   new MemberVariable(c, "native", "qt.NativeObject"),

                   EndArguments);

        addSymbols(EndArguments);

        addSymbols(
            // enums
            // member functions
            // MISSING: QShortcutEvent (QShortcutEvent; QShortcutEvent this,
            // QKeySequence key, "const QShortcut *" shortcut, bool ambiguous)
            new Function(c, "isAmbiguous", _n_isAmbiguous0, None, Compiled,
                         qt_QShortcutEvent_isAmbiguous_bool_QShortcutEvent,
                         Return, "bool", Parameters,
                         new Param(c, "this", "qt.QShortcutEvent"), End),
            new Function(c, "key", _n_key0, None, Compiled,
                         qt_QShortcutEvent_key_QKeySequence_QShortcutEvent,
                         Return, "qt.QKeySequence", Parameters,
                         new Param(c, "this", "qt.QShortcutEvent"), End),
            // static functions
            EndArguments);
        globalScope()->addSymbols(EndArguments);
        scope()->addSymbols(EndArguments);
    }

} // namespace Mu
