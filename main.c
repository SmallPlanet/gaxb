/* gaxb - Portable Architecture for XML Bindings
* 
* Things that need done:
* 1) Be able to parse an XML schema document, provide that document to a lua vm
* 2) Be able to pre-process a template file from template code to lua code
* 3) Be able to run scripts through a lua vm
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/dict.h>
#include <libxml/hash.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/pattern.h>

#include <libxml/xmlschemas.h>
#include <libxml/schemasInternals.h>
#include <libxml/xmlschemastypes.h>

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#define XMLCHAR(x) (xmlChar *)(x)

typedef struct _xmlSchemaBucket xmlSchemaBucket;
typedef xmlSchemaBucket *xmlSchemaBucketPtr;

struct _xmlSchemaBucket {
    int type;
    int flags;
    const xmlChar *schemaLocation;
    const xmlChar *origTargetNamespace;
    const xmlChar *targetNamespace;
    xmlDocPtr doc;
    void * relations;
    int located;
    int parsed;
    int imported;
    int preserveDoc;
    //xmlSchemaItemListPtr globals; /* Global components. */
    //xmlSchemaItemListPtr locals; /* Local components. */
};

#pragma mark -

static const char * TEMPLATE_BASE_PATH = "";
static const char * LANGUAGE_ID = NULL;
static const char * SCHEMA_PATH = NULL;
static const char * OUTPUT_PATH = "Generated";
static lua_State * luaVM = NULL;
static FILE * currentOutputFile = NULL;

static xmlSchemaPtr currentSchema = NULL;
static xmlXPathContextPtr xpathCtx = NULL;

static const xmlChar * copyToHashNamespacePrefix;

#pragma mark Prototypes

static char *trimNewline(char *s);

static void * hashLookup(xmlHashTablePtr table, const xmlChar * key);
static void copyNodeToLua(xmlNodePtr node, char * globalName);
static void copyNodeSetToLua(xmlNodeSetPtr nodes);

static void linkElementExtension(xmlNode * a_node);
static void copyElementAttributesToLua(xmlNode * a_node);
static void copyElementSequencesAttributesToLua(xmlNode * a_node);
static void copyElementAppinfosToLua(xmlNode * a_node);

static int copyToHash(void * payload, xmlHashTablePtr destination, xmlChar * name);
static int loadSchemaImports(void * payload, xmlSchemaPtr schema, xmlChar * name);
static int copyToLua(void * payload, xmlSchemaPtr schema, xmlChar * name);
static char parseSchemaFile();

static void preprocessTemplateFile(const char * inputPath, char * outputPath);

static void lua_init();
static void lua_destruct();
static void lua_run(char * scriptFormat, ...);
static void lua_runFile(const char * path);

static const char * pathForOutputFile(const char * path);
static const char * pathForTemplateFile(const char * path);
static void usage();


#pragma mark -

#pragma mark Helper functions

char *trimNewline(char *s)
{
    if (strchr(s, '\n'))
    {
        char *back = s + strlen(s);
        while ((*--back) != '\n');
        *(back) = '\0';
    }
    return s;
}

#pragma mark Schema Parser

void * hashLookup(xmlHashTablePtr table, const xmlChar * key)
// combined hash lookup function which properly handles namespaces
{
    xmlChar * localKey = xmlStrdup(key);
    xmlChar * nsPtr = XMLCHAR(strrchr((const char *)localKey, ':'));
    void * retVal;
    
    if(nsPtr)
    {
        // Check to see if this is the target namespace of the document; if it is, just strip the prefix
        xmlNodePtr schemaRootPtr = currentSchema->doc->children;
        xmlNsPtr ns = schemaRootPtr->nsDef;
        xmlChar * targetNamespace = xmlGetProp(schemaRootPtr, XMLCHAR("targetNamespace"));
        nsPtr[0] = 0;
        
        while (ns != NULL)
        {
            if(!xmlStrcmp(ns->prefix, localKey) || !xmlStrcmp(ns->href, localKey))
            {
                if(!xmlStrcmp(ns->href, targetNamespace))
                {
                    void * userdata = xmlHashLookup(table, nsPtr+1);
                    free(localKey);
                    return userdata;
                }
                else
                {
                    // We want to look it up using the full href prefixed to the localKey
                    char scratch[1024] = {0};
                    snprintf(scratch, 1024, "%s:%s", ns->href, nsPtr+1);
                    return xmlHashLookup(table, XMLCHAR(scratch));
                }
            }
            ns = ns->next;
        }
        
        nsPtr[0] = ':';
        
        retVal = xmlHashLookup(table, localKey);
        
        
        free(localKey);
        
        return retVal;
    }
    
    free(localKey);
    
    return xmlHashLookup(table, key);
}

void linkElementExtension(xmlNode * a_node)
{
    xmlNode *cur_node = a_node;
    
    if(!xmlStrcmp(a_node->name, XMLCHAR("extension")))
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            const xmlChar * ref = xmlGetProp(cur_node, XMLCHAR("base"));
            xmlSchemaAttributeGroupPtr p = (xmlSchemaAttributeGroupPtr)hashLookup(currentSchema->elemDecl, ref);
            
            if(p && p->type == XML_SCHEMA_TYPE_ELEMENT)
            {   
                lua_run("table.insert(__DEREFERENCE_AT_END, {table=TEMP,key='extension',ref='%s'})", ref);
                return;
            }
        }
    }
}

void copyElementAttributesToLua(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;
    
    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if(!xmlStrcmp(cur_node->name, XMLCHAR("attributeGroup")))
            {
                const xmlChar * ref = xmlGetProp(cur_node, XMLCHAR("ref"));
                xmlSchemaAttributeGroupPtr p = (xmlSchemaAttributeGroupPtr)hashLookup(currentSchema->attrgrpDecl, ref);

                if(p && p->type == XML_SCHEMA_TYPE_ATTRIBUTEGROUP)
                {
                    copyElementAttributesToLua(p->node);
                }
            }
            
            
            if(!xmlStrcmp(cur_node->name, XMLCHAR("attribute")))
            {
                lua_run("TEMP1 = {}");
                
                
                // TODO: is this really what we want to do?  I want auto referencing, but what
                // happens if there is a naming conflict?
                for(xmlAttrPtr attr = cur_node->properties; NULL != attr; attr = attr->next)
                {
                    lua_run("TEMP1['%s'] = '%s'", attr->name, attr->children->content);
                    
                    if(!xmlStrcmp(attr->name, XMLCHAR("type")))
                    {
                        if(xmlStrchr(attr->children->content, ':'))
                        {
                            lua_run("table.insert(__DEREFERENCE_AT_END, {table=TEMP1,key='type',ref='%s'})", attr->children->content);
                            //lua_run("TEMP1.type = function() return gaxb_reference('%s'); end", attr->children->content);
                        }
                    }
                }
                
                lua_run("table.insert(TEMP.attributes, TEMP1)");
            }
        }
    }
}

void copyElementSequencesAttributesToLua(xmlNode * a_node)
{
    xmlNode *cur_node = NULL, *seq_node = NULL;
    
    // a_node points to the <complexType/>.  Find the <sequence/>
    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE && !xmlStrcmp(cur_node->name, XMLCHAR("sequence")))
        {
            for (seq_node = cur_node->children; seq_node; seq_node = seq_node->next)
            {
                if (seq_node->type == XML_ELEMENT_NODE)
                {
                    if (!xmlStrcmp(seq_node->name, XMLCHAR("element")))
                    {
                        const xmlChar * type = xmlGetProp(seq_node, XMLCHAR("ref"));
                        if(type == NULL)
                        {
                            type = xmlGetProp(seq_node, XMLCHAR("type"));
                        }
                        
                        if(type == NULL)
                        {
                            // TODO: support embedded element definitions?
                            fprintf(stderr, "WARNING: embedded element definitions are not supported\n");
                        }
                        // 2) Elements can also be defined with a type attribute, which points to a type definition which contains their content
                        if(type != NULL)
                        {
                            lua_run("TEMP1 = {}");
                            
                            
                            // set a light ptr to the xmlNode
                            lua_getglobal(luaVM, "TEMP1");
                            lua_pushstring(luaVM,"xml");
                            lua_pushlightuserdata(luaVM, (void *)seq_node);
                            lua_settable(luaVM, -3);
                            
                            lua_run("TEMP1.type = '%s'", "element");
                            lua_run("TEMP1.namespace = '%s'", (strrchr((const char *)seq_node->ns->href, '/')+1));
                            lua_run("TEMP1.namespaceURL = '%s'", (const char *)seq_node->ns->href);
                            
                            // TODO: is this really what we want to do?  I want auto referencing, but what
                            // happens if there is a naming conflict?
                            for(xmlAttrPtr attr = seq_node->properties; NULL != attr; attr = attr->next)
                            {
                                lua_run("TEMP1['%s'] = '%s'", attr->name, attr->children->content);
                                
                                if(!xmlStrcmp(attr->name, XMLCHAR("ref")))
                                {
                                    if(xmlStrchr(attr->children->content, ':'))
                                    {
                                        const xmlChar * name = xmlStrchr(attr->children->content, ':')+1;
                                        
										lua_run("local result = false");
										lua_run("for k,v in pairs(__DEREFERENCE_AT_END) do if (v.ref == '%s') then result = true; break; end; end", attr->children->content);
										lua_run("if (result == false) then table.insert(__DEREFERENCE_AT_END, {table=TEMP1,key='type',ref='%s'}) end", attr->children->content);
                                        //lua_run("TEMP1.type = function() return gaxb_reference('%s'); end", attr->children->content);
                                        lua_run("TEMP1.name = '%s'", name);
                                    }
                                }
                            }
                                                    
                            lua_run("table.insert(TEMP.sequences, TEMP1)");
                        }
                    } 
                    else if (!xmlStrcmp(seq_node->name, XMLCHAR("any")))
                    {
                        lua_run("TEMP1 = {}");
                        
                        // set a light ptr to the xmlNode
                        lua_getglobal(luaVM, "TEMP1");
                        lua_pushstring(luaVM,"xml");
                        lua_pushlightuserdata(luaVM, (void *)seq_node);
                        lua_settable(luaVM, -3);
                        
                        lua_run("TEMP1.type = '%s'", "element");
                        lua_run("TEMP1.namespace = '%s'", (strrchr((const char *)seq_node->ns->href, '/')+1));
                        lua_run("TEMP1.namespaceURL = '%s'", (const char *)seq_node->ns->href);
                        lua_run("TEMP1.name = 'any'");
                        lua_run("table.insert(TEMP.sequences, TEMP1)");
                    }
                }
            }
        }
    }
}

void copyElementAppinfosToLua(xmlNode * a_node)
{
    // give me an annotation node
    xmlNode *cur_node = NULL;
    
    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (!xmlStrcmp(cur_node->name, XMLCHAR("appinfo")))
        {
            lua_run("table.insert(TEMP.appinfos, '%s')", trimNewline(cur_node->children->content));
        }
    }
}

int copyToLua(void * payload, xmlSchemaPtr schema, xmlChar * name)
{
    // copyToLua is responsible for taking libXML schema data types and transposing them to lua tables
    // for easy access from the template writers.
    int type = ((xmlSchemaElementPtr)payload)->type;
    
    if(type == XML_SCHEMA_TYPE_ELEMENT)
    {
        xmlSchemaElementPtr p = (xmlSchemaElementPtr)payload;
        lua_run("TEMP = {}");
        lua_run("TEMP.name = '%s'", p->name);
        lua_run("TEMP.type = '%s'", "element");
        lua_run("TEMP.namespace = '%s'", strrchr((char *)p->targetNamespace, '/')+1);
        lua_run("TEMP.namespaceURL = '%s'", p->targetNamespace);
        lua_run("TEMP.attributes = {}");
        lua_run("TEMP.sequences = {}");
        lua_run("TEMP.appinfos = {}");
        
        // set a light ptr to the xmlNode
        lua_getglobal(luaVM, "TEMP");
		lua_pushstring(luaVM,"xml");
		lua_pushlightuserdata(luaVM, (void *)p->node);
		lua_settable(luaVM, -3);

        // TODO: expose element attributes to lua.  Unfortunately, libxml2 doesn't make it easy as it could for us,
        // so we need to drop down from the schema ptr to the actual XML node and parse it by hand.
        
        // 1) Elements can be defined all in one block; aka, they contain their information
        const xmlChar * type = xmlGetProp(p->node, XMLCHAR("type"));
        if(type == NULL)
        {
            xmlNode *parseNode = p->node;
            if (!xmlStrcmp(parseNode->name, XMLCHAR("element")))
            {
                parseNode = parseNode->children;
            }
            if (!xmlStrcmp(parseNode->name, XMLCHAR("complexType")))
            {
                for (xmlAttr *prop = parseNode->properties; prop; prop=prop->next)
                {
                    if (!xmlStrcmp(prop->name, XMLCHAR("mixed")))
                    {
                        if (!xmlStrcmp(prop->children->content, XMLCHAR("true")))
                        {
                            lua_run("TEMP.mixedContent = true");
                        }
                    }
                }
            }
            for (xmlNode *child = parseNode->children; child; child = child->next)
            {
                if (!xmlStrcmp(child->name, XMLCHAR("complexContent")) && !xmlStrcmp(child->children->name, XMLCHAR("extension")))
                {
                    parseNode = child->children;
                    for (xmlNode *grandchild = parseNode->children; grandchild; grandchild=grandchild->next)
                    {
                        if (!xmlStrcmp(grandchild->name, XMLCHAR("annotation")))
                        {
                            copyElementAppinfosToLua(grandchild);
                        }
                    }
                }
                if (!xmlStrcmp(child->name, XMLCHAR("annotation")))
                {
                    copyElementAppinfosToLua(child);
                }
            }
            linkElementExtension(parseNode);
            copyElementAttributesToLua(parseNode);
            copyElementSequencesAttributesToLua(parseNode);
        }

        // 2) Elements can also be defined with a type attribute, which points to a type definition which contains their content
        if(type != NULL)
        {
            xmlSchemaTypePtr p = (xmlSchemaTypePtr)hashLookup(schema->typeDecl, type);
            
            if(p && p->type == XML_SCHEMA_TYPE_COMPLEX)
            {
                xmlNode *parseNode = p->node;
                if (!xmlStrcmp(parseNode->name, XMLCHAR("complexType")))
                {
                    for (xmlAttr *prop = parseNode->properties; prop; prop=prop->next)
                    {
                        if (!xmlStrcmp(prop->name, XMLCHAR("mixed")))
                        {
                            if (!xmlStrcmp(prop->children->content, XMLCHAR("true")))
                            {
                                lua_run("TEMP.mixedContent = true");
                            }
                        }
                    }
                }
                for (xmlNode *child = p->node->children; child; child = child->next)
                {
                    if (!xmlStrcmp(child->name, XMLCHAR("complexContent")) && !xmlStrcmp(child->children->name, XMLCHAR("extension")))
                    {
                        parseNode = child->children;
                        for (xmlNode *grandchild = parseNode->children; grandchild; grandchild=grandchild->next)
                        {
                            if (!xmlStrcmp(grandchild->name, XMLCHAR("annotation")))
                            {
                                copyElementAppinfosToLua(grandchild);
                            }
                        }
                    }
                    if (!xmlStrcmp(child->name, XMLCHAR("annotation")))
                    {
                        copyElementAppinfosToLua(child);
                    }
                }
                linkElementExtension(parseNode);
                copyElementAttributesToLua(parseNode);
                copyElementSequencesAttributesToLua(parseNode);
            }
            else
            {
                // TODO: no type.  this must be a simple type (like string)
                fprintf(stderr, "WARNING: simple types for root elements not implemented\n");
            }
        }
                
        if(name)
        {
            lua_run("table.insert(schema.elements, TEMP)");
        }
    }
    else if(type == XML_SCHEMA_TYPE_ATTRIBUTE)
    {
        xmlSchemaElementPtr p = (xmlSchemaElementPtr)payload;
        lua_run("TEMP = {}");
        lua_run("TEMP.name = '%s'", p->name);
        lua_run("TEMP.type = '%s'", "attribute");
        lua_run("TEMP.namespace = '%s'", strrchr((char *)p->targetNamespace, '/')+1);
        lua_run("TEMP.namespaceURL = '%s'", p->targetNamespace);
        
        // set a light ptr to the xmlNode
        lua_getglobal(luaVM, "TEMP");
		lua_pushstring(luaVM,"xml");
		lua_pushlightuserdata(luaVM, (void *)p->node);
		lua_settable(luaVM, -3);
        
        if(name)
            lua_run("table.insert(schema.attributes, TEMP)");
    }
    else if(type == XML_SCHEMA_TYPE_ATTRIBUTEGROUP)
    {
        /*
        xmlSchemaAttributeGroupPtr p = (xmlSchemaAttributeGroupPtr)payload;
        lua_run("TEMP = {}");
        lua_run("TEMP.name = '%s'", p->name);
        lua_run("TEMP.type = '%s'", "attributegroup");
        lua_run("TEMP.namespace = '%s'", strrchr((char *)p->targetNamespace, '/')+1);
        lua_run("TEMP.namespaceURL = '%s'", p->targetNamespace);
        
        // set a light ptr to the xmlNode
        lua_getglobal(luaVM, "TEMP");
		lua_pushstring(luaVM,"xml");
		lua_pushlightuserdata(luaVM, (void *)p->node);
		lua_settable(luaVM, -3);
        
        if(name)
            lua_run("table.insert(schema.attributeGroups, TEMP)");
         */
    }
    else if(type == XML_SCHEMA_TYPE_SIMPLE)
    {
        xmlSchemaTypePtr p = (xmlSchemaTypePtr)payload;
        lua_run("TEMP = {}");
        lua_run("TEMP.name = '%s'", p->name);
        lua_run("TEMP.type = '%s'", "simple");
        lua_run("TEMP.namespace = '%s'", strrchr((char *)p->targetNamespace, '/')+1);
        lua_run("TEMP.namespaceURL = '%s'", p->targetNamespace);
        
        // set a light ptr to the xmlNode
        lua_getglobal(luaVM, "TEMP");
		lua_pushstring(luaVM,"xml");
		lua_pushlightuserdata(luaVM, (void *)p->node);
		lua_settable(luaVM, -3);
        
        if(name)
            lua_run("table.insert(schema.simpleTypes, TEMP)");
    }
    else if(type == XML_SCHEMA_TYPE_COMPLEX)
    {
        xmlSchemaTypePtr p = (xmlSchemaTypePtr)payload;
        lua_run("TEMP = {}");
        lua_run("TEMP.name = '%s'", p->name);
        lua_run("TEMP.type = '%s'", "complex");
        lua_run("TEMP.namespace = '%s'", strrchr((char *)p->targetNamespace, '/')+1);
        lua_run("TEMP.namespaceURL = '%s'", p->targetNamespace);
        
        // set a light ptr to the xmlNode
        lua_getglobal(luaVM, "TEMP");
		lua_pushstring(luaVM,"xml");
		lua_pushlightuserdata(luaVM, (void *)p->node);
		lua_settable(luaVM, -3);
        
        if(name)
            lua_run("table.insert(schema.complexTypes, TEMP)");
    }
    else
    {
        fprintf(stderr, "Unknown ptr of type %d\n", type);
        return 0;
    }
    
    return 1;
}

int copyToHash(void * payload, xmlHashTablePtr destination, xmlChar * name)
{
    // Add the namespace specific name here...
    if(strchr((const char *)name, ':') == NULL)
    {
        char scratch[1024] = {0};
        snprintf(scratch, 1024, "%s:%s", copyToHashNamespacePrefix, name);
        if(hashLookup(destination, XMLCHAR(scratch)) == NULL)
        {
            xmlHashAddEntry	(destination, xmlStrdup(XMLCHAR(scratch)), payload);
        }
    }
    else
    {
        if(hashLookup(destination, XMLCHAR(name)) == NULL)
        {
            xmlHashAddEntry	(destination, name, payload);
        }
    }
    
    return 1;
}

int loadSchemaImports(void * payload, xmlSchemaPtr schema, xmlChar * name)
{
    xmlSchemaBucketPtr ptr = (xmlSchemaBucketPtr)payload;
    xmlDocPtr schema_doc = ptr->doc;
    
    xmlSchemaParserCtxtPtr parser_ctxt = xmlSchemaNewDocParserCtxt(schema_doc);
    if (parser_ctxt == NULL)
	{
        /* unable to create a parser context for the schema */
        xmlFreeDoc(schema_doc);
        return -2;
    }
    
    xmlSchemaPtr importedSchema = xmlSchemaParse(parser_ctxt);
    if (importedSchema == NULL)
	{
        /* the schema itself is not valid */
        xmlSchemaFreeParserCtxt(parser_ctxt);
        xmlFreeDoc(schema_doc);
        return -3;
    }
    
    // Now, somehow mux in the definitions...
    xmlNsPtr ns = importedSchema->doc->children->nsDef;
    while (ns != NULL)
    {
        if(ns->prefix != NULL)
        {
            if(xmlStrcmp(ptr->targetNamespace, ns->href) == 0)
            {
                copyToHashNamespacePrefix = ns->href;
            }
        }
        ns = ns->next;
    }
    
    xmlHashScan(importedSchema->idcDef, (xmlHashScanner) copyToHash, schema->idcDef);
    xmlHashScan(importedSchema->typeDecl, (xmlHashScanner) copyToHash, schema->typeDecl);
	xmlHashScan(importedSchema->elemDecl, (xmlHashScanner) copyToHash, schema->elemDecl);
    xmlHashScan(importedSchema->attrDecl, (xmlHashScanner) copyToHash, schema->attrDecl);
    xmlHashScan(importedSchema->attrgrpDecl, (xmlHashScanner) copyToHash, schema->attrgrpDecl);
    xmlHashScan(importedSchema->notaDecl, (xmlHashScanner) copyToHash, schema->notaDecl);
    xmlHashScan(importedSchema->groupDecl, (xmlHashScanner) copyToHash, schema->groupDecl);
    xmlHashScan(importedSchema->schemasImports, (xmlHashScanner) copyToHash, schema->schemasImports);
    
    return 1;
}

char parseSchemaFile()
{
    xmlDocPtr schema_doc = xmlReadFile(SCHEMA_PATH, NULL, XML_PARSE_NONET | XML_PARSE_XINCLUDE | XML_PARSE_NSCLEAN | XML_PARSE_NOENT);
    if (schema_doc == NULL)
	{
        /* the schema cannot be loaded or is not well-formed */
        return -1;
    }
	
    xmlSchemaParserCtxtPtr parser_ctxt = xmlSchemaNewDocParserCtxt(schema_doc);
    if (parser_ctxt == NULL)
	{
        /* unable to create a parser context for the schema */
        xmlFreeDoc(schema_doc);
        return -2;
    }

    xmlSchemaPtr schema = xmlSchemaParse(parser_ctxt);
    if (schema == NULL)
	{
        /* the schema itself is not valid */
        xmlSchemaFreeParserCtxt(parser_ctxt);
        xmlFreeDoc(schema_doc);
        return -3;
    }

    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(schema_doc);
    if(xpathCtx == NULL) {
        fprintf(stderr,"Error: unable to create new XPath context\n");
        xmlFreeDoc(schema_doc); 
        return(-1);
    }

    // Register the namespaces with the xpath context
    xmlNsPtr ns = schema->doc->children->nsDef;
    while (ns != NULL)
    {
        if(ns->prefix == NULL)
        {
            xmlXPathRegisterNs(xpathCtx, XMLCHAR(strrchr((const char *)ns->href, '/')+1), ns->href);
        }
        else
        {
            xmlXPathRegisterNs(xpathCtx, ns->prefix, ns->href);
        }
        ns = ns->next;
    }
    
    // Confirm all hashes are created
    if(!schema->idcDef)
        schema->idcDef = xmlHashCreate(5);
    if(!schema->typeDecl)
        schema->typeDecl = xmlHashCreate(5);
    if(!schema->elemDecl)
        schema->elemDecl = xmlHashCreate(5);
    if(!schema->attrDecl)
        schema->attrDecl = xmlHashCreate(5);
    if(!schema->attrgrpDecl)
        schema->attrgrpDecl = xmlHashCreate(5);
    if(!schema->notaDecl)
        schema->notaDecl = xmlHashCreate(5);
    if(!schema->groupDecl)
        schema->groupDecl = xmlHashCreate(5);
    if(!schema->schemasImports)
        schema->schemasImports = xmlHashCreate(5);
    
    currentSchema = schema;
    
    // Load the imports...
    xmlHashScan(schema->schemasImports, (xmlHashScanner) loadSchemaImports, schema);
    
    lua_run("schema.namespace = '%s'", strrchr((char *)currentSchema->targetNamespace, '/')+1);
    lua_run("schema.namespaceURL = '%s'", currentSchema->targetNamespace);
    
    xmlHashScan(schema->idcDef, (xmlHashScanner) copyToLua, schema);
    xmlHashScan(schema->typeDecl, (xmlHashScanner) copyToLua, schema);
	xmlHashScan(schema->elemDecl, (xmlHashScanner) copyToLua, schema);
    xmlHashScan(schema->attrDecl, (xmlHashScanner) copyToLua, schema);
    xmlHashScan(schema->attrgrpDecl, (xmlHashScanner) copyToLua, schema);
    xmlHashScan(schema->notaDecl, (xmlHashScanner) copyToLua, schema);
    xmlHashScan(schema->groupDecl, (xmlHashScanner) copyToLua, schema);
    xmlHashScan(schema->schemasImports, (xmlHashScanner) copyToLua, schema);
	
	fprintf(stderr, "Schema is valid and available\n");
    
    //xmlXPathFreeContext(xpathCtx);
	//xmlSchemaFreeParserCtxt(parser_ctxt);
    //xmlFreeDoc(schema_doc);
    
    // Run through and dereference all links now that processing is complete
    lua_run("for k,v in pairs(__DEREFERENCE_AT_END) do v.table[v.key] = gaxb_reference(v.ref); end");
    
    return 0;
}

#pragma mark Gaxb Templates

void preprocessTemplateFile(const char * inputPath, char * outputPath)
{
    
    // This function takes a template file and converts it to a valid lua script which can be run through the VM
    // Using the following rules:
    // 1) <%= %>  ==>  print(foo)
    // 2) <% %>  ==> 	foo
    // 3) (outside)	==> print('foo')
    char insideCode = 0;
    char codeBlockIsExpression = 0;
    FILE * input = fopen(inputPath, "rb");
    FILE * output = fopen(outputPath, "wb+");
    long pos;
    
    fprintf(output, "gaxb_print('");
    
    while(1)
    {
        char c = fgetc(input);
        char t;
        if(feof(input))
        {
            break;
        }
        
        // Check for boundary conditions
        pos = ftell(input);
        if(insideCode == 0)
        {
            if(c == '<')
            {
                t = fgetc(input);
                if(t == '%')
                {
                    insideCode = 1;
                    codeBlockIsExpression = 0;
                    
                    t = fgetc(input);
                    if(t == '=')
                    {
                        codeBlockIsExpression = 1;
                    }
                    else
                    {
                        fseek(input, -1, SEEK_CUR);
                    }
                    
                    fprintf(output, "')\n");
                    
                    if(codeBlockIsExpression)
                    {
                        fprintf(output, "gaxb_print(");
                    }
                    
                    continue;
                }
            }
        }
        else
        {
            if(c == '%')
            {
                t = fgetc(input);
                if(t == '>')
                {
                    if(codeBlockIsExpression)
                    {
                        fprintf(output, ")");
                    }
                    fprintf(output, "\n");
                    
                    fprintf(output, "gaxb_print('");
                    insideCode = 0;
                    continue;
                }
            }
        }
        
        fseek(input, pos, SEEK_SET);
        
        
        if(insideCode == 0)
        {
            if(c == '\'')
            {
                fputc('\\', output);
            }
            if(c == '\n')
            {
                fputc('\\', output);
                fputc('n', output);
            }
            else
            {
                fputc(c, output);
            }
        }
        else
        {
            fputc(c, output);
        }
    }
    
    if(insideCode == 0)
    {
        fprintf(output, "')\n");
    }
    else
    {
        if(codeBlockIsExpression)
        {
            fprintf(output, ")\n");
        }
    }
    
    fclose(output);
    fclose(input);
}

#pragma mark Lua

void copyNodeToLua(xmlNodePtr node, char * globalName)
{
    // table to hold an individual result
    xmlNodePtr child;
    
    lua_newtable(luaVM);
    
    lua_pushstring(luaVM, "name");
    lua_pushstring(luaVM, (const char *)node->name);
    lua_settable(luaVM, -3);
    
    lua_pushstring(luaVM, "namespace");
    lua_pushstring(luaVM, (strrchr((const char *)node->ns->href, '/')+1));
    lua_settable(luaVM, -3);
    
    lua_pushstring(luaVM, "namespaceURL");
    lua_pushstring(luaVM, (const char *)node->ns->href);
    lua_settable(luaVM, -3);
    
    lua_pushstring(luaVM, "attributes");
    lua_newtable(luaVM);
    for(xmlAttrPtr attr = node->properties; NULL != attr; attr = attr->next)
    {
        lua_pushstring(luaVM, (const char *)attr->name);
        lua_pushstring(luaVM, (const char *)attr->children->content);
        lua_settable(luaVM, -3);
    }
    
    lua_settable(luaVM, -3);
    
    for (child = node->children; child; child = child->next)
    {
        if( child->type == XML_TEXT_NODE)
        {
            lua_pushstring(luaVM, "content");
            lua_pushstring(luaVM, (const char *)child->content);
            lua_settable(luaVM, -3);
        }
    }
    
    lua_pushstring(luaVM, "children");
    lua_newtable(luaVM);

    // run through all children elements
    int i = 1;
    for (child = node->children; child; child = child->next)
    {
        if (child->type == XML_ELEMENT_NODE)
        {
            lua_pushnumber(luaVM, i++);
            copyNodeToLua(child, NULL);
            lua_settable(luaVM, -3);
        }
    }
    lua_settable(luaVM, -3);
    
    if(globalName)
    {
        lua_setglobal(luaVM, globalName);
    }
}

void copyNodeSetToLua(xmlNodeSetPtr nodes)
{
    xmlNodePtr cur;
    int size = (nodes) ? nodes->nodeNr : 0;
    int i;
    
    if(size == 0)
    {
        lua_run("NODERESULTS = nil");
        return;
    }
    
    // table to hold the results
    lua_run("NODERESULTS = {}");
    
    //fprintf(output, "Result (%d nodes):\n", size);
    for(i = 0; i < size; ++i)
    {
        if(nodes->nodeTab[i]->type == XML_NAMESPACE_DECL)
        {
            xmlNsPtr ns;
            
            ns = (xmlNsPtr)nodes->nodeTab[i];
            cur = (xmlNodePtr)ns->next;
            if(cur->ns) { 
                fprintf(stderr, "= namespace \"%s\"=\"%s\" for node %s:%s\n", 
                        ns->prefix, ns->href, cur->ns->href, cur->name);
            } else {
                fprintf(stderr, "= namespace \"%s\"=\"%s\" for node %s\n", 
                        ns->prefix, ns->href, cur->name);
            }
        }
        else if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE)
        {
            cur = nodes->nodeTab[i];   	    

            copyNodeToLua(cur, "TEMP");
            
            lua_run("table.insert(NODERESULTS, TEMP)");
        }
        else
        {
            cur = nodes->nodeTab[i];    
            fprintf(stderr, "= node \"%s\": type %d\n", cur->name, cur->type);
        }
    }
}


static int _engine_gaxb_xpath(lua_State *ls)
{
    
    int n = lua_gettop(ls);
    
    // _engine_gaxb_xpath() takes 1 or 2 arguments
    
    // If there is one argument, then:
    // 1: the xpath expression
    
    // If there are two arguments, then:
	// 1: a light ptr to the xmlNode to run xpath on
    // 2: the xpath expression
    
    xmlXPathObjectPtr xpathObj;
    
    if (n == 1 || n == 2)
	{
        xmlNode * node = NULL;
        const char * xpathExpr = NULL;
        
        if(n == 1)
        {
            xpathExpr = lua_tostring(ls, 1);
            
            xpathCtx->node = currentSchema->doc->children;
        }
        else if(n == 2)
        {
            node = (xmlNode *)lua_touserdata(ls, 1);
            xpathExpr = lua_tostring(ls, 2);
            
            xpathCtx->node = node;
        }
        
        // Evaluate xpath expression
        xpathObj = xmlXPathEvalExpression(XMLCHAR(xpathExpr), xpathCtx);
        if(xpathObj == NULL) {
            fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"\n", xpathExpr);
            exit(1);
        }
        
        // Do something cool with the results, like put them in a lua table
        copyNodeSetToLua(xpathObj->nodesetval);
        
        lua_getglobal(luaVM, "NODERESULTS");
        
        return 1;
    }
	else
	{
		fprintf(stderr, "_engine_gaxb_xpath called with %d arguments (requires 1 or 2)", n);
	}
    
    if(lua_islightuserdata(ls, 1) == 0)
    {
        fprintf(stderr, "_engine_gaxb_xpath with non-xml node as first argument");
        return 0;
    }
    
    
    
    
    
    
    
    return 1;
}

static int _engine_gaxb_reference(lua_State *ls)
{
	int n = lua_gettop(ls);
    
    // _engine_gaxb_reference() takes 1 arguments
	// 1: a string key ( such as namespace:foo ) to lookup and return the associated object
    
    if (n != 1)
	{
		fprintf(stderr, "_engine_gaxb_reference called with %d arguments (requires 1)", n);
        return 0;
    }
    
    const char * key = lua_tostring(ls, 1);
    
    //fprintf(currentOutputFile, "%s", stringToOutput);
    void * ptr = NULL;
        
    if(!ptr)    ptr = hashLookup(currentSchema->idcDef, XMLCHAR(key));
    if(!ptr)    ptr = hashLookup(currentSchema->elemDecl, XMLCHAR(key));
    if(!ptr)    ptr = hashLookup(currentSchema->typeDecl, XMLCHAR(key));
    if(!ptr)    ptr = hashLookup(currentSchema->attrDecl, XMLCHAR(key));
    if(!ptr)    ptr = hashLookup(currentSchema->attrgrpDecl, XMLCHAR(key));
    if(!ptr)    ptr = hashLookup(currentSchema->notaDecl, XMLCHAR(key));
    if(!ptr)    ptr = hashLookup(currentSchema->groupDecl, XMLCHAR(key));
    if(!ptr)    ptr = hashLookup(currentSchema->schemasImports, XMLCHAR(key));
        
        
    if(ptr)
    {
        if(copyToLua(ptr, currentSchema, NULL))
        {
            lua_getglobal(luaVM, "TEMP");
            return 1;
        }
        
        return 0;
    }
    
    return 0;
}

static int _engine_gaxb_print(lua_State *ls)
{
	int n = lua_gettop(ls);
    
    // _engine_gaxb_print() takes 1 arguments
	// 1: the string to output to the output file, without new line
    
    if (n != 1)
	{
		fprintf(stderr, "_engine_gaxb_print called with %d arguments (requires 1)", n);
        return 0;
    }
    
    const char * stringToOutput = lua_tostring(ls, 1);
    
    fprintf(currentOutputFile, "%s", stringToOutput);
    
    return 0;
}

static int _engine_gaxb_template(lua_State *ls)
{
	int n = lua_gettop(ls);
	
	// _engine_gaxb_template() takes 3 arguments
	// 1: name of the template file to use.  must be in the same directory as the main.lua file
    // 2: path to the output file
    // 3: the table to use as the "this" global variable when running the template
    
    if (n != 3 && n != 4)
	{
		fprintf(stderr, "_engine_gaxb_template called with %d arguments (requires 3)\n", n);
        return 0;
    }
    
    int argType1 = lua_type(ls, 1);
    int argType2 = lua_type(ls, 2);
    int argType3 = lua_type(ls, 3);
    
    int shouldOverwrite = 1;
    
    if(n == 4)
    {
        shouldOverwrite = lua_toboolean(ls, 4);
    }
    
    const char * templateFile = lua_tostring(ls, 1);
    const char * outputFilePath = lua_tostring(ls, 2);
    
    // check if file needs updating based on timestamps of inputPath, outputPath, and main.lua
    struct stat in, out, mainlua, schema;   

    if (!stat(pathForOutputFile(outputFilePath), &out))
    {
        if (!stat(pathForTemplateFile(templateFile), &in) && out.st_mtime > in.st_mtime)
        {
            if (!stat(pathForTemplateFile("main.lua"), &mainlua) && out.st_mtime > mainlua.st_mtime)
            {
                if (!stat(SCHEMA_PATH, &schema) && out.st_mtime > schema.st_mtime)
                {
                    fprintf(stdout, "gaxb_template: skipping %s\n", outputFilePath);
                    return 0;
                }
            }
        }
    }
	
    if(argType1 != LUA_TSTRING)
    {
        fprintf(stderr, "_engine_gaxb_template requires a string as its first argument");
        return 0;
    }
    if(argType2 != LUA_TSTRING)
    {
        fprintf(stderr, "_engine_gaxb_template requires a string as its second argument");
        return 0;
    }
    if(argType3 != LUA_TTABLE)
    {
        fprintf(stderr, "_engine_gaxb_template requires a table as its third argument");
        return 0;
    }
    
    fprintf(stderr, "Processing template %s, output to %s\n", templateFile, outputFilePath);
    
    if(n == 4)
    {
        lua_pop(luaVM, 1);
    }
    
    // This sets the last item on the stack (arg 3) as a global named "this"
	lua_setglobal((lua_State*)luaVM, "this");
    
    
    if(!shouldOverwrite)
    {
        FILE * t = fopen(pathForOutputFile(outputFilePath), "r");
        if(t)
        {
            fclose(t);
            return 0;
        }
        fclose(t);
    }
    
    // 0) open the output file
    currentOutputFile = fopen(pathForOutputFile(outputFilePath), "wb+");
    if(!currentOutputFile)
    {
        fprintf(stderr, "Error: unable to open output file %s", pathForOutputFile(outputFilePath));
        exit(1);
    }
    
    // 1) preprocess the template file to create a lua script we can run through the VM
    char processedFilePath[] = "/tmp/gaxbXXXXXXXX";
    int temp_fd = mkstemp(processedFilePath);
    preprocessTemplateFile(pathForTemplateFile(templateFile), processedFilePath);
    
    // 2) run the script through the VM, route the output to the appropriate output file
    lua_runFile(processedFilePath);
    
    // 3) Profit.
    fclose(currentOutputFile);
    unlink(processedFilePath);
	
	return 0;
}

#pragma mark -

void lua_init()
{
	luaVM = luaL_newstate();
    luaL_openlibs(luaVM);
    
    lua_register(luaVM, "gaxb_template", _engine_gaxb_template);
    lua_register(luaVM, "gaxb_print", _engine_gaxb_print);
    lua_register(luaVM, "gaxb_reference", _engine_gaxb_reference);
    lua_register(luaVM, "gaxb_xpath", _engine_gaxb_xpath);
    
    lua_run("schema = {}");
    lua_run("schema.elements = {}");
    lua_run("schema.attributes = {}");
    lua_run("schema.attributeGroups = {}");
    lua_run("schema.simpleTypes = {}");
    lua_run("schema.complexTypes = {}");
    
    lua_run("__DEREFERENCE_AT_END = {}");
}

void lua_destruct()
{
	if(luaVM)
	{
		lua_close(luaVM);
	}
}
    
void lua_run(char * scriptFormat, ...)
{
    char * s = NULL;
    va_list arguments;
    int err;
    
    va_start ( arguments, scriptFormat );
    vasprintf(&s, (const char *)scriptFormat, arguments);
    va_end ( arguments );
    
    err = luaL_loadstring((lua_State*)luaVM, s);
    if(err > 0)
    {
        // There was an error compiling the lua script
        fprintf(stderr, "Lua Compile Error: %s\n\n%s\n\n", lua_tostring((lua_State*)luaVM, -1), s);
        exit(2);
    }
        
    err = lua_pcall((lua_State*)luaVM, 0, 0, 0);
    if(err > 0)
    {
        fprintf(stderr, "Lua Runtime Error: %s\n\n%s\n\n", lua_tostring((lua_State*)luaVM, -1), s);
        exit(2);
    }
    
    free(s);
}

void lua_runFile(const char * path)
{
    int err;
    
    err = luaL_loadfile(luaVM, path);
    if(err > 0)
    {
        // There was an error compiling the lua script
        fprintf(stderr, "Lua Compile Error for file %s:\n %s\n", path, lua_tostring((lua_State*)luaVM, -1));
        exit(2);
    }
        
    err = lua_pcall((lua_State*)luaVM, 0, 0, 0);
    if(err > 0)
    {
        fprintf(stderr, "Lua Runtime Error for file %s:\n %s\n", path, lua_tostring((lua_State*)luaVM, -1));
        exit(2);
    }
}

#pragma mark -

const char * pathForOutputFile(const char * path)
// Yes, this will leak.  Not too worried about it.
{
    char * s = NULL;
    asprintf(&s, "%s/%s", OUTPUT_PATH, path);
    return (const char *)s;
}

const char * pathForTemplateFile(const char * path)
// Yes, this will leak.  Not too worried about it.
{
    char * s = NULL;
    asprintf(&s, "%s/%s/%s", TEMPLATE_BASE_PATH, LANGUAGE_ID, path);
    return (const char *)s;
}

void usage()
{
	fprintf(stdout, "usage: gaxb <language id> <path to XML schema> -t <path to template directory> -o <path to output directory>\n");
	exit(2);
}

#pragma mark -

int main(int argc, char * const argv[])
{
	if(argc < 3)
	{
		usage();
	}
	
    TEMPLATE_BASE_PATH = argv[0];
	LANGUAGE_ID = argv[1];
	SCHEMA_PATH = argv[2];
	
	// Get optional arguments
	int ch;
    optind = 3;
	while ((ch = getopt(argc, argv, "t:o:")) != -1) {
		switch (ch) {
            case 't':
				TEMPLATE_BASE_PATH = optarg;
				break;
			case 'o':
				OUTPUT_PATH = optarg;
				break;
			case '?':
				usage();
		}
	}
	argc -= optind;
	argv += optind;
	
	fprintf(stderr, "LANGUAGE_ID: %s\n", LANGUAGE_ID);
	fprintf(stderr, "SCHEMA_PATH: %s\n", SCHEMA_PATH);
	fprintf(stderr, "OUTPUT_PATH: %s\n", OUTPUT_PATH);	
	
	LIBXML_TEST_VERSION
	lua_init();
	
	if(parseSchemaFile())
    {
        // error
        exit(1);
    }
    
    // Run the main.lua script
    lua_runFile(pathForTemplateFile("main.lua"));
	
	lua_destruct();
	xmlCleanupParser();
	
	return 0;
}