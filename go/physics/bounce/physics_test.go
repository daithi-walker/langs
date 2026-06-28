package main

import (
	"math"
	"testing"
)

func TestNormalMovement(t *testing.T) {
	b := Ball{X: 100, Y: 100, VX: 4, VY: 3}
	b.Update(10, 800, 600)
	if math.Abs(float64(b.X-104)) > 0.01 {
		t.Errorf("expected X=104, got %f", b.X)
	}
	if math.Abs(float64(b.Y-103)) > 0.01 {
		t.Errorf("expected Y=103, got %f", b.Y)
	}
}

func TestLeftWallReflection(t *testing.T) {
	b := Ball{X: 10, Y: 100, VX: -5, VY: 3}
	b.Update(10, 800, 600)
	if math.Abs(float64(b.VX-5)) > 0.01 {
		t.Errorf("expected VX=5 after left bounce, got %f", b.VX)
	}
}

func TestRightWallReflection(t *testing.T) {
	b := Ball{X: 794, Y: 100, VX: 5, VY: 3}
	b.Update(10, 800, 600)
	if math.Abs(float64(b.VX+5)) > 0.01 {
		t.Errorf("expected VX=-5 after right bounce, got %f", b.VX)
	}
}

func TestTopWallReflection(t *testing.T) {
	b := Ball{X: 100, Y: 8, VX: 4, VY: -5}
	b.Update(10, 800, 600)
	if math.Abs(float64(b.VY-5)) > 0.01 {
		t.Errorf("expected VY=5 after top bounce, got %f", b.VY)
	}
}

func TestBottomWallReflection(t *testing.T) {
	b := Ball{X: 100, Y: 594, VX: 4, VY: 5}
	b.Update(10, 800, 600)
	if math.Abs(float64(b.VY+5)) > 0.01 {
		t.Errorf("expected VY=-5 after bottom bounce, got %f", b.VY)
	}
}

func TestSpeedIsPythagorean(t *testing.T) {
	b := Ball{VX: 3, VY: 4}
	if math.Abs(float64(b.Speed()-5)) > 0.001 {
		t.Errorf("expected speed=5, got %f", b.Speed())
	}
}

func TestSpeedPreservedOnBounce(t *testing.T) {
	b := Ball{X: 10, Y: 100, VX: -3, VY: 4}
	before := b.Speed()
	b.Update(10, 800, 600)
	if math.Abs(float64(b.Speed()-before)) > 0.001 {
		t.Errorf("speed changed on bounce: before=%f after=%f", before, b.Speed())
	}
}

func BenchmarkBallUpdate(b *testing.B) {
	ball := Ball{X: 400, Y: 300, VX: 4, VY: 3}
	for b.Loop() {
		ball.Update(30, 800, 600)
	}
}
