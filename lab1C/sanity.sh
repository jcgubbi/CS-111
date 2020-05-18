#!/bin/bash

# UCLA CS 111 Lab 1c testing script, written by Zhaoxing Bu (zbu@ucla.edu).
# This script should only be used when Zhaoxing is in the TA team for 111.

# DO NOT UPLOAD THIS SCRIPT TO WEB OR ANYWHERE ELSE. Any usage without permission
# is strictly forbidden.

# To reader: please read the entire script carefully.

# No partial credits.
# Do not run multiple testing scripts at the same time.
# Only run this on lnxsrv09.seas.ucla.edu please.
# REMEMBER to execute PATH=/usr/local/cs/bin:$PATH in shell to call the correct
# gcc for compiling students' work.
# This PATH change is restored after logging out.
# This script automatically changes the PATH value for you.
# Any comments, suggestions, problem reports are greatly welcomed.

# How to run the script? Place it with students' submissions in the same directory.
# Also, make sure you have downloaded pg98.txt and put it in the same directory.
# Run "./lab1c_sanity_script.sh UID", for example ./lab1c_sanity_script.sh 197912071.

if [ "${PATH:0:16}" == "/usr/local/cs/bin" ]
then
  true
else
  PATH=/usr/local/cs/bin:$PATH
fi

if ps | grep "simpsh"
then
  echo "simpsh is running in background."
  echo "Testing cannot continue."
  echo "Kill it and then run the script."
  exit 1
fi

if ! ls -lh pg98.txt
then
  echo "Please download pg98.txt!"
  exit 2
fi

echo "DO NOT run multiple testing scripts at the same time."
echo "Please check if there is any error message below."
echo "==="

# Check tarball.
STUDENT_UID="$1"
SUBMISSION="lab1-$STUDENT_UID.tar.gz"
if [ -e "$SUBMISSION" ]
then
  true
else
  echo "No submission: lab1-$STUDENT_UID.tar.gz"
	exit 1
fi

# Untar into student's directory.
rm -rf $STUDENT_UID
mkdir $STUDENT_UID
tar -C $STUDENT_UID -zxvf $SUBMISSION
cd $STUDENT_UID

# Make.
if [ -e "simpsh" ]
then
  rm -rf simpsh
fi
make || exit

make check
if [ $? == 0 ]
then
  echo "===>make check passed"
else
  echo "===>make check failed"
fi

rm -rf $SUBMISSION
make dist
if [ -e "$SUBMISSION" ]
then
  echo "===>make dist passed"
else
  echo "===>make dist failed"
fi

# Create testing directory.
TEMPDIR="lab1creadergradingtempdir"
rm -rf $TEMPDIR
mkdir $TEMPDIR
if [ "$(ls -A $TEMPDIR 2> /dev/null)" == "" ]
then
  true
else
  echo "Fatal error! The testing directory is not empty."
  exit 1
fi
mv simpsh ./$TEMPDIR/
cd $TEMPDIR


# Create testing files.
cat > a0.txt <<'EOF'
Hello world! CS 111! Knowledge crowns those who seek her.
EOF
cat a0.txt > a1.txt
cat a0.txt > a2.txt
cat a0.txt > a3.txt
cat a0.txt > a4.txt
cat a0.txt > a5.txt
cat a0.txt > a6.txt
cat a0.txt > a7.txt
cat a0.txt > a8.txt

cat > b0.txt <<'EOF'
FEAR IS THE PATH TO THE DARK SIDE...FEAR LEADS TO ANGER...ANGER LEADS TO HATE...HATE LEADS TO SUFFERING.
DO. OR DO NOT. THERE IS NO TRY.
EOF
cp b0.txt b1.txt

echo "==="
echo "Please DO NOT run multiple testing scripts at the same time."
echo "Make sure there is no simpsh running by you."
echo "Infinite waiting of simpsh due to unclosed pipe is unacceptable."
echo "Starting grading:"
NUM_PASSED=0
NUM_FAILED=0

echo -e "\n\n======ALERT! MANUAL CHECKING BEGINS======"

# Manual testing for 1c.
LOOP_INDEX_I=0
while [ $LOOP_INDEX_I -lt 100 ]
do
    cat ../../pg98.txt >> pg98_100.txt
    LOOP_INDEX_I=`expr $LOOP_INDEX_I + 1`
done
if ! ls -lh pg98_100.txt
then
  echo "Fatal error!"
  exit 3
fi


# Test case 1 --profile almost no time.
echo ""
echo "--->test case 1:"
echo "Check if there is time info for --rdwr option."
echo "Time should be (almost) 0"
./simpsh --creat --profile --rdwr test1io.txt >c1out.txt 2>c1err.txt
if [ $? == 0 ] && [ ! -s c1err.txt ] && [ -s c1out.txt ] && [ -e test1io.txt ] \
  && [ ! -s test1io.txt ] && wc -l < c1out.txt | grep -q "1"
then
  echo "----------c1out.txt----------"
  cat c1out.txt
  echo "----------c1out.txt----------"
else
  echo "===>test case 1 failed"
fi

# Test case 2 --profile scope.
echo ""
echo "--->test case 2:"
echo "Should only see time info for --rdwr, but not for --pipe or --command."
echo "And, we don't have wait here."
./simpsh --pipe --command 0 1 1 sleep 1 --creat --profile --rdwr test2io.txt \
  >c2out.txt 2>c2err.txt
if [ $? == 0 ] && [ ! -s c2err.txt ] && [ -s c2out.txt ] && [ -e test2io.txt ] \
  && [ ! -s test2io.txt ] && wc -l < c2out.txt | grep -q "1"
then
  echo "----------c2out.txt----------"
  cat c2out.txt
  echo "----------c2out.txt----------"
else
  echo "===>test case 2 failed"
fi

# Test case 3 --profile sort a large file.
echo ""
echo "--->test case 3:"
echo "Here you would see three time info, one for --command, which should be (almost) 0."
echo "Another time info, which is for --wait, should also be (almost) 0."
echo "The last time info is for child process, should not be 0."
echo "You can output time info for each child process, or just output a sum for"
echo "all children you waited for."
./simpsh --rdonly pg98_100.txt --creat --wronly test3out.txt --creat \
  --wronly test3err.txt --profile --command 0 1 2 sort --wait >c3out.txt \
  2>c3err.txt
if [ $? == 0 ] && [ ! -s c3err.txt ] && [ -s c3out.txt ] && [ -e test3out.txt ] \
  && [ -s test3out.txt ] && [ -e test3err.txt ] && [ ! -s test3err.txt ] \
  && (wc -l < c3out.txt | grep -q "4" || wc -l < c3out.txt | grep -q "5") 
then
  echo "----------c3out.txt----------"
  cat c3out.txt
  echo "----------c3out.txt----------"
else
  echo "===>test case 3 failed"
fi

# Finished testing.
echo ""
echo "Grading finished."
echo ""

if ps | grep "simpsh"
then
  echo "simpsh is running in background."
  echo "Check your code to make sure no simpsh is hanging there forever."
  echo "You will lose points for that."
fi

echo -e "\nOutput from times:"
times

echo ""
echo "Student's README file:"
echo "==="
cat ../README
echo "==="

echo "**********Report check**********"
if ls -lh ../*.pdf
then
  echo "Report submitted."
else
  echo "ALERT!!! No report was submitted!"
fi
