"""Ball physics — no rendering dependency.

Contains the two functions worth benchmarking: ``update`` (position +
wall reflection) and ``speed`` (scalar magnitude). Both a pure-Python
and a numpy-vectorised variant are provided so the benchmark can show
the difference.
"""

import math
from dataclasses import dataclass


@dataclass
class Ball:
    """Mutable state for a single bouncing ball.

    Attributes:
        x:  Centre x position (pixels).
        y:  Centre y position (pixels).
        vx: Horizontal velocity (pixels/frame).
        vy: Vertical velocity (pixels/frame).
    """

    x: float
    y: float
    vx: float
    vy: float


def update(ball: Ball, radius: int, width: int, height: int) -> None:
    """Advance ball by one frame; reflect elastically off walls.

    Modifies ball in place. Position is updated first, then each velocity
    component is negated when the ball edge crosses a boundary.

    Args:
        ball:   Ball state, modified in place.
        radius: Ball radius in pixels (must be > 0).
        width:  Window width in pixels.
        height: Window height in pixels.
    """
    ball.x += ball.vx
    ball.y += ball.vy
    if ball.x - radius < 0 or ball.x + radius > width:
        ball.vx = -ball.vx
    if ball.y - radius < 0 or ball.y + radius > height:
        ball.vy = -ball.vy


def speed(ball: Ball) -> float:
    """Return the scalar speed of the ball in pixels/frame.

    Args:
        ball: Ball state (read-only).

    Returns:
        sqrt(vx² + vy²).
    """
    return math.sqrt(ball.vx ** 2 + ball.vy ** 2)
