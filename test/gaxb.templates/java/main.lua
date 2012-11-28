
TYPEMAP = {};
TYPEMAP["boolean"] = "boolean";
TYPEMAP["short"] = "short";
TYPEMAP["int"] = "int";
TYPEMAP["nonNegativeInteger"] = "int";
TYPEMAP["positiveInteger"] = "int";
TYPEMAP["enum"] = "int";
TYPEMAP["long"] = "long";
TYPEMAP["string"] = "String";
TYPEMAP["base64Binary"] = "Date";
TYPEMAP["string"] = "String";
TYPEMAP["decimal"] = "float";
TYPEMAP["float"] = "float";
TYPEMAP["double"] = "double";
TYPEMAP["byte"] = "byte";
TYPEMAP["date"] = "Date";
TYPEMAP["dateTime"] = "Date";

TYPEPARSE = {};
TYPEPARSE["boolean"] = "Boolean.parseBoolean(s)";
TYPEPARSE["short"] = "Intenger.parseInteger(s)";
TYPEPARSE["int"] = "Intenger.parseInteger(s)";
TYPEPARSE["nonNegativeInteger"] = "Intenger.parseInteger(s)";
TYPEPARSE["positiveInteger"] = "Intenger.parseInteger(s)";
TYPEPARSE["enum"] = "Intenger.parseInteger(s)";
TYPEPARSE["long"] = "Intenger.parseInteger(s)";
TYPEPARSE["string"] = "s";
TYPEPARSE["base64Binary"] = "DateFormat.getDateInstance().parse(s)";
TYPEPARSE["string"] = "s";
TYPEPARSE["decimal"] = "Float.parseFloat(s)";
TYPEPARSE["float"] = "Float.parseFloat(s)";
TYPEPARSE["double"] = "Double.parseDouble(s)";
TYPEPARSE["byte"] = "Intenger.parseInteger(s)";
TYPEPARSE["date"] = "DateFormat.getDateInstance().parse(s)";
TYPEPARSE["dateTime"] = "DateFormat.getDateInstance().parse(s)";


function printAllKeys(t)
	print("===============")
	for k,v in pairs(t) do
		v = t[k];
		
		if(type(v) ~= "userdata") then
			print(k.." : "..type(v).." = "..tostring(v))
		else
			print(k.." : "..type(v))
		end
	end
	print("===============")
end

function capitalizedString(x)
	return x:gsub("^%l", string.upper);
end

function camelString(x)
	return x:gsub("^%u", string.lower);
end

function className(x)
	return x.name;
end

function pluralName(n)
	return n.."s";
end

function isPlural(v)
	if(v.maxOccurs ~= "1") then
		return true;
	end
	return false;
end

function parseCodeForItem(v)
	local t = TYPEPARSE[v.type];
	if(t == nil) then
		-- if t is nil then this is not a simple schema type.  We need to handle all possibilities here:
		t = v;
		
		-- If type is a function, then this is a reference to another type.  Call the function to dereference the other type
		if(type(t.type) == "table") then
			t = t.type;
		end
		
		if(t.type == "element") then
			return "new "..typeForItem(v).."(s)";
		end
		if(t.type == "simple") then
			-- if ENUM, then this is an int
			-- if ENUM_MASK, then this is an int
			-- if NAMED_ENUM, then this is the enum name
			-- if TYPEDEF, then this is a NSString *
			local appinfo = gaxb_xpath(t.xml, "./XMLSchema:annotation/XMLSchema:appinfo");
			if(appinfo ~= nil) then
				appinfo = appinfo[1].content;
			end
			
			if(appinfo == "ENUM" or appinfo == "ENUM_MASK" or appinfo == "NAMED_ENUM") then
				return parseCodeForItem("int");
			end
			if(appinfo == "TYPEDEF") then
				return parseCodeForItem("string");
			end
			
			
			-- If there is an appinfo, use that
			local appinfo = gaxb_xpath(t.xml, "./XMLSchema:annotation/XMLSchema:appinfo/XMLSchema:java");
			if(appinfo == nil) then
				appinfo = gaxb_xpath(t.xml, "./XMLSchema:annotation/XMLSchema:appinfo");
			end
			if(appinfo ~= nil) then
				return "new "..parseCodeForItem(appinfo[1].content).."(s)";
			end
			
			-- If there is no appinfo, then use the restriction
			local restrict = gaxb_xpath(t.xml, "./XMLSchema:restriction");
			if(restrict ~= nil) then
				-- is this schema type; need to resolve
				return parseCodeForItem(restrict[1].attributes.base)
			end
			
			return "N/A"
		end
	end
	
	return t;
end

function superclassForItem(v)
	if(v.extension ~= nil) then
		return className(v.extension)
	end
	return "Object"
end

function hasSuperclass(v)
	return (v.extension ~= nil)
end

function typeForItem(v)
	local t = TYPEMAP[v.type];
	if(t == nil) then
		-- if t is nil then this is not a simple schema type.  We need to handle all possibilities here:
		t = v;
		
		-- If type is a function, then this is a reference to another type.  Call the function to dereference the other type
		if(type(t.type) == "table") then
			t = t.type;
		end
		
		if(t.type == "element") then
			return className(t);
		end
		if(t.type == "simple") then
			-- if ENUM, then this is an int
			-- if ENUM_MASK, then this is an int
			-- if NAMED_ENUM, then this is the enum name
			-- if TYPEDEF, then this is a NSString *
			local appinfo = gaxb_xpath(t.xml, "./XMLSchema:annotation/XMLSchema:appinfo");
			if(appinfo ~= nil) then
				appinfo = appinfo[1].content;
			end
			
			if(appinfo == "ENUM" or appinfo == "ENUM_MASK" or appinfo == "NAMED_ENUM") then
				return "int"
			end
			if(appinfo == "TYPEDEF") then
				return "String"
			end
			
			
			-- If there is an appinfo, use that
			local appinfo = gaxb_xpath(t.xml, "./XMLSchema:annotation/XMLSchema:appinfo/XMLSchema:java");
			if(appinfo == nil) then
				appinfo = gaxb_xpath(t.xml, "./XMLSchema:annotation/XMLSchema:appinfo");
			end
			if(appinfo ~= nil) then
				return appinfo[1].content;
			end
			
			-- If there is no appinfo, then use the restriction
			local restrict = gaxb_xpath(t.xml, "./XMLSchema:restriction");
			if(restrict ~= nil) then
				-- is this schema type; need to resolve
				return TYPEMAP[restrict[1].attributes.base];
			end
			
			return "N/A"
		end
		
		return "UNDEFINED"
	end
	
	return t;
end
	
function string:split(sep)
    local sep, fields = sep or ":", {}
    local pattern = string.format("([^%s]+)", sep)
    self:gsub(pattern, function(c) fields[#fields+1] = c end)
    return fields
end
	
function split2(pString, pPattern)
   local Table = {}  -- NOTE: use {n = 0} in Lua-5.0
   local fpat = "(.-)" .. pPattern
   local last_end = 1
   local s, e, cap = pString:find(fpat, 1)
   while s do
      if s ~= 1 or cap ~= "" then
		  table.insert(Table,cap)
      end
      last_end = e+1
      s, e, cap = pString:find(fpat, last_end)
   end
   if last_end <= #pString then
      cap = pString:sub(last_end)
      table.insert(Table, cap)
   end
   return Table
end		

-- Create a gobal header which includes all of the definition stuff (such as enums)
print("Generating global header file...")
--gaxb_template("global.h", schema.namespace..".h", schema);

-- Run through all of the elements and generate code files for them
for k,v in pairs(schema.elements) do
	print("Generating class file"..className(v).."...")

	gaxb_template("element.java", className(v).."Base"..".java", v);
	gaxb_template("element_imp.java", className(v)..".java", v, false);

end
