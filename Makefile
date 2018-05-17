.PHONY : clean
all : mandel mandelv2 mandeltime

mandel : mandelbrot.c
	gcc -Wall -o $@ $^ -lglut -lGLU -lGL -lm -pthread -pg

mandeltime : mandelbrottime.c
	gcc -Wall -o $@ $^ -lglut -lGLU -lGL -lm -pthread

mandelv2: mandelbrotv2.c
	gcc -Wall -o $@ $^ -lglut -lGLU -lGL -lm -pthread -pg -lrt

clean: 
	rm -f mandel
