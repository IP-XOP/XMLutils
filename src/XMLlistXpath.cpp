/*
 *  XMLlistXpath.cpp
 *  XMLutils
 *
 *  Created by andrew on 10/12/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLutils.h"

static int
fill_xpath_list(xmlNodeSet *nodesetval)
{
	int err = 0;
	xmlChar *path = NULL;
	
	int ii = 0;

	//textwave to put element list in
	waveHndl textWav = NULL;
	char *textWavName = "M_listXPath";
	int overwrite = 1;		//wave will always be overwritten
	int type = TEXT_WAVE_TYPE;				//Xpaths will be text wave
	long dimensionSizes[MAX_DIMENSIONS+1];
	long numDimensions = 0;
	long indices[MAX_DIMENSIONS+1];
	int size = 0;
	
	Handle pathName = NULL;

	memset(indices, 0, sizeof(indices));
	memset(dimensionSizes,0,sizeof(dimensionSizes));
	pathName = NewHandle(0);
	if(err =MemError())
		goto done;
	
	//now 2D
	dimensionSizes[1] = 2;
	
	//need to make a textwave to contain the elements
	if(err = MDMakeWave(&textWav,textWavName,NULL,dimensionSizes,type,overwrite))
		goto done;

	size = (nodesetval) ? nodesetval->nodeNr : 0;
 
	//XPathNames in 1st column
			
    for (ii = 0; ii < size ; ii+=1) {
	
			path = xmlGetNodePath(nodesetval->nodeTab[ii]);
		
			if(err = MDGetWaveDimensions(textWav,&numDimensions,dimensionSizes))
				return err;
		
			dimensionSizes[0] = dimensionSizes[0]+1; 
			dimensionSizes[1] = 2;
			
			if(err = MDChangeWave(textWav,-1,dimensionSizes))
				goto done;
			
			if(err = PutCStringInHandle((char*)path,pathName))
				goto done;
			indices[0] = dimensionSizes[0]-1;
			indices[1] = 0;
			
			if(path != NULL){
				xmlFree(path);
				path = NULL;
			}
			if(err = MDSetTextWavePointValue(textWav,indices,pathName))
				goto done;
			
			if(err = PutCStringInHandle((char*)(nodesetval->nodeTab[ii]->name),pathName))
				goto done;
			indices[1] = 1;
			if(err = MDSetTextWavePointValue(textWav,indices,pathName))
				goto done;
			
		}

done:
if(path != NULL)
	xmlFree(path);
if(pathName != NULL)
	DisposeHandle(pathName);	

return err;
}

int
XMLlistXPath(XMLlistXpathStructPtr p){
	//the error code
	int err = 0;
	
	extern std::map<int,igorXMLfile> allXMLfiles;
	int fileID = -1;
	xmlXPathObject *xpathObj = NULL; 
	xmlDoc *doc = NULL;

	//the filename handle, Xpath handle,namespace handle,options handle
	char *xPath = NULL;
	char *ns    = NULL;
	//size of handles
	int sizexPath,sizens;
	
	if(p->xPath == NULL || p->ns == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
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
		XOPNotice("XMLlistXpath: FileID isn't valid\r");
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
	if(err = fill_xpath_list(xpathObj->nodesetval))
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
	if(p->xPath != NULL)
		DisposeHandle(p->xPath);
	if(p->ns)
		DisposeHandle(p->ns);
		
	return err;	
}
