#!/usr/bin/env bash

shopt -q globstar

for f in files/**/*
do
	echo $f
	cp $f ./test.file
	./main a test.file test.file.a 2>/dev/null
	./main d test.file.a test.file.b 2>/dev/null
	if ! diff -q test.file test.file.b > /dev/null; then
		  echo "adaptive error"
	fi
	rm test.file.*

	./main b test.file test.file.a 2>/dev/null
	./main d test.file.a test.file.b 2>/dev/null
	if ! diff -q test.file test.file.b > /dev/null; then
		  echo "bigramm error"
	fi
	rm test.file.*

	./main f test.file test.file.a 2>/dev/null
	./main d test.file.a test.file.b 2>/dev/null
	if ! diff -q test.file test.file.b > /dev/null; then
		  echo "Fano error"
	fi
	rm test.file.*

	./main h test.file test.file.a 2>/dev/null
	./main d test.file.a test.file.b 2>/dev/null
	if ! diff -q test.file test.file.b > /dev/null; then
		  echo "Huffman error"
	fi
	rm test.file.*

	./main r test.file test.file.a 2>/dev/null
	./main d test.file.a test.file.b 2>/dev/null
	if ! diff -q test.file test.file.b > /dev/null; then
		  echo "arithmetic error"
	fi
	rm test.file.*

	./main s test.file test.file.a 2>/dev/null
	./main d test.file.a test.file.b 2>/dev/null
	if ! diff -q test.file test.file.b > /dev/null; then
		  echo "Shannon error"
	fi
	rm test.file.*
done

rm test.file*
