/**
 * Standalone benchmark — outputs JSON for the report generator.
 *
 * <p>Warms up the JIT with WARMUP iterations (not timed), then runs RUNS
 * timed iterations of {@link BallPhysics#update}. The volatile sink
 * prevents the JIT from eliminating the loop as dead code.
 *
 * <p>Run: {@code make bench}
 */
public class BenchBounce {

    private static final int WARMUP = 100_000;
    private static final int RUNS   = 1_000_000;

    public static void main(String[] args) {
        BallPhysics ball = new BallPhysics(400f, 300f, 4f, 3f);

        for (int i = 0; i < WARMUP; i++)
            ball.update(30, 800, 600);

        long t0 = System.nanoTime();
        for (int i = 0; i < RUNS; i++)
            ball.update(30, 800, 600);
        long t1 = System.nanoTime();

        /* Prevent JIT dead-code elimination */
        float sink = ball.x;
        if (sink < -1e9f) System.err.println(sink); // never prints; fools JIT

        double meanNs = (double)(t1 - t0) / RUNS;
        System.out.printf("{\"lang\":\"java\",\"example\":\"bounce\",\"mean_ns\":%.4f,\"n\":%d}%n",
                meanNs, RUNS);
    }
}
