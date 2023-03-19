/*--------------------------------------------------------------------------
 *
 * pg_wsl.c
 *		Write to standby log through WAL.
 *
 * Portions Copyright (c) 1996-2023, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * Forked from  https://github.com/michaelpq/pg_plugins/signal_rmgr/signal_rmgr.c  
 *
 * IDENTIFICATION
 *		pg_wsl/pg_wsl.c
 *
 * The logic of the module is quite simple, so feel free to use it as a
 * template.  Note that upstream has an in-core module used for the purpose
 * of testing custom rmgrs in src/test/modules/test_custom_rgmrs/.
 *
 * -------------------------------------------------------------------------
 */

#include "postgres.h"

#include <signal.h>

#include "access/xlog.h"
#include "access/xlog_internal.h"
#include "access/xloginsert.h"
#include "fmgr.h"
#include "miscadmin.h"
#include "utils/builtins.h"
#include "utils/pg_lsn.h"

PG_MODULE_MAGIC;

/*
 * pg_wsl WAL records.
 */
typedef struct xl_wsl_rec
{
	Size		msg_size;	/* size of the message  */
	char		msg[FLEXIBLE_ARRAY_MEMBER];	/* payload */
} xl_wsl_rec;

#define SizeOfWslRmgr	(offsetof(xl_wsl_rec, msg))

/*
 * While developing or testing, use RM_EXPERIMENTAL_ID for rmid. For a real
 * extension, reserve a new resource manager ID to avoid conflicting with
 * other extensions; see:
 * https://wiki.postgresql.org/wiki/CustomWALResourceManagers
 *
 * Note that this conflicts with upstream's test_custom_rmgrs.
 */
#define RM_WSL_RMGR_ID		RM_EXPERIMENTAL_ID
#define PG_WSL_NAME		"pg_wsl"

/* RMGR API, see xlog_internal.h */
void		pg_wsl_rmgr_redo(XLogReaderState *record);
void		pg_wsl_rmgr_desc(StringInfo buf, XLogReaderState *record);
const char *pg_wsl_rmgr_identify(uint8 info);

static RmgrData pg_wsl_rmgr_rec = {
	.rm_name = PG_WSL_NAME,
	.rm_redo = pg_wsl_rmgr_redo,
	.rm_desc = pg_wsl_rmgr_desc,
	.rm_identify = pg_wsl_rmgr_identify
};

#define XLOG_WSL_RMGR	0x0

/* RMGR API implementation */

/*
 * The message is just logged.
 */
void
pg_wsl_rmgr_redo(XLogReaderState *record)
{
	uint8		info = XLogRecGetInfo(record) & ~XLR_INFO_MASK;
	xl_wsl_rec *xlrec;

	if (info != XLOG_WSL_RMGR)
		elog(PANIC, "pg_wsl_redo: unknown op code %u", info);

	xlrec = (xl_wsl_rec *) XLogRecGetData(record);

	elog(LOG, "pg_wsl_redo: message %s", xlrec->msg);

}

void
pg_wsl_rmgr_desc(StringInfo buf, XLogReaderState *record)
{
	char	   *rec = XLogRecGetData(record);
	uint8		info = XLogRecGetInfo(record) & ~XLR_INFO_MASK;

	if (info == XLOG_WSL_RMGR)
	{
		xl_wsl_rec *xlrec = (xl_wsl_rec *) rec;

		appendStringInfo(buf, "message %s (%zu bytes)",
						 xlrec->msg, xlrec->msg_size);
	}
}

const char *
pg_wsl_rmgr_identify(uint8 info)
{
	if ((info & ~XLR_INFO_MASK) == XLOG_WSL_RMGR)
		return "XLOG_WSL_RMGR";

	return NULL;
}

/*
 * SQL function for sending a message to a standby using custom WAL
 * resource managers.
 */
PG_FUNCTION_INFO_V1(pg_wsl_rmgr);
Datum
pg_wsl_rmgr(PG_FUNCTION_ARGS)
{
	text	   *arg = PG_GETARG_TEXT_PP(0);
	char	   *message = text_to_cstring(arg);
	XLogRecPtr	lsn;
	xl_wsl_rec xlrec;


	xlrec.msg_size = strlen(message) + 1;

	XLogBeginInsert();
	XLogRegisterData((char *) &xlrec, SizeOfWslRmgr);
	XLogRegisterData(message, strlen(message) + 1);

	/* Let's mark this record as unimportant, just in case. */
	XLogSetRecordFlags(XLOG_MARK_UNIMPORTANT);

	lsn = XLogInsert(RM_WSL_RMGR_ID, XLOG_WSL_RMGR);

	PG_RETURN_LSN(lsn);
}

/*
 * Module load callback
 */
void	_PG_init(void);
void
_PG_init(void)
{
	/*
	 * A WAL resource manager has to be loaded with shared_preload_libraries.
	 */
	RegisterCustomRmgr(RM_WSL_RMGR_ID, &pg_wsl_rmgr_rec);
}
