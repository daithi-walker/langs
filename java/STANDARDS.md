# Java — Coding Standards

> Also read the top-level [`../STANDARDS.md`](../STANDARDS.md) first.

## Why Java?

Java runs on the JVM (Java Virtual Machine), which compiles bytecode to
native code at runtime using a JIT (Just-In-Time) compiler. After warm-up,
Java can approach C speed on numerical code. It is the dominant language
in enterprise software and Android development.

For benchmarking: the JVM has a 0.1–2 second startup cost and a warm-up
period before the JIT kicks in. We account for this by discarding the
first few runs. Warmed-up Java is often within 2–5× of C. Cold-start Java
is often 5–20× slower — both numbers tell a story.

---

## Toolchain

| Tool  | Version | Install |
|-------|---------|---------|
| java  | ≥ 21 (LTS) | `brew install openjdk@21` |
| javac | ≥ 21    | included |

Check: `java --version`, `javac --version`

Add to PATH if needed: `export PATH="/opt/homebrew/opt/openjdk@21/bin:$PATH"`

We use **plain Java** (no Maven, no Gradle) for simplicity. Build is a
shell script or Makefile calling `javac` directly.

---

## Project Structure

```
<example>/
├── src/
│   ├── Main.java       — entry point only; no logic
│   └── <Module>.java   — pure logic class
├── test/
│   └── Test<Module>.java  — tests using JUnit (embedded jar)
├── lib/
│   └── junit-platform-console-standalone.jar  — single-jar JUnit runner
└── Makefile
```

---

## Style

### Formatting
- 4-space indent. No tabs.
- Opening brace on the same line (`K&R` style, also the Java convention).
- Lines ≤ 100 characters.
- Use an editor with Java formatting support or `google-java-format`.

### Naming
| Thing           | Convention      | Example              |
|-----------------|-----------------|----------------------|
| Classes         | `PascalCase`    | `BallPhysics`        |
| Methods         | `camelCase`     | `updateBall`         |
| Variables       | `camelCase`     | `gridPsi2`           |
| Constants       | `UPPER_SNAKE`   | `RADIUS`             |
| Packages        | `lowercase`     | `physics`            |
| Files           | Must match class name | `BallPhysics.java` |

### Types
- Use `float` for physics and graphics (consistent with C baseline).
- Use `double` only where precision is required.
- Use `float[]` for arrays; avoid `ArrayList<Float>` in hot paths
  (boxing overhead is significant for benchmarks).
- No `null` returns from public methods — use `Optional<T>` or throw.

### Classes and Methods
- Logic classes are stateless where possible (all static methods,
  or immutable state). This makes testing trivial.
- Keep classes small. One class = one concept.
- No `public` fields. Use `private` fields with accessors only when
  needed.

### Error Handling
- Throw `IllegalArgumentException` for bad inputs at boundaries.
- Throw `IllegalStateException` for broken invariants.
- Checked exceptions (`throws`) only for I/O. Never for logic errors.
- Never silently swallow exceptions in a bare `catch (Exception e) {}`.

---

## Documentation

Java uses Javadoc `/** ... */`:

```java
/**
 * Advances the ball position by one frame and reflects off walls.
 *
 * <p>Modifies the {@code state} object in place. Reflection is elastic:
 * no energy is lost on bounce. Units are pixels and pixels-per-frame.
 *
 * @param state  mutable ball state object
 * @param width  window width in pixels
 * @param height window height in pixels
 */
public static void updateBall(BallState state, int width, int height) { ... }
```

Every public class and public method must have Javadoc.
Generate with: `javadoc -d docs src/*.java`

---

## Testing with JUnit 5

Download the standalone runner once per repo:
```bash
curl -sL https://repo1.maven.org/maven2/org/junit/platform/junit-platform-console-standalone/1.10.2/junit-platform-console-standalone-1.10.2.jar \
     -o lib/junit-platform-console-standalone.jar
```

```java
// test/TestBallPhysics.java
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

class TestBallPhysics {

    @Test
    void ballReflectsOffLeftWall() {
        BallState state = new BallState(5f, 100f, -4f, 3f);
        BallPhysics.updateBall(state, 10, 800, 600);
        assertEquals(4f, state.vx, 1e-5f, "vx should flip sign on left wall");
    }
}
```

Compile and run:
```bash
javac -cp lib/junit-platform-console-standalone.jar src/*.java test/*.java -d out/
java -jar lib/junit-platform-console-standalone.jar --class-path out/ --scan-class-path
```

---

## Benchmarking

Use JMH (Java Microbenchmark Harness) for rigorous benchmarks, or a
simple manual loop for quick comparisons. We use a manual loop here for
portability (no Maven required):

```java
// Warm up the JIT first — discard these results
for (int i = 0; i < 1000; i++) methodUnderTest();

// Then time
long t0 = System.nanoTime();
for (int i = 0; i < N; i++) methodUnderTest();
long t1 = System.nanoTime();
double ms = (t1 - t0) / 1e6;
System.out.printf("%.3f ms%n", ms / N);
```

Always warm up. A cold JVM benchmark is nearly meaningless.

---

## Common Pitfalls

| Pitfall | Rule |
|---------|------|
| Autoboxing in hot loops | use `float[]` not `List<Float>`; boxing allocates objects |
| JVM cold start | warm up 500–1000 iterations before timing |
| String concatenation in loops | use `StringBuilder` |
| `float` vs `double` literals | `1.0` is a `double`; write `1.0f` for float |
| Forgetting `static` | logic methods should be `static` unless they need object state |
