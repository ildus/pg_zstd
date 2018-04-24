CREATE EXTENSION pg_zstd;
CREATE TABLE one(a TEXT COMPRESSION pg_zstd WITH (compressionlevel '1'));
CREATE TABLE one(a TEXT COMPRESSION pg_zstd WITH (level '1'));
CREATE TABLE two(a TEXT COMPRESSION pg_zstd WITH (level '2'));
CREATE TABLE three(a TEXT);
CREATE TABLE four(a TEXT);
ALTER TABLE four ALTER COLUMN a SET STORAGE external;

INSERT INTO one SELECT repeat('abcdefghijklmnopqrstuvwxyzzyxwvutsrqponmlkjihgfedcba', 1000) FROM generate_series(1, 1000) i;
INSERT INTO two SELECT repeat('abcdefghijklmnopqrstuvwxyzzyxwvutsrqponmlkjihgfedcba', 1000) FROM generate_series(1, 1000) i;
INSERT INTO three SELECT repeat('abcdefghijklmnopqrstuvwxyzzyxwvutsrqponmlkjihgfedcba', 1000) FROM generate_series(1, 1000) i;
INSERT INTO four SELECT repeat('abcdefghijklmnopqrstuvwxyzzyxwvutsrqponmlkjihgfedcba', 1000) FROM generate_series(1, 1000) i;

-- at least four times
SELECT pg_total_relation_size('one') < pg_total_relation_size('three') / 4;
SELECT pg_total_relation_size('two') < pg_total_relation_size('three') / 4;
SELECT pg_size_pretty(pg_total_relation_size('three'));
SELECT pg_size_pretty(pg_total_relation_size('four'));

SELECT repeat('abcdefghijklmnopqrstuvwxyzzyxwvutsrqponmlkjihgfedcba', 1000) = a FROM one LIMIT 1;
SELECT repeat('abcdefghijklmnopqrstuvwxyzzyxwvutsrqponmlkjihgfedcba', 1000) = a FROM two LIMIT 1;
