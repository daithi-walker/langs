/**
 * N-body gravitational simulation — N=1000 bodies, O(n²) direct pairwise forces.
 */
public class NBody {

    static final int N        = 1000;
    static final double G     = 6.674e-11;
    static final double SOFT  = 1e-4;

    final double[] x, y, z, vx, vy, vz, mass;

    public NBody() {
        x = new double[N]; y = new double[N]; z = new double[N];
        vx = new double[N]; vy = new double[N]; vz = new double[N];
        mass = new double[N];
    }

    public void init() {
        long s = 12345;
        for (int i = 0; i < N; i++) {
            s = s * 6364136223846793005L + 1442695040888963407L;
            x[i] = (double)((s >>> 33) & 0x7FFFFFFFL) / (double)(1L << 31) - 1.0;
            s = s * 6364136223846793005L + 1442695040888963407L;
            y[i] = (double)((s >>> 33) & 0x7FFFFFFFL) / (double)(1L << 31) - 1.0;
            s = s * 6364136223846793005L + 1442695040888963407L;
            z[i] = (double)((s >>> 33) & 0x7FFFFFFFL) / (double)(1L << 31) - 1.0;
            vx[i] = 0.0; vy[i] = 0.0; vz[i] = 0.0;
            mass[i] = 1.0;
        }
    }

    public void step(double dt) {
        double[] ax = new double[N];
        double[] ay = new double[N];
        double[] az = new double[N];
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (i == j) continue;
                double dx = x[j] - x[i];
                double dy = y[j] - y[i];
                double dz = z[j] - z[i];
                double r2 = dx*dx + dy*dy + dz*dz + SOFT*SOFT;
                double invR3 = 1.0 / (r2 * Math.sqrt(r2));
                double gm = G * mass[j];
                ax[i] += gm * dx * invR3;
                ay[i] += gm * dy * invR3;
                az[i] += gm * dz * invR3;
            }
        }
        for (int i = 0; i < N; i++) {
            vx[i] += ax[i] * dt;
            vy[i] += ay[i] * dt;
            vz[i] += az[i] * dt;
            x[i] += vx[i] * dt;
            y[i] += vy[i] * dt;
            z[i] += vz[i] * dt;
        }
    }
}
