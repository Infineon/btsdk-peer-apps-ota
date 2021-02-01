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

import UIKit

class UpgradeViewController: UIViewController, BluetoothPeripheralManagerDelegate, UIDocumentPickerDelegate {
    @IBOutlet weak var peripheralName: UILabel!
    @IBOutlet weak var openButton: UIButton!
    @IBOutlet weak var upgradeButton: UIButton!
    @IBOutlet weak var ofuFileUrlLabel: UILabel!
    @IBOutlet weak var progressView: UIProgressView!
    @IBOutlet weak var progressLabel: UILabel!
    @IBOutlet weak var logTextView: UITextView!

    static var ofuFileUrl: URL?

    var bluetoothPeripheralManager: BluetoothPeripheralManager?
    var activePeripheral: BluetoothPeripheral?
    var upgradeStateMachine: UpgradeStateMachine?
    var ofuImage: Data?

    @IBAction func pickOfuFile(_ sender: Any) {
        let picker = UIDocumentPickerViewController(documentTypes: ["public.data"], in: .open)
        picker.delegate = self
        present(picker, animated: true, completion: nil)
    }

    @IBAction func upgrade(_ sender: Any) {
        if ofuImage!.count <= 0 {
            appendLog("Invalid OFU image, size: \(ofuImage!.count)")
            return
        }

        if !activePeripheral!.isWicedOfuSeviceValid {
            self.appendLog("Invalid WICED OFU services")
            return
        }

        openButton.isEnabled = false
        upgradeButton.isEnabled = false
        setProgress(0)

        upgradeStateMachine!.reset()
        upgradeStateMachine!.process()
    }

    func setProgress(_ progress: Float) {
        progressView!.progress = progress
        progressLabel!.text = String(format: "%.1f%%", progress * 100)
    }

    func documentPicker(_ controller: UIDocumentPickerViewController, didPickDocumentsAt urls: [URL]) {
        let url: URL = urls[0]

        loadOfuImage(url)

        if ofuImage != nil {
            UpgradeViewController.ofuFileUrl = url
            appendLog("The OFU image was loaded from \(url)")
            appendLog("The image size: \(ofuImage!.count)")
            upgradeButton.isEnabled = true;
        }
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        peripheralName.text = activePeripheral!.name
        setProgress(0)
        logTextView.text = ""

        upgradeStateMachine = UpgradeStateMachine(upgradeViewController: self, delegate: activePeripheral!)

        if UpgradeViewController.ofuFileUrl != nil {
            loadOfuImage(UpgradeViewController.ofuFileUrl!)
        }

        activePeripheral!.upgradeViewController = self

        bluetoothPeripheralManager!.delegate = self
        bluetoothPeripheralManager!.connect(activePeripheral!)
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "BackToScan" {
            bluetoothPeripheralManager!.disconnect(activePeripheral!)
        }
    }

    fileprivate func loadOfuImage(_ url: URL) {
        ofuFileUrlLabel.text = url.description

        if !url.startAccessingSecurityScopedResource() {
            print("startAccessingSecurityScopedResource failed")
            return
        }
        do {
            try ofuImage = Data(contentsOf: url, options: .alwaysMapped)
        } catch {
            print("Cannot load \(url), error: \(error)")
        }
        url.stopAccessingSecurityScopedResource()
    }

    func bluetoothPeripheralManager(_ manager: BluetoothPeripheralManager, didConnect peripheral: BluetoothPeripheral) {
        if peripheral == activePeripheral {
            peripheral.discoverServices()
        }
    }

    func bluetoothPeripheralManager(_ manager: BluetoothPeripheralManager, didDisconnectPeripheral peripheral: BluetoothPeripheral, error: Error?) {
        alertAndPopToRootView(message: "\(peripheral.name) was disconnected.", title: "Error")
    }

    func appendLog(_ message: String) {
        let dateFormatter = DateFormatter()
        dateFormatter.setLocalizedDateFormatFromTemplate("HH:mm:ss")
        let logMsg = dateFormatter.string(from: Date()) + " " + message + "\n"
        logTextView.text = logTextView.text + logMsg

        // Always scroll the text view to bottom to show latest log message.
        let range = NSMakeRange(logTextView.text.count - 1, 1)
        logTextView.scrollRangeToVisible(range)
    }

    static var deafultAlertAction = UIAlertAction(title: "Ok", style: .default, handler: { (UIAlertAction) in return })

    private func alert(message: String, action: UIAlertAction = deafultAlertAction, title: String = "Alert") {
        let alertController = UIAlertController(title: title, message: message, preferredStyle: .alert)

        alertController.addAction(action)
        present(alertController, animated: true, completion: nil)
    }

    func alertBluetoothOffAndExit() {
        let msg = "Bluetooth not enabled.\n\nPlease enable Blutooth then try again.\n\nNow exit!"
        let alertAction = UIAlertAction(title: "OK",
                                        style: .default,
                                        handler: { (action: UIAlertAction) in exit(0) })
        alert(message: msg, action: alertAction, title: "Alert")
    }

    func alertAndPopToRootView(message: String, title: String = "Alert") {
        let alertAction = UIAlertAction(title: "OK",
                                        style: .default,
                                        handler: { (action: UIAlertAction) in self.performSegue(withIdentifier: "BackToScan", sender: self) })
        alert(message: message, action: alertAction, title: title)
    }
}
