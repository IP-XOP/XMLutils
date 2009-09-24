/*
 *  XMLparsedWaveFromXPath.cpp
 *  XMLutils
 *
 *  Created by andrew on 7/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

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


static int
outputXPathObjIntoWave(xmlDoc *doc, xmlXPathObjectPtr xpathObj, char* options){
	int err = 0;
	int numNodes = 0;
	int j = 0;
	long ii = 0;
	Handle transfer = NULL;

	char *outputBuf = NULL;
	char *pch = NULL;
	xmlChar* xmloutputBuf = NULL;
	//delimiter is going to be used in the string tokenizer
	//the default will be space.
	const char *delimiter = " ";
	
	//for know lets use the options string to pass in the delimiters
	if (strlen(options) > 0) {
		delimiter = options;
	}
	
	/* work out how many nodes in the nodeset from the Xpath object */
	numNodes = (xpathObj->nodesetval) ? xpathObj->nodesetval->nodeNr : 0;
	
	/* Wave for output of content*/
	waveHndl outputWav;
	char *waveName = "M_xmlcontent";
	long numDimensions;
	long dimensionSizes[MAX_DIMENSIONS+1];
	long olddimensionSizes[MAX_DIMENSIONS+1];
	long indices[MAX_DIMENSIONS+1];
	memset(indices, 0, sizeof(indices));
	memset(dimensionSizes, 0, sizeof(dimensionSizes));
	
	/* Wave for output of the node names associated with content */
	waveHndl NODEoutputWav;
	char *NODEwaveName = "W_xmlcontentnodes";
	long NODEdimensionSizes[MAX_DIMENSIONS+1];
	long NODEindices[MAX_DIMENSIONS+1];
	memset(NODEindices, 0, sizeof(NODEindices));
	memset(NODEdimensionSizes, 0, sizeof(NODEdimensionSizes));
	
	//make the output textwave
	dimensionSizes[0] = 0;
	dimensionSizes[1] = 0;
	dimensionSizes[2] = 0;			
	if(err = MDMakeWave(&outputWav,waveName,NULL,dimensionSizes,TEXT_WAVE_TYPE,1))
		goto done;
		
	//make the nodenames textwave
	NODEdimensionSizes[0] = 0;
	NODEdimensionSizes[1] = 0;
	NODEdimensionSizes[2] = 0;			
	if(err = MDMakeWave(&NODEoutputWav,NODEwaveName,NULL,NODEdimensionSizes,TEXT_WAVE_TYPE,1))
		goto done;

	/* Handle to transfer the string to the wave */
	transfer = NewHandle(0);
	if(transfer == NULL){
		err = NOMEM; goto done;
	}

	/* Now fill up the wave */
	/* Content from separate nodes is put in a separate column of M_xmlcontent */
	/* Associated Node Names are put in rows of W_xmlcontentnodes */
	for(j = 0; j < numNodes; ++j) {
		indices[1] = j;
		
		/* first put out the nodenames because that is easy */
		NODEindices[0] = j;
		NODEdimensionSizes[0] = j+1;
		if(err = MDChangeWave(NODEoutputWav,-1,NODEdimensionSizes))
			goto done;
		if(err = MDSetTextWavePointValue(NODEoutputWav,NODEindices,transfer))
			goto done;
		xmloutputBuf = xmlGetNodePath(xpathObj->nodesetval->nodeTab[j]);
		if(err = PutCStringInHandle((char*)xmloutputBuf,transfer))
			goto done;
		
		if(xmloutputBuf != NULL){
			xmlFree(xmloutputBuf);
			xmloutputBuf = NULL;
		}
		if(err = MDSetTextWavePointValue(NODEoutputWav,NODEindices,transfer))
			goto done;
					
		/* get the current dimensions of the wave */
		if(err = MDGetWaveDimensions(outputWav,&numDimensions,olddimensionSizes))
			goto done;
		
		switch(xpathObj->nodesetval->nodeTab[j]->type){
			case(XML_ATTRIBUTE_NODE):
				xmloutputBuf =xmlNodeGetContent(xpathObj->nodesetval->nodeTab[j]->children);
				break;
			case(XML_TEXT_NODE):
				xmloutputBuf = xmlNodeGetContent(xpathObj->nodesetval->nodeTab[j]);
				break;
			default:
				xmloutputBuf = xmlNodeGetContent(xpathObj->nodesetval->nodeTab[j]);
				break;
		}

		
		/* Get the string from the node */
		//xmloutputBuf = xmlNodeListGetString(doc, xpathObj->nodesetval->nodeTab[j], 0);
		outputBuf = strdup((char*)xmloutputBuf);
		if(outputBuf == NULL){
			err = NOMEM; goto done;
		}
		if(xmloutputBuf != NULL){
			xmlFree(xmloutputBuf);
			xmloutputBuf = NULL;
		}
			
		/* tokenize the output */
		ii=0;
		pch = strtok(outputBuf, delimiter);
		if(pch != NULL){		//the first strtok pops the first element off the stack.
			if(err = PutCStringInHandle(pch,transfer))
					goto done;
			indices[0] = ii;
			
			/* increase the number of rows in the wave if the number of tokens is greater than the number of rows */
			ii>=olddimensionSizes[0] ? dimensionSizes[0] = ii+1 : dimensionSizes[0] = olddimensionSizes[0];
			
			numNodes == 1 ? (dimensionSizes[1] = 0) : (dimensionSizes[1] = numNodes);
			if(err = MDChangeWave(outputWav,-1,dimensionSizes))
				goto done;
			if(err = MDSetTextWavePointValue(outputWav,indices,transfer))
				goto done;
			ii+=1;
		}
	
		while (pch != NULL){
			pch = strtok (NULL, delimiter);
				
			if(pch != NULL){
				if(err = PutCStringInHandle(pch,transfer))
					goto done;
								
				indices[0] = ii;
				
				/* increase the number of rows in the wave if the number of tokens is greater than the number of rows */				
				ii>=olddimensionSizes[0] ? dimensionSizes[0] = ii+1 : dimensionSizes[0] = olddimensionSizes[0];
	
				if(err = MDChangeWave(outputWav,-1,dimensionSizes))
					goto done;
				if(err = MDSetTextWavePointValue(outputWav,indices,transfer))
					goto done;
				ii+=1;
			}
		}
		if(outputBuf != NULL){
			free(outputBuf); outputBuf = NULL;
		}
		
    }
	
	WaveHandleModified(outputWav);
	
done:
	if(xmloutputBuf != NULL)
		xmlFree(xmloutputBuf);
	if (transfer != NULL)
		DisposeHandle(transfer);
	if (outputBuf != NULL)
		free(outputBuf);
		
	return err;
}

int
XMLWaveFmXPath(XMLWaveXPathStructPtr p){
	//the error code
	int err = 0;
	
	extern std::map<long,igorXMLfile> allXMLfiles;
	xmlXPathObjectPtr xpathObj = NULL; 
	xmlDocPtr doc = NULL;

	//the fileID, Xpath handle,namespace handle,options handle
	long fileID = -1;
	char *xPath = NULL;
	char *ns    = NULL;
	char *options = NULL;
	//size of handles
	int sizexPath,sizens,sizeoptions;
	
	/* check if any of the argument string handles are null */
	if(p->xPath == NULL || p->ns == NULL || p->options == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	/* work out the size of each of the argument string handles */
	sizexPath = GetHandleSize(p->xPath);
	sizens = GetHandleSize(p->ns);
	sizeoptions = GetHandleSize(p->options);
	
	//allocate space for the C-strings.
	xPath = (char*)malloc((sizexPath+1)*sizeof(char));
	if(xPath == NULL){
		err = NOMEM;goto done;
	}
	ns = (char*)malloc((sizens+1)*sizeof(char));
	if(ns == NULL){
		err = NOMEM;goto done;
	}
	options = (char*)malloc((sizeoptions+1)*sizeof(char));
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

	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLwavefmXPath: fileID isn't valid\r");
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[fileID].doc);
	}
 
	//execute Xpath expression
	//for some reason the xpathObj doesn't like being passed as a pointer argument, therefore return it as a result.
	xpathObj = execute_xpath_expression(doc, (xmlChar*) xPath, (xmlChar*) ns, &err); 
	if(err)
		goto done;
	

	if(err = outputXPathObjIntoWave(doc, xpathObj, options))
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
	if(xPath != NULL)
		free(xPath);
	if(ns != NULL)
		free(ns);
	if(options != NULL)
		free(options);
	DisposeHandle(p->xPath);
	DisposeHandle(p->options);
	DisposeHandle(p->ns);
	
	return err;	
}