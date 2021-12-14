# osargs

[![GoDoc](https://godoc.org/github.com/vbsw/osargs?status.svg)](https://godoc.org/github.com/vbsw/osargs) [![Go Report Card](https://goreportcard.com/badge/github.com/vbsw/osargs)](https://goreportcard.com/report/github.com/vbsw/osargs) [![Stability: Experimental](https://masterminds.github.io/stability/experimental.svg)](https://masterminds.github.io/stability/experimental.html)

## About
Package osargs provides functions to parse command line arguments. It is published on <https://github.com/vbsw/osargs> and <https://gitlab.com/vbsw/osargs>.

## Copyright
Copyright 2021, Vitali Baumtrok (vbsw@mailbox.org).

osargs is distributed under the Boost Software License, version 1.0. (See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

osargs is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the Boost Software License for more details.

## Usage

### Example A

	package main

	import (
		"fmt"
		"github.com/vbsw/osargs"
	)

	func main() {
		args := osargs.New()

		if args.Parse("--help", "-h").Available() {
			fmt.Println("valid parameters are -h or -v.")

		} else if args.Parse("--version", "-v").Available() {
			fmt.Println("version 1.0.0")

		} else {
			unparsedArgs := args.UnparsedArgs()

			if len(unparsedArgs) == 1 {
				fmt.Println("error: unknown parameter", unparsedArgs[0])

			} else if len(unparsedArgs) > 1 {
				fmt.Println("error: too many arguments")
			}
		}
	}

### Example B

	package main

	import (
		"fmt"
		"github.com/vbsw/osargs"
	)

	func main() {
		start := "0"
		end := "0"
		args := osargs.New()
		delimiter := osargs.NewDelimiter(false, false, "=")

		startArg := args.ParsePairs(delimiter, "start")
		endArg := args.ParsePairs(delimiter, "end")

		if startArg.Available() {
			start = startArg.Values[0]
			end = start
		}
		if endArg.Available() {
			end = endArg.Values[0]
		}
		fmt.Println("processing from", start, "to", end)
	}

Command line:

	$ ./test start=1 end=10
	$ processing from 1 to 10

## References
- https://golang.org/doc/install
- https://git-scm.com/book/en/v2/Getting-Started-Installing-Git
