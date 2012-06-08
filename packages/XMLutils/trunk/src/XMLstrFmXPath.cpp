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

#include <string>
#include "UTF8_multibyte_conv.h"

int 
print_xpath_nodes(xmlDocPtr doc, xmlNodeSetPtr nodes, Handle output) {
    int err = 0;
	int size;
    int i;
	xmlChar *xmloutputBuf = NULL;
	const char *space = " ";
	string data;
	
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
				data.append(space);

			data.append((const char*) xmloutputBuf, sizeof(xmlChar) * xmlStrlen(xmloutputBuf));

			xmlFree(xmloutputBuf);
			xmloutputBuf = NULL;
		}
	}
		
	if(err = UTF8toSystemEncoding(data))
		goto done;
	
	if(err = PutCStringInHandle((char*) data.c_str(), output))
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
	string xPath, ns, options;
	
	if(p->xPath == NULL || p->ns == NULL || p->options == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	xPath.append(*p->xPath, GetHandleSize(p->xPath));
	ns.append(*p->ns, GetHandleSize(p->ns));
	options.append(*p->options, GetHandleSize(p->options));
	options.append("\0");
	
	if(err = SystemEncodingToUTF8(xPath))
		goto done;
	if(err = SystemEncodingToUTF8(ns))
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
	xpathObj = execute_xpath_expression(doc, BAD_CAST xPath.c_str(), BAD_CAST ns.c_str(), &err);
	if(err)
		goto done;
	//and print it out to a handle
	err = print_xpath_nodes((allXMLfiles[(long)p->fileID].doc), xpathObj->nodesetval, output);

	
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
