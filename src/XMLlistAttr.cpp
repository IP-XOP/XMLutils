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
print_attr(xmlDocPtr doc, xmlNodeSetPtr nodes) {
    int err = 0;
	int size;
    int i;
	xmlChar* xmloutputBuf = (xmlChar*)("");
	xmlChar* attrVal = NULL;
	xmlAttr* properties = NULL;

	/* Wave for output of content*/
	waveHndl outputWav;
	Handle transfer = NULL;
	char *waveName = "M_listAttr";
	long numDimensions;
	long dimensionSizes[MAX_DIMENSIONS+1];
	long indices[MAX_DIMENSIONS+1];
	memset(indices, 0, sizeof(indices));
	memset(dimensionSizes, 0, sizeof(dimensionSizes));
	if(err = MDMakeWave(&outputWav,waveName,NULL,dimensionSizes,TEXT_WAVE_TYPE,1))
		goto done;
		/* Handle to transfer the string to the wave */
	transfer = NewHandle(0);
	if(transfer == NULL){
		err = NOMEM; goto done;
	}
	
	size = (nodes) ? nodes->nodeNr : 0;
    	
	for(i = 0; i < size; ++i) {
		
		for(properties = nodes->nodeTab[i]->properties ; properties != NULL ; properties = properties->next){
			if(err = MDGetWaveDimensions(outputWav,&numDimensions,dimensionSizes))
				goto done;
			indices[1] = 0;
			dimensionSizes[0] = dimensionSizes[0]+1;
			indices[0] = dimensionSizes[0]-1;
			dimensionSizes[1] = 3;
			if(err = MDChangeWave(outputWav,-1,dimensionSizes))
				goto done;
			xmloutputBuf = xmlGetNodePath(nodes->nodeTab[i]);
			if(err = PutCStringInHandle((char*)xmloutputBuf,transfer))
				goto done;
			if(err = MDSetTextWavePointValue(outputWav,indices,transfer))
				goto done;
			
			indices[1] = 1;
			xmloutputBuf = (xmlChar*) properties->name ;
			if(err = PutCStringInHandle((char*)xmloutputBuf,transfer))
				goto done;
			if(err = MDSetTextWavePointValue(outputWav,indices,transfer))
				goto done;
				
			indices[1] = 2;
			attrVal = xmlGetProp(nodes->nodeTab[i], properties->name);
			if(err = PutCStringInHandle((char*)attrVal,transfer))
				goto done;
			if(err = MDSetTextWavePointValue(outputWav,indices,transfer))
				goto done;
	
		}
    }
	
	
done:
if(xmloutputBuf != NULL)
	xmlFree(xmloutputBuf);
if (transfer != NULL)
	DisposeHandle(transfer);
if (attrVal != NULL)
	xmlFree(attrVal);
	return err;
}

int
XMLlistAttr(XMLlistAttrStruct *p){
	//the error code
	int err = 0;
	
	xmlXPathObject *xpathObj = NULL; 
	xmlDoc *doc = NULL;
	
	//the filename handle, Xpath handle,namespace handle,options handle
	char *fileName = NULL;
	char *xPath = NULL;
	char *ns    = NULL;
	//size of handles
	int sizefileName,sizexPath,sizens;
	
	//filename will be passed as a native path
	char nativePath[MAX_PATH_LEN+1];	
			
	if(p->fileNameStr == NULL || p->xPath == NULL || p->ns == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	sizefileName = GetHandleSize(p->fileNameStr);
	sizexPath = GetHandleSize(p->xPath);
	sizens = GetHandleSize(p->ns);
	
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
	
	/* get all of the igor input strings into C-strings */
	if (err = GetCStringFromHandle(p->fileNameStr, fileName, sizefileName))
		goto done;
	if (err = GetCStringFromHandle(p->xPath, xPath, sizexPath))
		goto done;
	if (err = GetCStringFromHandle(p->ns, ns, sizens))
		goto done;

	//get native filesystem filepath
	if (err = GetNativePath(fileName,nativePath))
		goto done;
    
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
	print_attr(doc, xpathObj->nodesetval);
	
	p->returnval = err;
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
	DisposeHandle(p->xPath);
	DisposeHandle(p->ns);
	DisposeHandle(p->fileNameStr);
		
	/* Shutdown libxml */
	xmlCleanupParser();
	
	return err;	
}
