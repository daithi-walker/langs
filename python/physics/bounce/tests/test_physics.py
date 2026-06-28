"""Unit tests for bounce physics (update, speed)."""

import math
import pytest
from physics import Ball, update, speed


def test_normal_movement() -> None:
    b = Ball(x=100.0, y=100.0, vx=4.0, vy=3.0)
    update(b, radius=10, width=800, height=600)
    assert math.isclose(b.x, 104.0, abs_tol=0.01)
    assert math.isclose(b.y, 103.0, abs_tol=0.01)


def test_left_wall_reflection() -> None:
    b = Ball(x=10.0, y=100.0, vx=-5.0, vy=3.0)
    update(b, radius=10, width=800, height=600)
    assert math.isclose(b.vx, 5.0, abs_tol=0.01), f"expected vx=5.0, got {b.vx}"


def test_right_wall_reflection() -> None:
    b = Ball(x=794.0, y=100.0, vx=5.0, vy=3.0)
    update(b, radius=10, width=800, height=600)
    assert math.isclose(b.vx, -5.0, abs_tol=0.01), f"expected vx=-5.0, got {b.vx}"


def test_top_wall_reflection() -> None:
    b = Ball(x=100.0, y=8.0, vx=4.0, vy=-5.0)
    update(b, radius=10, width=800, height=600)
    assert math.isclose(b.vy, 5.0, abs_tol=0.01), f"expected vy=5.0, got {b.vy}"


def test_bottom_wall_reflection() -> None:
    b = Ball(x=100.0, y=594.0, vx=4.0, vy=5.0)
    update(b, radius=10, width=800, height=600)
    assert math.isclose(b.vy, -5.0, abs_tol=0.01), f"expected vy=-5.0, got {b.vy}"


def test_speed_is_pythagorean() -> None:
    b = Ball(x=0.0, y=0.0, vx=3.0, vy=4.0)
    assert math.isclose(speed(b), 5.0, abs_tol=0.001)


def test_speed_preserved_on_bounce() -> None:
    b = Ball(x=10.0, y=100.0, vx=-3.0, vy=4.0)
    before = speed(b)
    update(b, radius=10, width=800, height=600)
    assert math.isclose(speed(b), before, abs_tol=0.001), "speed changed on elastic bounce"
