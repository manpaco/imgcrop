#include "canvaspanel.h"
#include "cropevent.h"
#include "defs.h"
#include <wx/graphics.h>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

CanvasPanel::CanvasPanel(wxWindow *parent, wxWindowID id) {
    Create(parent, id);
}

CanvasPanel::CanvasPanel(wxWindow *parent, wxWindowID id, wxBitmap &bm): CanvasPanel(parent, id) {
    initCanvas(bm);
    Bind(wxEVT_MOTION, &CanvasPanel::mouseMotion, this);
    Bind(wxEVT_LEFT_DOWN, &CanvasPanel::mousePress, this);
    Bind(wxEVT_LEFT_UP, &CanvasPanel::mouseRelease, this);
    Bind(wxEVT_PAINT, &CanvasPanel::onPaint, this);
    Bind(wxEVT_ERASE_BACKGROUND, [](wxEraseEvent &){});
}

void CanvasPanel::mouseMotion(wxMouseEvent &event) {
    ict::Zone lastZone(controller.getLocation(translatePointOut(lastPoint)));
    wxPoint currentPoint(event.GetPosition());
    ict::Zone currentZone(controller.getLocation(translatePointOut(currentPoint)));
    if(!controller.zonePressed()) {
        if(lastZone != currentZone) changeCursor(currentZone);
    } else {
        if(controller.modify(translatePointOut(currentPoint))) sendCropEvent();
    }
    lastPoint = currentPoint;
    event.Skip();
}

void CanvasPanel::mousePress(wxMouseEvent &event) {
    if(!HasCapture()) CaptureMouse();
    controller.press(translatePointOut(event.GetPosition()));
    event.Skip();
}

void CanvasPanel::mouseRelease(wxMouseEvent &event) {
    if(HasCapture()) ReleaseMouse();
    controller.release();
    changeCursor(controller.getLocation(translatePointOut(lastPoint)));
    event.Skip();
}

void CanvasPanel::sendCropEvent() {   
    wxSize cs(controller.cropSize());
    wxPoint co(relativeToImage(controller.cropPosition()));
    CropEvent toSend(EVT_CROP_CHANGE, GetId(), cs, co);
    toSend.SetEventObject(this);
    ProcessWindowEvent(toSend);
    RefreshRect(translateRectIn(prevCrop));
    prevCrop = controller.cropRect();
}

void CanvasPanel::onPaint(wxPaintEvent &event) {
    wxPaintDC painter(this);
    wxRegion damaged(GetUpdateRegion());
    wxRegionIterator it(damaged);
    while(it) {
        wxRect upd(it.GetX(), it.GetY(), it.GetW(), it.GetH());
        wxBitmap patch(buffer->GetSubBitmap(translateRectOut(upd)));
        patch.SetWidth(upd.GetWidth());
        patch.SetHeight(upd.GetHeight());
        painter.DrawBitmap(patch, upd.GetPosition());
        it++;
    }
    //if(!damaged.Intersect(translateRectIn(prevCrop))) {
    //    event.Skip();
    //    return;
    //}
    wxGraphicsContext *gcd = wxGraphicsContext::Create(painter);
    if(gcd) {
        paintFrame(translateRectIn(controller.cropRect()), gcd, false);
        paintFrame(translateRectIn(controller.rectZone(ict::NW)), gcd, true);
        paintFrame(translateRectIn(controller.rectZone(ict::NE)), gcd, true);
        paintFrame(translateRectIn(controller.rectZone(ict::SE)), gcd, true);
        paintFrame(translateRectIn(controller.rectZone(ict::SW)), gcd, true);
        delete gcd;
    }
    event.Skip();
}

void CanvasPanel::paintFrame(const wxRect &paint, wxGraphicsContext *gc, bool fill) {
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

void CanvasPanel::changeCursor(ict::Zone type) {
    if(type == ict::NE || type == ict::SW) 
        wxSetCursor(wxCursor(wxCURSOR_SIZENESW));
    if(type == ict::NW || type == ict::SE) 
        wxSetCursor(wxCursor(wxCURSOR_SIZENWSE));
    if(type == ict::N || type == ict::S) 
        wxSetCursor(wxCursor(wxCURSOR_SIZENS));
    if(type == ict::E || type == ict::W) 
        wxSetCursor(wxCursor(wxCURSOR_SIZEWE));
    if(type == ict::INNER)
        wxSetCursor(wxCursor(wxCURSOR_HAND));
    if(type == ict::NONE)
        wxSetCursor(wxNullCursor);
}

bool CanvasPanel::cropGeometry(wxRect *g) {
    wxPoint newPos(absoluteCoords(g->GetPosition()));
    wxSize newSz(g->GetSize());
    if(g->GetSize().x < 0) newSz.SetWidth(imgRect.GetWidth());
    if(g->GetSize().y < 0) newSz.SetHeight(imgRect.GetHeight());
    wxRect newG(newPos, newSz);
    if(controller.cropRect(newG)) sendCropEvent();
    newG = controller.cropRect();
    newG.SetPosition(relativeToImage(newG.GetPosition()));
    if(newG != *g) {
        *g = newG;
        return true;
    } else return false;    
}

wxPoint CanvasPanel::relativeToImage(const wxPoint &ap, bool scaled) const {
    wxPoint rel;
    if(scaled) rel = wxPoint(ap - translatePointIn(imgRect.GetPosition()));
    else rel = wxPoint(ap - imgRect.GetPosition());
    return rel;
}

wxPoint CanvasPanel::absoluteCoords(const wxPoint &rp, bool scaled) const {
    wxPoint abs;
    if(scaled) abs = wxPoint(rp + translatePointIn(imgRect.GetPosition()));
    else abs = wxPoint(rp + imgRect.GetPosition());
    return abs;
}

void CanvasPanel::fixCrop(bool op) {
    controller.fixRatio(op);
}

void CanvasPanel::allowGrow(bool op) {
    if(controller.constraint(!op)) sendCropEvent();
}

wxPoint CanvasPanel::cropOffset() const {
    return relativeToImage(controller.cropPosition());
}

bool CanvasPanel::cropSize(wxSize *s) {
    if(controller.cropSize(*s)) sendCropEvent();
    if(controller.cropSize() != *s) {
        *s = controller.cropSize();
        return true;
    }
    return false;
}

wxSize CanvasPanel::cropSize() const {
    return controller.cropSize();
}

int CanvasPanel::translateIn(int v) const {
    return v * scaleFactor;
}

int CanvasPanel::translateOut(int v) const {
    return v / scaleFactor;
}

wxRect CanvasPanel::translateRectIn(const wxRect &r) const {
    return wxRect(translateIn(r.GetX()), translateIn(r.GetY()), 
            translateIn(r.GetWidth()), translateIn(r.GetHeight()));
}

wxRect CanvasPanel::translateRectOut(const wxRect &r) const {
    return wxRect(translateOut(r.GetX()), translateOut(r.GetY()), 
            translateOut(r.GetWidth()), translateOut(r.GetHeight()));
}

wxPoint CanvasPanel::translatePointIn(const wxPoint &p) const {
    return wxPoint(translateIn(p.x), translateIn(p.y));
}

wxPoint CanvasPanel::translatePointOut(const wxPoint &p) const {
    return wxPoint(translateOut(p.x), translateOut(p.y));
}

wxSize CanvasPanel::translateSizeIn(const wxSize &s) const {
    return wxSize(translateIn(s.GetWidth()), translateIn(s.GetHeight()));
}

wxSize CanvasPanel::translateSizeOut(const wxSize &s) const {
    return wxSize(translateOut(s.GetWidth()), translateOut(s.GetHeight()));
}

void CanvasPanel::refreshCanvas() {
    updateSizes();
    Refresh();
}

void CanvasPanel::setScaleFactor(float sf) {
    if(sf == scaleFactor) return;
    scaleFactor = sf;
    refreshCanvas();
}

void CanvasPanel::initCanvas(wxBitmap &bm) {
    if(!bm.IsOk()) return;
    imgRect = wxRect(bm.GetWidth(), bm.GetHeight(), bm.GetWidth(), bm.GetHeight());
    controller = CropController(imgRect);
    prevCrop = imgRect;
    initBuffer(bm);
    bufferSize = buffer->GetSize();
    refreshCanvas();
}

void CanvasPanel::updateSizes() {
    wxSize newSize(imgRect.GetSize() * ict::IMG_MULTIPLIER);
    SetMinSize(translateSizeIn(newSize));
    SetSize(GetMinSize());
    buffer->SetWidth(translateIn(bufferSize.GetWidth()));
    buffer->SetHeight(translateIn(bufferSize.GetHeight()));
}

wxRect CanvasPanel::shadowRect() {
    wxPoint posOff(ict::SHADOW_OFFSET, ict::SHADOW_OFFSET);
    wxPoint shPos(imgRect.GetPosition() - posOff);
    wxSize szOff(ict::SHADOW_OFFSET * 2, ict::SHADOW_OFFSET * 2);
    wxSize shSz(imgRect.GetSize() + szOff);
    return wxRect(shPos, shSz);
}

void CanvasPanel::initBuffer(wxBitmap &bm) {
    if(buffer) delete buffer;
    buffer = new wxBitmap(imgRect.GetSize() * ict::IMG_MULTIPLIER);
    wxMemoryDC memBb(*buffer);
    wxBrush backBrush(wxColour(188, 188, 188));
    memBb.SetBrush(backBrush);
    memBb.DrawRectangle(0, 0, buffer->GetWidth(), buffer->GetHeight());
    wxBrush shadowBrush(wxColour(91, 91, 91));
    memBb.SetBrush(shadowBrush);
    memBb.DrawRectangle(shadowRect());
    memBb.DrawBitmap(bm, imgRect.GetPosition());
}

CanvasPanel::~CanvasPanel() {
    if(buffer) delete buffer;
}
