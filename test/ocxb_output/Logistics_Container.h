//
// Autogenerate by ocxb on 2012-01-06 10:17:48 -0500
//

#ifndef LOGISTICS_CONTAINER
#define LOGISTICS_CONTAINER

#import <UIKit/UIKit.h>





#ifndef SAFESTRING
#define SAFESTRING(x) ([self translateToXMLSafeString:x])
#endif
@interface Logistics_Container : NSObject
{
id parent;
NSNumber * uid;
	NSMutableArray * Packages;
BOOL ocxb_init_called;BOOL ocxb_dealloc_called;}

@property(nonatomic,retain) NSMutableArray * Packages;
@property (nonatomic, assign) id parent;
@property (nonatomic, retain) NSNumber * uid;


- (NSArray *) validAttributes;
- (NSString *) xmlns;
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