/**
 * Benchmark one nbody step — N=1000 bodies, O(n²) pairwise gravity.
 *
 * Outputs JSON: {"language":"java","example":"nbody","ns_per_op":...,"iterations":...}
 */
public class BenchNBody {

    private static final int WARMUP = 20;
    private static final int ITERS  = 200;

    public static void main(String[] args) {
        NBody sim = new NBody();
        sim.init();

        for (int i = 0; i < WARMUP; i++) sim.step(1e-3);
        sim.init();

        long t0 = System.nanoTime();
        for (int i = 0; i < ITERS; i++) sim.step(1e-3);
        long t1 = System.nanoTime();

        double ns = (double)(t1 - t0) / ITERS;
        System.out.printf(
            "{\"language\":\"java\",\"example\":\"nbody\",\"ns_per_op\":%.2f,\"iterations\":%d}%n",
            ns, ITERS);
    }
}
