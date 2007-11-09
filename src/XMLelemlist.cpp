///*
// *  XMLelementlist.cpp
// *  XMLutils
// *
// *  Created by andrew on 6/11/07.
// *  Copyright 2007 __MyCompanyName__. All rights reserved.
// *
// */
// //
//// 	try{
////		pj = new MyComplex [nlayers+2];
////	} catch(...){
////		err = NOMEM;
////		goto done;
////	}
////delete[] pj;	
//	
#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "XMLutils.h"

static int
fill_element_names(xmlNode * a_node, waveHndl textWav)
{
	int err = 0;
    xmlNode *cur_node = NULL;
	xmlChar *path = NULL;
	
	long size = 0;
	//	char *newline = "\r";
	long dimensionSizes[MAX_DIMENSIONS+1];
	long numDimensions = 0;
	long indices[MAX_DIMENSIONS+1];
	Handle pathName = NULL;
	
	memset(indices, 0, sizeof(indices));
					
    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
			path = xmlGetNodePath(cur_node);
		
			if(err = MDGetWaveDimensions(textWav,&numDimensions,dimensionSizes))
				return err;
		
			dimensionSizes[0] = dimensionSizes[0]+1; 
			dimensionSizes[1] = 3;     
			dimensionSizes[2] = 0;    
			
			if(err = MDChangeWave(textWav,-1,dimensionSizes))
				goto done;
			
			size = strlen((char*)path);
			if(err = PtrToHand(path, &pathName, size))
				goto done;
			indices[0] = dimensionSizes[0]-1;
			indices[1] = 0;
			if(path != NULL){
				xmlFree(path);
				path = NULL;
			}
			if(err = MDSetTextWavePointValue(textWav,indices,pathName))
				goto done;
			WaveHandleModified(textWav);
			if(pathName !=NULL){
				DisposeHandle(pathName);
				pathName = NULL;
			}
			if(cur_node->ns != NULL && cur_node->ns->href != NULL){
			size = strlen((char*)cur_node->ns->href);
			if(err = PtrToHand(cur_node->ns->href, &pathName, size))
				goto done;
			indices[1] = 1;
			if(err = MDSetTextWavePointValue(textWav,indices,pathName))
				goto done;
			WaveHandleModified(textWav);
			if(pathName !=NULL){
				DisposeHandle(pathName);
				pathName = NULL;
			}
			}
			if(cur_node->ns != NULL && cur_node->ns->prefix != NULL){
			size = strlen((char*)cur_node->ns->prefix);
			if(err = PtrToHand(cur_node->ns->prefix, &pathName, size))
				goto done;
			indices[1] = 2;
			if(err = MDSetTextWavePointValue(textWav,indices,pathName))
				goto done;
			WaveHandleModified(textWav);
			if(pathName !=NULL){
				DisposeHandle(pathName);
				pathName = NULL;
			}
			}
		}
	
		if(err = fill_element_names(cur_node->children, textWav))
			goto done;
		if(path != NULL)
			xmlFree(path);
		if(pathName != NULL)
			DisposeHandle(pathName);	
	}
done:
if(pathName != NULL)
	DisposeHandle(pathName);	

return err;
}

int
XMLelemlist(XMLelemlistStructPtr p){
	//the error code
	int err = 0;
	
	//the filename string
	char *fileName = NULL;
	char nativePath[MAX_PATH_LEN+1];
	
	//the xmldoc object
	xmlDoc *doc = NULL;
	
	//xmldoc root element
    xmlNode *root_element = NULL;
	
	//size of filenamehandle
	int size;
	
	//textwave to put element list in
	waveHndl textWav = NULL;
	char *textWavName = "W_ElementList";
	int overwrite = 1;		//wave will always be overwritten
	long dimensionSizes[MAX_DIMENSIONS+1];	//used for MDMakeWave
	int type = TEXT_WAVE_TYPE;				//Xpaths will be text wave
	memset(dimensionSizes, 0, sizeof(dimensionSizes));
	
	size = GetHandleSize(p->fileNameStr);
	
	//first step is to get the filename from the filename handle
	fileName = (char*)malloc(size*sizeof(char)+1);
	if(fileName == NULL){
		err = NOMEM;
		goto done;
	}
	
	if (err = GetCStringFromHandle(p->fileNameStr, fileName, size))
		goto done;
	if (err = GetNativePath(fileName,nativePath))
		goto done;
	
	/*parse the file and get the DOM */
    doc = xmlReadFile( fileName, NULL, 0);
	if(doc == NULL){
		err = XMLDOC_PARSE_ERROR;
		goto done;
	}
	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
   
	//need to make a textwave to contain the elements
	if(err = MDMakeWave(&textWav,textWavName,NULL,dimensionSizes,type,overwrite))
		goto done;
		
	if(err = fill_element_names(root_element, textWav))
		goto done;
	
done:
	if(fileName != NULL)
		free(fileName);
	
	/* free the document */
	xmlFreeDoc(doc);
	/*
	 *Free the global variables that may
	 *have been allocated by the parser.
	 */
	xmlCleanupParser();
	
	return err;	
}