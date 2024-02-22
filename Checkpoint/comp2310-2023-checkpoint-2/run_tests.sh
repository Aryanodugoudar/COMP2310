#! /usr/bin/env bash

# To run all tests:
#   ./run_tests.sh 
# To see the options available:
#   ./run_tests.sh -h

# Color output can be disable by using the -n opt.
RED='[0;31m'
GREEN='[0;32m'
BLUE='[0;34m'
BOLD='[1m'
NONE='[0;00m'

TESTDIR="./tests"
OUTPUTDIR="./output"
PROGRAM_NAME="db_engine"

# Run make clean and then make all.
run_make () {
  local moutput=""
  make clean > /dev/null 2>&1
  printf "${GREEN}Running ${BOLD}%-24s${NONE} " "make all:"
  moutput=$(make all 2>&1)
  if [ $? != 0 ]; then 
    printf "${RED}failed!${NONE}\n"
    echo "${moutput}"
    exit 1
  else 
    printf "${BLUE}ok!${NONE}\n"
  fi
  return 
}

# If test fails due to differences in expected and actual output, print names
# of files it is using for comparison purposes.
print_diff_info () {
  local actual_output=$1
  local expected_output=$2
  printf "    You can compare the results using 'diff' or 'cmp'\n" 
  printf "    - %s\n" "${BOLD}Actual   output${NONE}: ${actual_output}"
  printf "    - %s\n" "${BOLD}Expected output${NONE}: ${expected_output}"
  return
}

run_test () {
  local testname=$1
  local verbose=$2
  local diff_ret=0 # return code from diff
  local prog_ret=0 # return code from program itself
  local test_file="${TESTDIR}/${testname}.run"
  local passed=1

  read command file1 file2 expected retcode cmp_out < ${test_file}
  
  if [[ "$verbose" == "1" || "$cmp_out" == "1" ]]; then
    out="${OUTPUTDIR}/${testname}.out"
  else
    out="/dev/null"
  fi

  if [[ ${cmp_out} == "1" ]]; then output=$out; else output=$file2; fi

  printf "${BLUE}Running test ${BOLD}%-19s${NONE} " "${testname}:"



  # Run command, redirecting stdout and stderr
  { ./${PROGRAM_NAME} ${command} ${file1} ${file2}; } &> ${out} 2>&1

  # Check return code matches first
  # Even if this fails, it will still run the diff step
  prog_ret=$?
  if [ "${prog_ret}" != "${retcode}" ]; then
    passed=0
    printf "${RED}failed!${NONE}\n"
    printf "  - %s\n" "Expected return code ${retcode}, got ${prog_ret} instead"
  fi

  # Check expected output file exists
  if [ ! -f ${output} ]; then
    if [ ${passed} -ne 0 ]; then printf "${RED}failed!${NONE}\n"; fi
    passed=0
    printf "  - %s\n" "Output file ${output} does not exist"
    touch ${output}
  fi
  
  # Compare expected output with actual output
  if [ "${expected}" != "" ]; then 
    diff_output=$(diff ${output} ${expected})
    diff_ret=$?
    if [ ${diff_ret} -ne 0 ]; then
      if [ ${passed} -ne 0 ]; then printf "${RED}failed!${NONE}\n"; fi
      passed=0
      printf "  - %s\n" "Actual output does not match expected output"
      print_diff_info $output $expected
    fi 
  fi

  if [ $passed -eq 1 ]; then 
    printf "${GREEN}passed!${NONE}\n"
  else 
    # If test doesn't pass, print out what was run
    printf "  - %s\n" "${BOLD}What is being run${NONE}: ./${PROGRAM_NAME} ${command} ${file1} ${file2}"
  fi 

  # If verbose option set, print out the contents of stdout + stderr
  if [ "$verbose" == "1" ]; then
    printf "%s\n" " ============================ Output ============================ "
    cat ${out}
    printf "\n"
    printf "%s\n" " ================================================================ "
  fi

  return $passed
} 

# Print usage information
usage () {
  echo "usage: ./run_tests.sh [-h] [-v] [-n] [-t test] "
  echo "  -h           (help)    show this help message and exit."
  echo "  -n           (nocolor) disable color output when printing test results."
  echo "  -t test      (test)    run only the given test name"
  echo "  -v           (verbose) print stdout and stderr when running tests."
  return 0
}

verbose=0
color=1
test=""

options=`getopt hnvt: $*`
errcode=$?
if [ ${errcode} -ne 0 ]; then 
  usage
  exit 1
fi

set -- $options
for i; do
  case "$i" in
  -h)
    usage
    exit 0
    shift;;
  -n)
    RED=
    GREEN=
    BLUE=
    BOLD=
    UNDERLINE=
    NONE=
    shift;;
  -t)
    test=$2
    shift;;
  -v)
    verbose=1
    shift;;
  --)
    shift; break;;
  esac
done

if [ "${test}" != "" ]; then
  ALL_TESTS=${test}
else 
  CREATE_TESTS="basic_create large_create zeroes_create notfound_create"
  QUERY_TESTS="basic_lookup basic_intersection large_lookup large_intersection"
  PRINT_TESTS="basic_print large_print zeroes_print"
  ALL_TESTS="${CREATE_TESTS} ${QUERY_TESTS} ${PRINT_TESTS}"
fi

run_make
mkdir -p output

TOTAL_PASSED=0

(( tests_run = 0    ))
(( tests_passed = 0 ))
for t in $ALL_TESTS; do
  run_test $t $verbose
  x=$?
  (( tests_run = $tests_run + 1 ))
  (( tests_passed = $tests_passed + $x ))
done

printf "%s\n" "${BOLD}Total${NONE}: ${tests_passed}/${tests_run} tests passed."

