/*
SageThumbs - Thumbnail image shell extension.

Copyright (C) Nikolay Raspopov, 2008-2017.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "StdAfx.h"
#include "SQLite.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
// CDatabase

CDatabase::CDatabase(LPCTSTR szDatabase)
	: m_db			( NULL )
	, m_st			( NULL )
	, m_bBusy		( false )
#ifdef _DEBUG
	, m_nThread		( GetCurrentThreadId() )
#endif
{
	sqlite3_enable_shared_cache( 1 );

#if UNICODE
	int res = sqlite3_open16( szDatabase, &m_db );
#else
	int res = sqlite3_open( szDatabase, &m_db );
#endif

	if ( res == SQLITE_OK )
	{
		sqlite3_busy_timeout( m_db, 100 ); // 0.1 s
	}
}

CDatabase::~CDatabase()
{
	ATLASSERT( m_nThread == GetCurrentThreadId() );	// Don't pass database across thread boundaries

	Finalize();

	if ( m_db )
	{
		sqlite3_close( m_db );
		m_db = NULL;
	}
}

CDatabase::operator bool() const
{
	return ( m_db != NULL );
}

CString CDatabase::GetLastErrorMessage() const
{
	return ( m_db ? CString( 
#if UNICODE
		(LPCWSTR)sqlite3_errmsg16( m_db )
#else
		sqlite3_errmsg( m_db )
#endif
		) : CString() );
}

bool CDatabase::IsBusy() const
{
	return m_bBusy;
}

int CDatabase::GetCount() const
{
	return (int)m_raw.GetCount();
}

bool CDatabase::Exec(LPCTSTR szQuery)
{
	ATLASSERT( szQuery && *szQuery );

	m_sQuery = szQuery;

	while ( PrepareHelper() )
	{
		bool ret = Step();

		Finalize();

		if ( m_sQuery.IsEmpty() )
			// Done
			return ret;
	}
	return false;
}

bool CDatabase::Prepare(LPCTSTR szQuery)
{
	ATLASSERT( szQuery && *szQuery );

	m_sQuery = szQuery;

	return PrepareHelper();
}

bool CDatabase::PrepareHelper()
{
	ATLASSERT( m_nThread == GetCurrentThreadId() );	// Don't pass database across thread boundaries
	ATLASSERT( m_db );

	if ( ! m_db )
		// Database open error
		return false;

	Finalize();

	for ( ; ; )
	{
		LPCWSTR pszTail = NULL;
#if UNICODE
		int res = sqlite3_prepare16_v2( m_db, (LPCVOID)(LPCWSTR)m_sQuery, -1, &m_st, (LPCVOID*)&pszTail );
#else
		int res = sqlite3_prepare_v2( m_db, (LPCSTR)m_sQuery, -1, &m_st, (LPCVOID*)&pszTail );
#endif
		switch ( res )
		{
		case SQLITE_OK:
			if ( pszTail && *pszTail )
				m_sQuery = pszTail;
			else
				m_sQuery.Empty();

			if ( m_st )
				return true;

			// This happens for a comment or white-space
			break;

		case SQLITE_BUSY:
		case SQLITE_LOCKED:
			m_bBusy = true;
			return false;

		default:
			return false;
		}
	}
}

void CDatabase::Finalize()
{
	ATLASSERT( m_nThread == GetCurrentThreadId() );	// Don't pass database across thread boundaries

	if ( m_st )
	{
		sqlite3_finalize( m_st );
		m_st = NULL;
	}

	m_bBusy = false;
	m_raw.RemoveAll();
}

bool CDatabase::Step(DWORD dwTimeout)
{
	ATLASSERT( m_nThread == GetCurrentThreadId() );	// Don't pass database across thread boundaries
	ATLASSERT( m_st );

	if ( ! m_st )
		return false;

	m_bBusy = false;
	m_raw.RemoveAll();

	int res;
	const DWORD nStart = GetTickCount();
	for (;;)
	{
		res = sqlite3_step( m_st );
		if ( res != SQLITE_BUSY && res != SQLITE_LOCKED )
			break;
		if ( Span( GetTickCount(), nStart ) > dwTimeout )
			break;
		Sleep( 10 );
	}
	switch ( res )
	{
	case SQLITE_ROW:
		break;

	case SQLITE_BUSY:
	case SQLITE_LOCKED:
		m_bBusy = true;
		return false;

	case SQLITE_DONE:
		Finalize();
		return true;

	default:
		// To get good error message
		sqlite3_reset( m_st );
		return false;
	}

	// Save column names
	int count = sqlite3_data_count( m_st );
	for ( int i = 0; i < count; i++ )
	{
#if UNICODE
		m_raw.SetAt( (LPCWSTR)sqlite3_column_name16( m_st, i ), i );
#else
		m_raw.SetAt( (LPCSTR)sqlite3_column_name( m_st, i ), i );
#endif
	}
	return true;
}

int CDatabase::GetColumn(LPCTSTR szName) const
{
	int column;
	return ( m_st && m_raw.Lookup( szName, column ) ) ? column : -1;
}

int CDatabase::GetType(LPCTSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
	{
		return sqlite3_column_type( m_st, column );
	}
	return 0;
}

__int32 CDatabase::GetInt32(LPCTSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
	{
		return sqlite3_column_int( m_st, column );
	}
	return 0;
}

__int64 CDatabase::GetInt64(LPCTSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
	{
		return sqlite3_column_int64( m_st, column );
	}
	return 0;
}

double CDatabase::GetDouble(LPCTSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
	{
		return sqlite3_column_double( m_st, column );
	}
	return 0;
}

CString CDatabase::GetString(LPCTSTR pszName) const
{
	int column = GetColumn( pszName );
	if ( column != -1 )
	{
#if UNICODE
		return CString( (LPCWSTR)sqlite3_column_text16( m_st, column ) );
#else
		return CString( sqlite3_column_text( m_st, column ) );
#endif
	}
	return CString();
}

LPCVOID CDatabase::GetBlob(LPCTSTR pszName, int* pnLength) const
{
	LPCVOID pBlob = NULL;
	if ( pnLength )
		*pnLength = 0;
	int column = GetColumn( pszName );
	if ( column != -1 )
	{
		pBlob = sqlite3_column_blob( m_st, column );
		if ( pnLength )
			*pnLength = sqlite3_column_bytes( m_st, column );
	}
	return pBlob;
}

bool CDatabase::Bind(int nIndex, __int32 nData)
{
	return m_st && sqlite3_bind_int( m_st, nIndex, nData ) == SQLITE_OK;
}

bool CDatabase::Bind(int nIndex, __int64 nData)
{
	return m_st && sqlite3_bind_int64( m_st, nIndex, nData ) == SQLITE_OK;
}

bool CDatabase::Bind(int nIndex, double dData)
{
	return m_st && sqlite3_bind_double( m_st, nIndex, dData ) == SQLITE_OK;
}

bool CDatabase::Bind(int nIndex, LPCSTR szData)
{
	return m_st && sqlite3_bind_text( m_st, nIndex, szData, -1, SQLITE_STATIC ) == SQLITE_OK;
}

bool CDatabase::Bind(int nIndex, LPCWSTR szData)
{
	return m_st && sqlite3_bind_text16( m_st, nIndex, (LPVOID)szData, -1, SQLITE_STATIC ) == SQLITE_OK;
}

bool CDatabase::Bind(int nIndex, LPCVOID pData, int nLength)
{
	return m_st && sqlite3_bind_blob( m_st, nIndex, pData, nLength, SQLITE_STATIC ) == SQLITE_OK;
}
