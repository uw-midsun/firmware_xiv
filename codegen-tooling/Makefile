.PHONY: gen
gen: protos
	@echo "Generating from templates..."
	@python codegen/build.py 
	@find out -type f \( -iname '*.[ch]' -o -iname '*.ts' \) | xargs -r clang-format -i -fallback-style=Google
	@find out -type f \( -iname '*.go'  \) | xargs -r gofmt -w

.PHONY: gen-dbc
gen-dbc:
	@echo "Generating DBC file"
	@python codegen/build_dbc.py

.PHONY: lint
lint:
	@echo "Linting..."
	@pylint --disable=F0401 codegen/

.PHONY: protos 
protos:
	@echo "Compiling protos..."
	@mkdir -p genfiles
	@protoc -I=schema --python_out=genfiles --go_out=genfiles schema/can.proto

.PHONY: test
test: gen
	@echo "Testing..."
	@python -m unittest discover -s codegen

.PHONY: clean
clean:
	@rm -rf genfiles out
