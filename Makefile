do: run

compile:
	@echo "==================================================================================="
	g++ gkls.c rnd_gen.c Spark_main.cpp -o spark.out

run: compile
	clear
	./spark.out --func_cls=1 --func_id=1
	

test:
	g++ test.cpp -o test.out
	clear
	./test.out

cat:
	cat *.sh.po*
	cat *.sh.o*

queue:
	qstat -f
	# qdel

num:
	ls results/Disimpl-v/ | wc -l

mem_check:  compile
	valgrind --tool=memcheck --leak-check=full --track-origins=yes -v ./spark.out --func_cls=1 --func_id=1

profiler:  compile
	valgrind --tool=callgrind ./spark.out --func_cls=1 --func_id=1
	# git clone https://github.com/jrfonseca/gprof2dot bin/gprof2dot
	./bin/gprof2dot/gprof2dot.py -f callgrind callgrind.out.* | dot -Tsvg -o profile.svg
