/*
 *  XMLdumpDoc.cpp
 *  XMLutils
 *
 *  Created by andrew on 15/12/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLutils.h"
#include <string>
using namespace std;

int
XMLdocDump(XMLdocDumpStruct *p){
	int err = 0;
	extern std::map<long,igorXMLfile> allXMLfiles;
	long fileID = -1;
	xmlDoc *doc = NULL;
	
	int buffersize = 0;
	xmlChar *xmlbuff = NULL;
	
	string str;
	string newline("\n");
	size_t found;
	long message;
	
	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLdocdump: fileID isn't valid");
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[fileID].doc);
	}

    /*
	dump the document to IGOR history
	*/
	xmlIndentTreeOutput = 1;
	xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, 1);
	
	str = ((char*)xmlbuff);
	
	//IGOR doesn't like \n, it wants \r.  So convert all \n to \r.
	while(str.find(newline) != string::npos){
		found = str.find(newline);
		str.replace(found,1,"\r");
	}
	
	message = GetXOPMessage();
	//dump it to IGOR command line.
	XOPNotice(str.c_str());
	
done:
	/*
     * Free associated memory.
     */
	(err == 0)? (p->retval = 0):(p->retval = -1);

	if(err == FILEID_DOESNT_EXIST){
		err = 0;
		p->retval = -1;
	}
	
    if(xmlbuff != NULL)
		xmlFree(xmlbuff);

	return err;
}
