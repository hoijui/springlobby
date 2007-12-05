#ifndef SPRINGLOBBY_HEADERGUARD_MAINWINDOW_H
#define SPRINGLOBBY_HEADERGUARD_MAINWINDOW_H

#include <wx/frame.h>

class Ui;
class Channel;
class User;
class wxCommandEvent;
class wxListbookEvent;
class MainChatTab;
class MainJoinBattleTab;
class MainSinglePlayerTab;
class wxBoxSizer;
class wxListbook;
class MainOptionsTab;
class wxBitmap;
class wxImageList;
class ChatPanel;

// FIXME shouldn't copy this here
typedef wxWindow wxNotebookPage;


// Page indexes
#define PAGE_CHAT 0
#define PAGE_JOIN 1
#define PAGE_SINGLE 2
#define PAGE_OPTOS 3

//! @brief wxFrame that contains the main window of the client.
class MainWindow : public wxFrame
{
  public:
    MainWindow( Ui& ui );
    virtual ~MainWindow();

    // MainWindow interface
    void OpenChannelChat( Channel& channel );
    void OpenPrivateChat( User& user );

    void CloseAllChats();

    void ShowConfigure();

    void OnMenuJoin( wxCommandEvent& event );
    void OnMenuChat( wxCommandEvent& event );
    void OnMenuConnect( wxCommandEvent& event );
    void OnMenuDisconnect( wxCommandEvent& event );
    void OnMenuQuit( wxCommandEvent& event );
    void OnUnitSyncReload( wxCommandEvent& event );
    void OnReportBug( wxCommandEvent& event );
    void OnShowDocs( wxCommandEvent& event );
    void OnShowSettingsPP( wxCommandEvent& event );

    void OnUnitSyncReloaded();

    void OnTabsChanged( wxListbookEvent& event );
    MainChatTab& GetChatTab();
    MainJoinBattleTab& GetJoinTab();
    MainSinglePlayerTab& GetSPTab();
    ChatPanel* GetActiveChatPanel();
    ChatPanel* GetChannelChatPanel( const wxString& channel );
    void MakeImages();

  protected:
    // MainWindow variables

    Ui& m_ui;

    wxBoxSizer* m_main_sizer;
    wxListbook* m_func_tabs;
    wxNotebookPage* m_chat_page;

    MainChatTab* m_chat_tab;
    MainJoinBattleTab* m_join_tab;
    MainSinglePlayerTab* m_sp_tab;
    MainOptionsTab* m_opts_tab;

    wxBitmap* m_chat_icon;
    wxBitmap* m_battle_icon;
    wxBitmap* m_options_icon;
    wxBitmap* m_sp_icon;
    wxBitmap* m_select_image;

    wxImageList* m_func_tab_images;
    
    DECLARE_EVENT_TABLE()
};

//ChatPanel& servwin();

// wxWidget IDs
enum
{
    MENU_ABOUT = wxID_ABOUT,
    MENU_QUIT = wxID_EXIT,

    MENU_CONNECT = wxID_HIGHEST,
    MENU_DISCONNECT,
    MENU_JOIN,
    MENU_USYNC,
    MENU_TRAC,
    MENU_DOC,
    MENU_CHAT,
    MAIN_TABS

};

#endif // SPRINGLOBBY_HEADERGUARD_MAINWINDOW_H
