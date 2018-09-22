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
import AVFoundation

class SpeechViewController : UITableViewController {
    
    var sections = [String]()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        tableView.dataSource = self
        tableView.delegate = self
        
        sections.append(AVSpeechSynthesisVoice.currentLanguageCode())
        
        for voice in AVSpeechSynthesisVoice.speechVoices() {
            if sections.contains(voice.language) == false {
                sections.append(voice.language)
            }
        }
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        
//        let defaults = NSUserDefaults.standardUserDefaults()
//        var lang = AVSpeechSynthesisVoice.currentLanguageCode()
//        if let voiceid = defaults.stringForKey(PREF_TTSEVENT_VOICEID) {
//            if #available(iOS 9.0, *) {
//                if let voice = AVSpeechSynthesisVoice(identifier: voiceid) {
//                    let voices = getVoices(voice.language)
//                    if let row = voices.indexOf(voice) {
//                        let index = NSIndexPath(forRow: row, inSection: sections.indexOf(voice.language))
//                    }
//                }
//            } else {
//                // Fallback on earlier versions
//            }
//            
//        }
//        else if let voicelang = defaults.stringForKey(PREF_TTSEVENT_VOICELANG) {
//            lang = voicelang
//        }
//        else {
//            let voices = getVoices(lang)
//        }
//        
//        voices.
//        if voiceid != nil {
//            
//        }
//        else {
//            
//            let i = IndexPath(indexPathForRow: 0, inSection
//            tableView.selectRowAtIndexPath(IndexPath(, animated: <#T##Bool#>, scrollPosition: <#T##UITableViewScrollPosition#>)
//        }
        
    }
    
    func getVoices(_ language: String) -> [AVSpeechSynthesisVoice] {
        let voices = AVSpeechSynthesisVoice.speechVoices().filter({$0.language == language})
        return voices
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        return sections.count
    }
    
    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        return sections[section]
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return getVoices(sections[section]).count
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let language = sections[indexPath.section]
        let localname = (Locale.current as NSLocale).displayName(forKey: NSLocale.Key.identifier, value: language)
        let voices = getVoices(language)
        let cell = tableView.dequeueReusableCell(withIdentifier: "Voice Cell")
        if #available(iOS 9.0, *) {
            cell?.textLabel?.text = voices[indexPath.row].name
        } else {
            cell?.textLabel?.text = localname
        }
        cell?.detailTextLabel!.text = localname
        return cell!
    }
    
    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        let voices = getVoices(sections[indexPath.section])
        let voice = voices[indexPath.row]
        let defaults = UserDefaults.standard
        
        if #available(iOS 9.0, *) {
            defaults.setValue(voice.identifier, forKey: PREF_TTSEVENT_VOICEID)
            
            let utterance = String(format: NSLocalizedString("You have selected %@" , comment: "speech"), voice.name)
            newUtterance(utterance)

        } else {
            defaults.setValue(voice.language, forKey: PREF_TTSEVENT_VOICELANG)
            
            if let localname = (Locale.current as NSLocale).displayName(forKey: NSLocale.Key.identifier, value: voice.language) {
                let utterance = String(format: NSLocalizedString("You have selected %@" , comment: "speech"), localname)
                newUtterance(utterance)
            }
        }

    }


}
