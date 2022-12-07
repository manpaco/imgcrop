#include "rectangle.h"
#include "exceptionsdef.h"
#include <iterator>
#include <wx/graphics.h>
#include <wx/gtk/colour.h>

wxIMPLEMENT_DYNAMIC_CLASS(Rectangle, wxControl);
wxDEFINE_EVENT(EVT_RECTANGLE_CHANGE, wxCommandEvent);

Rectangle::Rectangle() {
    init();
}

Rectangle::Rectangle(wxWindow *parent, wxWindowID id, 
                const wxPoint &pos, 
                const wxSize &size, long style, 
                const wxValidator &validator, 
                const wxString &name) {
    init();
    Create(parent, id, pos, size, style, validator, name);
}

void Rectangle::init() {
    Bind(wxEVT_MOTION, &Rectangle::mouseMotion, this);
    Bind(wxEVT_LEFT_DOWN, &Rectangle::mousePress, this);
    Bind(wxEVT_LEFT_UP, &Rectangle::mouseRelease, this);
    Bind(wxEVT_LEAVE_WINDOW, &Rectangle::leaveWinHandler, this);
    Bind(wxEVT_ENTER_WINDOW, &Rectangle::enterWinHandler, this);
    Bind(wxEVT_PAINT, &Rectangle::onPaint, this);
    Bind(wxEVT_SIZE, &Rectangle::updateSizes, this);
}

void Rectangle::updateSizes(wxSizeEvent &event) {
    iz = wxRect(0, 0, event.GetSize().GetWidth(), event.GetSize().GetHeight());
    nz = wxRect(0, 0, event.GetSize().GetWidth(), corner);
    sz = wxRect(0, event.GetSize().GetHeight() - corner, event.GetSize().GetWidth(), corner);
    ez = wxRect(event.GetSize().GetWidth() - corner, 0, corner, event.GetSize().GetHeight());
    wz = wxRect(0, 0, corner, event.GetSize().GetHeight());
    nez = wxRect(event.GetSize().GetWidth() - corner, 0, corner, corner);
    nwz = wxRect(0, 0, corner, corner);
    sez = wxRect(event.GetSize().GetWidth() - corner, 
            event.GetSize().GetHeight() - corner, corner, corner);
    swz = wxRect(0, event.GetSize().GetHeight() - corner, corner, corner);
    event.Skip();
}

void Rectangle::sendChangeEvent() {
    wxCommandEvent toSend(EVT_RECTANGLE_CHANGE, GetId());
    toSend.SetEventObject(this);
    ProcessWindowEvent(toSend);
}

void Rectangle::setRatio(float r) {
    ratio = r;
}

void Rectangle::leaveWinHandler(wxMouseEvent &event) {
    if(!zonePressed) changeCursor(ict::NONE);
    mouseLeftWin = true;
}

void Rectangle::enterWinHandler(wxMouseEvent &event) {
    mouseLeftWin = false;
}

void Rectangle::mouseMotion(wxMouseEvent &event) {
    if(!zonePressed) {
        changeCursor(getLocation(event.GetPosition()));
        event.Skip();
        return;
    }
    if(zonePressed == ict::INNER) {
        wxPoint screenPoint(wxGetMousePosition());
        screenPoint = screenPoint - clientPressPoint;
        wxPoint parentClientPoint(GetParent()->ScreenToClient(screenPoint));
        wxRect newGeometry(parentClientPoint.x, parentClientPoint.y, GetSize().GetWidth(), GetSize().GetHeight());
        if(restricted) {
            int cx1 = restrictions.GetPosition().x, cx2 = restrictions.GetPosition().x + restrictions.GetSize().GetWidth();
            int nx1 = newGeometry.GetPosition().x, nx2 = newGeometry.GetPosition().x + newGeometry.GetSize().GetWidth();
            if(nx1 < cx1) newGeometry.SetPosition(wxPoint(cx1, newGeometry.GetPosition().y));
            if(nx2 > cx2) newGeometry.SetPosition(wxPoint(cx2 - GetSize().GetWidth(), newGeometry.GetPosition().y));
            int cy1 = restrictions.GetPosition().y, cy2 = restrictions.GetPosition().y + restrictions.GetSize().GetHeight();
            int ny1 = newGeometry.GetPosition().y, ny2 = newGeometry.GetPosition().y + newGeometry.GetSize().GetHeight();
            if(ny1 < cy1) newGeometry.SetPosition(wxPoint(newGeometry.GetPosition().x, cy1));
            if(ny2 > cy2) newGeometry.SetPosition(wxPoint(newGeometry.GetPosition().x, cy2 - GetSize().GetHeight()));
        }
        Move(newGeometry.GetPosition());
    } else {
        resizeUsing(zonePressed);
    }
    event.Skip();
}

void Rectangle::fixRatio(bool op) {
    fix = op;
    if(fix) setRatio((float)GetSize().GetWidth() / (float)GetSize().GetHeight());
}

bool Rectangle::fixRatio() const {
    return fix;
}

void Rectangle::setRestrictions(const wxRect &r) {
    restrictions = r;
}

void Rectangle::activateRestrictions(bool op) {
    restricted = op;
    setGeometryInternally(GetRect());
}

void Rectangle::setGeometryInternally(const wxRect &g) {
    float prevAx = accumX, prevAy = accumY;
    setGeometry(g);
    if(g.GetSize() == GetSize()) {
        accumX = prevAx;
        accumY = prevAy;
    }
}

void Rectangle::setGeometry(const wxRect &g) {
    wxRect next(g);
    wxRect prevRect = GetRect();
    fixHint = ict::SE;
    setRatio((float)g.GetWidth() / (float)g.GetHeight());
    accumX = 0.0; accumY = 0.0;
    if(restricted && !restrictions.Contains(next.GetPosition())) {
        next.SetPosition(restrictions.GetPosition());
    }
    modify(next);
    if(prevRect != GetRect() || g != GetRect()) sendChangeEvent();
}

void Rectangle::fitInRestrictions(wxRect &fixRatioRect) {
    bool exceedsTopY = fixRatioRect.GetY() < restrictions.GetY();
    bool exceedsBottomY = (fixRatioRect.GetY() + fixRatioRect.GetHeight()) > (restrictions.GetY() + restrictions.GetHeight());
    bool exceedsLeftX = fixRatioRect.GetX() < restrictions.GetX();
    bool exceedsRightX = (fixRatioRect.GetX() + fixRatioRect.GetWidth()) > (restrictions.GetX() + restrictions.GetWidth());
    wxRect aux(fixRatioRect);
    aux.Intersect(restrictions);
    int newWidth, newHeight;
    if(fixHint == ict::SE || fixHint == ict::S) {
        if(exceedsRightX) {
            newWidth = aux.GetWidth();
            defineY(newHeight, newWidth);
            fixRatioRect.SetSize(wxSize(newWidth, newHeight));
            exceedsBottomY = (fixRatioRect.GetY() + fixRatioRect.GetHeight()) > (restrictions.GetY() + restrictions.GetHeight());
        }
        if(exceedsBottomY) {
            newHeight = aux.GetHeight();
            defineX(newWidth, newHeight);
            fixRatioRect.SetSize(wxSize(newWidth, newHeight));
        }
    }
    if(fixHint == ict::SW || fixHint == ict::W) {
        if(exceedsLeftX) {
            newWidth = aux.GetWidth();
            defineY(newHeight, newWidth);
            fixRatioRect.SetSize(wxSize(newWidth, newHeight));
            fixRatioRect.SetPosition(wxPoint(restrictions.GetX(), fixRatioRect.GetY()));
            exceedsBottomY = (fixRatioRect.GetY() + fixRatioRect.GetHeight()) > (restrictions.GetY() + restrictions.GetHeight());
        }
        if(exceedsBottomY) {
            newHeight = aux.GetHeight();
            defineX(newWidth, newHeight);
            wxPoint newPos(fixRatioRect.GetX() + fixRatioRect.GetWidth() - newWidth, fixRatioRect.GetY());
            fixRatioRect.SetSize(wxSize(newWidth, newHeight));
            fixRatioRect.SetPosition(newPos);
        }
    }
    if(fixHint == ict::NE || fixHint == ict::E) {
        if(exceedsRightX) {
            newWidth = aux.GetWidth();
            defineY(newHeight, newWidth);
            wxPoint newPos(fixRatioRect.GetX(), fixRatioRect.GetY() + fixRatioRect.GetHeight() - newHeight);
            fixRatioRect.SetSize(wxSize(newWidth, newHeight));
            fixRatioRect.SetPosition(newPos);
            exceedsTopY = fixRatioRect.GetY() < restrictions.GetY();
        }
        if(exceedsTopY) {
            newHeight = aux.GetHeight();
            defineX(newWidth, newHeight);
            fixRatioRect.SetSize(wxSize(newWidth, newHeight));
            fixRatioRect.SetPosition(wxPoint(fixRatioRect.GetX(), restrictions.GetY()));
        }
    }
    if(fixHint == ict::NW || fixHint == ict::N) {
        if(exceedsLeftX) {
            newWidth = aux.GetWidth();
            defineY(newHeight, newWidth);
            wxPoint newPos(restrictions.GetX(), fixRatioRect.GetY() + fixRatioRect.GetHeight() - newHeight);
            fixRatioRect.SetSize(wxSize(newWidth, newHeight));
            fixRatioRect.SetPosition(newPos);
            exceedsTopY = fixRatioRect.GetY() < restrictions.GetY();
        }
        if(exceedsTopY) {
            newHeight = aux.GetHeight();
            defineX(newWidth, newHeight);
            wxPoint newPos(fixRatioRect.GetX() + fixRatioRect.GetWidth() - newWidth, restrictions.GetY());
            fixRatioRect.SetSize(wxSize(newWidth, newHeight));
            fixRatioRect.SetPosition(newPos);
        }
    }
}

void Rectangle::modify(const wxRect &ng) {
    wxRect next(ng);
    if(restricted && !restrictions.Contains(next)) {
        if(!fix) next.Intersect(restrictions);
        else fitInRestrictions(next);
    }
    SetSize(next.GetX(), next.GetY(), next.GetWidth(), next.GetHeight());
}

void Rectangle::defineX(int &dxToCalc, int &dyToUse) {
    accumX = (float)dyToUse * ratio;
    dxToCalc = std::floor(accumX);
    accumX -= dxToCalc;
    accumY = 0;
}

void Rectangle::defineY(int &dyToCalc, int &dxToUse) {
    accumY = (float)dxToUse / ratio;
    dyToCalc = std::floor(accumY);
    accumY -= dyToCalc;
    accumX = 0;
}

void Rectangle::accumulateX(int &dxToCalc, int &dyToUse) {
    accumX += (float)dyToUse * ratio;
    dxToCalc = std::floor(accumX);
    accumX -= dxToCalc;
    accumY = 0;
}

void Rectangle::accumulateY(int &dyToCalc, int &dxToUse) {
    accumY += (float)dxToUse / ratio;
    dyToCalc = std::floor(accumY);
    accumY -= dyToCalc;
    accumX = 0;
}

void Rectangle::resizeUsing(ict::Zone zone){
    if(fix) fixHint = zone;
    wxPoint mousePosition(wxGetMousePosition());
    wxPoint positionOnScreen(GetScreenPosition());
    wxPoint positionOnParent(GetPosition());
    int deltaX = 0, deltaY = 0;
    if(zone == ict::SE) {
        deltaX += mousePosition.x - 
                (positionOnScreen.x + GetSize().GetWidth());
        deltaY += mousePosition.y - 
                (positionOnScreen.y + GetSize().GetHeight());
        if(fix) accumulateX(deltaX, deltaY);
        wxRect newGeometry(positionOnParent.x, 
                positionOnParent.y, 
                GetSize().GetWidth() + deltaX, 
                GetSize().GetHeight() + deltaY);
        modify(newGeometry);
        return;
    }
    if(zone == ict::NW) {
        deltaX += mousePosition.x - positionOnScreen.x;
        deltaY += mousePosition.y - positionOnScreen.y;
        if(fix) accumulateX(deltaX, deltaY);
        wxRect newGeometry(positionOnParent.x + deltaX, 
                positionOnParent.y + deltaY,
                GetSize().GetWidth() - deltaX, 
                GetSize().GetHeight() - deltaY);
        modify(newGeometry);
        return;
    }
    if(zone == ict::NE) {
        deltaX += mousePosition.x - 
                (positionOnScreen.x + GetSize().GetWidth());
        deltaY += mousePosition.y - positionOnScreen.y;
        if(fix) { accumulateX(deltaX, deltaY); deltaX = -deltaX; }
        wxRect newGeometry(positionOnParent.x, 
                positionOnParent.y + deltaY, 
                GetSize().GetWidth() + deltaX, 
                GetSize().GetHeight() - deltaY);
        modify(newGeometry);
        return;
    }
    if(zone == ict::SW) {
        deltaX += mousePosition.x - positionOnScreen.x;
        deltaY += mousePosition.y - 
                (positionOnScreen.y + GetSize().GetHeight());
        if(fix) { accumulateX(deltaX, deltaY); deltaX = -deltaX; }
        wxRect newGeometry(positionOnParent.x + deltaX, 
                positionOnParent.y, 
                GetSize().GetWidth() - deltaX, 
                GetSize().GetHeight() + deltaY);
        modify(newGeometry);
        return;
    }
    if(zone == ict::N) {
        deltaY += mousePosition.y - positionOnScreen.y;
        if(fix) accumulateX(deltaX, deltaY);
        wxRect newGeometry(positionOnParent.x + deltaX, 
                positionOnParent.y + deltaY, 
                GetSize().GetWidth() - deltaX, 
                GetSize().GetHeight() - deltaY);
        modify(newGeometry);
        return;
    }
    if(zone == ict::S) {
        deltaY += mousePosition.y - 
                (positionOnScreen.y + GetSize().GetHeight());
        if(fix) accumulateX(deltaX, deltaY);
        wxRect newGeometry(positionOnParent.x, 
                positionOnParent.y, 
                GetSize().GetWidth() + deltaX, 
                GetSize().GetHeight() + deltaY);
        modify(newGeometry);
        return;
    }
    if(zone == ict::W) {
        deltaX += mousePosition.x - positionOnScreen.x;
        if(fix) { accumulateY(deltaY, deltaX); deltaY = -deltaY; }
        wxRect newGeometry(positionOnParent.x + deltaX, 
                positionOnParent.y, 
                GetSize().GetWidth() - deltaX, 
                GetSize().GetHeight() + deltaY);
        modify(newGeometry);
        return;
    }
    if(zone == ict::E) {
        deltaX += mousePosition.x - 
                (positionOnScreen.x + GetSize().GetWidth());
        if(fix) { accumulateY(deltaY, deltaX); deltaY = -deltaY; }
        wxRect newGeometry(positionOnParent.x, 
                positionOnParent.y + deltaY, 
                GetSize().GetWidth() + deltaX, 
                GetSize().GetHeight() - deltaY);
        modify(newGeometry);
        return;
    }
}

void Rectangle::changeCursor(ict::Zone type) {
    if(type == ict::N || type == ict::S) 
        wxSetCursor(wxCursor(wxCURSOR_SIZENS));
    if(type == ict::E || type == ict::W) 
        wxSetCursor(wxCursor(wxCURSOR_SIZEWE));
    if(type == ict::NE || type == ict::SW) 
        wxSetCursor(wxCursor(wxCURSOR_SIZENESW));
    if(type == ict::NW || type == ict::SE) 
        wxSetCursor(wxCursor(wxCURSOR_SIZENWSE));
    if(type == ict::INNER)
        wxSetCursor(wxCursor(wxCURSOR_HAND));
    if(type == ict::NONE)
        wxSetCursor(wxNullCursor);
}

void Rectangle::mousePress(wxMouseEvent &event) {
    try {
        if(!HasCapture()) CaptureMouse();
        else throw std::runtime_error("Error code: ");
    } catch (std::runtime_error &e) {
        std::cout << e.what() << MOUSE_ALREDY_CAPTURED << std::endl;
    }
    clientPressPoint = event.GetPosition();
    zonePressed = getLocation(event.GetPosition());
    rectInPress = GetRect();
}

void Rectangle::mouseRelease(wxMouseEvent &event) {
    try {
        if(HasCapture()) ReleaseMouse();
        else throw std::runtime_error("Error code: ");
    } catch (std::runtime_error &e) {
        std::cout << e.what() << MOUSE_NEVER_CAPTURED << std::endl;
    }
    if(rectInPress != GetRect()) sendChangeEvent();
    zonePressed = ict::NONE;
    if(mouseLeftWin) changeCursor(ict::NONE);
    else changeCursor(getLocation(event.GetPosition()));
}

ict::Zone Rectangle::getLocation(const wxPoint p) {
    if(nez.Contains(p)) return ict::NE;
    if(nwz.Contains(p)) return ict::NW;
    if(sez.Contains(p)) return ict::SE;
    if(swz.Contains(p)) return ict::SW;
    if(nz.Contains(p)) return ict::N;
    if(sz.Contains(p)) return ict::S;
    if(ez.Contains(p)) return ict::E;
    if(wz.Contains(p)) return ict::W;
    if(iz.Contains(p)) return ict::INNER;
    return ict::NONE;
}

void Rectangle::paintSpecialFrame(const wxRect &paint, wxGraphicsContext *gc, bool fill) {
    wxPen wLine(wxColour(*wxWHITE), 1);
    wxPen bLine(wxColour(*wxBLACK), 1);
    gc->SetBrush(wxBrush(wxColour(0, 0, 0, 0)));
    gc->SetPen(bLine);
    gc->DrawRectangle(paint.GetX(), paint.GetY(), paint.GetWidth() - 1, paint.GetHeight() - 1);
    gc->SetPen(wLine);
    gc->DrawRectangle(paint.GetX() + 1, paint.GetY() + 1, paint.GetWidth() - 3, paint.GetHeight() - 3);
    gc->SetPen(bLine);
    if(fill) { 
        gc->SetBrush(wxBrush(wxColour(*wxWHITE)));
        gc->SetPen(wxPen(wxColour(*wxWHITE)));
    }
    gc->DrawRectangle(paint.GetX() + 2, paint.GetY() + 2, paint.GetWidth() - 5, paint.GetHeight() - 5);
}

void Rectangle::onPaint(wxPaintEvent &) {
    wxPaintDC device(this);
    wxGraphicsContext *gcd = wxGraphicsContext::Create(device);
    if(gcd) {
        paintSpecialFrame(wxRect(0, 0, GetSize().GetWidth(), GetSize().GetHeight()), gcd, false);
        paintSpecialFrame(nwz, gcd, true);
        paintSpecialFrame(nez, gcd, true);
        paintSpecialFrame(sez, gcd, true);
        paintSpecialFrame(swz, gcd, true);
        delete gcd;
    }
}

Rectangle::~Rectangle() {

}
