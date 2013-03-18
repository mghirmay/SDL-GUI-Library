//
//  GUIMsgNew.cpp
//  Coin Checkers 2
//
//  Created by Nathan Daly on 11/19/12.
//  Copyright (c) 2012 Lions Entertainment. All rights reserved.
//

#include "GUIMsgNew.h"
#include "GUITimer.h"
#include "GameDisplay.h"
#include "GUIButton.h"
#include "GUIApp.h"

#include <iostream>
using std::cout; using std::endl;

const SDL_Color color = {0xbb, 0xbb, 0xbb, 0};

GUIMsgNew::GUIMsgNew(int w_, int h_, const std::string& msg_, Button_ctrs_t buttons_)
:GUIView(w_,h_), num_buttons(0)
{    
    set_msg(msg_);
    
    for (int i = 0; i < buttons_.size(); i++) {
        add_button(buttons_[i]);
    }
    
}

void GUIMsgNew::set_msg(const std::string msg_) {
    
    SDL_Color text_color = {0,0,0,0};
    GUIImage msg_text(createText2(msg_, 16, text_color));

    GUIImage bg = GUIImage::create_filled(get_w(), get_h(), color);
    draw_onto_self(bg, DispPoint());
    
    draw_onto_self(msg_text, DispPoint(get_w()/2 - msg_text.getw()/2, 
                                       get_h()/2 - msg_text.geth()/2));
}
void GUIMsgNew::add_button(GUIButton* button) {
    
    attach_subview(button, DispPoint(30, 30 + num_buttons*20));
    num_buttons++;
}



GUIMsgNew_Scroll_In::GUIMsgNew_Scroll_In(int w_, int h_, const std::string& msg_, Button_ctrs_t buttons_)
:GUIMsgNew(w_,h_, msg_, buttons_)
{ }


void GUIMsgNew_Scroll_In::pop_up(int timeout) 
{
    // Store a copy of the screen for the scroll away.
    GUIImage temp_screen(get_w(), get_h());
    int SCREEN_WIDTH = GUIApp::get()->get_screen_size().x;
    SDL_Rect screen_rect = {SCREEN_WIDTH/2 - get_w()/2, 0,
                            get_w(), get_h()};
    SDL_Rect dest_rect = {0, 0, get_w(), get_h()};
    
	SDL_BlitSurface(SDL_GetVideoSurface(), &screen_rect,
                    temp_screen, &dest_rect);
    
    // Scroll in
    double speed = get_h()/30;
    for (double i = -1 * get_h(); i <= 0; i+= speed) {
        speed = (get_h() - i)/30.;
        displayToScreen(get_image_ptr(), SCREEN_WIDTH/2 - get_w()/2, i, 1);
    }
    
    displayToScreen(get_image_ptr(), SCREEN_WIDTH/2 - get_w()/2, 0, 1);

    // wait
    SDL_Event event;
    
    if (timeout >= 0) {
        
        GUITimer t;
        t.start();
        
        do {
            SDL_PollEvent(&event);
        }
        while (t.get_time() < timeout);
    }
    else {
        do {
            SDL_PollEvent(&event);
        }
        while (event.type != SDL_MOUSEBUTTONDOWN);
    }

    // scroll out
    speed = get_h()/30;
    for (double i = 0; i >= -1 * get_h(); i -= speed) {
        speed = (get_h() - (-1.0 * get_h() - i))/30.;
        displayToScreen(temp_screen, SCREEN_WIDTH/2 - get_w()/2, 0, 0);
        displayToScreen(get_image_ptr(), SCREEN_WIDTH/2 - get_w()/2, i, 0);
        SDL_UpdateRects(SDL_GetVideoSurface(), 1, &screen_rect);
    }
    displayToScreen(temp_screen, SCREEN_WIDTH/2 - get_w()/2, 0, 1);

}




