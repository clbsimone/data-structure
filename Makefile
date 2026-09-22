EXERCISES = es1_ASDLab es2_ASDLab es3_ASDLab es4_ASDLab
.PHONY: all test clean docs check
all test clean docs:
	@set -e; for dir in $(EXERCISES); do $(MAKE) -C $$dir $@; done
check: all test
	python3 tests/integration.py
