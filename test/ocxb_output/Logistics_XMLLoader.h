//
// Autogenerate by ocxb on 2012-01-06 10:17:48 -0500
//

#ifndef LOGISTICS_XMLLOADER
#define LOGISTICS_XMLLOADER

#import <UIKit/UIKit.h>
#import "Logistics_Container.h"
#import "Logistics_Package.h"

enum {
	PACKAGE_SMALL,
	PACKAGE_MEDIUM,
	PACKAGE_LARGE,
};


@interface Logistics_XMLLoader : NSObject 
{NSMutableArray * element_stack;NSString * last_element_name;NSMutableString * scratch_string;NSMutableDictionary * attribute_map;NSMutableDictionary * plural_element_map;id parent;int uid;BOOL memLite;}@property (nonatomic, retain) NSMutableArray * element_stack;+ (id) readFromFile:(NSString *)path;
+ (id) readFromData:(NSData *)data;
+ (id) readFromData:(NSData *)data withParent:(id)parent;
+ (id) readFromString:(NSString *)xml_string;
+ (id) readFromFileFast:(NSString *)path;
+ (id) readFromDataFast:(NSData *)data;
+ (id) readFromStringFast:(NSString *)xml_string;
+ (void) useTBXML:(BOOL)b;
+ (void) write:(id)object toFile:(NSString *)path;
+ (NSData *) writeToData:(id)object;
+ (NSString *) writeToString:(id)object;
- (id) initWithParent:(id)p AndMemoryLite:(BOOL)m;
@end


#endif