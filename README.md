# pg_wsl
PostgreSQL extension to write from primary to standby log 

## Compiling

This module can be built using the standard PGXS infrastructure. For this to work, the pg_config program must be available in your $PATH:

```
git clone https://github.com/pierreforstmann/pg_wsl.git
cd pg_wsl
make
make install
```

This extension can only work with PG 15.

