#include "HttpEditor.h"
#include <wx/event.h>
#include <wx/splitter.h>
#include <wx/stc/stc.h> // Include the STC header
#include <wx/wx.h>

enum {
  ID_TOGGLE_PANEL = wxID_HIGHEST + 1,
      ID_MARK_REQUESTS,

    ID_RUN_REQUEST // New ID


};

class MyApp : public wxApp {
public:
  virtual bool OnInit();
};

class MyFrame : public wxFrame {
public:
  MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size);
  wxStyledTextCtrl *m_textCtrl;

private:
  void OnToggleResponsePanel(wxCommandEvent &event);
      void OnMarkRequests(wxCommandEvent& event);

      void OnRunRequest(wxCommandEvent& event); // New, simple handler


  wxSplitterWindow *m_splitter;
  HttpEditor *m_editor;
  wxTextCtrl *m_responsePanel; // New member for the right side

  wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(ID_TOGGLE_PANEL, MyFrame::OnToggleResponsePanel)
    EVT_MENU(ID_MARK_REQUESTS, MyFrame::OnMarkRequests)
    EVT_MENU(ID_RUN_REQUEST, MyFrame::OnRunRequest) // Bind the new menu event

        wxEND_EVENT_TABLE()

            wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
  MyFrame *frame =
      new MyFrame("wxSTC Editor", wxPoint(50, 50), wxSize(800, 600));
  frame->Show(true);
  return true;
}

MyFrame::MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
    : wxFrame(NULL, wxID_ANY, title, pos, size) {
  // --- 1. Create the Menu Bar ---
  wxMenuBar *menuBar = new wxMenuBar();
  wxMenu *viewMenu = new wxMenu();
  viewMenu->AppendCheckItem(ID_TOGGLE_PANEL, "Toggle Response Panel\tCtrl+R",
                            "Show or hide the response panel.");
  viewMenu->Check(ID_TOGGLE_PANEL,
                  true); // Check it initially since the panel is visible
  menuBar->Append(viewMenu, "&View");

      wxMenu* toolsMenu = new wxMenu();
    toolsMenu->Append(ID_MARK_REQUESTS, "Mark Request Lines", "Add run markers to all HTTP request lines.");
    // Add the new menu item with a keyboard shortcut
    toolsMenu->AppendSeparator();
    toolsMenu->Append(ID_RUN_REQUEST, "Run Request at Cursor\tF5", "Run the request on the current line.");
    menuBar->Append(toolsMenu, "&Tools");

  SetMenuBar(menuBar);

  // Create a status bar for feedback
  CreateStatusBar();
  SetStatusText("Ready");

  // --- 2. Create the Splitter Window ---
  // This will be the main container for our two panes.
  m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition,
                                    wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);

  // --- 3. Create the two panes ---
  // Note: The parent of these controls is now the m_splitter, not the frame.
  m_editor = new HttpEditor(m_splitter, wxID_ANY);

  m_responsePanel = new wxTextCtrl(
      m_splitter, wxID_ANY, "Response will appear here...", wxDefaultPosition,
      wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_WORDWRAP);

  // --- 4. Split the window ---
  // This is the key step. It divides the splitter window into two vertical
  // panes.
  m_splitter->SplitVertically(m_editor, m_responsePanel);

  // Set the initial position of the sash. 0 means the center.
  // Let's give the editor a bit more space initially (e.g., 60% of the width).
  m_splitter->SetSashPosition(600);

  // Set the minimum size of each pane to prevent them from being collapsed
  // by dragging the sash all the way to the edge.
  m_splitter->SetMinimumPaneSize(150);

  // --- 5. Set Initial Content ---
  wxString httpContent = R"(
@host = example.org
@port = 443

### Get a specific user
GET https://{{host}}:{{port}}/users/{{userId}}
Authorization: Bearer {{token}}
)";
  m_editor->SetText(httpContent);
}

void MyFrame::OnToggleResponsePanel(wxCommandEvent &event) {
  if (m_splitter->IsSplit()) {
    // If the window is split, unsplit it to hide the right panel.
    // The second argument (false) means we are removing the second pane
    // (m_responsePanel).
    m_splitter->Unsplit(m_responsePanel);
    SetStatusText("Response panel hidden");
  } else {
    // If the window is not split, split it again to show the panel.
    m_splitter->SplitVertically(m_editor, m_responsePanel);
    m_splitter->SetSashPosition(600); // Restore sash position
    SetStatusText("Response panel shown");
  }
}

void MyFrame::OnMarkRequests(wxCommandEvent& event)
{
    // First, clear all existing markers
    for (int i = 0; i < m_editor->GetLineCount(); ++i) {
        m_editor->SetRunRequestMarker(i, false); // Use the new method name
    }

    // Now, find lines that start with an HTTP method and add a marker
    for (int i = 0; i < m_editor->GetLineCount(); ++i) {
        wxString line = m_editor->GetLine(i);
        line.Trim(false);
        if (line.StartsWith("GET") || line.StartsWith("POST") || line.StartsWith("PUT") || line.StartsWith("DELETE")) {
            m_editor->SetRunRequestMarker(i, true); // Use the new method name
        }
    }
    SetStatusText("Run markers added to request lines.");
}

void MyFrame::OnRunRequest(wxCommandEvent& event)
{
    // 1. Get the line number where the cursor is
    int currentLine = m_editor->GetCurrentLine();
    wxString lineText = m_editor->GetLine(currentLine).Trim();

    // 2. Check if this line has our "Run Request" marker
    bool hasMarker = (m_editor->MarkerGet(currentLine) & (1 << 0)) != 0; // 0 is MARKER_RUN_REQUEST

    if (hasMarker)
    {
        // 3. If it does, run the request!
        m_responsePanel->Clear();
        m_responsePanel->AppendText("RUNNING REQUEST...\n\n");
        m_responsePanel->AppendText(wxString::Format("Line: %d\n", currentLine + 1));
        m_responsePanel->AppendText("Request: ");
        m_responsePanel->AppendText(lineText);
        m_responsePanel->AppendText("\n\n(Pretend we are sending this and showing the response here)");

        SetStatusText(wxString::Format("Running request on line %d", currentLine + 1));
    }
    else
    {
        // 4. If not, tell the user
        SetStatusText("No runnable request on the current line.");
    }
}
