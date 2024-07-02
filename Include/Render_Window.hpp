#ifndef RENDER_WINDOW_HPP
#define RENDER_WINDOW_HPP
#include "SFML/Graphics.hpp"
namespace Rendering {
	class WindowEventChannel {




	};
    class Render_Window : public sf::RenderWindow {
    protected:

    public:
        using sf::RenderWindow::RenderWindow;

        using sf::RenderWindow::draw;

        template<typename T>
        void draw(const T&);

    };
}


#endif // !RENDER_WINDOW_HPP

