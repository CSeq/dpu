# Can unfold with 2 threads

gcc -E nthreads.c -D N=20 -o input.i

cmd $PROG input.i -vv

test $EXITCODE = 0
grep ": 1 max-configs"
grep " unfolding: 21 threads created"
rm input.i