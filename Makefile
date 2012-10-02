FLAGS = -I/usr/include/GL
LIBS = -L/usr/include -lglut -lglui -lGLU -lGL -lm

APPS = 2dcube

all: $(APPS)

2dcube: 2dcube.cpp matrix.cpp
	g++ 2dcube.cpp matrix.cpp -o 2dcube $(FLAGS) $(LIBS)

clean:
	@rm -rf $(APPS)

