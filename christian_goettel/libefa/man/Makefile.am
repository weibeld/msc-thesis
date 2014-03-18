EXTRA_DIST=$(wildcard *.pod) $(wildcard *.[0-9])

man1_MANS=libefa.1

%.1: %.pod
	pod2man -c "Libefa" -r "Libefa $(VERSION)" $< > $@
