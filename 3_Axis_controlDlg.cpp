// 3_Axis_controlDlg.cpp : implementation file
//
/************************************************************************/
/* 平台步进距离记录                                                     */
//X轴5000个脉冲，16mm 1000个脉冲走3.2mm
//Y轴10000个脉冲，10mm 1000个脉冲走1mm
/************************************************************************/
#include "stdafx.h"
#include "3_Axis_control.h"
#include "3_Axis_controlDlg.h"
#include "head.h"
#include "gts.h"
#include <string>
#include "string.h"
#include "math.h"
#include <vector>
#include <sstream>
//#include "stack.h"
using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define WM_MY_MESSAGE (WM_USER+100)
struct my_data
{
	float acc;
	float dis;
};
my_data  data_axis1;
my_data  data_axis2;
my_data  data_axis3;
/******************************自定义全局变量****************************************************/
int flag_stop=0;
int flag = 0;
int flag_stop_P2P_move = 1;//为1的时候运动，为0的时候停止
BOOL flag_move_by_key = 0;//为1时表示启用键盘调整位置，为0时表示关闭
BOOL flag_servo_on=0;//为0表示关闭，为1表示开启
int AXIS=2;
double k=0,j=0;//onTimer里的变量
//volatile 作用是告诉编译器不用做任何优化，即无需将它放到一个寄存器中，并且该值可被外部改变
volatile BOOL flag_stop_P2P1=1;//为1表示不用结束，为0表示要结束 1轴的标志位
volatile BOOL flag_stop_P2P2=1;//为1表示不用结束，为0表示要结束 2轴的标志位
volatile BOOL flag_stop_P2P3=1;//为1表示不用结束，为0表示要结束 3轴的标志位
int position_x=0;
int position_y=0;
double distance_value_global=0;
BOOL flag_intelligent_proces = 0;//启动智能加工

BOOL flag_open_feed = 0;//表示开路进给，等于1的时候进给
BOOL flag_short_back = 0;//表示短路回退，等于1的时候回退
BOOL flag_intelligent_time = 0;

struct my_point
		{
			int x;
			int y;
			int z;
		};
my_point new_line_point;//直线插补的终点坐标（结构体类型）
my_point destination_point_turb[6];//初级涡轮加工时的大圆上6个点的坐标
my_point initial_point_turn[6];//因为需要从0°开始，这个为开始坐标

my_point global_position;//全局定位系统

int purose_speed = 1;//设置AXIS轴的目标速度

struct inf_circle_interpolation
{
	//沿逆时针方向运动
	int R;//半径
	int old_R;
	double angle;//角度
};
inf_circle_interpolation new_inf_circle_interpolation;
bool flag_finish_circle = 0;//标志位，是否完成一个整圆，是的话为1

struct inf_tu_ball
{
	//沿逆时针方向运动
	int min_R;//最小半径
	int max_R;//最大半径
	double angle;//角度
};
inf_tu_ball new_inf_tu_ball;

int times_click_circle = 0, times_click_helix = 0;//点击“圆柱加工”“圆平面加工”的次数，防止多次点击导致运动叠加
int times_click_cuboid = 0;//点击长方体加工的次数
bool flag_finish_cubio = 0;
int length_bottom_edge = 0;

int cycle_times_line=0,cycle_times_cylinder=0,cycle_times_cuboid=0,cycle_times_tuball=0;//记录反复加工当前已循环的次数
int max_cycle_times=0;//上位机设置的最大循环次数
CWnd* m_pCWnd;//用来获取主窗口指针
/******************************自定义全局变量****************************************************/

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMy3_Axis_controlDlg dialog

CMy3_Axis_controlDlg::CMy3_Axis_controlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMy3_Axis_controlDlg::IDD, pParent)
	, m_x_distance(0)
	, m_y_distance(0)
{
	//{{AFX_DATA_INIT(CMy3_Axis_controlDlg)
	m_distance_value = 0.0f;
	m_acc_dcc_value = 0.0f;
	m_step_distance = 0.0f;
	m_step_speed = 0.0f;
	//  m_circle_R_add = 0;
	m_min_r = 50;
	m_max_r = 400;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_circle_R = 50;
	m_x_distance = 500;
	m_y_distance = 500;

	m_global_position_z = 0;
	m_global_position_x = 0;
	m_global_position_y = 0;
	m_global_position_z = 0;
	//  m_strTXData = _T("");
	m_strRXData = _T("");
	m_voltage = _T("");
	m_length_bottom_edge = 600;
	m_cycle_times_expect = 65535;
	m_cycle_times_now = 0;
}

void CMy3_Axis_controlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMy3_Axis_controlDlg)
	DDX_Control(pDX, IDC_AXIS_NUM, m_axis_num);
	DDX_Text(pDX, IDC_DISTANCE, m_distance_value);
	DDX_Text(pDX, IDC_ACC_and_DCC, m_acc_dcc_value);
	DDX_Text(pDX, IDC_step_distance, m_step_distance);
	DDX_Text(pDX, IDC_step_speed, m_step_speed);
	DDX_Control(pDX, IDC_TCHART1, m_TeeChart);
	//  DDX_Text(pDX, IDC_circle_interpolation_x_plus, m_circle_R_add);
	DDX_Text(pDX, IDC_MIN_R, m_min_r);
	DDX_Text(pDX, IDC_MAX_R, m_max_r);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_circle_interpolation_x_plus, m_circle_R);
	DDX_Text(pDX, IDC_X_DISTANCE, m_x_distance);
	DDX_Text(pDX, IDC_Y_DISTANCE, m_y_distance);
	DDX_Text(pDX, IDC_GLOBAL_POSITION_Z, m_global_position_z);
	DDX_Text(pDX, IDC_GLOBAL_POSITION_X, m_global_position_x);
	DDX_Text(pDX, IDC_GLOBAL_POSITION_Y, m_global_position_y);
	DDX_Text(pDX, IDC_GLOBAL_POSITION_Z, m_global_position_z);
	//  DDX_Text(pDX, IDC_EDIT_RXDATA, m_strTXData);
	DDX_Control(pDX, IDC_MSCOMM1, m_ctrlComm);
	DDX_Text(pDX, IDC_EDIT_RXDATA, m_strRXData);
	DDX_Control(pDX, IDC_COMBO_COM, m_com_select);
	DDX_Control(pDX, IDC_COMBO_BT, m_bt_select);
	DDX_Text(pDX, IDC_VOLTAGE, m_voltage);
	DDX_Text(pDX, IDC_LENGTH_BOTTOM_EDGE, m_length_bottom_edge);
	DDV_MinMaxInt(pDX, m_length_bottom_edge, 0, 30000);
	//  DDX_Text(pDX, IDC_CYCLE_TIMES, m_cycle_times);
	DDX_Text(pDX, IDC_CYCLE_TIMES_EXPECT, m_cycle_times_expect);
	DDX_Text(pDX, IDC_CYCLE_TIMES_NOW, m_cycle_times_now);
}

BEGIN_MESSAGE_MAP(CMy3_Axis_controlDlg, CDialog)
	//{{AFX_MSG_MAP(CMy3_Axis_controlDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_INIT, OnInit)
	ON_BN_CLICKED(IDC_SHUTDOWN_SERVO, OnShutdownServo)
	ON_BN_CLICKED(IDC_QUIT, OnQuit)
	ON_BN_CLICKED(IDC_X_PLUS, OnXPlus)
	ON_BN_CLICKED(IDC_X_MINUS, OnXMinus)
	ON_BN_CLICKED(IDC_Y_PLUS, OnYPlus)
	ON_BN_CLICKED(IDC_Y_MINUS, OnYMinus)
	ON_BN_CLICKED(IDC_Z_PLUS, OnZPlus)
	ON_BN_CLICKED(IDC_Z_MINUS, OnZMinus)
	ON_BN_CLICKED(IDC_START_PROCESS, OnStartProcess)
	ON_BN_CLICKED(IDC_STOP_PROCESS, OnStopProcess)
	ON_BN_CLICKED(IDC_STOP_ALL_AXIS, OnStopAllAxis)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_StopDrawing, OnStopDrawing)
	ON_BN_CLICKED(IDC_CleanPicture, OnCleanPicture)
	ON_BN_CLICKED(IDC_StartDrawing, OnStartDrawing)
	ON_BN_CLICKED(IDC_PROCESS_H, OnProcessH)
	ON_BN_CLICKED(IDC_PROCESS_I, OnProcessI)
	ON_BN_CLICKED(IDC_PROCESS_T, OnProcessT)
	ON_BN_CLICKED(IDC_STOPHIT, OnStophit)
	ON_BN_CLICKED(IDC_TEST_LINE_INTERPOLATION, OnTestLineInterpolation)
	ON_BN_CLICKED(IDC_TEST_CIRCLE_INTERPOLATION, OnTestCircleInterpolation)
	ON_BN_CLICKED(IDC_TURB_PROCESS, OnTurbProcess)
//	ON_BN_CLICKED(IDC_update_R_circle, Onupdate_R_circle)
	ON_BN_CLICKED(IDC_TUO_BALL, OnTuoBall)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON4, &CMy3_Axis_controlDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BACK_TO_START, &CMy3_Axis_controlDlg::OnBnClickedBackToStart)
	ON_BN_CLICKED(IDC_Intelligent_proces, &CMy3_Axis_controlDlg::OnBnClickedIntelligentproces)
	ON_BN_CLICKED(IDC_TEST_SHORT, &CMy3_Axis_controlDlg::OnBnClickedTestShort)
	ON_BN_CLICKED(IDC_TEST_OPEN, &CMy3_Axis_controlDlg::OnBnClickedTestOpen)
	ON_BN_CLICKED(IDC_NORMAL_PROCES, &CMy3_Axis_controlDlg::OnBnClickedNormalProces)
	ON_BN_CLICKED(IDC_OPEN_SERIAL_PORT, &CMy3_Axis_controlDlg::OnBnClickedOpenSerialPort)
	ON_BN_CLICKED(IDC_PROCES_CUBOID, &CMy3_Axis_controlDlg::OnBnClickedProcesCuboid)

	ON_MESSAGE(WM_MY_MESSAGE, OnMyMessage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMy3_Axis_controlDlg message handlers

BOOL CMy3_Axis_controlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_axis_num.SetCurSel(0);
	m_acc_dcc_value = 0.1;
	m_distance_value = 1000;

	m_step_speed = 0.5;
	m_step_distance = 100;

	m_com_select.SetCurSel(2);
	m_bt_select.SetCurSel(1);

	m_pCWnd = AfxGetMainWnd();//获取主窗口的指针

	UpdateData(false);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMy3_Axis_controlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMy3_Axis_controlDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMy3_Axis_controlDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CMy3_Axis_controlDlg::OnInit() 
{
	// TODO: Add your control notification handler code here

	global_position.x = 0;
	global_position.y = 0;
	global_position.z = 0;

	short sRtn,sRtn1,sRtn2,sRtn3;
	long lAxisStatus;
	CString str,str1;
	// 打开运动控制器
	sRtn = GT_Open();
	if (sRtn != 0)
	{
		MessageBox("GT_Open faild");
	}
	sRtn = GT_Reset();
	if (sRtn != 0)
	{
		MessageBox("GT_Reset faild");
	}
//	sRtn = GT_LoadConfig ("3_axis.cfg");
	sRtn = GT_LoadConfig ("3_axis_lim_change3_to_4.cfg");
	if (sRtn != 0)
	{
		MessageBox("GT_LoadConfig faild");
	}
	sRtn = GT_EncOn(1);
	sRtn = GT_EncOn(2);
	sRtn = GT_EncOn(4);
	// 配置完成后要更新状态
	sRtn = GT_ClrSts(4);
	sRtn = GT_ClrSts(2);
	sRtn = GT_ClrSts(1);	
	// 轴1伺服使能
	TPid m_Pid;	
	sRtn=GT_GetPid(2,1,&m_Pid);
	m_Pid.kp=2;
	m_Pid.ki=0.01;
	m_Pid.kd=0;
	sRtn1=GT_SetPid(1,1,&m_Pid);
	sRtn1 = GT_AxisOn(1);

	sRtn2=GT_SetPid(2,1,&m_Pid);
	sRtn2 = GT_AxisOn(2);

	sRtn3=GT_SetPid(4,1,&m_Pid);
	sRtn3 = GT_AxisOn(4);

	if (sRtn1 == 0 && sRtn2 == 0 && sRtn3 == 0)
	{
		MessageBox("三轴伺服均开启成功！");
		flag_servo_on = 1;
	}
	
	else MessageBox("伺服开启失败！");
	
}

void CMy3_Axis_controlDlg::OnShutdownServo() 
{
	// TODO: Add your control notification handler code here
	short sRtn1,sRtn2,sRtn3;
	sRtn1 = GT_AxisOff(1);
	sRtn2 = GT_AxisOff(2);
	sRtn3 = GT_AxisOff(4);

	if(sRtn1 != 0)
	{
		MessageBox("伺服轴 1 关闭失败！");
	}
	else if(sRtn2 != 0)
	{
		MessageBox("伺服轴 2 关闭失败！");
	}
	else if(sRtn3 != 0)
	{
		MessageBox("伺服轴 3 关闭失败！");
	}	
	else 
	{
		MessageBox("伺服关闭成功！");
		flag_servo_on = 0;
	}
	sRtn3 = GT_Close();
}

void CMy3_Axis_controlDlg::OnQuit() 
{
	// TODO: Add your control notification handler code here
	DestroyWindow( ); 
}

void CMy3_Axis_controlDlg::OnXPlus() 
{
	short sRtn;
	TTrapPrm trap;
	long sts;
	double prfPos;
	int new_axis = 1;
	UpdateData(true);
	// TODO: Add your control notification handler code here
	// 位置清零
	sRtn = GT_ZeroPos(new_axis);
	// AXIS轴规划位置清零
	sRtn = GT_SetPrfPos(new_axis, 0);
	// 将AXIS轴设为点位模式
	sRtn = GT_PrfTrap(new_axis);
	// 读取点位运动参数
	sRtn = GT_GetTrapPrm(new_axis, &trap);
	trap.acc = m_step_speed;
	trap.dec = m_step_speed;
	trap.smoothTime = 25;
	// 设置点位运动参数
	sRtn = GT_SetTrapPrm(new_axis, &trap);
	// 设置AXIS轴的目标位置
	sRtn = GT_SetPos(new_axis, m_step_distance);
	// 设置AXIS轴的目标速度
	sRtn = GT_SetVel(new_axis, purose_speed);
	// 启动AXIS轴的运动
	sRtn = GT_Update(1<<(new_axis-1));
	do
	{
		// 读取AXIS轴的状态
		sRtn = GT_GetSts(new_axis, &sts);
		// 读取AXIS轴的规划位置
		sRtn = GT_GetPrfPos(new_axis, &prfPos);
	}while(sts&0x400); // 等待AXIS轴规划停止
	global_position.x += m_step_distance;//更新当前位置信息
	m_global_position_x = global_position.x;
	UpdateData(false);
}

void CMy3_Axis_controlDlg::OnXMinus() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	short sRtn;
	TTrapPrm trap;
	long sts;
	double prfPos;
	int new_axis = 1;
	// TODO: Add your control notification handler code here
	// 位置清零
	sRtn = GT_ZeroPos(new_axis);
	// AXIS轴规划位置清零
	sRtn = GT_SetPrfPos(new_axis, 0);
	// 将AXIS轴设为点位模式
	sRtn = GT_PrfTrap(new_axis);
	// 读取点位运动参数
	sRtn = GT_GetTrapPrm(new_axis, &trap);
	trap.acc = m_step_speed;
	trap.dec = m_step_speed;
	trap.smoothTime = 25;
	// 设置点位运动参数
	sRtn = GT_SetTrapPrm(new_axis, &trap);
	// 设置AXIS轴的目标位置
	sRtn = GT_SetPos(new_axis, -m_step_distance);
	// 设置AXIS轴的目标速度
	sRtn = GT_SetVel(new_axis, purose_speed);
	// 启动AXIS轴的运动
	sRtn = GT_Update(1<<(new_axis-1));
	do
	{
		// 读取AXIS轴的状态
		sRtn = GT_GetSts(new_axis, &sts);
		// 读取AXIS轴的规划位置
		sRtn = GT_GetPrfPos(new_axis, &prfPos);
	}while(sts&0x400); // 等待AXIS轴规划停止
	global_position.x -= m_step_distance;//更新当前位置信息
	m_global_position_x = global_position.x;
	UpdateData(false);
}

void CMy3_Axis_controlDlg::OnYPlus() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	short sRtn;
	TTrapPrm trap;
	long sts;
	double prfPos;
	int new_axis = 2;
	// TODO: Add your control notification handler code here
	// 位置清零
	sRtn = GT_ZeroPos(new_axis);
	// AXIS轴规划位置清零
	sRtn = GT_SetPrfPos(new_axis, 0);
	// 将AXIS轴设为点位模式
	sRtn = GT_PrfTrap(new_axis);
	// 读取点位运动参数
	sRtn = GT_GetTrapPrm(new_axis, &trap);
	trap.acc = m_step_speed;
	trap.dec = m_step_speed;
	trap.smoothTime = 25;
	// 设置点位运动参数
	sRtn = GT_SetTrapPrm(new_axis, &trap);
	// 设置AXIS轴的目标位置
	sRtn = GT_SetPos(new_axis, m_step_distance);
	// 设置AXIS轴的目标速度
	sRtn = GT_SetVel(new_axis, purose_speed);
	// 启动AXIS轴的运动
	sRtn = GT_Update(1<<(new_axis-1));
	do
	{
		// 读取AXIS轴的状态
		sRtn = GT_GetSts(new_axis, &sts);
		// 读取AXIS轴的规划位置
		sRtn = GT_GetPrfPos(new_axis, &prfPos);
	}while(sts&0x400); // 等待AXIS轴规划停止
	global_position.y += m_step_distance;//更新当前位置信息
	m_global_position_y = global_position.y;
	UpdateData(false);
}

void CMy3_Axis_controlDlg::OnYMinus() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	short sRtn;
	TTrapPrm trap;
	long sts;
	double prfPos;
	int new_axis = 2;
	// TODO: Add your control notification handler code here
	// 位置清零
	sRtn = GT_ZeroPos(new_axis);
	// AXIS轴规划位置清零
	sRtn = GT_SetPrfPos(new_axis, 0);
	// 将AXIS轴设为点位模式
	sRtn = GT_PrfTrap(new_axis);
	// 读取点位运动参数
	sRtn = GT_GetTrapPrm(new_axis, &trap);
	trap.acc = m_step_speed;
	trap.dec = m_step_speed;
	trap.smoothTime = 25;
	// 设置点位运动参数
	sRtn = GT_SetTrapPrm(new_axis, &trap);
	// 设置AXIS轴的目标位置
	sRtn = GT_SetPos(new_axis, -m_step_distance);
	// 设置AXIS轴的目标速度
	sRtn = GT_SetVel(new_axis, purose_speed);
	// 启动AXIS轴的运动
	sRtn = GT_Update(1<<(new_axis-1));
	do
	{
		// 读取AXIS轴的状态
		sRtn = GT_GetSts(new_axis, &sts);
		// 读取AXIS轴的规划位置
		sRtn = GT_GetPrfPos(new_axis, &prfPos);
	}while(sts&0x400); // 等待AXIS轴规划停止
	global_position.y -= m_step_distance;//更新当前位置信息
	m_global_position_y = global_position.y;
	UpdateData(false);
}

void CMy3_Axis_controlDlg::OnZPlus() 
{
	// TODO: Add your control notification handler code here
	short sRtn;
	TTrapPrm trap;
	long sts;
	double prfPos;
	int new_axis = 4;
	UpdateData(true);
	// TODO: Add your control notification handler code here
	// 位置清零
	sRtn = GT_ZeroPos(new_axis);
	// AXIS轴规划位置清零
	sRtn = GT_SetPrfPos(new_axis, 0);
	// 将AXIS轴设为点位模式
	sRtn = GT_PrfTrap(new_axis);
	// 读取点位运动参数
	sRtn = GT_GetTrapPrm(new_axis, &trap);
	trap.acc = m_step_speed;
	trap.dec = m_step_speed;
	trap.smoothTime = 25;
	// 设置点位运动参数
	sRtn = GT_SetTrapPrm(new_axis, &trap);
	// 设置AXIS轴的目标位置
	sRtn = GT_SetPos(new_axis, m_step_distance);
	// 设置AXIS轴的目标速度
	sRtn = GT_SetVel(new_axis, purose_speed);
	// 启动AXIS轴的运动
	sRtn = GT_Update(1<<(new_axis-1));
	do
	{
		// 读取AXIS轴的状态
		sRtn = GT_GetSts(new_axis, &sts);
		// 读取AXIS轴的规划位置
		sRtn = GT_GetPrfPos(new_axis, &prfPos);
	}while(sts&0x400); // 等待AXIS轴规划停止
	global_position.z += m_step_distance;//更新当前位置信息
//	m_global_position_z = global_position.z;
	sRtn=GT_GetAxisEncPos(4,&j);
	m_global_position_z =  j;
	UpdateData(false);
}

void CMy3_Axis_controlDlg::OnZMinus() 
{
	// TODO: Add your control notification handler code here
	short sRtn;
	TTrapPrm trap;
	long sts;
	double prfPos;
	int new_axis = 4;
	UpdateData(true);
	// TODO: Add your control notification handler code here
	// 位置清零
	sRtn = GT_ZeroPos(new_axis);
	// AXIS轴规划位置清零
	sRtn = GT_SetPrfPos(new_axis, 0);
	// 将AXIS轴设为点位模式
	sRtn = GT_PrfTrap(new_axis);
	// 读取点位运动参数
	sRtn = GT_GetTrapPrm(new_axis, &trap);
	trap.acc = m_step_speed;
	trap.dec = m_step_speed;
	trap.smoothTime = 10;
	// 设置点位运动参数
	sRtn = GT_SetTrapPrm(new_axis, &trap);
	// 设置AXIS轴的目标位置
	sRtn = GT_SetPos(new_axis, -m_step_distance);
	// 设置AXIS轴的目标速度
	sRtn = GT_SetVel(new_axis, purose_speed);
	// 启动AXIS轴的运动
	sRtn = GT_Update(1<<(new_axis-1));
	do
	{
		// 读取AXIS轴的状态
		sRtn = GT_GetSts(new_axis, &sts);
		// 读取AXIS轴的规划位置
		sRtn = GT_GetPrfPos(new_axis, &prfPos);
	}while(sts&0x400); // 等待AXIS轴规划停止
	global_position.z -= m_step_distance;//更新当前位置信息
//	m_global_position_z = global_position.z;
	sRtn=GT_GetAxisEncPos(4,&j);
	m_global_position_z =  j;
	UpdateData(false);
}

static DWORD WINAPI ThreadFunc1(LPVOID pvoid)
{
	
	int j=1;
	short sRtn;
	long sts;
	double prfPos;
	do
	{
		j=-j;
		if(j==1)
		{
			sRtn = GT_SetPos(1, 0);
			global_position.x -= data_axis1.dis;//更新当前位置信息
		}
		else
		{
			sRtn = GT_SetPos(1, data_axis1.dis);
			global_position.x += data_axis1.dis;//更新当前位置信息
		}
		// 设置AXIS轴的目标速度
		sRtn = GT_SetVel(1, 1);
		// 启动AXIS轴的运动
		sRtn = GT_Update(1<<(1-1));
		do
		{
			// 读取AXIS轴的状态
			sRtn = GT_GetSts(1, &sts);
			// 读取AXIS轴的规划位置
			sRtn = GT_GetPrfPos(1, &prfPos);
		}while(sts&0x400); // 等待AXIS轴规划停止
	}while(!(flag_stop_P2P1 == 0 && j == 1));
	return 0;
}
static DWORD WINAPI ThreadFunc2(LPVOID pvoid)
{
	int j=1;
	short sRtn;
	long sts;
	double prfPos;
	do
	{
		j=-j;
		if(j==1)
		{
			sRtn = GT_SetPos(2, 0);
			global_position.y -= data_axis1.dis;//更新当前位置信息
		}
		else
		{
			sRtn = GT_SetPos(2, data_axis2.dis);
			global_position.y += data_axis1.dis;//更新当前位置信息
		}
		// 设置AXIS轴的目标速度
		sRtn = GT_SetVel(2, 1);
		// 启动AXIS轴的运动
		sRtn = GT_Update(1<<(2-1));
		do
		{
			// 读取AXIS轴的状态
			sRtn = GT_GetSts(2, &sts);
			// 读取AXIS轴的规划位置
			sRtn = GT_GetPrfPos(2, &prfPos);
		}while(sts&0x400); // 等待AXIS轴规划停止
	}while(!(flag_stop_P2P2 == 0 && j == 1));
	return 0;
}
static DWORD WINAPI ThreadFunc3(LPVOID pvoid)
{
	int j=1;
	short sRtn;
	long sts;
	double prfPos;
	do
	{
		j=-j;
		if(j==1)
		{
			sRtn = GT_SetPos(4, 0);
			global_position.z -= data_axis1.dis;//更新当前位置信息
		}
		else
		{
			sRtn = GT_SetPos(4, data_axis3.dis);
			global_position.z += data_axis1.dis;//更新当前位置信息
		}
		// 设置AXIS轴的目标速度
		sRtn = GT_SetVel(4, 1);
		// 启动AXIS轴的运动
		sRtn = GT_Update(1<<(4-1));
		do
		{
			// 读取AXIS轴的状态
			sRtn = GT_GetSts(4, &sts);
			// 读取AXIS轴的规划位置
			sRtn = GT_GetPrfPos(4, &prfPos);
		}while(sts&0x400); // 等待AXIS轴规划停止
	}while(!(flag_stop_P2P3 == 0 && j == 1));
	return 0;
}
int change_axis_string2int(CString str)
{
	if (str=="1") return 1;
	else if (str=="2") return 2;
	else if (str=="3") return 3;
	else if (str=="4") return 4;
	else return 3;

}

void CMy3_Axis_controlDlg::OnStartProcess() 
{
	// TODO: Add your control notification handler code here
	
	CString strCBText,str;
	m_axis_num.GetLBText( m_axis_num.GetCurSel(), strCBText);
	AXIS = change_axis_string2int(strCBText);
	if (flag_servo_on == 1)
	{
		int cycle_time_temp;
		short sRtn;
		TTrapPrm trap;
		long sts;
		double prfPos;

		UpdateData(true);
		distance_value_global = m_distance_value;
//		cycle_time_temp = m_cycle_time;
		sRtn = GT_ZeroPos(AXIS);
		// AXIS轴规划位置清零
		sRtn = GT_SetPrfPos(AXIS, 0);
		// 将AXIS轴设为点位模式
		sRtn = GT_PrfTrap(AXIS);
		// 读取点位运动参数
		sRtn = GT_GetTrapPrm(AXIS, &trap);
		trap.acc = m_acc_dcc_value;
		trap.dec = m_acc_dcc_value;
		trap.smoothTime = 2;
		// 设置点位运动参数
		sRtn = GT_SetTrapPrm(AXIS, &trap);
		// 设置AXIS轴的目标位置
		int i=0,j=1;
//		for(i=0;i<50;i++)
		switch(AXIS)
		{
			case 1:
				flag_stop_P2P1 = 1;
				data_axis1.acc = m_acc_dcc_value;
				data_axis1.dis = m_distance_value;
				hThread1 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc1,&data_axis1,0,&ThreadID1);
				break;
			case 2:
				flag_stop_P2P2 = 1;
				data_axis2.acc = m_acc_dcc_value;
				data_axis2.dis = m_distance_value;
				hThread2 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc2,NULL,0,&ThreadID2);
				break;
			case 4:
				flag_stop_P2P3 = 1;
				data_axis3.acc = m_acc_dcc_value;
				data_axis3.dis = m_distance_value;
				hThread3 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc3,NULL,0,&ThreadID3);
				break;
		}
		
	}
	else MessageBox("请先点击初始化，开启控制卡及伺服轴！");
	UpdateData(false);
}

void CMy3_Axis_controlDlg::OnStopProcess() 
{
	// TODO: Add your control notification handler code here
//	flag_stop_P2P = 0;
	CString strCBText,str;
	m_axis_num.GetLBText( m_axis_num.GetCurSel(), strCBText);
	AXIS = change_axis_string2int(strCBText);
	switch(AXIS)
		{
			case 1:
				flag_stop_P2P1 = 0;
				break;
			case 2:
				flag_stop_P2P2 = 0;
				break;
			case 4:
				flag_stop_P2P3 = 0;
				break;
		}
}

void CMy3_Axis_controlDlg::OnStopAllAxis() 
{
	// TODO: Add your control notification handler code here
	flag_stop_P2P1 = 0;
	flag_stop_P2P2 = 0;
	flag_stop_P2P3 = 0;
}

void CMy3_Axis_controlDlg::OnStartDrawing() 
{
	// TODO: Add your control notification handler code here
	flag_stop=0;
	
	for (int i1=0;i1<100;i1++)
	{
		m_TeeChart.Series(0).Add(0,"",NULL);
		m_TeeChart.Series(1).Add(0,"",NULL);

	}
	m_TeeChart.Series(0).SetTitle("目标轨迹");
	m_TeeChart.Series(1).SetTitle("实际轨迹");
	SetTimer(1,1,NULL);
}

void CMy3_Axis_controlDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	short strn;
	
	m_global_position_z = j;

	CString strCBText,str;
	m_axis_num.GetLBText( m_axis_num.GetCurSel(), strCBText);
	AXIS = change_axis_string2int(strCBText);
	if (flag_stop==0 || flag_stop==1)
	{
		flag_stop++;
		strn=GT_GetAxisPrfPos(AXIS,&k);
		strn=GT_GetAxisEncPos(AXIS,&j);
		m_TeeChart.Series(0).Add(k,"",1);
		m_TeeChart.Series(1).Add(j,"",1);
		m_TeeChart.GetAxis().GetBottom().Scroll(1.0,TRUE);

	}
	else if (flag_stop<5000 && flag_stop>1)
	{
		flag_stop++;
		
		strn=GT_GetAxisEncPos(AXIS,&j);
		m_TeeChart.Series(0).Add(k,"",1);
		m_TeeChart.Series(1).Add(j,"",1);
		
		strn=GT_GetAxisPrfPos(AXIS,&k);
		m_TeeChart.GetAxis().GetBottom().Scroll(1.0,TRUE);

	}
	else
	{
		KillTimer(1);
	}
	CDialog::OnTimer(nIDEvent);
}

void CMy3_Axis_controlDlg::OnStopDrawing() 
{
	// TODO: Add your control notification handler code here
	flag_stop=5000;
}

void CMy3_Axis_controlDlg::OnCleanPicture() 
{
	// TODO: Add your control notification handler code here
	flag_stop=5000;
	for (int i1=0;i1<m_TeeChart.GetSeriesCount();i1++)
	{
		m_TeeChart.Series(i1).Clear();//Clean()
		m_TeeChart.Series(i1).Clear();
//		m_TeeChart.Series(2).Add(0,"",NULL);
	}
}

/*void CMy3_Axis_controlDlg::x_minus(int len)
{
	UpdateData(true);
	short sRtn;
	TTrapPrm trap;
	long sts;
	double prfPos;
	int new_axis = 1;
	// TODO: Add your control notification handler code here
	// 位置清零
	sRtn = GT_ZeroPos(new_axis);
	// AXIS轴规划位置清零
	sRtn = GT_SetPrfPos(new_axis, 0);
	// 将AXIS轴设为点位模式
	sRtn = GT_PrfTrap(new_axis);
	// 读取点位运动参数
	sRtn = GT_GetTrapPrm(new_axis, &trap);
	trap.acc = m_step_speed;
	trap.dec = m_step_speed;
	trap.smoothTime = 25;
	// 设置点位运动参数
	sRtn = GT_SetTrapPrm(new_axis, &trap);
	// 设置AXIS轴的目标位置
	sRtn = GT_SetPos(new_axis, len);
	// 设置AXIS轴的目标速度
	sRtn = GT_SetVel(new_axis, 1);
	// 启动AXIS轴的运动
	sRtn = GT_Update(1<<(new_axis-1));
	do
	{
		// 读取AXIS轴的状态
		sRtn = GT_GetSts(new_axis, &sts);
		// 读取AXIS轴的规划位置
		sRtn = GT_GetPrfPos(new_axis, &prfPos);
	}while(sts&0x400); // 等待AXIS轴规划停止
	UpdateData(false);
}*/

void x_plus(int len)
{
	short sRtn;
	TTrapPrm trap;
	long sts;
	double prfPos;
	int new_axis = 1;
//	UpdateData(true);
	// TODO: Add your control notification handler code here
	// 位置清零
	sRtn = GT_ZeroPos(new_axis);
	// AXIS轴规划位置清零
	sRtn = GT_SetPrfPos(new_axis, 0);
	// 将AXIS轴设为点位模式
	sRtn = GT_PrfTrap(new_axis);
	// 读取点位运动参数
	sRtn = GT_GetTrapPrm(new_axis, &trap);
	trap.acc = 0.05;
	trap.dec = 0.05;
	trap.smoothTime = 25;
	// 设置点位运动参数
	sRtn = GT_SetTrapPrm(new_axis, &trap);
	// 设置AXIS轴的目标位置
	sRtn = GT_SetPos(new_axis, len);
	// 设置AXIS轴的目标速度
	sRtn = GT_SetVel(new_axis, purose_speed);
	// 启动AXIS轴的运动
	sRtn = GT_Update(1<<(new_axis-1));
	do
	{
		// 读取AXIS轴的状态
		sRtn = GT_GetSts(new_axis, &sts);
		// 读取AXIS轴的规划位置
		sRtn = GT_GetPrfPos(new_axis, &prfPos);
	}while(sts&0x400); // 等待AXIS轴规划停止
	global_position.x += len;//更新当前位置信息

	HWND hWnd = ::FindWindow(NULL,_T("3_Axis_control"));
	SetDlgItemInt(hWnd,IDC_GLOBAL_POSITION_X, global_position.x,TRUE);
//	pWnd -> SetWindowTextA(s);
//	UpdateData(false);

}
void y_plus(int len)
{
	
	short sRtn;
	TTrapPrm trap;
	long sts;
	double prfPos;
	int new_axis = 2;
	// TODO: Add your control notification handler code here
	// 位置清零
	sRtn = GT_ZeroPos(new_axis);
	// AXIS轴规划位置清零
	sRtn = GT_SetPrfPos(new_axis, 0);
	// 将AXIS轴设为点位模式
	sRtn = GT_PrfTrap(new_axis);
	// 读取点位运动参数
	sRtn = GT_GetTrapPrm(new_axis, &trap);
	trap.acc = 0.05;
	trap.dec = 0.05;
	trap.smoothTime = 25;
	// 设置点位运动参数
	sRtn = GT_SetTrapPrm(new_axis, &trap);
	// 设置AXIS轴的目标位置
	sRtn = GT_SetPos(new_axis, len);
	// 设置AXIS轴的目标速度
	sRtn = GT_SetVel(new_axis, purose_speed);
	// 启动AXIS轴的运动
	sRtn = GT_Update(1<<(new_axis-1));
	do
	{
		// 读取AXIS轴的状态
		sRtn = GT_GetSts(new_axis, &sts);
		// 读取AXIS轴的规划位置
		sRtn = GT_GetPrfPos(new_axis, &prfPos);
	}while(sts&0x400); // 等待AXIS轴规划停止
	global_position.y += len;//更新当前位置信息
	HWND hWnd = ::FindWindow(NULL,_T("3_Axis_control"));
	SetDlgItemInt(hWnd,IDC_GLOBAL_POSITION_Y, global_position.y,TRUE);
}
void z_plus(int len)
{
	short sRtn;
	TTrapPrm trap;
	long sts;
	double prfPos;
	int new_axis = 4;
	// TODO: Add your control notification handler code here
	// 位置清零
	sRtn = GT_ZeroPos(new_axis);
	// AXIS轴规划位置清零
	sRtn = GT_SetPrfPos(new_axis, 0);
	// 将AXIS轴设为点位模式
	sRtn = GT_PrfTrap(new_axis);
	// 读取点位运动参数
	sRtn = GT_GetTrapPrm(new_axis, &trap);
	trap.acc = 0.05;
	trap.dec = 0.05;
	trap.smoothTime = 25;
	// 设置点位运动参数
	sRtn = GT_SetTrapPrm(new_axis, &trap);
	// 设置AXIS轴的目标位置
	sRtn = GT_SetPos(new_axis, len);
	// 设置AXIS轴的目标速度
	sRtn = GT_SetVel(new_axis, purose_speed);
	// 启动AXIS轴的运动
	sRtn = GT_Update(1<<(new_axis-1));
	do
	{
		// 读取AXIS轴的状态
		sRtn = GT_GetSts(new_axis, &sts);
		// 读取AXIS轴的规划位置
		sRtn = GT_GetPrfPos(new_axis, &prfPos);
	}while(sts&0x400); // 等待AXIS轴规划停止
	global_position.z += len;//更新当前位置信息
	HWND hWnd = ::FindWindow(NULL,_T("3_Axis_control"));
	SetDlgItemInt(hWnd,IDC_GLOBAL_POSITION_Z, global_position.z,TRUE);
}

void z_plus_Quick(int len)
{
	short sRtn;
	TTrapPrm trap;
	long sts;
	double prfPos;
	int new_axis = 4;
	// TODO: Add your control notification handler code here
	// 位置清零
	sRtn = GT_ZeroPos(new_axis);
	// AXIS轴规划位置清零
	sRtn = GT_SetPrfPos(new_axis, 0);
	// 将AXIS轴设为点位模式
	sRtn = GT_PrfTrap(new_axis);
	// 读取点位运动参数
	sRtn = GT_GetTrapPrm(new_axis, &trap);
	trap.acc = 1;
	trap.dec = 1;
	trap.smoothTime = 25;
	// 设置点位运动参数
	sRtn = GT_SetTrapPrm(new_axis, &trap);
	// 设置AXIS轴的目标位置
	sRtn = GT_SetPos(new_axis, len);
	// 设置AXIS轴的目标速度
	sRtn = GT_SetVel(new_axis, 100);
	// 启动AXIS轴的运动
	sRtn = GT_Update(1<<(new_axis-1));
	do
	{
		// 读取AXIS轴的状态
		sRtn = GT_GetSts(new_axis, &sts);
		// 读取AXIS轴的规划位置
		sRtn = GT_GetPrfPos(new_axis, &prfPos);
	}while(sts&0x400); // 等待AXIS轴规划停止
	global_position.z += len;//更新当前位置信息
	HWND hWnd = ::FindWindow(NULL,_T("3_Axis_control"));
	SetDlgItemInt(hWnd,IDC_GLOBAL_POSITION_Z, global_position.z,TRUE);
}

/*void CMy3_Axis_controlDlg::y_minus(int len)
{
	UpdateData(true);
	short sRtn;
	TTrapPrm trap;
	long sts;
	double prfPos;
	int new_axis = 2;
	// TODO: Add your control notification handler code here
	// 位置清零
	sRtn = GT_ZeroPos(new_axis);
	// AXIS轴规划位置清零
	sRtn = GT_SetPrfPos(new_axis, 0);
	// 将AXIS轴设为点位模式
	sRtn = GT_PrfTrap(new_axis);
	// 读取点位运动参数
	sRtn = GT_GetTrapPrm(new_axis, &trap);
	trap.acc = m_step_speed;
	trap.dec = m_step_speed;
	trap.smoothTime = 25;
	// 设置点位运动参数
	sRtn = GT_SetTrapPrm(new_axis, &trap);
	// 设置AXIS轴的目标位置
	sRtn = GT_SetPos(new_axis, len);
	// 设置AXIS轴的目标速度
	sRtn = GT_SetVel(new_axis, 1);
	// 启动AXIS轴的运动
	sRtn = GT_Update(1<<(new_axis-1));
	do
	{
		// 读取AXIS轴的状态
		sRtn = GT_GetSts(new_axis, &sts);
		// 读取AXIS轴的规划位置
		sRtn = GT_GetPrfPos(new_axis, &prfPos);
	}while(sts&0x400); // 等待AXIS轴规划停止
	UpdateData(false);
}*/

BOOL stop_hit = 0;//等于1表示要停止
BOOL process_h = 0;//正在加工H
BOOL process_i = 0;
BOOL process_t = 0;


static DWORD WINAPI ThreadFunc4(LPVOID pvoid)
{
	int x_len = -1000*0.44;
	int y_len = -5000*0.44;
	while(stop_hit != 1)
		{
			y_plus(-y_len);
			y_plus(y_len/2);
			x_plus(x_len);
			y_plus(y_len/2);
			y_plus(-y_len);
			y_plus(y_len/2);
			x_plus(-x_len);
			y_plus(y_len/2);
		}
	return 0;
}
static DWORD WINAPI ThreadFunc5(LPVOID pvoid)
{
	int x_len = -1000*0.33;
	int y_len = -5000*0.44;
	while(stop_hit != 1)
		{
			
			x_plus(x_len);
			x_plus(-x_len/2);
			y_plus(-y_len);
			x_plus(-x_len/2);
			x_plus(x_len);
			x_plus(-x_len/2);
			y_plus(y_len);
			x_plus(-x_len/2);
		}
	return 0;
}
static DWORD WINAPI ThreadFunc6(LPVOID pvoid)
{
	int x_len = -1400*0.44;
	int y_len = -5000*0.44;
	while(stop_hit != 1)
		{
			x_plus(x_len);
			x_plus(-x_len/2);
			y_plus(-y_len);
			y_plus(y_len);
			x_plus(-x_len/2);
		}
	return 0;
}
void CMy3_Axis_controlDlg::OnProcessH()
{
	// TODO: Add your control notification handler code here
	if (process_i == 1 || process_t == 1)
	{
		MessageBox("请先停止当前字母的加工！");
	}
	else
	{
		process_i == 1;
		process_t == 1;
		process_h == 1;
		stop_hit = 0;
		hThread4 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc4,NULL,0,&ThreadID4);
	}
	
}

void CMy3_Axis_controlDlg::OnProcessI() 
{
	// TODO: Add your control notification handler code here
	if (process_h == 1 || process_t == 1)
	{
		MessageBox("请先停止当前字母的加工！");
	}
	else
	{
		process_h == 1;
		process_t == 1;
		process_i == 1;
		stop_hit = 0;
		hThread5 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc5,NULL,0,&ThreadID5);
		
	}
}
void CMy3_Axis_controlDlg::OnProcessT() 
{
	// TODO: Add your control notification handler code here
	if (process_h == 1 || process_i == 1)
	{
		MessageBox("请先停止当前字母的加工！");
	}
	else
	{
		process_h == 1;
		process_i == 1;
		process_t == 1;
		stop_hit = 0;
		hThread6 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc6,NULL,0,&ThreadID6);
		
	}
	
	
}

void CMy3_Axis_controlDlg::OnStophit() 
{
	// TODO: Add your control notification handler code here
	stop_hit = 1;
}
static DWORD WINAPI ThreadFunc_line_interpolation(LPVOID pvoid)
{
	int x_step_len = 10;
	int y_step_len = 10;
	int total_x_length = 0;//目前插补已走过的X方向的总路程
	int total_y_length = 0;
	while(stop_hit != 1 && total_x_length < new_line_point.x && total_y_length < new_line_point.y   && cycle_times_line < max_cycle_times)
		{
//			cycle_times_line++;
			x_plus(x_step_len);
			total_x_length += x_step_len;
			y_plus(3.2 * y_step_len);
			total_y_length += y_step_len;
		}
	return 0;
}
void CMy3_Axis_controlDlg::OnTestLineInterpolation() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	stop_hit = 0;
	new_line_point.x = m_x_distance;
	new_line_point.y = m_y_distance;
	max_cycle_times = m_cycle_times_expect;
	hThread_line_interpolation = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc_line_interpolation,NULL,0,&ThreadID_line_interpolation);
	UpdateData(FALSE);
}
static DWORD WINAPI ThreadFunc_circle_interpolation(LPVOID pvoid)
{
	double step_angle = 1.0;
	double step_radian = step_angle * 3.1415926 / 180;
	int old_position_x = new_inf_circle_interpolation.R;//目前插补已走过的X方向的总路程
	int old_position_y = 0;
	
	int new_position_x = 0;
	int new_position_y = 0;
	double total_angle = 0;//已经走过的角度

//	while(stop_hit != 1 && total_angle <= new_inf_circle_interpolation.angle)
	while( !(stop_hit == 1 && flag_finish_circle == 1) && cycle_times_cylinder <max_cycle_times)
		{
			flag_finish_circle = 0;
			step_radian = total_angle * 3.1415926 / 180;
			new_position_x = new_inf_circle_interpolation.R * cos(step_radian);
			new_position_y = new_inf_circle_interpolation.R * sin(step_radian);

			x_plus(new_position_x - old_position_x);
			y_plus(3.2*(new_position_y - old_position_y));
			total_angle += step_angle;
			
			old_position_x = new_position_x;
			old_position_y = new_position_y;

			if (total_angle == 360)
			{
				total_angle = 0;
				flag_finish_circle = 1;
				cycle_times_cylinder++;
			}
		}
	return 0;
}
void CMy3_Axis_controlDlg::OnTestCircleInterpolation() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	times_click_circle ++;
	max_cycle_times = m_cycle_times_expect;
	if (times_click_circle == 1)
	{
		stop_hit = 0;
		new_inf_circle_interpolation.R = m_circle_R;
		new_inf_circle_interpolation.angle = 360;
		hThread_circle_interpolation = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc_circle_interpolation,NULL,0,&ThreadID_circle_interpolation);
	}
	else
	{
		if (stop_hit == 1 && flag_finish_circle == 1 )
		{
			stop_hit = 0;
			cycle_times_cylinder=0;
			new_inf_circle_interpolation.R = m_circle_R;
			new_inf_circle_interpolation.angle = 360;
			hThread_circle_interpolation = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc_circle_interpolation,NULL,0,&ThreadID_circle_interpolation);
		}
		else MessageBox("请先点击“停止加工”按钮，并等待当前加工进程结束");
	}
	UpdateData(FALSE);
}

static DWORD WINAPI ThreadFunc_turb_process(LPVOID pvoid)
{
	double step_angle = 1.0;
	double step_radian = step_angle * 3.1415926 / 180;
	int old_position_x = 0;//目前插补已走过的X方向的总路程
	int old_position_y = 0;
	
	int new_position_x = 0;
	int new_position_y = 0;
	double total_angle = 60;//已经走过的角度
	double finally_angle = 0;

	bool flag = 0;//标志位，是否到达真正的起点，是-->1
	for (int i = 0; i < 6; i++)
	{
		
		//先走到起点 --> Z轴抬起1000个脉冲，XY运动到指定位置，Z轴下降1000个脉冲
		z_plus(1000);
//		x_plus(initial_point_turn[i].x);
//     		y_plus(3.2 * initial_point_turn[i].y);
		
		old_position_x = destination_point_turb[i].x;
		old_position_y = destination_point_turb[i].y;
		x_plus(old_position_x);
		y_plus(3.2 * old_position_y);
		finally_angle = total_angle + 60;
		
		while(stop_hit != 1 && total_angle <= finally_angle)
		{
			step_radian = total_angle * 3.1415926 / 180;
			new_position_x = new_inf_circle_interpolation.R * cos(step_radian) + initial_point_turn[i].x;
			new_position_y = new_inf_circle_interpolation.R * sin(step_radian) + initial_point_turn[i].y;

			x_plus(new_position_x - old_position_x);
			y_plus(3.2*(new_position_y - old_position_y));
			if (flag == 0)
			{
				flag = 1;
				z_plus(-1000);
			}
			total_angle += step_angle;
			
			old_position_x = new_position_x;
			old_position_y = new_position_y;
		}
		flag = 0;
	}

	total_angle = 0;//已经走过的角度
	x_plus(new_inf_circle_interpolation.R);
	while(stop_hit != 1 && total_angle <= 360)
		{
			step_radian = total_angle * 3.1415926 / 180;
			new_position_x = new_inf_circle_interpolation.R * cos(step_radian);
			new_position_y = new_inf_circle_interpolation.R * sin(step_radian);

			x_plus(new_position_x - old_position_x);
			y_plus(3.2*(new_position_y - old_position_y));
			total_angle += step_angle;
			
			old_position_x = new_position_x;
			old_position_y = new_position_y;
		}

	
	return 0;
}

void CMy3_Axis_controlDlg::OnTurbProcess() 
{
	// TODO: Add your control notification handler code here
	stop_hit = 0;
	new_inf_circle_interpolation.R = 5000;
	new_inf_circle_interpolation.angle = 3.1415926 * 2 / 3;//0°-60°
	int temp_R = new_inf_circle_interpolation.R;

	destination_point_turb[0].x = temp_R;
	destination_point_turb[0].y = 0;

	destination_point_turb[1].x = temp_R/2;
	destination_point_turb[1].y = temp_R*1.73/2;

	destination_point_turb[2].x = -temp_R/2;
	destination_point_turb[2].y = temp_R*1.73/2;

	destination_point_turb[3].x = -temp_R;
	destination_point_turb[3].y = 0;

	destination_point_turb[4].x = -temp_R/2;
	destination_point_turb[4].y = -temp_R*1.73/2;

	destination_point_turb[5].x = temp_R/2;
	destination_point_turb[5].y = -temp_R*1.73/2;
	
	initial_point_turn[0].x = temp_R / 2;
	initial_point_turn[0].y = - temp_R * 1.732 / 2;

	initial_point_turn[1].x = temp_R;
	initial_point_turn[1].y = 0;

	initial_point_turn[2].x = temp_R / 2;
	initial_point_turn[2].y = temp_R * 1.732 / 2;

	initial_point_turn[3].x = -temp_R / 2;
	initial_point_turn[3].y = temp_R * 1.732 / 2;

	initial_point_turn[4].x = -temp_R;
	initial_point_turn[4].y = 0;

	initial_point_turn[5].x = -temp_R / 2;
	initial_point_turn[5].y = -temp_R * 1.732 / 2;

	hThread_circle_interpolation = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc_turb_process,NULL,0,&ThreadID_circle_interpolation);

}

static DWORD WINAPI ThreadFunc_tu_ball(LPVOID pvoid)
{
	double step_angle = 5;
	int step_R = 1;
	double step_radian = step_angle * 3.1415926 / 180;
	int old_position_x = new_inf_tu_ball.min_R;
	int old_position_y = 0;
	int R_new = new_inf_tu_ball.min_R;
	
	int new_position_x = 0;
	int new_position_y = 0;
	double total_angle = 0;//已经走过的角度

	vector <my_point> point_of_path;
	my_point temp_destination_point;
//	while(stop_hit != 1 && total_angle <= new_inf_circle_interpolation.angle)
	while(R_new <= new_inf_tu_ball.max_R)
	{
		step_radian = total_angle * 3.1415926 / 180;
		new_position_x = R_new * cos(step_radian);
		new_position_y = R_new * sin(step_radian);

		temp_destination_point.x = new_position_x;
		temp_destination_point.y = new_position_y;
		point_of_path.push_back(temp_destination_point);

		total_angle += step_angle;
		R_new += step_R;
	}
	while( stop_hit == 0  && cycle_times_tuball < max_cycle_times)
		{
			cycle_times_tuball++;
			for (int i = 0; i < point_of_path.size(); i ++)
			{
				new_position_x = point_of_path[i].x;
				new_position_y = point_of_path[i].y;

				x_plus(new_position_x - old_position_x);
				y_plus(3.2*(new_position_y - old_position_y));

				old_position_x = new_position_x;
				old_position_y = new_position_y;
			}
			for (int i = point_of_path.size()-1; i >= 0; i --)
			{
				new_position_x = point_of_path[i].x;
				new_position_y = point_of_path[i].y;

				x_plus(new_position_x - old_position_x);
				y_plus(3.2*(new_position_y - old_position_y));

				old_position_x = new_position_x;
				old_position_y = new_position_y;
			}
		}
	return 0;
}
void CMy3_Axis_controlDlg::OnTuoBall() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	times_click_helix ++;
	max_cycle_times = m_cycle_times_expect;
	if (times_click_helix == 1)
	{
		stop_hit = 0;
		new_inf_tu_ball.min_R = m_min_r;
		new_inf_tu_ball.max_R = m_max_r;
		new_inf_tu_ball.angle = 360;
		hThread_tu_ball = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc_tu_ball,NULL,0,&ThreadID_circle_interpolation);
	}
	else MessageBox("请退出系统重新开始加工");
	UpdateData(FALSE);
}



void CMy3_Axis_controlDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CMy3_Axis_controlDlg::OnBnClickedBackToStart()
{
	// TODO: 在此添加控件通知处理程序代码
	if (flag_servo_on == 0)
	{
		MessageBox(_T("请先开启伺服"));
	}
	else
	{
//		if(MessageBox(_T("提醒")_T("确认要返回起点吗？"),MB_OK | MB_OKCANCEL) == IDOK)
//		{
			x_plus(-global_position.x);
			y_plus(-global_position.y);
			z_plus(-global_position.z);
//		}
		
	}
}

static DWORD WINAPI ThreadFunc_short_back(LPVOID pvoid)
{
	while(flag_short_back == 1 && flag_open_feed == 0)
	{
		z_plus_Quick(500);
	}
	return 0;
}

static DWORD WINAPI ThreadFunc_open_feed(LPVOID pvoid)
{
	while(flag_open_feed == 1 &&  flag_short_back== 0)
	{
		z_plus(-200);
	}
	return 0;
}

void CMy3_Axis_controlDlg::OnBnClickedIntelligentproces()
{
	// TODO: 在此添加控件通知处理程序代码
	CWnd *h1;
	CString str;
//	flag_intelligent_time ++;
	h1 = GetDlgItem(IDC_Intelligent_proces);

	if(flag_intelligent_time == 1) //结束智能加工
	{
		UpdateData(true);
		flag_intelligent_time = 0;
		str=_T("开启智能加工");
		flag_short_back = 0;
		flag_open_feed = 0;
		flag_intelligent_proces = 0;
		
		h1->SetWindowText(str);
		UpdateData(FALSE);
		//改变按钮名称为打开串口
	}
	else//开启智能加工
	{
		UpdateData(true);
		flag_intelligent_time = 1;
		str=_T("结束智能加工");
		flag_short_back = 0;
		flag_open_feed = 0;
		flag_intelligent_proces = 1;
		
		h1->SetWindowText(str);
		UpdateData(FALSE);
	}

	
	
}

void CMy3_Axis_controlDlg::OnBnClickedTestShort()
{
	// TODO: 在此添加控件通知处理程序代码
	if (flag_intelligent_proces == 1)
	{
		flag_short_back = 1;
		flag_open_feed = 0;
		hThread_short_back = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc_short_back,NULL,0,&ThreadID_short_back);
	}
	else MessageBox(_T("请先点击智能加工！"));
//	
}


void CMy3_Axis_controlDlg::OnBnClickedTestOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	if (flag_intelligent_proces == 1)
	{
		flag_short_back = 0;
		flag_open_feed = 1;
		hThread_open_feed = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc_open_feed,NULL,0,&ThreadID_open_feed);
	}
	else MessageBox(_T("请先点击智能加工！"));
	
}

void CMy3_Axis_controlDlg::OnBnClickedNormalProces()
{
	// TODO: 在此添加控件通知处理程序代码
	if (flag_intelligent_proces == 1)
	{
		flag_short_back = 0;
		flag_open_feed = 0;
	}
	else MessageBox(_T("请先点击智能加工！"));
}

void CMy3_Axis_controlDlg::OnBnClickedOpenSerialPort()
{
	// TODO: 在此添加控件通知处理程序代码
	flag_stop=0;
	CString str;
	CString str1;
	CString n;
	GetDlgItemText(IDC_OPEN_SERIAL_PORT,str);
	CWnd *h1;
	h1 = GetDlgItem(IDC_OPEN_SERIAL_PORT);

	//指向控件的caption
	
	if(!m_ctrlComm.get_PortOpen())
	{
		m_bt_select.GetLBText(m_bt_select.GetCurSel(),str1);//取得所选的字符串，并存放在str1里面???
		str1=str1+','+'n'+','+'8'+','+'1';
		//这句话很关键
		m_ctrlComm.put_CommPort((m_com_select.GetCurSel()+1));//选择串口???
		m_ctrlComm.put_InputMode(1);//设置输入方式为二进制方式
		m_ctrlComm.put_Settings(str1);//波特率为（波特率组á合框）无校验，8数据位，1个停止位???
		m_ctrlComm.put_InputLen(1024);//设置当前接收区数据长度为1024???
		m_ctrlComm.put_RThreshold(1);//缓冲区一个字符引发事件???
		m_ctrlComm.put_RTSEnable(1);
		//设置RT允许
		m_ctrlComm.put_PortOpen(true);
		//打开串口
		if(m_ctrlComm.get_PortOpen())
		{
			str=_T("关闭串口");
			UpdateData(true);
			h1->SetWindowText(str);//改变按钮名称为‘’关闭串口"
		}
	}
	else
	{
		m_ctrlComm.put_PortOpen(false);
		if(str!=_T("打开串口"))
		{
			str=_T("打开串口");
			UpdateData(true);
			h1->SetWindowText(str);
			//改变按钮名称为打开串口
		}
	}
}
BEGIN_EVENTSINK_MAP(CMy3_Axis_controlDlg, CDialog)
	ON_EVENT(CMy3_Axis_controlDlg, IDC_MSCOMM1, 1, CMy3_Axis_controlDlg::OnComm, VTS_NONE)
END_EVENTSINK_MAP()


void CMy3_Axis_controlDlg::OnComm()
{
	// TODO: 在此处添加消息处理程序代码
	UpdateData(true);
	VARIANT variant_inp;   //Variant 是一种特殊的数据类型，除了定长String数据及用户定义类型外，可以包含任何种类的数据。  
	COleSafeArray safearray_inp;       
	LONG len,k;      
	BYTE rxdata[2048]; //设置BYTE数组 An 8-bit integer that is not signed.       
	CString strtemp;
	int change_string_to_int3 = 0;

	int port_open = 0;
	int port_short = 0;

	int num = 0;
	//得到的数据为：1070000000-1090000000
	if(m_ctrlComm.get_CommEvent() == 2) //事件值为2表示接收缓冲区内有字符       
	{              
		////////以下你可以根据自己的通信协议加入处理代码
//		m_strRXData = "";   
		variant_inp=m_ctrlComm.get_Input();     //读缓冲区    
		safearray_inp=variant_inp;              //VARIANT型变量转换为ColeSafeArray型变量  
		len=safearray_inp.GetOneDimSize();      //得到有效数据长度          
		for(k=0;k<len;k++)               
			safearray_inp.GetElement(&k,rxdata+k);//转换为BYTE型数组
		for(k=0;k<len;k++)                    //将数组转换为Cstring型变量      
		{
			BYTE bt=*(char*)(rxdata+k);//字符型        
//			strtemp.Format(L"%c",bt); //将字符送入临时变量strtemp存放     
			m_strRXData+=bt; //加入接收编辑框对应字符串
			num ++;

			if (num == 1)
			{
				port_short = bt - '0';
				
			}
			else if (num == 2)
			{
				num = 0;
				port_open = bt - '0';
				
			}
			UpdateData(FALSE);
			if (port_short == 1 && port_open == 0 && flag_intelligent_proces == 1)
			{
				flag_open_feed = 0;
				flag_short_back = 1;
				hThread_short_back = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc_short_back,NULL,0,&ThreadID_short_back);
			}
			else if (port_open == 1 && port_short == 0 && flag_intelligent_proces == 1)
			{
				flag_open_feed = 1;
				flag_short_back = 0;
				hThread_open_feed = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc_open_feed,NULL,0,&ThreadID_open_feed);
			}
			else
			{
				flag_open_feed = 0;
				flag_short_back = 0;
			}
			m_strRXData = (_T(""));
/*			if (num == 10)
			{
				change_string_to_int3 = (m_strRXData[0] - '0') * 1000 + (m_strRXData[1] - '0') * 100 + (m_strRXData[2] - '0')*10 + (m_strRXData[3] - '0');
				m_voltage.Format(_T("%d"),change_string_to_int3);
				UpdateData(FALSE);
				
				num = 0;
				if (flag_intelligent_proces == 1)
				{
					if (change_string_to_int3 >1083)
					{
						flag_open_feed = 1;
						flag_short_back = 0;
						hThread_open_feed = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc_open_feed,NULL,0,&ThreadID_open_feed);
					}
					else if (change_string_to_int3 >1080 && change_string_to_int3 <= 1083)
					{
						flag_open_feed = 0;
						flag_short_back = 0;
					}
					else
					{
						flag_open_feed = 0;
						flag_short_back = 1;
						hThread_short_back = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc_short_back,NULL,0,&ThreadID_short_back);
					}
				}
				
				m_strRXData = (_T(""));
			}*/
		}
		
		UpdateData(FALSE);
		
	}
	m_strRXData = (_T(""));
	m_voltage = "";	
	
}
static DWORD WINAPI Thread_process_cubio_1(LPVOID pvoid)
{
	while(stop_hit == 0 && cycle_times_cuboid < max_cycle_times)
	{
		cycle_times_cuboid++;
		y_plus(3.2*length_bottom_edge/2);
		x_plus(-length_bottom_edge);
		y_plus(-3.2*length_bottom_edge);
		x_plus(length_bottom_edge);
		y_plus(3.2*length_bottom_edge/2);
	}
	return 0;
	
}
LRESULT CMy3_Axis_controlDlg::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	// TODO: 处理用户自定义消息 
	UpdateData(TRUE);//更新控件的值
	cycle_times_cuboid = *((int*)lParam);

	UpdateData(FALSE);
	return 0;
}
static DWORD WINAPI Thread_process_cubio_2(LPVOID pvoid)
{
	int len = 0;
	int step_len = 50;
	CMy3_Axis_controlDlg *pDlg = (CMy3_Axis_controlDlg *)pvoid;  
	int * temp_value = (int *)pvoid;
	while(stop_hit == 0 && cycle_times_cuboid < max_cycle_times)
	{
		cycle_times_cuboid++;
		temp_value++;
		len = 0;
		while(len<length_bottom_edge)
		{
			y_plus(3.2*length_bottom_edge);
			x_plus(-step_len);
			len += step_len;
			y_plus(-3.2*length_bottom_edge);
			x_plus(-step_len);
			len += step_len;
		}
		len = length_bottom_edge;
		while(len>0)
		{
			y_plus(3.2*length_bottom_edge);
			x_plus(step_len);
			len -= step_len;
			y_plus(-3.2*length_bottom_edge);
			x_plus(step_len);
			len -= step_len;
		}
		SendMessage(pDlg->m_hWnd,WM_MY_MESSAGE, 0, (LPARAM) cycle_times_cuboid);
//		::SetDlgItemText(AfxGetMainWnd()->m_hWnd, IDC_CYCLE_TIMES_NOW,*((LPCSTR *)cycle_times_cuboid) );
		
	}
	return 0;

}
void CMy3_Axis_controlDlg::OnBnClickedProcesCuboid()
{
	// TODO: 在此添加控件通知处理程序代码

	//方案1：重复走正方形四条边，中间剩余一个正方形
/*	UpdateData(TRUE);
	length_bottom_edge = m_length_bottom_edge;
	max_cycle_times = m_cycle_times_expect;
	times_click_cuboid++;
	if (times_click_cuboid == 1)
	{
		stop_hit = 0;
		hThread_process_cubio_1 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Thread_process_cubio_1,NULL,0,&ThreadID_process_cubio_1);
	}
	UpdateData(FALSE);*/

	//方案2：走“之”字，加工一个正方形平面

	UpdateData(TRUE);
	length_bottom_edge = m_length_bottom_edge;
	max_cycle_times = m_cycle_times_expect;
	times_click_cuboid++;
	if (times_click_cuboid == 1)
	{
		stop_hit = 0;
		hThread_process_cubio_2 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Thread_process_cubio_2,&cycle_times_cuboid,0,&ThreadID_process_cubio_2);
	}
	UpdateData(FALSE);


}
