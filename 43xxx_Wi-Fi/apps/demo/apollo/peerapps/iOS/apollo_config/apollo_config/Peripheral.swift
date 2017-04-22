//
//  PeripheralController.swift
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

import Foundation
import CoreBluetooth

// Service UUIDs
let ApolloConfigServiceUUID             = CBUUID(string: "04574543-9CBA-4B22-BC78-55040A01C246")
// Characteristic UUIDs
let ApolloConfigNWModeUUID              = CBUUID(string: "5B1C19EF-9DD9-4BC4-B848-CFCFDE16B861")
let ApolloConfigNWSSIDUUID              = CBUUID(string: "ACA0EF7C-EEAA-48AD-9508-19A6CEF6B356")
let ApolloConfigNWSecurityUUID          = CBUUID(string: "CAC2ABA4-EDBB-4C4A-BBAF-0A84A5CD93A1")
let ApolloConfigNWPassphraseUUID        = CBUUID(string: "40B7DE33-93E4-4C8B-A876-D833B415A6CE")
let ApolloConfigNWChannelUUID           = CBUUID(string: "87F3E72E-94A9-4453-BA85-DCF95490F0EB")
let ApolloConfigNWBandUUID              = CBUUID(string: "F000AA22-0451-4000-B000-000000000000")
let ApolloConfigSpeakerNameUUID         = CBUUID(string: "FC352C5B-536D-4E4A-B97E-4B3F0705D11D")
let ApolloConfigSpeakerChannelUUID      = CBUUID(string: "DA6808BD-F08F-41F7-9325-008948779759")
let ApolloConfigSpeakerOutputPortUUID   = CBUUID(string: "F000AA41-0451-4000-B000-000000000000")
let ApolloConfigSpeakerOutputVolumeUUID = CBUUID(string: "98749EAB-2DCC-4DA4-B533-5016F7A06B2C")
let ApolloConfigSourceInputUUID         = CBUUID(string: "1CD734D3-5592-4FA3-B0AB-826473B2F8B0")
let ApolloConfigSourceLoopbackUUID      = CBUUID(string: "F000AA52-0451-4000-B000-000000000000")
let ApolloConfigSourceInputPortUUID     = CBUUID(string: "F000AA51-0451-4000-B000-000000000000")
let ApolloConfigSourceInputVolumeUUID   = CBUUID(string: "04C77CE6-D0AB-4B86-BC10-D33625E059F6")

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
    
    var cells = [cellData]()
    
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