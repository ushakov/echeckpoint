#!/bin/bash

if [ "z$1" == "z" ]; then
   name=test
else
   name=$1
fi

sudo uisp -dprog=bsd --erase
sudo uisp -dprog=bsd --upload if=$name.hex
sudo uisp -dprog=bsd --verify if=$name.hex
