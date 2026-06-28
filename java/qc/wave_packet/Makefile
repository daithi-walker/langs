JAVAC = javac
JAVA  = java
OUTDIR = out

.PHONY: bench clean

bench: $(OUTDIR)/WavePacket.class
	$(JAVA) -cp $(OUTDIR) BenchWavePacket

$(OUTDIR)/WavePacket.class: src/WavePacket.java src/BenchWavePacket.java
	mkdir -p $(OUTDIR)
	$(JAVAC) -d $(OUTDIR) src/WavePacket.java src/BenchWavePacket.java

clean:
	rm -rf $(OUTDIR)
