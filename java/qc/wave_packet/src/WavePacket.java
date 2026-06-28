/**
 * Split-operator TDSE physics for a 1D wave packet.
 *
 * Identical parameters to c/qc/wave_packet/bench.c.
 * Uses a pure-Java Cooley-Tukey FFT (no external dependencies).
 */
public class WavePacket {

    public static final int    N             = 1024;
    public static final double DX            = 0.1;
    public static final double DT            = 0.004;
    public static final double MASS          = 1.0;
    public static final double X0            = N * DX * 0.25;
    public static final double SIGMA         = N * DX * 0.06;
    public static final double K0            = 4.0;
    public static final double BARRIER_X0    = N * DX * 0.55;
    public static final double BARRIER_WIDTH = N * DX * 0.04;
    public static final double BARRIER_H_DEF = 8.0;

    /** Real and imaginary parts of ψ, phase_v, phase_t stored as parallel arrays. */
    public  double[] psiRe, psiIm;
    private double[] phaseVRe, phaseVIm;
    private double[] phaseTRe, phaseTIm;

    public WavePacket(double barrierHeight) {
        psiRe    = new double[N]; psiIm    = new double[N];
        phaseVRe = new double[N]; phaseVIm = new double[N];
        phaseTRe = new double[N]; phaseTIm = new double[N];
        buildPhases(barrierHeight);
        initPsi();
    }

    private void buildPhases(double barrierHeight) {
        for (int i = 0; i < N; i++) {
            double x = i * DX;
            double d = Math.abs(x - BARRIER_X0);
            double v = (d < BARRIER_WIDTH * 0.5) ? barrierHeight : 0.0;
            double angle = -v * DT * 0.5;
            phaseVRe[i] = Math.cos(angle);
            phaseVIm[i] = Math.sin(angle);
        }
        for (int j = 0; j < N; j++) {
            double kj = (j <= N / 2) ? j : j - N;
            kj *= 2.0 * Math.PI / (N * DX);
            double angle = -(kj * kj) * DT / (2.0 * MASS);
            phaseTRe[j] = Math.cos(angle);
            phaseTIm[j] = Math.sin(angle);
        }
    }

    public void initPsi() {
        double norm = 0.0;
        for (int i = 0; i < N; i++) {
            double x   = i * DX;
            double env = Math.exp(-(x - X0) * (x - X0) / (4.0 * SIGMA * SIGMA));
            psiRe[i] = env * Math.cos(K0 * x);
            psiIm[i] = env * Math.sin(K0 * x);
            norm += psiRe[i] * psiRe[i] + psiIm[i] * psiIm[i];
        }
        norm = Math.sqrt(norm * DX);
        for (int i = 0; i < N; i++) { psiRe[i] /= norm; psiIm[i] /= norm; }
    }

    /** One split-operator TDSE step — half V, full T (via FFT), half V. */
    public void step() {
        // Half V-step
        for (int i = 0; i < N; i++) {
            double re = psiRe[i] * phaseVRe[i] - psiIm[i] * phaseVIm[i];
            double im = psiRe[i] * phaseVIm[i] + psiIm[i] * phaseVRe[i];
            psiRe[i] = re; psiIm[i] = im;
        }

        // FFT forward
        fft(psiRe, psiIm, false);
        double invN = 1.0 / N;
        for (int j = 0; j < N; j++) {
            double re = (psiRe[j] * invN) * phaseTRe[j] - (psiIm[j] * invN) * phaseTIm[j];
            double im = (psiRe[j] * invN) * phaseTIm[j] + (psiIm[j] * invN) * phaseTRe[j];
            psiRe[j] = re; psiIm[j] = im;
        }
        // FFT inverse
        fft(psiRe, psiIm, true);

        // Half V-step
        for (int i = 0; i < N; i++) {
            double re = psiRe[i] * phaseVRe[i] - psiIm[i] * phaseVIm[i];
            double im = psiRe[i] * phaseVIm[i] + psiIm[i] * phaseVRe[i];
            psiRe[i] = re; psiIm[i] = im;
        }
    }

    /** In-place Cooley-Tukey FFT. N must be power of 2. */
    private static void fft(double[] re, double[] im, boolean inverse) {
        int n = re.length;
        // Bit-reversal
        for (int i = 1, j = 0; i < n; i++) {
            int bit = n >> 1;
            for (; (j & bit) != 0; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) {
                double t = re[i]; re[i] = re[j]; re[j] = t;
                t = im[i]; im[i] = im[j]; im[j] = t;
            }
        }
        // Butterfly
        for (int len = 2; len <= n; len <<= 1) {
            double ang = 2.0 * Math.PI / len * (inverse ? 1 : -1);
            double wRe = Math.cos(ang), wIm = Math.sin(ang);
            for (int i = 0; i < n; i += len) {
                double curRe = 1, curIm = 0;
                for (int k = 0; k < len / 2; k++) {
                    double uRe = re[i+k], uIm = im[i+k];
                    double vRe = re[i+k+len/2]*curRe - im[i+k+len/2]*curIm;
                    double vIm = re[i+k+len/2]*curIm + im[i+k+len/2]*curRe;
                    re[i+k] = uRe+vRe; im[i+k] = uIm+vIm;
                    re[i+k+len/2] = uRe-vRe; im[i+k+len/2] = uIm-vIm;
                    double nextRe = curRe*wRe - curIm*wIm;
                    curIm = curRe*wIm + curIm*wRe;
                    curRe = nextRe;
                }
            }
        }
    }
}
