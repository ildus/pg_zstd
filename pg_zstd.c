#include "postgres.h"
#include "fmgr.h"
#include "zstd.h"
#include "access/cmapi.h"
#include "commands/defrem.h"
#include "nodes/parsenodes.h"
#include "utils/builtins.h"
#include "utils/datum.h"


PG_MODULE_MAGIC;

typedef struct
{
	int	level;
} zstd_option;

PG_FUNCTION_INFO_V1( zstd_handler );

static void
zstd_check(Form_pg_attribute att, List *options)
{
	ListCell   *lc;

	foreach (lc, options)
	{
		DefElem    *def = (DefElem *) lfirst(lc);

		if (strcmp(def->defname, "level") == 0)
		{
			int level = pg_atoi(defGetString(def), 4, 0);

			if (level < 1)
				elog(WARNING, "Acceleration value <= 0 will be replaced by 1 (default)");
		}
		else
			elog(ERROR, "Unknown option '%s'", def->defname);
	}
}

static void *
zstd_initstate(Oid acoid, List *options)
{
	ListCell   *lc;
	zstd_option *opt = (zstd_option *) palloc(sizeof(zstd_option));

	/* default acceleration */
	opt->level = 1;

	/* iterate through user options */
	foreach (lc, options)
	{
		DefElem    *def = (DefElem *) lfirst(lc);

		if (strcmp(def->defname, "level") == 0)
			opt->level = pg_atoi(defGetString(def), 4, 0);
		else
			elog(ERROR, "Unknown option '%s'", def->defname);
	}

	return (void *) opt;
}

static bytea *
zstd_compress(CompressionAmOptions *cmoptions, const bytea *value)
{
	zstd_option *opt = (zstd_option *) cmoptions->acstate;
	int		src_len = (Size) VARSIZE_ANY_EXHDR(value);
	int		dst_len;
	int		len;
	bytea  *ret;

	dst_len = ZSTD_compressBound(src_len);
	ret = (bytea *) palloc(dst_len + VARHDRSZ_CUSTOM_COMPRESSED);

	len = ZSTD_compress((char *) ret + VARHDRSZ_CUSTOM_COMPRESSED,
						dst_len,
						(char *) VARDATA_ANY(value),
						src_len, opt->level);

	if (ZSTD_isError(len))
	{
		pfree(ret);
		return NULL;
	}

	SET_VARSIZE_COMPRESSED(ret, len + VARHDRSZ_CUSTOM_COMPRESSED);
	return ret;
}

static bytea *
zstd_decompress(CompressionAmOptions *cmoptions, const bytea *value)
{
	int		src_len = VARSIZE_ANY(value) - VARHDRSZ_CUSTOM_COMPRESSED;
	int		dst_len = VARRAWSIZE_4B_C(value);
	int		len;
	char   *data;
	bytea  *ret;

	data = (char *) value + VARHDRSZ_CUSTOM_COMPRESSED;
	len = ZSTD_getDecompressedSize(data, dst_len);

	if (len == ZSTD_CONTENTSIZE_ERROR)
		elog(ERROR, "decompression error: not zstd");
	else if (len == ZSTD_CONTENTSIZE_UNKNOWN)
		elog(ERROR, "can't determine original size");

	ret = (bytea *) palloc(dst_len + VARHDRSZ);
	SET_VARSIZE(ret, dst_len + VARHDRSZ);

	len = ZSTD_decompress(
		(char *) VARDATA(ret), dst_len,
		data, src_len);

	if (len != dst_len)
		elog(ERROR, "Decompression error: %s", ZSTD_getErrorName(len));

	return ret;
}

Datum
zstd_handler(PG_FUNCTION_ARGS)
{
	CompressionAmRoutine *routine = makeNode(CompressionAmRoutine);

	routine->cmcheck = zstd_check;
	routine->cminitstate = zstd_initstate;
	routine->cmcompress = zstd_compress;
	routine->cmdecompress = zstd_decompress;

	PG_RETURN_POINTER(routine);
}
