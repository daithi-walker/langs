/**
 * Benchmark the split-operator TDSE step.
 *
 * Outputs JSON: {"language":"java","example":"wave_packet","ns_per_op":...,"iterations":...}
 */
public class BenchWavePacket {

    private static final int WARMUP = 200;
    private static final int ITERS  = 2_000;

    public static void main(String[] args) {
        WavePacket sim = new WavePacket(WavePacket.BARRIER_H_DEF);

        // Warmup — let JIT compile the hot path
        for (int i = 0; i < WARMUP; i++) sim.step();
        sim.initPsi();

        long t0 = System.nanoTime();
        for (int i = 0; i < ITERS; i++) sim.step();
        long t1 = System.nanoTime();

        double ns = (double)(t1 - t0) / ITERS;
        System.out.printf(
            "{\"language\":\"java\",\"example\":\"wave_packet\",\"ns_per_op\":%.2f,\"iterations\":%d}%n",
            ns, ITERS);
    }
}
