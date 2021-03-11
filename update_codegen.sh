#!/usr/bin/env bash

cd projects/codegen-tooling
source .venv/bin/activate
make gen

cd ..
cd ..
ls
mv projects/codegen-tooling/out/* libraries/codegen-tooling/inc/