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
print_xpath_nodes(xmlDocPtr doc, xmlNodeSetPtr nodes, Handle output) {
    int err = 0;
	int size;
    int i;
	xmlChar *xmloutputBuf = NULL;
	char *space = " ";
	MemoryStruct data;
	
	size = (nodes) ? nodes->nodeNr : 0;
    
//	if(size == 0){
//		if(err = PtrAndHand(nothing,output,sizeof(nothing)))
//			goto done;
//	}
	
	for(i = 0; i < size; ++i) {
		switch(nodes->nodeTab[i]->type){
			case(XML_ATTRIBUTE_NODE):
				xmloutputBuf =xmlNodeGetContent(nodes->nodeTab[i]->children);
				break;
			case(XML_TEXT_NODE):
					xmloutputBuf = xmlNodeGetContent(nodes->nodeTab[i]);
				break;
			default:
				xmloutputBuf = xmlNodeGetContent(nodes->nodeTab[i]);//(doc, nodes->nodeTab[i], 1);
				break;
		}
		
		if(xmloutputBuf){
			if(i)
				if(data.append(space, sizeof(char))){
					err = NOMEM;
					goto done;
				}
			if(data.append(xmloutputBuf, sizeof(xmlChar), xmlStrlen(xmloutputBuf)) == -1){
				err = NOMEM;
				goto done;
			}
			xmlFree(xmloutputBuf);
			xmloutputBuf = NULL;
		}
	}
		
	if(err = UTF8toSystemEncoding(&data))
		goto done;
	
	if(err = PutCStringInHandle((char*) data.getData(), output))
		goto done;
	
done:
if(xmloutputBuf)
	xmlFree(xmloutputBuf);
	
	return err;
}

int
XMLstrFmXPath(XMLstrFmXpathStructPtr p){
	//the error code
	int err = 0;
	//the output Handle
	Handle output = NULL;
	
	extern std::map<long,igorXMLfile> allXMLfiles;
	long fileID = -1;
	xmlXPathObject *xpathObj = NULL; 
	xmlDoc *doc = NULL;

	//the filename handle, Xpath handle,namespace handle,options handle
	MemoryStruct xPath, ns, options;
	
	if(p->xPath == NULL || p->ns == NULL || p->options == NULL){
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
	if(options.append(*p->options, GetHandleSize(p->options)) == -1){
		err = NOMEM;
		goto done;
	}
	if(options.nullTerminate() == -1){
		err = NOMEM;
		goto done;
	}
	
	if(err = SystemEncodingToUTF8(&xPath))
		goto done;
	if(err = SystemEncodingToUTF8(&ns))
		goto done;
		
	//get a handle for the output
	output = NewHandle(0); 
	if (output == NULL) 
		return NOMEM; 
    p->returnString = output;

	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLstrfmXPath: fileID isn't valid\r");
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
	err = print_xpath_nodes((allXMLfiles[p->fileID].doc), xpathObj->nodesetval, output);

	
done:
	p->returnString = NULL;
	if(output)
		p->returnString = output;
	
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
	if(p->options != NULL)
		DisposeHandle(p->options);
	if(p->ns)
		DisposeHandle(p->ns);
		
	return err;	
}
