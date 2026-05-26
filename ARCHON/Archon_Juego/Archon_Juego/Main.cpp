#include <SFML/Graphics.hpp>
#include "Coordinador.h"

sf::RenderWindow* gVentana = nullptr;

int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Archon Clash Royale");
    window.setFramerateLimit(60);

    gVentana = &window;

    Coordinador coordinador;
    coordinador.inicializar();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            coordinador.gestionarEventos(window, event);
        }

        if (coordinador.salir()) window.close();

        coordinador.actualizar(window);

        window.clear(sf::Color(20, 20, 30));
        coordinador.dibujar(window);
        window.display();
    }

    return 0;
}