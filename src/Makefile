ifeq ($(OS),Windows_NT)
	GDAL_CFLAGS=-I/c/OSGeo4W/include
	GDAL_LIBS=/c/OSGeo4W/lib/gdal_i.lib
	LIB_NAME=midas.dll
	EXT=.exe
else
	GDAL_LIBS=`gdal-config --libs`
	LIB_NAME=libmidas.so
endif
CFLAGS=-Wall -Werror -O3 $(GDAL_CFLAGS)
LDFLAGS=-O3

all: $(LIB_NAME) mefa$(EXT) meshed$(EXT) melfp$(EXT)

clean:
	$(RM) *.o

LIB_OBJS=midas.o \
	 timeval_diff.o \
	 raster.o \
	 recode.o \
	 accumulate.o \
	 accumulate_lessmem.o \
	 accumulate_moremem.o \
	 outlet_list.o \
	 outlets.o \
	 delineate.o \
	 delineate_lessmem.o \
	 delineate_moremem.o \
	 shed_hier.o \
	 point_list.o \
	 lfp.o \
	 lfp_lessmem.o \
	 lfp_moremem.o \
	 write_lfp.o

$(LIB_OBJS): CFLAGS+=-fpic -fopenmp $(GDAL_CFLAGS)
$(LIB_NAME): LDFLAGS+=-shared -Wl,-soname,$(LIB_NAME) -fopenmp -lm $(GDAL_LIBS)

$(LIB_NAME): $(LIB_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

mefa$(EXT): mefa.o $(LIB_NAME)
	$(CC) $(LDFLAGS) -o $@ $^

meshed$(EXT): meshed.o $(LIB_NAME)
	$(CC) $(LDFLAGS) -o $@ $^

melfp$(EXT): melfp.o $(LIB_NAME)
	$(CC) $(LDFLAGS) -o $@ $^

*.o: global.h raster.h
accumulate*.o: accumulate_funcs.h
delineate*.o: delineate_funcs.h
lfp*.o: lfp_funcs.h
