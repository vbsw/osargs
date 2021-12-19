/*
 *          Copyright 2021, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include "osargsc.h"
#include <stdlib.h>
#include <shellapi.h>

static void copy_strs_a(LPCSTR *const srcStrings, const int srcLength, LPWSTR **strings, int **lengths, int *length)
{
	int totalLen = 0;
	strings[0] = (LPWSTR*)malloc(sizeof(LPWSTR) * srcLength);
	lengths[0] = (int*)malloc(sizeof(int) * srcLength);
	length[0] = srcLength;
	for (int i = 0; i < srcLength; i++)
	{
		const int length = (int)strlen(srcStrings[i]);
		lengths[0][i] = length;
		totalLen += length + 1;
	}
	LPWSTR const srcStringsCopy = (LPWSTR)malloc(sizeof(WCHAR) * totalLen);
	for (int offset = 0, i = 0; i < srcLength; i++)
	{
		MultiByteToWideChar(CP_UTF8, 0, srcStrings[i], lengths[0][i] + 1, &srcStringsCopy[offset], lengths[0][i] + 1);
		strings[0][i] = &srcStringsCopy[offset];
		offset += lengths[0][i] + 1;
	}
}

static void copy_strs_w(LPCWSTR *const srcStrings, const int srcLength, LPWSTR **strings, int **lengths, int *length)
{
	int totalLen = 0;
	strings[0] = (LPWSTR*)malloc(sizeof(LPWSTR) * srcLength);
	lengths[0] = (int*)malloc(sizeof(int) * srcLength);
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
	LPWSTR const srcStringsCopy = (LPWSTR)malloc(sizeof(WCHAR) * totalLen);
	for (int offset = 0, i = 0; i < srcLength; i++)
	{
		memcpy(&srcStringsCopy[offset], srcStrings[i], sizeof(WCHAR) * (size_t)(lengths[0][i] + 1));
		strings[0][i] = &srcStringsCopy[offset];
		offset += lengths[0][i] + 1;
	}
}

static int has_prefix(LPCWSTR const str, LPCWSTR const prefix)
{
	int i = 0;
	while (str[i] == prefix[i])
		i++;
	if (prefix[i] == 0)
		return i;
	return -1;
}

static void result_ensure_capacity(osargs_result_t *const result)
{
	if (result->values == NULL)
	{
		result->values = (LPWSTR*)malloc(sizeof(LPWSTR));
		result->capacity = 1;
	}
	else if (result->length == result->capacity)
	{
		result->capacity *= 2;
		LPWSTR *const valuesNew = (LPWSTR*)malloc(sizeof(LPWSTR) * result->capacity);
		memcpy(valuesNew, result->values, sizeof(LPWSTR) * result->length);
		free(result->values);
		result->values = valuesNew;
	}
}

static osargs_delimiter_t *delimiter_new(const BOOL blank, const BOOL empty)
{
	osargs_delimiter_t *const delimiter = (osargs_delimiter_t*)malloc(sizeof(osargs_delimiter_t));
	delimiter->blank = blank;
	delimiter->empty = empty;
	delimiter->tokens = NULL;
	delimiter->length = 0;
	return delimiter;
}

static int delimiter_has_prefix(LPCWSTR const str, LPCWSTR const prefix)
{
	int i = 0;
	while (str[i] == prefix[i])
		i++;
	if (prefix[i] == 0)
		return i;
	return -1;
}

static void osargs_init_a(osargs_t *const osargs, LPCSTR *const argv, const int argc)
{
	if (argv && argc > 0)
	{
		copy_strs_a(argv, argc, &osargs->values, &osargs->lengths, &osargs->length);
		osargs->parsed = (BOOL*)malloc(sizeof(BOOL) * argc);
		memset(osargs->parsed, FALSE, sizeof(BOOL) * argc);
	}
}

static void osargs_init_w(osargs_t *const osargs, LPCWSTR *const argv, const int argc)
{
	if (argv && argc > 0)
	{
		copy_strs_w(argv, argc, &osargs->values, &osargs->lengths, &osargs->length);
		osargs->parsed = (BOOL*)malloc(sizeof(BOOL) * argc);
		memset(osargs->parsed, FALSE, sizeof(BOOL) * argc);
	}
}

static void osargs_ensure_results_capacity(osargs_t *const osargs)
{
	if (osargs->results == NULL)
	{
		osargs->results = (osargs_result_t**)malloc(sizeof(osargs_result_t*) * 8);
		osargs->resultsCap = 8;
	}
	else if (osargs->resultsLen == osargs->resultsCap)
	{
		osargs->resultsCap *= 2;
		osargs_result_t **const resultsNew = (osargs_result_t**)malloc(sizeof(osargs_result_t*) * osargs->resultsCap);
		memcpy(resultsNew, osargs->results, sizeof(osargs_result_t*) * osargs->resultsLen);
		free(osargs->results);
		osargs->results = resultsNew;
	}
}

static void osargs_ensure_delimiters_capacity(osargs_t *const osargs)
{
	if (osargs->delimiters == NULL)
	{
		osargs->delimiters = (osargs_delimiter_t**)malloc(sizeof(osargs_delimiter_t*) * 1);
		osargs->delimitersCap = 1;
	}
	else if (osargs->delimitersLen == osargs->delimitersCap)
	{
		osargs->delimitersCap *= 2;
		osargs_delimiter_t **const delimitersNew = (osargs_delimiter_t**)malloc(sizeof(osargs_delimiter_t*) * osargs->delimitersCap);
		memcpy(delimitersNew, osargs->delimiters, sizeof(osargs_delimiter_t*) * osargs->delimitersLen);
		free(osargs->delimiters);
		osargs->delimiters = delimitersNew;
	}
}

static osargs_result_t *osargs_add_result(osargs_t *const osargs, osargs_result_t *const result)
{
	osargs_ensure_results_capacity(osargs);
	osargs->results[osargs->resultsLen] = result;
	osargs->resultsLen++;
	return result;
}

static osargs_result_t *osargs_result_new()
{
	osargs_result_t *const result = (osargs_result_t*)malloc(sizeof(osargs_result_t));
	ZeroMemory(result, sizeof(osargs_result_t));
	return result;
}

static void osargs_result_destroy(osargs_result_t *const result)
{
	if (result->values)
	{
		int i;
		for (i = 0; i < result->length; i++)
			free(result->values[i]);
		free(result->values);
	}
	free(result);
}

static osargs_delimiter_t *delimiter_new_a(const BOOL blank, const BOOL empty, LPCSTR *const tokens, const int length)
{
	osargs_delimiter_t *const delimiter = delimiter_new(blank, empty);
	if (length > 0)
	{
		int *lengths = NULL;
		copy_strs_a(tokens, length, &delimiter->tokens, &lengths, &delimiter->length);
		if (lengths)
			free(lengths);
	}
	return delimiter;
}

static osargs_delimiter_t *delimiter_new_w(const BOOL blank, const BOOL empty, LPCWSTR *const tokens, const int length)
{
	osargs_delimiter_t *const delimiter = delimiter_new(blank, empty);
	if (length > 0)
	{
		int *lengths = NULL;
		copy_strs_w(tokens, length, &delimiter->tokens, &lengths, &delimiter->length);
		if (lengths)
			free(lengths);
	}
	return delimiter;
}

static void osargs_delimiter_destroy(osargs_delimiter_t *const delimiter)
{
	if (delimiter->tokens)
	{
		free(delimiter->tokens[0]);
		free(delimiter->tokens);
	}
	free(delimiter);
}

static void osargs_delimiter_add(osargs_t *osargs, osargs_delimiter_t *delimiter)
{
	osargs_ensure_delimiters_capacity(osargs);
	osargs->delimiters[osargs->delimitersLen] = delimiter;
	osargs->delimitersLen++;
}

void osargs_result_add(osargs_result_t *const result, LPCWSTR const value)
{
	const int length = (int)wcslen(value);
	return osargs_result_add_len(result, value, length);
}

void osargs_result_add_len(osargs_result_t *const result, LPCWSTR const value, const int length)
{
	result_ensure_capacity(result);
	LPWSTR valueNew = (LPWSTR)malloc(sizeof(WCHAR) * (length + 1));
	memcpy(valueNew, value, sizeof(WCHAR) * (size_t)(length + 1));
	result->values[result->length] = valueNew;
	result->length++;
}

int osargs_delimiter_get_matching_token_length(osargs_delimiter_t *const delimiter, LPCWSTR const str)
{
	for (int i = 0; i < delimiter->length; i++)
	{
		LPCWSTR const token = delimiter->tokens[i];
		const int prefixLen = delimiter_has_prefix(str, token);
		if (prefixLen >= 0)
			return prefixLen;
	}
	if (delimiter->empty)
		return 0;
	return -1;
}

static osargs_result_t *osargs_parse_pairs_with_blank(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR *flags, int length)
{
	osargs_result_t *const result = osargs_add_result(osargs, osargs_result_new());
	for (int i = 0; i < osargs->length && !osargs->parsed[i]; i++)
	{
		LPWSTR const value = osargs->values[i];
		for (int j = 0; j < length; j++)
		{
			LPCWSTR const flag = flags[j];
			const int prefixLength = has_prefix(value, flag);
			if (prefixLength >= 0)
			{
				if (osargs->lengths[i] == prefixLength)
				{
					osargs->parsed[i] = TRUE;
					if (i + 1 < osargs->length)
					{
						osargs->parsed[i + 1] = TRUE;
						osargs_result_add_len(result, osargs->values[i + 1], osargs->lengths[i + 1]);
						i++;
					}
					else
					{
						osargs_result_add_len(result, &value[prefixLength], 0);
					}
					break;
				}
				else
				{
					LPWSTR const valueWithoutFlag = &value[prefixLength];
					const int tokenLength = osargs_delimiter_get_matching_token_length(delimiter, valueWithoutFlag);
					if (tokenLength >= 0)
					{
						osargs->parsed[i] = TRUE;
						osargs_result_add(result, &valueWithoutFlag[tokenLength]);
						break;
					}
				}
			}
		}
	}
	return result;
}

static osargs_result_t *osargs_parse_pairs_without_blank(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR *flags, int length)
{
	osargs_result_t *const result = osargs_add_result(osargs, osargs_result_new());
	for (int i = 0; i < osargs->length && !osargs->parsed[i]; i++)
	{
		LPWSTR const value = osargs->values[i];
		for (int j = 0; j < length; j++)
		{
			LPCWSTR const flag = flags[j];
			const int prefixLength = has_prefix(value, flag);
			if (prefixLength >= 0)
			{
				if (osargs->lengths[i] == prefixLength)
				{
					osargs->parsed[i] = TRUE;
					osargs_result_add_len(result, &value[prefixLength], 0);
					break;
				}
				else
				{
					LPWSTR const valueWithoutFlag = &value[prefixLength];
					const int tokenLength = osargs_delimiter_get_matching_token_length(delimiter, valueWithoutFlag);
					if (tokenLength >= 0)
					{
						osargs->parsed[i] = TRUE;
						osargs_result_add(result, &valueWithoutFlag[tokenLength]);
						break;
					}
				}
			}
		}
	}
	return result;
}

osargs_t *osargs_new()
{
	osargs_t *const osargs = (osargs_t*)malloc(sizeof(osargs_t));
	ZeroMemory(osargs, sizeof(osargs_t));
	int argc;
	LPCTSTR const cmdl = GetCommandLine();
	#ifdef UNICODE
	LPWSTR *const argv = CommandLineToArgvW(cmdl, &argc);
	#else
	const int cmdlLen = (int)strlen(cmdl) + 1;
	LPWSTR const cmdlW = (LPWSTR)malloc(sizeof(WCHAR) * cmdlLen);
	MultiByteToWideChar(CP_UTF8, 0, cmdl, cmdlLen, cmdlW, cmdlLen);
	LPWSTR *const argv = CommandLineToArgvW(cmdlW, &argc);
	delete[] cmdlW;
	#endif // !UNICODE
	if (argc > 1)
		osargs_init_w(osargs, (LPCWSTR*)&argv[1], argc - 1);
	LocalFree(argv);
	return osargs;
}

osargs_t *osargs_new_a(LPCSTR *const argv, const int argc)
{
	osargs_t *const osargs = (osargs_t*)malloc(sizeof(osargs_t));
	ZeroMemory(osargs, sizeof(osargs_t));
	osargs_init_a(osargs, argv, argc);
	return osargs;
}

osargs_t *osargs_new_w(LPCWSTR *const argv, const int argc)
{
	osargs_t *const osargs = (osargs_t*)malloc(sizeof(osargs_t));
	ZeroMemory(osargs, sizeof(osargs_t));
	osargs_init_w(osargs, argv, argc);
	return osargs;
}

void osargs_destroy(osargs_t *const osargs)
{
	if (osargs->values)
	{
		free(osargs->values[0]);
		free(osargs->values);
	}
	if (osargs->lengths)
		free(osargs->lengths);
	if (osargs->results)
	{
		for (int i = 0; i < osargs->resultsLen; i++)
			osargs_result_destroy(osargs->results[i]);
		free(osargs->results);
	}
	if (osargs->delimiters)
	{
		for (int i = 0; i < osargs->delimitersLen; i++)
			osargs_delimiter_destroy(osargs->delimiters[i]);
		free(osargs->delimiters);
	}
	if (osargs->unparsedArgs)
		free(osargs->unparsedArgs);
}

osargs_result_t *osargs_parse_a(osargs_t *osargs, LPCSTR *flags, int length)
{
	if (osargs->length > 0 && length > 0)
	{
		LPWSTR *const flagsW = (LPWSTR*)malloc(sizeof(LPWSTR) * (size_t)length);
		for (int i = 0; i < length; i++)
		{
			LPCSTR const flag = flags[i];
			const int flagLen0 = (int)strlen(flag) + 1;
			LPWSTR flagW = (LPWSTR)malloc(sizeof(WCHAR) * (size_t)flagLen0);
			MultiByteToWideChar(CP_UTF8, 0, flag, flagLen0, flagW, flagLen0);
			flagsW[i] = flagW;
		}
		osargs_result_t *const result = osargs_parse_w(osargs, (LPCWSTR*)flagsW, length);
		for (int i = 0; i < length; i++)
			free(flagsW[i]);
		free(flagsW);
		return result;
	}
	return osargs_add_result(osargs, osargs_result_new());
}

osargs_result_t *osargs_parse_a1(osargs_t *osargs, LPCSTR flagA) {
	if (osargs->length > 0) { LPCSTR flags[1] = { flagA }; return osargs_parse_a(osargs, flags, 1); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_a2(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB) {
	if (osargs->length > 0) { LPCSTR flags[2] = { flagA, flagB }; return osargs_parse_a(osargs, flags, 2); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_a3(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC) {
	if (osargs->length > 0) { LPCSTR flags[3] = { flagA, flagB, flagC }; return osargs_parse_a(osargs, flags, 3); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_a4(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD) {
	if (osargs->length > 0) { LPCSTR flags[4] = { flagA, flagB, flagC, flagD }; return osargs_parse_a(osargs, flags, 4); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_a5(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE) {
	if (osargs->length > 0) { LPCSTR flags[5] = { flagA, flagB, flagC, flagD, flagE }; return osargs_parse_a(osargs, flags, 5); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_a6(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE, LPCSTR flagF) {
	if (osargs->length > 0) { LPCSTR flags[6] = { flagA, flagB, flagC, flagD, flagE, flagF }; return osargs_parse_a(osargs, flags, 6); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_a7(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE, LPCSTR flagF, LPCSTR flagG) {
	if (osargs->length > 0) { LPCSTR flags[7] = { flagA, flagB, flagC, flagD, flagE, flagF, flagG }; return osargs_parse_a(osargs, flags, 7); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_a8(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE, LPCSTR flagF, LPCSTR flagG, LPCSTR flagH) {
	if (osargs->length > 0) { LPCSTR flags[8] = { flagA, flagB, flagC, flagD, flagE, flagF, flagG }; return osargs_parse_a(osargs, flags, 8); }
	return osargs_add_result(osargs, osargs_result_new());
}

osargs_result_t *osargs_parse_w(osargs_t *osargs, LPCWSTR *flags, int length)
{
	osargs_result_t *const result = osargs_add_result(osargs, osargs_result_new());
	if (osargs->length > 0)
	{
		for (int i = 0; i < osargs->length && !osargs->parsed[i]; i++)
		{
			LPWSTR const value = osargs->values[i];
			for (int j = 0; j < length; j++)
			{
				LPCWSTR const flag = flags[j];
				if (!wcscmp(value, flag))
				{
					osargs_result_add_len(result, value, osargs->lengths[i]);
					osargs->parsed[i] = TRUE;
					break;
				}
			}
		}
	}
	return result;
}

osargs_result_t *osargs_parse_w1(osargs_t *osargs, LPCWSTR flagA) {
	if (osargs->length > 0) { LPCWSTR flags[1] = { flagA }; return osargs_parse_w(osargs, flags, 1); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_w2(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB) {
	if (osargs->length > 0) { LPCWSTR flags[2] = { flagA, flagB }; return osargs_parse_w(osargs, flags, 2); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_w3(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC) {
	if (osargs->length > 0) { LPCWSTR flags[3] = { flagA, flagB, flagC }; return osargs_parse_w(osargs, flags, 3); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_w4(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD) {
	if (osargs->length > 0) { LPCWSTR flags[4] = { flagA, flagB, flagC, flagD }; return osargs_parse_w(osargs, flags, 4); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_w5(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE) {
	if (osargs->length > 0) { LPCWSTR flags[5] = { flagA, flagB, flagC, flagD, flagE }; return osargs_parse_w(osargs, flags, 5); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_w6(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE, LPCWSTR flagF) {
	if (osargs->length > 0) { LPCWSTR flags[6] = { flagA, flagB, flagC, flagD, flagE, flagF }; return osargs_parse_w(osargs, flags, 6); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_w7(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE, LPCWSTR flagF, LPCWSTR flagG) {
	if (osargs->length > 0) { LPCWSTR flags[7] = { flagA, flagB, flagC, flagD, flagE, flagF, flagG }; return osargs_parse_w(osargs, flags, 7); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_w8(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE, LPCWSTR flagF, LPCWSTR flagG, LPCWSTR flagH) {
	if (osargs->length > 0) { LPCWSTR flags[8] = { flagA, flagB, flagC, flagD, flagE, flagF, flagG, flagH }; return osargs_parse_w(osargs, flags, 8); }
	return osargs_add_result(osargs, osargs_result_new());
}

osargs_result_t *osargs_parse_pairs_a(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR *flags, int length)
{
	if (osargs->length > 0 && length > 0)
	{
		LPWSTR *const flagsW = new LPWSTR[length];
		for (int i = 0; i < length; i++)
		{
			LPCSTR const flag = flags[i];
			const int flagLen0 = (int)strlen(flag) + 1;
			LPWSTR flagW = new WCHAR[flagLen0];
			MultiByteToWideChar(CP_UTF8, 0, flag, flagLen0, flagW, flagLen0);
			flagsW[i] = flagW;
		}
		osargs_result_t *const result = osargs_parse_pairs_w(osargs, delimiter, (LPCWSTR*)flagsW, length);
		for (int i = 0; i < length; i++)
			delete[] flagsW[i];
		delete[] flagsW;
		return result;
	}
	return osargs_add_result(osargs, osargs_result_new());
}

osargs_result_t *osargs_parse_pairs_a1(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA) {
	if (osargs->length > 0) { LPCSTR flags[1] = { flagA }; return osargs_parse_pairs_a(osargs, delimiter, flags, 1); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_a2(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB) {
	if (osargs->length > 0) { LPCSTR flags[2] = { flagA, flagB }; return osargs_parse_pairs_a(osargs, delimiter, flags, 2); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_a3(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC) {
	if (osargs->length > 0) { LPCSTR flags[3] = { flagA, flagB, flagC }; return osargs_parse_pairs_a(osargs, delimiter, flags, 3); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_a4(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD) {
	if (osargs->length > 0) { LPCSTR flags[4] = { flagA, flagB, flagC, flagD }; return osargs_parse_pairs_a(osargs, delimiter, flags, 4); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_a5(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE) {
	if (osargs->length > 0) { LPCSTR flags[5] = { flagA, flagB, flagC, flagD, flagE }; return osargs_parse_pairs_a(osargs, delimiter, flags, 5); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_a6(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE, LPCSTR flagF) {
	if (osargs->length > 0) { LPCSTR flags[6] = { flagA, flagB, flagC, flagD, flagE, flagF }; return osargs_parse_pairs_a(osargs, delimiter, flags, 6); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_a7(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE, LPCSTR flagF, LPCSTR flagG) {
	if (osargs->length > 0) { LPCSTR flags[7] = { flagA, flagB, flagC, flagD, flagE, flagF, flagG }; return osargs_parse_pairs_a(osargs, delimiter, flags, 7); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_a8(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE, LPCSTR flagF, LPCSTR flagG, LPCSTR flagH) {
	if (osargs->length > 0) { LPCSTR flags[8] = { flagA, flagB, flagC, flagD, flagE, flagF, flagG, flagH }; return osargs_parse_pairs_a(osargs, delimiter, flags, 8); }
	return osargs_add_result(osargs, osargs_result_new());
}

osargs_result_t *osargs_parse_pairs_w(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR *flags, int length)
{
	if (osargs->length > 0)
	{
		if (delimiter->blank)
			return osargs_parse_pairs_with_blank(osargs, delimiter, flags, length);
		return osargs_parse_pairs_without_blank(osargs, delimiter, flags, length);
	}
	return osargs_add_result(osargs, osargs_result_new());
}

osargs_result_t *osargs_parse_pairs_w1(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA) {
	if (osargs->length > 0) { LPCWSTR flags[1] = { flagA }; return osargs_parse_pairs_w(osargs, delimiter, flags, 1); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_w2(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB) {
	if (osargs->length > 0) { LPCWSTR flags[2] = { flagA, flagB }; return osargs_parse_pairs_w(osargs, delimiter, flags, 2); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_w3(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC) {
	if (osargs->length > 0) { LPCWSTR flags[3] = { flagA, flagB, flagC }; return osargs_parse_pairs_w(osargs, delimiter, flags, 3); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_w4(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD) {
	if (osargs->length > 0) { LPCWSTR flags[4] = { flagA, flagB, flagC, flagD }; return osargs_parse_pairs_w(osargs, delimiter, flags, 4); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_w5(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE) {
	if (osargs->length > 0) { LPCWSTR flags[5] = { flagA, flagB, flagC, flagD, flagE }; return osargs_parse_pairs_w(osargs, delimiter, flags, 5); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_w6(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE, LPCWSTR flagF) {
	if (osargs->length > 0) { LPCWSTR flags[6] = { flagA, flagB, flagC, flagD, flagE, flagF }; return osargs_parse_pairs_w(osargs, delimiter, flags, 6); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_w7(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE, LPCWSTR flagF, LPCWSTR flagG) {
	if (osargs->length > 0) { LPCWSTR flags[7] = { flagA, flagB, flagC, flagD, flagE, flagF, flagG }; return osargs_parse_pairs_w(osargs, delimiter, flags, 7); }
	return osargs_add_result(osargs, osargs_result_new());
}
osargs_result_t *osargs_parse_pairs_w8(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE, LPCWSTR flagF, LPCWSTR flagG, LPCWSTR flagH) {
	if (osargs->length > 0) { LPCWSTR flags[8] = { flagA, flagB, flagC, flagD, flagE, flagF, flagG, flagH }; return osargs_parse_pairs_w(osargs, delimiter, flags, 8); }
	return osargs_add_result(osargs, osargs_result_new());
}

osargs_delimiter_t *osargs_delimiter_new_a(osargs_t *osargs, BOOL blank, BOOL empty, LPCSTR *tokens, int length)
{
	osargs_delimiter_t *const delimiter = delimiter_new_a(blank, empty, tokens, length);
	osargs_delimiter_add(osargs, delimiter);
	return delimiter;
}

osargs_delimiter_t *osargs_delimiter_new_a1(osargs_t *osargs, BOOL blank, BOOL empty, LPCSTR tokenA) {
	LPCSTR tokens[1]; int length = 0;
	if (tokenA) { tokens[length] = tokenA; length++; }
	return osargs_delimiter_new_a(osargs, blank, empty, tokens, length);
}
osargs_delimiter_t *osargs_delimiter_new_a2(osargs_t *osargs, BOOL blank, BOOL empty, LPCSTR tokenA, LPCSTR tokenB) {
	LPCSTR tokens[2]; int length = 0;
	if (tokenA) { tokens[length] = tokenA; length++; }
	if (tokenB) { tokens[length] = tokenB; length++; }
	return osargs_delimiter_new_a(osargs, blank, empty, tokens, length);
}
osargs_delimiter_t *osargs_delimiter_new_a3(osargs_t *osargs, BOOL blank, BOOL empty, LPCSTR tokenA, LPCSTR tokenB, LPCSTR tokenC) {
	LPCSTR tokens[3]; int length = 0;
	if (tokenA) { tokens[length] = tokenA; length++; }
	if (tokenB) { tokens[length] = tokenB; length++; }
	if (tokenC) { tokens[length] = tokenC; length++; }
	return osargs_delimiter_new_a(osargs, blank, empty, tokens, length);
}
osargs_delimiter_t *osargs_delimiter_new_a4(osargs_t *osargs, BOOL blank, BOOL empty, LPCSTR tokenA, LPCSTR tokenB, LPCSTR tokenC, LPCSTR tokenD) {
	LPCSTR tokens[4]; int length = 0;
	if (tokenA) { tokens[length] = tokenA; length++; }
	if (tokenB) { tokens[length] = tokenB; length++; }
	if (tokenC) { tokens[length] = tokenC; length++; }
	if (tokenD) { tokens[length] = tokenD; length++; }
	return osargs_delimiter_new_a(osargs, blank, empty, tokens, length);
}

osargs_delimiter_t *osargs_delimiter_new_w(osargs_t *osargs, BOOL blank, BOOL empty, LPCWSTR *tokens, int length)
{
	osargs_delimiter_t *const delimiter = delimiter_new_w(blank, empty, tokens, length);
	osargs_delimiter_add(osargs, delimiter);
	return delimiter;
}

osargs_delimiter_t *osargs_delimiter_new_w1(osargs_t *osargs, BOOL blank, BOOL empty, LPCWSTR tokenA) {
	LPCWSTR tokens[1]; int length = 0;
	if (tokenA) { tokens[length] = tokenA; length++; }
	return osargs_delimiter_new_w(osargs, blank, empty, tokens, length);
}
osargs_delimiter_t *osargs_delimiter_new_w2(osargs_t *osargs, BOOL blank, BOOL empty, LPCWSTR tokenA, LPCWSTR tokenB) {
	LPCWSTR tokens[2]; int length = 0;
	if (tokenA) { tokens[length] = tokenA; length++; }
	if (tokenB) { tokens[length] = tokenB; length++; }
	return osargs_delimiter_new_w(osargs, blank, empty, tokens, length);
}
osargs_delimiter_t *osargs_delimiter_new_w3(osargs_t *osargs, BOOL blank, BOOL empty, LPCWSTR tokenA, LPCWSTR tokenB, LPCWSTR tokenC) {
	LPCWSTR tokens[3]; int length = 0;
	if (tokenA) { tokens[length] = tokenA; length++; }
	if (tokenB) { tokens[length] = tokenB; length++; }
	if (tokenC) { tokens[length] = tokenC; length++; }
	return osargs_delimiter_new_w(osargs, blank, empty, tokens, length);
}
osargs_delimiter_t *osargs_delimiter_new_w4(osargs_t *osargs, BOOL blank, BOOL empty, LPCWSTR tokenA, LPCWSTR tokenB, LPCWSTR tokenC, LPCWSTR tokenD) {
	LPCWSTR tokens[4]; int length = 0;
	if (tokenA) { tokens[length] = tokenA; length++; }
	if (tokenB) { tokens[length] = tokenB; length++; }
	if (tokenC) { tokens[length] = tokenC; length++; }
	if (tokenD) { tokens[length] = tokenD; length++; }
	return osargs_delimiter_new_w(osargs, blank, empty, tokens, length);
}

LPWSTR *osargs_get_unparsed_args(osargs_t *osargs, int *length)
{
	length[0] = 0;
	int capacity = 0;
	if (osargs->unparsedArgs)
	{
		free(osargs->unparsedArgs);
		osargs->unparsedArgs = NULL;
	}
	for (int i = 0; i < osargs->length; i++)
	{
		if (!osargs->parsed[i])
		{
			if (osargs->unparsedArgs == NULL)
			{
				osargs->unparsedArgs = (LPWSTR*)malloc(sizeof(LPWSTR) * 4);
				capacity = 4;
			}
			else if (length[0] == capacity)
			{
				capacity = capacity * 2;
				LPWSTR *const unparsedArgsNew = (LPWSTR*)malloc(sizeof(LPWSTR) * (size_t)capacity);
				memcpy(unparsedArgsNew, osargs->unparsedArgs, sizeof(LPWSTR) * (size_t)length[0]);
				free(osargs->unparsedArgs);
				osargs->unparsedArgs = unparsedArgsNew;
			}
			osargs->unparsedArgs[length[0]] = osargs->values[i];
			length[0]++;
		}
	}
	return osargs->unparsedArgs;
}
