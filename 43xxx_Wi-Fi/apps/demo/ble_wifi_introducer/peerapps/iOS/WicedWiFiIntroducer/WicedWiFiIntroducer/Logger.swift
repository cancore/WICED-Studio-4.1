//
//  Logger.swift
//  testing
//
//  Copyright © 2015 broadcom. All rights reserved.
//

import Foundation

public class Logger {
    
    public class func debug(message:AnyObject) {
        //#if DEBUG
            print("\(message)")
        //#endif
    }
    
}