#include "interpolator.h"

#include <wx/wx.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

class MainFrame : public wxFrame {
public:
    explicit MainFrame(const wxString &title);
private:
    wxPanel *pnl;
    wxButton *btnInterpolate;
    wxBoxSizer *mainSizer, *inputSizer, *memoSizer;
    wxTextCtrl *edtFirst, *edtLast, *edtStep;
    wxTextCtrl *memoInput, *memoOutput;

private:
    wxString removeEndZeros(wxString& st);
    void showInterpolatedData(wxTextCtrl* memo, const vector<Coord>& vInterpolated);

    void OnPressBtnInterpolate(wxMouseEvent& event);
    void onClickTextCtrl(wxMouseEvent& event);
};

IMPLEMENT_APP(MyApp)

[[maybe_unused]] bool MyApp::OnInit() {
    MainFrame *frame = new MainFrame(_("wxInterPolator"));
    frame->Show(true);

    return true;
}

MainFrame::MainFrame(const wxString &title):wxFrame(NULL, wxID_ANY, title) {
    CreateStatusBar(1);
    SetStatusText(wxT("To start enter or paste x,y data to the left field"), 0);

    pnl = new wxPanel(this, wxID_ANY);

    btnInterpolate = new wxButton(pnl, wxID_ANY, "Interpolate");

    btnInterpolate->Bind(wxEVT_LEFT_UP, &MainFrame::OnPressBtnInterpolate, this);

    edtFirst = new wxTextCtrl(pnl, wxID_ANY, wxT("First value:"));
    edtLast = new wxTextCtrl(pnl, wxID_ANY, wxT("Last value:"));
    edtStep = new wxTextCtrl(pnl, wxID_ANY, wxT("Step value:"));

    memoInput = new wxTextCtrl(pnl, wxID_ANY, wxT(""),
                               wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_RICH,
                               wxDefaultValidator, wxTextCtrlNameStr);

    memoOutput = new wxTextCtrl(pnl, wxID_ANY, wxT(""),
                                wxDefaultPosition, wxDefaultSize,
                                wxTE_MULTILINE | wxTE_RICH,
                                wxDefaultValidator, wxTextCtrlNameStr);

    edtFirst->Bind(wxEVT_LEFT_UP, &MainFrame::onClickTextCtrl, this);
    edtLast->Bind(wxEVT_LEFT_UP, &MainFrame::onClickTextCtrl, this);
    edtStep->Bind(wxEVT_LEFT_UP, &MainFrame::onClickTextCtrl, this);

    inputSizer = new wxBoxSizer(wxHORIZONTAL);

    inputSizer->Add(edtFirst, 1,  wxEXPAND | wxALL, 5);
    inputSizer->Add(edtLast, 1,  wxEXPAND | wxALL, 5);
    inputSizer->Add(edtStep, 1,  wxEXPAND | wxALL, 5);
    inputSizer->Add(btnInterpolate, 0, wxALL, 5);

    memoSizer =  new wxBoxSizer(wxHORIZONTAL);
    memoSizer->Add(memoInput, 1, wxEXPAND | wxALL, 5);
    memoSizer->Add(memoOutput, 1, wxEXPAND | wxALL, 5);

    mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(inputSizer, 0, wxALL, 5);
    mainSizer->Add(memoSizer, 1, wxEXPAND | wxALL, 5);

    pnl->SetSizer(mainSizer);

    mainSizer->Fit(this); // resize (fit) main window based on elements inside sizer

    wxSize ws = this->GetSize();

    SetMinSize(wxSize(ws.GetWidth(), 500));
    SetMaxSize(wxSize(ws.GetWidth(), 500));
    Centre();
}

void MainFrame::OnPressBtnInterpolate(wxMouseEvent& event) {
    event.Skip(); // if skip it - bugs with text selection appear

    wxString tFirst = edtFirst->GetValue();
    wxString tLast = edtLast->GetValue();
    wxString tStep = edtStep->GetValue();

    double dFirst, dLast, dStep;

    if(!tFirst.ToCDouble(&dFirst) or
            !tLast.ToCDouble(&dLast) or
            !tStep.ToCDouble(&dStep)) {
        SetStatusText("Error! Please enter real numbers", 0);
        return;
    }

    wxString wxMemoText = MainFrame::memoInput->GetValue();

    Interpolator interpolator(dFirst, dLast, dStep, std::string(wxMemoText.mb_str()), '\n', '\t');

    showInterpolatedData(memoOutput, interpolator.getInterpolatedData());
}

void MainFrame::showInterpolatedData(wxTextCtrl* memo, const vector<Coord>& datas) {
    if(datas.empty()) {
        memo->SetValue("No interpolated data was found");
        return;
    }

    wxString line = ""s;
    wxString x = ""s;
    wxString y = ""s;

    wxString longString = ""s;

    for(const auto& it : datas) {
        x = to_string(it.x);
        x = removeEndZeros(x); // utils.h

        y = to_string(it.y);
        y = removeEndZeros(y);

        line = x + "\t" + y + "\n";
        longString += line;
    }

    memo->SetValue(longString);
}

wxString MainFrame::removeEndZeros(wxString& st) {
    size_t end = st.find_last_not_of('0') + 1 ;
    size_t point_pos = st.find_last_of('.') + 1 ;

    if(end == point_pos) {
        return st.erase(end+1);
    } else {
        return st.erase(end);
    }
}

// If entered any char then the field is cleared
void MainFrame::onClickTextCtrl(wxMouseEvent& event) {
    // get object where the event was generated
    wxTextCtrl *tc = (wxTextCtrl*)event.GetEventObject();
    event.Skip();

    wxString textValue = tc->GetValue();

    double nd = 0; // double value of number

    if(!textValue.ToCDouble(&nd)) tc->Clear();
}
