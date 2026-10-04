
all: kernel loader


loader: SCPS_150.97.ninja FORCE
	@echo Building SCPS_150.97...
	@ninja -f SCPS_150.97.ninja


kernel: KERNEL.XFF.ninja FORCE
	@echo Building KERNEL.XFF...
	@ninja -f KERNEL.XFF.ninja


build/SCPS_150.97/%.o: SCPS_150.97.ninja FORCE
	@ninja -f SCPS_150.97.ninja $@


build/KERNEL.XFF/%.o: KERNEL.XFF.ninja FORCE
	@ninja -f KERNEL.XFF.ninja $@


SCPS_150.97.ninja:
	@./configure.py -c -t loader


KERNEL.XFF.ninja:
	@./configure.py -c -t kernel


FORCE:


clean:
	@rm -rf asm assets build
	@rm -rf *.ninja
.PHONY : clean

# Local build + verified Preview disc + PCSX2 runtime check.
PYTHON ?= python3

pcsx2-prepare:
	$(PYTHON) scripts/pcsx2_check.py prepare

pcsx2-run:
	$(PYTHON) scripts/pcsx2_check.py run

pcsx2-check-config:
	$(PYTHON) scripts/pcsx2_check.py run --check-config

pcsx2: pcsx2-prepare
	$(PYTHON) scripts/pcsx2_check.py run

test-pipeline:
	$(PYTHON) -m unittest discover -s tools/tests -v

.PHONY: pcsx2 pcsx2-prepare pcsx2-run pcsx2-check-config test-pipeline

