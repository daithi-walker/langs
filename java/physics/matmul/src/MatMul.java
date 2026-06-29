public class MatMul {

    public static final int N = 256;

    private final double[] a;
    private final double[] b;
    private final double[] c;

    public MatMul() {
        a = new double[N * N];
        b = new double[N * N];
        c = new double[N * N];
    }

    public void fill() {
        double n2 = N * N;
        for (int i = 0; i < N * N; i++) {
            a[i] = i / n2;
            b[i] = i / n2;
        }
    }

    public void multiply() {
        for (int i = 0; i < N; i++) {
            for (int k = 0; k < N; k++) {
                double aik = a[i * N + k];
                for (int j = 0; j < N; j++)
                    c[i * N + j] += aik * b[k * N + j];
            }
        }
    }

    public void clearC() {
        java.util.Arrays.fill(c, 0.0);
    }

    public double getC0() { return c[0]; }
}
