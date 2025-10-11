#include "page.h"

class MainPage : public Page {
    public:
        MainPage();
        void render(SDL_Renderer* renderer) override;

        void handleEvent(const SDL_Event& event) override;

    private:
        bool touched = false;
        int touchX = 0;
        int touchY = 0;


};