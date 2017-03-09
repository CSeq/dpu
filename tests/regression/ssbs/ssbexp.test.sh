# The great and only SSB3 example !!

#xxx i=2 xxx
#dpu: summary: 4 max-configs, 0 SSBs, 53 events, 24.0 ev/trail
#xxx i=4 xxx
#dpu: summary: 8 max-configs, 0 SSBs, 129 events, 40.0 ev/trail
#xxx i=6 xxx
#dpu: summary: 12 max-configs, 0 SSBs, 229 events, 56.0 ev/trail
#xxx i=8 xxx
#dpu: summary: 16 max-configs, 0 SSBs, 353 events, 72.0 ev/trail
#xxx i=10 xxx
#dpu: summary: 20 max-configs, 0 SSBs, 501 events, 88.0 ev/trail
#xxx i=12 xxx
#dpu: summary: 24 max-configs, 0 SSBs, 673 events, 104.0 ev/trail
#xxx i=14 xxx
#dpu: summary: 28 max-configs, 0 SSBs, 869 events, 120.0 ev/trail
#xxx i=16 xxx
#dpu: summary: 32 max-configs, 0 SSBs, 1089 events, 136.0 ev/trail
#xxx i=18 xxx
#dpu: summary: 36 max-configs, 0 SSBs, 1333 events, 152.0 ev/trail
#xxx i=20 xxx
#dpu: summary: 40 max-configs, 0 SSBs, 1601 events, 168.0 ev/trail

conf_ev[2 ]="4 53"
conf_ev[4 ]="8 129"
conf_ev[6 ]="12 229"
conf_ev[8 ]="16 353"
conf_ev[10]="20 501"
conf_ev[12]="24 673"
conf_ev[14]="28 869"
conf_ev[16]="32 1089"
conf_ev[18]="36 1333"
conf_ev[20]="40 1601"

export conf_ev

MAX=20
K=$(seq 2 2 $MAX)

for i in $K; do
   gcc -E ssbexp.c -D PARAM1=$i -o input.$i.i
done

# the command to test
cmd for i in $K; do \
      $PROG input.$i.i -a2 -s 1M -vv > out.$i; done;

# the checks to perform on the output
echo K is -$K-
test $EXITCODE = 0
ls -l out*

for i in $K; do \
   echo xxx i=$i xxx; grep "dpu: stats: " out.$i; done

for i in $K; do \
   echo xxx i=$i xxx; grep "dpu: summary: " out.$i; done

for i in $K; do \
   grep "0 SSBs" out.$i; done

for i in $K; do \
   for j in $L; do \
      grep -i " unfolding: $(($i + 4)) threads created" out.$i.$j; done; done

for i in $K; do \
   test "$(grep "dpu: summary: " out.$i | awk '{print $3, $7}')" = "${conf_ev[$i]}"; done

for i in $K; do \
   rm input.$i.i; rm out.$i; done
