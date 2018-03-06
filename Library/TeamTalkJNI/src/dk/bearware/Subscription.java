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

public interface Subscription {
    public static final int SUBSCRIBE_NONE                    = 0x00000000;
    public static final int SUBSCRIBE_USER_MSG                = 0x00000001;
    public static final int SUBSCRIBE_CHANNEL_MSG             = 0x00000002;
    public static final int SUBSCRIBE_BROADCAST_MSG           = 0x00000004;
    public static final int SUBSCRIBE_CUSTOM_MSG              = 0x00000008;
    public static final int SUBSCRIBE_VOICE                   = 0x00000010;
    public static final int SUBSCRIBE_VIDEOCAPTURE            = 0x00000020;
    public static final int SUBSCRIBE_DESKTOP                 = 0x00000040;
    public static final int SUBSCRIBE_DESKTOPINPUT            = 0x00000080;
    public static final int SUBSCRIBE_MEDIAFILE               = 0x00000100;

    public static final int SUBSCRIBE_INTERCEPT_USER_MSG      = 0x00010000;
    public static final int SUBSCRIBE_INTERCEPT_CHANNEL_MSG   = 0x00020000;
    public static final int SUBSCRIBE_INTERCEPT_CUSTOM_MSG    = 0x00080000;
    public static final int SUBSCRIBE_INTERCEPT_VOICE         = 0x00100000;
    public static final int SUBSCRIBE_INTERCEPT_VIDEOCAPTURE  = 0x00200000;
    public static final int SUBSCRIBE_INTERCEPT_DESKTOP       = 0x00400000;
    public static final int SUBSCRIBE_INTERCEPT_MEDIAFILE     = 0x01000000;
}
