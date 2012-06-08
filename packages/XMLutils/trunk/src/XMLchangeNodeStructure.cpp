/*
 *  untitled.cpp
 *  XMLutils
 *
 *  Created by andrew on 18/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLutils.h"
#include <string>

#include "UTF8_multibyte_conv.h"

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

int add_nodes(xmlXPathObject *xpathObj, const xmlChar* nodeName, const xmlChar* ns, int nodeType, const xmlChar* content){
	int err = 0;
	xmlNode *cur_node = NULL;
	xmlNode *added_node = NULL;
	int numNodes,j,curtype;
	
	xmlChar *encContent = NULL;
	xmlIndentTreeOutput = 1;
	
	//the content may have non-allowed XML characters, such as < or >, encode them
	if(xpathObj->nodesetval && xpathObj->nodesetval->nodeTab && xpathObj->nodesetval->nodeTab[0] != NULL){
		encContent = xmlEncodeEntitiesReentrant(xpathObj->nodesetval->nodeTab[0]->doc, content);
	}
	
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
						added_node = xmlNewChild(cur_node,NULL,nodeName,encContent);
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
				{
					added_node = xmlNewText(encContent);
					added_node = xmlAddChild(cur_node,added_node);
				}
				break;
			case XML_CDATA_SECTION_NODE:
				break;
			case XML_ENTITY_REF_NODE:
				break;
			case XML_ENTITY_NODE:
			case XML_PI_NODE:
				break;
			case XML_COMMENT_NODE :
				if(curtype == 1 ||
				   curtype == 3 ||
				   curtype == 9 ||
				   curtype == 11 ||
				   curtype == 5 ||
				   curtype == 6)
				{
					added_node = xmlNewComment(content);
					added_node = xmlAddChild(cur_node,added_node);
				}
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
done:
	if(encContent != NULL)
		xmlFree(encContent);
	
	return err;
}

int XMLaddNode(XMLaddNodeStruct *p){
	//the error code
	int err = 0;
	
	extern std::map<long, igorXMLfile> allXMLfiles;
	xmlXPathObjectPtr xpathObj = NULL; 
	xmlDocPtr doc = NULL;
	
	//the fileID, Xpath handle,namespace handle,options handle
	long fileID = -1;
	string xPath, ns, nodeName, content;
	
	int nodeType = -1;
	
	/* check if any of the argument string handles are null */
	if(p->xPath == NULL || p->ns == NULL || p->nodeName == NULL || p->content == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	xPath.append(*p->xPath, GetHandleSize(p->xPath));
	ns.append(*p->ns, GetHandleSize(p->ns));
	nodeName.append(*p->nodeName, GetHandleSize(p->nodeName));
	content.append(*p->content, GetHandleSize(p->content));
	
	if(err = SystemEncodingToUTF8(xPath))
		goto done;
	if(err = SystemEncodingToUTF8(ns))
		goto done;
	if(err = SystemEncodingToUTF8(nodeName))
		goto done;
	if(err = SystemEncodingToUTF8(content))
		goto done;
	
	//check if the node name is invalid
	if(xmlValidateName(BAD_CAST nodeName.c_str() , 0) != 0){
		err = INVALID_NODE_NAME;
		goto done;
	}
	
	nodeType = (int)roundf(p->nodeType);
	
	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLaddNode: fileID doesn't exist\r");
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
	
	if(err = add_nodes(xpathObj, BAD_CAST nodeName.c_str(), BAD_CAST ns.c_str(), nodeType, BAD_CAST content.c_str()))
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
	
	extern std::map<long,igorXMLfile> allXMLfiles;
	xmlXPathObjectPtr xpathObj = NULL; 
	xmlDocPtr doc = NULL;
	
	string xPath, ns;
	
	//the fileID, Xpath handle,namespace handle,options handle
	long fileID = -1;
	
	/* check if any of the argument string handles are null */
	if(p->xPath == NULL || p->ns == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	xPath.append(*p->xPath, GetHandleSize(p->xPath));
	ns.append(*p->ns, GetHandleSize(p->ns));
	
	if(err = SystemEncodingToUTF8(xPath))
		goto done;
	if(err = SystemEncodingToUTF8(ns))
		goto done;
	
	
	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLdelNode: fileID isn't valid\r");
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[(long) p->fileID].doc);
	}
	
	//execute Xpath expression
	//for some reason the xpathObj doesn't like being passed as a pointer argument, therefore return it as a result.
	xpathObj = execute_xpath_expression(doc, (xmlChar*) xPath.c_str(), (xmlChar*) ns.c_str(), &err); 
	if(err)
		goto done;
	
	if(err = delete_nodes(xpathObj))
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
	
	DisposeHandle(p->xPath);
	DisposeHandle(p->ns);
	
	return err;
	
}



