
TYPEMAP = {};
TYPEMAP["boolean"] = "bool";
TYPEMAP["float"] = "float";
TYPEMAP["double"] = "double";
TYPEMAP["decimal"] = "float";
TYPEMAP["int"] = "int";
TYPEMAP["short"] = "int";
TYPEMAP["nonNegativeInteger"] = "unsigned int";
TYPEMAP["positiveInteger"] = "unsigned int";
TYPEMAP["enum"] = "int";
TYPEMAP["long"] = "long";
TYPEMAP["string"] = "std::string";
TYPEMAP["base64Binary"] = "int *";
TYPEMAP["byte"] = "char";
TYPEMAP["date"] = "time_t";
TYPEMAP["dateTime"] = "time_t";


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


function capitalizedString(x)
	return x:gsub("^%l", string.upper);
end

function camelString(x)
	return x:gsub("^%u", string.lower);
end

function classNameWithNamespace(namespace,name)
	return capitalizedString(namespace).."_"..capitalizedString(name);
end

function className(x)
	return classNameWithNamespace(x.namespace,x.name);
end

function classFilename(x)
	return className(x);
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
		return className(v.extension);
	end
	return "cocos2d::CCObject"
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
			if (t.namespace == "XMLSchema") then
				local pipepos = string.find(t.ref, ":")
				if (pipepos ~= nil) then
					ns = string.sub(t.ref, 1, pipepos-1);
					return classNameWithNamespace(ns,t.name).."*";
				end
			end
			print(table.tostring(t));
			return className(t).."*";
		end
		if(t.type == "simple") then
			-- if ENUM, then this is an int
			-- if ENUM_MASK, then this is an int
			-- if NAMED_ENUM, then this is the enum name
			-- if TYPEDEF, then this is a string
			local appinfo = gaxb_xpath(t.xml, "./XMLSchema:annotation/XMLSchema:appinfo");
			if(appinfo ~= nil) then
				appinfo = appinfo[1].content;
			end
			
			if(appinfo == "ENUM" or appinfo == "ENUM_MASK" or appinfo == "NAMED_ENUM") then
				return "int"
			end
			if(appinfo == "TYPEDEF") then
				return "string"
			end
			
			
			-- If there is an appinfo, use that
			local appinfo = gaxb_xpath(t.xml, "./XMLSchema:annotation/XMLSchema:appinfo/XMLSchema:java"); -- todo
			if(appinfo == nil) then
				appinfo = gaxb_xpath(t.xml, "./XMLSchema:annotation/XMLSchema:appinfo");
			end
			if(appinfo ~= nil) then
				--print(tostring(appinfo[1].content));
				return appinfo[1].content.."*";
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

function isObject(v)
	return (string.sub(typeForItem(v),-1) == "*");
end

function classNameStripped(v)
	return string.sub(v,1,string.len(v)-1);
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
gaxb_template("global.cpp", schema.namespace..".cpp", schema);

-- Run through all of the elements and generate code files for them
for k,v in pairs(schema.elements) do
	print("Generating class file"..classFilename(v).."...");

	gaxb_template("element.cpp", classFilename(v)..".cpp", v);
	gaxb_template("element.h", classFilename(v)..".h", v);

	print("Done with class file");
end






















