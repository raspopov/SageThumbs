// CreateWorkspace.cpp : implementation file
//

#include "stdafx.h"
#include "LocalizeRC.h"
#include "CreateWorkspace.h"


// CCreateWorkspace dialog

IMPLEMENT_DYNAMIC(CCreateWorkspace, CDialog)
CCreateWorkspace::CCreateWorkspace(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateWorkspace::IDD, pParent)
	, m_strInputRC(_T(""))
	, m_strLangINI(_T(""))
	, m_strOutputRC(_T(""))
{
}

CCreateWorkspace::~CCreateWorkspace()
{
}

void CCreateWorkspace::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_INPUTRC, m_strInputRC);
	DDX_Text(pDX, IDC_LANGINI, m_strLangINI);
	DDX_Text(pDX, IDC_OUTPUTRC, m_strOutputRC);
}


BEGIN_MESSAGE_MAP(CCreateWorkspace, CDialog)
	ON_BN_CLICKED(IDC_CHNG_INPUTRC, OnBnClickedChngInputrc)
	ON_BN_CLICKED(IDC_CHNG_LANGINI, OnBnClickedChngLangini)
	ON_BN_CLICKED(IDC_CHNG_OUTPUTRC, OnBnClickedChngOutputrc)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CCreateWorkspace message handlers

void CCreateWorkspace::OnBnClickedChngInputrc()
{
	
}

void CCreateWorkspace::OnBnClickedChngLangini()
{
	
}

void CCreateWorkspace::OnBnClickedChngOutputrc()
{
	
}

BOOL CCreateWorkspace::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Load Workspace
	CIniEx IniEx;
	if( !IniEx.Open( strWorkspace ) )
		return false;

	m_strInputRC = IniEx.GetValue( ENT_INPUTRC );
	m_strLangINI = IniEx.GetValue( ENT_LANGINI );
	m_strOutputRC = IniEx.GetValue( ENT_OUTPUTRC );

	UpdateData( false );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}



void CCreateWorkspace::OnBnClickedOk()
{
	UpdateData( true );

	

	OnOK();
}
