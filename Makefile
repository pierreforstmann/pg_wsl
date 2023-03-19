MODULES = pg_wsl 

EXTENSION = pg_wsl
DATA = pg_wsl--1.0.sql
PGFILEDESC = "write to standby log from primary"

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
