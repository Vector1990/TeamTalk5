/*
 * Copyright (c) 2005-2017, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

package dk.bearware.events;

import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.TextMessage;
import dk.bearware.User;
import dk.bearware.UserAccount;
import dk.bearware.BannedUser;

public interface CommandListener {

    public void onCmdError(int cmdId, ClientErrorMsg errmsg);
    public void onCmdSuccess(int cmdId);
    public void onCmdProcessing(int cmdId, boolean complete);
    
    public void onCmdMyselfLoggedIn(int my_userid, UserAccount useraccount);
    public void onCmdMyselfLoggedOut();
    public void onCmdMyselfKickedFromChannel();
    public void onCmdMyselfKickedFromChannel(User kicker);
    
    public void onCmdUserLoggedIn(User user);
    public void onCmdUserLoggedOut(User user);
    public void onCmdUserUpdate(User user);
    
    public void onCmdUserJoinedChannel(User user);
    public void onCmdUserLeftChannel(int channelid, User user);
    
    public void onCmdUserTextMessage(TextMessage textmessage);
 
    public void onCmdChannelNew(Channel channel);
    public void onCmdChannelUpdate(Channel channel);
    public void onCmdChannelRemove(Channel channel);
    
    public void onCmdServerUpdate(ServerProperties serverproperties);
    
    public void onCmdFileNew(RemoteFile remotefile);
    public void onCmdFileRemove(RemoteFile remotefile);

    public void onCmdUserAccount(UserAccount useraccount);
    public void onCmdBannedUser(BannedUser banneduser);
}
    
