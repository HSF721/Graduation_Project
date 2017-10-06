// 3_Axis_controlDlg.h : header file
//
//{{AFX_INCLUDES()
#include "tchart1.h"
#include "mscomm1.h"
#include "afxwin.h"
//}}AFX_INCLUDES
#define WM_MY_MESSAGE (WM_USER+100) 
#if !defined(AFX_3_AXIS_CONTROLDLG_H__C60F4CF7_8011_4781_A9C4_FEFEF3800D3C__INCLUDED_)
#define AFX_3_AXIS_CONTROLDLG_H__C60F4CF7_8011_4781_A9C4_FEFEF3800D3C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMy3_Axis_controlDlg dialog

class CMy3_Axis_controlDlg : public CDialog
{
// Construction
public:
	CMy3_Axis_controlDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMy3_Axis_controlDlg)
	enum { IDD = IDD_MY3_AXIS_CONTROL_DIALOG };
	CComboBox	m_axis_num;
	float	m_distance_value;
	float	m_acc_dcc_value;
	float	m_step_distance;
	float	m_step_speed;
	CTChart	m_TeeChart;
//	int		m_circle_R;
	int		m_min_r;
	int		m_max_r;
	//}}AFX_DATA
//	void x_plus(int len);
//	void y_plus(int len);
	void x_minus(int len);
	void y_minus(int len);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMy3_Axis_controlDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	HANDLE hThread;
	HANDLE hThread1;
	HANDLE hThread2;
	HANDLE hThread3;
	HANDLE hThread4;
	HANDLE hThread5;
	HANDLE hThread6;
	HANDLE hThread_line_interpolation;
	HANDLE hThread_circle_interpolation;
	HANDLE hThread_tu_ball;

	HANDLE hThread_short_back;
	HANDLE hThread_open_feed;

	HANDLE hThread_process_cubio_1;
	HANDLE hThread_process_cubio_2;


	DWORD ThreadID;
	DWORD ThreadID1;
	DWORD ThreadID2;
	DWORD ThreadID3;
	DWORD ThreadID4;
	DWORD ThreadID5;
	DWORD ThreadID6;
	DWORD ThreadID_line_interpolation;
	DWORD ThreadID_circle_interpolation;
	DWORD ThreadID_tu_ball;

	DWORD ThreadID_short_back;
	DWORD ThreadID_open_feed;

	DWORD ThreadID_process_cubio_1;
	DWORD ThreadID_process_cubio_2;

	// Generated message map functions
	//{{AFX_MSG(CMy3_Axis_controlDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnInit();
	afx_msg void OnShutdownServo();
	afx_msg void OnQuit();
	afx_msg void OnXPlus();
	afx_msg void OnXMinus();
	afx_msg void OnYPlus();
	afx_msg void OnYMinus();
	afx_msg void OnZPlus();
	afx_msg void OnZMinus();
	afx_msg void OnStartProcess();
	afx_msg void OnStopProcess();
	afx_msg void OnStopAllAxis();
	afx_msg void OnButton1();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnStopDrawing();
	afx_msg void OnCleanPicture();
	afx_msg void OnStartDrawing();
	afx_msg void OnProcessH();
	afx_msg void OnProcessI();
	afx_msg void OnProcessT();
	afx_msg void OnStophit();
	afx_msg void OnTestLineInterpolation();
	afx_msg void OnTestCircleInterpolation();
	afx_msg void OnTurbProcess();
	afx_msg void Onupdate_R_circle();
	afx_msg void OnTuoBall();

	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam); 
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	int m_circle_R;
//	int x_plus;
//	int y_plus;
//	int m_x_distance;
//	int m_y_distance;
	int m_x_distance;
	int m_y_distance;
//	int m_global_position_x;
//	int m_global_position_y;
//	int m_global_position_z;
//	int m_global_position_z;
	int m_global_position_x;
	int m_global_position_y;
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedBackToStart();
	int m_global_position_z;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedIntelligentproces();
	afx_msg void OnBnClickedTestShort();
	afx_msg void OnBnClickedTestOpen();
	afx_msg void OnBnClickedNormalProces();
	afx_msg void OnBnClickedOpenSeririalPort();
	afx_msg void OnBnClickedOpenSerialPort();
//	CString m_strTXData;
	CMscomm1 m_ctrlComm;
	DECLARE_EVENTSINK_MAP()
	void OnComm();
	CString m_strRXData;
	CComboBox m_com_select;
	CComboBox m_bt_select;
	CString m_voltage;
	afx_msg void OnBnClickedProcesCuboid();
	int m_length_bottom_edge;
//	int m_cycle_times;
	int m_cycle_times_expect;
	int m_cycle_times_now;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_3_AXIS_CONTROLDLG_H__C60F4CF7_8011_4781_A9C4_FEFEF3800D3C__INCLUDED_)
