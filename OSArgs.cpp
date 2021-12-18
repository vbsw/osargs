/*
 *          Copyright 2021, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include "OSArgs.h"
#include "g2d/String.h"

static void copyStrsW(LPCWSTR *const srcStrings, const int srcLength, LPWSTR **strings, int **lengths, int *length)
{
	int totalLen = 0;
	strings[0] = new LPWSTR[srcLength];
	lengths[0] = new int[srcLength];
	length[0] = srcLength;
	for (int i = 0; i < srcLength; i++)
	{
		const int srcStrLen = (int)wcslen(srcStrings[i]);
		if (srcStrLen > 0)
		{
			lengths[0][i] = srcStrLen;
			totalLen += srcStrLen + 1;
		}
	}
	LPWSTR const srcStringsCopy = new WCHAR[totalLen];
	for (int offset = 0, i = 0; i < srcLength; i++)
	{
		memcpy(&srcStringsCopy[offset], srcStrings[i], sizeof(WCHAR) * (size_t)(lengths[0][i] + 1));
		strings[0][i] = &srcStringsCopy[offset];
		offset += lengths[0][i] + 1;
	}
}

static void copyStrs(LPCSTR *const srcStrings, const int srcLength, LPWSTR **strings, int **lengths, int *length)
{
	int totalLen = 0;
	strings[0] = new LPWSTR[srcLength];
	lengths[0] = new int[srcLength];
	length[0] = srcLength;
	for (int i = 0; i < srcLength; i++)
	{
		const int length = (int)strlen(srcStrings[i]);
		lengths[0][i] = length;
		totalLen += length + 1;
	}
	LPWSTR const srcStringsCopy = new WCHAR[totalLen];
	for (int offset = 0, i = 0; i < srcLength; i++)
	{
		MultiByteToWideChar(CP_UTF8, 0, srcStrings[i], lengths[0][i] + 1, &srcStringsCopy[offset], lengths[0][i] + 1);
		strings[0][i] = &srcStringsCopy[offset];
		offset += lengths[0][i] + 1;
	}
}

static int hasPrefix(LPCWSTR const str, LPCWSTR const prefix)
{
	int i = 0;
	while (str[i] == prefix[i])
		i++;
	if (prefix[i] == 0)
		return i;
	return -1;
}

OSArgs::Result::Result() : values(NULL), length(0), capacity(0)
{
}

OSArgs::Result::~Result()
{
	if (values)
	{
		for (int i = 0; i < length; i++)
			delete[] values[i];
		delete[] values;
	}
}

BOOL OSArgs::Result::isAvailable()
{
	return length > 0;
}

LPWSTR * OSArgs::Result::getValues()
{
	return values;
}

int OSArgs::Result::getLength()
{
	return length;
}

OSArgs::Result * OSArgs::Result::add(LPCWSTR const value)
{
	const int length = (int)wcslen(value);
	return add(value, length);
}

OSArgs::Result * OSArgs::Result::add(LPCWSTR const value, const int length)
{
	ensureCapacity();
	LPWSTR valueNew = new WCHAR[length + 1];
	memcpy(valueNew, value, sizeof(WCHAR) * (size_t)(length + 1));
	values[this->length] = valueNew;
	this->length++;
	return this;
}

void OSArgs::Result::ensureCapacity()
{
	if (values == NULL)
	{
		values = new LPWSTR[1];
		capacity = 1;
	}
	else if (length == capacity)
	{
		capacity = capacity * 2;
		LPWSTR *const valuesNew = new LPWSTR[capacity];
		memcpy(valuesNew, values, sizeof(LPWSTR) * length);
		delete[] values;
		values = valuesNew;
	}
}

OSArgs::Delimiter::Delimiter(const BOOL blank, const BOOL empty, LPCWSTR *const tokens, const int length) : blank(FALSE), empty(FALSE),
	tokens(NULL), length(0)
{
	this->blank = blank;
	this->empty = empty;
	if (length > 0)
	{
		int *lengths = NULL;
		copyStrsW(tokens, length, &this->tokens, &lengths, &this->length);
		if (lengths)
			delete[] lengths;
	}
}

OSArgs::Delimiter::Delimiter(const BOOL blank, const BOOL empty, LPCSTR *const tokens, const int length) : blank(FALSE), empty(FALSE),
	tokens(NULL), length(0)
{
	this->blank = blank;
	this->empty = empty;
	if (length > 0)
	{
		int *lengths = NULL;
		copyStrs(tokens, length, &this->tokens, &lengths, &this->length);
		if (lengths)
			delete[] lengths;
	}
}

OSArgs::Delimiter::~Delimiter()
{
	if (tokens)
	{
		delete[] tokens[0];
		delete[] tokens;
	}
}

BOOL OSArgs::Delimiter::isBlank()
{
	return blank;
}

int OSArgs::Delimiter::getMatchingTokenLength(LPCWSTR const str)
{
	for (int i = 0; i < length; i++)
	{
		LPCWSTR const token = tokens[i];
		const int prefixLen = hasPrefix(str, token);
		if (prefixLen >= 0)
			return prefixLen;
	}
	if (empty)
		return 0;
	return -1;
}

OSArgs::OSArgs() : values(NULL), lengths(NULL), parsed(NULL), length(0), unparsedArgs(NULL), results(NULL), resultsLen(0),
	resultsCap(0), delimiters(NULL), delimitersLen(0), delimitersCap(0)
{
	int argc;
	LPCTSTR const cmdl = GetCommandLine();
	#ifdef UNICODE
	LPWSTR *const argv = CommandLineToArgvW(cmdl, &argc);
	#else
	const int cmdlLen = (int)strlen(cmdl) + 1;
	LPWSTR const cmdlW = new WCHAR[cmdlLen];
	MultiByteToWideChar(CP_UTF8, 0, cmdl, cmdlLen, cmdlW, cmdlLen);
	LPWSTR *const argv = CommandLineToArgvW(cmdlW, &argc);
	delete[] cmdlW;
	#endif // !UNICODE
	if (argc > 1)
		init((LPCWSTR*)&argv[1], argc - 1);
	//init((LPCWSTR*)&argv[0], argc);
	LocalFree(argv);
}

OSArgs::OSArgs(LPCWSTR *const argv, const int argc) : values(NULL), lengths(NULL), parsed(NULL), length(0), unparsedArgs(NULL),
	results(NULL), resultsLen(0), resultsCap(0), delimiters(NULL), delimitersLen(0), delimitersCap(0)
{
	init(argv, argc);
}

OSArgs::OSArgs(LPCSTR *const argv, const int argc) : values(NULL), lengths(NULL), parsed(NULL), length(0), unparsedArgs(NULL),
	results(NULL), resultsLen(0), resultsCap(0), delimiters(NULL), delimitersLen(0), delimitersCap(0)
{
	init(argv, argc);
}

OSArgs::~OSArgs()
{
	if (values)
	{
		delete[] values[0];
		delete[] values;
	}
	if (lengths)
		delete[] lengths;
	if (results)
	{
		for (int i = 0; i < resultsLen; i++)
			delete results[i];
		delete[] results;
	}
	if (delimiters)
	{
		for (int i = 0; i < delimitersLen; i++)
			delete delimiters[i];
		delete[] delimiters;
	}
	if (unparsedArgs)
		delete[] unparsedArgs;
}

OSArgs::Delimiter * OSArgs::newDelimiter(const BOOL blank, const BOOL empty, LPCWSTR *const tokens, const int length)
{
	OSArgs::Delimiter *const delimiter = new OSArgs::Delimiter(blank, empty, tokens, length);
	return newDelimiter(delimiter);
}

OSArgs::Delimiter * OSArgs::newDelimiter(const BOOL blank, const BOOL empty, LPCWSTR const tokenA, LPCWSTR const tokenB, LPCWSTR const tokenC, LPCWSTR const tokenD)
{
	LPCWSTR tokens[4];
	int length = 0;
	if (tokenA) { tokens[length] = tokenA; length++; }
	if (tokenB) { tokens[length] = tokenB; length++; }
	if (tokenC) { tokens[length] = tokenC; length++; }
	if (tokenD) { tokens[length] = tokenD; length++; }
	Delimiter *const delimiter = new Delimiter(blank, empty, tokens, length);
	return newDelimiter(delimiter);
}

OSArgs::Delimiter * OSArgs::newDelimiter(const BOOL blank, const BOOL empty, LPCSTR *const tokens, const int length)
{
	OSArgs::Delimiter *const delimiter = new OSArgs::Delimiter(blank, empty, tokens, length);
	return newDelimiter(delimiter);
}

OSArgs::Delimiter * OSArgs::newDelimiter(const BOOL blank, const BOOL empty, LPCSTR const tokenA, LPCSTR const tokenB, LPCSTR const tokenC, LPCSTR const tokenD)
{
	LPCSTR tokens[4];
	int length = 0;
	if (tokenA) { tokens[length] = tokenA; length++; }
	if (tokenB) { tokens[length] = tokenB; length++; }
	if (tokenC) { tokens[length] = tokenC; length++; }
	if (tokenD) { tokens[length] = tokenD; length++; }
	Delimiter *const delimiter = new Delimiter(blank, empty, tokens, length);
	return newDelimiter(delimiter);
}

LPWSTR * OSArgs::getUnparsedArgs(int *const length)
{
	length[0] = 0;
	int capacity = 0;
	if (unparsedArgs)
	{
		delete[] unparsedArgs;
		unparsedArgs = NULL;
	}
	for (int i = 0; i < this->length; i++)
	{
		if (!parsed[i])
		{
			if (unparsedArgs == NULL)
			{
				unparsedArgs = new LPWSTR[4];
				capacity = 4;
			}
			else if (length[0] == capacity)
			{
				capacity = capacity * 2;
				LPWSTR *const unparsedArgsNew = new LPWSTR[capacity];
				memcpy(unparsedArgsNew, unparsedArgs, sizeof(LPWSTR) * (size_t)length[0]);
				delete[] unparsedArgs;
				unparsedArgs = unparsedArgsNew;
			}
			unparsedArgs[length[0]] = values[i];
			length[0]++;
		}
	}
	return unparsedArgs;
}

OSArgs::Result * OSArgs::parse(LPCWSTR *const flags, const int length)
{
	Result *result = newResult(new Result());
	if (this->length > 0)
	{
		for (int i = 0; i < this->length && !parsed[i]; i++)
		{
			LPWSTR const value = values[i];
			for (int j = 0; j < length; j++)
			{
				LPCWSTR const flag = flags[j];
				if (!wcscmp(value, flag))
				{
					result->add(value, lengths[i]);
					parsed[i] = TRUE;
					break;
				}
			}
		}
	}
	return result;
}

OSArgs::Result * OSArgs::parse(LPCWSTR const flagA, LPCWSTR const flagB, LPCWSTR const flagC, LPCWSTR const flagD, LPCWSTR const flagE, LPCWSTR const flagF, LPCWSTR const flagG, LPCWSTR const flagH)
{
	if (this->length > 0)
	{
		LPCWSTR flags[8];
		int length = 0;
		if (flagA) { flags[length] = flagA; length++; }
		if (flagB) { flags[length] = flagB; length++; }
		if (flagC) { flags[length] = flagC; length++; }
		if (flagD) { flags[length] = flagD; length++; }
		if (flagE) { flags[length] = flagE; length++; }
		if (flagF) { flags[length] = flagF; length++; }
		if (flagG) { flags[length] = flagG; length++; }
		if (flagH) { flags[length] = flagH; length++; }
		return parse(flags, length);
	}
	return newResult(new Result());
}

OSArgs::Result * OSArgs::parse(LPCSTR *const flags, const int length)
{
	if (this->length > 0 && length > 0)
	{
		LPWSTR *const flagsW = new LPWSTR[length];
		for (int i = 0; i < length; i++)
		{
			LPCSTR const flag = flags[i];
			const int flagLen0 = strlen(flag) + 1;
			LPWSTR flagW = new WCHAR[flagLen0];
			MultiByteToWideChar(CP_UTF8, 0, flag, flagLen0, flagW, flagLen0);
			flagsW[i] = flagW;
		}
		Result *const result = parse((LPCWSTR*)flagsW, length);
		for (int i = 0; i < length; i++)
			delete[] flagsW[i];
		delete[] flagsW;
		return result;
	}
	return newResult(new Result());
}

OSArgs::Result * OSArgs::parse(LPCSTR const flagA, LPCSTR const flagB, LPCSTR const flagC, LPCSTR const flagD, LPCSTR const flagE, LPCSTR const flagF, LPCSTR const flagG, LPCSTR const flagH)
{
	if (this->length > 0)
	{
		LPCSTR flags[8];
		int length = 0;
		if (flagA) { flags[length] = flagA; length++; }
		if (flagB) { flags[length] = flagB; length++; }
		if (flagC) { flags[length] = flagC; length++; }
		if (flagD) { flags[length] = flagD; length++; }
		if (flagE) { flags[length] = flagE; length++; }
		if (flagF) { flags[length] = flagF; length++; }
		if (flagG) { flags[length] = flagG; length++; }
		if (flagH) { flags[length] = flagH; length++; }
		return parse(flags, length);
	}
	return newResult(new Result());
}

OSArgs::Result * OSArgs::parsePairs(Delimiter *const delimiter, LPCWSTR *const flags, const int length)
{
	if (this->length > 0)
	{
		if (delimiter->isBlank())
			return parsePairsWithBlank(delimiter, flags, length);
		return parsePairsWithoutBlank(delimiter, flags, length);
	}
	return newResult(new Result());
}

OSArgs::Result * OSArgs::parsePairs(Delimiter *const delimiter, LPCWSTR const flagA, LPCWSTR const flagB, LPCWSTR const flagC, LPCWSTR const flagD, LPCWSTR const flagE, LPCWSTR const flagF, LPCWSTR const flagG, LPCWSTR const flagH)
{
	LPCWSTR flags[8];
	int length = 0;
	if (flagA) { flags[length] = flagA; length++; }
	if (flagB) { flags[length] = flagB; length++; }
	if (flagC) { flags[length] = flagC; length++; }
	if (flagD) { flags[length] = flagD; length++; }
	if (flagE) { flags[length] = flagE; length++; }
	if (flagF) { flags[length] = flagF; length++; }
	if (flagG) { flags[length] = flagG; length++; }
	if (flagH) { flags[length] = flagH; length++; }
	return parsePairs(delimiter, flags, length);
}

OSArgs::Result * OSArgs::parsePairs(Delimiter *const delimiter, LPCSTR *const flags, const int length)
{
	if (this->length > 0 && length > 0)
	{
		LPWSTR *const flagsW = new LPWSTR[length];
		for (int i = 0; i < length; i++)
		{
			LPCSTR const flag = flags[i];
			const int flagLen0 = strlen(flag) + 1;
			LPWSTR flagW = new WCHAR[flagLen0];
			MultiByteToWideChar(CP_UTF8, 0, flag, flagLen0, flagW, flagLen0);
			flagsW[i] = flagW;
		}
		Result *const result = parsePairs(delimiter, (LPCWSTR*)flagsW, length);
		for (int i = 0; i < length; i++)
			delete[] flagsW[i];
		delete[] flagsW;
		return result;
	}
	return newResult(new Result());
}

OSArgs::Result * OSArgs::parsePairs(Delimiter *const delimiter, LPCSTR const flagA, LPCSTR const flagB, LPCSTR const flagC, LPCSTR const flagD, LPCSTR const flagE, LPCSTR const flagF, LPCSTR const flagG, LPCSTR const flagH)
{
	if (this->length > 0)
	{
		LPCSTR flags[8];
		int length = 0;
		if (flagA) { flags[length] = flagA; length++; }
		if (flagB) { flags[length] = flagB; length++; }
		if (flagC) { flags[length] = flagC; length++; }
		if (flagD) { flags[length] = flagD; length++; }
		if (flagE) { flags[length] = flagE; length++; }
		if (flagF) { flags[length] = flagF; length++; }
		if (flagG) { flags[length] = flagG; length++; }
		if (flagH) { flags[length] = flagH; length++; }
		return parsePairs(delimiter, flags, length);
	}
	return newResult(new Result());
}

void OSArgs::init(LPCWSTR *const argv, const int argc)
{
	if (argv && argc > 0)
	{
		copyStrsW(argv, argc, &values, &lengths, &length);
		parsed = new BOOL[argc];
		memset(parsed, FALSE, sizeof(BOOL) * argc);
	}
}

void OSArgs::init(LPCSTR *const argv, const int argc)
{
	if (argv && argc > 0)
	{
		copyStrs(argv, argc, &values, &lengths, &length);
		parsed = new BOOL[argc];
		memset(parsed, FALSE, sizeof(BOOL) * argc);
	}
}

OSArgs::Result * OSArgs::newResult(OSArgs::Result *const result)
{
	ensureResultsCapacity();
	results[resultsLen] = result;
	resultsLen++;
	return result;
}

OSArgs::Delimiter * OSArgs::newDelimiter(OSArgs::Delimiter *const delimiter)
{
	ensureDelimitersCapacity();
	delimiters[delimitersLen] = delimiter;
	delimitersLen++;
	return delimiter;
}

OSArgs::Result * OSArgs::parsePairsWithBlank(Delimiter *const delimiter, LPCWSTR *const flags, const int length)
{
	Result *result = newResult(new Result());
	for (int i = 0; i < this->length && !parsed[i]; i++)
	{
		LPWSTR const value = values[i];
		for (int j = 0; j < length; j++)
		{
			LPCWSTR const flag = flags[j];
			const int prefixLength = hasPrefix(value, flag);
			if (prefixLength >= 0)
			{
				if (lengths[i] == prefixLength)
				{
					parsed[i] = TRUE;
					if (i + 1 < this->length)
					{
						parsed[i + 1] = TRUE;
						result->add(values[i + 1], lengths[i + 1]);
						i++;
					}
					else
					{
						result->add(&value[prefixLength], 0);
					}
					break;
				}
				else
				{
					LPWSTR const valueWithoutFlag = &value[prefixLength];
					const int tokenLength = delimiter->getMatchingTokenLength(valueWithoutFlag);
					if (tokenLength >= 0)
					{
						parsed[i] = TRUE;
						result->add(&valueWithoutFlag[tokenLength]);
						break;
					}
				}
			}
		}
	}
	return result;
}

OSArgs::Result * OSArgs::parsePairsWithoutBlank(Delimiter *const delimiter, LPCWSTR *const flags, const int length)
{
	Result *result = newResult(new Result());
	for (int i = 0; i < this->length && !parsed[i]; i++)
	{
		LPWSTR const value = values[i];
		for (int j = 0; j < length; j++)
		{
			LPCWSTR const flag = flags[j];
			const int prefixLength = hasPrefix(value, flag);
			if (prefixLength >= 0)
			{
				if (lengths[i] == prefixLength)
				{
					parsed[i] = TRUE;
					result->add(&value[prefixLength], 0);
					break;
				}
				else
				{
					LPWSTR const valueWithoutFlag = &value[prefixLength];
					const int tokenLength = delimiter->getMatchingTokenLength(valueWithoutFlag);
					if (tokenLength >= 0)
					{
						parsed[i] = TRUE;
						result->add(&valueWithoutFlag[tokenLength]);
						break;
					}
				}
			}
		}
	}
	return result;
}

void OSArgs::ensureResultsCapacity()
{
	if (results == NULL)
	{
		results = new Result*[8];
		resultsCap = 8;
	}
	else if (resultsLen == resultsCap)
	{
		resultsCap = resultsCap * 2;
		Result **const resultsNew = new Result*[resultsCap];
		memcpy(resultsNew, results, sizeof(Result*) * resultsLen);
		delete[] results;
		results = resultsNew;
	}
}

void OSArgs::ensureDelimitersCapacity()
{
	if (delimiters == NULL)
	{
		delimiters = new Delimiter*[1];
		delimitersCap = 1;
	}
	else if (delimitersLen == delimitersCap)
	{
		delimitersCap = delimitersCap * 2;
		Delimiter **const delimitersNew = new Delimiter*[delimitersCap];
		memcpy(delimitersNew, delimiters, sizeof(Delimiter*) * delimitersLen);
		delete[] delimiters;
		delimiters = delimitersNew;
	}
}
