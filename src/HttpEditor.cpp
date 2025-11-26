#include "HttpEditor.h"
#include <wx/dcmemory.h>
#include <wx/graphics.h>
#include <wx/log.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/tokenzr.h>

wxBEGIN_EVENT_TABLE(HttpEditor, wxStyledTextCtrl)
    EVT_STC_STYLENEEDED(wxID_ANY, HttpEditor::OnStyleNeeded)
        EVT_CONTEXT_MENU(HttpEditor::OnContextMenu)
            EVT_MENU(ID_CUSTOM_MENU_ITEM, HttpEditor::OnCustomItemSelected)

    // Map all standard menu IDs to our single handler
    EVT_MENU(wxID_UNDO, HttpEditor::OnStandardItemSelected)
        EVT_MENU(wxID_REDO, HttpEditor::OnStandardItemSelected)
            EVT_MENU(wxID_CUT, HttpEditor::OnStandardItemSelected)
                EVT_MENU(wxID_COPY, HttpEditor::OnStandardItemSelected)
                    EVT_MENU(wxID_PASTE, HttpEditor::OnStandardItemSelected)
                        EVT_MENU(wxID_CLEAR, HttpEditor::OnStandardItemSelected)
                            EVT_MENU(wxID_SELECTALL,
                                     HttpEditor::OnStandardItemSelected)

                                wxEND_EVENT_TABLE()

                                    HttpEditor::HttpEditor(wxWindow *parent,
                                                           wxWindowID id)
    : wxStyledTextCtrl(parent, id) {
  // 1. Set the lexer to CONTAINER. This is crucial!
  SetLexer(wxSTC_LEX_CONTAINER);

  // 2. Define the visual appearance for each style.
  wxFont font(wxFontInfo(11).Family(wxFONTFAMILY_TELETYPE));
  StyleSetFont(STYLE_DEFAULT, font);
  StyleSetForeground(STYLE_DEFAULT, wxColour(200, 200, 200));
  StyleSetBackground(STYLE_DEFAULT, wxColour(30, 30, 30));
  StyleClearAll(); // Apply default style to all others

  // Comments
  StyleSetForeground(STYLE_COMMENT, wxColour(128, 128, 128));
  StyleSetFont(STYLE_COMMENT, font.Italic());

  // Variables (@name = value)
  StyleSetForeground(STYLE_VARIABLE_SYMBOL, wxColour(255, 128, 0)); // Orange
  StyleSetForeground(STYLE_VARIABLE_NAME, wxColour(0, 200, 255));   // Cyan
  StyleSetForeground(STYLE_OPERATOR, wxColour(255, 255, 255));      // White
  StyleSetForeground(STYLE_VARIABLE_VALUE,
                     wxColour(152, 251, 152)); // Light Green

  // Request Line (METHOD URL)
  StyleSetForeground(STYLE_METHOD, wxColour(255, 128, 128)); // Light Red
  StyleSetBold(STYLE_METHOD, true);
  StyleSetForeground(STYLE_URL, wxColour(255, 255, 128)); // Yellow

  // Headers (Key: Value)
  StyleSetForeground(STYLE_HEADER_KEY, wxColour(128, 255, 128)); // Light Green
  StyleSetForeground(STYLE_HEADER_VALUE, wxColour(200, 200, 200)); // Light Gray

  // JSON Body
  StyleSetForeground(STYLE_JSON_KEY, wxColour(255, 178, 102)); // Orange
  StyleSetForeground(STYLE_JSON_STRING, wxColour(206, 145, 120));
  StyleSetForeground(STYLE_JSON_KEYWORD, wxColour(255, 128, 255)); // Magenta
  StyleSetForeground(STYLE_JSON_NUMBER, wxColour(128, 208, 255));  // Light Blue

  SetMarginType(0, wxSTC_MARGIN_NUMBER);
  SetMarginWidth(0, 40);

  SetMarginType(1, wxSTC_MARGIN_SYMBOL);
  SetMarginWidth(1, 20);

  // We don't need SetMarginSensitive if we aren't handling clicks
  SetMarginSensitive(1, false);
  SetMarginMask(1, (1 << MARKER_RUN_REQUEST));

  MarkerDefine(MARKER_RUN_REQUEST, wxSTC_MARK_ARROWS);
  MarkerSetForeground(MARKER_RUN_REQUEST,
                      wxColour(255, 255, 0)); // Yellow color
  MarkerSetBackground(MARKER_RUN_REQUEST,
                      wxColour(0, 0, 255)); // Blue background

  // 3. Set some general editor properties
  SetTabWidth(4);
  SetUseTabs(false);
  SetCaretForeground(wxColour(255, 255, 255));
}

void HttpEditor::OnStyleNeeded(wxStyledTextEvent &event) {
  // The position up to which styling is needed.
  int endPosNeeded = event.GetPosition();

  // The position from which we can start styling.
  // Scintilla keeps track of this. This is the key to fixing the bug.
  int startPos = GetEndStyled();

  // Convert positions to line numbers.
  int lineStart = LineFromPosition(startPos);
  int lineEnd = LineFromPosition(endPosNeeded);

  // Style each line in the required range.
  for (int currentLine = lineStart; currentLine <= lineEnd; ++currentLine) {
    StyleLine(currentLine);
  }
}

void HttpEditor::StyleLine(int lineNumber) {
  int startPos = PositionFromLine(lineNumber);
  int endPos = GetLineEndPosition(lineNumber);
  wxString line = GetLine(lineNumber);
  line.Trim(false); // Trim leading whitespace for easier checks

  StartStyling(startPos);

  if (line.StartsWith("#") || line.StartsWith("###")) {

    SetStyling(endPos - startPos, STYLE_COMMENT);

  } else if (line.Contains("=") && line.StartsWith("@")) {

    int nameEnd = line.Find('=');
    if (nameEnd != wxNOT_FOUND) {

      SetStyling(1, STYLE_VARIABLE_SYMBOL);         // '@'
      SetStyling(nameEnd - 1, STYLE_VARIABLE_NAME); // 'name'
      SetStyling(1, STYLE_OPERATOR);                // '='
      SetStyling(endPos - startPos - nameEnd - 1,
                 STYLE_VARIABLE_VALUE); // 'value'

    } else {
      SetStyling(endPos - startPos, STYLE_DEFAULT);
    }

  } else if (line.StartsWith("GET") || line.StartsWith("POST") ||
             line.StartsWith("PUT") || line.StartsWith("DELETE") ||
             line.StartsWith("PATCH")) {
    wxStringTokenizer tkz(line, " \t\n\r");
    if (tkz.HasMoreTokens()) {
      wxString method = tkz.GetNextToken();
      SetStyling(method.length(), STYLE_METHOD);                      // 'GET'
      SetStyling(1, STYLE_OPERATOR);                                  // ' '
      SetStyling(endPos - startPos - method.length() - 1, STYLE_URL); // 'URL'
    }
  } else if (line.Contains(":") && !line.StartsWith("{") &&
             !line.Trim(false).starts_with('"')) {
    int colonPos = line.Find(':');
    if (colonPos != wxNOT_FOUND) {
      SetStyling(colonPos, STYLE_HEADER_KEY); // 'key'
      SetStyling(1, STYLE_OPERATOR);          // ':'
      SetStyling(endPos - startPos - colonPos - 1,
                 STYLE_HEADER_VALUE); // ' value'
    } else {
      SetStyling(endPos - startPos, STYLE_DEFAULT);
    }
  } else {
    // Default to JSON-like styling for bodies
    wxString text = GetLine(lineNumber);
    int pos = 0;
    while (pos < text.length()) {
      wxChar ch = text[pos];
      if (ch == '{' || ch == '}' || ch == '[' || ch == ']') {
        SetStyling(1, STYLE_OPERATOR);
        pos++;
      } else if (ch == '"') {
        int endQuotePos = text.find('"', pos + 1);
        if (endQuotePos != wxString::npos) {
          int styleToApply = STYLE_JSON_STRING; // Default to value string

          // Look ahead for a colon to see if it's a key
          int colonPos = text.find(':', endQuotePos);
          // A key is a string followed by a colon, possibly with whitespace.
          bool isKey =
              (colonPos != wxString::npos) &&
              (text.find_first_not_of(" \t", endQuotePos + 1) == colonPos);

          if (isKey) {
            styleToApply = STYLE_JSON_KEY;
          }

          SetStyling(endQuotePos - pos + 1, styleToApply);
          pos = endQuotePos + 1;
        } else {
          SetStyling(1, STYLE_DEFAULT);
          pos++;
        }
      } else if (ch == ':' || ch == ',') {
        SetStyling(1, STYLE_OPERATOR);
        pos++;
      } else if (iswdigit(ch) || (ch == '-' && pos + 1 < text.length() &&
                                  iswdigit(text[pos + 1]))) {
        int numStart = pos;
        while (pos < text.length() &&
               (iswdigit(text[pos]) || text[pos] == '.')) {
          pos++;
        }
        SetStyling(pos - numStart, STYLE_JSON_NUMBER);
      } else if (iswalpha(ch)) {
        int wordStart = pos;
        while (pos < text.length() && iswalpha(text[pos])) {
          pos++;
        }
        wxString word = text.SubString(wordStart, pos - 1);
        if (word == "true" || word == "false" || word == "null") {
          SetStyling(pos - wordStart, STYLE_JSON_KEYWORD);
        } else {
          SetStyling(pos - wordStart, STYLE_DEFAULT);
        }
      } else {
        // Whitespace or other characters
        SetStyling(1, STYLE_DEFAULT);
        pos++;
      }
    }
  }
}

void HttpEditor::OnContextMenu(wxContextMenuEvent &event) {
  wxMenu menu;

  // --- Recreate Standard Items ---
  menu.Append(wxID_UNDO, _("&Undo"));
  menu.Append(wxID_REDO, _("&Redo"));
  menu.AppendSeparator();
  menu.Append(wxID_CUT, _("Cu&t"));
  menu.Append(wxID_COPY, _("&Copy"));
  menu.Append(wxID_PASTE, _("&Paste"));
  menu.Append(wxID_CLEAR, _("&Delete"));
  menu.AppendSeparator();
  menu.Append(wxID_SELECTALL, _("Select &All"));

  // --- Replicate Default Behavior (Enable/Disable Logic) ---
  // This is the crucial part that makes the menu behave like the default.
  menu.Enable(wxID_UNDO, CanUndo());
  menu.Enable(wxID_REDO, CanRedo());
  menu.Enable(wxID_CUT, CanCut());
  menu.Enable(wxID_COPY, CanCopy());
  menu.Enable(wxID_PASTE, CanPaste());
  menu.Enable(wxID_CLEAR,
              !IsEmpty()); // "Delete" is only useful if there's text
  menu.Enable(wxID_SELECTALL,
              !IsEmpty()); // "Select All" is only useful if there's text

  // --- Add Your Custom Items ---
  menu.AppendSeparator();
  menu.Append(ID_CUSTOM_MENU_ITEM, _("&My Custom Action"));

  // --- Show the Menu ---
  // Use the position from the event to show the menu at the correct location.
  PopupMenu(&menu);
}

// This is the new, essential function
void HttpEditor::OnStandardItemSelected(wxCommandEvent &event) {
  // Call the appropriate wxTextCtrl method based on the event's ID
  switch (event.GetId()) {
  case wxID_UNDO:
    Undo();
    break;

  case wxID_REDO:
    Redo();
    break;

  case wxID_CUT:
    Cut();
    break;

  case wxID_COPY:
    Copy();
    break;

  case wxID_PASTE:
    Paste();
    break;

  case wxID_CLEAR:
    // The "Delete" action corresponds to the Delete() method
    Clear();
    break;

  case wxID_SELECTALL:
    SelectAll();
    break;

  default:
    // If we don't recognize the ID, skip it to allow other handlers to process
    // it.
    event.Skip();
    break;
  }
}

void HttpEditor::OnCustomItemSelected(wxCommandEvent &event) {
  wxMessageBox(_("Custom action triggered!"), _("Info"),
               wxOK | wxICON_INFORMATION, this);
}

void HttpEditor::SetRunRequestMarker(int lineNumber, bool show) {
  if (lineNumber < 0 || lineNumber >= GetLineCount())
    return;

  if (show) {
    MarkerAdd(lineNumber, MARKER_RUN_REQUEST);
  } else {
    MarkerDelete(lineNumber, MARKER_RUN_REQUEST);
  }
}
