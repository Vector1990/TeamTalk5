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

import UIKit

class AppInfo {

    static let TTLINK_PREFIX = "tt://"

    static let OSTYPE = "iOS"

    static let DEFAULT_TCPPORT = 10333
    static let DEFAULT_UDPPORT = 10333
    
    static let WEBLOGIN_FACEBOOK = "facebook"
    static let WEBLOGIN_FACEBOOK_PASSWDPREFIX = "token="
    
    enum BundleInfo {
        case name, version_NO
    }
    
    static func getBundleInfo(_ b: BundleInfo) -> String {
        let bundle = Bundle.main
        let dict = bundle.infoDictionary

        switch b {
        case .name :
            if let info = dict?["CFBundleName"] {
                return info as! String
            }
            return "Unknown"
        case .version_NO :
            if let info = dict?["CFBundleShortVersionString"] {
                return info as! String
            }
            return "0.1"
        }
    }
    
    static func getAppName() -> String {
        return getBundleInfo(.name)
    }
    static func getAppVersion() -> String {
        return getBundleInfo(.version_NO)
    }
    
    static func getServersURL() -> String {
        return "http://www.bearware.dk/teamtalk/tt5servers.php?client=" + getAppName() +
            "&version=" + getAppVersion() +
            "&dllversion=" + TEAMTALK_VERSION + "&os=" + OSTYPE

    }

    static func getUpdateURL() -> String {
        return "http://www.bearware.dk/teamtalk/tt5update.php?client=" + getAppName() +
            "&version=" + getAppVersion() + "&dllversion=" + TEAMTALK_VERSION + "&os=" + OSTYPE

    }
}
