//
//  AstronomicalObject.swift
//  BigPlanets
//
//  Created by Quinn McHenry on 6/4/14.
//  Copyright (c) 2014 Small Planet Digital. All rights reserved.
//

class GalaxyBaseObject {
    var xmlns: String {
        return "http://schema.smallplanet.com/Galaxy"
    }
    var parent: GalaxyBaseObject?
}

class AstronomicalObject: GalaxyBaseObject {
//    var uid ?
//    var gaxb_init_called = false
//    var gaxb_dealloc_called = false 
    var originalValues = Dictionary<String, String> ()
    
    // attributes
    var name: String? {
        willSet {
            self.gaxbValueWillChange("name")
        }
        didSet {
            self.gaxbValueDidChange("name")
            nameExists = true
        }
    }
    var nameExists = false
    
    var mass: Float?
    var massExists: Bool {
        return mass != nil
    }
    var meanOrbitalDistance: Float?
    var meanOrbitalDistanceExists: Bool {
        return meanOrbitalDistance != nil
    }
    var orbitalPeriod: Float?
    var orbitalPeriodExists: Bool {
        return orbitalPeriod != nil
    }
    var equatorialRadius: Float?
    var equatorialRadiusExists: Bool {
        return equatorialRadius != nil
    }
    var equatorialGravity: Float?
    var equatorialGravityExists: Bool {
        return equatorialGravity != nil
    }

    
    func gaxbValueWillChange(name:String) {
        
    }
    func gaxbValueDidChange(name:String) {
        
    }


    func appendXMLAttributesForSubclass(var xml:String, useOriginalValues:Bool? = false) {
        if (useOriginalValues) {
            for (key, value) in originalValues {
                xml += " \(key)='\(value)'"
            }
        } else {
            if (name) {
                xml += " name='\(name)'"
            }
            if (mass) {
                xml += " mass='\(String(mass!))'"
            }
            if (meanOrbitalDistance) {
                xml += " meanOrbitalDistance='\(String(meanOrbitalDistance!))'"
            }
            if (orbitalPeriod) {
                xml += " orbitalPeriod='\(String(orbitalPeriod!))'"
            }
            if (equatorialRadius) {
                xml += " equatorialRadius='\(String(equatorialRadius!))'"
            }
            if (equatorialGravity) {
                xml += " equatorialGravity='\(String(equatorialGravity!))'"
            }
        }
    }

    func appendXMLElementsForSubclass(xml: String, useOriginalValues:Bool? = false) {
        
    }
    
    func appendXML(var xml:String, useOriginalValues:Bool? = false) {
        xml += "<AstronomicalObject"
        if (parent!.xmlns != xmlns) {
            xml += " xmlns='\(xmlns)'"
        }
    }
    
//    var description: String {
//        return
//    }
//    - (NSString *) translateToXMLSafeString:(NSString *)__value
//    - (NSString *) description

//    - (NSDate *) dateFromString:(NSString *)date_string WithFormat:(NSString *)date_format

//    - (NSDate *) schemaDateTimeFromString:(NSString *)date_string
//    - (NSDate *) schemaDateFromString:(NSString *)date_string
//    - (NSString *) dateTimeStringFromSchema:(NSDate *)_date
//    - (NSString *) dateStringFromSchema:(NSDate *)_date

    
}
