//
/*
 * Copyright 2016-2021, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
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

import CoreBluetooth
import UIKit

class BluetoothPeripheral: NSObject, CBPeripheralDelegate, UpgradeStateMachineDelegate {
    static let otaUpgradeServiceUuid = CBUUID(string: "ae5d1e47-5c13-43a0-8635-82ad38a1381f")
    static let otaUpgradeControlPointUuid = CBUUID(string: "a3dd50bf-f7a7-4e99-838e-570a086c661b")
    static let otaUpgradeControlDataUuid = CBUUID(string: "a2e86c7a-d961-4091-b74f-2409e72efe26")

    override var description: String {
        return "name: \(name), UUID: \(identifier?.description ?? "nil")"
    }
    var upgradeViewController: UpgradeViewController?
    var peripheral: CBPeripheral
    var advertisementData: [String : Any]
    var rssi: NSNumber

    var name: String {
        return peripheral.name ?? "Unnamed"
    }
    var identifier: UUID? {
        return peripheral.identifier
    }
    var otaService: CBService?
    var otaControlPoint: CBCharacteristic?
    var otaControlData: CBCharacteristic?
    var isWicedOfuSeviceValid: Bool {
        return otaService != nil && otaControlPoint != nil && otaControlData != nil
    }

    init(_ peripheral: CBPeripheral, advertisementData: [String : Any], rssi: NSNumber) {
        self.peripheral = peripheral
        self.advertisementData = advertisementData
        self.rssi = rssi
        super.init()

        peripheral.delegate = self
    }

    func discoverServices() {
        print("Discovering services ...")
        peripheral.discoverServices([BluetoothPeripheral.otaUpgradeServiceUuid])
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        if error != nil {
            print(error!)
            return
        }

        if (peripheral.services == nil) {
            return
        }
        print("Services: \(peripheral.services!)")
        if (peripheral.services?.count == 0) {
            return
        }

        if peripheral.services![0].uuid != BluetoothPeripheral.otaUpgradeServiceUuid {
            return
        }
        otaService = peripheral.services![0]

        print("Discovering characteristics ...")
        peripheral.discoverCharacteristics(nil, for: otaService!)
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        if error != nil {
            print(error!)
            return
        }

        if service.characteristics == nil {
            return
        }
        print("Characteristics: \(service.characteristics!)")

        for characteristic in service.characteristics! {
            if characteristic.uuid == BluetoothPeripheral.otaUpgradeControlPointUuid {
                otaControlPoint = characteristic
            }
            else if characteristic.uuid == BluetoothPeripheral.otaUpgradeControlDataUuid {
                otaControlData = characteristic
            }
        }

        if otaControlPoint == nil {
            print("Cannot find otaControlPoint")
        }

        if otaControlData == nil {
            print("Cannot find otaControlData")
        }

        if isWicedOfuSeviceValid {
            appendLog("WICED OFU service and characteristic found success")
            if upgradeViewController!.ofuImage?.count != 0 {
                upgradeViewController!.upgradeButton.isEnabled = true
            }
        } else {
            appendLog("WICED OFU service and characteristic not found")
            upgradeViewController!.alertAndPopToRootView(message: "Device \"\(name)\" , go back to scan peripherals.", title: "Info")
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        if error != nil {
            print("peripheral didUpdateValue error: \(error!)")
            return
        }
        if characteristic == otaControlPoint {
            print("didUpdateValueFor otaControlPoint: \(characteristic.value!)")
            writeControlPointCompletion?(nil, .notification)
        } else {
            print("didUpdateValueFor \(characteristic), value: \(characteristic.value!)")
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didUpdateNotificationStateFor characteristic: CBCharacteristic, error: Error?) {
        self.appendLog("didUpdateNotificationStateFor isNotifying = \(otaControlPoint!.isNotifying)")
        controlNotifyCompletion?(error)
    }

    func peripheral(_ peripheral: CBPeripheral, didWriteValueFor characteristic: CBCharacteristic, error: Error?) {
        if characteristic == otaControlPoint {
            writeControlPointCompletion?(error, .writeAck)
        }
        else if characteristic == otaControlData {
            writeControlDataCompletion?(error, .writeAck)
        }
    }


    var controlNotifyCompletion: ((Error?) -> ())?
    var writeControlPointCompletion: ((Error?, UpgradeStateMachine.CompletionDataType) -> ())?
    var writeControlDataCompletion: ((Error?, UpgradeStateMachine.CompletionDataType) -> ())?

    func enableControlNotify() {
        print("otaControlPoint?.isNotifying: \(otaControlPoint!.isNotifying)")
        appendLog("wicedOtaEnableNotification, isNotifying = \(otaControlPoint!.isNotifying)")

        if (otaControlPoint!.isNotifying) {
            controlNotifyCompletion?(nil)
            return
        }

        appendLog("try to enabled otaControlPoint notification")
        peripheral.setNotifyValue(true, for: otaControlPoint!)
    }

    func writeControlPoint(_ data: Data) {
        peripheral.writeValue(data, for: otaControlPoint!, type: .withResponse)
    }

    func writeControlData(_ data: Data) {
        peripheral.writeValue(data, for: otaControlData!, type: .withResponse)
    }

    func readControlPoint() -> Data? {
        return otaControlPoint!.value
    }

    func appendLog(_ message: String) {
        print(message)
        upgradeViewController!.appendLog(message)
    }
}
