#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class OSArgs
{
public:
	class Result
	{
	public:
		Result();
		~Result();
		BOOL isAvailable();
		LPWSTR *getValues();
		int getLength();
		Result *add(LPCWSTR value);
		Result *add(LPCWSTR value, int length);
	private:
		LPWSTR *values;
		int length;
		int capacity;
		void ensureCapacity();
	};
	class Delimiter
	{
	public:
		Delimiter(BOOL blank, BOOL empty, LPCWSTR *tokens, int length);
		Delimiter(BOOL blank, BOOL empty, LPCSTR *tokens, int length);
		~Delimiter();
		BOOL isBlank();
		int getMatchingTokenLength(LPCWSTR str);
	private:
		BOOL blank;
		BOOL empty;
		LPWSTR *tokens;
		int length;
	};
	OSArgs();
	OSArgs(LPCWSTR *argv, int argc);
	OSArgs(LPCSTR *argv, int argc);
	~OSArgs();
	Result *parse(LPCWSTR *flags, int length);
	Result *parse(LPCWSTR flagA = NULL, LPCWSTR flagB = NULL, LPCWSTR flagC = NULL, LPCWSTR flagD = NULL, LPCWSTR flagE = NULL, LPCWSTR flagF = NULL, LPCWSTR flagG = NULL, LPCWSTR flagH = NULL);
	Result *parse(LPCSTR *flags, int length);
	Result *parse(LPCSTR flagA = NULL, LPCSTR flagB = NULL, LPCSTR flagC = NULL, LPCSTR flagD = NULL, LPCSTR flagE = NULL, LPCSTR flagF = NULL, LPCSTR flagG = NULL, LPCSTR flagH = NULL);
	Result *parsePairs(Delimiter *delimiter, LPCWSTR *flags, int length);
	Result *parsePairs(Delimiter *delimiter, LPCWSTR flagA = NULL, LPCWSTR flagB = NULL, LPCWSTR flagC = NULL, LPCWSTR flagD = NULL, LPCWSTR flagE = NULL, LPCWSTR flagF = NULL, LPCWSTR flagG = NULL, LPCWSTR flagH = NULL);
	Result *parsePairs(Delimiter *delimiter, LPCSTR *flags, int length);
	Result *parsePairs(Delimiter *delimiter, LPCSTR flagA = NULL, LPCSTR flagB = NULL, LPCSTR flagC = NULL, LPCSTR flagD = NULL, LPCSTR flagE = NULL, LPCSTR flagF = NULL, LPCSTR flagG = NULL, LPCSTR flagH = NULL);
	Delimiter *newDelimiter(BOOL blank, BOOL empty, LPCWSTR *tokens, int length);
	Delimiter *newDelimiter(BOOL blank, BOOL empty, LPCWSTR tokenA = NULL, LPCWSTR tokenB = NULL, LPCWSTR tokenC = NULL, LPCWSTR tokenD = NULL);
	Delimiter *newDelimiter(BOOL blank, BOOL empty, LPCSTR *tokens, int length);
	Delimiter *newDelimiter(BOOL blank, BOOL empty, LPCSTR tokenA = NULL, LPCSTR tokenB = NULL, LPCSTR tokenC = NULL, LPCSTR tokenD = NULL);
	LPWSTR *getUnparsedArgs(int *length);
private:
	LPWSTR *values;
	int *lengths;
	BOOL *parsed;
	int length;
	LPWSTR *unparsedArgs;
	Result **results;
	int resultsLen;
	int resultsCap;
	Delimiter **delimiters;
	int delimitersLen;
	int delimitersCap;
	void init(LPCWSTR *argv, int argc);
	void init(LPCSTR *argv, int argc);
	Result *newResult(Result *result);
	Delimiter *newDelimiter(Delimiter *delimiter);
	Result *parsePairsWithBlank(Delimiter *delimiter, LPCWSTR *flags, int length);
	Result *parsePairsWithoutBlank(Delimiter *delimiter, LPCWSTR *flags, int length);
	void ensureResultsCapacity();
	void ensureDelimitersCapacity();
};
