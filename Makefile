CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread
SRC = *.cpp

VulkanTest: $(SRC)
	g++ $(CFLAGS) -o TriangleApp $(SRC) $(LDFLAGS)

.PHONY: test clean

test: TriangleApp
	./RemakeApp01

clean:
	rm -f TriangleApp
