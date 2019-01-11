DOTDIR = report/dot
DOXDIR = report/doxygen
TEXDIR = report/tex
TEXINCDIR = $(TEXDIR)/include

CDIR = client
SDIR = server
UDIR = report/utils

CFLOW=cflow --level "0= "
SIMPLECFLOW=grep -v -f cflow.ignore
CFLOW2DOT=$(UDIR)/cflow2dot

REPORT = report.pdf

# latex -> pdf
PDFLATEX = pdflatex -interaction=nonstopmode

# Отчёт. PDFLATEX вызывается дважды для нормального 
# создания ссылок, это НЕ опечатка.
$(REPORT): $(TEXS) $(addprefix $(TEXINCDIR)/, ccflow01_dot.pdf scflow01_dot.pdf ccflow02_dot.pdf scflow02_dot.pdf)
	cd $(TEXDIR) && $(PDFLATEX) report.tex && $(PDFLATEX) report.tex && cp $(REPORT) ..

# Файлы latex
TEXS = $(wildcard $(TEXDIR)/*.tex)

# .dot -> _dot.tex
$(TEXINCDIR)/%_dot.tex: $(DOTDIR)/%.dot
	dot2tex -ftikz --autosize --crop  $< > $@

# _dot.tex -> _dot.pdf
$(TEXINCDIR)/%_dot.pdf: $(TEXINCDIR)/%_dot.tex
	$(PDFLATEX) -output-directory $(TEXINCDIR) $<

$(DOTDIR)/ccflow01.dot: $(addprefix $(CDIR)/, controller.c file_viewer.c worker.c dictionary.c)
	$(CFLOW) $^ | $(SIMPLECFLOW) | $(CFLOW2DOT) > $@

$(DOTDIR)/scflow01.dot: $(addprefix $(SDIR)/, socket.c server.c process.c message.c)
	$(CFLOW) $^ | $(SIMPLECFLOW) | $(CFLOW2DOT) > $@

$(DOTDIR)/ccflow02.dot: $(addprefix $(CDIR)/, smtp_interface.c)
	$(CFLOW) $^ | $(SIMPLECFLOW) | $(CFLOW2DOT) > $@

$(DOTDIR)/scflow02.dot: $(addprefix $(SDIR)/, smtp.c)
	$(CFLOW) $^ | $(SIMPLECFLOW) | $(CFLOW2DOT) > $@

clean:
	rm -rf  $(DOXDIR)/* $(DOTDIR)/*.dot $(TEXINCDIR)/*;
