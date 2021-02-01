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

class BluetoothPeripheralManager: NSObject, CBCentralManagerDelegate {
    static var peripherals = [BluetoothPeripheral]()

    var tableView: UITableView
    var centralManager: CBCentralManager?

    var delegate: BluetoothPeripheralManagerDelegate?

    init(tableView: UITableView) {
        self.tableView = tableView
        super.init()

        centralManager = CBCentralManager(delegate: self, queue: nil)
    }

    func startScan() {
        BluetoothPeripheralManager.peripherals.removeAll()
        tableView.reloadData()
        if #available(iOS 10, *) {
            tableView.refreshControl!.endRefreshing()
        }

        print("Start scanning peripherals ...")
        centralManager!.scanForPeripherals(withServices: nil, options: [CBCentralManagerScanOptionAllowDuplicatesKey : true])
    }

    func stopScan() {
        print("Stop scanning peripherals ...")
        centralManager!.stopScan()
    }

    func connect(_ bluetoothPeriperal: BluetoothPeripheral) {
        print("Connecting \(bluetoothPeriperal.peripheral) ...")
        centralManager!.connect(bluetoothPeriperal.peripheral, options: nil)
    }

    func disconnect(_ bluetoothPeriperal: BluetoothPeripheral) {
        print("Disconnecting \(bluetoothPeriperal.peripheral) ...")
        centralManager!.cancelPeripheralConnection(bluetoothPeriperal.peripheral)
    }

    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == .poweredOn {
            print("BluetoothPeripheralManager poweredOn")
            startScan()
        }
        else {
            print("BluetoothPeripheralManager DidUpdateState: \(central.state.rawValue)")
        }
    }

    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        for knownPeripheral in BluetoothPeripheralManager.peripherals {
            if knownPeripheral.identifier == peripheral.identifier {
                return
            }
        }

        if advertisementData[CBAdvertisementDataIsConnectable] as! NSNumber != 1 {
            return
        }

        print("didDiscover \(peripheral)")

        BluetoothPeripheralManager.peripherals.append(BluetoothPeripheral(peripheral, advertisementData: advertisementData, rssi: RSSI))

        tableView.reloadData()
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("centralManager didConnect: \(peripheral)")

        for bluetoothPeripheral in BluetoothPeripheralManager.peripherals {
            if bluetoothPeripheral.identifier == peripheral.identifier {
                delegate?.bluetoothPeripheralManager(self, didConnect: bluetoothPeripheral)
                break
            }
        }
    }

    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        print("centralManager didFailToConnect: \(peripheral)")
        if error != nil {
            print("Error: \(error!)")
        }
    }

    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        print("centralManager didDisconnectPeripheral: \(peripheral)")
        if error != nil {
            print("Error: \(error!)")
        }

        for bluetoothPeripheral in BluetoothPeripheralManager.peripherals {
            if bluetoothPeripheral.identifier == peripheral.identifier {
                delegate?.bluetoothPeripheralManager(self, didDisconnectPeripheral: bluetoothPeripheral, error: error)
                break
            }
        }
    }
}
