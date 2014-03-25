EXTRA_DIST = libefa.tex

dist_doc_DATA = libefa.pdf

# PDF targets
PDFTARGETS=libefa.pdf

all-local: $(PDFLATEX)

pdfdocs: $(PDFTARGETS)
%.pdf: %.tex
	$(PDFLATEX) $<

clean-local:
	rm -f *.pdf *.aux *.log
