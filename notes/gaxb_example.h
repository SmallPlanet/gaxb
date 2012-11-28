<%
-- BIG LUA SCRIPT	
	
%>

- Preprocessor generates a full lua script which is run to generate the code
<%= %>  ==>  print(foo)
<% %>  ==> 	foo
(outside)	==> print('foo')
<% -- Comment %>
	
EXAMPLE:
<% if isSuperElement then %>
#import "<%= string.upper(superElementNameWithNamespace) %>.h"
<% end %>


if isSuperElement then
	print('#import "');
	print(string.upper(superElementNameWithNamespace));
	print('.h');
end


	
LUA GLOBALS
	schema - full schema hierarchy
	this - this is the current element for this template
	arguments - command line arguments pass to gaxb
	schema.xml - access to the original xml
	this.xml - access to the original xml for the current element
	

#ifndef <%= string.upper(elementNameWithNamespace) %>
#define <%= string.upper(elementNameWithNamespace) %>

#import <UIKit/UIKit.h>
<% if schema.isSuperElement then %>
#import "<%= string.upper(superElementNameWithNamespace) %>.h"
// Comment
<% end %>

#ifndef SAFESTRING
#define SAFESTRING(x) ([self translateToXMLSafeString:x])
#endif
@interface <%= elementNameWithNamespace %> : <%= superElementNameWithNamespace %>
{

	<% for attribute in attributes do %>
	<%= attribute.resolvedType %> <%= attribute.name %>;
	<% end %>
}

<% for attribute in attributes do %>
@property (nonatomic<%= if attribute.isBasicType then "" else ",assign" end) <%= attribute.resolvedType %> <%= attribute.name %>;
@property (nonatomic,readonly) <%= attribute.resolvedType %> <%= attribute.name %>Exists;
<% end %>


- (NSArray *) validAttributes;
- (void) setSpinningWithString:(NSString *)string;
- (void) setHidesWhenStoppedWithString:(NSString *)string;
- (void) setStyleWithString:(NSString *)string;
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