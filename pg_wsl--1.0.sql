/* pg_wsl/pg_wsl--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pg_wsl" to load this file. \quit

--
-- pg_wsl()
--
--
CREATE FUNCTION pg_wsl_rmgr(
    IN message text DEFAULT 'Hello',
    OUT lsn pg_lsn
)
AS 'MODULE_PATHNAME', 'pg_wsl_rmgr'
LANGUAGE C STRICT PARALLEL UNSAFE;
