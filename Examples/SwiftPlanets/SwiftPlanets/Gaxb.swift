//
//  Gaxb.swift
//  Copyright (c) 2014 Small Planet. All rights reserved.
//

import UIKit

protocol GaxbElement {
    var parent: GaxbElement? { get }
    var xmlns: String { get }
    func setElement(element: GaxbElement, key:String)
    func setAttribute(value: String, key:String)
    func attributesXML(useOriginalValues:Bool) -> String
    func sequencesXML(useOriginalValues:Bool) -> String
    func toXML(useOriginalValues:Bool) -> String
    func toXML() -> String
}

protocol GaxbType {
    init(GaxbString withGaxbString: String)
    mutating func setWithGaxbString(GaxbString: String)
    func toGaxbString() -> String
}

extension CGRect: GaxbType {
    init(GaxbString withGaxbString: String) {
        let (origin, size) = CGRect.componentsFromString(withGaxbString)
        self.init(origin: origin, size: size)
    }
    mutating func setWithGaxbString(GaxbString: String) {
        var (newOrigin, newSize) = CGRect.componentsFromString(GaxbString)
        origin = newOrigin
        size = newSize
    }
    func toGaxbString() -> String {
        return "\(origin.x),\(origin.y),\(size.width),\(size.height)"
    }
    static func componentsFromString(string: String) -> (CGPoint, CGSize) {
        var x=0, y=0, w=0, h=0
        var components = string.componentsSeparatedByString(",")
        if components.count == 4 {
            x = components[0].toInt()!
            y = components[1].toInt()!
            w = components[2].toInt()!
            h = components[3].toInt()!
        }
        let origin = CGPoint(x: x, y: y)
        let size = CGSize(width: w, height: h)
        return (origin, size)
    }
}
