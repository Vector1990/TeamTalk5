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

import UIKit

class ServerDetailViewController : UITableViewController, UITextFieldDelegate {

    var server = Server()

    var nameItems = [UITableViewCell]()
    var conItems = [UITableViewCell]()
    var authItems = [UITableViewCell]()
    var chanItems = [UITableViewCell]()
    var actionItems = [UITableViewCell]()
    
    var namefield : UITextField?
    var ipaddrfield : UITextField?
    var tcpportfield : UITextField?
    var udpportfield : UITextField?
    var usernamefield : UITextField?
    var passwdfield : UITextField?
    var fbloginfield : UISwitch?
    var chanfield : UITextField?
    var chpasswdfield : UITextField?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // ServerList Entry section
        let namecell = UITableViewCell(style: .default, reuseIdentifier: nil)
        namefield = newTableCellTextField(namecell, label: NSLocalizedString("Name", comment: "server entry"), initial: server.name)
        namefield!.delegate = self
        nameItems.append(namecell)

        // Connection section
        let ipaddrcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        ipaddrfield = newTableCellTextField(ipaddrcell, label: NSLocalizedString("Host address", comment: "server entry"), initial: server.ipaddr)
        ipaddrfield!.delegate = self
        ipaddrfield!.keyboardType = .URL
        ipaddrfield!.spellCheckingType = .no
        ipaddrfield!.autocorrectionType = .no
        ipaddrfield!.autocapitalizationType = .none
        conItems.append(ipaddrcell)

        let tcpportcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        tcpportfield = newTableCellTextField(tcpportcell, label: NSLocalizedString("TCP Port", comment: "server entry"), initial: String(server.tcpport))
        tcpportfield!.delegate = self
        tcpportfield!.keyboardType = .numberPad
        conItems.append(tcpportcell)

        let udpportcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        udpportfield = newTableCellTextField(udpportcell, label: NSLocalizedString("UDP Port", comment: "server entry"), initial: String(server.udpport))
        udpportfield!.delegate = self
        udpportfield!.keyboardType = .numberPad
        conItems.append(udpportcell)

        // create auth items
        refreshAuthorizationItems(facebook: self.server.username == AppInfo.WEBLOGIN_FACEBOOK)
        
        //initial channel
        let chancell = UITableViewCell(style: .default, reuseIdentifier: nil)
        chanfield = newTableCellTextField(chancell, label: NSLocalizedString("Channel", comment: "server entry"), initial: server.channel)
        chanfield!.delegate = self
        chanfield!.autocorrectionType = .no
        chanfield!.spellCheckingType = .no
        chanfield!.autocapitalizationType = .none
        chanItems.append(chancell)

        let chpasswdcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        chpasswdfield = newTableCellTextField(chpasswdcell, label: NSLocalizedString("Password", comment: "server entry"), initial: server.chanpasswd)
        chpasswdfield!.delegate = self
        chpasswdfield!.autocorrectionType = .no
        chpasswdfield!.spellCheckingType = .no
        chpasswdfield!.autocapitalizationType = .none
        chpasswdfield!.isSecureTextEntry = true
        chanItems.append(chpasswdcell)

        let connectcell = tableView.dequeueReusableCell(withIdentifier: "Connect Server")!
        actionItems.append(connectcell)
        
        let deletecell = tableView.dequeueReusableCell(withIdentifier: "Delete Server")!
        actionItems.append(deletecell)
        
        tableView.dataSource = self
        tableView.delegate = self
        
        let def = NotificationCenter.default
        
        def.addObserver(self, selector: #selector(ServerDetailViewController.keyboardWillShow(_:)), name: NSNotification.Name.UIKeyboardWillShow, object: nil)
        def.addObserver(self, selector: #selector(ServerDetailViewController.keyboardWillHide(_:)), name: NSNotification.Name.UIKeyboardWillHide, object: nil)
    }
    
    @objc func keyboardWillShow(_ notify: Notification) {
        moveForKeyboard(notify, up: true)
    }
    
    @objc func keyboardWillHide(_ notify: Notification) {
        moveForKeyboard(notify, up: false)
    }

    func moveForKeyboard(_ notify: Notification, up: Bool) {
        if let userInfo = notify.userInfo {
            if let keyboardFrame = (userInfo[UIKeyboardFrameEndUserInfoKey] as AnyObject).cgRectValue {
                
                let selfFrame = self.view.frame
                var newTableFrame = tableView.frame
                
                if up {
                    newTableFrame.size.height = selfFrame.height - keyboardFrame.height
                }
                else {
                    var tabBarHeight : CGFloat = 0.0
                    if self.tabBarController != nil {
                        tabBarHeight = (self.tabBarController?.tabBar.frame.size.height)!
                    }
                    
                    newTableFrame.size.height = selfFrame.height - tabBarHeight
                }
                
                tableView.frame = newTableFrame
            }
        }
    }

    func refreshAuthorizationItems(facebook: Bool) {
        self.authItems.removeAll()
        
        // Authentication section
        let usernamecell = UITableViewCell(style: .default, reuseIdentifier: nil)
        usernamefield = newTableCellTextField(usernamecell, label: NSLocalizedString("Username", comment: "server entry"), initial: facebook ? AppInfo.WEBLOGIN_FACEBOOK : server.username)
        usernamefield!.delegate = self
        usernamefield!.autocorrectionType = .no
        usernamefield!.spellCheckingType = .no
        usernamefield!.autocapitalizationType = .none
        if facebook == false {
            authItems.append(usernamecell)
        }
        
        let passwdcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        passwdfield = newTableCellTextField(passwdcell, label: NSLocalizedString("Password", comment: "server entry"), initial: facebook ? "" : server.password)
        passwdfield!.delegate = self
        passwdfield!.autocorrectionType = .no
        passwdfield!.spellCheckingType = .no
        passwdfield!.autocapitalizationType = .none
        passwdfield!.isSecureTextEntry = true
        if facebook == false {
            authItems.append(passwdcell)
        }
        
        let fbcell = UITableViewCell(style: .default, reuseIdentifier: nil)
        fbloginfield = newTableCellSwitch(fbcell, label: NSLocalizedString("Facebook Login", comment: "server entry"), initial: facebook)
        fbloginfield?.addTarget(self, action: #selector(facebookLogin(_:)), for: .valueChanged)
        authItems.append(fbcell)
    }
    
    func saveServerDetail() {
        server.name = namefield!.text!
        server.ipaddr = ipaddrfield!.text!
        if let n = Int(tcpportfield!.text!) {
            server.tcpport = n
        }
        if let n = Int(udpportfield!.text!) {
            server.udpport = n
        }
        server.username = usernamefield!.text!
        server.password = passwdfield!.text!
        server.publicserver = false
        server.channel = chanfield!.text!
        server.chanpasswd = chpasswdfield!.text!
    }
    
    func textFieldDidBeginEditing(_ textfield: UITextField) {
        let cell = textfield.superview as! UITableViewCell
        tableView.scrollToRow(at: tableView.indexPath(for: cell)!, at: .top, animated: true)
    }
    
    //    func textFieldShouldEndEditing(textfield: UITextField) -> Bool {
    //        return true
    //    }
    
    func textFieldShouldReturn(_ textfield: UITextField) -> Bool {
        textfield.resignFirstResponder()
        return false
    }
    
    @objc func facebookLogin(_ sender: UISwitch) {
        refreshAuthorizationItems(facebook: sender.isOn)
        tableView.reloadData()
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        return 5
    }
    
    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch section {
        case 0 :
            return NSLocalizedString("Server List Entry", comment: "server entry")
        case 1 :
            return NSLocalizedString("Connection", comment: "server entry")
        case 2 :
            return NSLocalizedString("Authentication", comment: "server entry")
        case 3 :
            return NSLocalizedString("Join Channel", comment: "server entry")
        case 4 :
            return NSLocalizedString("Actions", comment: "server entry")
        default :
            return nil
        }
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        
        switch section {
        case 0 :
            return nameItems.count
        case 1 :
            return conItems.count
        case 2 :
            return authItems.count
        case 3 :
            return chanItems.count
        case 4 :
            return actionItems.count
        default :
            return 0
        }
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        
        switch indexPath.section {
        case 0 :
            return nameItems[indexPath.row]
        case 1 :
            return conItems[indexPath.row]
        case 2 :
            return authItems[indexPath.row]
        case 3 :
            return chanItems[indexPath.row]
        case 4 :
            return actionItems[indexPath.row]
        default :
            break
        }
        
        return UITableViewCell()
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "Connect From ServerDetail" {
            
            saveServerDetail()
            
            let vc = segue.destination as! MainTabBarController
            vc.setTeamTalkServer(server)
        }
    }
}
