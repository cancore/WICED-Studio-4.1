//
//  SimpleViewController.swift
//  apollo_config

/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */
 
import UIKit
import CoreBluetooth

class SimpleViewController: UIViewController, ConnectPeripheralProtocol, ReadPeripheralProtocol {

    var serviceUUIDString:String        = "04574543-9CBA-4B22-BC78-55040A01C246"
    var peripheralUUIDString:String     = "76B1FDBA-FE17-2A00-0000-000000000000"
    var characteristicUUIDString:String = "B6251F0B-3869-4C0D-ACAB-D93F45187E6F"
    
    var data: DataManager!
    var discoveredPeripherals : Dictionary<CBPeripheral, Peripheral> = [:]
    var senderDiscovered : Bool = false
    var timer = NSTimer()
    var writeCount = 0
    var sourceCharacteristicCount  = 5
    var speakerCharacteristicCount = 6
    var firstPeripheralCount = 0;

    @IBOutlet weak var statusButton: UIButton!
    @IBOutlet weak var statusLabel: UILabel!

    override func viewDidLoad() {
        super.viewDidLoad()
        
        statusButton.titleLabel?.lineBreakMode = NSLineBreakMode.ByWordWrapping
        statusButton.titleLabel?.textAlignment = NSTextAlignment.Center
        
        // not sure all the BLE devices will be deteced by this call
        initBluetooth()
        
        statusLabel.text = "Scanning"
        
        timer = NSTimer.scheduledTimerWithTimeInterval(10, target:self, selector: Selector("updateCounter"), userInfo: nil, repeats: true)
        
        while(discoveredPeripherals.count == 0) {
            
            sleep(1)
        }
      
        // Do any additional setup after loading the view.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    func updateCounter() {
        if senderDiscovered == false {
           self.startScanning()
        }
        else {
           timer.invalidate()
        }
    }

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        // Get the new view controller using segue.destinationViewController.
        // Pass the selected object to the new view controller.
    }
    */
    
    
    @IBAction func statusButtonClick(sender: AnyObject) {
        
        for peripheral:Peripheral in self.discoveredPeripherals.values {
            
            CentralManager.sharedInstance().connectPeripheral(peripheral)
        }
    }
    
    func isPeripheralA2DPConnected(peripheral : Peripheral) -> Bool {
        
        self.senderDiscovered = false
        
        let advertvalues = peripheral.advertisements["kCBAdvDataManufacturerData"]
        
        if advertvalues != nil {
            
            // the number of elements:
            let count = advertvalues!.length / sizeof(UInt8)
            
            // create array of appropriate length:
            var array = [UInt8](count: count, repeatedValue: 0)
            
            // copy bytes into array
            advertvalues!.getBytes(&array, length:count * sizeof(UInt8))
            
            if array[array.count-1] == 3 {

                self.senderDiscovered = true
            }
        }
        
        return self.senderDiscovered
    }
    
    
    func isPeripheralConfigured(peripheral : Peripheral) -> Bool {
        
        var configured = false
        
        let advertvalues = peripheral.advertisements["kCBAdvDataManufacturerData"]
        
        if advertvalues != nil {
            
            // the number of elements:
            let count = advertvalues!.length / sizeof(UInt8)
            
            // create array of appropriate length:
            var array = [UInt8](count: count, repeatedValue: 0)
            
            // copy bytes into array
            advertvalues!.getBytes(&array, length:count * sizeof(UInt8))
            
            if array[array.count-1] != 0 {

                configured = true
            }
            
        }
        
        return configured
    }
    
    
    
    func initBluetooth() {
        
        CentralManager.sharedInstance().connectPeripheralDelegate = self
    }
    
    // MARK: ConnectPeripheralProtocol
    func didConnectPeripheral(cbPeripheral: CBPeripheral!) {
        
        Logger.debug("AppDelegate#didConnectPeripheral \(cbPeripheral.name)")
        if let peripheral = self.discoveredPeripherals[cbPeripheral] {

            // look for only main apollo service
            peripheral.discoverServices([CBUUID(string: serviceUUIDString)], delegate: self)
            
        }
        
    }
    
    func didDisconnectPeripheral(cbPeripheral: CBPeripheral!, error: NSError!, userClickedCancel: Bool) {
        Logger.debug("AppDelegate#didDisconnectPeripheral \(cbPeripheral.name)")
    }
    
    func didRestorePeripheral(peripheral: Peripheral) {
        Logger.debug("AppDelegate#didRestorePeripheral \(peripheral.name)")
    }
    
    func bluetoothBecomeAvailable() {
        self.startScanning()
    }
    
    func bluetoothBecomeUnavailable() {
        self.stopScanning()
    }
    
    
    // MARK: Public functions
    func startScanning() {
        for peripheral:Peripheral in self.discoveredPeripherals.values {
            peripheral.isNearby = false
        }
        CentralManager.sharedInstance().startScanning(afterPeripheralDiscovered, allowDuplicatesKey: true)
    }
    
    func stopScanning() {
        CentralManager.sharedInstance().stopScanning()
    }
    
    func afterPeripheralDiscovered(cbPeripheral:CBPeripheral, advertisementData:NSDictionary, RSSI:NSNumber) {
        
        Logger.debug("AppDelegate#afterPeripheralDiscovered: \(cbPeripheral)")
        
        var peripheral : Peripheral
        
        if let p = discoveredPeripherals[cbPeripheral] {
            
            peripheral = p
            
        }
        else {
            
            peripheral = Peripheral(cbPeripheral:cbPeripheral, advertisements:advertisementData as Dictionary<NSObject, AnyObject>, rssi:RSSI.integerValue)
            
            dispatch_async(dispatch_get_main_queue(), {
                
                self.statusLabel.text = "Speaker device found!"
                //self.statusLabel.text = "BT relay enabled speaker was found"
                self.statusButton.setTitle("Want to set up speaker network?", forState: UIControlState.Normal)
                //self.senderDiscovered = true
                if self.firstPeripheralCount++ == 0 {
                    peripheral.isSender = true
                }
            })
            
            discoveredPeripherals[peripheral.cbPeripheral] = peripheral
            
        }
        
        peripheral.isNearby = true

        NSNotificationCenter.defaultCenter().postNotificationName("afterPeripheralDiscovered", object: nil)
    }
    
    // MARK: Public Methods
    
    // Check if the characteristic has a valid data UUID
    func validDataCharacteristic (characteristic : CBCharacteristic) -> Bool {
        if characteristic.UUID == ApolloConfigNWModeUUID || characteristic.UUID == ApolloConfigNWSSIDUUID ||
            characteristic.UUID == ApolloConfigNWSecurityUUID || characteristic.UUID == ApolloConfigNWPassphraseUUID ||
            characteristic.UUID == ApolloConfigSpeakerNameUUID || characteristic.UUID == ApolloConfigSpeakerChannelUUID ||
            characteristic.UUID == ApolloConfigSpeakerOutputVolumeUUID || characteristic.UUID == ApolloConfigSourceInputUUID ||
            characteristic.UUID == ApolloConfigSourceInputVolumeUUID {
                return true
        }
        else {
            return false
        }
    }
    
    
    // MARK: ReadPeripheralProtocol
    
    func didDiscoverCharacteristicsofPeripheral(cbservice : CBService!) {
        
        var peripheral : Peripheral?
        
        if let p = discoveredPeripherals[cbservice.peripheral] {
            peripheral = p
        }
        else {
            
        }

        if peripheral?.isSender == true {
            
            for charateristic in cbservice.characteristics! {
                let thisCharacteristic = charateristic as CBCharacteristic
                
                print(thisCharacteristic)
                
                switch thisCharacteristic.UUID {
                    
                case ApolloConfigNWModeUUID:
                    
                    print("ApolloConfigNWModeUUID")
                    
                    var enableValue = 0
                    let enablyBytes = NSData(bytes: &enableValue, length: sizeof(UInt8))
                    
                    print("value being written \(enablyBytes)")
                    
                    thisCharacteristic.service.peripheral.readValueForCharacteristic(thisCharacteristic)
                    
                    print(thisCharacteristic.value)
                    
                    thisCharacteristic.service.peripheral.writeValue(enablyBytes, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    
                    usleep(1000*1000)
                    
                    thisCharacteristic.service.peripheral.writeValue(enablyBytes, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    
                case ApolloConfigNWSSIDUUID:
                    
                    print("ApolloConfigNWSSIDUUID")
                    
                   let str = "APOLLO"
                    
                    // this celltext needs to be converted to NSData and then fed
                    let data = str.dataUsingEncoding(NSUTF8StringEncoding)
                    
                    if let d = data {
                        thisCharacteristic.service.peripheral.writeValue(d, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    }
                    
                case ApolloConfigNWPassphraseUUID:
                    
                    print("ApolloConfigNWPassphraseUUID")
                    
                    let str = "12345678"
                    
                    // this celltext needs to be converted to NSData and then fed
                    let data = str.dataUsingEncoding(NSUTF8StringEncoding)
                    
                    if let d = data {
                        thisCharacteristic.service.peripheral.writeValue(d, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    }
                    
                case ApolloConfigSourceInputUUID:
                    
                    print("ApolloConfigSourceInputUUID")
                    
                    var enableValue = 1
                    let enablyBytes = NSData(bytes: &enableValue, length: sizeof(UInt8))
                    
                    thisCharacteristic.service.peripheral.writeValue(enablyBytes, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    
                case ApolloConfigSourceInputVolumeUUID:
                    
                    print("ApolloConfigSourceInputVolumeUUID")
                    
                    var enableValue = 50
                    let enablyBytes = NSData(bytes: &enableValue, length: sizeof(UInt8))
                    
                    thisCharacteristic.service.peripheral.writeValue(enablyBytes, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    
                default:
                    _ = 0
                    
                }
                
            }
            
        } // set speaker characteristics
        else {
            
            for charateristic in cbservice.characteristics! {
                let thisCharacteristic = charateristic as CBCharacteristic
                
                switch thisCharacteristic.UUID {
                    
                case ApolloConfigNWModeUUID:
                    
                    print("ApolloConfigNWModeUUID")

                    
                    var enableValue = 1
                    let enablyBytes = NSData(bytes: &enableValue, length: sizeof(UInt8))
                    
                    print("value being written \(enablyBytes)")
                    
                    thisCharacteristic.service.peripheral.readValueForCharacteristic(thisCharacteristic)
                    
                    print(thisCharacteristic.value)
                    
                    thisCharacteristic.service.peripheral.writeValue(enablyBytes, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    
                    usleep(1000*1000)
                    
                    thisCharacteristic.service.peripheral.writeValue(enablyBytes, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)

                    
                case ApolloConfigNWSSIDUUID:
                    
                    print("ApolloConfigNWSSIDUUID")
                    
                    let str = "APOLLO"
                    
                    // this celltext needs to be converted to NSData and then fed
                    let data = str.dataUsingEncoding(NSUTF8StringEncoding)
                    
                    if let d = data {
                        thisCharacteristic.service.peripheral.writeValue(d, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    }
                    
                case ApolloConfigNWPassphraseUUID:
                    
                    print("ApolloConfigNWPassphraseUUID")
                    
                    let str = "12345678"
                    
                    // this celltext needs to be converted to NSData and then fed
                    let data = str.dataUsingEncoding(NSUTF8StringEncoding)
                    
                    if let d = data {
                        thisCharacteristic.service.peripheral.writeValue(d, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    }
                    
                case ApolloConfigSpeakerNameUUID:
                    
                    print("ApolloConfigSpeakerNameUUID")
                    
                    let str = "Left Speaker"
                    
                    // this celltext needs to be converted to NSData and then fed
                    let data = str.dataUsingEncoding(NSUTF8StringEncoding)
                    
                    if let d = data {
                        thisCharacteristic.service.peripheral.writeValue(d, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    }
                    
                case ApolloConfigSpeakerChannelUUID:
                    
                    print("ApolloConfigSpeakerChannelUUID")
                    
                    var enableValue = 3
                    let enablyBytes = NSData(bytes: &enableValue, length: sizeof(UInt8))
                    
                    thisCharacteristic.service.peripheral.writeValue(enablyBytes, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)
                    
                case ApolloConfigSpeakerOutputVolumeUUID:
                    
                    print("ApolloConfigSpeakerOutputVolumeUUID")
                    
                    var enableValue = 50
                    let enablyBytes = NSData(bytes: &enableValue, length: sizeof(UInt8))
                    
                    thisCharacteristic.service.peripheral.writeValue(enablyBytes, forCharacteristic: thisCharacteristic, type: CBCharacteristicWriteType.WithResponse)

                    
                default:
                    _ = 0
                }
                
            }
            
        }
        
    }
    
    func didWriteValueForCharacteristic(cbPeripheral: CBPeripheral!, characteristic: CBCharacteristic!, error: NSError!) {
        
        //print(error)

        if let peripheral = self.discoveredPeripherals[cbPeripheral] {
            
            if peripheral.isSender == true {
            
            if peripheral.characteristicWriteCount == sourceCharacteristicCount-1 {
                
                Logger.debug("AppDelegate#didUpdateValueForCharacteristic: Cancel peripheral connection")
                CentralManager.sharedInstance().cancelPeripheralConnection(peripheral, userClickedCancel: true);
                
                dispatch_async(dispatch_get_main_queue(), {
                    
                    var count = 0
                    
                    for peripheral1:Peripheral in self.discoveredPeripherals.values {
                        
                        if peripheral1.isSender == true {
                        
                            if peripheral1.characteristicWriteCount == self.sourceCharacteristicCount-1 {
                                
                                count++
                                
                            }
                        }
                        if peripheral1.characteristicWriteCount == self.speakerCharacteristicCount-1 {
                            
                            count++
                        }
                        
                    }
                    
                    if count == self.discoveredPeripherals.values.count {
                        
                        self.statusLabel.text = "Apollo Speaker system configured!"
                        self.statusButton.setTitle("", forState: UIControlState.Normal)
                        
                    }
                    
                })
            }
            
            else {
                
                peripheral.characteristicWriteCount++
            }
            }
            else {
                
                if peripheral.characteristicWriteCount == speakerCharacteristicCount-1 {
                    
                    Logger.debug("AppDelegate#didUpdateValueForCharacteristic: Cancel peripheral connection")
                    CentralManager.sharedInstance().cancelPeripheralConnection(peripheral, userClickedCancel: true);
                    
                    dispatch_async(dispatch_get_main_queue(), {
                        
                        var count = 0
                        
                        for peripheral1:Peripheral in self.discoveredPeripherals.values {
                            
                            if peripheral1.isSender == true {
                                
                                if peripheral1.characteristicWriteCount == self.sourceCharacteristicCount-1 {
                                    
                                    count++
                                    
                                }

                            }
                            if peripheral1.characteristicWriteCount == self.speakerCharacteristicCount-1 {
                                
                                count++
                            }
                            
                        }
                        
                        if count == self.discoveredPeripherals.values.count {
                            
                            self.statusLabel.text = "Apollo Speaker system configured!"
                            self.statusButton.setTitle("", forState: UIControlState.Normal)
                            
                        }
                    })
                }
                    
                else {
                    
                    peripheral.characteristicWriteCount++
                }
            }
        }
        
    }
    
    
    func didUpdateValueForCharacteristic(cbPeripheral: CBPeripheral!, characteristic: CBCharacteristic!, error: NSError!) {
        
    }

}
