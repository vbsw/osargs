/*
 *          Copyright 2021, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

// Package osargs provides functions to parse command line arguments.
package osargs

import (
	"os"
	"strings"
)

// Arguments holds command line arguments.
type Arguments struct {
	Values []string
	Parsed []bool
}

// Delimiter is the separator between flag and value in command line argument.
type Delimiter struct {
	Tokens []string
	Blank  bool
	Empty  bool
}

// Result represents parsed argument in command line.
type Result struct {
	Values []string
}

// New creates and returns a new instance of Arguments.
// Arguments have all command line arguments except the first one, which is the program name.
func New() *Arguments {
	args := new(Arguments)
	if len(os.Args) > 1 {
		args.Values = make([]string, len(os.Args)-1)
		copy(args.Values, os.Args[1:])
		args.Parsed = make([]bool, len(args.Values))
	}
	return args
}

// NewDelimiter creates and returns a new instance of Delimiter.
func NewDelimiter(blank, empty bool, Tokens ...string) *Delimiter {
	delimiter := new(Delimiter)
	delimiter.Tokens = Tokens
	delimiter.Blank = blank
	delimiter.Empty = empty
	return delimiter
}

// UnparsedArgs returns unparsed arguments.
func (args *Arguments) UnparsedArgs() []string {
	var unparsedArgs []string
	if len(args.Values) > 0 {
		for i, value := range args.Values {
			if !args.Parsed[i] {
				if len(unparsedArgs) == 0 {
					unparsedArgs = make([]string, 0, len(args.Values)-i)
				}
				unparsedArgs = append(unparsedArgs, value)
			}
		}
	}
	return unparsedArgs
}

// Parse returns available flags in command line.
func (args *Arguments) Parse(flags ...string) *Result {
	result := new(Result)
	if len(args.Values) > 0 && len(flags) > 0 {
		for i, value := range args.Values {
			if !args.Parsed[i] {
				for _, flag := range flags {
					if value == flag {
						result.Values = append(result.Values, value)
						args.Parsed[i] = true
						break
					}
				}
			}
		}
	}
	return result
}

// ParsePairs returns the values of available flags in command line.
// The values are the ones that are separated from flags by the delimiter.
func (args *Arguments) ParsePairs(delimiter *Delimiter, flags ...string) *Result {
	result := new(Result)
	if len(args.Values) > 0 && len(flags) > 0 {
		if delimiter.Blank {
			args.parsePairsWithBlank(result, delimiter, flags)
		} else if len(delimiter.Tokens) > 0 || delimiter.Empty {
			args.parsePairsWithoutBlank(result, delimiter, flags)
		}
	}
	return result
}

func (args *Arguments) parsePairsWithBlank(result *Result, delimiter *Delimiter, flags []string) {
	for i := 0; i < len(args.Values); i++ {
		if !args.Parsed[i] {
			value := args.Values[i]
			for _, flag := range flags {
				if strings.HasPrefix(value, flag) {
					if len(value) == len(flag) {
						value := ""
						args.Parsed[i] = true
						if i+1 < len(args.Values) {
							value = args.Values[i+1]
							args.Parsed[i+1] = true
							i++
						}
						result.Values = append(result.Values, value)
						break
					} else {
						valueWithoutFlag := value[len(flag):]
						match, token := delimiter.MatchingToken(valueWithoutFlag)
						if match {
							result.Values = append(result.Values, valueWithoutFlag[len(token):])
							args.Parsed[i] = true
							break
						}
					}
				}
			}
		}
	}
}

func (args *Arguments) parsePairsWithoutBlank(result *Result, delimiter *Delimiter, flags []string) {
	for i, value := range args.Values {
		if !args.Parsed[i] {
			for _, flag := range flags {
				if strings.HasPrefix(value, flag) {
					if len(value) == len(flag) {
						result.Values = append(result.Values, "")
						args.Parsed[i] = true
						break
					} else {
						valueWithoutFlag := value[len(flag):]
						match, token := delimiter.MatchingToken(valueWithoutFlag)
						if match {
							result.Values = append(result.Values, valueWithoutFlag[len(token):])
							args.Parsed[i] = true
							break
						}
					}
				}
			}
		}
	}
}

// MatchingToken returns true and the delimiter token, if str starts with token.
func (delimiter *Delimiter) MatchingToken(str string) (bool, string) {
	for _, token := range delimiter.Tokens {
		if strings.HasPrefix(str, token) {
			return true, token
		}
	}
	return delimiter.Empty, ""
}

// Available returns true, if argument is in command line.
func (result *Result) Available() bool {
	return len(result.Values) > 0
}

// Count returns number of matches in command line.
func (result *Result) Count() int {
	return len(result.Values)
}
