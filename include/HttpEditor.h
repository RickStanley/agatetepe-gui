#ifndef HTTEDITOR_H
#define HTTEDITOR_H

#include <wx/event.h>
#include <wx/stc/stc.h>

enum { ID_CUSTOM_MENU_ITEM = wxID_HIGHEST + 1 };

class HttpEditor : public wxStyledTextCtrl {
public:
  HttpEditor(wxWindow *parent, wxWindowID id = wxID_ANY);

      void SetRunRequestMarker(int lineNumber, bool show);


private:
      enum {
        MARKER_RUN_REQUEST = 0
    };

  // --- Style Definitions ---
  // We use unique numbers for each style element.
  enum {
    STYLE_DEFAULT = wxSTC_STYLE_DEFAULT,
    STYLE_COMMENT = 1,
    STYLE_VARIABLE_SYMBOL = 2,
    STYLE_VARIABLE_NAME = 3,
    STYLE_OPERATOR = 4,
    STYLE_VARIABLE_VALUE = 5,
    STYLE_METHOD = 6,
    STYLE_URL = 7,
    STYLE_HEADER_KEY = 8,
    STYLE_HEADER_VALUE = 9,
    STYLE_JSON_KEY = 10,
    STYLE_JSON_STRING = 11,
    STYLE_JSON_KEYWORD = 12,
    STYLE_JSON_NUMBER = 13
  };

  // --- Event Handlers ---
  void OnStyleNeeded(wxStyledTextEvent &event);
  void OnContextMenu(wxContextMenuEvent &event);
  void OnCustomItemSelected(wxCommandEvent &event);
  void OnStandardItemSelected(wxCommandEvent &event);
  void OnMarginClick(wxStyledTextEvent &event);

  // --- Helper Functions ---
  void StyleLine(int lineNumber);


  wxDECLARE_EVENT_TABLE();
};

#endif // HTTEDITOR_H
