//
//  GUIView.cpp
//  Deep
//
//  Created by Nathan Daly on 9/10/12.
//  Copyright (c) 2012 Lions Entertainment. All rights reserved.
//

#include "GUIView.h"
#include "GUIApp.h"
#include "GUIImage.h"

#include "SDL/SDL_video.h"

#include "GUIWindow.h" // For Unhandled Click.
#include "GUIApp.h"  // For capture focus


#include <iostream>
#include <algorithm> // For std::find.
using namespace std;

SDL_Surface* prepare_SDL_surface(int w, int h);

const SDL_Color default_clear_color = {255,0,255,0};


GUIView::GUIView(int w_, int h_) 
:changed(false), w(w_), h(h_), image(prepare_SDL_surface(w_, h_)),
display(prepare_SDL_surface(w_, h_)), is_alpha(false),
parent(0)
{ 
    
}

SDL_Surface* prepare_SDL_surface(int w, int h) {
    
    Uint32 rmask, gmask, bmask, amask;
	
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
	
    SDL_Surface *temp = SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, 32,
                                             rmask, gmask, bmask, amask);
    if(temp == NULL) {
		throw GUIError("CreateRGBSurface failed: \n" + string(SDL_GetError()));
    }
	SDL_Surface* image = SDL_DisplayFormat(temp);
	if(!image) {
        throw GUIError("updateimage in CreateRGBSurface failed: \n" + string(SDL_GetError()));
    }
	SDL_FreeSurface(temp);
    
    return image;
}


GUIView::~GUIView() {
    
    while (!children.empty()) {
        
        delete children.front();
        children.pop_front();
    }
    
    SDL_FreeSurface(image);
    SDL_FreeSurface(display);
}

void GUIView::draw_onto_self(const GUIImage &image_, DispPoint pos) {
    
    mark_changed();
    
    // Using SDL, perform a blit from view to self.
	SDL_Rect dest_rect = {pos.x, pos.y, image_->w, image_->h};
	SDL_BlitSurface(image_, 0, image, &dest_rect);

}
void GUIView::fill_with_color(SDL_Color color) {
   
    draw_onto_self(GUIImage::create_filled(w, h, color), DispPoint());
}


// Draws image onto display.
void GUIView::render_image(SDL_Surface* source, int w, int h, DispPoint pos) {
    
    // Using SDL, perform a blit from view to self.
	SDL_Rect dest_rect = {pos.x, pos.y, w, h};
	SDL_BlitSurface(source, 0, display, &dest_rect);
}

void GUIView::set_clear_color(SDL_Color clear_color_) {
    
    is_alpha = true;
    clear_color = clear_color_;
    Uint32 colorkey = SDL_MapRGBA(image->format, clear_color.r, clear_color.g, clear_color.b, clear_color.unused);
    SDL_SetColorKey(display, SDL_SRCCOLORKEY, colorkey); // reset alpha
}
void GUIView::clear_alpha() {
    
    is_alpha = false;
    SDL_SetColorKey(display, 0, 0); // reset alpha
}


bool x_then_y_view_less_than(const GUIView* a, const GUIView* b) {
    if (a->pos.x < b->pos.x) return true;
    else if (a->pos.x == b->pos.x) return (a->pos.y < b->pos.y);
    else /*(a->pos.x > b->pos.x)*/ return false;
}

void GUIView::mark_changed() {
    
    changed = true;
    
    if (parent) parent->mark_changed();
    
    if (children.size() > 1) {
        
        /// @todo Perhaps the sorting method should be optional?
        ///  Either by x,y or by order attached?
        
//        children.sort(x_then_y_view_less_than);
    }
}


void GUIView::refresh() {
    
    if (!need_to_refresh()) return;
    
    // Refresh self. (First display image, then each child.)
    
    render_image(image, w, h, DispPoint(0,0));
    
    Subview_list_t::iterator child;
    for(child = children.begin(); child != children.end(); child++) {
        
        (*child)->refresh();
                
        render_image((*child)->display, (*child)->w, (*child)->h, (*child)->pos);
    }
    
    changed = false;
}

void GUIView::attach_subview(GUIView* view, DispPoint pos) {
    if (view->parent)
        throw GUIError("Candidate vew is already a subview of another view.");
    
    if (view == this) 
        throw GUIError("Cannot attach a view to itself!");

    /// @todo Check if out of bounds? Or maybe not..?
    
    view->pos = pos;
    children.push_back(view);
    view->parent = this;
    
    mark_changed();
}
// NOTE: Does not delete the view, only removes it from list!
void GUIView::remove_subview(GUIView* view) {
    
    if (!is_subview(view))
        throw GUIError("view is not a subview of this!");
    
    children.remove(view);
    view->parent = 0;
    
    mark_changed();
}
void GUIView::remove_last_subview() {
    
    if (children.empty())
        throw GUIError("view has not subviews!");
    
    GUIView *view = children.back();
    children.pop_back();
    view->parent = 0;
    
    mark_changed();
}
bool GUIView::is_subview(GUIView* view) {
    
    return (find(children.begin(), children.end(), view) != children.end());
}

void GUIView::move_subview(GUIView* view, DispPoint pos) {
    
    if (!is_subview(view))
        throw GUIError("view is not a subview of this!");
    
    if (view->pos == pos) return; // no need to mark change if already there!
    
    view->pos = pos;
    
    mark_changed();
}

void GUIView::mouse_down(DispPoint coord) {
    cout << "mouse down!: " << coord.x <<", "<< coord.y << endl;
    
    if (!handle_mouse_down(coord)) {
        if (parent) parent->mouse_down(coord + pos);
//        else throw Unhandled_Click(coord);
    }
}
void GUIView::mouse_up(DispPoint coord) {
    cout << "mouse up!: " << coord.x <<", "<< coord.y << endl;
    
    if (!handle_mouse_up(coord)) {
        if (parent) parent->mouse_up(coord + pos);
//        else throw Unhandled_Click(coord);
    }
}
void GUIView::mouse_motion(DispPoint coord, DispPoint rel_motion) {
//    cout << "mouse motion!: " << rel_motion.x <<", "<< rel_motion.y << endl;
    
    if (!handle_mouse_motion(coord, rel_motion)) {
        if (parent) parent->mouse_motion(coord + pos, rel_motion);
//        else throw Unhandled_Click(coord);
    }
}
void GUIView::mouse_scroll_start(bool up_down) {
    cout << "mouse scroll!: " << (up_down ? "up" : "down") << endl;
    
    if (!handle_mouse_scroll_start(up_down)) {
        if (parent) parent->mouse_scroll_start(up_down);
        //        else throw Unhandled_Click(coord);
    }
}void GUIView::mouse_scroll_stop(bool up_down) {
    cout << "mouse scroll!: " << (up_down ? "up" : "down") << endl;
    
    if (!handle_mouse_scroll_stop(up_down)) {
        if (parent) parent->mouse_scroll_stop(up_down);
        //        else throw Unhandled_Click(coord);
    }
}


void GUIView::key_down(SDL_keysym key) {
    cout << "key down!: " << key.sym << endl;
    
    if (!handle_key_down(key)) {
//        throw Unhandled_Key(key);
    }
}
void GUIView::key_up(SDL_keysym key) {
    cout << "key up!: " << key.sym << endl;
    
    if (!handle_key_up(key)) {
//        throw Unhandled_Key(key);
    }
}


GUIView* GUIView::get_view_from_point(DispPoint coord) const {
    
    // Can we cache this? Would that be faster?
    
    coord = adjust_to_rel(coord);
    if (!rel_point_is_on_me(coord)) return 0;
        
    // At worst, we know the point is on this view.
    const GUIView* result = this;
    
    // Check if any children have a deeper subview:
    Subview_list_t::const_iterator child;
    for (child = children.begin(); child != children.end(); ++child) {
        
        // Can assume that Views are sorted, so any new best will be above old best.
        GUIView* new_best = (*child)->get_view_from_point(coord);
        if (new_best) {
            result = new_best;
        }
    }
    
    return const_cast<GUIView*>(result);
}

bool GUIView::rel_point_is_on_me(DispPoint coord) const {
    
    return (coord.x >= 0 && coord.y >= 0
            && coord.x < w && coord.y < h);
}
//bool GUIView::rel_point_is_on_me(DispPoint coord) {
//    
//    return (coord.x >= pos.x && coord.y >= pos.y
//            && coord.x < pos.x + w && coord.y < pos.y + h);
//}
bool GUIView::abs_point_is_on_me(DispPoint coord) const {
    
    DispPoint abs_pos = get_abs_pos();
    return (coord.x >= abs_pos.x && coord.y >= abs_pos.y
            && coord.x < abs_pos.x + w && coord.y < abs_pos.y + h);
}


DispPoint GUIView::abs_from_rel(DispPoint coord) const {
    
    DispPoint abs_pos = get_abs_pos();
        return DispPoint(abs_pos.x + coord.x,
                         abs_pos.y + coord.y);
}
DispPoint GUIView::adjust_to_parent(DispPoint coord) const {
    return DispPoint(coord.x + pos.x, coord.y + pos.y);
}
DispPoint GUIView::adjust_to_rel(DispPoint coord) const {
    return DispPoint(coord.x - pos.x, coord.y - pos.y);
}


DispPoint GUIView::get_abs_pos() const {
    if (parent == 0) return pos;
    else {
        DispPoint parent_abs_pos = parent->get_abs_pos();
        return DispPoint(parent_abs_pos.x + pos.x,
                         parent_abs_pos.y + pos.y);
    }
}
DispPoint GUIView::get_rel_pos() const {
    return pos;
}


void GUIView::resize(int w_, int h_) {
    
    if (w == w_ && h == h_) {
        
        did_resize(w_,h_);
        return;
    }
    
    w = w_; h = h_;
    GUIView temp(w,h);

    std::swap(image, temp.image);
    std::swap(display, temp.display);
    
    if (is_alpha) {
        set_clear_color(clear_color);
    }
    
    SDL_Rect dest_rect = {0, 0, w, h};
	SDL_BlitSurface(temp.image, 0, image, &dest_rect);

    mark_changed();
    did_resize(w_,h_);
}







