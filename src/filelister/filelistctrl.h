#ifndef SPRINGLOBBY_HEADERGUARD_FILELISTCTRL_H
#define SPRINGLOBBY_HEADERGUARD_FILELISTCTRL_H

#include "filelistfilter.h"

#include "../customlistctrl.h"

class wxMenu;
class wxListEvent;
class wxCommandEvent;


class FileListCtrl : public customListCtrl
{
  public:
    FileListCtrl( wxWindow* parent );
    ~FileListCtrl();

    void Sort();

    void OnListRightClick( wxListEvent& event );
    void OnMouseMotion(wxMouseEvent& event);
    void OnColClick( wxListEvent& event );
  protected:
    static int wxCALLBACK CompareStatusUP(long item1, long item2, long sortData);
    static int wxCALLBACK CompareStatusDOWN(long item1, long item2, long sortData);
    static int wxCALLBACK CompareCountryUP(long item1, long item2, long sortData);
    static int wxCALLBACK CompareCountryDOWN(long item1, long item2, long sortData);
    static int wxCALLBACK CompareRankUP(long item1, long item2, long sortData);
    static int wxCALLBACK CompareRankDOWN(long item1, long item2, long sortData);

    struct {
      int col;
      bool direction;
    } m_sortorder[3];

    wxMenu* m_popup;
//    Ui& m_ui;
//    static Ui* m_ui_for_sort;


    DECLARE_EVENT_TABLE()
};

enum
{
    BLIST_LIST = wxID_HIGHEST,
    BLIST_DLMOD,
    BLIST_DLMAP
};

#endif // SPRINGLOBBY_HEADERGUARD_BATTLELISTCTRL_H