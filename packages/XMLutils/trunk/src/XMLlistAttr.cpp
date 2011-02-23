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
print_attr(xmlDocPtr doc, xmlNodeSetPtr nodes) {
    int err = 0;
	int size;
    int i;
	xmlChar* xmlNodePath = NULL;
	xmlChar* attrVal = NULL;
	xmlAttr* properties = NULL;
	MemoryStruct data;

	/* Wave for output of content*/
	waveHndl outputWav;
	Handle transfer = NULL;
	char *waveName = "M_listAttr";
	long numDimensions;
	long dimensionSizes[MAX_DIMENSIONS + 1];
	long indices[MAX_DIMENSIONS + 1];
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
			if(err = MDGetWaveDimensions(outputWav, &numDimensions, dimensionSizes))
				goto done;
			indices[1] = 0;
			dimensionSizes[0] = dimensionSizes[0] + 1;
			indices[0] = dimensionSizes[0] - 1;
			dimensionSizes[1] = 3;
			if(err = MDChangeWave(outputWav, -1, dimensionSizes))
				goto done;

			xmlNodePath = NULL;
			xmlNodePath = xmlGetNodePath(nodes->nodeTab[i]);
			
			if(data.reset(xmlNodePath, sizeof(xmlChar), xmlStrlen(xmlNodePath)) == -1){
				err = NOMEM;
				goto done;
			}
			if(err = UTF8toSystemEncoding(&data))
				goto done;
			
			if(xmlNodePath){
				xmlFree(xmlNodePath);
				xmlNodePath = NULL;
			}
			
			if(err = PutCStringInHandle((char*)data.getData(), transfer))
				goto done;
			if(err = MDSetTextWavePointValue(outputWav, indices, transfer))
				goto done;
			
			indices[1] = 1;
			if(data.reset((void*) properties->name, sizeof(xmlChar), xmlStrlen(properties->name)) == -1){
				err = NOMEM;
				goto done;
			}
			if(err = UTF8toSystemEncoding(&data))
				goto done;

			if(err = PutCStringInHandle((char*)data.getData(), transfer))
				goto done;
			if(err = MDSetTextWavePointValue(outputWav,indices, transfer))
				goto done;
				
			indices[1] = 2;
			attrVal = NULL;
			attrVal = xmlGetProp(nodes->nodeTab[i], properties->name);

			if(data.reset(attrVal, sizeof(xmlChar), xmlStrlen(attrVal)) == -1){
				err = NOMEM;
				goto done;
			}
			if(err = UTF8toSystemEncoding(&data))
				goto done;
			
			if(attrVal){
				xmlFree(attrVal);
				attrVal = NULL;
			}
				
			if(err = PutCStringInHandle((char*)data.getData(), transfer))
				goto done;
			if(err = MDSetTextWavePointValue(outputWav, indices, transfer))
				goto done;
		}
    }
	
done:

if(xmlNodePath != NULL)
	xmlFree(xmlNodePath);
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

	extern std::map<long,igorXMLfile> allXMLfiles;
	xmlXPathObject *xpathObj = NULL; 
	xmlDoc *doc = NULL;
	long fileID = -1;
	
	//the filename handle, Xpath handle,namespace handle,options handle
	MemoryStruct xPath, ns;
				
	if(p->xPath == NULL || p->ns == NULL){
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

	if(err = SystemEncodingToUTF8(&xPath))
		goto done;
	if(err = SystemEncodingToUTF8(&ns))
	   goto done;	
	
	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLlistAttr: fileID isn't valid\r");
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[fileID].doc);
	}
	
	//execute Xpath expression
	xpathObj = execute_xpath_expression(doc, BAD_CAST xPath.getData(), BAD_CAST ns.getData(), &err);
	if(err)
		goto done;
	//and print it out to a handle
	err = print_attr(doc, xpathObj->nodesetval);
	
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

	if(p->xPath)
		DisposeHandle(p->xPath);
	if(p->ns)
		DisposeHandle(p->ns);
	
	return err;	
}

