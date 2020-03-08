#!/bin/bash
search=("man 2 " "man 3 ")
index=0
while [ $index -lt 2 ]; do
    ${search[$index]} ${1}
    index=$(($index + 1))
done
