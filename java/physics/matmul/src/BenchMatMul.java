public class BenchMatMul {

    private static final int WARMUP = 5;
    private static final int ITERS  = 50;

    public static void main(String[] args) {
        MatMul mm = new MatMul();
        mm.fill();

        for (int i = 0; i < WARMUP; i++) { mm.clearC(); mm.multiply(); }
        mm.fill();

        long t0 = System.nanoTime();
        for (int i = 0; i < ITERS; i++) { mm.clearC(); mm.multiply(); }
        long t1 = System.nanoTime();

        double ns = (double)(t1 - t0) / ITERS;
        System.out.printf(
            "{\"language\":\"java\",\"example\":\"matmul\",\"ns_per_op\":%.2f,\"iterations\":%d}%n",
            ns, ITERS);
    }
}
