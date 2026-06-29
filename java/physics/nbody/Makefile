JAVAC  = javac
JAVA   = java
OUTDIR = out

.PHONY: bench clean

bench: $(OUTDIR)/NBody.class
	$(JAVA) -cp $(OUTDIR) BenchNBody

$(OUTDIR)/NBody.class: src/NBody.java src/BenchNBody.java
	mkdir -p $(OUTDIR)
	$(JAVAC) -d $(OUTDIR) src/NBody.java src/BenchNBody.java

clean:
	rm -rf $(OUTDIR)
