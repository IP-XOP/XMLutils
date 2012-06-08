///*
// *  XMLelementlist.cpp
// *  XMLutils
// *

#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "XMLutils.h"
#include <string>
#include "UTF8_multibyte_conv.h"


xmlChar* ARJNxmlGetNodePath(xmlNodePtr node)
{
    xmlNodePtr cur, tmp, next;
    xmlChar *buffer = NULL, *temp;
    size_t buf_len;
    xmlChar *buf;
    const char *sep;
    const char *name;
    char nametemp[100];
    int occur = 0, generic;

    if (node == NULL)
        return (NULL);

    buf_len = 500;
    buffer = (xmlChar *) xmlMallocAtomic(buf_len * sizeof(xmlChar));
    if (buffer == NULL) {
        return (NULL);
    }
    buf = (xmlChar *) xmlMallocAtomic(buf_len * sizeof(xmlChar));
    if (buf == NULL) {
        xmlFree(buffer);
        return (NULL);
    }

    buffer[0] = 0;
    cur = node;
    do {
        name = "";
        sep = "?";
        occur = 0;
        if ((cur->type == XML_DOCUMENT_NODE) ||
            (cur->type == XML_HTML_DOCUMENT_NODE)) {
            if (buffer[0] == '/')
                break;
            sep = "/";
            next = NULL;
        } else if (cur->type == XML_ELEMENT_NODE) {
	    generic = 0;
            sep = "/";
            name = (const char *) cur->name;
            if (cur->ns) {
		if (cur->ns->prefix != NULL) {
                    snprintf(nametemp, sizeof(nametemp) - 1, "%s:%s",
		    	(char *)cur->ns->prefix, (char *)cur->name);
		    nametemp[sizeof(nametemp) - 1] = 0;
		    name = nametemp;
		} else {
		    /*
		    * We cannot express named elements in the default
		    * namespace, so use "*".
		    */
		    generic = 1;
		    name = "*";
		}                
            }
            next = cur->parent;

            /*
             * Thumbler index computation
	     * TODO: the ocurence test seems bogus for namespaced names
             */
            tmp = cur->prev;
            while (tmp != NULL) {
                if ((tmp->type == XML_ELEMENT_NODE) &&
		    (generic ||
		     (xmlStrEqual(cur->name, tmp->name) &&
		     ((tmp->ns == cur->ns) ||
		      ((tmp->ns != NULL) && (cur->ns != NULL) &&
		       (xmlStrEqual(cur->ns->prefix, tmp->ns->prefix)))))))
                    occur++;
                tmp = tmp->prev;
            }
            if (occur == 0) {
                tmp = cur->next;
                while (tmp != NULL && occur == 0) {
                    if ((tmp->type == XML_ELEMENT_NODE) &&
			(generic ||
			 (xmlStrEqual(cur->name, tmp->name) &&
			 ((tmp->ns == cur->ns) ||
			  ((tmp->ns != NULL) && (cur->ns != NULL) &&
			   (xmlStrEqual(cur->ns->prefix, tmp->ns->prefix)))))))
                        occur++;
                    tmp = tmp->next;
                }
                if (occur != 0)
                    occur = 1;
            } else
                occur++;
        } else if (cur->type == XML_COMMENT_NODE) {
            sep = "/";
	    name = "comment()";
            next = cur->parent;

            /*
             * Thumbler index computation
             */
            tmp = cur->prev;
            while (tmp != NULL) {
                if (tmp->type == XML_COMMENT_NODE)
		    occur++;
                tmp = tmp->prev;
            }
            if (occur == 0) {
                tmp = cur->next;
                while (tmp != NULL && occur == 0) {
		  if (tmp->type == XML_COMMENT_NODE)
		    occur++;
                    tmp = tmp->next;
                }
                if (occur != 0)
                    occur = 1;
            } else
                occur++;
        } else if ((cur->type == XML_TEXT_NODE) ||
                   (cur->type == XML_CDATA_SECTION_NODE)) {
            sep = "/";
	    name = "text()";
            next = cur->parent;

            /*
             * Thumbler index computation
             */
            tmp = cur->prev;
            while (tmp != NULL) {
                if ((tmp->type == XML_TEXT_NODE) ||
		    (tmp->type == XML_CDATA_SECTION_NODE))
		    occur++;
                tmp = tmp->prev;
            }
	    /*
	    * Evaluate if this is the only text- or CDATA-section-node;
	    * if yes, then we'll get "text()", otherwise "text()[1]".
	    */
            if (occur == 0) {
                tmp = cur->next;
                while (tmp != NULL) {
		    if ((tmp->type == XML_TEXT_NODE) ||
			(tmp->type == XML_CDATA_SECTION_NODE))
		    {
			occur = 1;
			break;
		    }			
		    tmp = tmp->next;
		}
            } else
                occur++;
        } else if (cur->type == XML_PI_NODE) {
            sep = "/";
	    snprintf(nametemp, sizeof(nametemp) - 1,
		     "processing-instruction('%s')", (char *)cur->name);
            nametemp[sizeof(nametemp) - 1] = 0;
            name = nametemp;

	    next = cur->parent;

            /*
             * Thumbler index computation
             */
            tmp = cur->prev;
            while (tmp != NULL) {
                if ((tmp->type == XML_PI_NODE) &&
		    (xmlStrEqual(cur->name, tmp->name)))
                    occur++;
                tmp = tmp->prev;
            }
            if (occur == 0) {
                tmp = cur->next;
                while (tmp != NULL && occur == 0) {
                    if ((tmp->type == XML_PI_NODE) &&
			(xmlStrEqual(cur->name, tmp->name)))
                        occur++;
                    tmp = tmp->next;
                }
                if (occur != 0)
                    occur = 1;
            } else
                occur++;

        } else if (cur->type == XML_ATTRIBUTE_NODE) {
            sep = "/@";
            name = (const char *) (((xmlAttrPtr) cur)->name);
            if (cur->ns) {
	        if (cur->ns->prefix != NULL)
                    snprintf(nametemp, sizeof(nametemp) - 1, "%s:%s",
		    	(char *)cur->ns->prefix, (char *)cur->name);
		else
		    snprintf(nametemp, sizeof(nametemp) - 1, "%s",
		    	(char *)cur->name);
                nametemp[sizeof(nametemp) - 1] = 0;
                name = nametemp;
            }
            next = ((xmlAttrPtr) cur)->parent;
        } else {
            next = cur->parent;
        }

        /*
         * Make sure there is enough room
         */
        if (xmlStrlen(buffer) + sizeof(nametemp) + 20 > buf_len) {
            buf_len =
                2 * buf_len + xmlStrlen(buffer) + sizeof(nametemp) + 20;
            temp = (xmlChar *) xmlRealloc(buffer, buf_len);
            if (temp == NULL) {
                xmlFree(buf);
                xmlFree(buffer);
                return (NULL);
            }
            buffer = temp;
            temp = (xmlChar *) xmlRealloc(buf, buf_len);
            if (temp == NULL) {
                xmlFree(buf);
                xmlFree(buffer);
                return (NULL);
            }
            buf = temp;
        }
        if (occur == 0)
            snprintf((char *) buf, buf_len, "%s%s%s",
                     sep, name, (char *) buffer);
        else
            snprintf((char *) buf, buf_len, "%s%s[%d]%s",
                     sep, name, occur, (char *) buffer);
        snprintf((char *) buffer, buf_len, "%s", (char *)buf);
        cur = next;
    } while (cur != NULL);
    xmlFree(buf);
    return (buffer);
}

 
static int
fill_element_names(xmlNode * a_node, waveHndl textWav)
{
	int err = 0;
    xmlNode *cur_node = NULL;
	xmlChar *path = NULL;
	string data;
	
	xmlAttr* properties = NULL;
	
	char *sep = ";";
	char *sep1 = ":";
	CountInt dimensionSizes[MAX_DIMENSIONS+1];
	CountInt indices[MAX_DIMENSIONS+1];
	int numDimensions = 0;
	Handle pathName = NULL;

	memset(indices, 0, sizeof(indices));
	
	//XPath in 1st column
	//Namesoace in 2nd column
	//attribute names and properties in 3rd column
	//element name in 4th column
			
    for (cur_node = a_node; cur_node ; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
			//put XPATH in col 0
			pathName = NewHandle(0);
			if(err =MemError())
				goto done;
			path = ARJNxmlGetNodePath(cur_node);//xmlGetNodePath(cur_node);
			
			data.assign((const char*) path, sizeof(xmlChar) * xmlStrlen(path));

			if(err = UTF8toSystemEncoding(data))
				goto done;
			
			if(err = MDGetWaveDimensions(textWav, &numDimensions, dimensionSizes))
				return err;
		
			dimensionSizes[0] = dimensionSizes[0]+1; 
			dimensionSizes[1] = 4;     
			dimensionSizes[2] = 0;    
			
			if(err = MDChangeWave(textWav, -1, dimensionSizes))
				goto done;
			
			if(err = PutCStringInHandle((char*)data.c_str(), pathName))
				goto done;
			
			indices[0] = dimensionSizes[0] - 1;
			indices[1] = 0;
			
			if(path != NULL){
				xmlFree(path);
				path = NULL;
			}
			if(err = MDSetTextWavePointValue(textWav, indices, pathName))
				goto done;
			
			//put name in 4th col
			SetHandleSize(pathName , 0);
			if(MemError())
				goto done;	
			
			data.assign((const char*) cur_node->name, sizeof(xmlChar) * xmlStrlen(cur_node->name));

			if(err = UTF8toSystemEncoding(data))
				goto done;
			
			if(err = PutCStringInHandle((char*) data.c_str(), pathName))
				goto done;			
			indices[1] = 3;
			if(err = MDSetTextWavePointValue(textWav, indices, pathName))
				goto done;

			//put namespace in col1
			if(cur_node->ns != NULL && cur_node->ns->href != NULL){
				SetHandleSize(pathName , 0);
				if(MemError())
					goto done;
				
				if(cur_node->ns->prefix != NULL && xmlStrlen(cur_node->ns->prefix) > 0){
					data.assign((const char*) cur_node->ns->prefix, sizeof(xmlChar) * xmlStrlen(cur_node->ns->prefix));

					if(err = UTF8toSystemEncoding(data))
						goto done;
					
					if(err = PtrAndHand((void*) data.c_str(), pathName, data.size()))
						goto done;
					if(err = PtrAndHand((void*) "=", pathName, sizeof(char)))
						goto done;
				}
				data.assign((const char*) cur_node->ns->href, sizeof(xmlChar) * xmlStrlen(cur_node->ns->href));
				
				if(err = UTF8toSystemEncoding(data))
					goto done;
				
				if(err = PtrAndHand((char*) data.data(), pathName, data.size()))
					goto done;					
				
				indices[1] = 1;
				if(err = MDSetTextWavePointValue(textWav, indices, pathName))
					goto done;
			}
			
			SetHandleSize(pathName, 0);
			if(err = MemError())
				goto done;
			
			
			//put properties and attributes in 3rd col
			for(properties = cur_node->properties ; properties != NULL ; properties = properties->next){
				xmlChar *attributeProp = NULL;
				
				data.assign((const char*) properties->name, sizeof(xmlChar) * xmlStrlen(properties->name));

				if(err = UTF8toSystemEncoding(data))
					goto done;
				
				if(err = PtrAndHand((char*)data.data(), pathName, data.size()))
					goto done;
				if(err = PtrAndHand(sep1, pathName, sizeof(char) * strlen(sep1)))
					goto done;	
				
				attributeProp = xmlGetProp(cur_node, properties->name);

				data.assign((const char*) attributeProp, sizeof(xmlChar) * xmlStrlen(attributeProp));

				if(attributeProp)
					xmlFree(attributeProp);
				
				if(err = UTF8toSystemEncoding(data))
					goto done;
				
				if(err = PtrAndHand((char*)data.data(), pathName, data.size()))
					goto done;
				if(err = PtrAndHand(sep, pathName, sizeof(char) * strlen(sep)))
					goto done;
			}
			indices[1] = 2;
			if(err = MDSetTextWavePointValue(textWav, indices, pathName))
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
	extern std::map<long,igorXMLfile> allXMLfiles;
	long fileID = -1;	
    xmlNode *root_element = NULL;

	//textwave to put element list in
	waveHndl textWav = NULL;
	char *textWavName = "W_ElementList";
	int overwrite = 1;		//wave will always be overwritten
	CountInt dimensionSizes[MAX_DIMENSIONS + 1];	//used for MDMakeWave
	int type = TEXT_WAVE_TYPE;				//Xpaths will be text wave
	memset(dimensionSizes, 0, sizeof(dimensionSizes));

	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLelemlist: fileID isn't valid\r");
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[fileID].doc);
	}

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
   
	//need to make a textwave to contain the elements
	if(err = MDMakeWave(&textWav, textWavName, NULL, dimensionSizes, type, overwrite))
		goto done;
		
	if(err = fill_element_names(root_element, textWav))
		goto done;
	
done:
	(err == 0)? (p->retval = 0) : (p->retval = -1);

	if(err == FILEID_DOESNT_EXIST){
		err = 0;
	}
	return err;	
}