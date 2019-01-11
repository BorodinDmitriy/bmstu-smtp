DOTDIR = report/dot
DOXDIR = report/doxygen
CDIR = client
SDIR = server
UDIR = report/utils

CFLOW=cflow --level "0= "
SIMPLECFLOW=grep -v -f cflow.ignore
CFLOW2DOT=$(UDIR)/cflow2dot

code: $(DOTDIR)/ccflow01.dot $(DOTDIR)/scflow01.dot

$(DOTDIR)/ccflow01.dot: $(addprefix $(CDIR)/, controller.c smtp_interface.c file_viewer.c worker.c dictionary.c)
	$(CFLOW) $^ | $(SIMPLECFLOW) | $(CFLOW2DOT) > $@

$(DOTDIR)/scflow01.dot: $(addprefix $(SDIR)/, socket.c smtp.c server.c process.c message.c)
	$(CFLOW) $^ | $(SIMPLECFLOW) | $(CFLOW2DOT) > $@

clean:
	rm -rf  $(DOXDIR)/* $(DOTDIR)/*.dot;
