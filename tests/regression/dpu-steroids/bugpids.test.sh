# Bug about the missmatch between steroids TIDs and DPU PIDs

cmd $PROG bugpids.c -vv

test $EXITCODE = 0
grep "dpu: summary: 0 defects, 4 max-configs"
grep "dpu: stats: unfolding: 3 threads created"
