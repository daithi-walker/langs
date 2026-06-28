//! Ball physics — no rendering dependency.
//!
//! Contains `Ball` and its two methods: `update` (position + wall reflection)
//! and `speed` (scalar magnitude). This is the unit under test and the
//! function timed in benchmarks.

/// Mutable state for a single bouncing ball.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Ball {
    /// Centre x position (pixels).
    pub x: f32,
    /// Centre y position (pixels).
    pub y: f32,
    /// Horizontal velocity (pixels/frame).
    pub vx: f32,
    /// Vertical velocity (pixels/frame).
    pub vy: f32,
}

impl Ball {
    /// Creates a new ball at the given position with the given velocity.
    pub fn new(x: f32, y: f32, vx: f32, vy: f32) -> Self {
        Self { x, y, vx, vy }
    }

    /// Advances the ball by one frame and reflects elastically off walls.
    ///
    /// Position is updated first, then each velocity component is negated
    /// when the ball edge crosses a boundary. Reflection is lossless.
    ///
    /// # Arguments
    /// * `radius` - Ball radius in pixels (must be > 0).
    /// * `width`  - Window width in pixels.
    /// * `height` - Window height in pixels.
    #[inline]
    pub fn update(&mut self, radius: i32, width: i32, height: i32) {
        self.x += self.vx;
        self.y += self.vy;
        let r = radius as f32;
        if self.x - r < 0.0 || self.x + r > width as f32  { self.vx = -self.vx; }
        if self.y - r < 0.0 || self.y + r > height as f32 { self.vy = -self.vy; }
    }

    /// Returns the scalar speed of the ball in pixels/frame.
    #[inline]
    pub fn speed(&self) -> f32 {
        (self.vx * self.vx + self.vy * self.vy).sqrt()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_normal_movement() {
        let mut b = Ball::new(100.0, 100.0, 4.0, 3.0);
        b.update(10, 800, 600);
        assert!((b.x - 104.0).abs() < 0.01);
        assert!((b.y - 103.0).abs() < 0.01);
    }

    #[test]
    fn test_left_wall_reflection() {
        let mut b = Ball::new(10.0, 100.0, -5.0, 3.0);
        b.update(10, 800, 600);
        assert!((b.vx - 5.0).abs() < 0.01, "vx should flip to +5, got {}", b.vx);
    }

    #[test]
    fn test_right_wall_reflection() {
        let mut b = Ball::new(794.0, 100.0, 5.0, 3.0);
        b.update(10, 800, 600);
        assert!((b.vx + 5.0).abs() < 0.01, "vx should flip to -5, got {}", b.vx);
    }

    #[test]
    fn test_top_wall_reflection() {
        let mut b = Ball::new(100.0, 8.0, 4.0, -5.0);
        b.update(10, 800, 600);
        assert!((b.vy - 5.0).abs() < 0.01, "vy should flip to +5, got {}", b.vy);
    }

    #[test]
    fn test_bottom_wall_reflection() {
        let mut b = Ball::new(100.0, 594.0, 4.0, 5.0);
        b.update(10, 800, 600);
        assert!((b.vy + 5.0).abs() < 0.01, "vy should flip to -5, got {}", b.vy);
    }

    #[test]
    fn test_speed_is_pythagorean() {
        let b = Ball::new(0.0, 0.0, 3.0, 4.0);
        assert!((b.speed() - 5.0).abs() < 0.001);
    }

    #[test]
    fn test_speed_preserved_on_bounce() {
        let mut b = Ball::new(10.0, 100.0, -3.0, 4.0);
        let before = b.speed();
        b.update(10, 800, 600);
        assert!((b.speed() - before).abs() < 0.001);
    }
}
