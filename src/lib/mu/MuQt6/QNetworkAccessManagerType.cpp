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
#include <MuQt6/QNetworkAccessManagerType.h>
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
#include <MuQt6/QTimerEventType.h>
#include <MuQt6/QObjectType.h>
#include <MuQt6/QNetworkCookieJarType.h>
#include <MuQt6/QEventType.h>

namespace Mu
{
    using namespace std;

    //----------------------------------------------------------------------
    //  INHERITABLE TYPE IMPLEMENTATION

    // destructor
    MuQt_QNetworkAccessManager::~MuQt_QNetworkAccessManager()
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

    MuQt_QNetworkAccessManager::MuQt_QNetworkAccessManager(
        Pointer muobj, const CallEnvironment* ce, QObject* parent)
        : QNetworkAccessManager(parent)
    {
        _env = ce;
        _obj = reinterpret_cast<ClassInstance*>(muobj);
        _obj->retainExternal();
        MuLangContext* c = (MuLangContext*)_env->context();
        _baseType =
            c->findSymbolOfTypeByQualifiedName<QNetworkAccessManagerType>(
                c->internName("qt.QNetworkAccessManager"));
    }

    QStringList MuQt_QNetworkAccessManager::supportedSchemes() const
    {
        if (!_env)
            return QNetworkAccessManager::supportedSchemes();
        MuLangContext* c = (MuLangContext*)_env->context();
        const MemberFunction* F0 = _baseType->_func[0];
        const MemberFunction* F = _obj->classType()->dynamicLookup(F0);
        if (F != F0)
        {
            Function::ArgumentVector args(1);
            args[0] = Value(Pointer(_obj));
            Value rval = _env->call(F, args);
            return qstringlist(rval._Pointer);
        }
        else
        {
            return QNetworkAccessManager::supportedSchemes();
        }
    }

    bool MuQt_QNetworkAccessManager::event(QEvent* e)
    {
        if (!_env)
            return QNetworkAccessManager::event(e);
        MuLangContext* c = (MuLangContext*)_env->context();
        const MemberFunction* F0 = _baseType->_func[1];
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
            return QNetworkAccessManager::event(e);
        }
    }

    bool MuQt_QNetworkAccessManager::eventFilter(QObject* watched,
                                                 QEvent* event)
    {
        if (!_env)
            return QNetworkAccessManager::eventFilter(watched, event);
        MuLangContext* c = (MuLangContext*)_env->context();
        const MemberFunction* F0 = _baseType->_func[2];
        const MemberFunction* F = _obj->classType()->dynamicLookup(F0);
        if (F != F0)
        {
            Function::ArgumentVector args(3);
            args[0] = Value(Pointer(_obj));
            args[1] =
                Value(makeinstance<QObjectType>(c, watched, "qt.QObject"));
            args[2] = Value(makeqpointer<QEventType>(c, event, "qt.QEvent"));
            Value rval = _env->call(F, args);
            return (bool)(rval._bool);
        }
        else
        {
            return QNetworkAccessManager::eventFilter(watched, event);
        }
    }

    void MuQt_QNetworkAccessManager::customEvent(QEvent* event)
    {
        if (!_env)
        {
            QNetworkAccessManager::customEvent(event);
            return;
        }
        MuLangContext* c = (MuLangContext*)_env->context();
        const MemberFunction* F0 = _baseType->_func[3];
        const MemberFunction* F = _obj->classType()->dynamicLookup(F0);
        if (F != F0)
        {
            Function::ArgumentVector args(2);
            args[0] = Value(Pointer(_obj));
            args[1] = Value(makeqpointer<QEventType>(c, event, "qt.QEvent"));
            Value rval = _env->call(F, args);
        }
        else
        {
            QNetworkAccessManager::customEvent(event);
        }
    }

    void MuQt_QNetworkAccessManager::timerEvent(QTimerEvent* event)
    {
        if (!_env)
        {
            QNetworkAccessManager::timerEvent(event);
            return;
        }
        MuLangContext* c = (MuLangContext*)_env->context();
        const MemberFunction* F0 = _baseType->_func[4];
        const MemberFunction* F = _obj->classType()->dynamicLookup(F0);
        if (F != F0)
        {
            Function::ArgumentVector args(2);
            args[0] = Value(Pointer(_obj));
            args[1] = Value(
                makeqpointer<QTimerEventType>(c, event, "qt.QTimerEvent"));
            Value rval = _env->call(F, args);
        }
        else
        {
            QNetworkAccessManager::timerEvent(event);
        }
    }

    //----------------------------------------------------------------------
    //  Mu Type CONSTRUCTORS

    QNetworkAccessManagerType::QNetworkAccessManagerType(Context* c,
                                                         const char* name,
                                                         Class* super,
                                                         Class* super2)
        : Class(c, name, vectorOf2(super, super2))
    {
    }

    QNetworkAccessManagerType::~QNetworkAccessManagerType() {}

    //----------------------------------------------------------------------
    //  PRE-COMPILED FUNCTIONS

    static Pointer
    QNetworkAccessManager_QNetworkAccessManager_QObject(Thread& NODE_THREAD,
                                                        Pointer obj)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        ClassInstance* widget = reinterpret_cast<ClassInstance*>(obj);

        if (!widget)
        {
            return 0;
        }
        else if (QNetworkAccessManager* w =
                     object<QNetworkAccessManager>(widget))
        {
            QNetworkAccessManagerType* type =
                c->findSymbolOfTypeByQualifiedName<QNetworkAccessManagerType>(
                    c->internName("qt.QNetworkAccessManager"), false);
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
        NODE_RETURN(QNetworkAccessManager_QNetworkAccessManager_QObject(
            NODE_THREAD, NODE_ARG(0, Pointer)));
    }

    Pointer
    qt_QNetworkAccessManager_QNetworkAccessManager_QNetworkAccessManager_QNetworkAccessManager_QObject(
        Mu::Thread& NODE_THREAD, Pointer param_this, Pointer param_parent)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QObject* arg1 = object<QObject>(param_parent);
        setobject(param_this,
                  new MuQt_QNetworkAccessManager(
                      param_this, NODE_THREAD.process()->callEnv(), arg1));
        return param_this;
    }

    bool qt_QNetworkAccessManager_autoDeleteReplies_bool_QNetworkAccessManager(
        Mu::Thread& NODE_THREAD, Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        return arg0->autoDeleteReplies();
    }

    void qt_QNetworkAccessManager_clearAccessCache_void_QNetworkAccessManager(
        Mu::Thread& NODE_THREAD, Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        arg0->clearAccessCache();
    }

    void
    qt_QNetworkAccessManager_clearConnectionCache_void_QNetworkAccessManager(
        Mu::Thread& NODE_THREAD, Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        arg0->clearConnectionCache();
    }

    Pointer
    qt_QNetworkAccessManager_cookieJar_QNetworkCookieJar_QNetworkAccessManager(
        Mu::Thread& NODE_THREAD, Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        return makeinstance<QNetworkCookieJarType>(c, arg0->cookieJar(),
                                                   "qt.QNetworkCookieJar");
    }

    void
    qt_QNetworkAccessManager_enableStrictTransportSecurityStore_void_QNetworkAccessManager_bool_string(
        Mu::Thread& NODE_THREAD, Pointer param_this, bool param_enabled,
        Pointer param_storeDir)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        bool arg1 = (bool)(param_enabled);
        const QString arg2 = qstring(param_storeDir);
        arg0->enableStrictTransportSecurityStore(arg1, arg2);
    }

    bool
    qt_QNetworkAccessManager_isStrictTransportSecurityEnabled_bool_QNetworkAccessManager(
        Mu::Thread& NODE_THREAD, Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        return arg0->isStrictTransportSecurityEnabled();
    }

    bool
    qt_QNetworkAccessManager_isStrictTransportSecurityStoreEnabled_bool_QNetworkAccessManager(
        Mu::Thread& NODE_THREAD, Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        return arg0->isStrictTransportSecurityStoreEnabled();
    }

    int qt_QNetworkAccessManager_redirectPolicy_int_QNetworkAccessManager(
        Mu::Thread& NODE_THREAD, Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        return int(arg0->redirectPolicy());
    }

    void
    qt_QNetworkAccessManager_setAutoDeleteReplies_void_QNetworkAccessManager_bool(
        Mu::Thread& NODE_THREAD, Pointer param_this,
        bool param_shouldAutoDelete)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        bool arg1 = (bool)(param_shouldAutoDelete);
        arg0->setAutoDeleteReplies(arg1);
    }

    void
    qt_QNetworkAccessManager_setCookieJar_void_QNetworkAccessManager_QNetworkCookieJar(
        Mu::Thread& NODE_THREAD, Pointer param_this, Pointer param_cookieJar)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        QNetworkCookieJar* arg1 = object<QNetworkCookieJar>(param_cookieJar);
        arg0->setCookieJar(arg1);
    }

    void
    qt_QNetworkAccessManager_setRedirectPolicy_void_QNetworkAccessManager_int(
        Mu::Thread& NODE_THREAD, Pointer param_this, int param_policy)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        QNetworkRequest::RedirectPolicy arg1 =
            (QNetworkRequest::RedirectPolicy)(param_policy);
        arg0->setRedirectPolicy(arg1);
    }

    void
    qt_QNetworkAccessManager_setStrictTransportSecurityEnabled_void_QNetworkAccessManager_bool(
        Mu::Thread& NODE_THREAD, Pointer param_this, bool param_enabled)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        bool arg1 = (bool)(param_enabled);
        arg0->setStrictTransportSecurityEnabled(arg1);
    }

    void
    qt_QNetworkAccessManager_setTransferTimeout_void_QNetworkAccessManager_int(
        Mu::Thread& NODE_THREAD, Pointer param_this, int param_timeout)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        int arg1 = (int)(param_timeout);
        arg0->setTransferTimeout(arg1);
    }

    Pointer
    qt_QNetworkAccessManager_supportedSchemes_stringBSB_ESB__QNetworkAccessManager(
        Mu::Thread& NODE_THREAD, Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        return isMuQtObject(arg0)
                   ? makestringlist(
                         c, arg0->QNetworkAccessManager::supportedSchemes())
                   : makestringlist(c, arg0->supportedSchemes());
    }

    int qt_QNetworkAccessManager_transferTimeout_int_QNetworkAccessManager(
        Mu::Thread& NODE_THREAD, Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        return arg0->transferTimeout();
    }

    bool qt_QNetworkAccessManager_event_bool_QNetworkAccessManager_QEvent(
        Mu::Thread& NODE_THREAD, Pointer param_this, Pointer param_e)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        QEvent* arg1 = getqpointer<QEventType>(param_e);
        return isMuQtObject(arg0) ? arg0->QNetworkAccessManager::event(arg1)
                                  : arg0->event(arg1);
    }

    bool
    qt_QNetworkAccessManager_eventFilter_bool_QNetworkAccessManager_QObject_QEvent(
        Mu::Thread& NODE_THREAD, Pointer param_this, Pointer param_watched,
        Pointer param_event)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        QObject* arg1 = object<QObject>(param_watched);
        QEvent* arg2 = getqpointer<QEventType>(param_event);
        return isMuQtObject(arg0)
                   ? arg0->QNetworkAccessManager::eventFilter(arg1, arg2)
                   : arg0->eventFilter(arg1, arg2);
    }

    void qt_QNetworkAccessManager_customEvent_void_QNetworkAccessManager_QEvent(
        Mu::Thread& NODE_THREAD, Pointer param_this, Pointer param_event)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        QEvent* arg1 = getqpointer<QEventType>(param_event);
        if (isMuQtObject(arg0))
            ((MuQt_QNetworkAccessManager*)arg0)->customEvent_pub_parent(arg1);
        else
            ((MuQt_QNetworkAccessManager*)arg0)->customEvent_pub(arg1);
    }

    void
    qt_QNetworkAccessManager_timerEvent_void_QNetworkAccessManager_QTimerEvent(
        Mu::Thread& NODE_THREAD, Pointer param_this, Pointer param_event)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QNetworkAccessManager* arg0 = object<QNetworkAccessManager>(param_this);
        QTimerEvent* arg1 = getqpointer<QTimerEventType>(param_event);
        if (isMuQtObject(arg0))
            ((MuQt_QNetworkAccessManager*)arg0)->timerEvent_pub_parent(arg1);
        else
            ((MuQt_QNetworkAccessManager*)arg0)->timerEvent_pub(arg1);
    }

    static NODE_IMPLEMENTATION(_n_QNetworkAccessManager0, Pointer)
    {
        NODE_RETURN(
            qt_QNetworkAccessManager_QNetworkAccessManager_QNetworkAccessManager_QNetworkAccessManager_QObject(
                NODE_THREAD, NONNIL_NODE_ARG(0, Pointer),
                NODE_ARG(1, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_autoDeleteReplies0, bool)
    {
        NODE_RETURN(
            qt_QNetworkAccessManager_autoDeleteReplies_bool_QNetworkAccessManager(
                NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_clearAccessCache0, void)
    {
        qt_QNetworkAccessManager_clearAccessCache_void_QNetworkAccessManager(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer));
    }

    static NODE_IMPLEMENTATION(_n_clearConnectionCache0, void)
    {
        qt_QNetworkAccessManager_clearConnectionCache_void_QNetworkAccessManager(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer));
    }

    static NODE_IMPLEMENTATION(_n_cookieJar0, Pointer)
    {
        NODE_RETURN(
            qt_QNetworkAccessManager_cookieJar_QNetworkCookieJar_QNetworkAccessManager(
                NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_enableStrictTransportSecurityStore0, void)
    {
        qt_QNetworkAccessManager_enableStrictTransportSecurityStore_void_QNetworkAccessManager_bool_string(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, bool),
            NODE_ARG(2, Pointer));
    }

    static NODE_IMPLEMENTATION(_n_isStrictTransportSecurityEnabled0, bool)
    {
        NODE_RETURN(
            qt_QNetworkAccessManager_isStrictTransportSecurityEnabled_bool_QNetworkAccessManager(
                NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_isStrictTransportSecurityStoreEnabled0, bool)
    {
        NODE_RETURN(
            qt_QNetworkAccessManager_isStrictTransportSecurityStoreEnabled_bool_QNetworkAccessManager(
                NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_redirectPolicy0, int)
    {
        NODE_RETURN(
            qt_QNetworkAccessManager_redirectPolicy_int_QNetworkAccessManager(
                NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_setAutoDeleteReplies0, void)
    {
        qt_QNetworkAccessManager_setAutoDeleteReplies_void_QNetworkAccessManager_bool(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, bool));
    }

    static NODE_IMPLEMENTATION(_n_setCookieJar0, void)
    {
        qt_QNetworkAccessManager_setCookieJar_void_QNetworkAccessManager_QNetworkCookieJar(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer));
    }

    static NODE_IMPLEMENTATION(_n_setRedirectPolicy0, void)
    {
        qt_QNetworkAccessManager_setRedirectPolicy_void_QNetworkAccessManager_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int));
    }

    static NODE_IMPLEMENTATION(_n_setStrictTransportSecurityEnabled0, void)
    {
        qt_QNetworkAccessManager_setStrictTransportSecurityEnabled_void_QNetworkAccessManager_bool(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, bool));
    }

    static NODE_IMPLEMENTATION(_n_setTransferTimeout0, void)
    {
        qt_QNetworkAccessManager_setTransferTimeout_void_QNetworkAccessManager_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int));
    }

    static NODE_IMPLEMENTATION(_n_supportedSchemes0, Pointer)
    {
        NODE_RETURN(
            qt_QNetworkAccessManager_supportedSchemes_stringBSB_ESB__QNetworkAccessManager(
                NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_transferTimeout0, int)
    {
        NODE_RETURN(
            qt_QNetworkAccessManager_transferTimeout_int_QNetworkAccessManager(
                NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_event0, bool)
    {
        NODE_RETURN(
            qt_QNetworkAccessManager_event_bool_QNetworkAccessManager_QEvent(
                NODE_THREAD, NONNIL_NODE_ARG(0, Pointer),
                NODE_ARG(1, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_eventFilter0, bool)
    {
        NODE_RETURN(
            qt_QNetworkAccessManager_eventFilter_bool_QNetworkAccessManager_QObject_QEvent(
                NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer),
                NODE_ARG(2, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_customEvent0, void)
    {
        qt_QNetworkAccessManager_customEvent_void_QNetworkAccessManager_QEvent(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer));
    }

    static NODE_IMPLEMENTATION(_n_timerEvent0, void)
    {
        qt_QNetworkAccessManager_timerEvent_void_QNetworkAccessManager_QTimerEvent(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer));
    }

    void QNetworkAccessManagerType::load()
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

        addSymbols(
            new Function(c, "__allocate", BaseFunctions::classAllocate, None,
                         Return, ftn, End),

            new Function(c, tn, castFromObject, Cast, Compiled,
                         QNetworkAccessManager_QNetworkAccessManager_QObject,
                         Return, ftn, Parameters,
                         new Param(c, "object", "qt.QObject"), End),

            EndArguments);

        addSymbols(
            // enums
            // member functions
            new Function(
                c, "QNetworkAccessManager", _n_QNetworkAccessManager0, None,
                Compiled,
                qt_QNetworkAccessManager_QNetworkAccessManager_QNetworkAccessManager_QNetworkAccessManager_QObject,
                Return, "qt.QNetworkAccessManager", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"),
                new Param(c, "parent", "qt.QObject"), End),
            // MISSING: addStrictTransportSecurityHosts (void;
            // QNetworkAccessManager this, "const QList<QHstsPolicy> &"
            // knownHosts)
            new Function(
                c, "autoDeleteReplies", _n_autoDeleteReplies0, None, Compiled,
                qt_QNetworkAccessManager_autoDeleteReplies_bool_QNetworkAccessManager,
                Return, "bool", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"), End),
            // MISSING: cache ("QAbstractNetworkCache *"; QNetworkAccessManager
            // this)
            new Function(
                c, "clearAccessCache", _n_clearAccessCache0, None, Compiled,
                qt_QNetworkAccessManager_clearAccessCache_void_QNetworkAccessManager,
                Return, "void", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"), End),
            new Function(
                c, "clearConnectionCache", _n_clearConnectionCache0, None,
                Compiled,
                qt_QNetworkAccessManager_clearConnectionCache_void_QNetworkAccessManager,
                Return, "void", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"), End),
            // MISSING: connectToHost (void; QNetworkAccessManager this, string
            // hostName, "quint16" port) MISSING: connectToHostEncrypted (void;
            // QNetworkAccessManager this, string hostName, "quint16" port,
            // "const QSslConfiguration &" sslConfiguration) MISSING:
            // connectToHostEncrypted (void; QNetworkAccessManager this, string
            // hostName, "quint16" port, "const QSslConfiguration &"
            // sslConfiguration, string peerName)
            new Function(
                c, "cookieJar", _n_cookieJar0, None, Compiled,
                qt_QNetworkAccessManager_cookieJar_QNetworkCookieJar_QNetworkAccessManager,
                Return, "qt.QNetworkCookieJar", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"), End),
            // MISSING: deleteResource (QNetworkReply; QNetworkAccessManager
            // this, "const QNetworkRequest &" request)
            new Function(
                c, "enableStrictTransportSecurityStore",
                _n_enableStrictTransportSecurityStore0, None, Compiled,
                qt_QNetworkAccessManager_enableStrictTransportSecurityStore_void_QNetworkAccessManager_bool_string,
                Return, "void", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"),
                new Param(c, "enabled", "bool"),
                new Param(c, "storeDir", "string"), End),
            // MISSING: get (QNetworkReply; QNetworkAccessManager this, "const
            // QNetworkRequest &" request) MISSING: head (QNetworkReply;
            // QNetworkAccessManager this, "const QNetworkRequest &" request)
            new Function(
                c, "isStrictTransportSecurityEnabled",
                _n_isStrictTransportSecurityEnabled0, None, Compiled,
                qt_QNetworkAccessManager_isStrictTransportSecurityEnabled_bool_QNetworkAccessManager,
                Return, "bool", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"), End),
            new Function(
                c, "isStrictTransportSecurityStoreEnabled",
                _n_isStrictTransportSecurityStoreEnabled0, None, Compiled,
                qt_QNetworkAccessManager_isStrictTransportSecurityStoreEnabled_bool_QNetworkAccessManager,
                Return, "bool", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"), End),
            // MISSING: post (QNetworkReply; QNetworkAccessManager this, "const
            // QNetworkRequest &" request, QIODevice data) MISSING: post
            // (QNetworkReply; QNetworkAccessManager this, "const
            // QNetworkRequest &" request, QByteArray data) MISSING: post
            // (QNetworkReply; QNetworkAccessManager this, "const
            // QNetworkRequest &" request, "QHttpMultiPart *" multiPart)
            // MISSING: proxy ("QNetworkProxy"; QNetworkAccessManager this)
            // MISSING: proxyFactory ("QNetworkProxyFactory *";
            // QNetworkAccessManager this) MISSING: put (QNetworkReply;
            // QNetworkAccessManager this, "const QNetworkRequest &" request,
            // QIODevice data) MISSING: put (QNetworkReply;
            // QNetworkAccessManager this, "const QNetworkRequest &" request,
            // QByteArray data) MISSING: put (QNetworkReply;
            // QNetworkAccessManager this, "const QNetworkRequest &" request,
            // "QHttpMultiPart *" multiPart)
            new Function(
                c, "redirectPolicy", _n_redirectPolicy0, None, Compiled,
                qt_QNetworkAccessManager_redirectPolicy_int_QNetworkAccessManager,
                Return, "int", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"), End),
            // MISSING: sendCustomRequest (QNetworkReply; QNetworkAccessManager
            // this, "const QNetworkRequest &" request, QByteArray verb,
            // QIODevice data) MISSING: sendCustomRequest (QNetworkReply;
            // QNetworkAccessManager this, "const QNetworkRequest &" request,
            // QByteArray verb, QByteArray data) MISSING: sendCustomRequest
            // (QNetworkReply; QNetworkAccessManager this, "const
            // QNetworkRequest &" request, QByteArray verb, "QHttpMultiPart *"
            // multiPart)
            new Function(
                c, "setAutoDeleteReplies", _n_setAutoDeleteReplies0, None,
                Compiled,
                qt_QNetworkAccessManager_setAutoDeleteReplies_void_QNetworkAccessManager_bool,
                Return, "void", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"),
                new Param(c, "shouldAutoDelete", "bool"), End),
            // MISSING: setCache (void; QNetworkAccessManager this,
            // "QAbstractNetworkCache *" cache)
            new Function(
                c, "setCookieJar", _n_setCookieJar0, None, Compiled,
                qt_QNetworkAccessManager_setCookieJar_void_QNetworkAccessManager_QNetworkCookieJar,
                Return, "void", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"),
                new Param(c, "cookieJar", "qt.QNetworkCookieJar"), End),
            // MISSING: setProxy (void; QNetworkAccessManager this, "const
            // QNetworkProxy &" proxy) MISSING: setProxyFactory (void;
            // QNetworkAccessManager this, "QNetworkProxyFactory *" factory)
            new Function(
                c, "setRedirectPolicy", _n_setRedirectPolicy0, None, Compiled,
                qt_QNetworkAccessManager_setRedirectPolicy_void_QNetworkAccessManager_int,
                Return, "void", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"),
                new Param(c, "policy", "int"), End),
            new Function(
                c, "setStrictTransportSecurityEnabled",
                _n_setStrictTransportSecurityEnabled0, None, Compiled,
                qt_QNetworkAccessManager_setStrictTransportSecurityEnabled_void_QNetworkAccessManager_bool,
                Return, "void", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"),
                new Param(c, "enabled", "bool"), End),
            new Function(
                c, "setTransferTimeout", _n_setTransferTimeout0, None, Compiled,
                qt_QNetworkAccessManager_setTransferTimeout_void_QNetworkAccessManager_int,
                Return, "void", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"),
                new Param(
                    c, "timeout", "int",
                    Value(
                        (int)QNetworkRequest::DefaultTransferTimeoutConstant)),
                End),
            // MISSING: strictTransportSecurityHosts ("QList<QHstsPolicy>";
            // QNetworkAccessManager this)
            _func[0] = new MemberFunction(
                c, "supportedSchemes", _n_supportedSchemes0, None, Compiled,
                qt_QNetworkAccessManager_supportedSchemes_stringBSB_ESB__QNetworkAccessManager,
                Return, "string[]", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"), End),
            new Function(
                c, "transferTimeout", _n_transferTimeout0, None, Compiled,
                qt_QNetworkAccessManager_transferTimeout_int_QNetworkAccessManager,
                Return, "int", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"), End),
            // MISSING: createRequest (QNetworkReply; QNetworkAccessManager
            // this, flags QNetworkAccessManager::Operation op, "const
            // QNetworkRequest &" originalReq, QIODevice outgoingData) //
            // protected
            _func[1] = new MemberFunction(
                c, "event", _n_event0, None, Compiled,
                qt_QNetworkAccessManager_event_bool_QNetworkAccessManager_QEvent,
                Return, "bool", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"),
                new Param(c, "e", "qt.QEvent"), End),
            _func[2] = new MemberFunction(
                c, "eventFilter", _n_eventFilter0, None, Compiled,
                qt_QNetworkAccessManager_eventFilter_bool_QNetworkAccessManager_QObject_QEvent,
                Return, "bool", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"),
                new Param(c, "watched", "qt.QObject"),
                new Param(c, "event", "qt.QEvent"), End),
            // MISSING: metaObject ("const QMetaObject *"; QNetworkAccessManager
            // this) MISSING: childEvent (void; QNetworkAccessManager this,
            // "QChildEvent *" event) // protected MISSING: connectNotify (void;
            // QNetworkAccessManager this, "const QMetaMethod &" signal) //
            // protected
            _func[3] = new MemberFunction(
                c, "customEvent", _n_customEvent0, None, Compiled,
                qt_QNetworkAccessManager_customEvent_void_QNetworkAccessManager_QEvent,
                Return, "void", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"),
                new Param(c, "event", "qt.QEvent"), End),
            // MISSING: disconnectNotify (void; QNetworkAccessManager this,
            // "const QMetaMethod &" signal) // protected
            _func[4] = new MemberFunction(
                c, "timerEvent", _n_timerEvent0, None, Compiled,
                qt_QNetworkAccessManager_timerEvent_void_QNetworkAccessManager_QTimerEvent,
                Return, "void", Parameters,
                new Param(c, "this", "qt.QNetworkAccessManager"),
                new Param(c, "event", "qt.QTimerEvent"), End),
            // static functions
            EndArguments);
        globalScope()->addSymbols(EndArguments);
        scope()->addSymbols(EndArguments);

        const char** propExclusions = 0;

        populate(this, QNetworkAccessManager::staticMetaObject, propExclusions);
    }

} // namespace Mu
