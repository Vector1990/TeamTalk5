/*
* Copyright (c) 2005-2018, BearWare.dk
*
* Contact Information:
*
* Bjoern D. Rasmussen
* Kirketoften 5
* DK-8260 Viby J
* Denmark
* Email: contact@bearware.dk
* Phone: +45 20 20 54 59
* Web: http://www.bearware.dk
*
* This source code is part of the TeamTalk SDK owned by
* BearWare.dk. Use of this file, or its compiled unit, requires a
* TeamTalk SDK License Key issued by BearWare.dk.
*
* The TeamTalk SDK License Agreement along with its Terms and
* Conditions are outlined in the file License.txt included with the
* TeamTalk SDK distribution.
*
*/

#include <ace/ACE.h>

#include "TTUnitTest.h"

#include <myace/MyACE.h>

bool InitSound(TTInstance* ttClient, TTBOOL duplex/* = false*/, INT32 indev/* = -1*/, INT32 outdev/* = -1*/)
{
    int selindev = indev, seloutdev = outdev;
    if (indev == -1 || outdev == -1)
    {
        if (!TT_GetDefaultSoundDevices(&indev, &outdev))
            return false;
    }

    if (selindev == -1)
        selindev = indev;
    if (seloutdev == -1)
        seloutdev = outdev;

    if (duplex)
        return TT_InitSoundDuplexDevices(ttClient, selindev, seloutdev);
    TTBOOL success = TT_InitSoundInputDevice(ttClient, selindev);
    success &= TT_InitSoundOutputDevice(ttClient, seloutdev);
    return success;
}

bool Connect(TTInstance* ttClient, const TTCHAR hostname[TT_STRLEN], INT32 tcpport, INT32 udpport)
{
    if (!TT_Connect(ttClient, hostname, tcpport, udpport, 0, 0, FALSE))
        return false;
    return WaitForEvent(ttClient, CLIENTEVENT_CON_SUCCESS);
}

bool Login(TTInstance* ttClient, const TTCHAR nickname[TT_STRLEN], const TTCHAR username[TT_STRLEN], const TTCHAR passwd[TT_STRLEN])
{
    return WaitForCmdSuccess(ttClient, TT_DoLogin(ttClient, nickname, username, passwd));
}

bool JoinRoot(TTInstance* ttClient)
{
    auto chanid = TT_GetRootChannelID(ttClient);
    return WaitForCmdSuccess(ttClient, TT_DoJoinChannelByID(ttClient, chanid, ACE_TEXT("")));
}

bool WaitForEvent(TTInstance* ttClient, ClientEvent ttevent, std::function<bool(TTMessage)> pred, TTMessage* outmsg, int timeout /*= DEFWAIT*/)
{
    TTMessage msg = {};
    auto start = GETTIMESTAMP();
    while(GETTIMESTAMP() < start + timeout)
    {
        INT32 waitMsec = 10;
        if(TT_GetMessage(ttClient, &msg, &waitMsec) &&
            msg.nClientEvent == ttevent &&
            pred(msg))
        {
            if (outmsg)
                *outmsg = msg;
            
            return true;
        }
    }
    return false;
}

bool WaitForEvent(TTInstance* ttClient, ClientEvent ttevent, TTMessage* outmsg, int timeout /*= DEFWAIT*/)
{
    return WaitForEvent(ttClient, ttevent, [](TTMessage) { return true; }, outmsg, timeout);
}

bool WaitForCmdSuccess(TTInstance* ttClient, int cmdid, TTMessage* outmsg, int timeout /*= DEFWAIT*/)
{
    return WaitForEvent(ttClient, CLIENTEVENT_CMD_SUCCESS, [cmdid](TTMessage msg) {
        return msg.nSource == cmdid;
    }, outmsg, timeout);
}

bool WaitForCmdComplete(TTInstance* ttClient, int cmdid, TTMessage* outmsg, int timeout /*= DEFWAIT*/)
{
    return WaitForEvent(ttClient, CLIENTEVENT_CMD_PROCESSING, [cmdid](TTMessage msg) {
        return msg.nSource == cmdid && !msg.bActive;
    }, outmsg, timeout);
}
