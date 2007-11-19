/*
 *  XMLevaluateXPathExpression.cpp
 *  XMLutils
 *
 *  Created by andrew on 9/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#include "XMLutils.h"
/**
 * register_namespaces:
 * @xpathCtx:		the pointer to an XPath context.
 * @nsList:		the list of known namespaces in 
 *			"<prefix1>=<href1> <prefix2>=href2> ..." format.
 *
 * Registers namespaces from @nsList in @xpathCtx.
 *
 * Returns 0 on success and a negative value otherwise.
 */
int 
register_namespaces(xmlXPathContextPtr xpathCtx, const xmlChar* nsList) {
    int err = 0;
	xmlChar* nsListDup = NULL;
    xmlChar* prefix = NULL;
    xmlChar* href = NULL;
    xmlChar* next = NULL;
	
/*example
	xmlChar *prefix = (xmlChar*)("xrdml");
	xmlChar *ns_uri = (xmlChar*)("http://www.xrdml.com/XRDMeasurement/1.0");
	if( err = xmlXPathRegisterNs(xpathCtx,prefix, ns_uri));*/
   


    nsListDup = xmlStrdup(nsList);
    if(nsListDup == NULL) {
		err = FAILED_TO_REGISTER_NAMESPACE;
		goto done;	
    }
    
    next = nsListDup; 
    while(next != NULL) {
	// skip spaces
	while((*next) == ' ') next++;
	if((*next) == '\0') break;

	//find prefix
	prefix = next;
	next = (xmlChar*) xmlStrchr(next, '=');
	if(next == NULL) {
	    err = FAILED_TO_REGISTER_NAMESPACE;
		goto done;
	}
	*(next++) = '\0';	
	
	// find href 
	href = next;
	next = (xmlChar*)xmlStrchr(next, ' ');
	if(next != NULL) {
	    *(next++) = '\0';	
	}

	// do register namespace
	if(xmlXPathRegisterNs(xpathCtx, prefix, href) != 0) {
	   err = FAILED_TO_REGISTER_NAMESPACE;
	}
    }
	
done:
	if(nsListDup != NULL)
		xmlFree(nsListDup);
    return err;
}


/**
 * execute_xpath_expression:
 * @doc:			the parsed XML document.
 * @xpathExpr:		the xpath expression for evaluation.
 * @nsList:		the optional list of known namespaces in 
 *			"<prefix1>=<href1> <prefix2>=href2> ..." format.
 * @err:			the error code from this function.  Error code is 0 if successful.
 *
 * The XPath expression is first compiled, which should pick up a lot of erroneous expressions
 * The compiled XPath expression is then evaluated, with the XPath object that is created by that
 * evaluation being returned to the calling function.
 *
 * Returns xmlXPathObject*, which must be free'd using xmlXPathFreeObject() by the calling function.  This XPath object
 * contains the nodeSet that matches the expression.
 *
 */

xmlXPathObject*
execute_xpath_expression(xmlDoc *doc, xmlChar* xpathExpr, xmlChar* nsList, int *err) {
	
	xmlXPathObject *xpathObj = NULL;
    xmlXPathContext *context = NULL; 
	xmlXPathCompExpr *comp = NULL;
	
    /*
	Create xpath evaluation context
	*/
    context = xmlXPathNewContext(doc);
    if(context == NULL) {
       *err = XPATH_CONTEXT_CREATION_ERROR;
	   goto done;
    }
	
    /* Register namespaces from list (if any) */
    if((nsList != NULL) && (register_namespaces(context, nsList) < 0)) {
		*err = FAILED_TO_REGISTER_NAMESPACE;
		goto done;
    }

	/*
	 compile xPath expression
	by compiling we cutout any problems with weird expressions, hopefully!
	*/
	comp = xmlXPathCtxtCompile(context,xpathExpr);
    if(comp == NULL){
		*err = XPATH_COMPILE_ERROR;
		goto done;
	}

    /* Evaluate xpath expression */	
	xpathObj = xmlXPathCompiledEval(comp, context);
    if(xpathObj == NULL) {
        *err = UNABLE_TO_EVAL_XPATH_EXPR;
		goto done;
    }

done:
    /* Cleanup */
	if(comp != NULL)
		xmlXPathFreeCompExpr(comp);
    if(context != NULL)
		xmlXPathFreeContext(context);
    return xpathObj;
}
