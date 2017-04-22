//
//  PeripheralController.swift
//  testing
//
//  Copyright © 2015 broadcom. All rights reserved.
//

import Foundation
import CoreBluetooth

// Service UUID
let WiFiIntroducerConfigNWMConfigServiceUUID    = CBUUID(string: "1B7E8251-2877-41C3-B46E-CF057C562023")
// Characteristic UUIDs
let WiFiIntroducerConfigNWModeUUID              = CBUUID(string: "5B1C19EF-9DD9-4BC4-B848-CFCFDE16B861")
let WiFiIntroducerConfigNWSSIDUUID              = CBUUID(string: "ACA0EF7C-EEAA-48AD-9508-19A6CEF6B356")
let WiFiIntroducerConfigNWSecurityUUID          = CBUUID(string: "CAC2ABA4-EDBB-4C4A-BBAF-0A84A5CD93A1")
let WiFiIntroducerConfigNWPassphraseUUID        = CBUUID(string: "40B7DE33-93E4-4C8B-A876-D833B415A6CE")
let WiFiIntroducerConfigNWChannelUUID           = CBUUID(string: "87F3E72E-94A9-4453-BA85-DCF95490F0EB")
let WiFiIntroducerConfigNWBandUUID              = CBUUID(string: "F000AA22-0451-4000-B000-000000000000")
let WiFiIntroducerConfigSpeakerNameUUID         = CBUUID(string: "FC352C5B-536D-4E4A-B97E-4B3F0705D11D")
let WiFiIntroducerConfigSpeakerChannelUUID      = CBUUID(string: "DA6808BD-F08F-41F7-9325-008948779759")
let WiFiIntroducerConfigSpeakerOutputPortUUID   = CBUUID(string: "F000AA41-0451-4000-B000-000000000000")
let WiFiIntroducerConfigSpeakerOutputVolumeUUID = CBUUID(string: "98749EAB-2DCC-4DA4-B533-5016F7A06B2C")
let WiFiIntroducerConfigSourceInputUUID         = CBUUID(string: "1CD734D3-5592-4FA3-B0AB-826473B2F8B0")
let WiFiIntroducerConfigSourceLoopbackUUID      = CBUUID(string: "F000AA52-0451-4000-B000-000000000000")
let WiFiIntroducerConfigSourceInputPortUUID     = CBUUID(string: "F000AA51-0451-4000-B000-000000000000")
let WiFiIntroducerConfigSourceInputVolumeUUID   = CBUUID(string: "04C77CE6-D0AB-4B86-BC10-D33625E059F6")
let WiFiIntroducerConfigCharNotifyValueUUID     = CBUUID(string: "8AC32D3f-5CB9-4D44-BEC2-EE689169F626")

protocol ReadPeripheralProtocol {
    var serviceUUIDString:String {get}
    var characteristicUUIDString:String {get}
    func didDiscoverCharacteristicsofPeripheral(cbservice : CBService!)
    func didUpdateValueForCharacteristic(cbPeripheral: CBPeripheral!, characteristic:CBCharacteristic!, error:NSError!)
    func didWriteValueForCharacteristic(cbPeripheral: CBPeripheral!, characteristic:CBCharacteristic!, error:NSError!)
}

public class Peripheral : NSObject, CBPeripheralDelegate {
    var readPeripheralDelegate:ReadPeripheralProtocol!
    
    // INTERNAL
    internal var cbPeripheral    : CBPeripheral!
    
    // MARK: Public
    public var advertisements  : Dictionary<NSObject, AnyObject>!
    public var rssi            : Int!
    
    private var _name : String?
    public var name : String {
        get{
            // iOS does not advertise peripheral name in background
            // and even the peripheral is in foreground, the central might still use peripheral's old cached name
            // So only use peripheral's name when explicit name is unavialable
            if(_name == nil) {
                if let name = cbPeripheral.name {
                    return name
                } else {
                    return "Unknown"
                }
            } else {
                return _name!
            }
        }
        set{
            _name = newValue
        }
    }
    
    public var installationId : String?
    
    public var isNearby = false
    
    public var hasBeenConnected = false
    
    public var characteristicWriteCount = 0
    
    public var isSender = false
    
    public var senderName = ""
    public var speakerName = ""
    
    public var state : CBPeripheralState {
        return self.cbPeripheral.state
    }
    
    public var identifier : NSUUID! {
        return self.cbPeripheral.identifier
    }
    
    public init(cbPeripheral:CBPeripheral, advertisements:Dictionary<NSObject, AnyObject>, rssi:Int) {
        super.init()
        self.cbPeripheral = cbPeripheral
        // Fix bug: cbPeripheral.delegate will point to wrong instance because select peripheral screen refresh too fast
        // Move to Peripheral#discoverServices
        // self.cbPeripheral.delegate = self
        self.advertisements = advertisements
        self.rssi = rssi
    }
    
    func discoverServices(serviceUUIDs: [CBUUID]!, delegate: ReadPeripheralProtocol!) {
        self.cbPeripheral.delegate = self
        self.readPeripheralDelegate = delegate
        self.cbPeripheral.discoverServices(serviceUUIDs)
    }
    
    // MARK: CBPeripheralDelegate
    // peripheral
    public func peripheralDidUpdateName(_:CBPeripheral) {
        Logger.debug("Peripheral#peripheralDidUpdateName")
    }
    
    public func peripheral(_:CBPeripheral!, didModifyService invalidatedServices:[AnyObject]!) {
        if let delegate = self.readPeripheralDelegate {
            for service:CBService in invalidatedServices as! [CBService]! {
                if (service.UUID.UUIDString == delegate.serviceUUIDString) {
                    Logger.debug("Peripheral#didModifyServices \(service)")
                    CentralManager.sharedInstance().cancelPeripheralConnection(self, userClickedCancel: false)
                }
            }
        }
    }
    
    // services
    public func peripheral(peripheral:CBPeripheral, didDiscoverServices error:NSError?) {
        Logger.debug("Peripheral#didDiscoverServices: \(self.name) error: \(error)")
        if (error == nil) {
            if let _:ReadPeripheralProtocol = self.readPeripheralDelegate {
                for service:CBService in peripheral.services as [CBService]! {
                    peripheral.discoverCharacteristics(nil, forService: service)
                }
            }
        }
    }
    
    public func peripheral(_:CBPeripheral, didDiscoverIncludedServicesForService service:CBService, error:NSError?) {
        Logger.debug("Peripheral#didDiscoverIncludedServicesForService: \(self.name) error: \(error)")
    }
    
    // characteristics
    public func peripheral(_:CBPeripheral, didDiscoverCharacteristicsForService service:CBService, error:NSError?) {
        Logger.debug("Peripheral#didDiscoverCharacteristicsForService: \(self.name) error: \(error)")
        
        if (error == nil) {
            // Logger.debug("Peripheral#didUpdateValueForCharacteristic")
            if let delegate:ReadPeripheralProtocol = self.readPeripheralDelegate {
                
                delegate.didDiscoverCharacteristicsofPeripheral( service )
            }
        }
    }
    
    public func peripheral(_:CBPeripheral, didUpdateNotificationStateForCharacteristic characteristic:CBCharacteristic, error:NSError?) {
        Logger.debug("Peripheral#didUpdateNotificationStateForCharacteristic error: \(error)")
    }
    
    public func peripheral(peripheral:CBPeripheral, didUpdateValueForCharacteristic characteristic:CBCharacteristic, error:NSError?) {
        // Logger.debug("Peripheral#didUpdateValueForCharacteristic")
        if let delegate:ReadPeripheralProtocol = self.readPeripheralDelegate {
            delegate.didUpdateValueForCharacteristic(peripheral, characteristic: characteristic, error: error)
        }
    }
    
    public func peripheral(peripheral:CBPeripheral, didWriteValueForCharacteristic characteristic:CBCharacteristic, error: NSError?) {
        Logger.debug("Peripheral#didWriteValueForCharacteristic error: \(error)")
        print(error)
        if let delegate:ReadPeripheralProtocol = self.readPeripheralDelegate {
            delegate.didWriteValueForCharacteristic(peripheral, characteristic: characteristic, error: error)
        }
    }
    
    // descriptors
    public func peripheral(_:CBPeripheral, didDiscoverDescriptorsForCharacteristic characteristic:CBCharacteristic, error:NSError?) {
        Logger.debug("Peripheral#didDiscoverDescriptorsForCharacteristic error: \(error)")
    }
    
    public func peripheral(_:CBPeripheral, didUpdateValueForDescriptor descriptor:CBDescriptor, error:NSError?) {
        Logger.debug("Peripheral#didUpdateValueForDescriptor error: \(error)")
    }
    
    public func peripheral(_:CBPeripheral, didWriteValueForDescriptor descriptor:CBDescriptor, error:NSError?) {
        Logger.debug("Peripheral#didWriteValueForDescriptor error: \(error)")
    }
    
}