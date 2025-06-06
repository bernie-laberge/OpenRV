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
#include <MuQt6/QToolBoxType.h>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include <QtSvg/QtSvg>
#include <QSvgWidget>
#include <QtNetwork/QtNetwork>
#include <MuQt6/QWidgetType.h>
#include <MuQt6/QActionType.h>
#include <MuQt6/QIconType.h>
#include <Mu/BaseFunctions.h>
#include <Mu/Thread.h>
#include <Mu/Alias.h>
#include <Mu/SymbolicConstant.h>
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
#include <MuQt6/QSizeType.h>
#include <MuQt6/QEventType.h>
#include <MuQt6/QPaintEventType.h>
#include <MuQt6/QWidgetType.h>
#include <MuQt6/QIconType.h>
#include <MuQt6/QShowEventType.h>

namespace Mu
{
    using namespace std;

    //----------------------------------------------------------------------
    //  INHERITABLE TYPE IMPLEMENTATION

    // destructor
    MuQt_QToolBox::~MuQt_QToolBox()
    {
        if (_obj)
        {
            *_obj->data<Pointer>() = Pointer(0);
            _obj->releaseExternal();
        }
        _obj = 0;
        _env = 0;
        _baseType = 0;
    }

    MuQt_QToolBox::MuQt_QToolBox(Pointer muobj, const CallEnvironment* ce,
                                 QWidget* parent, Qt::WindowFlags f)
        : QToolBox(parent, f)
    {
        _env = ce;
        _obj = reinterpret_cast<ClassInstance*>(muobj);
        _obj->retainExternal();
        MuLangContext* c = (MuLangContext*)_env->context();
        _baseType = c->findSymbolOfTypeByQualifiedName<QToolBoxType>(
            c->internName("qt.QToolBox"));
    }

    void MuQt_QToolBox::itemInserted(int index)
    {
        if (!_env)
        {
            QToolBox::itemInserted(index);
            return;
        }
        MuLangContext* c = (MuLangContext*)_env->context();
        const MemberFunction* F0 = _baseType->_func[0];
        const MemberFunction* F = _obj->classType()->dynamicLookup(F0);
        if (F != F0)
        {
            Function::ArgumentVector args(2);
            args[0] = Value(Pointer(_obj));
            args[1] = Value(index);
            Value rval = _env->call(F, args);
        }
        else
        {
            QToolBox::itemInserted(index);
        }
    }

    void MuQt_QToolBox::itemRemoved(int index)
    {
        if (!_env)
        {
            QToolBox::itemRemoved(index);
            return;
        }
        MuLangContext* c = (MuLangContext*)_env->context();
        const MemberFunction* F0 = _baseType->_func[1];
        const MemberFunction* F = _obj->classType()->dynamicLookup(F0);
        if (F != F0)
        {
            Function::ArgumentVector args(2);
            args[0] = Value(Pointer(_obj));
            args[1] = Value(index);
            Value rval = _env->call(F, args);
        }
        else
        {
            QToolBox::itemRemoved(index);
        }
    }

    void MuQt_QToolBox::changeEvent(QEvent* ev)
    {
        if (!_env)
        {
            QToolBox::changeEvent(ev);
            return;
        }
        MuLangContext* c = (MuLangContext*)_env->context();
        const MemberFunction* F0 = _baseType->_func[2];
        const MemberFunction* F = _obj->classType()->dynamicLookup(F0);
        if (F != F0)
        {
            Function::ArgumentVector args(2);
            args[0] = Value(Pointer(_obj));
            args[1] = Value(makeqpointer<QEventType>(c, ev, "qt.QEvent"));
            Value rval = _env->call(F, args);
        }
        else
        {
            QToolBox::changeEvent(ev);
        }
    }

    bool MuQt_QToolBox::event(QEvent* e)
    {
        if (!_env)
            return QToolBox::event(e);
        MuLangContext* c = (MuLangContext*)_env->context();
        const MemberFunction* F0 = _baseType->_func[3];
        const MemberFunction* F = _obj->classType()->dynamicLookup(F0);
        if (F != F0)
        {
            Function::ArgumentVector args(2);
            args[0] = Value(Pointer(_obj));
            args[1] = Value(makeqpointer<QEventType>(c, e, "qt.QEvent"));
            Value rval = _env->call(F, args);
            return (bool)(rval._bool);
        }
        else
        {
            return QToolBox::event(e);
        }
    }

    void MuQt_QToolBox::showEvent(QShowEvent* e)
    {
        if (!_env)
        {
            QToolBox::showEvent(e);
            return;
        }
        MuLangContext* c = (MuLangContext*)_env->context();
        const MemberFunction* F0 = _baseType->_func[4];
        const MemberFunction* F = _obj->classType()->dynamicLookup(F0);
        if (F != F0)
        {
            Function::ArgumentVector args(2);
            args[0] = Value(Pointer(_obj));
            args[1] =
                Value(makeqpointer<QShowEventType>(c, e, "qt.QShowEvent"));
            Value rval = _env->call(F, args);
        }
        else
        {
            QToolBox::showEvent(e);
        }
    }

    QSize MuQt_QToolBox::sizeHint() const
    {
        if (!_env)
            return QToolBox::sizeHint();
        MuLangContext* c = (MuLangContext*)_env->context();
        const MemberFunction* F0 = _baseType->_func[5];
        const MemberFunction* F = _obj->classType()->dynamicLookup(F0);
        if (F != F0)
        {
            Function::ArgumentVector args(1);
            args[0] = Value(Pointer(_obj));
            Value rval = _env->call(F, args);
            return getqtype<QSizeType>(rval._Pointer);
        }
        else
        {
            return QToolBox::sizeHint();
        }
    }

    void MuQt_QToolBox::paintEvent(QPaintEvent* _p14)
    {
        if (!_env)
        {
            QToolBox::paintEvent(_p14);
            return;
        }
        MuLangContext* c = (MuLangContext*)_env->context();
        const MemberFunction* F0 = _baseType->_func[6];
        const MemberFunction* F = _obj->classType()->dynamicLookup(F0);
        if (F != F0)
        {
            Function::ArgumentVector args(2);
            args[0] = Value(Pointer(_obj));
            args[1] =
                Value(makeqpointer<QPaintEventType>(c, _p14, "qt.QPaintEvent"));
            Value rval = _env->call(F, args);
        }
        else
        {
            QToolBox::paintEvent(_p14);
        }
    }

    //----------------------------------------------------------------------
    //  Mu Type CONSTRUCTORS

    QToolBoxType::QToolBoxType(Context* c, const char* name, Class* super,
                               Class* super2)
        : Class(c, name, vectorOf2(super, super2))
    {
    }

    QToolBoxType::~QToolBoxType() {}

    //----------------------------------------------------------------------
    //  PRE-COMPILED FUNCTIONS

    static Pointer QToolBox_QToolBox_QObject(Thread& NODE_THREAD, Pointer obj)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        ClassInstance* widget = reinterpret_cast<ClassInstance*>(obj);

        if (!widget)
        {
            return 0;
        }
        else if (QToolBox* w = object<QToolBox>(widget))
        {
            QToolBoxType* type =
                c->findSymbolOfTypeByQualifiedName<QToolBoxType>(
                    c->internName("qt.QToolBox"), false);
            ClassInstance* o = ClassInstance::allocate(type);
            setobject(o, w);
            return o;
        }
        else
        {
            throw BadCastException();
        }
    }

    static NODE_IMPLEMENTATION(castFromObject, Pointer)
    {
        NODE_RETURN(
            QToolBox_QToolBox_QObject(NODE_THREAD, NODE_ARG(0, Pointer)));
    }

    Pointer qt_QToolBox_QToolBox_QToolBox_QToolBox_QWidget_int(
        Mu::Thread& NODE_THREAD, Pointer param_this, Pointer param_parent,
        int param_f)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QWidget* arg1 = object<QWidget>(param_parent);
        Qt::WindowFlags arg2 = (Qt::WindowFlags)(param_f);
        setobject(param_this, new MuQt_QToolBox(
                                  param_this, NODE_THREAD.process()->callEnv(),
                                  arg1, arg2));
        return param_this;
    }

    int qt_QToolBox_addItem_int_QToolBox_QWidget_QIcon_string(
        Mu::Thread& NODE_THREAD, Pointer param_this, Pointer param_widget,
        Pointer param_iconSet, Pointer param_text)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        QWidget* arg1 = object<QWidget>(param_widget);
        const QIcon arg2 = getqtype<QIconType>(param_iconSet);
        const QString arg3 = qstring(param_text);
        return arg0->addItem(arg1, arg2, arg3);
    }

    int qt_QToolBox_addItem_int_QToolBox_QWidget_string(Mu::Thread& NODE_THREAD,
                                                        Pointer param_this,
                                                        Pointer param_w,
                                                        Pointer param_text)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        QWidget* arg1 = object<QWidget>(param_w);
        const QString arg2 = qstring(param_text);
        return arg0->addItem(arg1, arg2);
    }

    Pointer qt_QToolBox_currentWidget_QWidget_QToolBox(Mu::Thread& NODE_THREAD,
                                                       Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        return makeinstance<QWidgetType>(c, arg0->currentWidget(),
                                         "qt.QWidget");
    }

    int qt_QToolBox_indexOf_int_QToolBox_QWidget(Mu::Thread& NODE_THREAD,
                                                 Pointer param_this,
                                                 Pointer param_widget)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        const QWidget* arg1 = object<QWidget>(param_widget);
        return arg0->indexOf(arg1);
    }

    int qt_QToolBox_insertItem_int_QToolBox_int_QWidget_QIcon_string(
        Mu::Thread& NODE_THREAD, Pointer param_this, int param_index,
        Pointer param_widget, Pointer param_icon, Pointer param_text)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        QWidget* arg2 = object<QWidget>(param_widget);
        const QIcon arg3 = getqtype<QIconType>(param_icon);
        const QString arg4 = qstring(param_text);
        return arg0->insertItem(arg1, arg2, arg3, arg4);
    }

    int qt_QToolBox_insertItem_int_QToolBox_int_QWidget_string(
        Mu::Thread& NODE_THREAD, Pointer param_this, int param_index,
        Pointer param_widget, Pointer param_text)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        QWidget* arg2 = object<QWidget>(param_widget);
        const QString arg3 = qstring(param_text);
        return arg0->insertItem(arg1, arg2, arg3);
    }

    bool qt_QToolBox_isItemEnabled_bool_QToolBox_int(Mu::Thread& NODE_THREAD,
                                                     Pointer param_this,
                                                     int param_index)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        return arg0->isItemEnabled(arg1);
    }

    Pointer qt_QToolBox_itemIcon_QIcon_QToolBox_int(Mu::Thread& NODE_THREAD,
                                                    Pointer param_this,
                                                    int param_index)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        return makeqtype<QIconType>(c, arg0->itemIcon(arg1), "qt.QIcon");
    }

    Pointer qt_QToolBox_itemText_string_QToolBox_int(Mu::Thread& NODE_THREAD,
                                                     Pointer param_this,
                                                     int param_index)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        return makestring(c, arg0->itemText(arg1));
    }

    Pointer qt_QToolBox_itemToolTip_string_QToolBox_int(Mu::Thread& NODE_THREAD,
                                                        Pointer param_this,
                                                        int param_index)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        return makestring(c, arg0->itemToolTip(arg1));
    }

    void qt_QToolBox_removeItem_void_QToolBox_int(Mu::Thread& NODE_THREAD,
                                                  Pointer param_this,
                                                  int param_index)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        arg0->removeItem(arg1);
    }

    void qt_QToolBox_setItemEnabled_void_QToolBox_int_bool(
        Mu::Thread& NODE_THREAD, Pointer param_this, int param_index,
        bool param_enabled)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        bool arg2 = (bool)(param_enabled);
        arg0->setItemEnabled(arg1, arg2);
    }

    void qt_QToolBox_setItemIcon_void_QToolBox_int_QIcon(
        Mu::Thread& NODE_THREAD, Pointer param_this, int param_index,
        Pointer param_icon)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        const QIcon arg2 = getqtype<QIconType>(param_icon);
        arg0->setItemIcon(arg1, arg2);
    }

    void qt_QToolBox_setItemText_void_QToolBox_int_string(
        Mu::Thread& NODE_THREAD, Pointer param_this, int param_index,
        Pointer param_text)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        const QString arg2 = qstring(param_text);
        arg0->setItemText(arg1, arg2);
    }

    void qt_QToolBox_setItemToolTip_void_QToolBox_int_string(
        Mu::Thread& NODE_THREAD, Pointer param_this, int param_index,
        Pointer param_toolTip)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        const QString arg2 = qstring(param_toolTip);
        arg0->setItemToolTip(arg1, arg2);
    }

    Pointer qt_QToolBox_widget_QWidget_QToolBox_int(Mu::Thread& NODE_THREAD,
                                                    Pointer param_this,
                                                    int param_index)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        return makeinstance<QWidgetType>(c, arg0->widget(arg1), "qt.QWidget");
    }

    void qt_QToolBox_itemInserted_void_QToolBox_int(Mu::Thread& NODE_THREAD,
                                                    Pointer param_this,
                                                    int param_index)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        if (isMuQtObject(arg0))
            ((MuQt_QToolBox*)arg0)->itemInserted_pub_parent(arg1);
        else
            ((MuQt_QToolBox*)arg0)->itemInserted_pub(arg1);
    }

    void qt_QToolBox_itemRemoved_void_QToolBox_int(Mu::Thread& NODE_THREAD,
                                                   Pointer param_this,
                                                   int param_index)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        int arg1 = (int)(param_index);
        if (isMuQtObject(arg0))
            ((MuQt_QToolBox*)arg0)->itemRemoved_pub_parent(arg1);
        else
            ((MuQt_QToolBox*)arg0)->itemRemoved_pub(arg1);
    }

    void qt_QToolBox_changeEvent_void_QToolBox_QEvent(Mu::Thread& NODE_THREAD,
                                                      Pointer param_this,
                                                      Pointer param_ev)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        QEvent* arg1 = getqpointer<QEventType>(param_ev);
        if (isMuQtObject(arg0))
            ((MuQt_QToolBox*)arg0)->changeEvent_pub_parent(arg1);
        else
            ((MuQt_QToolBox*)arg0)->changeEvent_pub(arg1);
    }

    bool qt_QToolBox_event_bool_QToolBox_QEvent(Mu::Thread& NODE_THREAD,
                                                Pointer param_this,
                                                Pointer param_e)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        QEvent* arg1 = getqpointer<QEventType>(param_e);
        return isMuQtObject(arg0)
                   ? ((MuQt_QToolBox*)arg0)->event_pub_parent(arg1)
                   : ((MuQt_QToolBox*)arg0)->event_pub(arg1);
    }

    void qt_QToolBox_showEvent_void_QToolBox_QShowEvent(Mu::Thread& NODE_THREAD,
                                                        Pointer param_this,
                                                        Pointer param_e)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        QShowEvent* arg1 = getqpointer<QShowEventType>(param_e);
        if (isMuQtObject(arg0))
            ((MuQt_QToolBox*)arg0)->showEvent_pub_parent(arg1);
        else
            ((MuQt_QToolBox*)arg0)->showEvent_pub(arg1);
    }

    Pointer qt_QToolBox_sizeHint_QSize_QToolBox(Mu::Thread& NODE_THREAD,
                                                Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        return isMuQtObject(arg0)
                   ? makeqtype<QSizeType>(c, arg0->QToolBox::sizeHint(),
                                          "qt.QSize")
                   : makeqtype<QSizeType>(c, arg0->sizeHint(), "qt.QSize");
    }

    void qt_QToolBox_paintEvent_void_QToolBox_QPaintEvent(
        Mu::Thread& NODE_THREAD, Pointer param_this, Pointer param__p14)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QToolBox* arg0 = object<QToolBox>(param_this);
        QPaintEvent* arg1 = getqpointer<QPaintEventType>(param__p14);
        if (isMuQtObject(arg0))
            ((MuQt_QToolBox*)arg0)->paintEvent_pub_parent(arg1);
        else
            ((MuQt_QToolBox*)arg0)->paintEvent_pub(arg1);
    }

    static NODE_IMPLEMENTATION(_n_QToolBox0, Pointer)
    {
        NODE_RETURN(qt_QToolBox_QToolBox_QToolBox_QToolBox_QWidget_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer),
            NODE_ARG(2, int)));
    }

    static NODE_IMPLEMENTATION(_n_addItem0, int)
    {
        NODE_RETURN(qt_QToolBox_addItem_int_QToolBox_QWidget_QIcon_string(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer),
            NODE_ARG(2, Pointer), NODE_ARG(3, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_addItem1, int)
    {
        NODE_RETURN(qt_QToolBox_addItem_int_QToolBox_QWidget_string(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer),
            NODE_ARG(2, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_currentWidget0, Pointer)
    {
        NODE_RETURN(qt_QToolBox_currentWidget_QWidget_QToolBox(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_indexOf0, int)
    {
        NODE_RETURN(qt_QToolBox_indexOf_int_QToolBox_QWidget(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_insertItem0, int)
    {
        NODE_RETURN(
            qt_QToolBox_insertItem_int_QToolBox_int_QWidget_QIcon_string(
                NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int),
                NODE_ARG(2, Pointer), NODE_ARG(3, Pointer),
                NODE_ARG(4, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_insertItem1, int)
    {
        NODE_RETURN(qt_QToolBox_insertItem_int_QToolBox_int_QWidget_string(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int),
            NODE_ARG(2, Pointer), NODE_ARG(3, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_isItemEnabled0, bool)
    {
        NODE_RETURN(qt_QToolBox_isItemEnabled_bool_QToolBox_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int)));
    }

    static NODE_IMPLEMENTATION(_n_itemIcon0, Pointer)
    {
        NODE_RETURN(qt_QToolBox_itemIcon_QIcon_QToolBox_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int)));
    }

    static NODE_IMPLEMENTATION(_n_itemText0, Pointer)
    {
        NODE_RETURN(qt_QToolBox_itemText_string_QToolBox_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int)));
    }

    static NODE_IMPLEMENTATION(_n_itemToolTip0, Pointer)
    {
        NODE_RETURN(qt_QToolBox_itemToolTip_string_QToolBox_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int)));
    }

    static NODE_IMPLEMENTATION(_n_removeItem0, void)
    {
        qt_QToolBox_removeItem_void_QToolBox_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int));
    }

    static NODE_IMPLEMENTATION(_n_setItemEnabled0, void)
    {
        qt_QToolBox_setItemEnabled_void_QToolBox_int_bool(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int),
            NODE_ARG(2, bool));
    }

    static NODE_IMPLEMENTATION(_n_setItemIcon0, void)
    {
        qt_QToolBox_setItemIcon_void_QToolBox_int_QIcon(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int),
            NODE_ARG(2, Pointer));
    }

    static NODE_IMPLEMENTATION(_n_setItemText0, void)
    {
        qt_QToolBox_setItemText_void_QToolBox_int_string(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int),
            NODE_ARG(2, Pointer));
    }

    static NODE_IMPLEMENTATION(_n_setItemToolTip0, void)
    {
        qt_QToolBox_setItemToolTip_void_QToolBox_int_string(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int),
            NODE_ARG(2, Pointer));
    }

    static NODE_IMPLEMENTATION(_n_widget0, Pointer)
    {
        NODE_RETURN(qt_QToolBox_widget_QWidget_QToolBox_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int)));
    }

    static NODE_IMPLEMENTATION(_n_itemInserted0, void)
    {
        qt_QToolBox_itemInserted_void_QToolBox_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int));
    }

    static NODE_IMPLEMENTATION(_n_itemRemoved0, void)
    {
        qt_QToolBox_itemRemoved_void_QToolBox_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int));
    }

    static NODE_IMPLEMENTATION(_n_changeEvent0, void)
    {
        qt_QToolBox_changeEvent_void_QToolBox_QEvent(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer));
    }

    static NODE_IMPLEMENTATION(_n_event0, bool)
    {
        NODE_RETURN(qt_QToolBox_event_bool_QToolBox_QEvent(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_showEvent0, void)
    {
        qt_QToolBox_showEvent_void_QToolBox_QShowEvent(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer));
    }

    static NODE_IMPLEMENTATION(_n_sizeHint0, Pointer)
    {
        NODE_RETURN(qt_QToolBox_sizeHint_QSize_QToolBox(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_paintEvent0, void)
    {
        qt_QToolBox_paintEvent_void_QToolBox_QPaintEvent(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer));
    }

    void QToolBoxType::load()
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

                   new Function(c, tn, castFromObject, Cast, Compiled,
                                QToolBox_QToolBox_QObject, Return, ftn,
                                Parameters,
                                new Param(c, "object", "qt.QObject"), End),

                   EndArguments);

        addSymbols(
            // enums
            // member functions
            new Function(
                c, "QToolBox", _n_QToolBox0, None, Compiled,
                qt_QToolBox_QToolBox_QToolBox_QToolBox_QWidget_int, Return,
                "qt.QToolBox", Parameters, new Param(c, "this", "qt.QToolBox"),
                new Param(c, "parent", "qt.QWidget"),
                new Param(c, "f", "int", Value((int)Qt::WindowFlags())), End),
            new Function(c, "addItem", _n_addItem0, None, Compiled,
                         qt_QToolBox_addItem_int_QToolBox_QWidget_QIcon_string,
                         Return, "int", Parameters,
                         new Param(c, "this", "qt.QToolBox"),
                         new Param(c, "widget", "qt.QWidget"),
                         new Param(c, "iconSet", "qt.QIcon"),
                         new Param(c, "text", "string"), End),
            new Function(c, "addItem", _n_addItem1, None, Compiled,
                         qt_QToolBox_addItem_int_QToolBox_QWidget_string,
                         Return, "int", Parameters,
                         new Param(c, "this", "qt.QToolBox"),
                         new Param(c, "w", "qt.QWidget"),
                         new Param(c, "text", "string"), End),
            // PROP: count (int; QToolBox this)
            // PROP: currentIndex (int; QToolBox this)
            new Function(c, "currentWidget", _n_currentWidget0, None, Compiled,
                         qt_QToolBox_currentWidget_QWidget_QToolBox, Return,
                         "qt.QWidget", Parameters,
                         new Param(c, "this", "qt.QToolBox"), End),
            new Function(c, "indexOf", _n_indexOf0, None, Compiled,
                         qt_QToolBox_indexOf_int_QToolBox_QWidget, Return,
                         "int", Parameters, new Param(c, "this", "qt.QToolBox"),
                         new Param(c, "widget", "qt.QWidget"), End),
            new Function(
                c, "insertItem", _n_insertItem0, None, Compiled,
                qt_QToolBox_insertItem_int_QToolBox_int_QWidget_QIcon_string,
                Return, "int", Parameters, new Param(c, "this", "qt.QToolBox"),
                new Param(c, "index", "int"),
                new Param(c, "widget", "qt.QWidget"),
                new Param(c, "icon", "qt.QIcon"),
                new Param(c, "text", "string"), End),
            new Function(c, "insertItem", _n_insertItem1, None, Compiled,
                         qt_QToolBox_insertItem_int_QToolBox_int_QWidget_string,
                         Return, "int", Parameters,
                         new Param(c, "this", "qt.QToolBox"),
                         new Param(c, "index", "int"),
                         new Param(c, "widget", "qt.QWidget"),
                         new Param(c, "text", "string"), End),
            new Function(c, "isItemEnabled", _n_isItemEnabled0, None, Compiled,
                         qt_QToolBox_isItemEnabled_bool_QToolBox_int, Return,
                         "bool", Parameters,
                         new Param(c, "this", "qt.QToolBox"),
                         new Param(c, "index", "int"), End),
            new Function(c, "itemIcon", _n_itemIcon0, None, Compiled,
                         qt_QToolBox_itemIcon_QIcon_QToolBox_int, Return,
                         "qt.QIcon", Parameters,
                         new Param(c, "this", "qt.QToolBox"),
                         new Param(c, "index", "int"), End),
            new Function(c, "itemText", _n_itemText0, None, Compiled,
                         qt_QToolBox_itemText_string_QToolBox_int, Return,
                         "string", Parameters,
                         new Param(c, "this", "qt.QToolBox"),
                         new Param(c, "index", "int"), End),
            new Function(c, "itemToolTip", _n_itemToolTip0, None, Compiled,
                         qt_QToolBox_itemToolTip_string_QToolBox_int, Return,
                         "string", Parameters,
                         new Param(c, "this", "qt.QToolBox"),
                         new Param(c, "index", "int"), End),
            new Function(c, "removeItem", _n_removeItem0, None, Compiled,
                         qt_QToolBox_removeItem_void_QToolBox_int, Return,
                         "void", Parameters,
                         new Param(c, "this", "qt.QToolBox"),
                         new Param(c, "index", "int"), End),
            new Function(
                c, "setItemEnabled", _n_setItemEnabled0, None, Compiled,
                qt_QToolBox_setItemEnabled_void_QToolBox_int_bool, Return,
                "void", Parameters, new Param(c, "this", "qt.QToolBox"),
                new Param(c, "index", "int"), new Param(c, "enabled", "bool"),
                End),
            new Function(c, "setItemIcon", _n_setItemIcon0, None, Compiled,
                         qt_QToolBox_setItemIcon_void_QToolBox_int_QIcon,
                         Return, "void", Parameters,
                         new Param(c, "this", "qt.QToolBox"),
                         new Param(c, "index", "int"),
                         new Param(c, "icon", "qt.QIcon"), End),
            new Function(c, "setItemText", _n_setItemText0, None, Compiled,
                         qt_QToolBox_setItemText_void_QToolBox_int_string,
                         Return, "void", Parameters,
                         new Param(c, "this", "qt.QToolBox"),
                         new Param(c, "index", "int"),
                         new Param(c, "text", "string"), End),
            new Function(
                c, "setItemToolTip", _n_setItemToolTip0, None, Compiled,
                qt_QToolBox_setItemToolTip_void_QToolBox_int_string, Return,
                "void", Parameters, new Param(c, "this", "qt.QToolBox"),
                new Param(c, "index", "int"), new Param(c, "toolTip", "string"),
                End),
            new Function(c, "widget", _n_widget0, None, Compiled,
                         qt_QToolBox_widget_QWidget_QToolBox_int, Return,
                         "qt.QWidget", Parameters,
                         new Param(c, "this", "qt.QToolBox"),
                         new Param(c, "index", "int"), End),
            _func[0] = new MemberFunction(
                c, "itemInserted", _n_itemInserted0, None, Compiled,
                qt_QToolBox_itemInserted_void_QToolBox_int, Return, "void",
                Parameters, new Param(c, "this", "qt.QToolBox"),
                new Param(c, "index", "int"), End),
            _func[1] = new MemberFunction(
                c, "itemRemoved", _n_itemRemoved0, None, Compiled,
                qt_QToolBox_itemRemoved_void_QToolBox_int, Return, "void",
                Parameters, new Param(c, "this", "qt.QToolBox"),
                new Param(c, "index", "int"), End),
            _func[2] = new MemberFunction(
                c, "changeEvent", _n_changeEvent0, None, Compiled,
                qt_QToolBox_changeEvent_void_QToolBox_QEvent, Return, "void",
                Parameters, new Param(c, "this", "qt.QToolBox"),
                new Param(c, "ev", "qt.QEvent"), End),
            _func[3] = new MemberFunction(
                c, "event", _n_event0, None, Compiled,
                qt_QToolBox_event_bool_QToolBox_QEvent, Return, "bool",
                Parameters, new Param(c, "this", "qt.QToolBox"),
                new Param(c, "e", "qt.QEvent"), End),
            _func[4] = new MemberFunction(
                c, "showEvent", _n_showEvent0, None, Compiled,
                qt_QToolBox_showEvent_void_QToolBox_QShowEvent, Return, "void",
                Parameters, new Param(c, "this", "qt.QToolBox"),
                new Param(c, "e", "qt.QShowEvent"), End),
            _func[5] = new MemberFunction(
                c, "sizeHint", _n_sizeHint0, None, Compiled,
                qt_QToolBox_sizeHint_QSize_QToolBox, Return, "qt.QSize",
                Parameters, new Param(c, "this", "qt.QToolBox"), End),
            // MISSING: initStyleOption (void; QToolBox this, "QStyleOptionFrame
            // *" option) // protected
            _func[6] = new MemberFunction(
                c, "paintEvent", _n_paintEvent0, None, Compiled,
                qt_QToolBox_paintEvent_void_QToolBox_QPaintEvent, Return,
                "void", Parameters, new Param(c, "this", "qt.QToolBox"),
                new Param(c, "_p14", "qt.QPaintEvent"), End),
            // static functions
            EndArguments);
        globalScope()->addSymbols(EndArguments);
        scope()->addSymbols(EndArguments);

        const char** propExclusions = 0;

        populate(this, QToolBox::staticMetaObject, propExclusions);
    }

} // namespace Mu
