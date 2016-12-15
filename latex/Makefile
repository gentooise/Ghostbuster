TEX=pdflatex

default: clean
	$(TEX) thesis.tex && $(TEX) thesis.tex

check:
	if [ -n "$(FILE)" ]; then aspell -l en_GB --mode=tex -c $(FILE); fi

clean:
	rm -f *.log
	rm -f *.toc
	rm -f *.aux
	rm -f *.xmpi
	rm -f *.out
