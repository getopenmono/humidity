#include "toucher.hpp"
#include "app_controller.h"

Toucher::Toucher (AppController * app)
:
	ctrl(app)
{
}

void Toucher::respondTouchBegin (mono::TouchEvent & unused)
{
	ctrl->handleTouch();
}
