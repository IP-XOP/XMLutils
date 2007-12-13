/*
 *  untitled.cpp
 *  XMLutils
 *
 *  Created by andrew on 18/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLutils.h"
/*
XML_ELEMENT_NODE = 1
XML_ATTRIBUTE_NODE = 2
XML_TEXT_NODE = 3
XML_CDATA_SECTION_NODE = 4
XML_ENTITY_REF_NODE = 5
XML_ENTITY_NODE = 6
XML_PI_NODE = 7
XML_COMMENT_NODE = 8
XML_DOCUMENT_NODE = 9
XML_DOCUMENT_TYPE_NODE = 10
XML_DOCUMENT_FRAG_NODE = 11
XML_NOTATION_NODE = 12
XML_HTML_DOCUMENT_NODE = 13
XML_DTD_NODE = 14
XML_ELEMENT_DECL = 15
XML_ATTRIBUTE_DECL = 16
XML_ENTITY_DECL = 17
XML_NAMESPACE_DECL = 18
XML_XINCLUDE_START = 19
XML_XINCLUDE_END = 20
XML_DOCB_DOCUMENT_NODE = 21
*/

int add_nodes(xmlXPathObject *xpathObj,xmlChar* nodeName, xmlChar* ns, int nodeType, xmlChar* content){
int err = 0;
xmlNode *cur_node = NULL;
xmlNode *added_node = NULL;
int numNodes,j,curtype;

xmlIndentTreeOutput = 1;

/* work out how many nodes in the nodeset from the Xpath object */
numNodes = (xpathObj->nodesetval) ? xpathObj->nodesetval->nodeNr : 0;

for(j = 0; j < numNodes; ++j) {
	cur_node = xpathObj->nodesetval->nodeTab[j];
	curtype = cur_node->type;
	switch(nodeType){
		case XML_ELEMENT_NODE:
			if(curtype==9 ||
				curtype==11 ||
				 curtype==5 ||
				  curtype==1 ||
				   curtype==6)
				if(xmlStrlen(content)==0){///in both cases put in default namespace
					added_node = xmlNewChild(cur_node,NULL,nodeName,NULL);			
				} else { 
					added_node = xmlNewChild(cur_node,NULL,nodeName,content);
				}
			break;
		case XML_ATTRIBUTE_NODE:
			break;
		case XML_TEXT_NODE:
			if(curtype==11 ||
				curtype==5 ||
				 curtype==1 ||
				  curtype==6 ||
				   curtype==2)
//				if(xmlStrlen(content)==0){
//					added_node = xmlNewChild(cur_node,NULL,nodeName,NULL);			
//				} else {
					added_node = xmlNewText(content);
					added_node = xmlAddChild(cur_node,added_node);
//					added_node = xmlNewChild(cur_node,NULL,nodeName,content);
//				}
			break;
		case XML_CDATA_SECTION_NODE:
			break;
		case XML_ENTITY_REF_NODE:
			break;
		case XML_ENTITY_NODE:
		case XML_PI_NODE:
			break;
		case XML_COMMENT_NODE :
			
			break;
		case XML_DOCUMENT_NODE :
			
			break;
		case XML_DOCUMENT_TYPE_NODE :
			
			break;
		case XML_DOCUMENT_FRAG_NODE:
			
			break;
		case XML_NOTATION_NODE :
			
			break;
		case XML_HTML_DOCUMENT_NODE:
			
			break;
		case XML_DTD_NODE:
			
			break;
		case XML_ELEMENT_DECL:
			
			break;
		case XML_ATTRIBUTE_DECL :
			
			break;
		case XML_ENTITY_DECL:
			break;
		case XML_NAMESPACE_DECL :
			break;
		case XML_XINCLUDE_START:
			break;
		case XML_XINCLUDE_END:
			break;
		case XML_DOCB_DOCUMENT_NODE:
			break;
	
	}
}

return err;
}

int XMLaddNode(XMLaddNodeStruct *p){
//the error code
	int err = 0;
	
	extern std::map<int,igorXMLfile> allXMLfiles;
	xmlXPathObjectPtr xpathObj = NULL; 
	xmlDocPtr doc = NULL;

	//the fileID, Xpath handle,namespace handle,options handle
	int fileID = -1;
	char *xPath = NULL;
	char *ns    = NULL;
	char *nodeName = NULL;
	char *content = NULL;
	int nodeType = -1;
	//size of handles
	int sizexPath,sizens,sizenodeName,sizecontent;
	
	/* check if any of the argument string handles are null */
	if(p->xPath == NULL || p->ns == NULL || p->nodeName == NULL || p->content == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	/* work out the size of each of the argument string handles */
	sizexPath = GetHandleSize(p->xPath);
	sizens = GetHandleSize(p->ns);
	sizenodeName = GetHandleSize(p->nodeName);
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
	nodeName = (char*)malloc(sizenodeName*sizeof(char)+1);
	if(nodeName == NULL){
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
	if (err = GetCStringFromHandle(p->nodeName, nodeName, sizenodeName))
		goto done;
	if (err = GetCStringFromHandle(p->content, content, sizecontent))
		goto done;
		
	nodeType = (int)roundf(p->nodeType);
	
	fileID = (int)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[p->fileID].doc);
	}
 
	//execute Xpath expression
	//for some reason the xpathObj doesn't like being passed as a pointer argument, therefore return it as a result.
	xpathObj = execute_xpath_expression(doc, (xmlChar*) xPath, (xmlChar*) ns, &err); 
	if(err)
		goto done;
	
	if(err = add_nodes(xpathObj,BAD_CAST nodeName, BAD_CAST ns, nodeType, BAD_CAST content))
		goto done;
		
done:
	p->retval = err;
	if(xpathObj != NULL)
		xmlXPathFreeObject(xpathObj); 
	if(xPath != NULL)
		free(xPath);
	if(ns != NULL)
		free(ns);
	if(nodeName != NULL)
		free(nodeName);
	if(content != NULL)
		free(content);	
	DisposeHandle(p->xPath);
	DisposeHandle(p->nodeName);
	DisposeHandle(p->ns);
	DisposeHandle(p->content);

return err;
}



int delete_nodes(xmlXPathObject *xpathObj){
int err = 0;
xmlNode *cur_node = NULL;
int numNodes,j;

/* work out how many nodes in the nodeset from the Xpath object */
numNodes = (xpathObj->nodesetval) ? xpathObj->nodesetval->nodeNr : 0;

for(j = 0; j < numNodes; ++j) {
	cur_node = xpathObj->nodesetval->nodeTab[j];
	xmlUnlinkNode(cur_node);
	xmlFreeNode(cur_node);
}

return err;
}

int XMLdelNode(XMLdelNodeStruct *p){
//the error code
	int err = 0;
	
	extern std::map<int,igorXMLfile> allXMLfiles;
	xmlXPathObjectPtr xpathObj = NULL; 
	xmlDocPtr doc = NULL;

	//the fileID, Xpath handle,namespace handle,options handle
	int fileID = -1;
	char *xPath = NULL;
	char *ns    = NULL;
	//size of handles
	int sizexPath,sizens;
	
	/* check if any of the argument string handles are null */
	if(p->xPath == NULL || p->ns == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	/* work out the size of each of the argument string handles */
	sizexPath = GetHandleSize(p->xPath);
	sizens = GetHandleSize(p->ns);
		
	//allocate space for the C-strings.
	xPath = (char*)malloc(sizexPath*sizeof(char)+1);
	if(xPath == NULL){
		err = NOMEM;goto done;
	}
	ns = (char*)malloc(sizens*sizeof(char)+1);
	if(ns == NULL){
		err = NOMEM;goto done;
	}
	
	/* get all of the igor input strings into C-strings */
	if (err = GetCStringFromHandle(p->xPath, xPath, sizexPath))
		goto done;
	if (err = GetCStringFromHandle(p->ns, ns, sizens))
		goto done;
	
	fileID = (int)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[p->fileID].doc);
	}
 
	//execute Xpath expression
	//for some reason the xpathObj doesn't like being passed as a pointer argument, therefore return it as a result.
	xpathObj = execute_xpath_expression(doc, (xmlChar*) xPath, (xmlChar*) ns, &err); 
	if(err)
		goto done;
	
	if(err = delete_nodes(xpathObj))
		goto done;
		
done:
	p->retval = err;
	if(xpathObj != NULL)
		xmlXPathFreeObject(xpathObj); 
	if(xPath != NULL)
		free(xPath);
	if(ns != NULL)
		free(ns);
	DisposeHandle(p->xPath);
	DisposeHandle(p->ns);

return err;

}



