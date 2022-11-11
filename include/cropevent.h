#ifndef CROPEVENT_H
#define CROPEVENT_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

class CropEvent : public wxEvent {
    public:
        CropEvent(wxEventType, int, const wxSize &, wxPoint &o);
        wxSize getSize() const;
        wxPoint getOffset() const;

        virtual wxEvent * Clone() const;
        wxDECLARE_DYNAMIC_CLASS(CropEvent);

    private:
        wxSize size;
        wxPoint offset;

};

wxDECLARE_EVENT(EVT_CROP_CHANGE, CropEvent);

#endif // RECTANGLEEVENT_H
