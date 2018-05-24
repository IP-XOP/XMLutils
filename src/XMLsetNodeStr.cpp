/*
 *  XMLsetNodeStr.cpp
 *  XMLutils
 *
 *  Created by andrew on 12/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLutils.h"
using namespace std;
#include <string>
using namespace std;

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
	xmlChar *encValue = NULL;
	
	if(nodes && nodes->nodeTab && nodes->nodeTab[0]!= NULL){
		encValue = xmlEncodeEntitiesReentrant(nodes->nodeTab[0]->doc, value);
	}
    size = (nodes) ? nodes->nodeNr : 0;
   
//	if(strlen((char*)value)==0)
//		value = (xmlChar*)("");
    /*
     * NOTE: the nodes are processed in reverse order, i.e. reverse document
     *       order because xmlNodeSetContent can actually free up descendant
     *       of the node and such nodes may have been selected too ! Handling
     *       in reverse order ensure that descendant are accessed first, before
     *       they get removed. Mixing XPath and modifications on a tree must be
     *       done carefully !
     */
    for(i = size - 1; i >= 0; i--) {
				xmlNodeSetContent(nodes->nodeTab[i], encValue);
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
	
	if(encValue != NULL)
		xmlFree(encValue);

	return err;
}

int
XMLsetNodeStr(XMLsetNodeStrStructPtr p){
	//the error code
	int err = 0;
	
	extern std::map<long,igorXMLfile> allXMLfiles;
	long fileID = -1;
	xmlXPathObjectPtr xpathObj = NULL; 
	xmlDocPtr doc = NULL;

	//Xpath handle,namespace handle,options handle
	string xPath, ns, content;
	
	/* check if any of the argument string handles are null */
	if(p->xPath == NULL || p->ns == NULL || p->content == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	xPath.append(*p->xPath, WMGetHandleSize(p->xPath));
	ns.append(*p->ns, WMGetHandleSize(p->ns));
	content.append(*p->content, WMGetHandleSize(p->content));
	
	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLsetNodeStr: fileID doesn't exist\r");
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[fileID].doc);
	}
	
	//execute Xpath expression
	//for some reason the xpathObj doesn't like being passed as a pointer argument, therefore return it as a result.
	xpathObj = execute_xpath_expression(doc, (xmlChar*) xPath.c_str(), (xmlChar*) ns.c_str(), &err); 
	if(err)
		goto done;
	
	if(err = update_xpath_nodes(xpathObj->nodesetval, (xmlChar*) content.c_str()))
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
	WMDisposeHandle(p->xPath);
	WMDisposeHandle(p->content);
	WMDisposeHandle(p->ns);
	
	return err;	
}
