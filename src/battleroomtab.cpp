/* Copyright (C) 2007, 2008 The SpringLobby Team. All rights reserved. */
//
// Class: BattleRoomTab
//

#include <wx/splitter.h>
#include <wx/intl.h>
#include <wx/combobox.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/choicdlg.h>
#include <wx/colordlg.h>
#include <wx/colour.h>
#include <wx/log.h>
#include <wx/bmpcbox.h>
#include <wx/image.h>
#include <wx/choice.h>
#if wxUSE_TOGGLEBTN
#include <wx/tglbtn.h>
#endif

#include <stdexcept>

#include "battleroomtab.h"
#include "ui.h"
#include "iunitsync.h"
#include "user.h"
#include "battle.h"
#include "utils.h"
#include "battleroomlistctrl.h"
#include "chatpanel.h"
#include "mapctrl.h"
#include "uiutils.h"
#include "addbotdialog.h"
#include "server.h"
#include "iconimagelist.h"
#include "settings++/custom_dialogs.h"
#include "autobalancedialog.h"
#include "settings.h"
#include "Helper/colorbutton.h"
#include "mapselectdialog.h"
#include "mmoptionwindows.h"
#include "aui/auimanager.h"

BEGIN_EVENT_TABLE(BattleRoomTab, wxPanel)

    EVT_BUTTON ( BROOM_START, BattleRoomTab::OnStart )
    EVT_BUTTON ( BROOM_LEAVE, BattleRoomTab::OnLeave )
    EVT_BUTTON ( BROOM_ADDBOT, BattleRoomTab::OnAddBot )

    EVT_CHECKBOX( BROOM_IMREADY, BattleRoomTab::OnImReady )
    EVT_CHECKBOX( BROOM_SPEC, BattleRoomTab::OnImSpec )
    EVT_COMBOBOX( BROOM_TEAMSEL, BattleRoomTab::OnTeamSel )
    EVT_COMBOBOX( BROOM_ALLYSEL, BattleRoomTab::OnAllySel )
    EVT_BUTTON( BROOM_COLOURSEL, BattleRoomTab::OnColourSel )
    EVT_COMBOBOX( BROOM_SIDESEL, BattleRoomTab::OnSideSel )

    EVT_COMBOBOX( BROOM_PRESETSEL, BattleRoomTab::OnPresetSel )
    EVT_BUTTON( BROOM_SAVEPRES, BattleRoomTab::OnSavePreset )
    EVT_BUTTON( BROOM_DELETEPRES, BattleRoomTab::OnDeletePreset )
    EVT_BUTTON( BROOM_SETDEFAULTPRES, BattleRoomTab::OnSetModDefaultPreset )

    EVT_BUTTON( BROOM_MAP_BROWSE, BattleRoomTab::OnMapBrowse )
    EVT_COMBOBOX( BROOM_MAP_SEL, BattleRoomTab::OnMapSelect )

		EVT_CHECKBOX( BROOM_LOCK, BattleRoomTab::OnLock )
    #if  wxUSE_TOGGLEBTN
    EVT_TOGGLEBUTTON( BROOM_AUTOLOCK, BattleRoomTab::OnAutoLock )
    #else
    EVT_CHECKBOX( BROOM_AUTOLOCK, BattleRoomTab::OnAutoLock )
    #endif

    EVT_BUTTON( BROOM_MANAGE_MENU, BattleRoomTab::OnShowManagePlayersMenu )

		EVT_MENU( BROOM_AUTOHOST, BattleRoomTab::OnAutoHost )
    EVT_MENU( BROOM_LOCK_BALANCE, BattleRoomTab::OnLockBalance )
    EVT_MENU ( BROOM_BALANCE, BattleRoomTab::OnBalance )
    EVT_MENU ( BROOM_FIXID, BattleRoomTab::OnFixTeams )
    EVT_MENU ( BROOM_FIXCOLOURS, BattleRoomTab::OnFixColours )

    EVT_LIST_ITEM_ACTIVATED( BROOM_OPTIONLIST, BattleRoomTab::OnOptionActivate)

END_EVENT_TABLE()

template < int N, int S = 1 >
class MyStrings : public wxArrayString
{
    public:
    MyStrings()
    {
        for ( int i = S; i <= N; ++i)
            Add( TowxString(i) );
    }
};

const MyStrings<SPRING_MAX_TEAMS> team_choices;
const MyStrings<SPRING_MAX_ALLIES> ally_choices;

BattleRoomTab::BattleRoomTab( wxWindow* parent, Ui& ui, Battle& battle ) :
        wxScrolledWindow( parent, -1 ),m_ui(ui), m_battle(battle)
{
    GetAui().manager->AddPane( this, wxLEFT, _T("battleroomtab") );

    // Create all widgets
    m_splitter = new wxSplitterWindow( this, -1, wxDefaultPosition, wxSize(100, 60) );

    UserBattleStatus& myself = m_battle.GetMe().BattleStatus();

    m_player_panel = new wxScrolledWindow( m_splitter , -1 );
    m_player_panel->SetScrollRate( 3, 3 );
    m_team_sel = new wxComboBox( m_player_panel, BROOM_TEAMSEL, _T("1"), wxDefaultPosition, wxSize(50,CONTROL_HEIGHT), team_choices );
    m_team_sel->SetToolTip(TE(_("Players with the same team number share control of their units.")));
    m_ally_sel = new wxComboBox( m_player_panel, BROOM_ALLYSEL, _T("1"), wxDefaultPosition, wxSize(50,CONTROL_HEIGHT), ally_choices );
    m_ally_sel->SetToolTip(TE(_("Players with the same ally number work together to achieve victory.")));
    m_color_sel = new ColorButton( m_player_panel, BROOM_COLOURSEL, myself.colour, wxDefaultPosition, wxSize(-1,CONTROL_HEIGHT) );
    m_color_sel->SetToolTip(TE(_("Select a color to identify your units in-game")));
    m_side_sel = new wxBitmapComboBox( m_player_panel, BROOM_SIDESEL, _T(""), wxDefaultPosition, wxSize(-1,CONTROL_HEIGHT) );
    m_side_sel->SetToolTip(TE(_("Select your faction")));
    m_spec_chk = new wxCheckBox( m_player_panel, BROOM_SPEC, _("Spectator"), wxDefaultPosition, wxSize(-1,CONTROL_HEIGHT) );
    m_spec_chk->SetToolTip(TE(_("Spectate (watch) the battle instead of playing")));
    m_ready_chk = new wxCheckBox( m_player_panel, BROOM_IMREADY, _("I'm ready"), wxDefaultPosition, wxSize(-1,CONTROL_HEIGHT) );
    m_ready_chk->SetToolTip(TE(_("Click this if you are content with the battle settings.")));

    try
    {
        wxArrayString sides = usync().GetSides( m_battle.GetHostModName() );
        for ( unsigned int i = 0; i < sides.GetCount(); i++ )
        {
            m_side_sel->Append( sides[i], icons().GetBitmap( icons().GetSideIcon( m_battle.GetHostModName(), i ) ) );
        }
    }
    catch (...) {}

    m_team_lbl = new wxStaticText( m_player_panel, -1, _("Team") );
    m_ally_lbl = new wxStaticText( m_player_panel, -1, _("Ally") );
    m_color_lbl = new wxStaticText( m_player_panel, -1, _("Color") );
    m_side_lbl = new wxStaticText( m_player_panel, -1, _("Side") );

    m_size_lbl = new wxStaticText( this, -1, _T("") );
    m_wind_lbl = new wxStaticText( this, -1, _T("") );
    m_tidal_lbl = new wxStaticText( this, -1, _T("") );

    m_map_combo = new wxComboBox( this, BROOM_MAP_SEL, _T(""), wxDefaultPosition, wxDefaultSize );

    m_minimap = new MapCtrl( this, 162, &m_battle, m_ui, true, true, true, false );
    m_minimap->SetToolTip(TE(_("A preview of the selected map.  You can see the starting positions, or (if set) starting boxes.")));

    m_browse_map_btn = new wxButton( this, BROOM_MAP_BROWSE, _("Map"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    m_browse_map_btn->SetSize( m_browse_map_btn->GetSize().GetWidth() * 2 , m_browse_map_btn->GetSize().GetHeight() ) ; // has 0 effect

    m_players = new BattleroomListCtrl( m_player_panel, (IBattle*)&battle, m_ui, false );
    m_chat = new ChatPanel( m_splitter, m_ui, battle );

    m_command_line = new wxStaticLine( this, -1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );

    m_leave_btn = new wxButton( this, BROOM_LEAVE, _("Leave"), wxDefaultPosition, wxSize(-1,CONTROL_HEIGHT) );
    m_leave_btn->SetToolTip(TE(_("Leave the battle and return to the battle list")));
    m_start_btn = new wxButton( this, BROOM_START, _("Start"), wxDefaultPosition, wxSize(-1,CONTROL_HEIGHT) );
    m_start_btn->SetToolTip(TE(_("Start the battle")));

    m_manage_players_btn = new wxButton( this, BROOM_MANAGE_MENU, _("Player Management"), wxDefaultPosition, wxSize(-1,CONTROL_HEIGHT) );
    m_manage_players_btn->SetToolTip(TE(_("Various functions to make team games simplers to setup")));

    m_addbot_btn = new wxButton( this, BROOM_ADDBOT, _("Add Bot..."), wxDefaultPosition, wxSize(-1,CONTROL_HEIGHT) );
    m_addbot_btn->SetToolTip(TE(_("Add a computer-controlled player to the game")));
    #if wxUSE_TOGGLEBTN
    m_autolock_chk = new wxToggleButton( this, BROOM_AUTOLOCK , _("Autolock on start"), wxDefaultPosition , wxSize( -1, CONTROL_HEIGHT), 0 );
    #else
    m_autolock_chk = new wxCheckBox( this, BROOM_AUTOLOCK, _("Autolock on start"), wxDefaultPosition, wxSize(-1,CONTROL_HEIGHT) );
    #endif
    m_autolock_chk->SetToolTip(TE(_("Automatically locks the battle when the game starts and unlock when it's finished.")));
    m_autolock_chk->SetValue( sett().GetLastAutolockStatus() );

    m_lock_chk = new wxCheckBox( this, BROOM_LOCK, _("Locked"), wxDefaultPosition, wxSize(-1,CONTROL_HEIGHT) );
    m_lock_chk->SetToolTip(TE(_("Prevent additional players from joining the battle")));

    m_manage_users_mnu = new wxMenu();

    m_autohost_mnu = new wxMenuItem( m_manage_users_mnu, BROOM_AUTOHOST, _( "Autohost" ), _("Toggle autohost mode.  This allows players to control your battle using commands like '!balance' and '!start'."), wxITEM_CHECK );
		m_manage_users_mnu->Append( m_autohost_mnu );
    m_autohost_mnu->Check( false );

    m_lock_balance_mnu = new wxMenuItem( m_manage_users_mnu, BROOM_LOCK_BALANCE, _( "Lock Balance" ), _("When activated, prevents anyone but the host to change team and ally"), wxITEM_CHECK );
    m_manage_users_mnu->Append( m_lock_balance_mnu );
    m_lock_balance_mnu->Check( false );

    wxMenuItem* m_balance_mnu = new wxMenuItem( m_manage_users_mnu, BROOM_BALANCE, _( "Balance alliances" ), _("Automatically balance players into two or more alliances") );
    m_manage_users_mnu->Append( m_balance_mnu );

    wxMenuItem* m_fix_colours_mnu = new wxMenuItem( m_manage_users_mnu, BROOM_FIXCOLOURS, _( "Fix colours" ), _("Make player colors unique") );
    m_manage_users_mnu->Append( m_fix_colours_mnu );

    wxMenuItem* m_fix_team_mnu = new wxMenuItem( m_manage_users_mnu, BROOM_FIXID, _( "Balance teams" ), _("Automatically balance players into control teams, by default none shares control") );
    m_manage_users_mnu->Append( m_fix_team_mnu );

	  wxStaticBoxSizer* m_preset_sizer;
		m_preset_sizer = new wxStaticBoxSizer( new wxStaticBox( this, 0, _("Manage Presets") ), wxVERTICAL );

		wxBoxSizer* m_preset_btns_sizer;
		m_preset_btns_sizer = new wxBoxSizer( wxHORIZONTAL );

    m_options_preset_sel = new wxComboBox( this, BROOM_PRESETSEL, sett().GetModDefaultPresetName( m_battle.GetHostModName() ), wxDefaultPosition, wxDefaultSize,  sett().GetPresetList(), wxCB_READONLY );
    m_options_preset_sel->SetToolTip(TE(_("Load battle preset")));

    m_preset_sizer->Add( m_options_preset_sel, 0, wxEXPAND | wxALL );

		m_save_btn = new wxButton( this, BROOM_SAVEPRES, _("Save"), wxDefaultPosition, wxDefaultSize );
		m_save_btn->SetToolTip( TE(_("Save a set of options.")) );

		m_preset_btns_sizer->Add( m_save_btn, 0, wxEXPAND );

		m_delete_btn = new wxButton( this, BROOM_DELETEPRES, _("Delete"), wxDefaultPosition, wxDefaultSize );
		m_delete_btn->SetToolTip( TE(_("Delete a set of options.")) );

		m_preset_btns_sizer->Add( m_delete_btn, 0, wxEXPAND );

		m_default_btn = new wxButton( this, BROOM_SETDEFAULTPRES, _("Set default"), wxDefaultPosition, wxDefaultSize );
		m_default_btn->SetToolTip( TE(_("Use the current set of options as mod's default.")) );

		m_preset_btns_sizer->Add( m_default_btn, 0, wxEXPAND );

		m_preset_sizer->Add( m_preset_btns_sizer, 0, wxEXPAND );


    m_opts_list = new wxListCtrl( this, BROOM_OPTIONLIST, wxDefaultPosition, wxDefaultSize, wxLC_NO_HEADER|wxLC_REPORT );
    //m_opts_list->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
    m_opts_list->SetFont( wxFont( 8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT ) );
    m_opts_list->SetToolTip( TE(_("Activate an element to quickly change it")) );
    wxListItem col;

    col.SetText( _("Option") );
    m_opts_list->InsertColumn( 0, col );
    col.SetText( _("Value") );
    m_opts_list->InsertColumn( 1, col );

    long pos = 0;

    m_opts_list->InsertItem( pos, _("Size") );
    m_opt_list_map[ _("Size") ] = pos++;
    m_opts_list->InsertItem( pos , _("Windspeed") );
    m_opt_list_map[ _("Windspeed") ] = pos++;
    m_opts_list->InsertItem( pos, _("Tidal strength") );
    m_opt_list_map[ _("Tidal strength") ] = pos++;

    m_opts_list->InsertItem( pos++, wxEmptyString );
    pos = AddMMOptionsToList( pos++, OptionsWrapper::EngineOption );
    m_opts_list->InsertItem( pos++, wxEmptyString );
    pos = AddMMOptionsToList( pos, OptionsWrapper::ModOption );
    m_opts_list->InsertItem( pos++, wxEmptyString );
    m_map_opts_index = pos;
    pos = AddMMOptionsToList( pos, OptionsWrapper::MapOption );


    // Create Sizers
    m_players_sizer = new wxBoxSizer( wxVERTICAL );
    m_player_sett_sizer = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer* m_map_select_sizer = new wxBoxSizer( wxHORIZONTAL );
    m_info_sizer = new wxBoxSizer( wxVERTICAL );
    m_top_sizer = new wxBoxSizer( wxHORIZONTAL );
    m_buttons_sizer = new wxBoxSizer( wxHORIZONTAL );
    //m_info1_sizer = new wxBoxSizer( wxHORIZONTAL );
    m_main_sizer = new wxBoxSizer( wxVERTICAL );

    int side_sel_width = m_side_sel->GetWidestItemWidth();
    wxBoxSizer* m_side_sel_sizer = new wxBoxSizer( wxHORIZONTAL );
    m_side_sel_sizer->SetMinSize( side_sel_width, CONTROL_HEIGHT );
    m_side_sel_sizer->Add( m_side_sel, 1, wxEXPAND );

    // Put widgets in place
    m_player_sett_sizer->Add( m_team_lbl, 0, wxEXPAND | wxALL, 2 );
    m_player_sett_sizer->Add( m_team_sel, 0, wxEXPAND | wxALL, 2 );
    m_player_sett_sizer->Add( m_ally_lbl, 0, wxEXPAND | wxALL, 2 );
    m_player_sett_sizer->Add( m_ally_sel, 0, wxEXPAND | wxALL, 2 );
    m_player_sett_sizer->Add( m_color_lbl, 0, wxEXPAND | wxALL, 2 );
    m_player_sett_sizer->Add( m_color_sel, 0, wxEXPAND | wxALL, 2 );
    m_player_sett_sizer->Add( m_side_lbl, 0, wxEXPAND | wxALL, 2 );
    m_player_sett_sizer->Add( m_side_sel_sizer, 0, wxEXPAND | wxALL, 2 );
    m_player_sett_sizer->Add( m_spec_chk, 0, wxEXPAND | wxALL, 2 );
    m_player_sett_sizer->Add( m_ready_chk, 0, wxEXPAND | wxALL, 2 );

    m_players_sizer->Add( m_players, 1, wxEXPAND );
    m_players_sizer->Add( m_player_sett_sizer, 0, wxEXPAND );

    m_player_panel->SetSizer( m_players_sizer );

    if ( sett().GetSplitBRoomHorizontally() )
        m_splitter->SplitHorizontally( m_player_panel, m_chat );
    else
        m_splitter->SplitVertically( m_player_panel, m_chat );

    //m_info1_sizer->Add( m_wind_lbl, 1, wxEXPAND );
    //m_info1_sizer->Add( m_size_lbl, 1, wxEXPAND );

    m_info_sizer->Add( m_minimap, 0, wxEXPAND );
    m_map_select_sizer->Add( m_map_combo, 0, wxALL | wxEXPAND | wxALIGN_CENTER_VERTICAL );
		m_map_select_sizer->Add( m_browse_map_btn, 0, wxALIGN_RIGHT );
    m_info_sizer->Add( m_map_select_sizer, 0, wxALL );
    //m_info_sizer->Add( m_info1_sizer, 0, wxEXPAND );
    //m_info_sizer->Add( m_tidal_lbl, 0, wxEXPAND );
    m_info_sizer->Add( m_opts_list, 1, wxEXPAND | wxTOP, 4 );
    m_info_sizer->Add( m_preset_sizer, 0, wxEXPAND, 4 );


    m_top_sizer->Add( m_splitter, 1, wxEXPAND | wxALL, 2 );
    m_top_sizer->Add( m_info_sizer, 0, wxEXPAND | wxALL, 2 );

    m_buttons_sizer->Add( m_leave_btn, 0, wxEXPAND | wxALL, 2 );
    m_buttons_sizer->AddStretchSpacer();
    m_buttons_sizer->Add( m_addbot_btn, 0, wxEXPAND | wxALL, 2 );
    m_buttons_sizer->Add( m_autolock_chk, 0, wxEXPAND | wxALL, 2 );
    m_buttons_sizer->Add( m_lock_chk, 0, wxEXPAND | wxALL, 2 );
    m_buttons_sizer->Add( m_manage_players_btn, 0, wxEXPAND | wxALL, 2 );
    m_buttons_sizer->Add( m_start_btn, 0, wxEXPAND | wxALL, 2 );

    m_main_sizer->Add( m_top_sizer, 1, wxEXPAND );
    m_main_sizer->Add( m_command_line, 0, wxEXPAND );
    m_main_sizer->Add( m_buttons_sizer, 0, wxEXPAND );

    m_splitter->SetMinimumPaneSize( 240 );

    for ( UserList::user_map_t::size_type i = 0; i < battle.GetNumUsers(); i++ )
    {
        m_players->AddUser( battle.GetUser( i ) );
    }

    if ( !IsHosted() )
    {
        m_options_preset_sel->Disable();
				m_save_btn->Disable();
				m_delete_btn->Disable();
				m_default_btn->Disable();
        m_start_btn->Disable();
        m_manage_players_btn->Disable();
        m_lock_chk->Disable();
        m_autolock_chk->Disable();
    }

    if ( IsHosted() && !m_battle.IsProxy() )
    {
        m_battle.SetImReady ( true );
        m_ready_chk->Disable();
    }

    if ( battle.IsProxy() )
    {
			m_battle.CustomBattleOptions().setSingleOption( _T("startpostype"), _T("3"), OptionsWrapper::EngineOption ); // set start position type to chose before game
			m_battle.SendHostInfo( wxString::Format(_T("%d_startpostype"), OptionsWrapper::EngineOption ) );
    }

		ReloadMaplist();

    UpdateBattleInfo( wxString::Format( _T("%d_mapname"), OptionsWrapper::PrivateOptions ) );
    UpdateBattleInfo();

    SetScrollRate( 3, 3 );
    SetSizer( m_main_sizer );
    Layout();
    unsigned int widthfraction = m_opts_list->GetClientSize().GetWidth() / 3;
    m_opts_list->SetColumnWidth( 0, widthfraction * 1.95 );
    m_opts_list->SetColumnWidth( 1, widthfraction * 0.95 );

}


BattleRoomTab::~BattleRoomTab()
{
    if (GetAui().manager)GetAui().manager->DetachPane( this );
}


bool BattleRoomTab::IsHosted()
{
    return m_battle.IsFounderMe();
}

wxString _GetStartPosStr( IBattle::StartType t )
{
    switch ( t )
    {
    case IBattle::ST_Fixed:
        return _("Fixed");
    case IBattle::ST_Random:
        return _("Random");
    case IBattle::ST_Choose:
        return _("Boxes");
    case IBattle::ST_Pick:
        return _("Pick");
    default:
        return _T("?");
    };
}


wxString _GetGameTypeStr( IBattle::GameType t )
{
    switch ( t )
    {
    case IBattle::GT_ComContinue:
        return _("Continue");
    case IBattle::GT_ComEnds:
        return _("End");
    case IBattle::GT_Lineage:
        return _("Lineage");
    default:
        return _T("?");
    };
}


void BattleRoomTab::UpdateBattleInfo()
{
    m_lock_chk->SetValue( m_battle.IsLocked() );
    m_minimap->UpdateMinimap();
}


void BattleRoomTab::UpdateBattleInfo( const wxString& Tag )
{
    long index = m_opt_list_map[ Tag ];
    OptionsWrapper::GameOption type = (OptionsWrapper::GameOption)s2l(Tag.BeforeFirst( '_' ));
    wxString key = Tag.AfterFirst( '_' );
    wxString value;
    if ( ( type == OptionsWrapper::MapOption ) || ( type == OptionsWrapper::ModOption ) || ( type == OptionsWrapper::EngineOption ) )
    {
        OptionType DataType = m_battle.CustomBattleOptions().GetSingleOptionType( key );
        value = m_battle.CustomBattleOptions().getSingleValue( key, (OptionsWrapper::GameOption)type );
        if ( m_battle.CustomBattleOptions().getDefaultValue( key, type) == value ) m_opts_list->SetItemFont( index, wxFont( 8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT ) );
        else m_opts_list->SetItemFont( index, wxFont( 8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD ) );
        if ( DataType == opt_bool )
        {
            value =  bool2yn( s2l( value ) ); // convert from 0/1 to literal Yes/No
        }
        else if ( DataType == opt_list )
        {
        	 value = m_battle.CustomBattleOptions().GetNameListOptValue( key, type ); // get the key full name not short key
        }
				m_opts_list->SetItem( index, 1, value );
    }
    else if ( type == OptionsWrapper::PrivateOptions )
    {
        if ( key == _T("mapname") ) // the map has been changed
        {
            try   // updates map info summary
            {
                ASSERT_EXCEPTION( m_battle.MapExists(), _("Map does not exist.") );
                UnitSyncMap map = m_battle.LoadMap();
                m_opts_list->SetItem( m_opt_list_map[ _("Size") ] , 1, wxString::Format( _T("%.0fx%.0f"), map.info.width/512.0, map.info.height/512.0 ) );
                m_opts_list->SetItem( m_opt_list_map[ _("Windspeed") ], 1, wxString::Format( _T("%d-%d"), map.info.minWind, map.info.maxWind) );
                m_opts_list->SetItem( m_opt_list_map[ _("Tidal strength") ], 1, wxString::Format( _T("%d"), map.info.tidalStrength) );
                //    m_opts_list->SetItem( 0, 1,  );
            }
            catch (...)
            {
                m_opts_list->SetItem( m_opt_list_map[ _("Size") ], 1, _T("?x?") );
                m_opts_list->SetItem( m_opt_list_map[ _("Windspeed") ], 1, _T("?-?") );
                m_opts_list->SetItem( m_opt_list_map[ _("Tidal strength") ], 1, _T("?") );
            }
            wxString mapname = RefineMapname( m_battle.GetHostMapName() );
						int index = m_map_combo->FindString( mapname );
						if ( index != wxNOT_FOUND ) m_map_combo->SetSelection( index );
						else m_map_combo->SetValue( mapname );

            //delete any eventual map option from the list and add options of the new map
            for ( long i = m_map_opts_index; i < m_opts_list->GetItemCount(); i++ ) m_opts_list->DeleteItem( i );
            AddMMOptionsToList( m_map_opts_index, OptionsWrapper::MapOption );

            m_minimap->UpdateMinimap();

        }
        else if ( key == _T("restrictions") )
        {
            m_opts_list->SetItem( index, 1, bool2yn( m_battle.RestrictedUnits().size() > 0 ) );
        }
    }
}

BattleroomListCtrl& BattleRoomTab::GetPlayersListCtrl()
{
    ASSERT_LOGIC( m_players != 0, _T("m_players = 0") );
    return *m_players;
}


void BattleRoomTab::UpdateUser( User& user )
{
    m_players->UpdateUser( user );

    m_minimap->UpdateMinimap();

    if ( &user != &m_battle.GetMe() ) return;

    UserBattleStatus& bs = user.BattleStatus();
    m_team_sel->SetSelection( bs.team );
    m_ally_sel->SetSelection( bs.ally );
    m_side_sel->SetSelection( bs.side );
    m_spec_chk->SetValue( bs.spectator );

    // Enable or disable widgets' sensitivity as appropriate.
    if ( bs.spectator )
    {
        m_ready_chk->SetValue ( true );
        m_ready_chk->Disable();
        m_side_sel->Disable();
        m_ally_sel->Disable();
        m_team_sel->Disable();
    }
    else
    {
        if ( !IsHosted() || m_battle.IsProxy() )
        {
            m_ready_chk->Enable();
        }

        m_ready_chk->SetValue( bs.ready );
        m_side_sel->Enable();
        m_ally_sel->Enable();
        m_team_sel->Enable();
    }

    icons().SetColourIcon( bs.team, user.BattleStatus().colour );
    m_color_sel->SetColor( user.BattleStatus().colour );
}


Battle& BattleRoomTab::GetBattle()
{
    return m_battle;
}


ChatPanel& BattleRoomTab::GetChatPanel()
{
    wxLogDebugFunc(_T(""));
    ASSERT_LOGIC( m_chat != 0, _T("m_chat = 0") );
    return *m_chat;
}


void BattleRoomTab::OnStart( wxCommandEvent& event )
{
    m_battle.GetMe().BattleStatus().ready = true;

    if ( !m_battle.IsEveryoneReady() )
    {
        int answer = customMessageBox( SL_MAIN_ICON, _("Some players are not ready yet.\nRing these players?"), _("Not ready"), wxYES_NO );
        if ( answer == wxYES )
        {
            m_battle.RingNotReadyPlayers();
            return;
        }
        answer = customMessageBox( SL_MAIN_ICON, _("Force start?"), _("Not ready"), wxYES_NO );
        if ( answer == wxNO ) return;
    }
    m_ui.StartHostedBattle();
}


void BattleRoomTab::OnLeave( wxCommandEvent& event )
{
    m_battle.Leave();
}



void BattleRoomTab::OnBalance( wxCommandEvent& event )
{
    AutoBalanceDialog::BalanceOptions defaultval;
    defaultval.type = (IBattle::BalanceType)sett().GetBalanceMethod();
    defaultval.respectclans = sett().GetBalanceClans();
    defaultval.strongclans = sett().GetBalanceStrongClans();
    defaultval.groupingsize = sett().GetBalanceGrouping();
    AutoBalanceDialog dlg( this, defaultval );
    if ( dlg.ShowModal() == wxID_OK )
    {
        AutoBalanceDialog::BalanceOptions balance = dlg.GetResult();
        sett().SetBalanceMethod( balance.type );
        sett().SetBalanceClans( balance.respectclans );
        sett().SetBalanceStrongClans( balance.strongclans );
        sett().SetBalanceGrouping( balance.groupingsize );
        m_battle.Autobalance( balance.type, balance.respectclans, balance.strongclans, balance.groupingsize );
    }
}


void BattleRoomTab::OnFixTeams( wxCommandEvent& event )
{
    AutoBalanceDialog::BalanceOptions defaultval;
    defaultval.type = (IBattle::BalanceType)sett().GetFixIDMethod();
    defaultval.respectclans = sett().GetFixIDClans();
    defaultval.strongclans = sett().GetFixIDStrongClans();
    defaultval.groupingsize = sett().GetFixIDGrouping();
    AutoBalanceDialog dlg( this, defaultval );
    if ( dlg.ShowModal() == wxID_OK )
    {
        AutoBalanceDialog::BalanceOptions balance = dlg.GetResult();
        sett().SetFixIDMethod( balance.type );
        sett().SetFixIDClans( balance.respectclans );
        sett().SetFixIDStrongClans( balance.strongclans );
        sett().SetFixIDGrouping( balance.groupingsize );
        m_battle.FixTeamIDs( balance.type, balance.respectclans, balance.strongclans, balance.groupingsize );
    }
}


void BattleRoomTab::OnFixColours( wxCommandEvent& event )
{
    wxLogMessage(_T(""));
    if (!IsHosted()) // Works with autohosts, and human hosts knows what it mean.
    {
        m_battle.Say(_T("!fixcolors"));
        return;
    }
    //m_battle.Say(_T("fixing colours"));
    m_battle.FixColours();
}


void BattleRoomTab::OnAddBot( wxCommandEvent& event )
{
    //customMessageBox(SL_MAIN_ICON,_T("Max players reached"),_T("Cannot add bot, maximum number of players already reached.") );
    AddBotDialog dlg( this, m_battle );
    if ( dlg.ShowModal() == wxID_OK )
    {
        UserBattleStatus bs;
        bs.team = m_battle.GetFreeTeamNum( false );
        bs.ally = bs.team;
        bs.sync = SYNC_SYNCED;
        bs.spectator = false;
        bs.side = 0;
        bs.ready = true;
        bs.handicap = 0;
        bs.colour = m_battle.GetNewColour();
        bs.aishortname = dlg.GetAIShortName();
        bs.aiversion = dlg.GetAIVersion();
        bs.aitype = dlg.GetAIType();
        bs.owner = m_battle.GetMe().GetNick();
        m_ui.GetServer().AddBot( m_battle.GetBattleId(), dlg.GetNick(), bs );
    }
}


void BattleRoomTab::OnImReady( wxCommandEvent& event )
{
    m_battle.SetImReady( m_ready_chk->GetValue() );
}


void BattleRoomTab::OnLock( wxCommandEvent& event )
{
    m_battle.SetIsLocked( m_lock_chk->GetValue() );
    m_battle.SendHostInfo( IBattle::HI_Locked );
}


void BattleRoomTab::OnAutoHost( wxCommandEvent& event )
{
    m_battle.GetAutoHost().SetEnabled( m_autohost_mnu->IsChecked() );
}


void BattleRoomTab::OnImSpec( wxCommandEvent& event )
{
    m_battle.ForceSpectator( m_battle.GetMe(), m_spec_chk->GetValue() );
}


void BattleRoomTab::OnTeamSel( wxCommandEvent& event )
{
		unsigned long team;
    m_team_sel->GetValue().ToULong( &team );
		m_battle.ForceTeam( m_battle.GetMe(), team -1  );
}


void BattleRoomTab::OnAllySel( wxCommandEvent& event )
{
		unsigned long ally;
    m_ally_sel->GetValue().ToULong( &ally );
		m_battle.ForceAlly( m_battle.GetMe(), ally -1  );
}


void BattleRoomTab::OnColourSel( wxCommandEvent& event )
{
    User& u = m_battle.GetMe();
    wxColour CurrentColour = u.BattleStatus().colour;
    CurrentColour = GetColourFromUser(this, CurrentColour);
    if ( !CurrentColour.IsColourOk() ) return;
    sett().SetBattleLastColour( CurrentColour );
    m_battle.ForceColour( u, CurrentColour );
}


void BattleRoomTab::OnSideSel( wxCommandEvent& event )
{
    m_battle.ForceSide( m_battle.GetMe(), m_side_sel->GetSelection() );
}


void BattleRoomTab::OnPresetSel( wxCommandEvent& event )
{
    wxString presetname = m_options_preset_sel->GetValue();
    if ( presetname.IsEmpty() ) return;
    m_battle.LoadOptionsPreset( presetname );
    m_battle.SendHostInfo( IBattle::HI_Send_All_opts );
}

void BattleRoomTab::OnAutoLock( wxCommandEvent& event )
{
    m_battle.SetAutoLockOnStart( m_autolock_chk->GetValue() );
    sett().SetLastAutolockStatus( m_autolock_chk->GetValue() );
}


void BattleRoomTab::OnLockBalance( wxCommandEvent& event )
{
  bool locked = m_lock_balance_mnu->IsChecked();
  m_battle.SetLockExternalBalanceChanges( locked );
}

void BattleRoomTab::OnShowManagePlayersMenu( wxCommandEvent& event )
{
  PopupMenu( m_manage_users_mnu );
}

void BattleRoomTab::OnUserJoined( User& user )
{
    if( !user.BattleStatus().IsBot() ) m_chat->Joined( user );
    m_players->AddUser( user );
    if ( &user == &m_battle.GetMe() )
    {
        m_players->SetSelectedIndex ( m_players->GetIndexFromData( &user ) );
    }
}


void BattleRoomTab::OnUserLeft( User& user )
{
    if( !user.BattleStatus().IsBot() ) m_chat->Parted( user, wxEmptyString );
    m_players->RemoveUser( user );
}


void BattleRoomTab::OnUnitSyncReloaded()
{
    m_minimap->UpdateMinimap();
    ReloadMaplist();
    UpdateBattleInfo();
    m_battle.SendMyBattleStatus(); // This should reset sync status.
}

long BattleRoomTab::AddMMOptionsToList( long pos, OptionsWrapper::GameOption optFlag )
{
    OptionsWrapper::wxStringTripleVec optlist = m_battle.CustomBattleOptions().getOptions( optFlag );
    for (OptionsWrapper::wxStringTripleVec::iterator it = optlist.begin(); it != optlist.end(); ++it)
    {
        m_opts_list->InsertItem( pos, it->second.first );
        wxString tag = wxString::Format(_T("%d_"), optFlag ) + it->first;
        m_opt_list_map[ tag ] = pos;
        UpdateBattleInfo( tag );
        pos++;
    }
    return pos;
}

void BattleRoomTab::UpdateHighlights()
{
    m_players->RefreshVisibleItems();
}


void BattleRoomTab::UpdatePresetList()
{
    m_options_preset_sel->Clear();
    m_options_preset_sel->Append(sett().GetPresetList());
    m_options_preset_sel->SetStringSelection(  m_battle.GetCurrentPreset() );
}


void BattleRoomTab::OnSavePreset( wxCommandEvent& event )
{
  wxString presetname;
	if ( ui().AskText( _("Enter preset name"), _("Enter a name to save the current set of options\nIf a preset with the same name already exist, it will be overwritten"), presetname ) ) return;
  if ( presetname.IsEmpty() )
  {
     customMessageBoxNoModal( SL_MAIN_ICON , _("Cannot save an options set without a name."), _("error"), wxICON_EXCLAMATION|wxOK );
     return;
  }
  m_battle.SaveOptionsPreset( presetname );
}


void BattleRoomTab::OnDeletePreset( wxCommandEvent& event )
{
  wxArrayString choices = m_battle.GetPresetList();
	int result = wxGetSingleChoiceIndex(_("Pick an existing option set from the list"),_("Set delete preset"), choices );
	if ( result < 0 ) return;
  m_battle.DeletePreset( choices[result] );
}

void BattleRoomTab::OnSetModDefaultPreset( wxCommandEvent& event )
{
  wxArrayString choices = m_battle.GetPresetList();
	int result = wxGetSingleChoiceIndex(_("Pick an existing option set from the list"),_("Set mod default preset"), choices );
	if ( result < 0 ) return;
  sett().SetModDefaultPresetName( m_battle.GetHostModName(), choices[result] );
}


void BattleRoomTab::OnMapBrowse( wxCommandEvent& event )
{
	wxLogDebugFunc( _T("") );
	MapSelectDialog dlg( (wxWindow*)&m_ui.mw(), m_ui );

	if ( dlg.ShowModal() == wxID_OK && dlg.GetSelectedMap() != NULL )
	{
		wxString mapname = dlg.GetSelectedMap()->name;
		wxLogDebugFunc( mapname );
		if ( !m_battle.IsFounderMe() )
		{
			m_battle.DoAction( _T("suggests ") + mapname );
			return;
		}
		const int idx = m_map_combo->FindString( RefineMapname( mapname ), true /*case sensitive*/ );
		if ( idx != wxNOT_FOUND ) SetMap( idx );

	}
}

void BattleRoomTab::ReloadMaplist()
{
  m_map_combo->Clear();

  wxArrayString maplist= usync().GetMapList();
 // maplist.Sort(CompareStringIgnoreCase);

  size_t nummaps = maplist.Count();
  for ( size_t i = 0; i < nummaps; i++ ) m_map_combo->Insert( RefineMapname(maplist[i]), i );
}

void BattleRoomTab::SetMap( int index )
{
  try
  {
    UnitSyncMap map = usync().GetMapEx( index );
    m_battle.SetLocalMap( map );

    m_battle.SendHostInfo( IBattle::HI_Map );
    for( unsigned int i=0;i<m_battle.GetNumRects();++i) if ( m_battle.GetStartRect( i ).exist ) m_battle.RemoveStartRect(i);
    m_battle.SendHostInfo( IBattle::HI_StartRects );
  } catch (...) {}
}

void BattleRoomTab::OnMapSelect( wxCommandEvent& event )
{
	if ( !m_battle.IsFounderMe() )
	{
		try
		{
			m_battle.DoAction( _T("suggests ") + usync().GetMap( m_map_combo->GetCurrentSelection() ).name );
		}
		catch(...)
		{
		}
		return;
	}
	SetMap( m_map_combo->GetCurrentSelection() );
}

void BattleRoomTab::OnOptionActivate( wxListEvent& event )
{
	if ( !m_battle.IsFounderMe() ) return;
	long index = event.GetIndex();
	if ( index == 0 ) return;
	wxString tag;
	for ( OptionListMap::iterator itor = m_opt_list_map.begin(); itor != m_opt_list_map.end(); itor++ )
	{
		if ( itor->second == index )
		{
			 tag = itor->first;
			 break;
		}
	}
	OptionsWrapper& optWrap = m_battle.CustomBattleOptions();
	OptionsWrapper::GameOption optFlag = (OptionsWrapper::GameOption)s2l(tag.BeforeFirst( '_' ));
	wxString key = tag.AfterFirst( '_' );
	OptionType type = optWrap.GetSingleOptionType( key );
	if ( !optWrap.keyExists( key, optFlag, false, type ) ) return;
	SingleOptionDialog dlg( m_battle, tag );
	dlg.ShowModal();
}

void BattleRoomTab::SortPlayerList()
{
    m_players->SortList();
}
