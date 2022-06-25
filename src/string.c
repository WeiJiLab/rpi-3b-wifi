#include <plibc/string.h>

// void *memset (void *pBuffer, int nValue, size_t nLength)
// {
// 	uint32_t *p32 = (uint32_t *) pBuffer;

// 	if ( ((uint32_t) p32 & 3) == 0
// 	    && nLength >= 16)
// 	{
// 		uint32_t nValue32 = nValue | nValue << 8;
// 		nValue32 |= nValue32 << 16;

// 		do
// 		{
// 			*p32++ = nValue32;
// 			*p32++ = nValue32;
// 			*p32++ = nValue32;
// 			*p32++ = nValue32;

// 			nLength -= 16;
// 		}
// 		while (nLength >= 16);
// 	}

// 	char *p = (char *) p32;

// 	while (nLength--)
// 	{
// 		*p++ = (char) nValue;
// 	}

// 	return pBuffer;
// }

#if STDLIB_SUPPORT <= 1

// void *memmove (void *pDest, const void *pSrc, size_t nLength)
// {
// 	char *pchDest = (char *) pDest;
// 	const char *pchSrc = (const char *) pSrc;

// 	if (   pchSrc < pchDest
// 	    && pchDest < pchSrc + nLength)
// 	{
// 		pchSrc += nLength;
// 		pchDest += nLength;

// 		while (nLength--)
// 		{
// 			*--pchDest = *--pchSrc;
// 		}

// 		return pDest;
// 	}

// 	return memcpy (pDest, pSrc, nLength);
// }

// int memcmp (const void *pBuffer1, const void *pBuffer2, size_t nLength)
// {
// 	const unsigned char *p1 = (const unsigned char *) pBuffer1;
// 	const unsigned char *p2 = (const unsigned char *) pBuffer2;

// 	while (nLength-- > 0)
// 	{
// 		if (*p1 > *p2)
// 		{
// 			return 1;
// 		}
// 		else if (*p1 < *p2)
// 		{
// 			return -1;
// 		}

// 		p1++;
// 		p2++;
// 	}

// 	return 0;
// }

// static size_t strlen (const char *pString)
// {
// 	size_t nResult = 0;

// 	while (*pString++)
// 	{
// 		nResult++;
// 	}

// 	return nResult;
// }

int strcmp(const char* pString1, const char* pString2) {
    while (*pString1 != '\0' && *pString2 != '\0') {
        if (*pString1 > *pString2) {
            return 1;
        } else if (*pString1 < *pString2) {
            return -1;
        }

        pString1++;
        pString2++;
    }

    if (*pString1 > *pString2) {
        return 1;
    } else if (*pString1 < *pString2) {
        return -1;
    }

    return 0;
}

static int toupper(int c) {
    if ('a' <= c && c <= 'z') {
        c -= 'a' - 'A';
    }

    return c;
}

int strcasecmp(const char* pString1, const char* pString2) {
    int nChar1, nChar2;

    while ((nChar1 = toupper(*pString1)) != '\0' && (nChar2 = toupper(*pString2)) != '\0') {
        if (nChar1 > nChar2) {
            return 1;
        } else if (nChar1 < nChar2) {
            return -1;
        }

        pString1++;
        pString2++;
    }

    nChar2 = toupper(*pString2);

    if (nChar1 > nChar2) {
        return 1;
    } else if (nChar1 < nChar2) {
        return -1;
    }

    return 0;
}

int strncmp(const char* pString1, const char* pString2, size_t nMaxLen) {
    while (nMaxLen > 0 && *pString1 != '\0' && *pString2 != '\0') {
        if (*pString1 > *pString2) {
            return 1;
        } else if (*pString1 < *pString2) {
            return -1;
        }

        nMaxLen--;
        pString1++;
        pString2++;
    }

    if (nMaxLen == 0) {
        return 0;
    }

    if (*pString1 > *pString2) {
        return 1;
    } else if (*pString1 < *pString2) {
        return -1;
    }

    return 0;
}

int strncasecmp(const char* pString1, const char* pString2, size_t nMaxLen) {
    int nChar1, nChar2;

    while (nMaxLen > 0 && (nChar1 = toupper(*pString1)) != '\0' && (nChar2 = toupper(*pString2)) != '\0') {
        if (nChar1 > nChar2) {
            return 1;
        } else if (nChar1 < nChar2) {
            return -1;
        }

        nMaxLen--;
        pString1++;
        pString2++;
    }

    nChar2 = toupper(*pString2);

    if (nMaxLen == 0) {
        return 0;
    }

    if (nChar1 > nChar2) {
        return 1;
    } else if (nChar1 < nChar2) {
        return -1;
    }

    return 0;
}

char* strcpy(char* pDest, const char* pSrc) {
    char* p = pDest;

    while (*pSrc) {
        *p++ = *pSrc++;
    }

    *p = '\0';

    return pDest;
}

char* strncpy(char* pDest, const char* pSrc, size_t nMaxLen) {
    char* pResult = pDest;

    while (nMaxLen > 0) {
        if (*pSrc == '\0') {
            break;
        }

        *pDest++ = *pSrc++;
        nMaxLen--;
    }

    if (nMaxLen > 0) {
        *pDest = '\0';
    }

    return pResult;
}

char* strcat(char* pDest, const char* pSrc) {
    char* p = pDest;

    while (*p) {
        p++;
    }

    while (*pSrc) {
        *p++ = *pSrc++;
    }

    *p = '\0';

    return pDest;
}

char* strchr(const char* pString, int chChar) {
    while (*pString) {
        if (*pString == chChar) {
            return (char*) pString;
        }

        pString++;
    }

    return 0;
}

// char *strstr (const char *pString, const char *pNeedle)
// {
// 	if (!*pString)
// 	{
// 		if (*pNeedle)
// 		{
// 			return 0;
// 		}

// 		return (char *) pString;
// 	}

// 	while (*pString)
// 	{
// 		size_t i = 0;

// 		while (1)
// 		{
// 			if (!pNeedle[i])
// 			{
// 				return (char *) pString;
// 			}

// 			if (pNeedle[i] != pString[i])
// 			{
// 				break;
// 			}

// 			i++;
// 		}

// 		pString++;
// 	}

// 	return 0;
// }

char* strtok_r(char* pString, const char* pDelim, char** ppSavePtr) {
    char* pToken;

    if (pString == 0) {
        pString = *ppSavePtr;
    }

    if (pString == 0) {
        return 0;
    }

    if (*pString == '\0') {
        *ppSavePtr = 0;

        return 0;
    }

    while (strchr(pDelim, *pString) != 0) {
        pString++;
    }

    if (*pString == '\0') {
        *ppSavePtr = 0;

        return 0;
    }

    pToken = pString;

    while (*pString != '\0' && strchr(pDelim, *pString) == 0) {
        pString++;
    }

    if (*pString != '\0') {
        *pString++ = '\0';
    }

    *ppSavePtr = pString;

    return pToken;
}

unsigned long strtoul(const char* pString, char** ppEndPtr, int nBase) {
    unsigned long ulResult = 0;
    unsigned long ulPrevResult;
    int bMinus = 0;
    int bFirst = 1;

    if (ppEndPtr != 0) {
        *ppEndPtr = (char*) pString;
    }

    if (nBase != 0 && (nBase < 2 || nBase > 36)) {
        return ulResult;
    }

    int c;
    while ((c = *pString) == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v') {
        pString++;
    }

    if (*pString == '+' || *pString == '-') {
        if (*pString++ == '-') {
            bMinus = 1;
        }
    }

    if (*pString == '0') {
        pString++;

        if (*pString == 'x' || *pString == 'X') {
            if (nBase != 0 && nBase != 16) {
                return ulResult;
            }

            nBase = 16;

            pString++;
        } else {
            if (nBase == 0) {
                nBase = 8;
            }
        }
    } else {
        if (nBase == 0) {
            nBase = 10;
        }
    }

    while (1) {
        int c = *pString;

        if (c < '0') {
            break;
        }

        if ('a' <= c && c <= 'z') {
            c -= 'a' - 'A';
        }

        if (c >= 'A') {
            c -= 'A' - '9' - 1;
        }

        c -= '0';

        if (c >= nBase) {
            break;
        }

        ulPrevResult = ulResult;

        ulResult *= nBase;
        ulResult += c;

        if (ulResult < ulPrevResult) {
            ulResult = (unsigned long) -1;

            if (ppEndPtr != 0) {
                *ppEndPtr = (char*) pString;
            }

            return ulResult;
        }

        pString++;
        bFirst = 0;
    }

    if (ppEndPtr != 0) {
        *ppEndPtr = (char*) pString;
    }

    if (bFirst) {
        return ulResult;
    }

    if (bMinus) {
        ulResult = -ulResult;
    }

    return ulResult;
}

// unsigned long long int strtoull (const char *pString, char **ppEndPtr, int nBase)
// {
// 	unsigned long long ullResult = 0;
// 	unsigned long long ullPrevResult;
// 	int bMinus = 0;
// 	int bFirst = 1;

// 	if (ppEndPtr != 0)
// 	{
// 		*ppEndPtr = (char *) pString;
// 	}

// 	if (   nBase != 0
// 	    && (   nBase < 2
// 	        || nBase > 36))
// 	{
// 		return ullResult;
// 	}

// 	int c;
// 	while ((c = *pString) == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
// 	{
// 		pString++;
// 	}

// 	if (   *pString == '+'
// 	    || *pString == '-')
// 	{
// 		if (*pString++ == '-')
// 		{
// 			bMinus = 1;
// 		}
// 	}

// 	if (*pString == '0')
// 	{
// 		pString++;

// 		if (   *pString == 'x'
// 		    || *pString == 'X')
// 		{
// 			if (   nBase != 0
// 			    && nBase != 16)
// 			{
// 				return ullResult;
// 			}

// 			nBase = 16;

// 			pString++;
// 		}
// 		else
// 		{
// 			if (nBase == 0)
// 			{
// 				nBase =  8;
// 			}
// 		}
// 	}
// 	else
// 	{
// 		if (nBase == 0)
// 		{
// 			nBase = 10;
// 		}
// 	}

// 	while (1)
// 	{
// 		int c = *pString;

// 		if (c < '0')
// 		{
// 			break;
// 		}

// 		if ('a' <= c && c <= 'z')
// 		{
// 			c -= 'a' - 'A';
// 		}

// 		if (c >= 'A')
// 		{
// 			c -= 'A' - '9' - 1;
// 		}

// 		c -= '0';

// 		if (c >= nBase)
// 		{
// 			break;
// 		}

// 		ullPrevResult = ullResult;

// 		ullResult *= nBase;
// 		ullResult += c;

// 		if (ullResult < ullPrevResult)
// 		{
// 			ullResult = (unsigned long) -1;

// 			if (ppEndPtr != 0)
// 			{
// 				*ppEndPtr = (char *) pString;
// 			}

// 			return ullResult;
// 		}

// 		pString++;
// 		bFirst = 0;
// 	}

// 	if (ppEndPtr != 0)
// 	{
// 		*ppEndPtr = (char *) pString;
// 	}

// 	if (bFirst)
// 	{
// 		return ullResult;
// 	}

// 	if (bMinus)
// 	{
// 		ullResult = -ullResult;
// 	}

// 	return ullResult;
// }

int atoi(const char* pString) {
    return (int) strtoul(pString, 0, 10);
}

#endif

int char2int(char chValue) {
    int nResult = chValue;

    if (nResult > 0x7F) {
        nResult |= -0x100;
    }

    return nResult;
}

// #ifndef __GNUC__

// u16 bswap16 (u16 usValue)
// {
// 	return    ((usValue & 0x00FF) << 8)
// 		| ((usValue & 0xFF00) >> 8);
// }

// uint32_t bswap32 (uint32_t ulValue)
// {
// 	return    ((ulValue & 0x000000FF) << 24)
// 		| ((ulValue & 0x0000FF00) << 8)
// 		| ((ulValue & 0x00FF0000) >> 8)
// 		| ((ulValue & 0xFF000000) >> 24);
// }

// #endif


long strtol(const char *nptr, char **endptr, int base)
{
	const char *p;
	long n, nn;
	int c, ovfl, v, neg, ndig;

	p = nptr;
	neg = 0;
	n = 0;
	ndig = 0;
	ovfl = 0;

	/*
	 * White space
	 */
	for(;;p++){
		switch(*p){
		case ' ':
		case '\t':
		case '\n':
		case '\f':
		case '\r':
		case '\v':
			continue;
		}
		break;
	}

	/*
	 * Sign
	 */
	if(*p=='-' || *p=='+')
		if(*p++ == '-')
			neg = 1;

	/*
	 * Base
	 */
	if(base==0){
		if(*p != '0')
			base = 10;
		else{
			base = 8;
			if(p[1]=='x' || p[1]=='X'){
				p += 2;
				base = 16;
			}
		}
	}else if(base==16 && *p=='0'){
		if(p[1]=='x' || p[1]=='X')
			p += 2;
	}else if(base<0 || 36<base)
		goto Return;

	/*
	 * Non-empty sequence of digits
	 */
	for(;; p++,ndig++){
		c = *p;
		v = base;
		if('0'<=c && c<='9')
			v = c - '0';
		else if('a'<=c && c<='z')
			v = c - 'a' + 10;
		else if('A'<=c && c<='Z')
			v = c - 'A' + 10;
		if(v >= base)
			break;
		nn = n*base + v;
		if(nn < n)
			ovfl = 1;
		n = nn;
	}

    Return:
	if(ndig == 0)
		p = nptr;
	if(endptr)
		*endptr = (char *)p;
	if(ovfl){
		if(neg)
			return -1;
		return -1;
	}
	if(neg)
		return -n;
	return n;
}
