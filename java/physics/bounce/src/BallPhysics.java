/**
 * Ball physics — no rendering dependency.
 *
 * <p>Contains the two methods worth benchmarking: {@link #update} (position
 * + wall reflection) and {@link #speed} (scalar magnitude). Keeping this
 * class free of SDL2 / AWT lets it be tested and benchmarked in isolation.
 */
public class BallPhysics {

    /** Mutable ball state (passed by reference via the enclosing object). */
    public float x, y, vx, vy;

    /**
     * Creates a ball at the given position and velocity.
     *
     * @param x  Initial centre x (pixels).
     * @param y  Initial centre y (pixels).
     * @param vx Initial horizontal velocity (pixels/frame).
     * @param vy Initial vertical velocity (pixels/frame).
     */
    public BallPhysics(float x, float y, float vx, float vy) {
        this.x = x; this.y = y; this.vx = vx; this.vy = vy;
    }

    /**
     * Advances the ball by one frame; reflects elastically off walls.
     *
     * <p>Position is updated first, then each velocity component is negated
     * when the ball edge crosses a boundary. No penetration correction.
     *
     * @param radius Ball radius in pixels (must be &gt; 0).
     * @param width  Window width in pixels.
     * @param height Window height in pixels.
     */
    public void update(int radius, int width, int height) {
        x += vx;
        y += vy;
        if (x - radius < 0 || x + radius > width)  vx = -vx;
        if (y - radius < 0 || y + radius > height) vy = -vy;
    }

    /**
     * Returns the scalar speed of the ball in pixels/frame.
     *
     * @return sqrt(vx² + vy²).
     */
    public float speed() {
        return (float) Math.sqrt(vx * vx + vy * vy);
    }
}
