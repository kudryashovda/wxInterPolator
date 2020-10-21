#include <wx/wx.h>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

using namespace std;

struct Coord {
    double x;
    double y;
};

void tokenizer(vector<string>& vs, string str, string delim);

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

class MainFrame : public wxFrame {
public:
    explicit MainFrame(const wxString &title);
    void getDataFromMemo(const wxTextCtrl& memo, const string& delim);
    vector<Coord> interpolate(double first, double last, double step);
    void showInterpolatedData(wxTextCtrl& memo);

private:
    wxPanel *pnl;
    wxButton *btnInterpolate;
    wxBoxSizer *mainSizer, *inputSizer, *memoSizer;
    wxTextCtrl *edtFirst, *edtLast, *edtStep;
    wxTextCtrl *memoInput, *memoOutput;

    wxString removeEndZeros(wxString& st);

    void OnPressBtnInterpolate(wxMouseEvent& event);
    void onClickTextCtrl(wxMouseEvent& event); // one foo for all edits

private:
    vector<Coord> m_vInput;
    vector<Coord> m_vInterpolated;
};

IMPLEMENT_APP(MyApp)

[[maybe_unused]] bool MyApp::OnInit() {
    MainFrame *frame = new MainFrame(_("wxInterPolator"));
    frame->Show(true);

    return true;
}


MainFrame::MainFrame(const wxString &title):wxFrame(NULL, wxID_ANY, title) {
    CreateStatusBar(1);
    SetStatusText(wxT("To start press 'Select File' button"), 0);

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

void MainFrame::getDataFromMemo(const wxTextCtrl& memo, const string& delim) {
    if(memo.GetValue() == ""s) {
        SetStatusText("No data was loaded");

        return;
    }

    m_vInput.clear();

    for(auto lineNo = 0; lineNo < memo.GetNumberOfLines(); ++lineNo) {
        vector<string> vs; // keep two string values of x, y

        tokenizer(vs, memo.GetLineText(lineNo).ToStdString(), delim);

        if(vs.size() == 0) continue;

        try {
            double x = stof(vs[0]);
            double y = stof(vs[1]);

            m_vInput.push_back({x,y});
        } catch(...) {
            continue;
        }
    }
}

void tokenizer(vector<string>& vs, string str, string delim) {
    size_t pos = 0;
    string token;
    vs.clear();

    if(str.find(delim) == string::npos) return;  // if no delim string - exit

    do {
        pos = str.find_first_of(delim);
        token = str.substr(0,pos);
        vs.push_back(token);
        str = str.substr(pos+1);
    } while(pos != string::npos);
}

void MainFrame::OnPressBtnInterpolate(wxMouseEvent& event) {
    event.Skip(); // if skip it - bugs with text selection appear

    getDataFromMemo(*memoInput, "\t"s);

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

    m_vInterpolated = interpolate(dFirst, dLast, dStep);

    showInterpolatedData(*memoOutput);
}

vector<Coord> MainFrame::interpolate(double first, double last, double step) {
    if (m_vInput.size() < 2) {
        return {};
    }
    
    vector<Coord> v_sorted(m_vInput.begin(), m_vInput.end());

    sort(v_sorted.begin(), v_sorted.end(), [](const Coord &a, const Coord &b) {
        return a.x < b.x;
    });

    if(first < v_sorted.front().x) first = v_sorted.front().x;  // check for lower boundary

    if(last > v_sorted.back().x) last = v_sorted.back().x;  // check for upper boundary

    size_t steps = 1 + (last - first) / step;

    size_t it = 1;

    vector<Coord> v_interpolated;

    for(size_t i = 0; i < steps; ++i) {
        double x = first + i * step;

        while(x > v_sorted[it].x) ++it;  // danger! Upper the range check was already done

        double k = 0, b = 0; // slope and intercept

        try {
            k = (v_sorted[it].y - v_sorted[it-1].y) / (v_sorted[it].x - v_sorted[it-1].x);
            b = v_sorted[it-1].y - k * v_sorted[it-1].x;
        } catch(...) {};

        v_interpolated.push_back({x, (k*x + b)});
    }

    return v_interpolated;
}

void MainFrame::showInterpolatedData(wxTextCtrl& memo) {
    wxString line = ""s;
    wxString x = ""s;
    wxString y = ""s;

    wxString longString = ""s;

    for(const auto& it : m_vInterpolated) {
        x = to_string(it.x);
        x = removeEndZeros(x); // utils.h

        y = to_string(it.y);
        y = removeEndZeros(y);

        line = x + "\t" + y + "\n";
        longString += line;
    }

    memo.SetValue(longString);
}

wxString MainFrame::removeEndZeros(wxString& st) {
    size_t end = st.find_last_not_of('0') + 1 ;
    size_t point_pos = st.find_last_of('.') + 1 ;

    if(end == point_pos) return st.erase(end+1);
    else return st.erase(end) ;
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
