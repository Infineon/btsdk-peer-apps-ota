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

class UpgradeStateMachine: NSObject {

    enum OtaUpgradeState {
        case idle
        case prepareForDownload
        case startDownload
        case dataTransfer
        case verify
        case abort
        case complete

        func nextState() -> OtaUpgradeState {
            switch self {
            case .idle:
                return .prepareForDownload
            case .prepareForDownload:
                return .startDownload
            case .startDownload:
                return .dataTransfer
            case .dataTransfer:
                return .verify
            case .verify:
                return .complete
            case .abort:
                return .complete
            case .complete:
                return .complete
            }
        }

        func description() -> String {
            switch self {
            case .idle:
                return "idle"
            case .prepareForDownload:
                return "prepareForDownload"
            case .startDownload:
                return "startDownload"
            case .dataTransfer:
                return "dataTransfer"
            case .verify:
                return "verify"
            case .abort:
                return "abort"
            case .complete:
                return "complete"
            }
        }
    }

    enum WicedOtaUpgradeCommand: Int {
        case prepareDownload = 1
        case startDownload = 2
        case verify = 3
        case finish = 4
        case getStatus = 5      // not currently used
        case clearStatus = 6    // not currently used
        case abort = 7
    }

    enum WicedOtaUpgradeStatus: UInt8 {
        case ok = 0
        case unsupported = 1
        case illegal = 2
        case verificationFailed = 3
        case invalidImage = 4
        case invalidImageSize = 5
        case moreData = 6
        case invalidAppId = 7
        case invalidVersion = 8
        case continueStatus = 9

        func description() -> String {
            switch self {
            case .ok:
                return "success"
            case .unsupported:
                return "unsupported command"
            case .illegal:
                return "illegal state"
            case .verificationFailed:
                return "image varification failed"
            case .invalidAppId:
                return "invalid App Id"
            case .invalidImage:
                return "invalid image"
            case .invalidImageSize:
                return "invalid image size"
            case .invalidVersion:
                return "invalid version"
            case .moreData:
                return "more data"
            case .continueStatus:
                return "continue"
            }
        }
    }

    enum CompletionDataType: Int {
        case unknonwn = 0
        case normal = 1
        case success = 2
        case error = 3
        case writeAck = 5       // Response for the send the Write/Read operation command data done.
        case notification = 6   // Response for the command after process.
    }

    struct WicedOtaCharaceteristicValue {
        // dataSize: 4 bytes; command: 1 byte; parameters: max 4 bytes
        private var bytes: [UInt8]
        var value: Data {
            return Data(bytes: bytes)
        }
        var dataSize: Int {
            return (Int(bytes[0] & 0xFF) |
                Int((bytes[1] << 8) & 0xFF) |
                Int((bytes[2] << 16) & 0xFF) |
                Int((bytes[3] << 24) & 0xFF))
        }

        init(command: WicedOtaUpgradeCommand) {
            let dataSize = 1
            bytes = [UInt8](repeating: 0, count: dataSize)
            bytes[0] = UInt8(command.rawValue)
        }

        init(command: WicedOtaUpgradeCommand, sParam: UInt16) {
            let dataSize = 3
            bytes = [UInt8](repeating: 0, count: dataSize)
            bytes[0] = UInt8(command.rawValue)
            bytes[1] = UInt8(sParam & 0xFF)
            bytes[2] = UInt8((sParam >> 8) & 0xFF)
        }

        init(command: WicedOtaUpgradeCommand, lParam: UInt32) {
            let dataSize = 5
            bytes = [UInt8](repeating: 0, count: dataSize)
            bytes[0] = UInt8(command.rawValue)
            bytes[1] = UInt8(lParam & 0xFF)
            bytes[2] = UInt8((lParam >> 8) & 0xFF)
            bytes[3] = UInt8((lParam >> 16) & 0xFF)
            bytes[4] = UInt8((lParam >> 24) & 0xFF)
        }
    }

    private static let TRANSFER_PACKET_SIZE: Int = 244

    private var upgradeViewController: UpgradeViewController?
    private var delegate: UpgradeStateMachineDelegate?
    private var state: OtaUpgradeState = .idle
    private var isUpgradeDoneSuccess: Bool = false
    private var offset: Int = 0
    private var transferPacketSize: Int = TRANSFER_PACKET_SIZE
    private var crc32 = Crc32()

    init(upgradeViewController: UpgradeViewController, delegate: UpgradeStateMachineDelegate) {
        super.init()

        self.upgradeViewController = upgradeViewController
        self.delegate = delegate
        self.delegate!.controlNotifyCompletion = enableControlComplete
        self.delegate!.writeControlPointCompletion = wicedOtaControlPointCommandCompletionHandler
        self.delegate!.writeControlDataCompletion = wicedOtaControlDataWriteCompletionHandler
    }

    func reset() {
        state = .idle
        isUpgradeDoneSuccess = false
    }

    func process() {
        switch state {
        case .idle:
            wicedOtaEnableNotification()
        case .prepareForDownload:
            wicedOtaPrepareForDownload()
        case .startDownload:
            wicedOtaStartDownload()
        case .dataTransfer:
            wicedOtatransferData()
        case .verify:
            appendLog("downloading done")
            wicedOtaVerify()
        case .abort:
            appendLog("error, abort upgrading")
            wicedOtaAborted()
        case .complete:
            wicedOtaCompleted()
        }
    }

    private func wicedOtaEnableNotification() {
        delegate?.enableControlNotify()
    }

    private func enableControlComplete(error: Error?) {
        if (error != nil) {
            self.appendLog("failed to enable indication for ControlPiont characteristic, \(error.debugDescription)")

            self.state = .complete
        } else {
            self.state = .prepareForDownload
        }

        process()
    }

    private func wicedOtaPrepareForDownload() {
        let cmdData = WicedOtaCharaceteristicValue(command: .prepareDownload)

        appendLog("wicedOtaPrepareForDownload")
        delegate!.writeControlPoint(cmdData.value)
    }

    private func wicedOtaStartDownload() {
        let cmdData = WicedOtaCharaceteristicValue(command: .startDownload, lParam: UInt32(upgradeViewController!.ofuImage!.count))

        appendLog("wicedOtaStartDownload")
        delegate!.writeControlPoint(cmdData.value)
    }

    private func wicedOtatransferData() {
        let imageSize = upgradeViewController!.ofuImage!.count

        if (imageSize > offset) {
            transferPacketSize = imageSize - offset
            transferPacketSize = (transferPacketSize > UpgradeStateMachine.TRANSFER_PACKET_SIZE) ?
                UpgradeStateMachine.TRANSFER_PACKET_SIZE : transferPacketSize

            let range: Range = offset..<(offset + transferPacketSize)
            let data = upgradeViewController!.ofuImage!.subdata(in: range)

            crc32.update(data)
            if ((offset + transferPacketSize) == imageSize) {
                // this is the last pacet Finalize CRC
                crc32.final()
                print("last block write offset=\(offset), \(data.count) bytes, updated crc32=\(crc32)")
            }

            delegate!.writeControlData(data)
        }
    }

    private func wicedOtaVerify() {
        appendLog("wicedOtaVerify, verify crc32=\(crc32)")
        let cmdData = WicedOtaCharaceteristicValue(command: .verify, lParam: UInt32(crc32.value))

        delegate!.writeControlPoint(cmdData.value)
    }

    private func wicedOtaAborted() {
        let imageSize = upgradeViewController!.ofuImage!.count
        let cmdData = WicedOtaCharaceteristicValue(command: .abort, lParam: UInt32(imageSize))

        appendLog("wicedOtaAborted")
        delegate!.writeControlPoint(cmdData.value)
    }

    private func wicedOtaCompleted() {
        appendLog("wicedOtaCompleted")

        if (isUpgradeDoneSuccess) {
            upgradeViewController!.setProgress(1)

            appendLog("firmware upgrade done success")

            // Firmware upgrade doen susscess, the Peripheral will reset itself automatically.
            // so, navigate to the device list page to rescan the Peripheral devices.
            upgradeViewController!.alertAndPopToRootView(message: "Firmware OTA upgrade done.\nCongratuation!", title: "Success")
        } else {
            appendLog("firmware upgrade done with error")
            upgradeViewController!.alertAndPopToRootView(message: "Firmware OTA upgrade failed!", title: "Error")
        }
    }

    private func wicedOtaControlPointCommandCompletionHandler(_ error: Error?, completionDataType respType: CompletionDataType) {
        if (error != nil) {
            self.appendLog("failed to write \(state.description()) command, \(error.debugDescription)")

            switch (state) {
            case .abort:
                state = .complete
                break
            case .complete:
                break
            default:
                state = .abort
                break
            }
        } else {
            if (respType == .writeAck) {
                self.appendLog("write \(state.description()) command to remote device success")
                return
            }

            var status: WicedOtaUpgradeStatus?
            var data = delegate!.readControlPoint()
            if (data == nil || data?.count == 0) {
                status = .unsupported
            } else {
                status = WicedOtaUpgradeStatus(rawValue: UInt8((data?[0])!))
            }
            self.appendLog("\"\(self.state.description())\" command done with status: " + (status?.description())!)

            if (status != .ok) {
                if self.state == .abort {
                    self.state = .complete
                } else {
                    self.state = .abort
                }
            } else if (self.state == .dataTransfer) {
                self.appendLog("should not receive control point update in data transfer state, ignore")
                return
            } else {
                if (self.state == .verify) {
                    self.isUpgradeDoneSuccess = true
                }

                self.state = state.nextState()
            }

            if (self.state == .dataTransfer) {
                // Reset the data tranfer states values.
                self.crc32.reset()
                self.offset = 0

                self.appendLog("downloading ...")
            }
        }

        self.process()
    }

    private func wicedOtaControlDataWriteCompletionHandler(_ error: Error?, completionDataType respType: CompletionDataType) {
        let imageSize = upgradeViewController!.ofuImage!.count

        if (respType == .notification) {
            print("Control Data characteristic notificaiton data, should not happen")
            return
        }

        if (error != nil) {
            print(error.debugDescription)
            self.appendLog("failed to write image data at offset=\(self.offset)")

            self.state = .abort
        } else {
            self.offset += transferPacketSize

            if (self.offset >= imageSize) {
                self.state = .verify
            } else {
                self.state = .dataTransfer
            }
        }

        upgradeViewController?.setProgress(Float(self.offset) / Float(imageSize))
        self.process()
    }

    private func appendLog(_ message: String) {
        upgradeViewController!.appendLog(message)
    }
}
