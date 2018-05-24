
#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "XMLutils.h"
#include <string>
using namespace std;

int 
set_attr(xmlDocPtr doc, xmlNodeSetPtr nodes, char* attribute, char* val) {
    int err = 0;
	int size;
    int i;
	xmlAttr  *cur = NULL;
	xmlChar *entityEncoded = NULL;

	entityEncoded = xmlEncodeEntitiesReentrant(doc, BAD_CAST val);
	
	size = (nodes) ? nodes->nodeNr : 0;
	
	for(i = 0; i < size; ++i) {
		cur = xmlSetProp(nodes->nodeTab[i], (xmlChar*) attribute, (xmlChar*) entityEncoded);
	}

	if(entityEncoded != NULL)
		xmlFree(entityEncoded);
		
	return err;
}

int
XMLsetAttr(XMLsetAttrStruct *p){
	//the error code
	int err = 0;
	
	extern std::map<long,igorXMLfile> allXMLfiles;
	long fileID = -1;
	xmlXPathObject *xpathObj = NULL; 
	xmlDoc *doc = NULL;
	
	//Xpath handle,namespace handle,options handle
	string xPath, ns, attribute, value;
	
	if(p->xPath == NULL || p->ns == NULL || p->attribute == NULL || p->val == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	xPath.append(*p->xPath, WMGetHandleSize(p->xPath));
	ns.append(*p->ns, WMGetHandleSize(p->ns));
	attribute.append(*p->attribute, WMGetHandleSize(p->attribute));
	value.append(*p->val, WMGetHandleSize(p->val));
	
	//check if the node name is invalid
	if(xmlValidateName(BAD_CAST attribute.c_str() , 0) != 0){
		err = INVALID_NODE_NAME;
		goto done;
	}
	
	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLsetAttr: fileID isn't valid\r");
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[fileID].doc);
	}
	
	//execute Xpath expression
	xpathObj = execute_xpath_expression(doc, BAD_CAST xPath.c_str(), BAD_CAST ns.c_str(), &err);
	if(err)
		goto done;
	
	if(err = set_attr(doc, xpathObj->nodesetval, (char*) attribute.c_str(), (char*) value.c_str()))
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
	if(p->xPath != NULL)
		WMDisposeHandle(p->xPath);
	if(p->ns != NULL)	
		WMDisposeHandle(p->ns);
	if(p->attribute != NULL)	
		WMDisposeHandle(p->attribute);
	if(p->val != NULL)
		WMDisposeHandle(p->val);
		
	return err;	
}



