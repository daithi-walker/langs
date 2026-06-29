JAVAC  = javac
JAVA   = java
OUTDIR = out

.PHONY: bench clean

bench: $(OUTDIR)/MatMul.class
	$(JAVA) -cp $(OUTDIR) BenchMatMul

$(OUTDIR)/MatMul.class: src/MatMul.java src/BenchMatMul.java
	mkdir -p $(OUTDIR)
	$(JAVAC) -d $(OUTDIR) src/MatMul.java src/BenchMatMul.java

clean:
	rm -rf $(OUTDIR)
