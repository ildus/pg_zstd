CREATE FUNCTION zstd_handler(INTERNAL)
RETURNS compression_am_handler
AS 'pg_zstd', 'zstd_handler'
LANGUAGE C STRICT;

CREATE ACCESS METHOD pg_zstd
TYPE COMPRESSION
HANDLER zstd_handler;
