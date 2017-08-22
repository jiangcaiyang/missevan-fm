#include "StdAfx.h"
#include "MainTray.h"

#include <base/operation/fileselect.h>
#include <common/strconv.h>
#include "base/global.h"
#include "ui/MainWindow.h"
#include "ui/Menu.h"
#include "Server.h"
#include "UserAccount.h"
#include "DeviceManager.h"
#include "LivePublisher.h"
#include "MissEvanFMWindow.h"

enum TrayMenuType {
	kTMTUser = 201,
	kTMTPush,
	kTMTPull,
	kTMTUpdate,
	kTMTAbout,
	kTMTExit,
};

enum TrayPushMenuType {
	kTMTState = 301,
	kTMTAudioMic,
	kTMTAudioHook,
};


CMainTray::CMainTray()
	: m_mainMenu(NULL)
	, m_pushMenu(NULL)
	, m_server_ptr(NULL)
	, m_dm(NULL)
{
}


CMainTray::~CMainTray()
{
	if (m_mainMenu) {
		delete m_mainMenu;
	}
	if (m_pushMenu) {
		delete m_pushMenu;
	}
}

BOOL CMainTray::Create(HWND hWnd, UINT uCallbackMessage, LPCTSTR szTip, HICON icon, UINT uID, BOOL bIsNotify, LPCTSTR szWindowTitle)
{
	int ret = CTrayIcon::Create(hWnd, uCallbackMessage, szTip, icon, uID, bIsNotify, szWindowTitle);

	if (ret) {
		m_pushMenu = new CMenu;
		m_mainMenu = new CMenu;
		m_pushMenu->CreatePopup();

		m_pushMenu->Append(kTMTState, L"空闲中");
		m_pushMenu->Append(kTMTAudioMic, L"打开麦克风");
		m_pushMenu->Append(kTMTAudioHook, L"设置背景乐");

		m_pushMenu->EnableItem(kTMTState, false, false);
		m_pushMenu->EnableItem(kTMTAudioMic, false, false);
		m_pushMenu->EnableItem(kTMTAudioHook, false, false);

		m_mainMenu->CreatePopup();

		m_mainMenu->Append(kTMTUser, L"用户（未登录）");
		m_mainMenu->Append(kTMTPush, L"推流", MF_STRING, m_pushMenu);
		m_mainMenu->Append(kTMTPull, L"拉流（空闲中）");
		m_mainMenu->Append(kTMTUpdate, L"检查更新");
		m_mainMenu->Append(kTMTAbout, L"关于");
		m_mainMenu->Append(kTMTExit, L"退出");

		//m_mainMenu->EnableItem(kTMTUser, false, false);
		m_mainMenu->EnableItem(kTMTPull, false, false);

		SetMenu(m_mainMenu);
	}

	return ret;
}

void CMainTray::SetServer(Server *p_server)
{
	m_server_ptr = p_server;
	m_dm = DeviceManager::GetInstance();
}

void CMainTray::UpdateMenu()
{
	if (m_server_ptr) {
		//m_server_ptr->
		//kStatUser;
		//kStatChat;
		//kStatPlayer
		uint32_t stat = m_server_ptr->GetStat();
		if (stat & Server::kStatUser) {
			std::wstring userStr = L"用户（";
			userStr += m_server_ptr->m_user_ptr->GetUsername();
			userStr += L"）";
			m_mainMenu->Modify(kTMTUser, userStr.c_str());
		} else {
			m_mainMenu->Modify(kTMTUser, L"用户（未登录）");
		}
		if (stat & Server::kStatPushConnect || stat & Server::kStatPushLive) {
			m_pushMenu->Modify(kTMTState, L"推流中");
			if (stat & Server::kStatPushLive) {
				bool enabled = m_server_ptr->m_publisher_ptr->IsLoopbackEnabled();
				m_pushMenu->Modify(kTMTAudioMic, L"关闭麦克风");
				if (enabled) {
					m_pushMenu->Modify(kTMTAudioHook, L"关闭背景乐");
				} else {
					m_pushMenu->Modify(kTMTAudioHook, L"设置背景乐");
				}
				m_pushMenu->EnableItem(kTMTAudioMic, false, false);
			} else {
				if (m_dm->IsMicOpened()) {
					m_pushMenu->Modify(kTMTAudioMic, L"关闭麦克风");
				}
				else {
					m_pushMenu->Modify(kTMTAudioMic, L"打开麦克风");
				}
				if (m_dm->IsAudioHooked()) {
					m_pushMenu->Modify(kTMTAudioHook, L"关闭背景乐");
				} else {
					m_pushMenu->Modify(kTMTAudioHook, L"设置背景乐");
				}
				m_pushMenu->EnableItem(kTMTAudioMic, true, false);
			}
			m_pushMenu->EnableItem(kTMTAudioHook, true, false);
		} else {
			m_pushMenu->Modify(kTMTState, L"空闲中");
			m_pushMenu->Modify(kTMTAudioMic, L"打开麦克风");
			m_pushMenu->Modify(kTMTAudioHook, L"设置背景乐");
			m_pushMenu->EnableItem(kTMTAudioMic, false, false);
			m_pushMenu->EnableItem(kTMTAudioHook, false, false);
		}
		m_pushMenu->EnableItem(kTMTState, false, false);
		if (stat & Server::kStatPlayer) {
			m_mainMenu->Modify(kTMTPull, L"拉流中");
			m_mainMenu->EnableItem(kTMTPull, true, false);
		} else {
			m_mainMenu->Modify(kTMTPull, L"拉流（空闲中）");
			m_mainMenu->EnableItem(kTMTPull, false, false);
		}
	}
}

LRESULT CMainTray::OnTrayNotification(WPARAM uID, LPARAM lEvent)
{
	switch (LOWORD(lEvent)) {
	case NIN_BALLOONUSERCLICK:
		ShellExecute(0, 0, L"https://fm.missevan.com", 0, 0, SW_SHOW);
		return 1;
	case WM_RBUTTONUP:
		UpdateMenu();
		break;
	}
	return CTrayIcon::OnTrayNotification(uID, lEvent);
}

LRESULT CMainTray::OnTrayMenu(UINT wParam, LONG lParam)
{
	switch (wParam) {
	case kTMTAudioMic: {
		uint32_t stat = m_server_ptr->GetStat();
		if (stat & Server::kStatPushConnect) {
			if (m_dm->IsMicOpened()) {
				m_dm->CloseMic();
			} else {
				m_dm->OpenMic();
			}
		}
	}
	break;
	case kTMTAudioHook: {
		// check live mode
		uint32_t stat = m_server_ptr->GetStat();
		if (stat & Server::kStatPushLive) {
			bool enabled = m_server_ptr->m_publisher_ptr->IsLoopbackEnabled();
			if (!enabled) {
				MessageBox(m_hWnd, L"高清模式下，本程序将会把系统播放的声音一并直播。", L"提示", MB_OK);
			}
			m_server_ptr->m_publisher_ptr->EnableLookbackCapture(!enabled);
		} else if (stat & Server::kStatPushConnect) {
			if (m_dm->IsAudioHooked()) {
				m_dm->EndHookAudio();
			} else {
				MessageBox(m_hWnd, L"请选择一个程序，本程序将会把其所播放的音乐一并直播，例如网易云音乐。", L"提示", MB_OK);
					operation::CFileSelect fsel(m_hWnd, operation::kOpen, L"可执行文件(*.exe)|*.exe||", L"请选择一个程序");
				if (fsel.Select()) {
					CW2C w2c(fsel.GetPath().c_str());
					m_dm->StartHookAudio(w2c.c_str());
				}
			}
		}
	}
	break;
	case kTMTUpdate: {
		((CMissEvanFMWindow *)global::mainWindow)->InitUpdate(false);
		break;
	}
	case kTMTAbout: {
		global::mainWindow->Show();
	}
	break;
	case kTMTExit: {
		// close main window, mark can close
		PostMessage(m_hWnd, WM_CLOSE, 1, 0);
	}
	break;
	default: {
		return 0;
	}
	}
	return 1;
}
