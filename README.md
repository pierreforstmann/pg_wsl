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

This extension can only work with PostgreSQL 15.

## Usage

Install `pg_wsl` on primary and standby node using above mentionned steps.

Add in primary instance and standby instance:

```
shared_preload_libraries='pg_wsl'
```

Restart primary and standby instances with:
```
pg_ctl stop
pg_ctl start
```
Create extension `pg_wsl` in primary instance:

```
create extension pg_wsl;
```

Connect to primary instance to run:

```
psql -c "select pg_wsl_rmgr('Hello from primary');"
```

Standby log should display something similar to:

```
2023-03-19 16:49:57.512 CET [50550] LOG:  pg_wsl_redo: message Hello from primary
2023-03-19 16:49:57.512 CET [50550] CONTEXT:  WAL redo at 0/110258A8 for pg_wsl/XLOG_WSL_RMGR: message Hello from primary (19 bytes)

```





