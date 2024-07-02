#pragma once
#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <SFML/Graphics.hpp>
//#include "Event_Supplier.hpp"
#include "Graph.hpp"
#include <unordered_map>
#include <random>
#include <unordered_set>
#include "Molecule.hpp"
#include "Animation.hpp"
#include "Time.hpp"


#define GAMESPACE_TO_RENDERSPACE 128
Rendering::Molecule_Primitive generate_nodes(int n);


Graph::Geometry_Graph generateNodes_(int n);

namespace Rendering {
    


    class View : public sf::View{
    protected:
        float ZoomFactor;

        float ZoomAmount;

        bool isPanning;

        sf::Vector2f panStart;

        const sf::Window* window;


    public:

       /* virtual Communication::Message handle(const Communication::Event& event) {
            switch (event.get_ID()) {
            case(Communication::Event::Event_ID::UI_EVENT):
                const sf::Event& casted = *(reinterpret_cast<const sf::Event*>(event.get_Data()));
                processEvent(casted);
            }




        }*/

        View() :window(nullptr), panStart({ 0,0 }), ZoomFactor(0.9), ZoomAmount(1.0), isPanning(false),sf::View() {}

        void update() {
            if (!isPanning || window == nullptr) return;
            auto panEnd = sf::Vector2f(sf::Mouse::getPosition(*window));
            auto zoomRatio = this->getSize().x / sf::Vector2f(window->getSize()).x;
            auto delta = panStart - panEnd;
            delta *= zoomRatio;
            move(delta.x,-delta.y);
            panStart = panEnd;

        }

        void setWindow(const sf::Window& target) {
            setSize(sf::Vector2f(target.getSize()).x, -sf::Vector2f(target.getSize()).y);
            window = &target;
        }

        void unsetWindow() {
            window = nullptr;
        }

        //sets the zoom factor, ie how much the view zooms for every unit of mouse wheel delta
        template<typename scaler>
        void setZoomFactor(const scaler& factor) {
            ZoomFactor = factor;
        }


        template<typename scaler>
        void setAspectRatio(const scaler& x, const scaler& y) {
            auto prevSize = this->getSize();
            auto ratio = y / x;
            this->setSize(prevSize.x,prevSize.y * ratio);

        }

        template<typename vector>
        void setAspectRatio(const vector& vec) {
            auto prevSize = this->getSize();
            auto ratio = vec.y / vec.x;
            this->setSize(prevSize.x, prevSize.y * ratio);
        }

        void processEvent(const sf::Event& event) {
            if (window == nullptr) return;
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    zoom(pow(ZoomFactor, event.mouseWheelScroll.delta));
                }
            }
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Middle) {
                isPanning = true;
                panStart = (sf::Vector2f)sf::Mouse::getPosition(*window);
            }
            else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Middle) {
                isPanning = false;
            }


        }
             

    };


    class Render_Window : public sf::RenderWindow{
    protected:
        
    public:
        using sf::RenderWindow::RenderWindow;

        using sf::RenderWindow::draw;

        template<typename T>
        void draw(const T&);

    };

	class Renderer {
    protected:
        Renderer();

        Render_Window window;

        View* view;
	public:

        static sf::Font font;

        const View* getView() const;

        const Render_Window& getWindow() const;

        static Renderer& instance();

        void Render();

	};










}




#endif