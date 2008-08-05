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
	xmlChar *xmloutputBuf = NULL;
	xmlChar *nothing = (xmlChar*)("");
	char *space = " ";
	
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


		if(xmloutputBuf != NULL){
			bufsize = strlen((char*)xmloutputBuf);
			
			if(err = PtrAndHand(xmloutputBuf,output,bufsize))
				goto done;
			bufsize = strlen(space);			
			if(err = PtrAndHand(space,output,bufsize))
				goto done;	
		
		}
		
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
	
	extern std::map<long,igorXMLfile> allXMLfiles;
	long fileID = -1;
	xmlXPathObject *xpathObj = NULL; 
	xmlDoc *doc = NULL;

	//the filename handle, Xpath handle,namespace handle,options handle
	char *xPath = NULL;
	char *ns    = NULL;
	char *options = NULL;
	//size of handles
	int sizexPath,sizens,sizeoptions;
	
	if(p->xPath == NULL || p->ns == NULL || p->options == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	sizexPath = GetHandleSize(p->xPath);
	sizens = GetHandleSize(p->ns);
	sizeoptions = GetHandleSize(p->options);
	
	//allocate space for the C-strings.
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
	if (err = GetCStringFromHandle(p->xPath, xPath, sizexPath))
		goto done;
	if (err = GetCStringFromHandle(p->ns, ns, sizens))
		goto done;
	if (err = GetCStringFromHandle(p->options, options, sizeoptions))
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
		doc = (allXMLfiles[p->fileID].doc);
	}
	
	//execute Xpath expression
	xpathObj = execute_xpath_expression(doc, BAD_CAST xPath, BAD_CAST ns, &err);
	if(err)
		goto done;
	//and print it out to a handle
	err = print_xpath_nodes((allXMLfiles[p->fileID].doc), xpathObj->nodesetval, output);

	
	p->returnString = output;
done:
	if(err == FILEID_DOESNT_EXIST ||
		err == XPATH_CONTEXT_CREATION_ERROR ||
		 err == FAILED_TO_REGISTER_NAMESPACE ||
		  err == XPATH_COMPILE_ERROR ||
		   err == UNABLE_TO_EVAL_XPATH_EXPR){
		err = 0;
	}
	if(xpathObj != NULL)
		xmlXPathFreeObject(xpathObj); 
	if(xPath != NULL)
		free(xPath);
	if(ns != NULL)
		free(ns);
	if(options != NULL)
		free(options);
	if(p->xPath != NULL)
		DisposeHandle(p->xPath);
	if(p->options != NULL)
		DisposeHandle(p->options);
	if(p->ns)
		DisposeHandle(p->ns);
		
	return err;	
}
