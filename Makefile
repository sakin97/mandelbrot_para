.PHONY : clean

mandel : mandelbrot.c
	gcc -Wall -o $@ $^ -lglut -lGLU -lGL -lm -pthread

mandeltime : mandelbrottime.c
	gcc -Wall -o $@ $^ -lglut -lGLU -lGL -lm -pthread

clean: 
	rm -f mandel
