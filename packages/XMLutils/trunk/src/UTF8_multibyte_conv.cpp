
#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "UTF8_multibyte_conv.h"
#ifndef HAVE_MEMUTILS
#include "memutils.h"
#endif

//all the internal libxml2 entries are contained as UTF8.  This function takes a MemoryStruct object containing a UTF8 
//string and converts it to the system encoding.  The replacement is then returned in the MemoryStruct object.
//input should be NULL terminated, result IS NULL terminated.
int UTF8toSystemEncoding(MemoryStruct *mem){
	int err = 0;
		
#ifdef _MACINTOSH_
	char *convBuffer = NULL;
	CFStringRef str;
	if(mem == NULL || mem->getMemSize() == 0)
		goto done;
	
	if((mem->getData())[mem->getMemSize() - 1] != '\0')
		mem->append((void*) "\0", sizeof(char));

	str = CFStringCreateWithBytes(NULL, (UInt8*)mem->getData(), strlen((const char*) mem->getData()), kCFStringEncodingUTF8, 0);
	
	mem->reset();
	
	if(str){
		CFIndex numChars;
		CFIndex usedBufLen;
		CFRange rangeToProcess = CFRangeMake(0, CFStringGetLength(str));
		
		numChars = CFStringGetBytes(str,
									rangeToProcess,
									CFStringGetSystemEncoding(),
									'?',
									0,
									NULL,
									0,
									&usedBufLen);
		if(numChars > 0){
			convBuffer = (char*) malloc(sizeof(char) * (usedBufLen + 1));
			memset(convBuffer, 0, sizeof(char) * (usedBufLen + 1));
			
			if(!convBuffer){
				CFRelease(str);
				err = NOMEM;
				goto done;
			}
			numChars = CFStringGetBytes(str,
										rangeToProcess,
										CFStringGetSystemEncoding(),
										'?',
										0,
										(UInt8*) convBuffer,
										sizeof(char) * usedBufLen,
										&usedBufLen);
			
			mem->append(convBuffer, sizeof(char), usedBufLen + 1);
			
			if(convBuffer)
				free(convBuffer);
		}
		CFRelease(str);
	}
#endif	
	
#ifdef _WINDOWS_
	int len, len2;
	wchar_t *convBuffer = NULL;
	char *convBuffer2 = NULL;
	
	static int defaultANSICodePage = 0;
	
	if(mem == NULL || mem->getMemSize() == 0)
		goto done;
	
	if((mem->getData())[mem->getMemSize() - 1] != '\0')
		mem->append((void*) "\0", sizeof(char));
	
	//have to have an intermediate step of converting to UTF-16
	len = MultiByteToWideChar(CP_UTF8, 0, (const char*) mem->getData(), (size_t) strlen((const char*) mem->getData()), NULL, 0);
	if(len){
		convBuffer = (wchar_t*) malloc(len * sizeof(wchar_t));
		if(convBuffer == NULL){
			err = NOMEM;
			goto done;
		}
	} else
		goto done;
	
	if(MultiByteToWideChar(CP_UTF8, 0, (const char*) mem->getData(), (size_t) strlen((const char*) mem->getData()), convBuffer, len) == 0){
		mem->reset();
		if(convBuffer)
			free(convBuffer);
		goto done;
	}
	
	mem->reset();

	/*	We have no way to know what code page is needed to represent the Unicode text
	 as multi-byte text. Therefore, we use the system default ANSI code page.
	 This will allow, for example, a Japanese user to enter Japanese text and load it into Igor as long as he is running on a Japanese OS.
	 */
	if (defaultANSICodePage == 0)			// Did not determine code page yet?
		defaultANSICodePage = GetACP();
	
	len2 = WideCharToMultiByte(defaultANSICodePage, 0, convBuffer, len, NULL, 0, NULL, NULL);
	
	convBuffer2 = (char*) malloc(sizeof(char) * (len2 + 1));
	memset(convBuffer2, 0, sizeof(char) * (len2 + 1));
	
	if(convBuffer2 == NULL){
		if(convBuffer)
			free(convBuffer);
		err = NOMEM;
		goto done;
	}
	
	if(WideCharToMultiByte(defaultANSICodePage, 0, convBuffer, len, convBuffer2, len2, NULL, NULL) == 0){
		if(convBuffer)
			free(convBuffer);
		if(convBuffer2)
			free(convBuffer2);
		goto done;
	} else {
		if(convBuffer)
			free(convBuffer);
		mem->append(convBuffer2, sizeof(char), len2 + 1);
		if(convBuffer2)
			free(convBuffer2);
	}
	
	
#endif	
	
done:
	if(err)
		mem->reset();
	
	return err;
}


int SystemEncodingToUTF8(MemoryStruct *mem){
	int err = 0;
	
#ifdef _MACINTOSH_
	char *convBuffer = NULL;
	CFStringRef str;
	if(mem == NULL || mem->getMemSize() == 0)
		goto done;

	if((mem->getData())[mem->getMemSize() - 1] != '\0')
		mem->append((void*) "\0", sizeof(char));
	
	str = CFStringCreateWithBytes(NULL, (UInt8*)mem->getData(), strlen((const char*) mem->getData()), CFStringGetSystemEncoding(), 0);
	
	mem->reset();
	
	if(str){
		CFIndex numChars;
		CFIndex usedBufLen;
		CFRange rangeToProcess = CFRangeMake(0, CFStringGetLength(str));
		
		numChars = CFStringGetBytes(str,
									rangeToProcess,
									kCFStringEncodingUTF8,
									'?',
									0,
									NULL,
									0,
									&usedBufLen);
		if(numChars > 0){
			convBuffer = (char*) malloc(sizeof(char) * (usedBufLen + 1));
			memset(convBuffer, 0, sizeof(char) * (usedBufLen + 1));
			
			if(!convBuffer){
				CFRelease(str);
				err = NOMEM;
				goto done;
			}
			numChars = CFStringGetBytes(str,
										rangeToProcess,
										kCFStringEncodingUTF8,
										'?',
										0,
										(UInt8*) convBuffer,
										sizeof(char) * usedBufLen,
										&usedBufLen);
			
			mem->append(convBuffer, sizeof(char), usedBufLen + 1);
			
			if(convBuffer)
				free(convBuffer);
		}
		CFRelease(str);
	}
#endif	
	
#ifdef _WINDOWS_
	int len, len2;
	wchar_t *convBuffer = NULL;
	char *convBuffer2 = NULL;
	
	static int defaultANSICodePage = 0;
	
	if(mem == NULL || mem->getMemSize() == 0)
		goto done;
	
	if((mem->getData())[mem->getMemSize() - 1] != '\0')
		mem->append((void*) "\0", sizeof(char));
	
	if (defaultANSICodePage == 0)			// Did not determine code page yet?
		defaultANSICodePage = GetACP();
	
	//have to have an intermediate step of converting to UTF-16
	len = MultiByteToWideChar(defaultANSICodePage, 0, (const char*) mem->getData(), (size_t) strlen((const char*) mem->getData()), NULL, 0);
	if(len){
		convBuffer = (wchar_t*) malloc(len * sizeof(wchar_t));
		if(convBuffer == NULL){
			err = NOMEM;
			goto done;
		}
	} else
		goto done;
	
	if(MultiByteToWideChar(defaultANSICodePage, 0, (const char*) mem->getData(), (size_t) strlen((const char*) mem->getData()), convBuffer, len) == 0){
		mem->reset();
		if(convBuffer)
			free(convBuffer);
		goto done;
	}
	
	mem->reset();
	
	//UTF-8 codepage is 65001.
	len2 = WideCharToMultiByte(65001, 0, convBuffer, len, NULL, 0, NULL, NULL);
	
	convBuffer2 = (char*) malloc(sizeof(char) * (len2 + 1));
	memset(convBuffer2, 0, sizeof(char) * (len2 + 1));
	
	if(convBuffer2 == NULL){
		if(convBuffer)
			free(convBuffer);
		err = NOMEM;
		goto done;
	}
	
	if(WideCharToMultiByte(65001, 0, convBuffer, len, convBuffer2, len2, NULL, NULL) == 0){
		if(convBuffer)
			free(convBuffer);
		if(convBuffer2)
			free(convBuffer2);
		goto done;
	} else {
		if(convBuffer)
			free(convBuffer);
		mem->append(convBuffer2, sizeof(char), len2 + 1);
		if(convBuffer2)
			free(convBuffer2);
	}
	
	
#endif	
	
done:
	if(err)
		mem->reset();
	
	return err;
}