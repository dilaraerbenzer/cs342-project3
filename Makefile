
all: librsm.a  app

librsm.a:  rsm.c
	gcc -Wall -c rsm.c
	ar -cvq librsm.a rsm.o
	ranlib librsm.a

app: app.c
	gcc -Wall -o app app.c -L. -lrsm -lpthread

clean: 
	rm -fr *.o *.a *~ a.out  app rsm.o rsm.a librsm.a
