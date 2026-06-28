import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Unit tests for {@link BallPhysics}.
 *
 * <p>Covers normal movement, all four wall reflections, speed calculation,
 * and speed conservation on bounce.
 */
class TestBallPhysics {

    @Test
    void normalMovement() {
        BallPhysics b = new BallPhysics(100f, 100f, 4f, 3f);
        b.update(10, 800, 600);
        assertEquals(104f, b.x, 0.01f);
        assertEquals(103f, b.y, 0.01f);
    }

    @Test
    void leftWallReflection() {
        BallPhysics b = new BallPhysics(10f, 100f, -5f, 3f);
        b.update(10, 800, 600);
        assertEquals(5f, b.vx, 0.01f, "vx should flip to +5 after left wall bounce");
    }

    @Test
    void rightWallReflection() {
        BallPhysics b = new BallPhysics(794f, 100f, 5f, 3f);
        b.update(10, 800, 600);
        assertEquals(-5f, b.vx, 0.01f, "vx should flip to -5 after right wall bounce");
    }

    @Test
    void topWallReflection() {
        BallPhysics b = new BallPhysics(100f, 8f, 4f, -5f);
        b.update(10, 800, 600);
        assertEquals(5f, b.vy, 0.01f, "vy should flip to +5 after top wall bounce");
    }

    @Test
    void bottomWallReflection() {
        BallPhysics b = new BallPhysics(100f, 594f, 4f, 5f);
        b.update(10, 800, 600);
        assertEquals(-5f, b.vy, 0.01f, "vy should flip to -5 after bottom wall bounce");
    }

    @Test
    void speedIsPythagorean() {
        BallPhysics b = new BallPhysics(0f, 0f, 3f, 4f);
        assertEquals(5f, b.speed(), 0.001f);
    }

    @Test
    void speedPreservedOnBounce() {
        BallPhysics b = new BallPhysics(10f, 100f, -3f, 4f);
        float before = b.speed();
        b.update(10, 800, 600);
        assertEquals(before, b.speed(), 0.001f, "speed should not change on elastic bounce");
    }
}
