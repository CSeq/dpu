# Can unfold up to 25 threads

# initialization
N=$(seq 1 2 25)
for i in $N; do
   gcc -E nthreads.c -D N=$i -o input$i.i
done

# the command to test
cmd for i in $N; do \
      $PROG input$i.i -vv > out$i; done

# the checks to perform on the output
echo $N
test $EXITCODE = 0
for i in $N; do echo xxx $i xxx; grep "dpu: stats: " out$i; done
for i in $N; do grep -i ": 1 max-configs" out$i; done
for i in $N; do grep -i " unfolding: $(($i + 1)) threads created" out$i; done
for i in $N; do rm input${i}.i; done
for i in $N; do rm out$i; done