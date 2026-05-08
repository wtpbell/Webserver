#!/bin/bash

# ============================================================================ #
# CGI Integration Test                                                         #
#                                                                              #
# A bare bone program of the CGI and CGIProgress for testing the forking and   #
# executing of the cgi scripts based purely on exit codes.                     #
# ============================================================================ #

set -uo pipefail

CGI=./bin/tests/cgi_test
EXIT_FAILURE=0

OK='\e[0;32m[OK]\e[0m'
KO='\e[0;31m[KO]\e[0m'
BLD='\e[1m'
CLR='\e[0m'

# Compiles the cgi integration test
# Requires three arguments:
# $1 = Maximum amount fork calls before failure
# $2 = Maximum amount of socketpairs before failure
# $3 = Maximum amount of dup2 before failure
make_cgi()
{
	echo -en "${BLD}Compiling integration test...${CLR}"
	MAKE=$(make cgi_test FORK_COUNT="$1" PAIR_COUNT="$2" DUP_COUNT="$3" -j 4)
	if [ $? -ne 0 ]; then
			echo -e "${BLD} failed!${CLR}"
			echo -e "Error: $MAKE"
			exit 1
	fi

	if [ ! -f $CGI ]; then
		echo -e "${BLD} failed!${CLR}"
		echo -e "Error: '$CGI' not found!"
		exit 1
	fi

	echo -e "${BLD} complete!${CLR}"
	sleep '0.25s'
	echo -e "\e[1F\e[0K\e[1F"
}

# Removes the cgi binaries of the cgi integration test
clean_cgi()
{
	MAKE=$(make cgi_clean)
	if [ $? -ne 0 ]; then
			echo -e 'Error:' $MAKE
			exit 1
	fi
}

# Assert the exit code
# $1 = Name or Message for test
# $2 = Exit code
# $3 = The expected exit code for succesful test
assert_exit_code()
{
	MSG=$1
	EXCODE=$2
	SUCCESS=$3

	if [ $EXCODE -eq $SUCCESS ]; then
		printf " Test: %-35s ${OK}\n" "$MSG"
	else
		printf " Test: %-35s ${KO}; got %s and expected %s\n" \
			"$MSG" "$EXCODE" "$SUCCESS"
		if [ $EXIT_FAILURE -eq 0 ]; then
			EXIT_FAILURE=1
		fi
	fi
}

# Runs a basic test for the CGI and CGIProgress
# Requires 4 arguments:
# $1 = Name of the test
# $2 = HTTP Target that will be used as path for finding the cgi program to run
# $3 = The expected exit code after running the integration test
test_case_get()
{
	REQUEST=$(cat <<- EOF | tr -d '\n'
		GET ${3} HTTP/1.1\r\n
		Host: local/test\r\n
		\r\n
	EOF
	)

	printf "$REQUEST" |& $CGI >& /dev/null
	assert_exit_code "$1" $? $2
}

# Runs a basic test for the CGI and CGIProgress with a randomized body
# Requires 3 arguments:
# $1 = Name of the test
# $2 = The expected exit code after running the integration test
# $3 = HTTP Target that will be used as path for finding the cgi program to run
test_case_with_body()
{
	REQUEST=$(cat <<- EOF | tr -d '\n'
		POST $3 HTTP/1.1\r\n
		Host: local/test\r\n
		Content-Length: 128\r\n
		Connection: keep-alive\r\n
		\r\n
		$(< /dev/urandom tr -dc A-Za-z0-9_ | head -c 128)
	EOF
	)

	printf "$REQUEST" |& $CGI > cgi_response
	assert_exit_code "$1" $? $2
}

# ALL TEST CASE

GOOD=0
FAIL=1
NOSCRIPT=7
NOPERMS=8
NOFORK=9
NOEXEC=10
NOPAIR=11
FORBID=12
INVALID=13
TIMEOUT=14
CUSTOM=99

clean_cgi && make_cgi 10 10 10
printf "${BLD}CGI Integration Tests: ${CLR}\n"
test_case_get "valid target"										$GOOD			"/cgi-bin/cgi_direct_response.sh"
test_case_get "with querystring"								$GOOD			"/cgi-bin/cgi_direct_response.sh?hello=world&john=doe"
test_case_get "with resource"										$GOOD			"/cgi-bin/cgi_direct_response.sh/some/random/page.html"
test_case_get "all combined"										$GOOD			"/cgi-bin/cgi_direct_response.sh/some/random/page.html?hello=world&john=doe"
test_case_get "script not found"								$NOSCRIPT	"/cgi-bin/do-not-add-me-as-a-file-please.sh"
test_case_get "no exec perms"										$NOPERMS	"/cgi-bin/no_exec_perms.sh"
test_case_get "script timed out"								$TIMEOUT	"/cgi-bin/put_in_timeout.sh"
test_case_get "invalid repsonse"								$INVALID	"/cgi-bin/invalid_response.sh"
test_case_get "execute directory"								$NOSCRIPT	"/cgi-bin/exec_me.sh"
test_case_get "execute symlink (within root)"		$GOOD			"/cgi-bin/exec_me.sh/sym_direct_response"
test_case_get "execute symlink (outside root)"	$FORBID		"/cgi-bin/level_1/ping.sh"
test_case_get "script exit non zero"						$CUSTOM		"/cgi-bin/exit_non_zero.sh"

printf "\n${BLD}CGI body and response Tests: ${CLR}\n"
test_case_with_body "valid cgi post"						$GOOD			"/cgi-bin/run.cgi"

printf "\n${BLD}Specialized Tests: ${CLR}\n"
clean_cgi && make_cgi 0 10 10
test_case_get "unable to fork"									$NOFORK		"/cgi-bin/cgi_direct_response.sh"

clean_cgi && make_cgi 10 0 10
test_case_get "unable to socketpair"						$NOPAIR		"/cgi-bin/cgi_direct_response.sh"

clean_cgi && make_cgi 10 10 0
test_case_get "unable to dup2"									$FAIL			"/cgi-bin/cgi_direct_response.sh"

exit $EXIT_FAILURE
