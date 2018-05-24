/*
 *  XMLschemaValidate.cpp
 *  XMLutils
 *
 *  Created by Andrew Nelson on 24/02/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLutils.h"
#include <libxml/xmlschemas.h>


int
XMLschemaValidate(XMLschemaValidateStruct *p){
	//the error code
	int err = 0;
	p->retval = 0;
	
	extern std::map<long,igorXMLfile> allXMLfiles;
	long fileID = -1;
	xmlDoc *doc = NULL;
	xmlDoc *schema_doc = NULL;
	xmlSchemaParserCtxt *parser_ctxt = NULL;
	xmlSchema *schema = NULL;
	xmlSchemaValidCtxt *valid_ctxt = NULL;
	int isValid;
	
	FILE *fp;
	char fullFilePath [MAX_PATH_LEN + 1], nativePath[MAX_PATH_LEN + 1];
	const char* macdelim = ":";
	char* isMAC = NULL;

	//get our XML file
	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLschemaValidate: fileID isn't valid\r");
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[fileID].doc);
	}
	
	if(err = GetCStringFromHandle(p->schemaFileName, fullFilePath, MAX_PATH_LEN))
		goto done;
	//get native filesystem filepath
	if (err = GetNativePath(fullFilePath, nativePath))
		goto done;
		
	//convert native Mac path to UNIX
#ifdef MACIGOR
	//see if its a MAC path by seeing if there is the Mac delimiter : in there
	char unixpath[MAX_PATH_LEN+1];
	if((isMAC = strstr(nativePath, macdelim)) && (err = HFSToPosixPath(nativePath, unixpath, 0)))
		goto done;
	if(isMAC)
		strcpy(nativePath, unixpath);
#endif
	
	fp = fopen(nativePath, "r");
	if( fp ) {
		// exists
		fclose(fp);
	} else {
		err = FILE_NOT_FOUND;
		XOPNotice("XMLschemaValidate: File(path) to schema doesn't exist, or file can't be opened\r");
		goto done;
		// doesnt exist, at least not for reading
	}
	
	schema_doc = xmlReadFile(nativePath, NULL, XML_PARSE_NONET);
    if (schema_doc == NULL) {
        /* the schema cannot be loaded or is not well-formed */
        err = XMLDOC_PARSE_ERROR;
		goto done;
    }
	
    parser_ctxt = xmlSchemaNewDocParserCtxt(schema_doc);
    if (parser_ctxt == NULL) {
        /* unable to create a parser context for the schema */
		err = XML_PARSERCTXT_ERROR;
		goto done;
    }
	
	schema = xmlSchemaParse(parser_ctxt);
    if (schema == NULL) {
        /* the schema itself is not valid */
		err = XMLSCHEMA_NOT_VALID;
		goto done;
    }

	valid_ctxt = xmlSchemaNewValidCtxt(schema);
    if (valid_ctxt == NULL) {
        /* unable to create a validation context for the schema */
		err = SCHEMA_CONTEXT_CREATION_ERROR;
		goto done;
    }
	
	//validate
	isValid = xmlSchemaValidateDoc(valid_ctxt, doc);

    /* force the return value to be non-negative on success */
    if(isValid == 0)
		p->retval = 1;
	
done:
	if(err)
		p->retval = 0;
	if(valid_ctxt)
	   xmlSchemaFreeValidCtxt(valid_ctxt);
	if(schema)
		xmlSchemaFree(schema);
	if(parser_ctxt)
		xmlSchemaFreeParserCtxt(parser_ctxt);
	if(schema_doc)
        xmlFreeDoc(schema_doc);


	if(p->schemaFileName != NULL)
		WMDisposeHandle(p->schemaFileName);
	
	return err;	
}
