//
//  Galaxy.swift
//  SwiftPlanets
//
//  Created by Quinn McHenry on 6/14/14.
//  Copyright (c) 2014 Small Planet. All rights reserved.
//

protocol GaxbElement {
    func setElement(element: GaxbElement, key:String)
    func setAttribute(value: String, key:String)
}

class Galaxy {
   
    class func classWithName(name : String) -> GaxbElement? {
        switch name {
        case "Moon":
            return Moon()
        case "Planet":
            return Planet()
        case "AstronomicalObject":
            return AstronomicalObject()
        case "Star":
            return Star()
        case "StarSystem":
            return StarSystem()
        default:
            return nil
        }
    }

    class func readFromFile(filepath : String) -> GaxbElement? {
        let rootXML: AnyObject! = RXMLElement.elementFromXMLFile(filepath)
        if rootXML as? RXMLElement {
            return Galaxy.parseElement(rootXML as RXMLElement)
        }
        return nil
    }

    class func parseElement(element: RXMLElement) -> GaxbElement? {
        println("element = " + element.tag)
        if let entity : GaxbElement = Galaxy.classWithName(element.tag) {
            println("aaa")
            let names = element.attributeNames() as String[]
            for name in names {
                let value = element.attribute(name) as String
                entity.setAttribute(value, key: name)
            }
            
            let block: (element: RXMLElement!) -> Void = { element in
                if let subEntity : GaxbElement? = Galaxy.parseElement(element) {
                    entity.setElement(subEntity!, key: element.tag!)
                    println("element.tag = " + element.tag )
                }
            }
            element.iterate("*", usingBlock:block)
            return entity
        }
        return nil
    }
    
}
