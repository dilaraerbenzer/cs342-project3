
all: librsm.a app myapp

librsm.a:  rsm.c
	gcc -Wall -c rsm.c
	ar -cvq librsm.a rsm.o
	ranlib librsm.a

app: app.c
	gcc -Wall -o app app.c -L. -lrsm -lpthread -lrt

myapp: myapp.c
	gcc -Wall -o myapp myapp.c -L. -lrsm -lpthread -lrt

clean: 
	rm -fr *.o *.a *~ a.out  app rsm.o rsm.a librsm.a
