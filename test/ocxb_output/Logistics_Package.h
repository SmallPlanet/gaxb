//
// Autogenerate by ocxb on 2012-01-06 10:17:48 -0500
//

#ifndef LOGISTICS_PACKAGE
#define LOGISTICS_PACKAGE

#import <UIKit/UIKit.h>



@class NSString;


#ifndef SAFESTRING
#define SAFESTRING(x) ([self translateToXMLSafeString:x])
#endif
@interface Logistics_Package : NSObject
{
id parent;
NSNumber * uid;
	NSString * title;
BOOL titleExists;
	int size;
BOOL sizeExists;
BOOL ocxb_init_called;BOOL ocxb_dealloc_called;}

@property(nonatomic,retain) NSString * title;
@property(nonatomic, readonly) BOOL titleExists;
@property (nonatomic) int size;
@property (nonatomic,readonly) BOOL sizeExists;
@property (nonatomic, assign) id parent;
@property (nonatomic, retain) NSNumber * uid;


- (NSArray *) validAttributes;
- (void) setTitleWithString:(NSString *)string;
- (void) setSizeWithString:(NSString *)string;
- (void) appendXML:(NSMutableString *)xml;
- (void) appendXMLAttributesForSubclass:(NSMutableString *)xml;
- (void) appendXMLElementsForSubclass:(NSMutableString *)xml;

- (id) ocxb_init;
- (void) ocxb_dealloc;
- (void) ocxb_valueWillChange:(NSString *)_name;
- (void) ocxb_valueDidChange:(NSString *)_name;
- (void) ocxb_loadDidComplete;


@end

#endif