#!/bin/bash

dimentions=(2 10 100)
points=(1000 10000 100000)
clusters=(10 100 1000)

for d in "${dimentions[@]}"; do
	for p in "${points[@]}"; do
		for c in "${clusters[@]}"; do			
			`./data-gen $d $p $c "datum/data-$d-$p-$c.txt"`
		done
	done
done