<%
FULL_NAME_CAPS = "_"..string.upper(this.namespace).."_"..string.upper(this.name).."BASE".."_";
CAP_NAME = capitalizedString(this.name);
FULL_NAME_CAMEL = capitalizedString(this.namespace).."_"..capitalizedString(this.name).."Base";
SUPERCLASS_OVERRIDE = ""; if (hasSuperclass(this)) then SUPERCLASS_OVERRIDE="override "; end
%>//
// Autogenerated by gaxb at <%= os.date("%I:%M:%S %p on %x") %>
//

import UIKit
import PlanetSwift

public class <%= CAP_NAME %>Base<% if (hasSuperclass(this)) then %> : <%= superclassForItem(this) %><% else %> : GaxbElement<% end %> {
<% if (hasSuperclass(this) == false) then %>
    public var xmlns: String = "<%= this.namespaceURL %>"
    public var parent: GaxbElement?
    public var originalValues = Dictionary<String, String> ()

    init() { }
    public func gaxbPrepare() { }
    public func gaxbDidPrepare() { }

<% end

local sequencesCount = 0;
local hasAnys = false;
for k,v in pairs(this.sequences) do
  sequencesCount = sequencesCount + 1;
  if (v.name == "any") then
    gaxb_print("\tpublic var anys: Array<GaxbElement> = []\n");
    hasAnys = true;
	elseif (isPlural(v)) then
  %>    public var <%= lowercasedString(pluralName(v.name)) %>: Array<<%= simpleTypeForItem(v) %>> = []
<%
  else
  %>    public var <%= lowercasedString(v.name) %>: <%= simpleTypeForItem(v) %>?
<%
	end
end

%>
	public <%= SUPERCLASS_OVERRIDE %>func visit(visitor: (GaxbElement) -> ()) {
<%if hasSuperclass(this) then %>        super.visit(visitor)
<% end %>        visitor(self)
<%for k,v in pairs(this.sequences) do
			if (v.name == "any") then
		 		%>        for any in anys {
            any.visit(visitor)
        }
<%		elseif (isPlural(v)) then
				%>for child in <%= lowercasedString(pluralName(v.name)) %> { child.visit(visitor) }
<%
			else
		 		%><%= lowercasedString(v.name) %>.visit(visitor)
<%		end
		end %>	}

<%

%>    <%= SUPERCLASS_OVERRIDE %>public func setElement(element: GaxbElement, key:String) {
<% if (hasSuperclass(this)) then %>        super.setElement(element, key:key)
<%  end
   if (sequencesCount > 0) then
%>        switch key {<%
      for k,v in pairs(this.sequences) do
        if (v.name ~= "any") then %>
            case "<%= capitalizedString(v.name) %>":
                if let e = element as? <%= capitalizedString(v.name) %> {
<% if (isPlural(v)) then %>                    <%= lowercasedString(pluralName(v.name)) %>.append(e)
                    e.setParent(self)
<% else %>                <%= lowercasedString(v.name) %> = e
                    e.setParent(self)
<%   end %>                }<%
    end %>
            default:<%    end
    if (hasAnys) then %>
                anys.append(element)
                element.setParent(self)
<% else %>
                break
<% end %>        }
<%  end %>    }
<% if (hasSuperclass(this) == false) then %>
    public func setParent(parent: GaxbElement) {
        self.parent = parent
    }
<% end %>
    <%= SUPERCLASS_OVERRIDE %> public func isKindOfClass(className: String) -> Bool {
        if className == "<%= CAP_NAME %>" {
            return true
        }
<% if (hasSuperclass(this)) then
%>        return super.isKindOfClass(className)
<% else
%>        return false
<% end
%>    }
<%
  for k,v in pairs(this.attributes) do %>
	public var <%= v.name %>: <%if (isEnumForItem(v)) then %><%= capitalizedString(this.namespace) %>.<% end %><%= typeForItem(v) %><%
	if (v.default == nil) then %>?<% else %> = <%if (isEnumForItem(v)) then %>.<% end %><%= v.default %><%
	end %>
    func <%= v.name %>AsString() -> String {<%
 if (v.type=="string") then %>
        return <%= v.name %><% if (v.default == nil) then %>!<% end %>
<% elseif (isEnumForItem(v)) then %>
        return <%= v.name %><% if (v.default == nil) then %>!<% end %>.rawValue
<% elseif (isGaxbTypeForItem(v)) then %>
        return <%= v.name %><% if (v.default == nil) then %>!<% end %>.toGaxbString()
<% else %>
        return <%= v.name %><% if (v.default == nil) then %>!<% end %>.toGaxbString()
<% end
%>    }
    public func set<%= capitalizedString(v.name) %>(value: String) {
<%	if (typeNameForItem(v)=="Bool") then
%>        self.<%= v.name %> = value == "true"<%
    elseif (typeNameForItem(v)=="Int") then
%>        self.<%= v.name %> = value.toInt()!<%
elseif (typeNameForItem(v)=="Float") then
%>        self.<%= v.name %> = (value as NSString).floatValue<%
elseif (typeNameForItem(v)=="Double") then
%>        self.<%= v.name %> = (value as NSString).doubleValue<%
elseif (typeNameForItem(v)=="String") then
%>        self.<%= v.name %> = value<%
elseif (isEnumForItem(v)) then
%>        if let tmp = <%= capitalizedString(this.namespace) %>.<%= typeForItem(v) %>(rawValue: value) {
            <%= v.name %> = tmp
        }<%
elseif (isGaxbTypeForItem(v)) then
%>        <%= v.name %> =  <%= typeForItem(v) %>(gaxbString: value)<%
end %>
    }
<%
	end %>
    <%= SUPERCLASS_OVERRIDE %>public func setAttribute(value: String, key:String) {
<% if (hasSuperclass(this)) then %>        super.setAttribute(value, key:key)
<% else %>        originalValues[key] = value
<% end %>        switch key {
<% for k,v in pairs(this.attributes) do
%>            case "<%= v.name %>":
                set<%= capitalizedString(v.name) %>(value)
<% end
%>            default:
                break
        }
    }

    <%= SUPERCLASS_OVERRIDE %>public func imprintAttributes(receiver: GaxbElement?) -> GaxbElement? {
<% if (this.attributes) then
%>       if let obj = receiver as? <%= CAP_NAME %> {
<% for k,v in pairs(this.attributes) do
%>            if <%if (v.default == nil) then %><%= v.name %> != nil && <% end %>obj.originalValues["<%= v.name %>"] == nil {
                obj.<%= v.name %> = <%= v.name %>
            }
<% end
%>       }
<% end %>       return <% if (hasSuperclass(this)) then %>super.imprintAttributes(receiver)<% else %>receiver<% end %>
    }
<%
-- MixedContent is a big todo
	if (this.mixedContent) then %>
@synthesize MixedContent;
-(void) setMixedContent:(NSString *)v
{
    if([v isKindOfClass:[NSString class]] == NO)
    {
        v = [v description];
    }
    MixedContent = v;
};
- (NSString *) MixedContentAsString { return [MixedContent description]; }
- (void) setMixedContentWithString:(NSString *)string
{
	[self setMixedContent:[[NSClassFromString(@"NSString") alloc] initWithString:string]];
}
<%
	end
%>
    <%= SUPERCLASS_OVERRIDE %>public func attributesXML(useOriginalValues:Bool) -> String {
        var xml = ""
        if useOriginalValues {
            for (key, value) in originalValues {
                xml += " \\(key)='\\(value)'"
            }
        } else {
<% for k,v in pairs(this.attributes) do
%><% if (v.default == nil) then %>            if <%= v.name %> != nil {
    <% end %>            xml += " <%= v.name %>='\\(<%= v.name %>AsString())'"
<% if (v.default == nil) then %>            }
    <% end
end
%>    }
<% if (hasSuperclass(this)) then
%>        xml += super.attributesXML(useOriginalValues)
<% end %>
        return xml
    }

    <%= SUPERCLASS_OVERRIDE %>public func sequencesXML(useOriginalValues:Bool) -> String {
        var xml = ""<%
    for k,v in pairs(this.sequences) do
      if (isPlural(v)) then %>
        for <%= lowercasedString(v.name) %> in <%= lowercasedString(pluralName(v.name)) %> {
            xml += <%= lowercasedString(v.name) %>.toXML()
        }<% else %>    xml += <%= lowercasedString(v.name) %>.toXML()<% end
    end
 if (hasSuperclass(this)) then %>
        xml += super.sequencesXML(useOriginalValues)
<% end %>
        return xml
    }

    <%= SUPERCLASS_OVERRIDE %>public func toXML(useOriginalValues:Bool) -> String {
        var xml = "<<%= CAP_NAME %>"
        if (parent == nil || parent?.xmlns != xmlns) {
            xml += " xmlns='\\(xmlns)'"
        }

        xml += attributesXML(useOriginalValues)

        var sXML = sequencesXML(useOriginalValues)
        xml += sXML == "" ? "/>" : ">\\n\\(sXML)</<%= CAP_NAME %>>"
        return xml
    }

    <%= SUPERCLASS_OVERRIDE %>public func toXML() -> String {
        return toXML(false)
    }

    <%= SUPERCLASS_OVERRIDE %>public func description() -> String {
        return toXML()
    }

}
