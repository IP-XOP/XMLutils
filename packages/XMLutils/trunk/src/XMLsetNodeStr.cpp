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
static int
update_xpath_nodes(xmlNodeSetPtr nodes, const xmlChar* value) {
    int err = 0;
	int size;
    int i;
		
    size = (nodes) ? nodes->nodeNr : 0;
   
	if(strlen((char*)value)==0)
		value = (xmlChar*)("\r");
    /*
     * NOTE: the nodes are processed in reverse order, i.e. reverse document
     *       order because xmlNodeSetContent can actually free up descendant
     *       of the node and such nodes may have been selected too ! Handling
     *       in reverse order ensure that descendant are accessed first, before
     *       they get removed. Mixing XPath and modifications on a tree must be
     *       done carefully !
     */
    for(i = size - 1; i >= 0; i--) {
				xmlNodeSetContent(nodes->nodeTab[i], value);
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
	done:
	return err;
}

int
XMLsetNodeStr(XMLsetNodeStrStructPtr p){
	//the error code
	int err = 0;
	
	extern std::map<int,igorXMLfile> allXMLfiles;
	int fileID = -1;
	xmlXPathObjectPtr xpathObj = NULL; 
	xmlDocPtr doc = NULL;

	//Xpath handle,namespace handle,options handle
	char *xPath = NULL;
	char *ns    = NULL;
	char *content = NULL;
	//size of handles
	int sizexPath,sizens,sizecontent;
	
	/* check if any of the argument string handles are null */
	if(p->xPath == NULL || p->ns == NULL || p->content == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	/* work out the size of each of the argument string handles */
	sizexPath = GetHandleSize(p->xPath);
	sizens = GetHandleSize(p->ns);
	sizecontent = GetHandleSize(p->content);
	
	//allocate space for the C-strings.
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
	if (err = GetCStringFromHandle(p->xPath, xPath, sizexPath))
		goto done;
	if (err = GetCStringFromHandle(p->ns, ns, sizens))
		goto done;
	if (err = GetCStringFromHandle(p->content, content, sizecontent))
		goto done;

	fileID = (int)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = allXMLfiles[p->fileID].doc;
	}
	
	//execute Xpath expression
	//for some reason the xpathObj doesn't like being passed as a pointer argument, therefore return it as a result.
	xpathObj = execute_xpath_expression(doc, (xmlChar*) xPath, (xmlChar*) ns, &err); 
	if(err)
		goto done;
	
	if(err = update_xpath_nodes(xpathObj->nodesetval, (xmlChar*) content))
		goto done;
			
done:
	p->retval = err;
	if(xpathObj != NULL)
		xmlXPathFreeObject(xpathObj); 
	if(xPath != NULL)
		free(xPath);
	if(ns != NULL)
		free(ns);
	if(content != NULL)
		free(content);
	DisposeHandle(p->xPath);
	DisposeHandle(p->content);
	DisposeHandle(p->ns);
	
	return err;	
}