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

#ifndef __MuQt6__QFileDeviceType__h__
#define __MuQt6__QFileDeviceType__h__
#include <iostream>
#include <Mu/Class.h>
#include <Mu/MuProcess.h>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include <QtNetwork/QtNetwork>
#include <QtWebEngineWidgets/QtWebEngineWidgets>
#include <QtQml/QtQml>
#include <QtQuick/QtQuick>
#include <QtQuickWidgets/QtQuickWidgets>
#include <QtSvg/QtSvg>
#include <QSvgWidget>
#include <MuQt6/Bridge.h>

namespace Mu
{
    class MuQt_QFileDevice;

    class QFileDeviceType : public Class
    {
    public:
        typedef MuQt_QFileDevice MuQtType;
        typedef QFileDevice QtType;

        //
        //  Constructors
        //

        QFileDeviceType(Context* context, const char* name,
                        Class* superClass = 0, Class* superClass2 = 0);

        virtual ~QFileDeviceType();

        static bool isInheritable() { return true; }

        static inline ClassInstance* cachedInstance(const MuQtType*);

        //
        //  Class API
        //

        virtual void load();

        MemberFunction* _func[18];
    };

    // Inheritable object

    class MuQt_QFileDevice : public QFileDevice
    {
    public:
        virtual ~MuQt_QFileDevice();
        virtual QString fileName() const;
        virtual QFileDevice::Permissions permissions() const;
        virtual bool resize(qint64 sz);
        virtual bool setPermissions(QFileDevice::Permissions permissions);
        virtual bool atEnd() const;
        virtual void close();
        virtual bool isSequential() const;
        virtual qint64 pos() const;
        virtual bool seek(qint64 pos);
        virtual qint64 size() const;
        virtual qint64 bytesAvailable() const;
        virtual qint64 bytesToWrite() const;
        virtual bool canReadLine() const;
        virtual bool open(QIODeviceBase::OpenMode mode);
        virtual bool reset();
        virtual bool waitForBytesWritten(int msecs);
        virtual bool waitForReadyRead(int msecs);

    protected:
        virtual qint64 skipData(qint64 maxSize);

    public:
        qint64 skipData_pub(qint64 maxSize) { return skipData(maxSize); }

        qint64 skipData_pub_parent(qint64 maxSize)
        {
            return QFileDevice::skipData(maxSize);
        }

    public:
        const QFileDeviceType* _baseType;
        ClassInstance* _obj;
        const CallEnvironment* _env;
    };

    inline ClassInstance*
    QFileDeviceType::cachedInstance(const QFileDeviceType::MuQtType* obj)
    {
        return obj->_obj;
    }

} // namespace Mu

#endif // __MuQt__QFileDeviceType__h__
