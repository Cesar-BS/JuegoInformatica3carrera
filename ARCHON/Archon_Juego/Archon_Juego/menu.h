#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

enum class EstadoMenu {
    MENU_PRINCIPAL,
    JUGANDO,
    INSTRUCCIONES,
    SALIR
};

class Menu {
private:
    sf::Texture texFondoNormal;
    sf::Sprite  sprBackground;
    sf::Font    font;

    std::vector<sf::Text> menuOptions;

    sf::Texture texInstrucciones;
    sf::Sprite  sprInstrucciones;
    bool        mostrandoInstrucciones = false;
    bool        texInstruccionesCargada = false;

    float anchoVentana, altoVentana;

    void ajustarFondo(const sf::Texture& texture);
    void configurarMenu();

public:
    Menu(float width, float height);
    bool cargarRecursos();
    void actualizar(sf::RenderWindow& window);
    void dibujar(sf::RenderWindow& window);
    EstadoMenu procesarEventos(sf::RenderWindow& window, sf::Event& event);
};