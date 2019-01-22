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

package dk.bearware;

import java.util.Vector;

import junit.framework.TestCase;

public abstract class TeamTalkTestCaseBase extends TestCase {

    public static boolean ENCRYPTED = false;
    public static boolean DEBUG_OUTPUT = false;
    public static final int DEF_WAIT = 15000;

    public static String ADMIN_USERNAME = "admin", ADMIN_PASSWORD = "admin", ADMIN_NICKNAME = "Admin";
    public static String IPADDR = "127.0.0.1";

    public static int TCPPORT = 0, UDPPORT = 0;

    public static String SYSTEMID = "teamtalk";

    public static int INPUTDEVICEID = -1, OUTPUTDEVICEID = -1;
    public static String VIDEODEVICEID = "None", VIDEODEVDISABLE="None"; //set to "None" to ignore video capture tests

    public static final String CRYPTO_CERT_FILE = "ttservercert.pem", CRYPTO_KEY_FILE = "ttserverkey.pem";
    public static final String MUXEDMEDIAFILE_WAVE = "muxwavefile.wav";
    public static final String MUXEDMEDIAFILE_SPEEX = "muxwavefile_speex.ogg";
    public static final String MUXEDMEDIAFILE_SPEEX_VBR = "muxwavefile_speex_vbr.ogg";
    public static final String MUXEDMEDIAFILE_OPUS = "muxwavefile_opus.ogg";
    public static final String MEDIAFILE_AUDIO = "music.wav";
    public static final String MEDIAFILE = "video.avi";
    public static final String HTTPS_MEDIAFILE = "https://www.bearware.dk/test/giana.wma";
    public Vector<TeamTalkBase> ttclients = new Vector<TeamTalkBase>();

    public abstract TeamTalkBase newClientInstance();

    protected void setUp() throws Exception {
        super.setUp();

        String prop = System.getProperty("dk.bearware.sndinputid");
        if(prop != null && !prop.isEmpty())
            this.INPUTDEVICEID = Integer.parseInt(prop);

        prop = System.getProperty("dk.bearware.sndoutputid");
        if(prop != null && !prop.isEmpty())
            this.OUTPUTDEVICEID = Integer.parseInt(prop);

        prop = System.getProperty("dk.bearware.encrypted");
        if(prop != null && !prop.isEmpty())
            this.ENCRYPTED = Integer.parseInt(prop) != 0;

        prop = System.getProperty("dk.bearware.serverip");
        if(prop != null && !prop.isEmpty())
            this.IPADDR = prop;

        prop = System.getProperty("dk.bearware.videodevid");
        if(prop != null && !prop.isEmpty())
            this.VIDEODEVICEID = prop;

        if(TCPPORT == 0 && UDPPORT == 0) {
            if(this.ENCRYPTED) {
                TCPPORT = Constants.DEFAULT_TCP_PORT_ENCRYPTED;
                UDPPORT = Constants.DEFAULT_UDP_PORT_ENCRYPTED;
            }
            else {
                TCPPORT = Constants.DEFAULT_TCP_PORT;
                UDPPORT = Constants.DEFAULT_UDP_PORT;
            }
        }
    }

    protected void tearDown() throws Exception {
        super.tearDown();

        for(TeamTalkBase ttclient : ttclients) {
            ttclient.disconnect();
            if((ttclient.getFlags() & ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX) == ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX) {
                ttclient.closeSoundDuplexDevices();
            }
            else {
                ttclient.closeSoundInputDevice();
                ttclient.closeSoundOutputDevice();
            }
            ttclient.closeVideoCaptureDevice();
            ttclient.stopStreamingMediaFileToChannel();
        }
        ttclients.clear();
    }

    protected void initSound(TeamTalkBase ttclient) {
        initSound(ttclient, false);
    }

    protected void initSound(TeamTalkBase ttclient, boolean duplex) {

        Vector<SoundDevice> devs = new Vector<SoundDevice>();
        assertTrue("get sound devs", ttclient.getSoundDevices(devs));
        System.out.println("---- Sound Devices ----");
        for(int i=0;i<devs.size();i++)
            printSoundDevice(devs.get(i));

        IntPtr indev = new IntPtr(), outdev = new IntPtr();
        if(INPUTDEVICEID < 0 && OUTPUTDEVICEID < 0)
           assertTrue("get default devs", ttclient.getDefaultSoundDevices(indev, outdev));

        if(INPUTDEVICEID >= 0)
            indev.value = INPUTDEVICEID;
        if(OUTPUTDEVICEID >= 0)
            outdev.value = OUTPUTDEVICEID;

        if(duplex) {
            assertTrue("init duplex devs", ttclient.initSoundDuplexDevices(indev.value, outdev.value));
        }
        else {
            assertTrue("init input dev", ttclient.initSoundInputDevice(indev.value));
            assertTrue("init output dev", ttclient.initSoundOutputDevice(outdev.value));
        }

        SpeexDSP spxdsp = new SpeexDSP(true), spxdsp2 = new SpeexDSP();
        assertTrue("set Speex DSP", ttclient.setSoundInputPreprocess(spxdsp));

        assertTrue("get Speex DSP", ttclient.getSoundInputPreprocess(spxdsp2));
        assertEquals("agc1", spxdsp.bEnableAGC, spxdsp2.bEnableAGC);
        assertEquals("agc2", spxdsp.nGainLevel, spxdsp2.nGainLevel);
        assertEquals("agc3", spxdsp.nMaxIncDBSec, spxdsp2.nMaxIncDBSec);
        assertEquals("agc4", spxdsp.nMaxDecDBSec, spxdsp2.nMaxDecDBSec);
        assertEquals("agc5", spxdsp.nMaxGainDB, spxdsp2.nMaxGainDB);
        assertEquals("agc6", spxdsp.bEnableDenoise, spxdsp2.bEnableDenoise);
        assertEquals("agc7", spxdsp.nMaxNoiseSuppressDB, spxdsp2.nMaxNoiseSuppressDB);
        assertEquals("agc8", spxdsp.nEchoSuppress, spxdsp2.nEchoSuppress);
        assertEquals("agc9", spxdsp.nEchoSuppressActive, spxdsp2.nEchoSuppressActive);
    }

    public interface ServerInterleave {
        public void interleave();
    }

    static ServerInterleave nop = new ServerInterleave() {
        public void interleave() {
        }
    };

    protected static void connect(TeamTalkBase ttclient)
    {
        connect(ttclient, SYSTEMID);
    }

    protected static void connect(TeamTalkBase ttclient, String systemID) {
        connect(ttclient, systemID, nop);
    }

    protected static void connect(TeamTalkBase ttclient, String systemID, ServerInterleave server) {
        connect(ttclient, systemID, IPADDR, TCPPORT, UDPPORT, server);
    }

    protected static void connect(TeamTalkBase ttclient, String systemID,
                                  String hostaddr, int tcpport, int udpport, ServerInterleave server)
    {
        assertTrue("connect call", ttclient.connectSysID(hostaddr, tcpport, udpport, 0, 0, ENCRYPTED, systemID));

        assertTrue("wait connect", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CON_SUCCESS, DEF_WAIT, server));
    }

    protected static void login(TeamTalkBase ttclient, String nick, String username, String passwd) {
        login(ttclient, nick, username, passwd, "");
    }

    protected static void login(TeamTalkBase ttclient, String nick, String username, String passwd, String clientname) {
        login(ttclient, nick, username, passwd, clientname, nop);
    }

    protected static void login(TeamTalkBase ttclient, String nick, String username,
                                String passwd, String clientname, ServerInterleave server)
    {
        int cmdid = ttclient.doLoginEx(nick, username, passwd, clientname);
        assertTrue("do login", cmdid > 0);

        TTMessage msg = new TTMessage();
        assertTrue("wait login", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN, DEF_WAIT, msg, server));

        UserAccount account = msg.useraccount;
        assertEquals("username set", username, account.szUsername);
        //Assert.AreEqual(passwd, account.szPassword, "password set");
        assertTrue("Wait login complete", waitCmdComplete(ttclient, cmdid, 1000));
        assertTrue("Authorized", hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_AUTHORIZED));
    }

    protected void makeUserAccount(String nickname, String username, String password, int userrights)
    {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, nickname, ADMIN_USERNAME, ADMIN_PASSWORD);
        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = username;
        useraccount.szPassword = password;
        useraccount.uUserRights = userrights;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        assertTrue("New user accout ok", waitCmdSuccess(ttclient, ttclient.doNewUserAccount(useraccount), DEF_WAIT));
        assertTrue("Disconnect", ttclient.disconnect());
    }

    protected static void joinRoot(TeamTalkBase ttclient) {
        joinRoot(ttclient, nop);
    }

    protected static void joinRoot(TeamTalkBase ttclient, ServerInterleave server)
    {
        assertTrue("Auth ok", hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_AUTHORIZED));

        assertTrue("root exists", ttclient.getRootChannelID() > 0);

        int cmdid = ttclient.doJoinChannelByID(ttclient.getRootChannelID(), "");

        assertTrue("do join root", cmdid > 0);

        assertTrue("Wait join complete", waitCmdComplete(ttclient, cmdid, 1000, server));

        assertEquals("In root channel", ttclient.getMyChannelID(), ttclient.getRootChannelID());
    }

    protected static boolean waitForEvent(TeamTalkBase ttclient, int nClientEvent,
                                          int waittimeout, TTMessage msg) {
        return waitForEvent(ttclient, nClientEvent, waittimeout, msg, nop);
    }

    protected static boolean waitForEvent(TeamTalkBase ttclient, int nClientEvent,
                                          int waittimeout, ServerInterleave interleave) {
        return waitForEvent(ttclient, nClientEvent, waittimeout, new TTMessage(), interleave);
    }
/*
    protected static boolean waitForEvent(TeamTalkBase ttclient, int nClientEvent,
                                          int waittimeout, TeamTalkEventHandler eventhandler,
                                          ServerInterleave interleave) {
        
        long start = System.currentTimeMillis();
        TTMessage tmp = new TTMessage();
        boolean gotmsg;
        do {
            gotmsg = eventhandler.processEvent(ttclient, waittimeout);

            interleave.interleave();

            if(System.currentTimeMillis() - start >= waittimeout)
                break;
        }
        while (!gotmsg || tmp.nClientEvent != nClientEvent);
    }
*/
    protected static boolean waitForEvent(TeamTalkBase ttclient, int nClientEvent,
                                          int waittimeout, TTMessage msg, ServerInterleave interleave) {
        long start = System.currentTimeMillis();
        TTMessage tmp = new TTMessage();
        boolean gotmsg;
        do {
            gotmsg = ttclient.getMessage(tmp, 0);

            interleave.interleave();

            if(DEBUG_OUTPUT && gotmsg) {
                System.out.println(System.currentTimeMillis() + " #" + ttclient.getMyUserID() + ": " + tmp.nClientEvent);
                if(tmp.nClientEvent == ClientEvent.CLIENTEVENT_CMD_ERROR) {
                    System.out.println("Command error: " + tmp.clienterrormsg.szErrorMsg);
                }
            }
            if(System.currentTimeMillis() - start >= waittimeout)
                break;
        }
        while (!gotmsg || tmp.nClientEvent != nClientEvent);

        if (tmp.nClientEvent == nClientEvent)
        {
            if (DEBUG_OUTPUT)
                System.out.println("Success. Event: " + nClientEvent);

            msg.nClientEvent = tmp.nClientEvent;
            msg.ttType = tmp.ttType;
            msg.nSource = tmp.nSource;

            msg.bActive = tmp.bActive;
            msg.channel = tmp.channel;
            msg.clienterrormsg = tmp.clienterrormsg;
            msg.desktopinput = tmp.desktopinput;
            msg.filetransfer = tmp.filetransfer;
            msg.mediafileinfo = tmp.mediafileinfo;
            msg.nBytesRemain = tmp.nBytesRemain;
            msg.nPayloadSize = tmp.nPayloadSize;
            msg.nStreamID = tmp.nStreamID;
            msg.remotefile = tmp.remotefile;
            msg.serverproperties = tmp.serverproperties;
            msg.serverstatistics = tmp.serverstatistics;
            msg.textmessage = tmp.textmessage;
            msg.user = tmp.user;
            msg.useraccount = tmp.useraccount;
            msg.banneduser = tmp.banneduser;
            msg.nStreamType = tmp.nStreamType;
            //if assert fails it's because the TTType isn't handled here
            assertTrue("TTType unhandled: " + tmp.ttType, tmp.ttType <= TTType.__STREAMTYPE);
        }
        else
        {
            if (DEBUG_OUTPUT)
                System.out.println("Failed. Event: " + nClientEvent);
        }
        return tmp.nClientEvent == nClientEvent;
    }


    protected static boolean waitForEvent(TeamTalkBase ttclient, int nClientEvent, int waittimeout)  {
        TTMessage msg = new TTMessage();
        return waitForEvent(ttclient, nClientEvent, waittimeout, msg);
    }

    protected static boolean waitCmdComplete(TeamTalkBase ttclient, int cmdid, int waittimeout) {
        return waitCmdComplete(ttclient, cmdid, waittimeout, nop);
    }

    protected static boolean waitCmdComplete(TeamTalkBase ttclient, int cmdid, int waittimeout, ServerInterleave interleave) {
        TTMessage msg = new TTMessage();

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_PROCESSING, waittimeout, msg, interleave))
        {
            if (msg.nSource == cmdid && msg.bActive == false)
                return true;
        }
        return false;
    }

    protected static boolean waitCmdSuccess(TeamTalkBase ttclient, int cmdid, int waittimeout) {
        return waitCmdSuccess(ttclient, cmdid, waittimeout, nop);
    }

    protected static boolean waitCmdSuccess(TeamTalkBase ttclient, int cmdid,
                                            int waittimeout, ServerInterleave interleave) {
        TTMessage msg = new TTMessage();

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, waittimeout, msg, interleave))
        {
            if (msg.nSource == cmdid)
                return true;
        }

        return false;
    }

    protected static boolean waitCmdError(TeamTalkBase ttclient, int cmdid, int waittimeout, TTMessage msg) {
        return waitCmdError(ttclient, cmdid, waittimeout, msg, nop);
    }

    protected static boolean waitCmdError(TeamTalkBase ttclient, int cmdid, int waittimeout, ServerInterleave interleave) {
        return waitCmdError(ttclient, cmdid, waittimeout, new TTMessage(), interleave);
    }
    protected static boolean waitCmdError(TeamTalkBase ttclient, int cmdid, int waittimeout, TTMessage msg, ServerInterleave interleave) {

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, waittimeout, msg, interleave))
        {
            if (msg.nSource == cmdid)
                return true;
        }

        return false;
    }

    protected static boolean waitCmdError(TeamTalkBase ttclient, int cmdid, int waittimeout)
    {
        TTMessage msg = new TTMessage();
        return waitCmdError(ttclient, cmdid, waittimeout, msg);
    }

    protected static Channel buildDefaultChannel(TeamTalkBase ttclient, String name) {
        return buildDefaultChannel(ttclient, name, Codec.OPUS_CODEC);
    }

    protected static Channel buildDefaultChannel(TeamTalkBase ttclient, String name, int codec) {
        Channel chan = new Channel();
        chan.nParentID = ttclient.getRootChannelID();
        chan.szName = name;
        chan.szTopic = "a topic";
        chan.nMaxUsers = 128;
        chan.audiocodec.nCodec = codec;
        switch(codec) {
            case Codec.OPUS_CODEC :
                chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
                chan.audiocodec.opus.nChannels = 1;
                chan.audiocodec.opus.nBitRate = 64000;
                chan.audiocodec.opus.nComplexity = 5;
                chan.audiocodec.opus.nTxIntervalMSec = 20;
                chan.audiocodec.opus.nSampleRate = 48000;
                chan.audiocodec.opus.bDTX = true;
                chan.audiocodec.opus.bFEC = true;
                chan.audiocodec.opus.bVBR = true;
                chan.audiocodec.opus.bVBRConstraint = false;
                break;
            case Codec.SPEEX_CODEC :
                chan.audiocodec.speex = new SpeexCodec(true);
                break;
            case Codec.SPEEX_VBR_CODEC :
                chan.audiocodec.speex_vbr = new SpeexVBRCodec(true);
                break;
        }
        return chan;
    }

    public static String getCurrentMethod()
    {
        return Thread.currentThread().getStackTrace()[2].getMethodName();
    }

    static boolean hasFlag(int flags, int flag) {
        return (flags & flag) == flag;
    }

    static void printSoundDevice(SoundDevice dev) {
        System.out.println("Sound dev " + Integer.toString(dev.nDeviceID) + ":");
        System.out.println("\tName:" + dev.szDeviceName);
        System.out.println("\tInput channels: " + Integer.toString(dev.nMaxInputChannels));
        System.out.println("\tOutput channels: " + Integer.toString(dev.nMaxOutputChannels));
        System.out.print("\tInput sample rates: ");
        for(int j=0;j<dev.inputSampleRates.length;j++)
            System.out.print(Integer.toString(dev.inputSampleRates[j]) + ", ");
        System.out.print("\tOutput sample rates: ");
        for(int j=0;j<dev.outputSampleRates.length;j++)
            System.out.print(Integer.toString(dev.outputSampleRates[j]) + ", ");
        System.out.println("");
        System.out.println("\tDefault sample rate: " + Integer.toString(dev.nDefaultSampleRate));
    }
}
