all : raytracer

raytracer : raytracer.o
	g++ -o raytracer raytracer.o 

raytracer.o : raytracer.cpp
	g++ -c raytracer.cpp

clean :
	rm raytracer *.o
