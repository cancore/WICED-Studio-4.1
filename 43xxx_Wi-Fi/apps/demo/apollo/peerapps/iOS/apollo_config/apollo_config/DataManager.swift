//
//  DataManager.swift
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

let peripheralSectionCount = 3
let peripheralCountArray : [Int] = [ 0, 0, 3]
let sectionHeaderName : [String] = ["Master Speaker (Connect via BT)", "Remote Speaker", "Discovered Speakers"]

struct peripheralData {
    var name = ""
}

struct peripheralSectionData {
    var name = ""
}

struct cellData {
    // The absolute location of the cell in the tableview (which only has one section)
    var trueIndex = -1
    // Speaker: The location of the cell with respect to its section (maybe unused)
    // peripheralSection: The index of the peripheralSection in an array of all peripheralSections
    var peripheraSectionIndex = -1
    // Section or row
    var isSection = false
    // Speaker count (either 1 for a speaker or # of speakers in a trip)
    var peripheralCount = 0
    // Speaker metadata is populated for speaker cells or nil for peripheralSections
    var peripheralMeta: Peripheral?
    // peripheralSection metadata is populated for peripheralSection cells or nil for speakers
    var peripheralSectionMeta: peripheralSectionData?
    
    mutating func updateTrueIndex(index: Int) {
        trueIndex = index
    }
    
    mutating func updateperipheraSectionIndex(index: Int) {
        peripheraSectionIndex = index
    }
}

// This class provides editing touch recognition for the peripheralSection cell
class peripheralSectionCell: UITableViewCell {
    var indexPath = NSIndexPath()
    
    required init(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)!
    }
    
    override func layoutSubviews() {
        super.layoutSubviews()
        
    }
}

// This class maintains the tableView datasource
class DataManager {
    var cellHistory = [cellData]()
    var cells : Array<cellData> = []
    var cellsCollapsed = [cellData]()
    var movingperipheralSection = false
    
    var peripheralList : Dictionary<CBPeripheral, Peripheral> = [:]
    
    init(test : Dictionary<CBPeripheral, Peripheral>) {
        peripheralList = test
        addRandomData(peripheralSectionCount, peripheralsPer: peripheralCountArray)
    }
    
    func valueofCells() -> Array<cellData> {
        return self.cells
    }
    
    func addRandomData(peripheralSections: Int, peripheralsPer: [Int]) {
        
        var peripheralOffset = 0
        let count = peripheralList.count
        
        for peripheralSection in 0...2 {
            let peripheralSectionMeta = peripheralSectionData(name: "\(sectionHeaderName[peripheralSection])")
            let peripheralCountVal = peripheralsPer[peripheralSection]
            let cellMeta = cellData(trueIndex: peripheralSection + peripheralOffset, peripheraSectionIndex: peripheralSection, isSection: true, peripheralCount: peripheralCountVal, peripheralMeta: nil, peripheralSectionMeta: peripheralSectionMeta)
            
            cells.append(cellMeta)
            print("\(peripheralSectionMeta.name) @ row \(peripheralSection + peripheralOffset)")

            for peripheral in 0...count-1 {
                // rather than this static generation we have to resort to dynamic generation
                let key = Array(peripheralList.keys)[peripheral]
                
                print(peripheralList[key]!.name)
                
                let cellMeta = cellData(trueIndex: peripheralSection + peripheralOffset + 1, peripheraSectionIndex: peripheralSection, isSection: false, peripheralCount: 1, peripheralMeta: peripheralList[key]!, peripheralSectionMeta: nil)
                
                if peripheralSection != 0 && peripheralSection != 1 {
                    cells.append(cellMeta)
                    peripheralOffset++
                }
            }
            
        }
    }
    
    func getRows() -> Int {
        if movingperipheralSection {
            return cellsCollapsed.count
        }
        else {
            return cells.count
        }
    }
    
    func cellID(indexPath: NSIndexPath) -> String {
        var curCells = [cellData]()
        
        if movingperipheralSection {
            curCells = cellsCollapsed
        }
        else {
            curCells = cells
        }
        
        if curCells[indexPath.row].isSection {
            return "cellPeripheralSection"
        }
        
        return "cellPeripheral"
    }
    
    func cellText(indexPath: NSIndexPath) -> String {
        var curCells = [cellData]()
        
        if movingperipheralSection {
            curCells = cellsCollapsed
        }
        else {
            curCells = cells
        }
        
        if let peripheralSectionName = curCells[indexPath.row].peripheralSectionMeta?.name {
            return peripheralSectionName
        }
        else if let peripheral = curCells[indexPath.row].peripheralMeta {
            return peripheral.name
        }
        
        return "--ERROR--"
    }
    
    func completeMove(origin: NSIndexPath, destination: NSIndexPath) {
        // Determine direction of the move and grab the cell data
        let movedUp = origin.row > destination.row ? true : false
        let cellData = cells[origin.row]

        if !movingperipheralSection {
            let startRow = origin.row
            let endRow = destination.row
            
            if movedUp && startRow != endRow {
                var curSection = cellData.peripheraSectionIndex
                var passedFirst = false
                
                for (var i = startRow; i >= endRow; i--) {
                    // passed cells index getting incremented as you move up
                    cells[i].trueIndex++
                    
                    
                    // the cell you are moving up index keeps decrementing while you move up
                    cells[startRow].trueIndex--
                    
                    if cells[i].isSection && !passedFirst {
                        passedFirst = true
                        curSection--
                        cells[i].peripheralCount--
                    }
                }
                
                for (var i = endRow; i >= 0; i--) {
                    if cells[i].isSection {
                        cells[i].peripheralCount++
                        break
                    }
                }
                
                cells[startRow].peripheraSectionIndex = curSection
            }
            else {
                var curSection = cellData.peripheraSectionIndex
                _ = false
                
                for (var i = startRow; i <= endRow; i++) {
                    cells[i].trueIndex--
                    cells[startRow].trueIndex++
                    
                    if cells[i].isSection {
                        curSection++
                    }
                }
                
                for (var i = 0; i < cells.count; i++) {
                    if cells[i].peripheraSectionIndex == curSection && cells[i].isSection {
                        cells[i].peripheralCount++
                    }
                    
                    if cells[i].peripheraSectionIndex == cellData.peripheraSectionIndex && cells[i].isSection {
                        cells[i].peripheralCount++
                    }
                }
                
                cells[startRow].peripheraSectionIndex = curSection
            }
            
            rectifyTrueIndices()
        }
    }
    
    func rectifyTrueIndices() {
        var newCells = [cellData]()
        var curCell = 0
        var curTrueIndex = 0
        
        while (newCells.count != cells.count) {
            if cells[curCell].trueIndex == curTrueIndex {
                newCells.append(cells[curCell])
                curTrueIndex++
            }
            
            curCell++
            
            if curCell == cells.count {
                curCell = 0
            }
        }
        
        cells = newCells
    }
}