JAVAC = javac
JAVA  = java
JUNIT = lib/junit-platform-console-standalone.jar
OUT   = out

.PHONY: all test bench clean deps

all: $(OUT)/Main.class

deps:
	@mkdir -p lib
	@[ -f $(JUNIT) ] || curl -sL \
	  "https://repo1.maven.org/maven2/org/junit/platform/junit-platform-console-standalone/1.10.2/junit-platform-console-standalone-1.10.2.jar" \
	  -o $(JUNIT)
	@echo "JUnit downloaded."

$(OUT)/Main.class: src/BallPhysics.java src/Main.java
	@mkdir -p $(OUT)
	$(JAVAC) -d $(OUT) src/BallPhysics.java src/Main.java

test: deps $(OUT)/BallPhysics.class
	@mkdir -p $(OUT)
	$(JAVAC) -cp $(JUNIT):$(OUT) -d $(OUT) test/TestBallPhysics.java
	$(JAVA) -jar $(JUNIT) --class-path $(OUT) --scan-class-path

bench: $(OUT)/BenchBounce.class
	$(JAVA) -cp $(OUT) BenchBounce

$(OUT)/BallPhysics.class: src/BallPhysics.java
	@mkdir -p $(OUT)
	$(JAVAC) -d $(OUT) src/BallPhysics.java

$(OUT)/BenchBounce.class: src/BenchBounce.java src/BallPhysics.java
	@mkdir -p $(OUT)
	$(JAVAC) -d $(OUT) src/BallPhysics.java src/BenchBounce.java

clean:
	rm -rf $(OUT)
