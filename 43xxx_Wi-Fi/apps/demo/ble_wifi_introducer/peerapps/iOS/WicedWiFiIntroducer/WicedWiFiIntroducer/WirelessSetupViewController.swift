//
//  ViewController.swift
//  SimpleTableView
//
//  Created by Andrei Puni on 25/12/14.
//  Copyright (c) 2014 Andrei Puni. All rights reserved.
//

import UIKit
import CoreBluetooth


protocol WirelessSetupViewControllerDelegate {
    func setUpComplete(data: Bool)
}

class WirelessSetupViewController: UIViewController, UITextFieldDelegate, ConnectPeripheralProtocol, ReadPeripheralProtocol{
    
    var serviceUUIDString:String        = "1B7E8251-2877-41C3-B46E-CF057C562023"
    var peripheralUUIDString:String     = "7EC612F7-F388-0284-624C-BE76C314C6CD"
    var characteristicUUIDString:String = "B6251F0B-3869-4C0D-ACAB-D93F45187E6F"
    
    
    @IBOutlet weak var deviceText: UILabel!
    @IBOutlet weak var WifiNetwrkText: UILabel!
    @IBOutlet weak var passwordText: UITextField!
    //@IBOutlet weak var ConnectButton: UIButton!

    @IBOutlet weak var Switch: UISwitch!


    var securityMode : Int?
    var ssidName : String?
    var passphraseToken : String?
    var delegate: WirelessSetupViewControllerDelegate?

    @IBOutlet weak var deviceName: UILabel!
    var items: [String] = ["WPA2-TKIP", "WPA2-AES", "WPA2-MIXED"]

    override func viewDidLoad() {
        
        super.viewDidLoad()
        
        
        // not sure all the BLE devices will be deteced by this call
        CentralManager.sharedInstance().connectPeripheralDelegate = self
        var devName = "BRCM Wiced Device"
        for peripheral:Peripheral in DataManager.sharedInstance().discoveredPeripherals.values {
                devName = peripheral.name
        }
        if devName != "BRCM Wiced Device" {
            WifiNetwrkText.text = "\u{2022} \(devName)";
            //deviceName.text = devName
        }

        let ssidName1 = SSIDNetwork()
        ssidName = "WIFI NTW"
        Switch.on = false;
        Switch.tintColor = UIColor.whiteColor()
        Switch.backgroundColor = UIColor.whiteColor()
        Switch.layer.cornerRadius = 16.0
        let temp = ssidName1.getNetworkSSID()
        if temp != nil {

            deviceText.text = "\u{2022} \(temp)";
            ssidName = temp
            
        }
        let tap: UITapGestureRecognizer = UITapGestureRecognizer(target: self, action: "dissmissKeyboard");
        view.addGestureRecognizer(tap)
        passwordText.layer.sublayerTransform = CATransform3DMakeTranslation(10, 0, 0)

    }

    @IBAction func Connect(sender: AnyObject) {
        print("im getting selected :) ")
        print(passwordText.text)
        for peripheral:Peripheral in DataManager.sharedInstance().discoveredPeripherals.values {
                    CentralManager.sharedInstance().connectPeripheral(peripheral)
        }

    }

    @IBAction func showPasswordChange(sender: AnyObject) {
        if Switch.on {
            passwordText.resignFirstResponder()
            passwordText.secureTextEntry = false
        } else {
            Switch.tintColor = UIColor.whiteColor()
            Switch.backgroundColor = UIColor.whiteColor()
            Switch.layer.cornerRadius = 16.0
            passwordText.secureTextEntry = true
        }
    }

    func dissmissKeyboard() {
        view.endEditing(true)
    }

    func textFieldShouldReturn(textField: UITextField) -> Bool {
        // Hide the keyboard.
        textField.resignFirstResponder()
        return true
    }
    
    @IBAction func passwordFiledTouchOutside(sender: AnyObject) {
        passwordText.endEditing(true)
        passwordText.resignFirstResponder()
    }
    @IBAction func PasswordEditingFinished(sender: AnyObject) {
        passwordText.resignFirstResponder()
    }


    // MARK: ConnectPeripheralProtocol
    func didConnectPeripheral(cbPeripheral: CBPeripheral!) {
        
        Logger.debug("AppDelegate#didConnectPeripheral \(cbPeripheral.name)")
        
        if let peripheral = DataManager.sharedInstance().discoveredPeripherals[cbPeripheral] {
            
            // look for only main apollo service
            peripheral.discoverServices([CBUUID(string: serviceUUIDString)], delegate: self)
            //peripheral.discoverServices(nil, delegate: self)
            
        }
        
    }
    
    func didDisconnectPeripheral(cbPeripheral: CBPeripheral!, error: NSError!, userClickedCancel: Bool) {
        Logger.debug("AppDelegate#didDisconnectPeripheral \(cbPeripheral.name)")
    }
    
    func didRestorePeripheral(peripheral: Peripheral) {
        Logger.debug("AppDelegate#didRestorePeripheral \(peripheral.name)")
    }
    
    func bluetoothBecomeAvailable() {
        //self.startScanning()
    }
    
    func bluetoothBecomeUnavailable() {
        //self.stopScanning()
    }
    
    
    // MARK: ReadPeripheralProtocol
    
    func didDiscoverCharacteristicsofPeripheral(cbservice : CBService!) {
        print("didDiscoverCharacteristicsofPeripheral  \(cbservice.characteristics)")
        
        var peripheral : Peripheral?
        
        if let p = DataManager.sharedInstance().discoveredPeripherals[cbservice.peripheral] {
            peripheral = p
        }
        else {
            
        }
        
        //if peripheral?.isSender == true {
            
            for charateristic in cbservice.characteristics! {
                let thisCharacteristic = charateristic as CBCharacteristic
                print("UUID == \(thisCharacteristic.UUID)")
                switch thisCharacteristic.UUID {
                    
                case WiFiIntroducerConfigCharNotifyValueUUID:
                    print("WiFiIntroducerConfigCharNotifyValueUUID")
                    thisCharacteristic.service.peripheral.setNotifyValue(true, forCharacteristic: thisCharacteristic)
                    break

                case WiFiIntroducerConfigNWSecurityUUID:
                    
                    print("WiFiIntroducerConfigNWSecurityUUID")
                    
                    securityMode = 2
                    
                    switch securityMode! {
                        
                    case 0:
                        print("WPA2-TKIP")
                        
                    case 1:
                        print("WPA2-AES")
                        
                    case 2:
                        print("WPA2-MIXED")
                        
                    default:
                        print("unknown value for security mode")
                        break
                        
                    }
                    
                    var enableValue = securityMode!
                    let enablyBytes = NSData(bytes: &enableValue, length: sizeof(UInt8))
                    
                    print("value being written \(enablyBytes)")
                    
                    thisCharacteristic.service.peripheral.readValueForCharacteristic(thisCharacteristic)
                    
                    sleep(1)
                    
                    print(thisCharacteristic.value)
                    
                    thisCharacteristic.service.peripheral.writeValue(enablyBytes, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    
                    break
                    
                case WiFiIntroducerConfigNWSSIDUUID:
                    
                    print("WiFiIntroducerConfigNWSSIDUUID")
                    
                    
                    thisCharacteristic.service.peripheral.readValueForCharacteristic(thisCharacteristic)
                    
                    sleep(1)
                    
                    let str = ssidName!
                    
                    print(str)
                    
                    // this celltext needs to be converted to NSData and then fed
                    let data = str.dataUsingEncoding(NSUTF8StringEncoding)
                    
                    thisCharacteristic.service.peripheral.readValueForCharacteristic(thisCharacteristic)
                    
                    print(thisCharacteristic.value)
                    
                    if let d = data {
                        thisCharacteristic.service.peripheral.writeValue(d, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    }
                    
                    thisCharacteristic.service.peripheral.readValueForCharacteristic(thisCharacteristic)
                    
                    if let d = data {
                        thisCharacteristic.service.peripheral.writeValue(d, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    }
                    break
                case WiFiIntroducerConfigNWPassphraseUUID:
                    
                    print("WiFiIntroducerConfigNWPassphraseUUID")
                    
                    let str = passwordText.text!
                    
                    print(str)
                    
                    
                    thisCharacteristic.service.peripheral.readValueForCharacteristic(thisCharacteristic)
                    
                    sleep(1)
                    
                    
                    // this celltext needs to be converted to NSData and then fed
                    let data = str.dataUsingEncoding(NSUTF8StringEncoding)
                    
                    thisCharacteristic.service.peripheral.readValueForCharacteristic(thisCharacteristic)
                    
                    //print(thisCharacteristic.value)
                    
                    if let d = data {
                        thisCharacteristic.service.peripheral.writeValue(d, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    }
                    break
                    
                default:
                    _ = 0
                    
                }
                
            }
            
        //}
        //CentralManager.sharedInstance().cancelPeripheralConnection(peripheral!, userClickedCancel: true);
        
        //self.submitButton.setTitle("Configured!", forState: UIControlState.Normal)
    }
    
    func didWriteValueForCharacteristic(cbPeripheral: CBPeripheral!, characteristic: CBCharacteristic!, error: NSError!) {
        print("didWriteValueForCharacteristic")
    }
    
    
    func didUpdateValueForCharacteristic(cbPeripheral: CBPeripheral!, characteristic: CBCharacteristic!, error: NSError!) {
        print("didUpdateValueForCharacteristic  characteristic.UUID = \(characteristic.UUID)  value = \(characteristic.value)")
        if characteristic.UUID == WiFiIntroducerConfigCharNotifyValueUUID {
            print(characteristic.value)

 
            // Convert NSData to array of signed 16 bit values
            let dataBytes = characteristic.value
            let dataLength = dataBytes!.length
            var dataArray = [Int16](count: dataLength, repeatedValue: 0)
            dataBytes!.getBytes(&dataArray, length: dataLength * sizeof(Int16))

            let temp = dataArray[0];

            // Display on the temp label
            // = NSString(format: "%.2f", ambientTemperature)

            print("value = **** \(temp )") // Element 1 of the array will be ambient temperature raw value
            
            
            var peripheral : Peripheral?
            
            if let p = DataManager.sharedInstance().discoveredPeripherals[cbPeripheral] {
                peripheral = p
            }
            
            CentralManager.sharedInstance().cancelPeripheralConnection(peripheral!, userClickedCancel: true);
           

            if temp == 1 {
           
                notifyCallingViewController(true)
                
            } else if temp == 0 {
                
               notifyCallingViewController(false)
            }
        }
    }


    func notifyCallingViewController(val:Bool) {
        print("notifyCallingViewController setUpComplete")
        self.delegate?.setUpComplete(val)
    }
}

