/*
 *  XMLsetAttr.cpp
 *  XMLutils
 *
 *  Created by andrew on 13/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
/*
 *  XMLlistAttr.cpp
 *  XMLutils
 *
 *  Created by andrew on 13/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */


/*
 *  XMLstringFromXPath.cpp
 *  XMLutils
 *
 *  Created by andrew on 7/11/07.
 *	based on http://xmlsoft.org/examples/xpath1.c
 *
 */

#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "XMLutils.h"
#ifndef HAVE_MEMUTILS
#include "memutils.h"
#endif
#include "UTF8_multibyte_conv.h"

int 
set_attr(xmlDocPtr doc, xmlNodeSetPtr nodes, char* attribute, char* val) {
    int err = 0;
	int size;
    int i;
	xmlAttr  *cur = NULL;
	xmlChar *entityEncoded = NULL;

	entityEncoded = xmlEncodeEntitiesReentrant(doc, BAD_CAST val);
	
	size = (nodes) ? nodes->nodeNr : 0;
	
	for(i = 0; i < size; ++i) {
		cur = xmlSetProp(nodes->nodeTab[i], (xmlChar*) attribute, (xmlChar*) entityEncoded);
	}

done:
	if(entityEncoded!= NULL)
		xmlFree(entityEncoded);
		
	return err;
}

int
XMLsetAttr(XMLsetAttrStruct *p){
	//the error code
	int err = 0;
	
	extern std::map<long,igorXMLfile> allXMLfiles;
	long fileID = -1;
	xmlXPathObject *xpathObj = NULL; 
	xmlDoc *doc = NULL;
	
	//Xpath handle,namespace handle,options handle
	MemoryStruct xPath, ns, attribute, value;
	
	if(p->xPath == NULL || p->ns == NULL || p->attribute == NULL || p->val == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	if(xPath.append(*p->xPath, GetHandleSize(p->xPath)) == -1){
		err = NOMEM;
		goto done;
	}
	if(ns.append(*p->ns, GetHandleSize(p->ns)) == -1){
		err = NOMEM;
		goto done;
	}
	if(attribute.append(*p->attribute, GetHandleSize(p->attribute)) == -1){
		err = NOMEM;
		goto done;
	}
	if(value.append(*p->val, GetHandleSize(p->val)) == -1){
		err = NOMEM;
		goto done;
	}
	
	if(err = SystemEncodingToUTF8(&xPath))
		goto done;
	if(err = SystemEncodingToUTF8(&ns))
		goto done;
	if(err = SystemEncodingToUTF8(&attribute))
		goto done;
	if(err = SystemEncodingToUTF8(&value))
	   goto done;
	
	//check if the node name is invalid
	if(xmlValidateName(BAD_CAST attribute.getData() , 0) != 0){
		err = INVALID_NODE_NAME;
		goto done;
	}
	
	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLsetAttr: fileID isn't valid\r");
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[fileID].doc);
	}
	
	//execute Xpath expression
	xpathObj = execute_xpath_expression(doc, BAD_CAST xPath.getData(), BAD_CAST ns.getData(), &err);
	if(err)
		goto done;
	
	if(err =set_attr(doc, xpathObj->nodesetval, (char*) attribute.getData(), (char*) value.getData()))
		goto done;
	
done:
	(err == 0)? (p->retval = 0):(p->retval = -1);
	if(err == FILEID_DOESNT_EXIST ||
		err == XPATH_CONTEXT_CREATION_ERROR ||
		 err == FAILED_TO_REGISTER_NAMESPACE ||
		  err == XPATH_COMPILE_ERROR ||
		   err == UNABLE_TO_EVAL_XPATH_EXPR){
		err = 0;
	}
	
	if(xpathObj != NULL)
		xmlXPathFreeObject(xpathObj); 
	if(p->xPath != NULL)
		DisposeHandle(p->xPath);
	if(p->ns != NULL)	
		DisposeHandle(p->ns);
	if(p->attribute != NULL)	
		DisposeHandle(p->attribute);
	if(p->val != NULL)
		DisposeHandle(p->val);
		
	return err;	
}



