#ifndef TOOLSPANEL_H
#define TOOLSPANEL_H

class wxColourPickerEvent;
class wxFileDirPickerEvent;
class wxChoice;
class wxTextCtrl;
class wxSlider;
class wxFilePickerCtrl;
class wxColourPickerCtrl;
class wxStaticText;

#include "wx/scrolwin.h"
#include "wx/checkbox.h"
#include "wx/valnum.h"

#if wxUSE_COLLPANE
    #include <wx/collpane.h>
#endif

#include "optscontainer.h"
#include "defs.h"

class ToolsPanel: public wxScrolledCanvas {
    public:
        ToolsPanel(wxWindow *parent, wxWindowID id);
        unsigned int widthCrop() const;
        unsigned int hegihtCrop() const;
        void widthCrop(unsigned int width);
        void heightCrop(unsigned int height);
        void cropSize(const wxSize &s);
        void cropGeometry(const wxRect &r);
        wxSize cropSize() const;
        bool valid();
        void strokeWidth(unsigned int sw);
        OptionsContainer currentOpts() const;
        void setOpts(const OptionsContainer &oc);
        void clear(bool enableOp);
        void collapseBlocks();
        ~ToolsPanel();

    private:
        void widthChange(wxCommandEvent &event);
        void heightChange(wxCommandEvent &event);
        void fixRatioChange(wxCommandEvent &event);
        void shapeChange(wxCommandEvent &event);
        void colourChange(wxColourPickerEvent &event);
        void imageChange(wxFileDirPickerEvent &event);
        void blurChange(wxScrollEvent &event);
        void growChoiceChange(wxCommandEvent &event);
        void growStateChange(wxCommandEvent &event);
        void updateVirtualSize(wxCollapsiblePaneEvent &event);
        void strokeColorChange(wxColourPickerEvent &event);
        void strokeWidthChange(wxCommandEvent &event);

        void setBindings();
        void createTools();
        void overlayTools();
        void createAspectBlock();
        void createGrowBlock();
        void createShapeBlock();
        void initGrowChoices();
        void initShapeChoices();
        void growChoiceState(bool state, int choice);
        void updateGrowBlock();
        
        class UnfocusedCheckBox : public wxCheckBox {
            public:
                UnfocusedCheckBox(wxWindow *parent, wxWindowID id, const wxString &label) {
                    Create(parent, id, label);
                }
                bool AcceptsFocus() const {
                    return false;
                }
        };
        
        wxCollapsiblePane *aspectBlock;
        wxTextCtrl *widthCtrl;
        wxTextCtrl *heightCtrl;
        UnfocusedCheckBox *fixRatio;

        wxCollapsiblePane *growBlock;
        UnfocusedCheckBox *growCheck;
        wxChoice *growSelector;
        wxSlider *backBlur;
        wxFilePickerCtrl *imagePicker;
        wxColourPickerCtrl *colorPicker;
        wxString growChoices[ict::GROW_CHOICE_SIZE];
        wxStaticText *blurText;
        wxStaticText *colorText;
        wxStaticText *imageText;

        wxCollapsiblePane *shapeBlock;
        wxChoice *shapeSelector;
        wxTextCtrl *strokeWidthCtrl;
        wxColourPickerCtrl *strokeColorPicker;
        wxString shapeChoices[ict::SHAPE_CHOICE_SIZE];

        OptionsContainer opts;
        wxIntegerValidator<unsigned int> wVal;
        wxIntegerValidator<unsigned int> hVal;
        wxIntegerValidator<unsigned int> swVal;

};

#endif // TOOLSPANEL_H
