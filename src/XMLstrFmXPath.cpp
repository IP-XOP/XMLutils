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
print_xpath_nodes(xmlDocPtr doc, xmlNodeSetPtr nodes, Handle output) {
    int err = 0;
	int size,bufsize;
    int i;
	xmlChar* xmloutputBuf = (xmlChar*)("");
	char *space = " ";
	
	size = (nodes) ? nodes->nodeNr : 0;
    
	if(size == 0){
		if(err = PtrAndHand(xmloutputBuf,output,sizeof(xmloutputBuf)))
			goto done;
	}
	
	for(i = 0; i < size; ++i) {
		assert(nodes->nodeTab[i]);
		xmloutputBuf = xmlNodeListGetString(doc, nodes->nodeTab[i]->xmlChildrenNode, 1);

		bufsize = strlen((char*)xmloutputBuf);
		
		if(err = PtrAndHand(xmloutputBuf,output,bufsize))
			goto done;
		bufsize = strlen(space);			
		if(err = PtrAndHand(space,output,bufsize))
			goto done;	
    }
	
	
done:
if(xmloutputBuf != NULL)
	xmlFree(xmloutputBuf);
	
	return err;
}

int
XMLstrFmXPath(XMLstrFmXpathStructPtr p){
	//the error code
	int err = 0;
	//the output Handle
	Handle output = NULL;
	
	xmlXPathObject *xpathObj = NULL; 
	xmlDoc *doc = NULL;
	
	//the filename handle, Xpath handle,namespace handle,options handle
	char *fileName = NULL;
	char *xPath = NULL;
	char *ns    = NULL;
	char *options = NULL;
	//size of handles
	int sizefileName,sizexPath,sizens,sizeoptions;
	
	//filename will be passed as a native path
	char nativePath[MAX_PATH_LEN+1];	
			
	if(p->fileNameStr == NULL || p->xPath == NULL || p->ns == NULL || p->options == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	sizefileName = GetHandleSize(p->fileNameStr);
	sizexPath = GetHandleSize(p->xPath);
	sizens = GetHandleSize(p->ns);
	sizeoptions = GetHandleSize(p->options);
	
	//allocate space for the C-strings.
	fileName = (char*)malloc(sizefileName*sizeof(char)+1);
	if(fileName == NULL){
		err = NOMEM;goto done;
	}
	xPath = (char*)malloc(sizexPath*sizeof(char)+1);
	if(xPath == NULL){
		err = NOMEM;goto done;
	}
	ns = (char*)malloc(sizens*sizeof(char)+1);
	if(ns == NULL){
		err = NOMEM;goto done;
	}
	options = (char*)malloc(sizeoptions*sizeof(char)+1);
	if(options == NULL){
		err = NOMEM;goto done;
	}
	
	/* get all of the igor input strings into C-strings */
	if (err = GetCStringFromHandle(p->fileNameStr, fileName, sizefileName))
		goto done;
	if (err = GetCStringFromHandle(p->xPath, xPath, sizexPath))
		goto done;
	if (err = GetCStringFromHandle(p->ns, ns, sizens))
		goto done;
	if (err = GetCStringFromHandle(p->options, options, sizeoptions))
		goto done;

	//get native filesystem filepath
	if (err = GetNativePath(fileName,nativePath))
		goto done;
		
	//get a handle for the output
	output = NewHandle(0); 
	if (output == NULL) 
		return NOMEM; 
    
	/* Load XML document */
    doc = xmlParseFile(fileName);
    if (doc == NULL) {
		err = XMLDOC_PARSE_ERROR;
		goto done;
    }
	
	//execute Xpath expression
	xpathObj = execute_xpath_expression(doc, BAD_CAST xPath, BAD_CAST ns, &err);
	if(err)
		goto done;
	//and print it out to a handle
	print_xpath_nodes(doc, xpathObj->nodesetval, output);
	
	p->returnString = output;
done:
	if(doc != NULL)
		xmlFreeDoc(doc); 
	if(xpathObj != NULL)
		xmlXPathFreeObject(xpathObj); 
	if(fileName != NULL)
		free(fileName);
	if(xPath != NULL)
		free(xPath);
	if(ns != NULL)
		free(ns);
	if(options != NULL)
		free(options);
	DisposeHandle(p->xPath);
	DisposeHandle(p->options);
	DisposeHandle(p->ns);
	DisposeHandle(p->fileNameStr);
		
	/* Shutdown libxml */
	xmlCleanupParser();
	
	return err;	
}
