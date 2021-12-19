#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	LPWSTR *values;
	int length;
	int capacity;
} osargs_result_t;

typedef struct {
	BOOL blank;
	BOOL empty;
	LPWSTR *tokens;
	int length;
} osargs_delimiter_t;

typedef struct {
	LPWSTR *values;
	int *lengths;
	BOOL *parsed;
	int length;
	LPWSTR *unparsedArgs;
	osargs_result_t **results;
	int resultsLen;
	int resultsCap;
	osargs_delimiter_t **delimiters;
	int delimitersLen;
	int delimitersCap;
} osargs_t;

extern void osargs_result_add(osargs_result_t *result, LPCWSTR value);
extern void osargs_result_add_len(osargs_result_t *result, LPCWSTR value, int length);
extern int osargs_delimiter_get_matching_token_length(osargs_delimiter_t *delimiter, LPCWSTR str);

extern osargs_t *osargs_new();
extern osargs_t *osargs_new_a(LPCSTR *argv, int argc);
extern osargs_t *osargs_new_w(LPCWSTR *argv, int argc);
extern void osargs_destroy(osargs_t *osargs);
extern osargs_result_t *osargs_parse_a(osargs_t *osargs, LPCSTR *flags, int length);
extern osargs_result_t *osargs_parse_a1(osargs_t *osargs, LPCSTR flagA);
extern osargs_result_t *osargs_parse_a2(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB);
extern osargs_result_t *osargs_parse_a3(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC);
extern osargs_result_t *osargs_parse_a4(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD);
extern osargs_result_t *osargs_parse_a5(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE);
extern osargs_result_t *osargs_parse_a6(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE, LPCSTR flagF);
extern osargs_result_t *osargs_parse_a7(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE, LPCSTR flagF, LPCSTR flagG);
extern osargs_result_t *osargs_parse_a8(osargs_t *osargs, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE, LPCSTR flagF, LPCSTR flagG, LPCSTR flagH);
extern osargs_result_t *osargs_parse_w(osargs_t *osargs, LPCWSTR *flags, int length);
extern osargs_result_t *osargs_parse_w1(osargs_t *osargs, LPCWSTR flagA);
extern osargs_result_t *osargs_parse_w2(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB);
extern osargs_result_t *osargs_parse_w3(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC);
extern osargs_result_t *osargs_parse_w4(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD);
extern osargs_result_t *osargs_parse_w5(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE);
extern osargs_result_t *osargs_parse_w6(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE, LPCWSTR flagF);
extern osargs_result_t *osargs_parse_w7(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE, LPCWSTR flagF, LPCWSTR flagG);
extern osargs_result_t *osargs_parse_w8(osargs_t *osargs, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE, LPCWSTR flagF, LPCWSTR flagG, LPCWSTR flagH);
extern osargs_result_t *osargs_parse_pairs_a(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR *flags, int length);
extern osargs_result_t *osargs_parse_pairs_a1(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA);
extern osargs_result_t *osargs_parse_pairs_a2(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB);
extern osargs_result_t *osargs_parse_pairs_a3(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC);
extern osargs_result_t *osargs_parse_pairs_a4(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD);
extern osargs_result_t *osargs_parse_pairs_a5(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE);
extern osargs_result_t *osargs_parse_pairs_a6(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE, LPCSTR flagF);
extern osargs_result_t *osargs_parse_pairs_a7(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE, LPCSTR flagF, LPCSTR flagG);
extern osargs_result_t *osargs_parse_pairs_a8(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCSTR flagA, LPCSTR flagB, LPCSTR flagC, LPCSTR flagD, LPCSTR flagE, LPCSTR flagF, LPCSTR flagG, LPCSTR flagH);
extern osargs_result_t *osargs_parse_pairs_w(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR *flags, int length);
extern osargs_result_t *osargs_parse_pairs_w1(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA);
extern osargs_result_t *osargs_parse_pairs_w2(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB);
extern osargs_result_t *osargs_parse_pairs_w3(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC);
extern osargs_result_t *osargs_parse_pairs_w4(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD);
extern osargs_result_t *osargs_parse_pairs_w5(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE);
extern osargs_result_t *osargs_parse_pairs_w6(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE, LPCWSTR flagF);
extern osargs_result_t *osargs_parse_pairs_w7(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE, LPCWSTR flagF, LPCWSTR flagG);
extern osargs_result_t *osargs_parse_pairs_w8(osargs_t *osargs, osargs_delimiter_t *delimiter, LPCWSTR flagA, LPCWSTR flagB, LPCWSTR flagC, LPCWSTR flagD, LPCWSTR flagE, LPCWSTR flagF, LPCWSTR flagG, LPCWSTR flagH);
extern osargs_delimiter_t *osargs_delimiter_new_a(osargs_t *osargs, BOOL blank, BOOL empty, LPCSTR *tokens, int length);
extern osargs_delimiter_t *osargs_delimiter_new_a1(osargs_t *osargs, BOOL blank, BOOL empty, LPCSTR tokenA);
extern osargs_delimiter_t *osargs_delimiter_new_a2(osargs_t *osargs, BOOL blank, BOOL empty, LPCSTR tokenA, LPCSTR tokenB);
extern osargs_delimiter_t *osargs_delimiter_new_a3(osargs_t *osargs, BOOL blank, BOOL empty, LPCSTR tokenA, LPCSTR tokenB, LPCSTR tokenC);
extern osargs_delimiter_t *osargs_delimiter_new_a4(osargs_t *osargs, BOOL blank, BOOL empty, LPCSTR tokenA, LPCSTR tokenB, LPCSTR tokenC, LPCSTR tokenD);
extern osargs_delimiter_t *osargs_delimiter_new_w(osargs_t *osargs, BOOL blank, BOOL empty, LPCWSTR *tokens, int length);
extern osargs_delimiter_t *osargs_delimiter_new_w1(osargs_t *osargs, BOOL blank, BOOL empty, LPCWSTR tokenA);
extern osargs_delimiter_t *osargs_delimiter_new_w2(osargs_t *osargs, BOOL blank, BOOL empty, LPCWSTR tokenA, LPCWSTR tokenB);
extern osargs_delimiter_t *osargs_delimiter_new_w3(osargs_t *osargs, BOOL blank, BOOL empty, LPCWSTR tokenA, LPCWSTR tokenB, LPCWSTR tokenC);
extern osargs_delimiter_t *osargs_delimiter_new_w4(osargs_t *osargs, BOOL blank, BOOL empty, LPCWSTR tokenA, LPCWSTR tokenB, LPCWSTR tokenC, LPCWSTR tokenD);
extern LPWSTR *osargs_get_unparsed_args(osargs_t *osargs, int *length);

#ifdef __cplusplus
}
#endif