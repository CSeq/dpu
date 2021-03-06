
round() {
    python -c "print '%.3f' % (float ($1) / (1000 * 1000 * 1000))"
}

preprocess_family()
{
   CFILE=$1
   IPATH=$2
   P1NAME=$3
   P1VALS=$4
   P2NAME=$5
   P2VALS=$6
   CPP=cpp

   # only 1 parameter
   if test -z "$P2NAME"; then
      for P1 in `echo $P1VALS`; do
         P1_=$(echo "$P1" | sed 's/^0\+//') # remove trailing 0s
         CMD="$CPP -E -D PARAM1=$P1_ $CFILE -o ${IPATH}-${P1NAME}${P1}.i"
         echo $CMD
         $CMD
      done
      return
   fi

   # 2 parameters
   for P1 in `echo $P1VALS`; do
      for P2 in `echo $P2VALS`; do
         P1_=$(echo "$P1" | sed 's/^0\+//') # remove trailing 0s
         P2_=$(echo "$P2" | sed 's/^0\+//') # remove trailing 0s
         CMD="$CPP -E -D PARAM1=$P1_ -D PARAM2=$P2_ $CFILE -o ${IPATH}-${P1NAME}${P1}_${P2NAME}${P2}.i"
         echo $CMD
         $CMD
      done
   done
}

run_dpu ()
{
   # expects the following variables to be defiend:
   # $N       - identifier of the benchmark
   # $CMD     - the command to run
   # $LOG     - the path to the log file to generate
   # $TIMEOUT - a timeout specification valid for timeout(1)

   CMD="/usr/bin/time -v timeout $TIMEOUT $CMD"
   echo "name      $N" > $LOG
   echo "cmd       $CMD" >> $LOG

   # run the program
   echo "# $CMD"
   BEGIN=`date +%s%N`
   $CMD > ${LOG}.stdout 2> ${LOG}.stderr
   EXITCODE=$?
   END=`date +%s%N`

   # check for timeouts
   if test "$EXITCODE" = 124; then
      DEFECTS="-"
      WALLTIME="TO"
      MAXCONFS="-"
      SSBS="-"
      EVENTS="-"
   elif test "$EXITCODE" != 0; then
      if grep "dpu: error.*unhandled" ${LOG}.{stdout,stderr}; then
         WALLTIME="MO"
      else
         WALLTIME="ERR"
      fi
      DEFECTS="-"
      MAXCONFS="-"
      SSBS="-"
      EVENTS="-"
   else
      WALLTIME=`round $(($END-$BEGIN))`
      DEFECTS=$(grep "dpu: summary" ${LOG}.stdout | awk '{print $3}')
      MAXCONFS=$(grep "dpu: summary" ${LOG}.stdout | awk '{print $5}')
      SSBS=$(grep "dpu: summary" ${LOG}.stdout | awk '{print $7}')
      EVENTS=$(grep "dpu: summary" ${LOG}.stdout | awk '{print $9}')
   fi

   # max RSS
   MAXRSS=$(grep 'Maximum resident set size' ${LOG}.stderr | awk '{print $6}')
   MAXRSS=$(bc <<< "$MAXRSS / 1024")

   # dump numbers to the log file
   echo "" >> $LOG
   echo "exitcode  $EXITCODE" >> $LOG
   #echo "begin     $BEGIN" >> $LOG
   #echo "end       $END" >> $LOG
   echo "defects   $DEFECTS" >> $LOG
   echo "walltime  $WALLTIME" >> $LOG
   echo "maxrss    ${MAXRSS}M" >> $LOG
   echo "maxconfs  $MAXCONFS" >> $LOG
   echo "SSBs      $SSBS" >> $LOG
   echo "events    $EVENTS" >> $LOG

   # print a summary

   FORMAT='%-40s %8s %8s %8s %8s %8s %8s\n'
   printf "$FORMAT" LOG, WTIME, MAXRSS, MAXCON, SSBS, EVENTS, DEFECTS,
   printf "$FORMAT\n" $LOG, $WALLTIME, $MAXRSS, $MAXCONFS, $SSBS, $EVENTS, $DEFECTS,

   echo -e "\n\nstdout:" >> $LOG
   cat ${LOG}.stdout >> $LOG
   echo -e "\nstderr:" >> $LOG
   cat ${LOG}.stderr >> $LOG
   rm ${LOG}.stdout
   rm ${LOG}.stderr

   return $EXITCODE
}

run_nidhugg ()
{
   # expects the following variables to be defiend:
   # $N       - identifier of the benchmark
   # $CMD     - the command to run
   # $LOG     - the path to the log file to generate
   # $TIMEOUT - a timeout specification valid for timeout(1)

   CMD="/usr/bin/time -v timeout $TIMEOUT $CMD"
   echo "name      $N" > $LOG
   echo "cmd       $CMD" >> $LOG

   # run the program
   echo "# $CMD"
   BEGIN=`date +%s%N`
   $CMD > ${LOG}.stdout 2> ${LOG}.stderr
   EXITCODE=$?
   END=`date +%s%N`

   # check for timeouts
   if test "$EXITCODE" = 124; then
      WALLTIME="TO"
      MAXCONFS="-"
      SSBS="-"
   elif test "$EXITCODE" != 0; then
      WALLTIME="ERR"
      MAXCONFS="-"
      SSBS="-"
   else
      WALLTIME=`round $(($END-$BEGIN))`
      MAXCONFS=$(grep 'Trace count: ' ${LOG}.stdout | sed 's/.*Trace count: //' | awk '{print $1}')
      SSBS=$(grep 'Trace count: ' ${LOG}.stdout | sed 's/.*Trace count: //' | awk '{print $3}')
   fi

   # max RSS
   MAXRSS=$(grep 'Maximum resident set size' ${LOG}.stderr | awk '{print $6}')
   MAXRSS=$(bc <<< "$MAXRSS / 1024")

   # dump numbers to the log file
   echo "" >> $LOG
   echo "exitcode  $EXITCODE" >> $LOG
   #echo "begin     $BEGIN" >> $LOG
   #echo "end       $END" >> $LOG
   echo "walltime  $WALLTIME" >> $LOG
   echo "maxrss    ${MAXRSS}M" >> $LOG
   echo "maxconfs  $MAXCONFS" >> $LOG
   echo "SSBs      $SSBS" >> $LOG

   # print a summary
   FORMAT='%-40s %8s %8s %8s %8s\n'
   printf "$FORMAT" LOG, WTIME, MAXRSS, MAXCON, SSBS,
   printf "$FORMAT\n" $LOG, $WALLTIME, $MAXRSS, $MAXCONFS, $SSBS,

   echo -e "\n\nstdout:" >> $LOG
   cat ${LOG}.stdout >> $LOG
   echo -e "\nstderr:" >> $LOG
   cat ${LOG}.stderr >> $LOG
   rm ${LOG}.stdout
   rm ${LOG}.stderr

   return $EXITCODE
}

print_date()
{
   MSG=$1

   echo "======="
   if test "$MSG"; then
      echo "$MSG"
   fi
   echo -n "Date: "
   date -R
   echo "======="
}

handler()
{
   echo "Signal received, stopping now"
   print_date "Exiting due to signal"
   exit 1
}
