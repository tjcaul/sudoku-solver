#!/bin/bash
for threads in {0..10}; do
	scripts/build.sh -DTHREADS=$threads -o solve-test

	echo THREADS = $threads:
	for i in {1..3}; do
		\time -p ./solve-test puzzles/invalid-med.txt 2>&1 > /dev/null |
		grep real |
		sed -E 's/real[[:space:]]*//'
	done
done
rm solve-test
