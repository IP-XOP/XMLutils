
#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "UTF8_multibyte_conv.h"


//all the internal libxml2 entries are contained as UTF8.  This function takes a MemoryStruct object containing a UTF8 
//string and converts it to the system encoding.  The replacement is then returned in the MemoryStruct object.
//input should be NULL terminated, result IS NULL terminated.
int UTF8toSystemEncoding(string mem){
	int err = 0;
		
#ifdef MACIGOR
	char *convBuffer = NULL;
	CFStringRef str;

	if(mem.size() == 0){
		mem.clear();
		goto done;
	}	
	
	if(mem.data()[mem.size() - 1] != '\0')
	   mem.append("\0");
	   
	str = CFStringCreateWithBytes(NULL, (UInt8*)mem.data(), mem.size(), kCFStringEncodingUTF8, 0);
	
	mem.clear();
	
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
			if(!convBuffer){
				CFRelease(str);
				err = 1;
				goto done;
			}
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
			
			mem.append(convBuffer, usedBufLen + 1);
			
			if(convBuffer)
				free(convBuffer);
		}
		CFRelease(str);
	}
#endif	
	
#ifdef WINIGOR
	int len, len2;
	wchar_t *convBuffer = NULL;
	char *convBuffer2 = NULL;
	
	static int defaultANSICodePage = 0;
	
	if(mem.size() == 0){
		mem.clear();
		goto done;
	}	
	
	if(mem.data()[mem.size() - 1] != '\0')
		mem.append("\0");
	
	//have to have an intermediate step of converting to UTF-16
	len = MultiByteToWideChar(CP_UTF8, 0, (const char*) mem.data(), mem.size(), NULL, 0);
	if(len){
		convBuffer = (wchar_t*) malloc(len * sizeof(wchar_t));
		if(convBuffer == NULL){
			err = 1;
			goto done;
		}
	} else
		goto done;
	
	if(MultiByteToWideChar(CP_UTF8, 0, (const char*) mem.data(), mem.size(), convBuffer, len) == 0){
		mem.clear();
		if(convBuffer)
			free(convBuffer);
		goto done;
	}
	
	mem.clear();

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
		err = 1;
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
		mem.append(convBuffer2, len2 + 1);

		if(convBuffer2)
			free(convBuffer2);
	}
	
	
#endif	
	
done:
	if(err)
		mem.clear();
	
	if(!err && mem.size() == 0)
		mem.append("\0");
	
	return err;
}


int SystemEncodingToUTF8(string mem){
	int err = 0;
	
#ifdef MACIGOR
	char *convBuffer = NULL;
	CFStringRef str;

	if(mem.size() == 0)
		mem.clear();

	if(mem.data()[mem.size() - 1] != '\0')
		mem.append("\0");
	
	str = CFStringCreateWithBytes(NULL, (UInt8*)mem.data(), mem.size(), CFStringGetSystemEncoding(), 0);
	
	mem.clear();
	
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
				err = 1;
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
			
			mem.append(convBuffer, usedBufLen + 1);
			
			if(convBuffer)
				free(convBuffer);
		}
		CFRelease(str);
	}
#endif	
	
#ifdef WINIGOR
	int len, len2;
	wchar_t *convBuffer = NULL;
	char *convBuffer2 = NULL;
	
	static int defaultANSICodePage = 0;
	
	if(mem.size() == 0)
		mem.clear();
	
	if(mem.data()[mem.size() - 1] != '\0')
		mem.append("\0");
	
	if (defaultANSICodePage == 0)			// Did not determine code page yet?
		defaultANSICodePage = GetACP();
	
	//have to have an intermediate step of converting to UTF-16
	len = MultiByteToWideChar(defaultANSICodePage, 0, (const char*) mem.data(), mem.size(), NULL, 0);
	if(len){
		convBuffer = (wchar_t*) malloc(len * sizeof(wchar_t));
		if(convBuffer == NULL){
			err = 1;
			goto done;
		}
	} else
		goto done;
	
	if(MultiByteToWideChar(defaultANSICodePage, 0, (const char*) mem.data(), mem.size(), convBuffer, len) == 0){
		mem.clear();
		if(convBuffer)
			free(convBuffer);
		goto done;
	}
	
	mem.clear();
	
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
		mem.append(convBuffer2, len2 + 1);

		if(convBuffer2)
			free(convBuffer2);
	}
	
	
#endif	
	
done:
	if(err)
		mem.clear();

	if(!err && mem.size() == 0)
		mem.append("\0");
	
	return err;
}