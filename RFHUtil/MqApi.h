#pragma once

#include "cmqc.h"

// P4.1 PR D — extracted from DataArea.h.
//
// The IBM MQ runtime DLL is loaded dynamically at app startup (via
// DataArea::loadMQdll) so the app can run on machines without MQ installed
// — the menus and dialogs still work, only the queue operations are
// disabled. Each MQ API call goes through a function pointer populated
// by GetProcAddress; the typedefs below describe the pointer shapes.
//
// PR D moves the typedefs + the pointers themselves out of DataArea into
// a focused MqApi struct so PR E can hand a pointer to MqApi to
// MQConnection and let connect2QM / discQM / etc. live there. DataArea
// keeps reference aliases for the individual pointer names so all
// existing call sites in DataArea.cpp (XMQConnX(...), XMQOpen(...), ...)
// compile unchanged.

//////////////////////////////////////////////////////////////////
//  MQCONNX Function -- Connect Queue Manager (extended)
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQCONNX) (
    PMQCHAR     pQMgrName,    // Name of queue manager
    PMQCNO      pConnectOpts, // Options that control the action of MQCONNX
    PMQHCONN    pHconn,       // Connection handle
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQDISC Function -- Disconnect Queue Manager
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQDISC) (
    PMQHCONN    pHconn,       // Connection handle
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQBACK Function -- Back Out Changes
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQBACK) (
    MQHCONN     Hconn,        // Connection handle
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQBEGIN Function -- Begin Unit of Work
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQBEGIN) (
    MQHCONN     Hconn,        // Connection handle
    PMQVOID     pBeginOptions,// Options that control the action of MQBEGIN
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQCMIT Function -- Commit Changes
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQCMIT) (
    MQHCONN     Hconn,        // Connection handle
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQOPEN Function -- Open Object
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQOPEN) (
    MQHCONN     Hconn,        // Connection handle
    PMQVOID     pObjDesc,     // Object descriptor
    MQLONG      Options,      // Options that control the action of MQOPEN
    PMQHOBJ     pHobj,        // Object handle
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQCLOSE Function -- Close Object
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQCLOSE) (
    MQHCONN     Hconn,        // Connection handle
    PMQHOBJ     pHobj,        // Object handle
    MQLONG      Options,      // Options that control the action of MQCLOSE
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQGET Function -- Get Message
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQGET) (
    MQHCONN     Hconn,        // Connection handle
    MQHOBJ      Hobj,         // Object handle
    PMQVOID     pMsgDesc,     // Message descriptor
    PMQVOID     pGetMsgOpts,  // Options that control the action of MQGET
    MQLONG      BufferLength, // Length in bytes of the Buffer area
    PMQVOID     pBuffer,      // Area to contain the message data
    PMQLONG     pDataLength,  // Length of the message
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQPUT Function -- Put Message
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQPUT) (
    MQHCONN     Hconn,        // Connection handle
    MQHOBJ      Hobj,         // Object handle
    PMQVOID     pMsgDesc,     // Message descriptor
    PMQVOID     pPutMsgOpts,  // Options that control the action of MQPUT
    MQLONG      BufferLength, // Length of the message in Buffer
    PMQVOID     pBuffer,      // Message data
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQINQ Function -- Inquire Object Attributes
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQINQ) (
    MQHCONN     Hconn,        // Connection handle
    MQHOBJ      Hobj,         // Object handle
    MQLONG      SelectorCount,// Count of selectors
    PMQLONG     pSelectors,   // Array of attribute selectors
    MQLONG      IntAttrCount, // Count of integer attributes
    PMQLONG     pIntAttrs,    // Array of integer attributes
    MQLONG      CharAttrLength,// Length of character attributes buffer
    PMQCHAR     pCharAttrs,   // Character attributes
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQSUB Function -- Subscribe to topic
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQSUB) (
    MQHCONN     Hconn,        // Connection handle
    PMQVOID     pSubDesc,     // Subscription descriptor
    PMQHOBJ     pHobj,        // Object handle for queue
    PMQHOBJ     pHsub,        // Subscription object handle
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQSUBRQ Function -- Subscription Request
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQSUBRQ) (
    MQHCONN     Hconn,        // Connection handle
    MQHOBJ      Hsub,         // Subscription handle
    MQLONG      Action,       // Action requested on the subscription
    PMQVOID     pSubRqOpts,   // Subscription Request Options
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQCRTMH Function -- Create Message Handle
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQCRTMH) (
    MQHCONN     Hconn,        // Connection handle
    PMQVOID     pCrtMsgHOpts, // Options that control the action of MQCRTMH
    PMQHMSG     pHmsg,        // Message handle
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQDLTMH Function -- Delete Message Handle
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQDLTMH) (
    MQHCONN     Hconn,        // Connection handle
    PMQHMSG     pHmsg,        // Message handle
    PMQVOID     pDltMsgHOpts, // Options that control the action of MQDLTMH
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQINQMP Function -- Inquire Message Property
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQINQMP) (
    MQHCONN     Hconn,        // Connection handle
    MQHMSG      Hmsg,         // Message handle
    PMQVOID     pInqPropOpts, // Options that control the action of MQINQMP
    PMQVOID     pName,        // Property name
    PMQVOID     pPropDesc,    // Property descriptor
    PMQLONG     pType,        // Property data type
    MQLONG      ValueLength,  // Length in bytes of the Value area
    PMQVOID     pValue,       // Property value
    PMQLONG     pDataLength,  // Length of the property value
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

//////////////////////////////////////////////////////////////////
//  MQSETMP Function -- Set Message Property
//////////////////////////////////////////////////////////////////

typedef void (MQENTRY* XMQSETMP) (
    MQHCONN     Hconn,        // Connection handle
    MQHMSG      Hmsg,         // Message handle
    PMQVOID     pSetPropOpts, // Options that control the action of MQSETMP
    PMQVOID     pName,        // Property name
    PMQVOID     pPropDesc,    // Property descriptor
    MQLONG      Type,         // Property data type
    MQLONG      ValueLength,  // Length of the Value area
    PMQVOID     pValue,       // Property value
    PMQLONG     pCompCode,    // Completion code
    PMQLONG     pReason);     // Reason code qualifying CompCode

// Dynamically-loaded MQ runtime entry points. Populated by
// DataArea::loadMQdll at app startup; null until then. PR E gives
// MQConnection a pointer to this struct so connect2QM / discQM /
// attemptReconnection / performHealthCheck can move onto MQConnection
// while still calling through the same dynamically-loaded entry points.
struct MqApi {
    HINSTANCE   mqmdll;

    XMQCONNX    XMQConnX;
    XMQDISC     XMQDisc;
    XMQBEGIN    XMQBegin;
    XMQCMIT     XMQCmit;
    XMQBACK     XMQBack;
    XMQOPEN     XMQOpen;
    XMQCLOSE    XMQClose;
    XMQGET      XMQGet;
    XMQPUT      XMQPut;
    XMQINQ      XMQInq;
    XMQSUB      XMQSub;
    XMQSUBRQ    XMQSubRq;
    XMQCRTMH    XMQCrtMh;
    XMQDLTMH    XMQDltMh;
    XMQINQMP    XMQInqMp;
    XMQSETMP    XMQSetMp;

    MqApi()
        : mqmdll(NULL)
        , XMQConnX(NULL)
        , XMQDisc(NULL)
        , XMQBegin(NULL)
        , XMQCmit(NULL)
        , XMQBack(NULL)
        , XMQOpen(NULL)
        , XMQClose(NULL)
        , XMQGet(NULL)
        , XMQPut(NULL)
        , XMQInq(NULL)
        , XMQSub(NULL)
        , XMQSubRq(NULL)
        , XMQCrtMh(NULL)
        , XMQDltMh(NULL)
        , XMQInqMp(NULL)
        , XMQSetMp(NULL)
    {}
};
