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
	
	xmlChar* attrName = NULL;
	xmlAttr* properties = NULL;
	
	long size = 0;
	char *sep = ";";
	char *sep1 = ":";
	long dimensionSizes[MAX_DIMENSIONS+1];
	long numDimensions = 0;
	long indices[MAX_DIMENSIONS+1];
	Handle pathName = NULL;

	memset(indices, 0, sizeof(indices));
	
	//XPath in 1st column
	//Namesoace in 2nd column
	//attribute names and properties in 3rd column
			
    for (cur_node = a_node; cur_node ; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
			pathName = NewHandle(0);
			if(err =MemError())
				goto done;
			path = xmlGetNodePath(cur_node);
		
			if(err = MDGetWaveDimensions(textWav,&numDimensions,dimensionSizes))
				return err;
		
			dimensionSizes[0] = dimensionSizes[0]+1; 
			dimensionSizes[1] = 3;     
			dimensionSizes[2] = 0;    
			
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

			if(cur_node->ns != NULL && cur_node->ns->href != NULL){
				if(err = PutCStringInHandle((char*)(char*)cur_node->ns->href,pathName))
					goto done;
				indices[1] = 1;
				if(err = MDSetTextWavePointValue(textWav,indices,pathName))
					goto done;
			}
			
			SetHandleSize(pathName,0);
			if(err = MemError())
				goto done;
			
			for(properties = cur_node->properties ; properties != NULL ; properties = properties->next){
				attrName = (xmlChar*) properties->name;
				if(err = PtrAndHand((char*)attrName,pathName,strlen((char*)attrName)))
					goto done;
				if(err = PtrAndHand(sep1,pathName,strlen(sep1)))
					goto done;	
				attrName = xmlGetProp(cur_node, properties->name);
				if(err = PtrAndHand((char*)attrName,pathName,strlen((char*)attrName)))
					goto done;
				if(err = PtrAndHand(sep,pathName,strlen(sep)))
					goto done;	
			}
			indices[1] = 2;
			if(err = MDSetTextWavePointValue(textWav,indices,pathName))
					goto done;
			if(pathName != NULL){
				DisposeHandle(pathName);
				pathName = NULL;
			}
//			if(cur_node->ns != NULL && cur_node->ns->prefix != NULL){
//			size = strlen((char*)cur_node->ns->prefix);
//			if(err = PtrToHand(cur_node->ns->prefix, &pathName, size))
//				goto done;
//			indices[1] = 2;
//			if(err = MDSetTextWavePointValue(textWav,indices,pathName))
//				goto done;
//			WaveHandleModified(textWav);
//			if(pathName !=NULL){
//				DisposeHandle(pathName);
//				pathName = NULL;
//			}
//			}
			if((cur_node->children != NULL) && (err = fill_element_names(cur_node->children, textWav)))
				goto done;
		}
		if(path != NULL){
			xmlFree(path);
			path = NULL;
			}
		if(pathName != NULL){
			DisposeHandle(pathName);
			pathName = NULL;
			}	
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
	
	xmlDoc *doc = NULL;
	extern std::map<int,igorXMLfile> allXMLfiles;
	int fileID = -1;	
	//xmldoc root element
    xmlNode *root_element = NULL;

	//textwave to put element list in
	waveHndl textWav = NULL;
	char *textWavName = "W_ElementList";
	int overwrite = 1;		//wave will always be overwritten
	long dimensionSizes[MAX_DIMENSIONS+1];	//used for MDMakeWave
	int type = TEXT_WAVE_TYPE;				//Xpaths will be text wave
	memset(dimensionSizes, 0, sizeof(dimensionSizes));

	fileID = (int)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[p->fileID].doc);
	}

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
   
	//need to make a textwave to contain the elements
	if(err = MDMakeWave(&textWav,textWavName,NULL,dimensionSizes,type,overwrite))
		goto done;
		
	if(err = fill_element_names(root_element, textWav))
		goto done;
	
done:
	p->retval = err;
	return err;	
}