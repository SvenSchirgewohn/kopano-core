/*
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright 2005 - 2016 Zarafa and its licensors
 */

#ifndef ECMEMTABLE_H
#define ECMEMTABLE_H

#include <kopano/zcdefs.h>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <mapitags.h>
#include <mapidefs.h>
#include <kopano/ECKeyTable.h>
#include <kopano/ECUnknown.h>
#include <kopano/memory.hpp>
#include <kopano/ustringutil.h>
#include <kopano/Util.h>

namespace KC {

struct ECTableEntry {
	memory_ptr<SPropValue> lpsPropVal, lpsID;
	BOOL			fDeleted;
	BOOL			fDirty;
	BOOL			fNew;
	ULONG			cValues;
};

struct ECMEMADVISE {
	ULONG				ulEventMask;
	object_ptr<IMAPIAdviseSink> lpAdviseSink;
	//ULONG				ulConnection;
};

typedef std::map<int, ECMEMADVISE *> ECMapMemAdvise;


/* Status returned in HrGetAllWithStatus() */
#define ECROW_NORMAL	0
#define ECROW_ADDED		1
#define ECROW_MODIFIED	2
#define ECROW_DELETED	3

/*
 * This is a client-side implementation of IMAPITable, based on 
 * SPropValue's. You can add/delete/modify data through HrModifyRow
 * and you can get the data from its IMAPITable interface
 *
 * We use the ECKeyTable engine for the actual cursor/sorting system
 */
class ECMemTableView;

class _kc_export ECMemTable : public ECUnknown {
protected:
	ECMemTable(const SPropTagArray *lpsPropTagArray, ULONG ulRowPropTag);
	virtual ~ECMemTable();
public:
	static HRESULT Create(const SPropTagArray *lpsPropTagArray, ULONG ulRowPropTag, ECMemTable **lppRecipTable);
	virtual HRESULT QueryInterface(REFIID refiid, void **iface) _kc_override;
	virtual HRESULT HrGetView(const ECLocale &locale, ULONG ulFlags, ECMemTableView **lpView);
	virtual HRESULT HrModifyRow(ULONG flags, const SPropValue *id, const SPropValue *prop, ULONG n);
	virtual HRESULT HrUpdateRowID(LPSPropValue lpId, LPSPropValue lpProps, ULONG cValues);

	virtual HRESULT HrClear();

	virtual HRESULT HrDeleteAll();

	// Get the modified, deleted and added tables in the row
	virtual HRESULT HrGetAllWithStatus(LPSRowSet *lppRowSet, LPSPropValue *lppIDs, LPULONG *lppulStatus);
	virtual HRESULT HrGetRowID(LPSPropValue lpRow, LPSPropValue *lpID);
	virtual HRESULT HrGetRowData(LPSPropValue lpRow, ULONG *lpcValues, LPSPropValue *lppRowData);

	// Update all rows as being clean, remove deleted rows
	virtual HRESULT HrSetClean();

protected:
	// Data
	std::map<unsigned int, ECTableEntry>	mapRows;
	std::vector<ECMemTableView *>			lstViews;
	ULONG									ulRowPropTag;
	std::unique_ptr<SPropTagArray> lpsColumns;
	std::recursive_mutex m_hDataMutex;

	friend class ECMemTableView;
	ALLOC_WRAP_FRIEND;
};

class _kc_export ECMemTableView KC_FINAL_OPG :
    public ECUnknown, public IMAPITable {
protected:
	_kc_hidden ECMemTableView(ECMemTable *, const ECLocale &, ULONG flags);
	_kc_hidden virtual ~ECMemTableView(void);
public:
	_kc_hidden static HRESULT Create(ECMemTable *, const ECLocale &, ULONG flags, ECMemTableView **ret);
	virtual HRESULT QueryInterface(REFIID refiid, void **iface) _kc_override;
	_kc_hidden virtual HRESULT UpdateRow(ULONG update_type, ULONG id);
	_kc_hidden virtual HRESULT Clear(void);
	_kc_hidden virtual HRESULT GetLastError(HRESULT, ULONG flags, MAPIERROR **ret) _kc_override;
	_kc_hidden virtual HRESULT Advise(ULONG event_mask, IMAPIAdviseSink *, ULONG *conn) _kc_override;
	_kc_hidden virtual HRESULT Unadvise(ULONG conn) _kc_override;
	_kc_hidden virtual HRESULT GetStatus(ULONG *table_status, ULONG *table_type) _kc_override;
	virtual HRESULT SetColumns(const SPropTagArray *lpPropTagArray, ULONG ulFlags) _kc_override;
	virtual HRESULT QueryColumns(ULONG ulFlags, SPropTagArray **lpPropTagArray) _kc_override;
	_kc_hidden virtual HRESULT GetRowCount(ULONG flags, ULONG *count) _kc_override;
	_kc_hidden virtual HRESULT SeekRow(BOOKMARK origin, LONG row_count, LONG *rows_sought) _kc_override;
	_kc_hidden virtual HRESULT SeekRowApprox(ULONG numerator, ULONG denominator) _kc_override;
	_kc_hidden virtual HRESULT QueryPosition(ULONG *row, ULONG *numerator, ULONG *denominator) _kc_override;
	_kc_hidden virtual HRESULT FindRow(const SRestriction *, BOOKMARK origin, ULONG flags) _kc_override;
	_kc_hidden virtual HRESULT Restrict(const SRestriction *, ULONG flags) _kc_override;
	_kc_hidden virtual HRESULT CreateBookmark(BOOKMARK *pos) _kc_override;
	_kc_hidden virtual HRESULT FreeBookmark(BOOKMARK pos) _kc_override;
	_kc_hidden virtual HRESULT SortTable(const SSortOrderSet *sort_crit, ULONG flags) _kc_override;
	_kc_hidden virtual HRESULT QuerySortOrder(SSortOrderSet **sort_crit) _kc_override;
	virtual HRESULT QueryRows(LONG lRowCount, ULONG ulFlags, SRowSet **lppRows) _kc_override;
	_kc_hidden virtual HRESULT Abort() _kc_override;
	_kc_hidden virtual HRESULT ExpandRow(ULONG ikey_size, BYTE *ikey, ULONG row_count, ULONG flags, SRowSet **rows, ULONG *more_rows) _kc_override;
	_kc_hidden virtual HRESULT CollapseRow(ULONG ikey_size, BYTE *ikey, ULONG flags, ULONG *row_count) _kc_override;
	_kc_hidden virtual HRESULT WaitForCompletion(ULONG flags, ULONG timeout, ULONG *table_status) _kc_override;
	_kc_hidden virtual HRESULT GetCollapseState(ULONG flags, ULONG ikey_size, BYTE *ikey, ULONG *collapse_size, BYTE **collapse_state) _kc_override;
	_kc_hidden virtual HRESULT SetCollapseState(ULONG flags, ULONG collapse_size, BYTE *collapse_state, BOOKMARK *location) _kc_override;

private:
	_kc_hidden HRESULT GetBinarySortKey(const SPropValue *pv, ECSortCol &);
	_kc_hidden HRESULT ModifyRowKey(sObjectTableKey *row_item, sObjectTableKey *prev_row, ULONG *action);
	_kc_hidden HRESULT QueryRowData(const ECObjectTableList *row_list, SRowSet **rows);
	_kc_hidden HRESULT Notify(ULONG table_event, sObjectTableKey *row_item, sObjectTableKey *prev_row);

	ECKeyTable lpKeyTable;
	ECMemTable *			lpMemTable;
	ECMapMemAdvise			m_mapAdvise;
	std::unique_ptr<SSortOrderSet> lpsSortOrderSet;
	std::unique_ptr<SPropTagArray> lpsPropTags; /* columns */
	memory_ptr<SRestriction> lpsRestriction;
	ECLocale				m_locale;
	ULONG m_ulConnection = 1; // Next advise id
	ULONG					m_ulFlags;

	_kc_hidden virtual HRESULT UpdateSortOrRestrict(void);
	ALLOC_WRAP_FRIEND;
};

} /* namespace */

#endif // ECMemTable_H
