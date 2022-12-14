#ifndef RECTANGLE_H
#define RECTANGLE_H

class wxGraphicsContext;

#include "wx/control.h"
#include "defs.h"

class DuctileRectangle : public wxControl {
    public:
        DuctileRectangle();
        DuctileRectangle(wxWindow *parent, wxWindowID id, 
                const wxPoint &pos=wxDefaultPosition, 
                const wxSize &size=wxDefaultSize, long style=wxBORDER_NONE, 
                const wxValidator &validator=wxDefaultValidator, 
                const wxString &name=wxControlNameStr);    
        void fixRatio(bool op);
        bool fixRatio() const;
        void setRatio(float r);
        void setRestrictions(const wxRect &r);
        void activateRestrictions(bool op);
        void setGeometry(const wxRect &g);
        void sendChangeEvent();
        ~DuctileRectangle();
        
        wxDECLARE_DYNAMIC_CLASS(DuctileRectangle);

    private:
        ict::Zone getLocation(const wxPoint);
        void changeCursor(ict::Zone type);
        void modify(const wxRect &ng);
        void resizeUsing(ict::Zone);
        void fitInRestrictions(wxRect &fixRatioRect);
        void init();
        void onPaint(wxPaintEvent &);
        void mouseMotion(wxMouseEvent &);
        void mousePress(wxMouseEvent &);
        void mouseRelease(wxMouseEvent &);
        void leaveWinHandler(wxMouseEvent &);
        void enterWinHandler(wxMouseEvent &);
        void updateSizes(wxSizeEvent &);
        void paintSpecialFrame(const wxRect &, wxGraphicsContext *, bool);
        void accumulateX(int &dxToCalc, int &dyToUse);
        void accumulateY(int &dyToCalc, int &dxToUse);
        void defineX(int &dxToCalc, int &dyToUse);
        void defineY(int &dyToCalc, int &dxToUse);
        void setGeometryInternally(const wxRect &g);

        wxPoint clientPressPoint;
        wxRect iz, nz, sz, ez, wz, nez, nwz, sez, swz;
        bool mouseLeftWin = true;
        ict::Zone zonePressed = ict::NONE;
        wxRect rectInPress;

        float ratio;
        bool fix = false;
        float accumX = 0.0, accumY = 0.0;
        wxRect restrictions;
        bool restricted = false;
        ict::Zone fixHint;
        
        const int bestWidth = 1;
        const int corner = 10;
        const int resizeLimit = corner * 3;

};

wxDECLARE_EVENT(EVT_RECTANGLE_CHANGE, wxCommandEvent);

#endif // RECTANGLE_H
