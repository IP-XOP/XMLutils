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
		cur = xmlSetProp(nodes->nodeTab[i], (xmlChar*)attribute, (xmlChar*)entityEncoded);
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
	
	extern std::map<int,igorXMLfile> allXMLfiles;
	int fileID = -1;
	xmlXPathObject *xpathObj = NULL; 
	xmlDoc *doc = NULL;
	
	//Xpath handle,namespace handle,options handle
	char *xPath = NULL;
	char *ns    = NULL;
	char *attribute = NULL;
	char *value = NULL;
	//size of handles
	int sizexPath,sizens,sizeattribute,sizevalue;

			
	if(p->xPath == NULL || p->ns == NULL || p->attribute == NULL || p->val == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	sizexPath = GetHandleSize(p->xPath);
	sizens = GetHandleSize(p->ns);
	sizeattribute = GetHandleSize(p->attribute);
	sizevalue = GetHandleSize(p->val);
	
	//allocate space for the C-strings.
	xPath = (char*)malloc(sizexPath*sizeof(char)+1);
	if(xPath == NULL){
		err = NOMEM;goto done;
	}
	ns = (char*)malloc(sizens*sizeof(char)+1);
	if(ns == NULL){
		err = NOMEM;goto done;
	}
	attribute = (char*)malloc(sizeattribute*sizeof(char)+1);
	if(attribute == NULL){
		err = NOMEM;goto done;
	}	
	value =  (char*)malloc(sizevalue*sizeof(char)+1);
	if(value == NULL){
		err = NOMEM;goto done;
	}	
	
	/* get all of the igor input strings into C-strings */
	if (err = GetCStringFromHandle(p->xPath, xPath, sizexPath))
		goto done;
	if (err = GetCStringFromHandle(p->ns, ns, sizens))
		goto done;
	if (err = GetCStringFromHandle(p->attribute, attribute, sizeattribute))
		goto done;
	if (err = GetCStringFromHandle(p->val, value, sizevalue))
		goto done;
		
	//check if the node name is invalid
	if(xmlValidateName(BAD_CAST attribute , 0) != 0){
		err = INVALID_NODE_NAME;
		goto done;
	}
	
	fileID = (int)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[p->fileID].doc);
	}
	
	//execute Xpath expression
	xpathObj = execute_xpath_expression(doc, BAD_CAST xPath, BAD_CAST ns, &err);
	if(err)
		goto done;
		
	set_attr(doc, xpathObj->nodesetval,attribute,value);
	
	p->returnval = err;
done:
	if(xpathObj != NULL)
		xmlXPathFreeObject(xpathObj); 
	if(xPath != NULL)
		free(xPath);
	if(ns != NULL)
		free(ns);
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



