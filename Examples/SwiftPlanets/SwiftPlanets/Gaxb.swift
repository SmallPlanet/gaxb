//
//  Gaxb.swift
//  Copyright (c) 2014 Small Planet. All rights reserved.
//

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
