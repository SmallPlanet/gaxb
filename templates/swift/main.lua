
TYPEMAP = {};
TYPEMAP["boolean"] = "Bool";
TYPEMAP["short"] = "Int";
TYPEMAP["int"] = "Int";
TYPEMAP["integer"] = "Int";
TYPEMAP["nonNegativeInteger"] = "Int";
TYPEMAP["positiveInteger"] = "Int";
TYPEMAP["enum"] = "Int";
TYPEMAP["long"] = "long";
TYPEMAP["string"] = "String";
TYPEMAP["base64Binary"] = "Data";
TYPEMAP["string"] = "String";
TYPEMAP["decimal"] = "Double";
TYPEMAP["float"] = "Float";
TYPEMAP["double"] = "Double";
TYPEMAP["byte"] = "Character";
TYPEMAP["date"] = "Date";
TYPEMAP["dateTime"] = "Date";


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

function table.val_to_str ( v )
  if "string" == type( v ) then
    v = string.gsub( v, "\n", "\\n" )
    if string.match( string.gsub(v,"[^'\"]",""), '^"+$' ) then
      return "'" .. v .. "'"
    end
    return '"' .. string.gsub(v,'"', '\\"' ) .. '"'
  else
    return "table" == type( v ) and table.tostring( v ) or
      tostring( v )
  end
end

function table.key_to_str ( k )
  if "string" == type( k ) and string.match( k, "^[_%a][_%a%d]*$" ) then
    return k
  else
    return "[" .. table.val_to_str( k ) .. "]"
  end
end

function table.tostring( tbl )
  local result, done = {}, {}
  if (tbl == nil) then return end
  for k, v in ipairs( tbl ) do
    table.insert( result, table.val_to_str( v ) )
    done[ k ] = true
  end
  for k, v in pairs( tbl ) do
    if not done[ k ] then
      table.insert( result,
        table.key_to_str( k ) .. "=" .. table.val_to_str( v ) )
    end
  end
  return "{" .. table.concat( result, "," ) .. "}"
end


function table.insertIfNotPresent(t,x)
	for _,v in ipairs(t) do
		if (v == x) then
			return
		end
	end
	table.insert(t,x)
end

function string.split(str,sep)
        local sep, fields = sep or ":", {}
        local pattern = string.format("([^%s]+)", sep)
        str:gsub(pattern, function(c) fields[#fields+1] = c end)
        return fields
end

function capitalizedString(x)
	if (x == nil) then return nil; end
	return (x:gsub("^%l", string.upper));
end

function lowercasedString(x)
	return (x:gsub("^.", string.lower,1));
end

function cleanedName(v)
	-- adds underscore to certain reserved names like class, id, restrict
	if (v == "class") then
		return "_class";
	elseif (v == "restrict") then
		return "_restrict";
	else
		return v;
	end
end

function className(x)
	--printAllKeys(x)
	return capitalizedString(x.name);
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

function superclassForItem(v)
	if(v.extension ~= nil) then
		return className(v.extension)
	end
	return ""
end

function hasSuperclass(v)
	return (v.extension ~= nil)
end

function classNameFromRef(t)
	local r = t.ref;
	local parts = string.split(r,":");
	if (#parts == 2) then
		return capitalizedString(parts[2]);
	else
		return "UNKNOWN_REF"
	end
end

function simpleTypeForItem(v)
	local t = TYPEMAP[v.type];
	if (t ~= nil) then
		return v.type;
	end

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
		-- if simpleType, just grab the original type from xml
		return lowercasedString(v.type.namespace)..":"..v.type.name;
	end
	if (t.ref ~= nil) then
		return 	classNameFromRef(t)
	end
	print(table.tostring(t))
	print(typeForItem(v))
	return "UNDEFINEDX"
end

function isEnumForItem(v)
	local t = TYPEMAP[v.type];
	if (t == nil) then
		t = v;
		-- If type is a function, then this is a reference to another type.  Call the function to dereference the other type
		if(type(t.type) == "table") then
			t = t.type;
		end
		if(t.type == "simple") then
			-- if ENUM, then this is an int
			-- if ENUM_MASK, then this is an int
			-- if NAMED_ENUM, then this is the enum name
			-- if TYPEDEF, then this is a String
			local appinfo = gaxb_xpath(t.xml, "./XMLSchema:annotation/XMLSchema:appinfo");
			if(appinfo ~= nil) then
				appinfo = appinfo[1].content;
			end

			if(appinfo == "ENUM" or appinfo == "ENUM_MASK" or appinfo == "NAMED_ENUM") then
				return "true"
			end
		end
	end
	return false;
end

function isGaxbTypeForItem(v)
	local t = TYPEMAP[v.type];
	if (t == nil) then
		t = v;
		-- If type is a function, then this is a reference to another type.  Call the function to dereference the other type
		if(type(t.type) == "table") then
			t = t.type;
		end
		if(t.type == "simple") then
			-- if ENUM, then this is an int
			-- if ENUM_MASK, then this is an int
			-- if NAMED_ENUM, then this is the enum name
			-- if TYPEDEF, then this is a String
			local appinfo = gaxb_xpath(t.xml, "./XMLSchema:annotation/XMLSchema:appinfo");
			if(appinfo ~= nil) then
				appinfo = appinfo[1].content;
			end

			if(appinfo == "ENUM" or appinfo == "ENUM_MASK" or appinfo == "NAMED_ENUM") then
				return false;
			end
			return true;
		end
	end
	return false;
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

		if(t.ref ~= nil) then
			return classNameFromRef(t).."";
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
				return t.name;
			end
			if(appinfo == "TYPEDEF") then
				return "String"
			end

			-- If there is an appinfo, use that
			local appinfo = gaxb_xpath(t.xml, "./XMLSchema:annotation/XMLSchema:appinfo/XMLSchema:objc");
			if(appinfo == nil) then
				appinfo = gaxb_xpath(t.xml, "./XMLSchema:annotation/XMLSchema:appinfo");
			end
			if(appinfo ~= nil) then
				appinfo = appinfo[1].content;
				local type = TYPEMAP[appinfo];
				if(type == nil) then
					return appinfo;
				else
					return type;
				end
			end

			-- If there is no appinfo, then use the restriction
			local restrict = gaxb_xpath(t.xml, "./XMLSchema:restriction");
			if(restrict ~= nil) then
				-- is this schema type; need to resolve
				return TYPEMAP[restrict[1].attributes.base];
			end

			return "N/A_typeForItem"
		end
		--print(table.tostring(v));
		return "UNDEFINED_typeForItem";
	end

	return t;
end

function typeNameForItem(v)
	local t = typeForItem(v);
	return t;
end

function isObject(v)
	-- return true if v represents an NSObject descendant, false otherwise (ie should we retain this guy)
	local t = TYPEMAP[v.type];
	if (t ~= nil) then
		-- t is not nil so we know the type already.
		return true;
	else
		-- if t is nil then this is not a simple schema type.  We need to handle all possibilities here:
		t = v;

		-- If type is a function, then this is a reference to another type.  Call the function to dereference the other type
		if(type(t.type) == "table") then
			t = t.type;
		end

		if(t.ref ~= nil) then
			return true;
		end

		if(t.type == "element") then
			return false;
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
				return false
			end
			if(appinfo == "TYPEDEF") then
				return true
			end

			-- If there is an appinfo, use that
			if(appinfo ~= nil) then
				-- is this schema type? If it is, it isn't an object
				local type = TYPEMAP[appinfo];
				if (type == nil or string.sub(type,-1) == "*") then
					return true;
				else
					return false;
				end
			end

			-- If there is no appinfo, then use the restriction
			local restrict = gaxb_xpath(t.xml, "./XMLSchema:restriction");
			if(restrict ~= nil) then
				-- is this schema type; need to resolve
				local type =  TYPEMAP[restrict[1].attributes.base];
				if (string.sub(type,-1) == "*") then
					return true;
				else
					return false;
				end
			end
			-- what to do if we get here?
			print("unhandled case in isObject(): 1")
			return "N/A_isObject"
		end
		-- or here?
		print("unhandled case in isObject(): 2 *** ")
		--print(table.tostring(v));
		return "UNDEFINED_isObject"
	end
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
gaxb_template("global_base.swift", schema.namespace.."Base.swift", schema);
gaxb_template("global.swift", schema.namespace..".swift", schema, false);

for k,v in pairs(schema.simpleTypes) do
	if (schema.namespace == v.namespace) then
		local appinfo = gaxb_xpath(v.xml, "./XMLSchema:annotation/XMLSchema:appinfo");
		local enums = gaxb_xpath(v.xml, "./XMLSchema:restriction/XMLSchema:enumeration");

		if(appinfo ~= nil) then
			appinfo = appinfo[1].content;
		end

--		if(appinfo == "ENUM" or appinfo == "NAMED_ENUM" or appinfo == "ENUM_MASK" or appinfo == "TYPEDEF") then
--			gaxb_template("constants.h", schema.namespace.."_XMLConstants.h", schema);
--			break;
--		end
	end
end

-- Run through all of the elements and generate code files for them
for k,v in pairs(schema.elements) do
	-- if not in the schema namespace, skip
	if (schema.namespace == v.namespace) then
		v.name = cleanedName(v.name);
		for k1,v1 in pairs(v.attributes) do
			v1.name = cleanedName(v1.name);
		end
		for k1,v1 in pairs(v.sequences) do
			v1.name = cleanedName(v1.name);
		end
		print("Generating class file "..className(v).."...")
		gaxb_template("element.swift", className(v)..".swift", v, false);
		gaxb_template("element_base.swift", className(v).."Base.swift", v);

		--gaxb_template("structures.swift", "structures.swift", v, false);
	end
end
