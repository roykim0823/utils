#!/bin/bash

if [ -r $1 ] && [ -s $1 ]
then
	echo This file is useful.
fi

USER='bob'

if [ $USER == 'bob' ] || [ $USER == 'andy' ]
then
	ls -alh
else
	ls
fi
