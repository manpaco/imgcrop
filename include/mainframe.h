#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "Magick++/Image.h"
#include <wx/wxprec.h>
#include "wx/splitter.h"

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "canvaspanel.h"
#include "toolspanel.h"
#include "previewpanel.h"
#include "cropevent.h"

typedef std::tuple<wxPoint, OptionsContainer> State;

class MainFrame: public wxFrame {
    
    public:
        MainFrame();
        ~MainFrame();

    private:
        void updatePreview(wxBitmap &bm);
        void allocateMem();
        void setBindings();
        void overlayPanels();
        void saveState(wxCommandEvent &);
        Magick::Image composeUsingState(Magick::Image &img);
        void composePreview();
        wxBitmap createBitmap(Magick::Image &img);
        void onCropChange(CropEvent &);
        void initVars();
        void updateHistory(State toSave);
        void redo(wxCommandEvent &);
        void undo(wxCommandEvent &);
        void updateCropGeometry();
        void onFixRatio(wxCommandEvent &);
        void onAllowGrow(wxCommandEvent &event);
        void createMenuBar();

        CanvasPanel *canvas = nullptr;
        ToolsPanel *tools = nullptr;
        wxSplitterWindow *mainSplitter = nullptr;
        wxSplitterWindow *sideSplitter = nullptr;
        PreviewPanel *preview = nullptr;
        wxBoxSizer *mainSizer = nullptr;
        Magick::Image *sourceImg = nullptr;
        Magick::Image *lowResImg = nullptr;

        wxMenuBar *topMenuBar;
        wxMenu *mFile, *mEdit, *mHelp;

        std::list<State> history{};
        std::list<State>::iterator currentState;

};

#endif // MAINFRAME_H
