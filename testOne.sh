#!/bin/bash
# testOne.sh functional test script
# Call with test platform, test-program test-set [,options [,arguments]]
# test-set is a char usually 1-9 for the various .out files, usually use 1 for .out1
# e.g. "./testOne.sh pentium hello"
# or   "./testOne.sh sparc fibo 1 '' 10"
# or   "./testOne.sh sparc switch_cc 6 '-Td -nG' '2 3 4 5 6'"
# Note: options and arguments are quoted strings
# $1 = platform $2 = test $3 = test-set $4 = options $5 = parameters to the recompiled executable

echo $* > functest.res
rm -rf functest/$1/$2

SPACES="                                                 "
RES="Result for $1"
WHITE=${SPACES:0:(18 - ${#RES})}
RES="$RES$WHITE $2:"
WHITE=${SPACES:0:(34 - ${#RES})}
RES=$RES$WHITE
echo -n -e "$RES"

sh -c "./boomerang -o functest/$1 $4 test/$1/$2 2>/dev/null >/dev/null"
ret=$?
if [[ ret -ge 128 ]]; then
	SIGNAL="signal $((ret-128))"
	if [ "$SIGNAL" = "signal 9" ]; then SIGNAL="a kill signal"; fi
	if [ "$SIGNAL" = "signal 11" ]; then SIGNAL="a segmentation fault"; fi
	if [ "$SIGNAL" = "signal 15" ]; then SIGNAL="a termination signal"; fi
	RESULT="Boomerang FAILED set $3 with $SIGNAL"
else
	if [[ ! -f functest/$1/$2/$2.c ]]; then
		RESULT="NO BOOMERANG OUTPUT set $3!"
	else
		cat `ls -rt functest/$1/$2/*.c` > functest.c
		# if test/$1/$2.sed exists, use it to make "known error" corrections to the source code
		if [[ -f test/$1/$2.sed ]]; then
			echo Warning... $1/$2.sed used >> functest.res
			sed -i -f test/$1/$2.sed functest.c
			ret=$?
			if [[ ret -ne 0 ]]; then
				echo test/$1/$2.sed FAILED! >> functest.res
				exit 10
			fi
		fi
		gcc -D__size32=int -D__size16=short -D__size8=char -o functest.exe functest.c >> functest.res 2>&1
		if [[ $? != 0 ]]; then
			RESULT="Compile FAILED"
		else
			sh -c "./functest.exe $5 > functest.out 2>&1"
			ret=$?
			if [[ ret -ge 128 ]]; then
				SIGNAL="signal $((ret-128))"
				if [ "$SIGNAL" = "signal 9" ]; then SIGNAL="a kill signal"; fi
				if [ "$SIGNAL" = "signal 11" ]; then SIGNAL="a segmentation fault"; fi
				if [ "$SIGNAL" = "signal 15" ]; then SIGNAL="a termination signal"; fi
				RESULT="EXECUTION TERMINATED with $SIGNAL"
			else
				if [[ ret -ne 0 ]]; then
					echo Warning! return code from execute was $((ret)) >> functest.res
				fi
				diff -u test/source/$2.out$3 functest.out >> functest.res
				ret=$?
				if [[ ret -ne 0 ]]; then
					RESULT="FAILED diff set $3"
				else
					RESULT="Passed set $3"
				fi
			fi
		fi
	fi
fi
grep goto functest.c > /dev/null
if [[ $? -eq 0 ]]; then
	RESULT=$RESULT" (gotos in output)"
fi
echo $RESULT
echo -e "$RES""$RESULT" >> functest.res
echo >> functest.res
cat functest.res >> functests.out
#grep "^Result" functest.res
rm -f functest.{res,c,exe,out}
