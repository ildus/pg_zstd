MODULE_big = pg_zstd
OBJS = pg_zstd.o $(WIN32RES)

EXTENSION = pg_zstd
EXTVERSION = 1.0
DATA = $(EXTENSION)--$(EXTVERSION).sql
PGFILEDESC = "zstd compression method"

REGRESS = basic

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

LDFLAGS += -lzstd
