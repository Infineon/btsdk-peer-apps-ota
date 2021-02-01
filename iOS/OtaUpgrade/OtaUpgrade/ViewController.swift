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

class ViewController: UIViewController, UITableViewDataSource {
    @IBOutlet weak var bluetoothPeripheralTableView: UITableView!

    let refreshControl = UIRefreshControl()

    var bluetoothPeripheralManager: BluetoothPeripheralManager?

    override func viewDidLoad() {
        super.viewDidLoad()

        if bluetoothPeripheralManager == nil {
            bluetoothPeripheralManager = BluetoothPeripheralManager(tableView: bluetoothPeripheralTableView)
        }

        refreshControl.attributedTitle = NSAttributedString(string: "Refreshing ...")
        refreshControl.addTarget(self, action: #selector(refreshPeripherals), for: .valueChanged)
        if #available(iOS 10, *) {
            bluetoothPeripheralTableView.refreshControl = refreshControl
        }
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    @IBAction func refreshPeripherals()
    {
        bluetoothPeripheralManager?.startScan()
    }

    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return BluetoothPeripheralManager.peripherals.count
    }

    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "BluetoothPeripheralTableViewCell", for: indexPath) as! BluetoothPeripheralTableViewCell
        let peripheral = BluetoothPeripheralManager.peripherals[indexPath.row]
        cell.name.text = peripheral.name
        cell.rssi.text = peripheral.rssi.description
        return cell
    }

    func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        return "RSSI\tName\t\t(Pull down to refresh)"
    }

    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "NavigateToUpgrade" {
            bluetoothPeripheralManager!.stopScan()

            let upgradeViewController = segue.destination as! UpgradeViewController
            upgradeViewController.bluetoothPeripheralManager = bluetoothPeripheralManager
            upgradeViewController.activePeripheral = BluetoothPeripheralManager.peripherals[bluetoothPeripheralTableView.indexPathForSelectedRow!.row]
        }
    }
}
