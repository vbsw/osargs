# osargs

[![Stability: Experimental](https://masterminds.github.io/stability/experimental.svg)](https://masterminds.github.io/stability/experimental.html)

## About
osargs provides functions to parse command line arguments. It is published on <https://github.com/vbsw/osargs> and <https://gitlab.com/vbsw/osargs>.

## Copyright
Copyright 2021, Vitali Baumtrok (vbsw@mailbox.org).

osargs is distributed under the Boost Software License, version 1.0. (See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

osargs is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the Boost Software License for more details.

## Usage

### Example A

	auto args = new OSArgs();
	
	if (args->parse("--help", "-h")->isAvailable()) {
		printf("valid parameters are -h or -v\n");

	} else if (args->parse("--version", "-v")->isAvailable()) {
		printf("version 1.0.0\n");

	} else {
		int length;
		auto unparsedArgs = args->getUnparsedArgs(&length);

		if (length == 1) {
			printf("error: unknown parameter &ls\n", unparsedArgs[0]);
		} else if (length > 1) {
			printf("error: too many arguments\n");
		}
	}
	delete args;

### Example B

	int start = 0;
	int end = 0;
	auto args = new OSArgs();
	auto delimiter = args->newDelimiter(FALSE, FALSE, "=");

	auto startArg = args->parsePairs(delimiter, "start");
	auto endArg = args->parsePairs(delimiter, "end");
	
	if (startArg->isAvailable()) {
		start = startArg->getValues()[0];
		end = start;
	}
	if (startArg->isAvailable()) {
		end = endArg->getValues()[0];
	}
	printf("processing from &d to &d", start, end);
	delete args;

Command line:

	$ ./test start=1 end=10
	$ processing from 1 to 10

## References
- https://git-scm.com/book/en/v2/Getting-Started-Installing-Git
