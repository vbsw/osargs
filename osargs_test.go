/*
 *          Copyright 2021, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

package osargs

import (
	"testing"
)

func TestParseA(t *testing.T) {
	args := new(Arguments)
	args.Values = []string{"asdf", "--version"}
	args.Parsed = make([]bool, len(args.Values))
	versionArg := args.Parse("-v", "--version")

	if !versionArg.Available() {
		t.Error()
	} else {
		if versionArg.Values[0] != args.Values[1] {
			t.Error(versionArg.Values[0])
		}
	}
	if args.Parsed[0] {
		t.Error()
	}
	if !args.Parsed[1] {
		t.Error()
	}
}

func TestParseB(t *testing.T) {
	args := new(Arguments)
	args.Values = []string{"--start", "asdf", "-s", "qwer"}
	args.Parsed = make([]bool, len(args.Values))
	startArg := args.Parse("-s", "--start")

	if !startArg.Available() {
		t.Error()
	} else {
		if startArg.Count() == 2 {
			if startArg.Values[0] != args.Values[0] {
				t.Error(startArg.Values[0])
			}
			if startArg.Values[1] != args.Values[2] {
				t.Error(startArg.Values[1])
			}
		} else {
			t.Error(startArg.Count())
		}
	}
}

func TestParsePairs(t *testing.T) {
	args := new(Arguments)
	args.Values = []string{"asdf", "--start=123"}
	args.Parsed = make([]bool, len(args.Values))
	delimiter := NewDelimiter(false, true, "=")
	startArg := args.ParsePairs(delimiter, "-s", "--start")

	if !startArg.Available() {
		t.Error()
	} else {
		if startArg.Count() == 1 {
			if startArg.Values[0] != "123" {
				t.Error(startArg.Values[0])
			}
		} else {
			t.Error(startArg.Count())
		}
	}
	if args.Parsed[0] {
		t.Error()
	}
	if !args.Parsed[1] {
		t.Error()
	}
}

func TestUnparsedArgs(t *testing.T) {
	args := new(Arguments)
	args.Values = []string{"--start", "asdf", "-s", "qwer"}
	args.Parsed = make([]bool, len(args.Values))
	args.Parse("--start", "-s")
	rest := args.UnparsedArgs()

	if len(rest) != 2 {
		t.Error(len(rest))

	} else if rest[0] != args.Values[1] {
		t.Error(rest[0])

	} else if rest[1] != args.Values[3] {
		t.Error(rest[1])
	}
}
