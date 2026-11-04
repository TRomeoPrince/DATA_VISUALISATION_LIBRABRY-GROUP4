#include "Theme.hpp"
Theme Theme::light(){return Theme{};}
Theme Theme::dark(){Theme t;t.background="#171f2a";t.foreground="#ffffff";
t.grid="#465260";t.accent="#55b4ee";t.annotation="#ffc45b";return t;}