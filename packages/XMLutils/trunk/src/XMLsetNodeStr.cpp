/*
 *  XMLsetNodeStr.cpp
 *  XMLutils
 *
 *  Created by andrew on 12/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLutils.h"

/**
 * update_xpath_nodes:
 * @nodes:		the nodes set.
 * @value:		the new value for the node(s)
 *
 * Please note that nodes->nodeTab[i]->children is set, as this is a normally a text node.  If you set nodes->nodeTab[i] this will wipe out all child nodes
 * of that element.
 *
 */
static void
update_xpath_nodes(xmlNodeSetPtr nodes, const xmlChar* value) {
    int size;
    int i;
	xmlNodePtr nextTextNode = NULL;
	int changed = 0;
	
    size = (nodes) ? nodes->nodeNr : 0;
   
	if(strlen((char*)value)==0)
		value = (xmlChar*)("\n");
    /*
     * NOTE: the nodes are processed in reverse order, i.e. reverse document
     *       order because xmlNodeSetContent can actually free up descendant
     *       of the node and such nodes may have been selected too ! Handling
     *       in reverse order ensure that descendant are accessed first, before
     *       they get removed. Mixing XPath and modifications on a tree must be
     *       done carefully !
     */
    for(i = size - 1; i >= 0; i--) {
	
	//nodes->nodeTab[i] = xmlNewTextChild(nodes->nodeTab[i],NULL,nodes->nodeTab[i]->name ,value);

		nextTextNode = nodes->nodeTab[i]->children;
		
		while(nextTextNode != NULL){
			if(nextTextNode->type == XML_TEXT_NODE && changed == 0){
				xmlNodeSetContent(nextTextNode, value);
				changed = 1;
			} else if(nextTextNode->type == XML_TEXT_NODE && changed == 1) {
				xmlNodeSetContent(nextTextNode,(xmlChar*)("\n"));
			}
			nextTextNode = nextTextNode->next;
		}

	/*
	 * All the elements returned by an XPath query are pointers to
	 * elements from the tree *except* namespace nodes where the XPath
	 * semantic is different from the implementation in libxml2 tree.
	 * As a result when a returned node set is freed when
	 * xmlXPathFreeObject() is called, that routine must check the
	 * element type. But node from the returned set may have been removed
	 * by xmlNodeSetContent() resulting in access to freed data.
	 * This can be exercised by running
	 *       valgrind xpath2 test3.xml '//discarded' discarded
	 * There is 2 ways around it:
	 *   - make a copy of the pointers to the nodes from the result set 
	 *     then call xmlXPathFreeObject() and then modify the nodes
	 * or
	 *   - remove the reference to the modified nodes from the node set
	 *     as they are processed, if they are not namespace nodes.
	 */
	if (nodes->nodeTab[i]->type != XML_NAMESPACE_DECL)
	    nodes->nodeTab[i] = NULL;
    }
}

int
XMLsetNodeStr(XMLsetNodeStrStructPtr p){
	//the error code
	int err = 0;
	
	xmlXPathObjectPtr xpathObj = NULL; 
	xmlDocPtr doc = NULL;

	//the filename handle, Xpath handle,namespace handle,options handle
	char *fileName = NULL;
	char *xPath = NULL;
	char *ns    = NULL;
	char *content = NULL;
	//size of handles
	int sizefileName,sizexPath,sizens,sizecontent;
	
	//filename will be passed as a native path
	char nativePath[MAX_PATH_LEN+1];	
	
	/* check if any of the argument string handles are null */
	if(p->fileNameStr == NULL || p->xPath == NULL || p->ns == NULL || p->content == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	/* work out the size of each of the argument string handles */
	sizefileName = GetHandleSize(p->fileNameStr);
	sizexPath = GetHandleSize(p->xPath);
	sizens = GetHandleSize(p->ns);
	sizecontent = GetHandleSize(p->content);
	
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
	content = (char*)malloc(sizecontent*sizeof(char)+1);
	if(content == NULL){
		err = NOMEM;goto done;
	}

	/* get all of the igor input strings into C-strings */
	if (err = GetCStringFromHandle(p->fileNameStr, fileName, sizefileName))
		goto done;
	if (err = GetCStringFromHandle(p->xPath, xPath, sizexPath))
		goto done;
	if (err = GetCStringFromHandle(p->ns, ns, sizens))
		goto done;
	if (err = GetCStringFromHandle(p->content, content, sizecontent))
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
	//for some reason the xpathObj doesn't like being passed as a pointer argument, therefore return it as a result.
	xpathObj = execute_xpath_expression(doc, (xmlChar*) xPath, (xmlChar*) ns, &err); 
	if(err)
		goto done;
	
	update_xpath_nodes(xpathObj->nodesetval, (xmlChar*) content);
	
	if(xmlSaveFile(fileName,doc) == -1){
		err = XML_COULDNT_SAVE;
		goto done;
	}
		
done:
	p->retval = err;
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
	if(content != NULL)
		free(content);
	DisposeHandle(p->xPath);
	DisposeHandle(p->content);
	DisposeHandle(p->ns);
	DisposeHandle(p->fileNameStr);
	/* Shutdown libxml */
	xmlCleanupParser();
	
	return err;	
}